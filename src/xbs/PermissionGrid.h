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

/* Protection from multiple includes. */
#ifndef INCLUDED_PERMISSION_GRID_H
#define INCLUDED_PERMISSION_GRID_H

#define DEBUG_PERMISSION_GRID 1

typedef unsigned char GridDataType;
const unsigned char pgone = 0x80;
#include <Model.h>

// small class to deal with int vectors
class int3
{
public:
    int3 (int a=0, int b=0, int c=0)
    {
        data[0] = a;
        data[1] = b;
        data[2] = c;
    };
    int &operator[] (int index)
    {
        return data[index];
    };
    int3 &operator = (const xbsVec3 &vec)
    {
        data[0] = (int)vec.data[0];
        data[1] = (int)vec.data[1];
        data[2] = (int)vec.data[2];
        return *this;
    };
    int3 &operator -= (const int3 &vec)
    {
        data[0] -= vec.data[0];
        data[1] -= vec.data[1];
        data[2] -= vec.data[2];
        return *this;
    };
    int3 operator - (const int3 &vec) const
    {
        int3 result = *this;
        result -= vec;
        return result;
    };
    inline void set (const xbsVec3 & vec)
    {
        data[0] = (int)(vec.data[0]);
        data[1] = (int)(vec.data[1]);
        data[2] = (int)(vec.data[2]);
    };
    inline void increment()
    {
        ++data[0];
        ++data[1];
        ++data[2];
    };
    int data[3];
};

class PermissionGrid
{
public:
	PermissionGrid(const xbsVec3 &minPt, const xbsVec3 &maxPt);
	~PermissionGrid();
    void createGrid (xbsReal error=0.10f, xbsReal precision=2.0f);
	void insertTriangle(const xbsTriangle * t);
    bool triangleIsValid(const xbsTriangle * t);
    void dumpToOutfile(const char * file);

private:
	GridDataType * grid;
    int gridSize;
    xbsVec3 minGrid, maxGrid, voxelSize;
    int3 numDivs;
    xbsReal alpha;

    void voxelize(xbsVec3 v0, xbsVec3 v1, xbsVec3 v2, const xbsReal &tolerance);
    // triangle voxelization helper routine
    void getPlane (const char vecNum, 
        const xbsVec3 &v0, const xbsVec3 &v1, const xbsVec3 &v2,
        xbsVec3 bbmin, xbsVec3 bbmax, xbsVec3 &step, xbsReal &dist);

    bool triangleIntersectsBox(xbsVec3 &triNorm, xbsReal & d, const xbsVec3 & b1);
	
    int determineGridID(const xbsVec3 &v);
	int3 determineGridID3(const xbsVec3 &v);
    
    int bits_per_data;

    void turnOnGridCell(int id);
    void turnOffGridCell(int id);
    bool gridCellOn(int id);
    bool gridCellOff(int id);
    
};

/* Protection from multiple includes. */
#endif // INCLUDED_PERMISSION_GRID_H
