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



/*----------------------------- Local Includes -----------------------------*/
#include <PermissionGrid.h>

/*----------------------------- Local Constants -----------------------------*/
static const xbsReal FPthreshold = (xbsReal)0.0001;

/*------------------------------ Local Macros -------------------------------*/

/*------------------------------- Local Types -------------------------------*/

/*------------------------------ Local Globals ------------------------------*/

/*------------------------ Local Function Prototypes ------------------------*/

/*---------------------------------Functions-------------------------------- */

/*****************************************************************************\
 @ PermissionGrid::PermissionGrid
 -----------------------------------------------------------------------------
 description : Constructor
 input       : Model's bounding box
 output      : Nothing
 notes       : Defaulting to one BYTE per grid cell for now
\*****************************************************************************/
PermissionGrid::PermissionGrid(const xbsVec3 &minPt, const xbsVec3 &maxPt)
: minGrid(minPt), maxGrid(maxPt), grid(NULL), gridSize(0), bits_per_data(8*sizeof(GridDataType))
{
    //bits_per_data = 1;
    //createGrid();
};

/*****************************************************************************\
 @ PermissionGrid::~PermissionGrid
 -----------------------------------------------------------------------------
 description : Destructor
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
PermissionGrid::~PermissionGrid()
{
    if (grid != NULL) delete[] grid;
}

/*****************************************************************************\
 @ pgEquals
 -----------------------------------------------------------------------------
 description : Check to see if two numbers are within a tolerance
 input       : Two numbers to be compared
 output      : Bool, indicating if the numbers are within tolerance
 notes       : To avoid floating point errors
\*****************************************************************************/
static inline bool pgEquals(const xbsReal &a, const xbsReal &b)
{
    return (fabs(a-b) < FPthreshold);
}

/*****************************************************************************\
 @ PermissionGrid::createGrid
 -----------------------------------------------------------------------------
 description : Generate the grid based on error and precision
 input       : Error, precision
 output      : None, just creates the grid
 notes       :
\*****************************************************************************/
void PermissionGrid::createGrid (xbsReal error, xbsReal precision)
{
    // bounding box for entire model
    xbsVec3 bbox = maxGrid-minGrid;
    // calculate alpha and adjust to correspond to bbox
    alpha = error - error/precision * sqrt(3.0f) * 0.5f;
    alpha *= bbox.length();
    alpha = fabs(alpha);// let's hope this never makes a difference

    if (DEBUG_PERMISSION_GRID)
    {
        fprintf (stderr, "\n\tCreating Permission Grid with:\n");
        fprintf (stderr, "\tmin= %f %f %f\n\tmax= %f %f %f\n",
            minGrid[0], minGrid[1], minGrid[2], 
            maxGrid[0], maxGrid[1], maxGrid[2]);
        fprintf (stderr, "\terror= %f\n\tprecision= %f\n\talpha= %f\n",
            error, precision, alpha);
    }
    // l from pg paper
    xbsReal l = bbox.length() * error/precision;
    // slightly changed from paper 
    voxelSize = xbsVec3(l,l,l); 

    maxGrid += voxelSize;
    minGrid -= voxelSize;
    bbox = maxGrid - minGrid;

    // R from pg paper
    numDivs = bbox/voxelSize;
    // account for the remainder of the above division
    numDivs.increment();
    // this is the final total number of grid cells
    gridSize = (int)(numDivs[0] * numDivs[1] * numDivs[2]);
    // adjust so voxel dimensions are exact multiples of the entire bounding box
    // if only one cell wide, then revert to l
    voxelSize[0] = (numDivs[0]>1)?(maxGrid[0]-minGrid[0])/numDivs[0] : l;
    voxelSize[1] = (numDivs[1]>1)?(maxGrid[1]-minGrid[1])/numDivs[1] : l;
    voxelSize[2] = (numDivs[2]>1)?(maxGrid[2]-minGrid[2])/numDivs[2] : l;

    if (DEBUG_PERMISSION_GRID)
    {
        fprintf (stderr, "\n\tPermission Grid Dimensions: %i x %i x %i (%i)\n", 
            (int)numDivs[0], (int)numDivs[1], (int)numDivs[2], gridSize);
        fprintf (stderr, "\tVoxel size = (%f,%f,%f)\n", voxelSize[0], voxelSize[1], voxelSize[2]);
        fprintf (stderr, "\tMemory used: %i bytes\n", gridSize / bits_per_data + 1);
    }

    if (grid != NULL)
        delete[]grid;
    grid = new GridDataType[gridSize / bits_per_data +1];
    memset(grid, 0, gridSize*sizeof(GridDataType) / bits_per_data + 1);

}

