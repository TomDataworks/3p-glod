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

#ifndef _INCLUDED_XBS_ENUMS_H
#define _INCLUDED_XBS_ENUMS_H

enum QueueMode  { Greedy, Lazy, Independent, Randomized };
enum OperationType  { Vertex_Cluster, Vertex_Pair, Edge_Collapse,
                      Half_Edge_Collapse };
enum OutputType { MT_Hierarchy, Discrete_Hierarchy, DiscretePatch_Hierarchy,
                  VDS_Hierarchy };
enum SnapshotMode { PercentReduction, ManualTriSpec, ManualErrorSpec };

#endif /* _INCLUDED_XBS_ENUMS_H */
