/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sfio/sfio.h>
#include <parse.h>
#include <gprstate.h>
#include <expr/expr.h>
#include <stddef.h>
#include <stdio.h>

    typedef struct {
	Exnode_t *guard;
	Exnode_t *action;
    } case_stmt;

#define UDATA "userval"

    typedef struct {
	Agrec_t h;
	Extype_t iu;
	Agedge_t* ine;
    } nval_t;

    typedef struct {
	Agrec_t h;
	char lock;
    } gval_t;

    typedef struct {
	Agrec_t h;
    } uval_t;

#define OBJ(p) ((Agobj_t*)p)

    typedef nval_t ndata;
    typedef uval_t edata;
    typedef gval_t gdata;

#define nData(n)    ((ndata*)(aggetrec(n,UDATA,0)))
#define gData(g)    ((gdata*)(aggetrec(g,UDATA,0)))

#define SRCOUT    0x1
#define INDUCE    0x2
#define CLONE     0x4

#define WALKSG    0x1
#define BEGG      0x2
#define ENDG      0x4

    typedef struct {
	Exnode_t *begg_stmt;
	int walks;
	size_t n_nstmts;
	size_t n_estmts;
	case_stmt *node_stmts;
	case_stmt *edge_stmts;
    } comp_block; 

    typedef struct {
	int flags;
	Expr_t *prog;
	Exnode_t *begin_stmt;
	size_t n_blocks;
	comp_block  *blocks;
	Exnode_t *endg_stmt;
	Exnode_t *end_stmt;
    } comp_prog;

    extern comp_prog *compileProg(parse_prog *, Gpr_t *, int);
    extern void freeCompileProg (comp_prog *p);
    extern int usesGraph(comp_prog *);
    extern int walksGraph(comp_block *);
    extern Agraph_t *readG(FILE *fp);
    extern Agraph_t *openG(char *name, Agdesc_t);
    extern Agraph_t *openSubg(Agraph_t * g, char *name);
    extern Agnode_t *openNode(Agraph_t * g, char *name);
    extern Agedge_t *openEdge(Agraph_t* g, Agnode_t * t, Agnode_t * h, char *key);

#ifdef __cplusplus
}
#endif