/*****************************************************************************\
 @ PermissionGrid::dumpToOutfile
 -----------------------------------------------------------------------------
 description : Debugging tool for pgvis
 input       : 
 output      : Output written ONLY if in DEBUG_PERMISSION_GRID mode
 notes       : Need to rewrite if not using BYTES for cells
\*****************************************************************************/
void PermissionGrid::dumpToOutfile( const char * filename )
{
    if (DEBUG_PERMISSION_GRID)
    {
        FILE * output = fopen(filename, "wb");
        fwrite(&bits_per_data, sizeof(int), 1, output);
        fwrite(numDivs.data, sizeof(int), 3, output);
        fwrite(grid, sizeof(GridDataType), gridSize/bits_per_data+1, output);
        fclose(output);
    }
}

/*****************************************************************************\
 @ PermissionGrid::determineGridID3
 -----------------------------------------------------------------------------
 description : Calculate the 3-D grid ID
 input       : Point in three-space
 output      : That point's ID in x,y and z
 notes       :
\*****************************************************************************/
int3 PermissionGrid::determineGridID3(const xbsVec3 &v)
{
    int i;
    int3 idvec;
    xbsVec3 temp = v-minGrid;
    // if a vertex is near a grid cell boundary, bump it up to the next cell up
    for (i=0; i<3; ++i)
        if (pgEquals(voxelSize[i],fmod(temp[i],voxelSize[i])) && temp[i] < maxGrid[i]-voxelSize[i]/2)
            temp[i] += voxelSize[i]/2;
    idvec.set(temp / voxelSize);
    return idvec;
};

/*****************************************************************************\
 @ PermissionGrid::determineGridID
 -----------------------------------------------------------------------------
 description : Calculate the 1-D grid ID
 input       : Point in three-space
 output      : That point's ID in the 1-D grid array
 notes       :
\*****************************************************************************/
int PermissionGrid::determineGridID(const xbsVec3 &v)
{
    int id = -1;
    int3 idvec = determineGridID3(v);
    id = idvec[0] + idvec[1]*numDivs[0] + idvec[2]*numDivs[0]*numDivs[1];
    // shouldn't ever happen
    if (id > gridSize)
    {
        fprintf (stderr, "ERROR determining grid ID, id=%i, gridSize=%i\n", id, gridSize);
        return -1;
    }

    return id;
};

// how is this not already defined somewhere?
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

/*****************************************************************************\
 @ PermissionGrid::getPlane
 -----------------------------------------------------------------------------
 description : Generate plane equation and step size
 input       : Triangle and vector number (which is actually a char)
 output      : Step sizes and distance to min bbox point
 notes       : See Incremental Voxelization paper for more details
\*****************************************************************************/
void PermissionGrid::getPlane (const char vecNum, 
                               const xbsVec3 &v1, const xbsVec3 &v2, const xbsVec3 &v3,
                               xbsVec3 bbmin, xbsVec3 bbmax, xbsVec3 &step, xbsReal &dist)
{
    xbsVec3 normal, plane, bbmindiff;
    xbsReal D;
    switch (vecNum)
    {
    case 'a':
        normal = ((v3 - v1).cross(v2 - v1)).normalize();
        D = -normal.dot(v1);
        break;
    case 'b':
        normal = (v3-v1);
        normal /= normal.SquaredLength();
        D = -normal.dot(v1);
        break;
    case 'c':
        normal = (v2-v3);
        normal /= normal.SquaredLength();
        D = -normal.dot(v3);
        break;
    case 'd':
        normal = (v1-v2);
        normal /= normal.SquaredLength();
        D = -normal.dot(v2);
        break;
    case 'e':
        plane = ((v3 - v1).cross(v2 - v1));
        normal = (v1-v3).cross(plane).normalize();
        D = -normal.dot(v1);
        break;
    case 'f':
        plane = ((v3 - v1).cross(v2 - v1));
        normal = (v3-v2).cross(plane).normalize();
        D = -normal.dot(v2);
        break;
    case 'g':
        plane = ((v3 - v1).cross(v2 - v1));
        normal = (v2-v1).cross(plane).normalize();
        D = -normal.dot(v1);
        break;
    default :
        normal = xbsVec3(0,0,0);
        D = 0;
    };
    dist = normal.dot(bbmin) + D;
    xbsVec3 bb = bbmax - bbmin;
    int3 gridDiff = determineGridID3(bbmax) - determineGridID3(bbmin);
    gridDiff.increment();
    // incremental voxelization paper was wrong!
    //step = xbsVec3 (
    //    bb[0] / gridDiff[0] *normal[0], 
    //    bb[1] / gridDiff[1] *normal[1] - normal[0]*(bb[0]), 
    //    bb[2] / gridDiff[2] *normal[2] - normal[1]*(bb[1]) - normal[0]*(bb[0]));
    step = xbsVec3 (
        bb[0] / gridDiff[0] * normal[0], 
        bb[1] / gridDiff[1] * normal[1] - normal[0]*(bb[0]), 
        bb[2] / gridDiff[2] * normal[2] - normal[1]*(bb[1]));
}

