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
#include <neatogen/digcola.h>
#ifdef DIGCOLA
#include <neatogen/matrix_ops.h>
#include <neatogen/conjgrad.h>
#include <stddef.h>

static void construct_b(vtx_data * graph, int n, double *b)
{
    /* construct a vector - b s.t. -b[i]=\sum_j -w_{ij}*\delta_{ij}
     * (the "balance vector")
     * Note that we build -b and not b, since our matrix is not the 
     * real laplacian L, but its negation: -L. 
     * So instead of solving Lx=b, we will solve -Lx=-b
     */
    int i;

    double b_i = 0;

    for (i = 0; i < n; i++) {
	b_i = 0;
	if (graph[0].edists == NULL) {
	    continue;
	}
	for (size_t j = 1; j < graph[i].nedges; j++) { // skip the self loop
	    b_i += graph[i].ewgts[j] * graph[i].edists[j];
	}
	b[i] = b_i;
    }
}

#define hierarchy_cg_tol 1e-3

int
compute_y_coords(vtx_data * graph, int n, double *y_coords,
		 int max_iterations)
{
    /* Find y coords of a directed graph by solving L*x = b */
    int i, rv = 0;
    double *b = gv_calloc(n, sizeof(double));
    double tol = hierarchy_cg_tol;
    size_t nedges = 0;
    float *old_ewgts = graph[0].ewgts;

    construct_b(graph, n, b);

    init_vec_orth1(n, y_coords);

    for (i = 0; i < n; i++) {
	nedges += graph[i].nedges;
    }

    /* replace original edge weights (which are lengths) with uniform weights */
    /* for computing the optimal arrangement */
    float *uniform_weights = gv_calloc(nedges, sizeof(float));
    for (i = 0; i < n; i++) {
	graph[i].ewgts = uniform_weights;
	uniform_weights[0] = (float) -(graph[i].nedges - 1);
	for (size_t j = 1; j < graph[i].nedges; j++) {
	    uniform_weights[j] = 1;
	}
	uniform_weights += graph[i].nedges;
    }

    if (conjugate_gradient(graph, y_coords, b, n, tol, max_iterations) < 0) {
	rv = 1;
    }

    /* restore original edge weights */
    free(graph[0].ewgts);
    for (i = 0; i < n; i++) {
	graph[i].ewgts = old_ewgts;
	old_ewgts += graph[i].nedges;
    }

    free(b);
    return rv;
}

#endif				/* DIGCOLA */
