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

#include <glcomp/glcompdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

    extern glCompPanel *glCompPanelNew(glCompObj * parentObj, GLfloat x,
				       GLfloat y, GLfloat w, GLfloat h);

/*events*/
    extern int glCompPanelDraw(glCompObj * o);

#ifdef __cplusplus
}
#endif
