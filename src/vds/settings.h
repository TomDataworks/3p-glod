/******************************************************************************
 * Copyright 2004 David Luebke, Brenden Schubert                              *
 *                University of Virginia                                      *
 ******************************************************************************
 * This file is distributed as part of the VDSlib library, and, as such,      *
 * falls under the terms of the VDSlib public license. VDSlib is distributed  *
 * without any warranty, implied or otherwise. See the VDSlib license for     *
 * more details.                                                              *
 *                                                                            *
 * You should have recieved a copy of the VDSlib Open-Source License with     *
 * this copy of VDSlib; if not, please visit the VDSlib web page,             *
 * http://vdslib.virginia.edu/license for more information.                   *
 ******************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H

// uncomment for single-threaded mode
#define SINGLE_THREADED

// uncomment for pruning (not keeping the leaf nodes of the tree in the unfoldqueue)
#define PRUNING

// uncomment for reverse-pruning (not keeping the portion of the tree above 
// the parents of the boundary in the foldqueue)
#define REVERSE_PRUNING

// displays messages when resizing queues/caches
//#define VERBOSE_MEM_MANAGEMENT

// uncomment for performance outputs
//#define PERFORMANCE_OUTPUT

// uncomment to generate timing information
// higher levels delve deeper into the subprocedures
//#define TIMING_LEVEL_1 // basic time spent updating simplification params and node errors, simplifying, rendering
//#define TIMING_LEVEL_2 // breakdown of updating node errors - calculating errors and heapify for each queue
						 // also tracks the average sizes of the nodequeues
//#define TIMING_LEVEL_3
//#define TIMING_LEVEL_4
//#define TIMING_LEVEL_5 // finding and adding VertexRenderDatums and TriRenderDatums

#ifndef WIN32
#undef PERFORMANCE_OUTPUT
#undef TIMING_LEVEL_1
#undef TIMING_LEVEL_2
#undef TIMING_LEVEL_3
#undef TIMING_LEVEL_4
#undef TIMING_LEVEL_5
#endif

#endif // #ifndef SETTINGS_H
