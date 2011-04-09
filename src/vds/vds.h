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

//  conventions
//    m before all member variables
//    i means the variable is an index into an array
//    p means the variable is a pointer to data
//    f means the variable is a function pointer
//    r means reference
//    g means variable is a global
//    comment types:
//    :TODO:  something that needs to be done
//    :DEPENDENCY: a constant's value or relative value is explicitly relied on
//                 or one function relies on some peculiar or non-obvious behavior
//                 of another.
//    :KLUDGE: A hack or expensive way of doing something

#ifndef VDS_H
#define VDS_H

#ifdef _WIN32
# include <windows.h>
# include <iostream>
# include <vector>
# include <GL/gl.h>
# include "zthreads.h"
# include <assert.h>
#else
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
# include <GL/gl.h>
# include <GL/glx.h>
# include <GL/glext.h>
#endif
#endif

#include "settings.h"
#include "primtypes.h"

#ifndef _WIN32
typedef union _LARGE_INTEGER {
	struct {
		unsigned int LowPart;
		long HighPart;
	};
	long long QuadPart;
} LARGE_INTEGER;
# define BYTE unsigned char
# define DWORD unsigned int 
#endif

// disable warning C4786: symbol greater than 255 character,
// okay to ignore, STL generates many of these.
#ifdef _WIN32
#pragma warning(disable: 4786)
#endif

#ifndef NULL
#define NULL 0
#endif

namespace VDS
{
	struct BudgetItem;
	
	typedef unsigned long NodeIndex;
	typedef unsigned long TriIndex;
	typedef unsigned int ProxyIndex;
	typedef unsigned short PatchIndex;

	typedef void* UserNodeData;
	typedef void* UserForestData;

	#define FOLD_NODE false
	#define UNFOLD_NODE true
	
	struct TriProxy
	{
		ProxyIndex proxies[3];
		inline ProxyIndex &operator [](unsigned int iProxy)
		{
			assert(iProxy == 0 || iProxy == 1 || iProxy ==2);
			return proxies[iProxy];
		}
		inline ProxyIndex operator [](unsigned int iProxy) const
		{
			assert(iProxy == 0 || iProxy == 1 || iProxy ==2);
			return proxies[iProxy];
		}
	};

	struct TriProxyBackRef
	{
		NodeIndex backrefs[3];
		inline NodeIndex &operator [](unsigned int iProxy)
		{
			assert(iProxy == 0 || iProxy == 1 || iProxy ==2);
			return backrefs[iProxy];
		}
		inline NodeIndex operator [](unsigned int iProxy) const
		{
			assert(iProxy == 0 || iProxy == 1 || iProxy ==2);
			return backrefs[iProxy];
		}

        TriIndex miNextLiveTris[3];
	};

	class VertexRenderDatum
	{
	public:
		VertexRenderDatum() { }

		Point3 Position;
		Vec3 Normal;
		ByteColorA Color;
		Point2 TexCoords;

		// node index for debugging purposes
		NodeIndex Node;
	};

	struct ViewIndependentError
	{
		void *ErrorData;
		int Size;
	};

	class NodeQueue;
	class FreeList;
	struct PQElement;
	class Forest;
	class Node;
	class Tri;
	class Cut;
	class Simplifier;
	class Renderer;
    class ForestBuilder; // this needs to be def'd to grant it friend access to other classes
	class Manager;

	typedef void (*RenderFunc)(Renderer &, PatchIndex);
	typedef Float (*ErrorFunc)(BudgetItem *, const Cut *);
	typedef ViewIndependentError &(*ViewIndependentErrorFunc)(Node *, const Forest &);
	
} //namespace VDS

#ifdef _WIN32
// Performance timing variables
	static LARGE_INTEGER times[100];
/*	[0] timer_start_time
	[1] timer_last_display
	[2] timer_simplify_time_total
	[3] timer_render_time_total
	[4] timer_update_simp_params_total
	[5] 
	[6] timer_simplification_total
	[7] fold_time
	[8] unfold_time
	[9] fold_check
	[10] fold_remove_children
	[11] fold_remove_subtris
	[12] fold_queue_manip
	[13] unfold_activate_children
	[14] unfold_move_livetris
	[15] unfold_activate_subtris
	[16] unfold_queue_manip
	[17] unfold_finding_trirenderdatum_slot
	[18] unfold_adding_trirenderdatum
	[19] unfold_finding_vertexrenderdatum_slot
	[20] unfold_adding_vertexrenderdatum
	[21] update_budget_errors_calc
	[22] update_budget_errors_queue_manip
	[23] total time spent in draw()
	[24] total time spent in DrawCuts()
	[25] time updating node errors
	[26] time calculating errors for foldqueue
	[27] time calculating errors for unfoldqueue
	[28] time for heapify of foldqueue
	[29] time for heapify of unfoldqueue
	[30] time spent in copy of renderdata to fast memory

*/
	static bool timer_on;
	static LARGE_INTEGER PerfFreq;

	static unsigned int frame_counter;
	static unsigned int tris_introduced;
	static unsigned int tris_removed;
	static unsigned int foldqueue_size;
	static unsigned int unfoldqueue_size;
	static unsigned int queue_size_counter;
	static unsigned int tris_rendered;
	static unsigned int tris_rendered_total;
	static float render_time_total;
	static float simp_time_total;
	static unsigned int vertex_cache_size;
	static float acmr;
#endif

#endif //#ifndef VDS_H