/*****************************************************************************\
 @ PermissionGrid::voxelize
 -----------------------------------------------------------------------------
 description : Voxelize a triangle based on tolerance
 input       : Triangle's three vertices, tolerance
 output      : None, but grid is adjusted to account for this triangle
 notes       : Incremental algorithm for computing distance to a plane
               from Dachille and Kaufman (2000?), "Incremental Triangle Voxelization"
\*****************************************************************************/
void PermissionGrid::voxelize(xbsVec3 v0, xbsVec3 v1, xbsVec3 v2, 
                              const xbsReal &tolerance)
{
    xbsReal distA, distB, distC, distD, distE, distF, distG;
    xbsVec3 stepA, stepB, stepC, stepD, stepE, stepF, stepG;
    // determine triangle bounding box
    xbsVec3 bbmin, bbmax;
    bbmin[0] = min(min(v0[0], v1[0]), v2[0]);
    bbmin[1] = min(min(v0[1], v1[1]), v2[1]);
    bbmin[2] = min(min(v0[2], v1[2]), v2[2]);
    bbmax[0] = max(max(v0[0], v1[0]), v2[0]);
    bbmax[1] = max(max(v0[1], v1[1]), v2[1]);
    bbmax[2] = max(max(v0[2], v1[2]), v2[2]);
    // XXX expand bounding box - uncomment and fix errors when get time
    // problems arise with cells on boundaries currently
    //bbmin -= xbsVec3(tolerance, tolerance, tolerance);
    //bbmax += xbsVec3(tolerance, tolerance, tolerance);

    // calculate plane eqns for 7 planes defined by these three points
    getPlane ('a', v0, v1, v2, bbmin, bbmax, stepA, distA);
    getPlane ('b', v0, v1, v2, bbmin, bbmax, stepB, distB);
    getPlane ('c', v0, v1, v2, bbmin, bbmax, stepC, distC);
    getPlane ('d', v0, v1, v2, bbmin, bbmax, stepD, distD);
    getPlane ('e', v0, v1, v2, bbmin, bbmax, stepE, distE);
    getPlane ('f', v0, v1, v2, bbmin, bbmax, stepF, distF);
    getPlane ('g', v0, v1, v2, bbmin, bbmax, stepG, distG);

    int3 voxel;
    xbsVec3 xyz;
    // some squared numbers to keep track of
    xbsReal dist2;
    xbsReal tolerance2 = tolerance * tolerance;
    int3 bbminID = determineGridID3(bbmin);
    int3 bbmaxID = determineGridID3(bbmax);

    // see incremental voxelization paper
    for(xyz[2]=bbmin[2], voxel[2]=bbminID[2];
        voxel[2] <= bbmaxID[2];
        voxel[2]++, xyz[2]+=voxelSize[2])
    {
        for(xyz[1]=bbmin[1], voxel[1]=bbminID[1];
            voxel[1] <= bbmaxID[1];
            voxel[1]++, xyz[1]+=voxelSize[1])
        {
            for(xyz[0]=bbmin[0], voxel[0]=bbminID[0];
                voxel[0] <= bbmaxID[0];
                voxel[0]++, xyz[0]+=voxelSize[0])
            {
                dist2 = MAXFLOAT;
                if (fabs(distA) <= tolerance)
                {
                    if (distE >= 0 && distF >= 0 && distG >= 0)      // region 1
                        dist2 = 0;
                    else if (distD>=0 && distD<=1 && distG<=0 && distG>=(-tolerance))        // region 2
                        dist2 = distA*distA+distG*distG;
                    else if (distC>=0 && distC<=1 && distF<=0 && distF>=(-tolerance))        // region 3
                        dist2 = distA*distA+distF*distF;
                    else if (distB>=0 && distB<=1 && distE<=0 && distE>=(-tolerance))        // region 4
                        dist2 = distA*distA+distE*distE;
                    else if (distB<=0 && distB>=(-tolerance) && distD>=1 && distD<=(1+tolerance)) // region 5
                        dist2 = (v0-xyz).SquaredLength();
                    else if (distD<=0 && distD>=(-tolerance) && distC>=1 && distC<=(1+tolerance)) // region 6
                        dist2 = (v1-xyz).SquaredLength();
                    else if (distC<=0 && distC>=(-tolerance) && distB>=1 && distB<=(1+tolerance)) // region 7
                        dist2 = (v2-xyz).SquaredLength();
                    if (dist2 <= tolerance2)
                        //turnOnGridCell(determineGridID(xyz));
                        turnOnGridCell(voxel[0] + voxel[1]*numDivs[0] + voxel[2]*numDivs[0]*numDivs[1]);
                }
                distA += stepA[0];
                distB += stepB[0];
                distC += stepC[0];
                distD += stepD[0];
                distE += stepE[0];
                distF += stepF[0];
                distG += stepG[0];
                // snap to 0 or 1 to avoid floating point error creap
                if (pgEquals(distA, 0)) distA = 0; if (pgEquals(distA, 1)) distA = 1;
                if (pgEquals(distB, 0)) distB = 0; if (pgEquals(distB, 1)) distB = 1;
                if (pgEquals(distC, 0)) distC = 0; if (pgEquals(distC, 1)) distC = 1;
                if (pgEquals(distD, 0)) distD = 0; if (pgEquals(distD, 1)) distD = 1;
                if (pgEquals(distE, 0)) distE = 0; if (pgEquals(distE, 1)) distE = 1;
                if (pgEquals(distF, 0)) distF = 0; if (pgEquals(distF, 1)) distF = 1;
                if (pgEquals(distG, 0)) distG = 0; if (pgEquals(distG, 1)) distG = 1;
            }
            distA += stepA[1];
            distB += stepB[1];
            distC += stepC[1];
            distD += stepD[1];
            distE += stepE[1];
            distF += stepF[1];
            distG += stepG[1];
        }
        distA += stepA[2];
        distB += stepB[2];
        distC += stepC[2];
        distD += stepD[2];
        distE += stepE[2];
        distF += stepF[2];
        distG += stepG[2];
    }
}

