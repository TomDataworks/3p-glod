/******************************************************************************
 * Copyright 2003 Jonathan Cohen, Nat Duca, David Luebke, Brenden Schubert    *
 *                Johns Hopkins University and University of Virginia         *
 ******************************************************************************
 * This file is distributed as part of the GLOD library, and as such, falls   *
 * under the terms of the GLOD public license. GLOD is distributed without    *
 * any warranty, implied or otherwise. See the GLOD license for more details. *
 *                                                                            *
 * You should have recieved a copy of the GLOD Open-Source License with this  *
 * copy of GLOD; if not, please visit the GLOD web page,                      *
 * http://www.cs.jhu.edu/~graphics/GLOD/license for more information          *
 ******************************************************************************/
// at some point, it makes sense for glod to simply link to and use the 
// stderror and stdrender libraries included with the vds distribution,
// replacing these callbacks, as they are just copied from those libraries
// (probably not worth the trouble to do it now, but putting this here as a
// reminder)

#ifndef VDS_CALLBACKS_H
#define VDS_CALLBACKS_H

#include <stdio.h>
#include "vds.h"
#include "vdsaux.h"

void ImmediateModeRenderCallback(VDS::Renderer &rRenderer, VDS::PatchIndex PatchID);
void FastRenderCallback(VDS::Renderer &rRenderer, VDS::PatchIndex PatchID);
void VBOFastRenderCallback(VDS::Renderer &rRenderer, VDS::PatchIndex PatchID);
VDS::Float StdErrorScreenSpace(VDS::BudgetItem *pItem, const VDS::Cut *pCut);
VDS::Float StdErrorScreenSpaceNoFrustum(VDS::BudgetItem *pItem, const VDS::Cut *pCut);
VDS::Float StdErrorObjectSpace(VDS::BudgetItem *pItem, const VDS::Cut *pCut);
VDS::Float StdErrorObjectSpaceNoFrustum(VDS::BudgetItem *pItem, const VDS::Cut *pCut);

#endif // #ifndef VDS_CALLBACKS
