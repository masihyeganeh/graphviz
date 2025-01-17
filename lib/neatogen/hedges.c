/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <cgraph/alloc.h>
#include <neatogen/mem.h>
#include <neatogen/hedges.h>
#include <common/render.h>
#include <stdbool.h>

#define DELETED -2

Halfedge *ELleftend, *ELrightend;

static Freelist hfl;
static int ELhashsize;
static Halfedge **ELhash;
static int ntry, totalsearch;

void ELcleanup(void)
{
    freeinit(&hfl, sizeof(**ELhash));
    free(ELhash);
    ELhash = NULL;
}

void ELinitialize(void)
{
    int i;

    freeinit(&hfl, sizeof(**ELhash));
    ELhashsize = 2 * sqrt_nsites;
    if (ELhash == NULL)
	ELhash = gv_calloc(ELhashsize, sizeof(Halfedge *));
    for (i = 0; i < ELhashsize; ++i)
	ELhash[i] = NULL;
    ELleftend = HEcreate(NULL, 0);
    ELrightend = HEcreate(NULL, 0);
    ELleftend->ELleft = NULL;
    ELleftend->ELright = ELrightend;
    ELrightend->ELleft = ELleftend;
    ELrightend->ELright = NULL;
    ELhash[0] = ELleftend;
    ELhash[ELhashsize - 1] = ELrightend;
}


Site *hintersect(Halfedge * el1, Halfedge * el2)
{
    Edge *e1, *e2, *e;
    Halfedge *el;
    double d, xint, yint;
    bool right_of_site;
    Site *v;

    e1 = el1->ELedge;
    e2 = el2->ELedge;
    if (e1 == NULL || e2 == NULL)
	return NULL;
    if (e1->reg[1] == e2->reg[1])
	return NULL;

    d = e1->a * e2->b - e1->b * e2->a;
    if (-1.0e-10 < d && d < 1.0e-10)
	return NULL;

    xint = (e1->c * e2->b - e2->c * e1->b) / d;
    yint = (e2->c * e1->a - e1->c * e2->a) / d;

    if (e1->reg[1]->coord.y < e2->reg[1]->coord.y ||
	(e1->reg[1]->coord.y == e2->reg[1]->coord.y &&
	 e1->reg[1]->coord.x < e2->reg[1]->coord.x)) {
	el = el1;
	e = e1;
    } else {
	el = el2;
	e = e2;
    };
    right_of_site = xint >= e->reg[1]->coord.x;
    if ((right_of_site && el->ELpm == le) || (!right_of_site && el->ELpm == re))
	return NULL;

    v = getsite();
    v->refcnt = 0;
    v->coord.x = xint;
    v->coord.y = yint;
    return v;
}

/* returns 1 if p is to right of halfedge e */
int right_of(Halfedge * el, Point * p)
{
    Edge *e;
    Site *topsite;
    int above, fast;
    double dxp, dyp, dxs, t1, t2, t3, yl;

    e = el->ELedge;
    topsite = e->reg[1];
    bool right_of_site = p->x > topsite->coord.x;
    if (right_of_site && el->ELpm == le)
	return 1;
    if (!right_of_site && el->ELpm == re)
	return 0;

    if (e->a == 1.0) {
	dyp = p->y - topsite->coord.y;
	dxp = p->x - topsite->coord.x;
	fast = 0;
	if ((!right_of_site && e->b < 0.0) || (right_of_site && e->b >= 0.0)) {
	    above = dyp >= e->b * dxp;
	    fast = above;
	} else {
	    above = p->x + p->y * e->b > e->c;
	    if (e->b < 0.0)
		above = !above;
	    if (!above)
		fast = 1;
	};
	if (!fast) {
	    dxs = topsite->coord.x - (e->reg[0])->coord.x;
	    above = e->b * (dxp * dxp - dyp * dyp) <
		dxs * dyp * (1.0 + 2.0 * dxp / dxs + e->b * e->b);
	    if (e->b < 0.0)
		above = !above;
	};
    } else {			/*e->b==1.0 */
	yl = e->c - e->a * p->x;
	t1 = p->y - yl;
	t2 = p->x - topsite->coord.x;
	t3 = yl - topsite->coord.y;
	above = t1 * t1 > t2 * t2 + t3 * t3;
    };
    return el->ELpm == le ? above : !above;
}

Halfedge *HEcreate(Edge * e, char pm)
{
    Halfedge *answer;
    answer = getfree(&hfl);
    answer->ELedge = e;
    answer->ELpm = pm;
    answer->PQnext = NULL;
    answer->vertex = NULL;
    answer->ELrefcnt = 0;
    return answer;
}


void ELinsert(Halfedge * lb, Halfedge * new)
{
    new->ELleft = lb;
    new->ELright = lb->ELright;
    (lb->ELright)->ELleft = new;
    lb->ELright = new;
}

/* Get entry from hash table, pruning any deleted nodes */
static Halfedge *ELgethash(int b)
{
    Halfedge *he;

    if (b < 0 || b >= ELhashsize)
	return NULL;
    he = ELhash[b];
    if (he == NULL || he->ELedge != (Edge *) DELETED)
	return he;

/* Hash table points to deleted half edge.  Patch as necessary. */
    ELhash[b] = NULL;
    if (--he->ELrefcnt == 0)
	makefree(he, &hfl);
    return NULL;
}

Halfedge *ELleftbnd(Point * p)
{
    int i, bucket;
    Halfedge *he;

/* Use hash table to get close to desired halfedge */
    bucket = (p->x - xmin) / deltax * ELhashsize;
    if (bucket < 0)
	bucket = 0;
    if (bucket >= ELhashsize)
	bucket = ELhashsize - 1;
    he = ELgethash(bucket);
    if (he == NULL) {
	for (i = 1; ; ++i) {
	    if ((he = ELgethash(bucket - i)) != NULL)
		break;
	    if ((he = ELgethash(bucket + i)) != NULL)
		break;
	};
	totalsearch += i;
    };
    ++ntry;
/* Now search linear list of halfedges for the corect one */
    if (he == ELleftend || (he != ELrightend && right_of(he, p))) {
	do {
	    he = he->ELright;
	} while (he != ELrightend && right_of(he, p));
	he = he->ELleft;
    } else
	do {
	    he = he->ELleft;
	} while (he != ELleftend && !right_of(he, p));

/* Update hash table and reference counts */
    if (bucket > 0 && bucket < ELhashsize - 1) {
	if (ELhash[bucket] != NULL)
	    --ELhash[bucket]->ELrefcnt;
	ELhash[bucket] = he;
	++ELhash[bucket]->ELrefcnt;
    };
    return he;
}


/* This delete routine can't reclaim node, since pointers from hash
   table may be present.   */
void ELdelete(Halfedge * he)
{
    (he->ELleft)->ELright = he->ELright;
    (he->ELright)->ELleft = he->ELleft;
    he->ELedge = (Edge *) DELETED;
}


Halfedge *ELright(Halfedge * he)
{
    return he->ELright;
}

Halfedge *ELleft(Halfedge * he)
{
    return he->ELleft;
}


Site *leftreg(Halfedge * he)
{
    if (he->ELedge == NULL)
	return (bottomsite);
    return he->ELpm == le ? he->ELedge->reg[le] : he->ELedge->reg[re];
}

Site *rightreg(Halfedge * he)
{
    if (he->ELedge == NULL)
	return (bottomsite);
    return he->ELpm == le ? he->ELedge->reg[re] : he->ELedge->reg[le];
}