/*****************************************************************************\
 @ PermissionGrid::insertTriangle
 -----------------------------------------------------------------------------
 description : Insert a triangle into the grid
 input       : Triangle
 output      : Nothing
 notes       : just a wrapper for voxelize
\*****************************************************************************/
void PermissionGrid::insertTriangle(const xbsTriangle * t)
{
    voxelize(t->verts[0]->coord, t->verts[1]->coord, t->verts[2]->coord, alpha);
};

/*****************************************************************************\
 @ PermissionGrid::triangleIntersectsBox
 -----------------------------------------------------------------------------
 description : Tests if a triangle intersects a box
 input       : b1 is the min point of the bounding box, and that box's 
               dimensions are defined by voxelSize
 output      : 
 notes       : Just checks to make sure all the signs are the same
\*****************************************************************************/
bool PermissionGrid::triangleIntersectsBox(xbsVec3 &triNorm, xbsReal & d, const xbsVec3 & b1)
{
    xbsVec3 b2, b3, b4, b5, b6, b7, b8;
    xbsReal sign;

    b2 = b1 + xbsVec3(voxelSize[0],0,0);
    b3 = b1 + xbsVec3(0,voxelSize[1],0);
    b4 = b1 + xbsVec3(voxelSize[0],voxelSize[1],0);
    b5 = b1 + xbsVec3(0,0,voxelSize[2]);
    b6 = b2 + xbsVec3(0,0,voxelSize[2]);
    b7 = b3 + xbsVec3(0,0,voxelSize[2]);
    b8 = b4 + xbsVec3(0,0,voxelSize[2]);

    sign = triNorm.dot(b1) + d;
    
    // is there a better/faster way to do this?
    if (sign > 0)
    {
        if ((triNorm.dot(b2)+d < 0) || (triNorm.dot(b3)+d < 0) ||
            (triNorm.dot(b4)+d < 0) || (triNorm.dot(b5)+d < 0) ||
            (triNorm.dot(b6)+d < 0) || (triNorm.dot(b7)+d < 0) ||
            (triNorm.dot(b8)+d < 0) )
            return true;
    }
    else
    {
        if ((triNorm.dot(b2)+d > 0) || (triNorm.dot(b3)+d > 0) ||
            (triNorm.dot(b4)+d > 0) || (triNorm.dot(b5)+d > 0) ||
            (triNorm.dot(b6)+d > 0) || (triNorm.dot(b7)+d > 0) ||
            (triNorm.dot(b8)+d > 0) )
            return true;
    }
    return false;
}

