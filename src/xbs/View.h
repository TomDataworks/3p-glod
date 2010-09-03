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

#ifndef _VIEW_H
#define _VIEW_H

class GLOD_View
{
    public:
        Mat4 matrix;
        xbsVec3 eye;
        xbsVec3 forward;
        xbsVec3 up;
        xbsReal yFOV; // degrees
        xbsReal aspect;  // w / h
        xbsReal tanFOVby2;
        
        //    float     xPixels;
    
        GLOD_View()
        {
            eye.set(0,0,0);
            forward.set(0,0,-1);
            up.set(0,1,0);
            yFOV = 45.0f; aspect = 4.0f/3.0f;
            //      xPixels = 640;
            tanFOVby2 = tan((yFOV/2.0) * (M_PI/180.0));
        }
    
        void SetFrom(float m1[16], float m2[16], float m3[16]);
        //xbsReal computePixelsOfError(xbsVec3 center, xbsReal objectSpaceError);
        xbsReal computePixelsOfError(xbsVec3 center, xbsVec3 offsets, xbsReal objectSpaceError, int area=-1);
        xbsReal checkFrustrum(xbsVec3 center, xbsVec3 offsets, int area=-1);
};


#endif /* _VIEW_H */
