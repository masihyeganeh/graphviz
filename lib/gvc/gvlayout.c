/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

/*
 *  layout engine wrapper
 *
 */

#include "config.h"

#include <common/const.h>
#include <gvc/gvplugin_layout.h>
#include <gvc/gvcint.h>
#include <cgraph/cgraph.h>
#include <gvc/gvcproc.h>
#include <gvc/gvc.h>
#include <stdbool.h>
#include <stddef.h>

extern void graph_init(Agraph_t *g, bool use_rankdir);
extern void graph_cleanup(Agraph_t *g);
extern void gv_fixLocale (int set);
extern void gv_initShapes (void);

int gvlayout_select(GVC_t * gvc, const char *layout)
{
    gvplugin_available_t *plugin;
    gvplugin_installed_t *typeptr;

    plugin = gvplugin_load(gvc, API_layout, layout, NULL);
    if (plugin) {
	typeptr = plugin->typeptr;
	gvc->layout.type = typeptr->type;
	gvc->layout.engine = typeptr->engine;
	gvc->layout.id = typeptr->id;
	gvc->layout.features = typeptr->features;
	return GVRENDER_PLUGIN;  /* FIXME - need better return code */
    }
    return NO_SUPPORT;
}

/* gvLayoutJobs:
 * Layout input graph g based on layout engine attached to gvc.
 * Check that the root graph has been initialized. If not, initialize it.
 * Return 0 on success.
 */
int gvLayoutJobs(GVC_t * gvc, Agraph_t * g)
{
    gvlayout_engine_t *gvle;
    char *p;
    int rc;

    agbindrec(g, "Agraphinfo_t", sizeof(Agraphinfo_t), true);
    GD_gvc(g) = gvc;
    if (g != agroot(g)) {
        agbindrec(agroot(g), "Agraphinfo_t", sizeof(Agraphinfo_t), true);
        GD_gvc(agroot(g)) = gvc;
    }

    if ((p = agget(g, "layout"))) {
        gvc->layout.engine = NULL;
	rc = gvlayout_select(gvc, p);
	if (rc == NO_SUPPORT) {
	    agerr (AGERR, "Layout type: \"%s\" not recognized. Use one of:%s\n",
	        p, gvplugin_list(gvc, API_layout, p));
	    return -1;
	}
    }

    gvle = gvc->layout.engine;
    if (! gvle)
	return -1;

    gv_fixLocale (1);
    graph_init(g, !!(gvc->layout.features->flags & LAYOUT_USES_RANKDIR));
    GD_drawing(agroot(g)) = GD_drawing(g);
    gv_initShapes ();
    if (gvle && gvle->layout) {
	gvle->layout(g);


	if (gvle->cleanup)
	    GD_cleanup(g) = gvle->cleanup;
    }
    gv_fixLocale (0);
    return 0;
}

bool gvLayoutDone(Agraph_t * g)
{
    return LAYOUT_DONE(g);
}

/* gvFreeLayout:
 * Free layout resources.
 * First, if the graph has a layout-specific cleanup function attached,
 * use it and reset.
 * Then do the general graph cleanup.
 */
int gvFreeLayout(GVC_t * gvc, Agraph_t * g)
{
    (void)gvc;

    /* skip if no Agraphinfo_t yet */
    if (! agbindrec(g, "Agraphinfo_t", 0, true))
	    return 0;

    if (GD_cleanup(g)) {
	(GD_cleanup(g))(g);
	GD_cleanup(g) = NULL;
    }
    
    graph_cleanup(g);
    return 0;
}