/*****************************************************************************\
 @ PermissionGrid::triangleIsValid
 -----------------------------------------------------------------------------
 description : Check if the generated triangle is valid against the grid
 input       : Triangle to be tested
 output      : Bool
 notes       : For all voxels the triangle's plane intersects, projects 
               voxel "center" P onto triangle's plane and checks if 
               projected point lands in triangle.  If it does, then queries 
               the grid
\*****************************************************************************/
bool PermissionGrid::triangleIsValid(const xbsTriangle * t)
{
    xbsReal u,v,w;                          // barycentric coordinates
    xbsReal DistToPlane;                    // distance from point P to plane of triangle
    xbsVec3 bbmin, bbmax, xyz, center, P;

    // rename stuff for ease
    xbsVec3 A = t->verts[0]->coord;
    xbsVec3 B = t->verts[1]->coord;
    xbsVec3 C = t->verts[2]->coord;
    xbsVec3 AB = B-A;
    xbsVec3 BC = C-B;
    xbsVec3 CA = A-C;

    // define plane equation
    xbsVec3 tnorm = (AB.cross(C-A)).normalize();
    xbsReal d = -tnorm.dot(A);

    // calculate now for reuse later
    // this is actually 1/(2*area) but we just need it to be proportional to area
    //xbsReal tAreaInv = 1 / AB.cross(AC).length();
    xbsReal barySumInv;

    // determine triangle bounding box
    bbmin[0] = min(min(A[0], B[0]), C[0]);
    bbmin[1] = min(min(A[1], B[1]), C[1]);
    bbmin[2] = min(min(A[2], B[2]), C[2]);
    bbmax[0] = max(max(A[0], B[0]), C[0]);
    bbmax[1] = max(max(A[1], B[1]), C[1]);
    bbmax[2] = max(max(A[2], B[2]), C[2]);

    for (xyz[2]=bbmin[2]; xyz[2]<=bbmax[2]; xyz[2]+=voxelSize[2])
    {
        for (xyz[1]=bbmin[1]; xyz[1]<=bbmax[1]; xyz[1]+=voxelSize[1])
        {
            for (xyz[0]=bbmin[0]; xyz[0]<=bbmax[0]; xyz[0]+=voxelSize[0])
            {
                if (triangleIntersectsBox(tnorm, d, xyz))
                {
                    // project voxel center onto plane of triangle
                    // XXX this really isn't the voxel center though
                    // fix this in the future... need to decide what point to project
                    center = xyz+voxelSize*0.5;
                    DistToPlane = (tnorm.dot(center) + d);
                    P = center - tnorm*DistToPlane;

                    // compute barycentric coordinates of projected point P
                    u = BC.cross(P-B).dot(tnorm);
                    v = CA.cross(P-C).dot(tnorm);
                    w = AB.cross(P-A).dot(tnorm);
                    barySumInv = 1.0 / (u + v + w);
                    u *= barySumInv;
                    v *= barySumInv;
                    w *= barySumInv;

                    // if P does not lie in triangle, then ignore this voxel
                    if ( u<0 || u>1 || v<0 )
                        continue;

                    //if (gridCellOff(determineGridID(xyz)) && triangleIntersectsBox(tnorm, d, xyz))
                    if (gridCellOff(determineGridID(xyz)))
                        return false;
                }
            }
        }
    }

    return true;

    //project the triangle onto a plane then rasterize that triangle into the the lost dimension
    // paper computes it this way but i don't think it's correct
};


void PermissionGrid::turnOnGridCell(int id)  
{ 
    grid [id / bits_per_data] |= pgone >> id % bits_per_data; 
};
void PermissionGrid::turnOffGridCell(int id)  
{ 
    grid [id / bits_per_data] &= !(pgone >> id % bits_per_data); 
};
bool PermissionGrid::gridCellOn(int id)
{ 
    return (grid [id / bits_per_data] & (pgone >> id % bits_per_data)) != 0; 
};
bool PermissionGrid::gridCellOff(int id)
{ 
    return (grid [id / bits_per_data] & (pgone >> id % bits_per_data)) == 0; 
};
    
