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
#include "xbs.h"

#include "glod_glext.h"

#include "Continuous.h"
#include "vds_callbacks.h"

void
VDSHierarchy::initialize(Model *model)
{
    vif = new Vif;
    quadricMultiplier = 1;
    vif->NumPatches = model->getNumPatches();
    
    char hasColor, hasNormal, hasTexcoord;
    model->hasAttributes(hasColor, hasNormal, hasTexcoord);

    if (hasColor)
        vif->ColorsPresent = true;
    
    if (hasNormal)
        vif->NormalsPresent = true;
    
    if (hasTexcoord)
        vif->NumTextures = 1;

    // add Vertex Positions for input vertices
    VDS::Point3 coord;
    VDS::ByteColorA color;
    VDS::Vec3 normal;
    VDS::Point2 *texcoord = new VDS::Point2[1];
    for (int vnum=0; vnum<model->getNumVerts(); vnum++)
    {
        xbsVertex *vert = model->getVert(vnum);
        vert->fillVDSData(coord, color, normal, texcoord[0]);
        vert->mtIndex = vif->addVertPos(coord, color, normal, texcoord);
        if (hasTexcoord)
            texcoord = new VDS::Point2[1];
    }

    // note: we don't delete any texcoord actually used by VIF
    delete [] texcoord;
    
    // add vertices
    for (int vnum=0; vnum<model->getNumVerts(); vnum++)
    {
        xbsVertex *vert = model->getVert(vnum);
        // I think VIF patches go from 1..NumPatches!
        vert->mtIndex =
            vif->addVert(vert->mtIndex,
                         (VDS::PatchIndex)vert->tris[0]->patchNum + 1,
                         false, 0);
    }

    // fix up vertex coincident info
    for (int vnum=0; vnum<model->getNumVerts(); vnum++)
    {
        xbsVertex *vert = model->getVert(vnum);
        if (vert->nextCoincident == vert)
            continue;
        vif->Vertices[vert->mtIndex].CoincidentVertexFlag = true;
        vif->Vertices[vert->mtIndex].CoincidentVertex =
            vert->nextCoincident->mtIndex;
    }
    
             
    // add triangles
    for (int tnum=0; tnum<model->getNumTris(); tnum++)
    {
        xbsTriangle *tri = model->getTri(tnum);
        // I think VIF patches go from 1..NumPatches!
        vif->addTri(tri->verts[0]->mtIndex,
                    tri->verts[1]->mtIndex,
                    tri->verts[2]->mtIndex,
                    tri->patchNum + 1);
    }

    return;
}


/*****************************************************************************\
 @ VDSHierarchy::update
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
VDSHierarchy::update(Model *model, Operation *op,
                     xbsVertex **sourceMappings,
                     xbsTriangle **changedTris, int numChangedTris,
                     xbsTriangle **destroyedTris, int numDestroyedTris)
{

    // The basic operation is to create a merge for each mapped
    // source-destination pair. But there are some qualifications. If the
    // destination is new, we have to create the VertexPosition first and
    // make its first vertex, rather than cloning an existing vertex. Also,
    // if the source-destination pair results in a vertex with no
    // triangles, we can merge it in with a destination that will have
    // triangles. Also, if there are any other vertices from the source or
    // destination that will result in no triangles, we can merge them in
    // with a non-empty destination vertex. It seems like we should try to
    // find all the empty vertices at the start so we can merge them all in
    // with the first non-empty destination. What if there is no non-empty
    // destination?

    
    xbsVertex *source = op->getSource();
    xbsVertex *destination = op->getDestination();
    
    int numSource = source->numCoincident();
    int numDestination = destination->numCoincident();

    // Invert the given mappings -- compute for each destination vertex
    // which of the source vertices map to it (it could be more than 1).
    // Also note which of the source vertices have a NULL mapping (because
    // they are already a triangle with 0 vertices
    xbsVertex ***destinationMappings = new xbsVertex **[numDestination];
    int *numDestinationMappings = new int[numDestination];
    for (int i=0; i<numDestination; i++)
    {
        destinationMappings[i] = NULL;
        numDestinationMappings[i] = 0;
    }
    xbsVertex **nullMappings = new xbsVertex *[numSource];
    int numNullMappings = 0;

    xbsVertex *currentSource = source;
    for (int i=0; i<numSource;
         i++, currentSource = currentSource->nextCoincident)
    {
        xbsVertex *vert = sourceMappings[i];
        if (vert == NULL)
        {
            if (currentSource->numTris > 0)
            {
                fprintf(stderr, "Vert with NULL mappings still has tris!\n");
                exit(1);
            }
            nullMappings[numNullMappings++] = currentSource;
            continue;
        }
        
        int destIndex = vert->coincidentIndex();
        xbsVertex **mappings = new xbsVertex *[numDestinationMappings[destIndex]+1];
        for (int j=0; j<numDestinationMappings[destIndex]; j++)
            mappings[j] = destinationMappings[destIndex][j];
        mappings[numDestinationMappings[destIndex]++] = currentSource;
        delete [] destinationMappings[destIndex];
        destinationMappings[destIndex] = mappings;
    }

    
    // Compute how many triangles will be on each destination vertex after
    // removing the destoyed triangles and re-assigning all of the changed
    // tris from the source to destination

    int *triCounts = new int[numDestination];
    xbsVertex *currentDestination = destination;
    for (int i=0;  i < numDestination;
         i++, currentDestination = currentDestination->nextCoincident)
        triCounts[i] = currentDestination->numTris;

    // Reduce tri count for each destroyed tri
    for (int i=0; i<numDestroyedTris; i++)
    {
        xbsTriangle *tri = destroyedTris[i];
        for (int vnum=0; vnum<3; vnum++)
        {
            xbsVertex *vert = tri->verts[vnum];
            if (vert->minCoincident() != destination)
                continue;
            triCounts[vert->coincidentIndex()]--;
        }
    }
    
    // Increase tri count for each changed tri (that got moved from a
    // source vert to a destination vert)
    for (int i=0; i<numChangedTris; i++)
    {
        xbsTriangle *tri = changedTris[i];
        for (int vnum=0; vnum<3; vnum++)
        {
            xbsVertex *vert = tri->verts[vnum];
            if (vert->minCoincident() != source)
                continue;
            xbsVertex *destvert = sourceMappings[vert->coincidentIndex()];
            if (destvert == NULL)
            {
                fprintf(stderr, "Vertex on changed tri has NULL mapping!\n");
                exit(1);
            }
            triCounts[destvert->coincidentIndex()]++;
        }
    }

    // Count number of empty and non-empty destination verts
    int numEmptyDestinations = 0;
    int numNonEmptyDestinations = 0;
    int numEmptySources = numNullMappings;
    for (int i=0; i<numDestination; i++)
    {
        if (triCounts[i] == 0)
        {
            numEmptyDestinations++;
            numEmptySources += numDestinationMappings[i];
        }
        else
            numNonEmptyDestinations++;
    }



    //
    // Make the merges
    //
    
    int *parents =
        new int[source->numCoincident() + destination->numCoincident()];
    int numParents = 0;

    // We will actually have some parent vertices with triangles
    int firstMerge = 1;
    xbsVertex *destVert = destination;
    for (int i=0; i<numDestination;
         i++, destVert = destVert->nextCoincident)
    {
        if (((triCounts[i] == 0) && (numNonEmptyDestinations > 0)) ||
            ((numNonEmptyDestinations == 0) &&
             ((firstMerge == 0) || destVert->mtIndex == -1)))
            continue;
        
        VifMerge merge;
        
        
        // Make or clone the new parent
        if (destVert->mtIndex == -1)
        {
            // New vertex was created by this half edge collapse (due to
            // multi-attribute vertex handling, etc.
            
            char hasColor, hasNormal, hasTexcoord;
            model->hasAttributes(hasColor, hasNormal, hasTexcoord);
            
            VDS::Point3 coord;
            VDS::ByteColorA color;
            VDS::Vec3 normal;
            VDS::Point2 *texcoord = new VDS::Point2[1];
            
            destVert->fillVDSData(coord, color, normal, texcoord[0]);
            int vertPos = vif->addVertPos(coord, color, normal, texcoord);
            if (!hasTexcoord)
                delete texcoord;
            
            // I think VIF patches go from 1..NumPatches!
            merge.ParentNode =
                vif->addVert(vertPos,
                             destinationMappings[destVert->coincidentIndex()][0]->tris[0]->patchNum + 1,
                             false, 0);
        }
        else
        {
            merge.ParentNode =
                vif->addVert(vif->Vertices[destVert->mtIndex].VertexPosition,
                             vif->Vertices[destVert->mtIndex].PatchID,
                             false, 0);
        }
        
        
        parents[numParents++] = merge.ParentNode;
        
        
        //
        // Count children vertices of merge
        //

        merge.NumNodesInMerge = 0;
        // Source mappings to this destination
        if (triCounts[i] > 0)
            merge.NumNodesInMerge += numDestinationMappings[i];
        // Original destination vert
        if ((destVert->mtIndex != -1) && (triCounts[i] > 0))
            merge.NumNodesInMerge++;
        // Include all empty vertices in the first non-empty merge
        if (firstMerge == 1)
            merge.NumNodesInMerge += numEmptySources + numEmptyDestinations;
        
        // allocate
        merge.NodesBeingMerged = new unsigned int[merge.NumNodesInMerge];
        merge.NumNodesInMerge = 0;
        
        //
        // Fill in vertices of the merge
        //

        if (triCounts[i] > 0)
        {
            // mapped source vertices
            for (int snum=0; snum<numDestinationMappings[i]; snum++)
            {
                if (destinationMappings[i][snum]->mtIndex == -1)
                {
                    fprintf(stderr, "oops -- mtIndex == -1\n");
                }
                else
                {
                    merge.NodesBeingMerged[merge.NumNodesInMerge++] =
                        destinationMappings[i][snum]->mtIndex;
                    destinationMappings[i][snum]->mtIndex = -1;
                }
            }
            // the destination vertex itself
            if (destVert->mtIndex != -1)
            {
                merge.NodesBeingMerged[merge.NumNodesInMerge++] =
                    destVert->mtIndex;
            }
            destVert->mtIndex = merge.ParentNode;
        }
         
        // empty vertices
        if (firstMerge == 1)
        {
            // null mappings
            for (int nullNum = 0; nullNum < numNullMappings; nullNum++)
            {
                if (nullMappings[nullNum]->mtIndex == -1)
                {
                    fprintf(stderr, "oops2 -- mtIndex == -1\n");
                }
                else
                {
                    merge.NodesBeingMerged[merge.NumNodesInMerge++] =
                        nullMappings[nullNum]->mtIndex;
                    nullMappings[nullNum]->mtIndex = -1;
                }
            }
            
            for (int destNum = 0; destNum < numDestination; destNum++)
            {
                if (triCounts[destNum] > 0)
                    continue;
                for (int snum=0; snum<numDestinationMappings[destNum]; snum++)
                {
                    if (destinationMappings[destNum][snum]->mtIndex == -1)
                    {
                        fprintf(stderr, "oops3 -- mtIndex == -1\n");
                    }
                    else
                    {
                        merge.NodesBeingMerged[merge.NumNodesInMerge++] =
                            destinationMappings[destNum][snum]->mtIndex;
                        destinationMappings[destNum][snum]->mtIndex = -1;
                    }
                }
                xbsVertex *dv = destination->coincidentVert(destNum);
                if (dv->mtIndex == -1)
                {
                    fprintf(stderr, "oops4 -- mtIndex == -1\n");
                }
                else
                {
                    merge.NodesBeingMerged[merge.NumNodesInMerge++] =
                        dv->mtIndex;
                    dv->mtIndex = -1;
                }
            }
        }
        
        vif->addMerge(merge);
        
        
        firstMerge = 0;
    }
    

    
    if (numNonEmptyDestinations == 0)
    {
        // preserve this vertex to be merged in the final merge during
        // VDSHierarchy::finalize()

        if (numDanglingVerts == maxDanglingVerts)
        {
            int *newDanglingVerts = new int[maxDanglingVerts*2];
            for (int i=0; i<numDanglingVerts; i++)
                newDanglingVerts[i] = danglingVerts[i];
            delete [] danglingVerts;
            danglingVerts = newDanglingVerts;
            maxDanglingVerts *= 2;
        }
        danglingVerts[numDanglingVerts++] = parents[0];
    }
    
    

    //
    // Fix up the coincident ring
    //

    if (numParents > 1)
    {
        for (int i=0; i<numParents; i++)
        {
            vif->Vertices[parents[i]].CoincidentVertexFlag = true;
            vif->Vertices[parents[i]].CoincidentVertex =
                parents[(i+1)%numParents];
        }
    }
    delete [] parents;
    parents = NULL;
    numParents = 0;


    // clean up space
    delete [] nullMappings;
    nullMappings = NULL;
    numNullMappings = 0;

    delete [] triCounts;
    triCounts = NULL;
    
    for (int i=0; i<numDestination; i++)
    {
        delete [] destinationMappings[i];
        destinationMappings[i] = NULL;
        numDestinationMappings[i] = 0;
    }
    delete [] numDestinationMappings;
    numDestinationMappings = NULL;
    delete [] destinationMappings;
    destinationMappings = NULL;

    return;
} /** End of VDSHierarchy::update() **/

/*****************************************************************************\
 @ VDSHierarchy::update
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
VDSHierarchy::update(Model *model, EdgeCollapse *op,
                     xbsVertex **sourceMappings, xbsVertex **destMappings,
                     xbsTriangle **changedTris, int numChangedTris,
                     xbsTriangle **destroyedTris, int numDestroyedTris,
                     xbsVertex *generated_vert)
{
    // similar to half edge collapse, but need to generate new VDS vertices
    // for the generated_vert, etc.
    

    // The basic operation is to create a merge for each mapped
    // source-destination pair. But there are some qualifications. If the
    // destination is new, we have to create the VertexPosition first and
    // make its first vertex, rather than cloning an existing vertex. Also,
    // if the source-destination pair results in a vertex with no
    // triangles, we can merge it in with a destination that will have
    // triangles. Also, if there are any other vertices from the source or
    // destination that will result in no triangles, we can merge them in
    // with a non-empty destination vertex. It seems like we should try to
    // find all the empty vertices at the start so we can merge them all in
    // with the first non-empty destination. What if there is no non-empty
    // destination?

    
    xbsVertex *source = op->getSource();
    xbsVertex *destination = op->getDestination();
    
    int numSource = source->numCoincident();
    int numDestination = destination->numCoincident();
    int numGen = generated_vert->numCoincident();
    
    // Invert the given mappings -- compute for each generated vertex which
    // of the source vertices and destination vertices map to it (it could
    // be more than 1).  Also note which of the source and destination
    // vertices have a NULL mapping (because they are already a vertex with
    // 0 triangles
    xbsVertex ***genSourceMappings = new xbsVertex **[numGen];
    int *numGenSourceMappings = new int[numGen];
    for (int i=0; i<numGen; i++)
    {
        genSourceMappings[i] = NULL;
        numGenSourceMappings[i] = 0;
    }
    xbsVertex ***genDestMappings = new xbsVertex **[numGen];
    int *numGenDestMappings = new int[numGen];
    for (int i=0; i<numGen; i++)
    {
        genDestMappings[i] = NULL;
        numGenDestMappings[i] = 0;
    }
    
    xbsVertex **nullMappings = new xbsVertex *[numSource+numDestination];
    int numNullMappings = 0;


    xbsVertex *currentSource = source;
    for (int i=0; i<numSource;
         i++, currentSource = currentSource->nextCoincident)
    {
        xbsVertex *vert = sourceMappings[i];
        if (vert == NULL)
        {
            if (currentSource->numTris > 0)
            {
                fprintf(stderr, "Vert with NULL mappings still has tris!\n");
                exit(1);
            }
            nullMappings[numNullMappings++] = currentSource;
            continue;
        }
        
        int genIndex = vert->coincidentIndex();
        xbsVertex **mappings = new xbsVertex *[numGenSourceMappings[genIndex]+1];
        for (int j=0; j<numGenSourceMappings[genIndex]; j++)
            mappings[j] = genSourceMappings[genIndex][j];
        mappings[numGenSourceMappings[genIndex]++] = currentSource;
        delete [] genSourceMappings[genIndex];
        genSourceMappings[genIndex] = mappings;
    }

    xbsVertex *currentDest = destination;
    for (int i=0; i<numDestination;
         i++, currentDest = currentDest->nextCoincident)
    {
        xbsVertex *vert = destMappings[i];
        if (vert == NULL)
        {
            if (currentDest->numTris > 0)
            {
                fprintf(stderr, "Vert with NULL mappings still has tris!\n");
                exit(1);
            }
            nullMappings[numNullMappings++] = currentDest;
            continue;
        }
        
        int genIndex = vert->coincidentIndex();
        xbsVertex **mappings = new xbsVertex *[numGenDestMappings[genIndex]+1];
        for (int j=0; j<numGenDestMappings[genIndex]; j++)
            mappings[j] = genDestMappings[genIndex][j];
        mappings[numGenDestMappings[genIndex]++] = currentDest;
        delete [] genDestMappings[genIndex];
        genDestMappings[genIndex] = mappings;
    }


    
    // Compute how many triangles will be on each generated vertex after
    // removing the destoyed triangles and re-assigning all of the changed
    // tris from the source and destination to the generated vertex

    int *triCounts = new int[numGen];
    xbsVertex *currentGen = generated_vert;
    for (int i=0;  i < numGen;
         i++, currentGen = currentGen->nextCoincident)
    {
        if (currentGen->numTris != 0)
        {
            fprintf(stderr, "generated vertex already has tris!\n");
            exit(1);
        }
        triCounts[i] = 0;
    }
    
    // Increase tri count for each changed tri (that got moved from a
    // source vert or a destination vert to the generated vertex)
    for (int i=0; i<numChangedTris; i++)
    {
        xbsTriangle *tri = changedTris[i];
        for (int vnum=0; vnum<3; vnum++)
        {
            xbsVertex *vert = tri->verts[vnum];
            xbsVertex *min = vert->minCoincident();
            xbsVertex *gen;
            if (min == source)
                gen = sourceMappings[vert->coincidentIndex()];
            else if (min == destination)
                gen = destMappings[vert->coincidentIndex()];
            else
                continue;
            
            if (gen == NULL)
            {
                fprintf(stderr, "Vertex on changed tri has NULL mapping!\n");
                exit(1);
            }
            triCounts[gen->coincidentIndex()]++;
        }
    }

    // Count number of empty and non-empty generated verts
    int numEmptyGen = 0;
    int numNonEmptyGen = 0;
    int numEmptySources = numNullMappings;
    for (int i=0; i<numGen; i++)
    {
        if (triCounts[i] == 0)
        {
            numEmptyGen++;
            numEmptySources += numGenSourceMappings[i] + numGenDestMappings[i];
        }
        else
            numNonEmptyGen++;
    }



    //
    // Make the merges
    //
    
    int *parents =
        new int[source->numCoincident() + destination->numCoincident()];
    int numParents = 0;


    int firstMerge = 1;
    xbsVertex *genVert = generated_vert;
    for (int i=0; i<numGen;
         i++, genVert = genVert->nextCoincident)
    {
        if (((triCounts[i] == 0) && (numNonEmptyGen > 0)) ||
            ((numNonEmptyGen == 0) && (firstMerge == 0)))
            continue;
        
        VifMerge merge;
        
        
        // Make the new parent
            
        char hasColor, hasNormal, hasTexcoord;
        model->hasAttributes(hasColor, hasNormal, hasTexcoord);
        
        VDS::Point3 coord;
        VDS::ByteColorA color;
        VDS::Vec3 normal;
        VDS::Point2 *texcoord = new VDS::Point2[1];
        
        genVert->fillVDSData(coord, color, normal, texcoord[0]);
        int vertPos = vif->addVertPos(coord, color, normal, texcoord);
        if (!hasTexcoord)
            delete texcoord;
            
        // I think VIF patches go from 1..NumPatches!
        int patch;
        if (numGenSourceMappings[i] > 0)
        {
            if (genSourceMappings[i][0]->numTris <= 0)
            {
                fprintf(stderr, "Empty vert mapped to generated vert!\n");
                exit(1);
            }
            patch = genSourceMappings[i][0]->tris[0]->patchNum;
        }
        else if (numGenDestMappings[i] > 0)
        {
            if (genDestMappings[i][0]->numTris <= 0)
            {
                fprintf(stderr, "Empty dest vert mapped to generated vert!\n");
                exit(1);
            }
            patch = genDestMappings[i][0]->tris[0]->patchNum;
        }
        else
        {
            fprintf(stderr, "No vertex mapped to generated vertex!\n");
            exit(1);
        }
        
        merge.ParentNode = vif->addVert(vertPos, patch + 1, false, 0);
        
        parents[numParents++] = merge.ParentNode;

        if (triCounts[i] > 0)
            genVert->mtIndex = merge.ParentNode;
        
        
        //
        // Count children vertices of merge
        //

        merge.NumNodesInMerge = 0;
        // Source and destination to this generated vert
        if (triCounts[i] > 0)
            merge.NumNodesInMerge +=
                numGenSourceMappings[i] + numGenDestMappings[i];
        // Include all empty vertices in the first non-empty merge
        if (firstMerge == 1)
            merge.NumNodesInMerge += numEmptySources;
        
        // allocate
        merge.NodesBeingMerged = new unsigned int[merge.NumNodesInMerge];
        merge.NumNodesInMerge = 0;
        
        //
        // Fill in vertices of the merge
        //

        if (triCounts[i] > 0)
        {
            // mapped source vertices
            for (int snum=0; snum<numGenSourceMappings[i]; snum++)
            {
                if (genSourceMappings[i][snum]->mtIndex == -1)
                {
                    fprintf(stderr, "oops -- mtIndex == -1\n");
                }
                else
                {
                    merge.NodesBeingMerged[merge.NumNodesInMerge++] =
                        genSourceMappings[i][snum]->mtIndex;
                    genSourceMappings[i][snum]->mtIndex = -1;
                }
            }
            // mapped destination vertices
            for (int dnum=0; dnum<numGenDestMappings[i]; dnum++)
            {
                if (genDestMappings[i][dnum]->mtIndex == -1)
                {
                    fprintf(stderr, "oops -- mtIndex == -1\n");
                }
                else
                {
                    merge.NodesBeingMerged[merge.NumNodesInMerge++] =
                        genDestMappings[i][dnum]->mtIndex;
                    genDestMappings[i][dnum]->mtIndex = -1;
                }
            }
        }
        
        // empty vertices
        if (firstMerge == 1)
        {
            // null mappings
            for (int nullNum = 0; nullNum < numNullMappings; nullNum++)
            {
                if (nullMappings[nullNum]->mtIndex == -1)
                {
                    fprintf(stderr, "oops2 -- mtIndex == -1\n");
                }
                else
                {
                    merge.NodesBeingMerged[merge.NumNodesInMerge++] =
                        nullMappings[nullNum]->mtIndex;
                    nullMappings[nullNum]->mtIndex = -1;
                }
            }
            
            for (int genNum = 0; genNum < numGen; genNum++)
            {
                if (triCounts[genNum] > 0)
                    continue;
                
                for (int snum=0; snum<numGenSourceMappings[genNum]; snum++)
                {
                    if (genSourceMappings[genNum][snum]->mtIndex == -1)
                    {
                        fprintf(stderr, "oops3 -- mtIndex == -1\n");
                    }
                    else
                    {
                        merge.NodesBeingMerged[merge.NumNodesInMerge++] =
                            genSourceMappings[genNum][snum]->mtIndex;
                        genSourceMappings[genNum][snum]->mtIndex = -1;
                    }
                }
                for (int dnum=0; dnum<numGenDestMappings[genNum]; dnum++)
                {
                    if (genDestMappings[genNum][dnum]->mtIndex == -1)
                    {
                        fprintf(stderr, "oops3 -- mtIndex == -1\n");
                    }
                    else
                    {
                        merge.NodesBeingMerged[merge.NumNodesInMerge++] =
                            genDestMappings[genNum][dnum]->mtIndex;
                        genDestMappings[genNum][dnum]->mtIndex = -1;
                    }
                }
            }
        }
        
        vif->addMerge(merge);
        
        
        firstMerge = 0;
    }
    
    if ((numNonEmptyGen > 0) && (numParents != numNonEmptyGen))
    {
        fprintf(stderr, "Wrong number of parents generated!\n");
        exit(1);
    }
    
    if ((numNonEmptyGen == 0) && (numParents != 1))
    {
        fprintf(stderr, "No parents generated!\n");
        exit(1);
    }
    
    if (numNonEmptyGen == 0)
    {
        // preserve this vertex to be merged in the final merge during
        // VDSHierarchy::finalize()

        if (numDanglingVerts == maxDanglingVerts)
        {
            int *newDanglingVerts = new int[maxDanglingVerts*2];
            for (int i=0; i<numDanglingVerts; i++)
                newDanglingVerts[i] = danglingVerts[i];
            delete [] danglingVerts;
            danglingVerts = newDanglingVerts;
            maxDanglingVerts *= 2;
        }
        danglingVerts[numDanglingVerts++] = parents[0];
    }
    
    

    //
    // Fix up the coincident ring
    //

    if (numParents > 1)
    {
        for (int i=0; i<numParents; i++)
        {
            vif->Vertices[parents[i]].CoincidentVertexFlag = true;
            vif->Vertices[parents[i]].CoincidentVertex =
                parents[(i+1)%numParents];
        }
    }
    delete [] parents;
    parents = NULL;
    numParents = 0;


    // clean up space
    delete [] nullMappings;
    nullMappings = NULL;
    numNullMappings = 0;

    delete [] triCounts;
    triCounts = NULL;
    
    for (int i=0; i<numGen; i++)
    {
        delete [] genSourceMappings[i];
        genSourceMappings[i] = NULL;
        numGenSourceMappings[i] = 0;
        delete [] genDestMappings[i];
        genDestMappings[i] = NULL;
        numGenDestMappings[i] = 0;
    }
    delete [] numGenSourceMappings;
    numGenSourceMappings = NULL;
    delete [] genSourceMappings;
    genSourceMappings = NULL;
    delete [] numGenDestMappings;
    numGenDestMappings = NULL;
    delete [] genDestMappings;
    genDestMappings = NULL;

    return;
} /** End of VDSHierarchy::update() **/


/*****************************************************************************\
 @ VDSHierarchy::finalize
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
VDSHierarchy::finalize(Model *model)
{
    // Vif/VDS needs all vertices to finally merge to one, so do that here
    // (even though all the triangles are already gone)

    if ((model->getNumVerts() + numDanglingVerts) > 1)
    {
        VifMerge merge;

        merge.NodesBeingMerged =
            new unsigned int[model->getNumVerts() + numDanglingVerts];
        merge.NumNodesInMerge = 0;
        for (int i=0; i<model->getNumVerts(); i++)
        {
            xbsVertex *vert = model->getVert(i);
            if (vert->mtIndex == -1)
                continue;
            merge.NodesBeingMerged[merge.NumNodesInMerge++] =
                vert->mtIndex;
            vert->mtIndex = -1;
        }
        for (int i=0; i<numDanglingVerts; i++)
        {
            merge.NodesBeingMerged[merge.NumNodesInMerge++] =
                danglingVerts[i];
        }
        

        // clone a vertex to be the parent
        merge.ParentNode = 
            vif->addVert(vif->Vertices[merge.NodesBeingMerged[0]].VertexPosition,
                         vif->Vertices[merge.NodesBeingMerged[0]].PatchID,
                         false, 0);
        
        vif->addMerge(merge);
    }
    

    delete [] danglingVerts;
    danglingVerts = NULL;
    numDanglingVerts = 0;
    maxDanglingVerts = 0;
    
    // convert Vif to a VDS

#if 1
//    fprintf(stderr, "new VDS::Forest\n");
    mpForest = new Forest;
    mpForest->GetDataFromVif(*vif);
#endif

#if 1
    delete vif;
    vif = NULL;
#endif

    return;
       
} /** End of VDSHierarchy::finalize() **/



/*****************************************************************************\
 @ VDSHierarchy::makeCut
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
GLOD_Cut*
VDSHierarchy::makeCut()
{
    //fprintf(stderr, "VDSHierarchy::makeCut()\n");
    VDSCut *newcut = new VDSCut(this);
    //      fprintf(stderr, "VDSHierarchy::makeCut() returning VDSCut: %p, mpCut: %p\n", newcut, newcut->mpCut);
        
    return (GLOD_Cut*) newcut;
        
} /** End of VDSHierarchy::makeCut() **/

/*****************************************************************************\
 @ VDSCut::updateView
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
int VDSHierarchy::getReadbackSize() {
    return mpForest->GetBinaryVDSSize();
}

/*****************************************************************************\
 @ VDSCut::updateView
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void VDSHierarchy::readback(void* dst) 
{
    mpForest->WriteBinaryVDStoBuffer((char*)dst);
}


/*****************************************************************************\
 @ VDSCut::updateView
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
int VDSHierarchy::load(void* src) 
{
    return mpForest->ReadBinaryVDSfromBuffer((char*)src);
}


void VDSHierarchy::changeQuadricMultiplier(GLfloat multiplier){
    quadricMultiplier  = multiplier;
}
/*****************************************************************************\
 @ VDSCut::VDSCut
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
VDSCut::VDSCut(VDSHierarchy *hier) 
{
    VBO_id = 0;
    hierarchy = hier;
    mpCut = new VDS::Cut;
    mpCut->mpExternalViewClass = &view;
    mpRenderer = new VDS::Renderer(hier->mpForest->mNumNodes, hier->mpForest->mNumTris);
    s_VDSMemoryManager.AddRenderer(mpRenderer);
    
    //              mpRenderer->SetRenderFunc(ImmediateModeRenderCallback);
    mpRenderer->SetRenderFunc(FastRenderCallback);
    
    mpCut->SetForest(hierarchy->mpForest);
    mpCut->SetRenderer(mpRenderer);
    
    mpRenderer->AddCut(mpCut);
};

/*****************************************************************************\
 @ VDSCut::updateView
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
VDSCut::viewChanged()
{
    /*
      VDS::Vec3 forward(
      view.forward.data[0], view.forward.data[1], view.forward.data[2]);
      mpCut->SetLookVector(forward);
      VDS::Vec3 up(
      view.up.data[0], view.up.data[1], view.up.data[2]);
      mpCut->SetUpVector(up);
      VDS::Point3 eye(
      view.eye.data[0], view.eye.data[1], view.eye.data[2]);
      mpCut->SetViewPoint(eye);
      mpCut->SetFov(view.yFOV * 1.2f);
      mpCut->SetAspectRatio(view.aspect);
      mpCut->CalcFrustum();
    */
    //    float m[16];
    
    mpCut->SetTransformationMatrix(view.matrix);
    //fprintf(stderr, "fw(%.2f, %.2f, %.2f), up(%.2f, %.2f, %.2f), vp(%.2f, %.2f, %.2f)\n",
    //              forward.X, forward.Y, forward.Z, up.X, up.Y, up.Z, eye.X, eye.Y, eye.Z);

} /** End of VDSCut::updateView **/


/*****************************************************************************\
 @ VDSCut::setGroup
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
VDSCut::setGroup(GLOD_Group* glodgroup)
{
    group = glodgroup;
    mpCut->SetSimplifier(group->mpSimplifier);
} /** End of VDSCut::setGroup **/


/*****************************************************************************\
 @ VDSCut::BindAdaptXform
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
VDSCut::BindAdaptXform()
{
    float m[16] = { 1,0,0,0,
                    0,1,0,0,
                    0,0,1,0,
                    0,0,0,1};

    glGetFloatv(GL_MODELVIEW_MATRIX,m);

    mpCut->SetTransformationMatrix(m);
    //mpCut->UpdateViewParametersFromMatrix();
    mpCut->SetTransformationMatrix(view.matrix);

} /** End of VDSCut::BindAdaptXform **/



/*****************************************************************************\
 @ VDSCut::adaptObjectSpaceErrorThreshold
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
VDSCut::adaptObjectSpaceErrorThreshold(float threshold)
{
    if (!group->vds_objects_adapted)
    {
        mpCut->mpSimplifier->UpdateNodeErrors();
        mpCut->mpSimplifier->SimplifyThreshold(threshold);
        group->vds_objects_adapted = true;
        updateStats();
    }
    
} /** End of VDSCut::adaptObjectSpaceErrorThreshold **/



/*****************************************************************************\
 @ VDSCut::adaptScreenSpaceErrorThreshold
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
VDSCut::adaptScreenSpaceErrorThreshold(float threshold)
{
    if (!group->vds_objects_adapted)
    {
        mpCut->mpSimplifier->UpdateNodeErrors();
        mpCut->mpSimplifier->SimplifyThreshold(threshold);
        group->vds_objects_adapted = true;
        updateStats();
    }
} /** End of VDSCut::adaptScreenSpaceErrorThreshold **/

/*****************************************************************************\
 @ VDSCut::draw
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
VDSCut::draw(int patchnum)
{
    if (VBO_id == 0)
        initVBO();
    
    if (!glodHasVBO())
    {
        if (mpCut != NULL)
            mpCut->mpRenderer->RenderPatch(patchnum);
    }
    else
    {
        //              printf("Using VBO\n");
        
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBO_id);
        
        mpRenderer->mpOldFastVertexRenderData = mpRenderer->mpFastVertexRenderData;
        _glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
        
        glEnableClientState(GL_VERTEX_ARRAY);
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBO_id);
        
        if (mpRenderer->mpCut->mpForest->mNormalsPresent)
            glEnableClientState(GL_NORMAL_ARRAY);
        else
            glDisableClientState(GL_NORMAL_ARRAY);
        
        if (mpRenderer->mpCut->mpForest->mColorsPresent)
            glEnableClientState(GL_COLOR_ARRAY);
        else
            glDisableClientState(GL_COLOR_ARRAY);
        
        if (mpRenderer->mpCut->mpForest->mNumTextures > 0)
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        else
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        
        if (mpCut != NULL)
            mpCut->mpRenderer->RenderPatch(patchnum);
        
        mpRenderer->mpFastVertexRenderData = (VDS::VertexRenderDatum *) _glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
        
        if (mpRenderer->mpFastVertexRenderData == NULL)
            fprintf(stderr, "Error - glMapBufferARB() returned a null memory address.\n");
        
        if (!mpRenderer->mCopyDataToFastMemoryPerFrame)
        {
            mpRenderer->mpVertexRenderData = 
                mpRenderer->mpFastVertexRenderData;
        }
        
        if (mpRenderer->mpFastVertexRenderData != mpRenderer->mpOldFastVertexRenderData)
        {
            fprintf(stderr, "Vertex buffer object changed addresses; updating BudgetItem pVertexRenderDatum pointers.\n");
            fprintf(stderr, "New Address: %x\n", mpRenderer->mpFastVertexRenderData);
            
            unsigned int k;
            unsigned int index;
            for (k = 1; k <= mpRenderer->mpCut->mpForest->mNumNodes; ++k)
            {
                if (mpRenderer->mpCut->mpNodeRefs[k] != NULL)
                {
                    index = 
                        mpRenderer->mpCut->mpNodeRefs[k]->pVertexRenderDatum -
                        mpRenderer->mpOldFastVertexRenderData;
                    mpRenderer->mpCut->mpNodeRefs[k]->pVertexRenderDatum = 
                        &(mpRenderer->mpFastVertexRenderData[index]);
                }
            }
        }
        mpRenderer->mpOldFastVertexRenderData = 
            mpRenderer->mpFastVertexRenderData;
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
};

/*****************************************************************************\
 @ VDSCut::coarsen
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
VDSCut::coarsen(ErrorMode mode, int triTermination, float ErrorTermination)
{
    mpCut->mpSimplifier->UpdateNodeErrors();
    if (triTermination < 0)
        triTermination = 0;
    mpCut->mpSimplifier->SimplifyBudgetAndThreshold(triTermination, true, 
                                                    ErrorTermination);
    updateStats();
}

/*****************************************************************************\
 @ VDSCut::refine
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       : 
\*****************************************************************************/
void
VDSCut::refine(ErrorMode mode, int triTermination, float ErrorTermination)
{
    mpCut->mpSimplifier->UpdateNodeErrors();
    mpCut->mpSimplifier->SimplifyBudgetAndThreshold(triTermination, true, 
                                                    ErrorTermination);
    updateStats();
}

// not sure what these are supposed to do, but i had to stick
// 'em in or it wouldn't build -BS
#ifdef _WIN32
#pragma warning (disable:4172)
#pragma warning (disable:4700)
#endif

void VDSCut::updateStats()
{
    currentNumTris = 0;
    int i = 0;
    for (i = 0; i < mpCut->mpSimplifier->mNumCuts; ++i)
        currentNumTris += mpCut->mpSimplifier->mpCuts[i]->mNumActiveTris;
    
    if (mpCut->mpSimplifier->mpUnfoldQueue->Size > 0)
    {
        VDS::NodeIndex refinenode =
            mpCut->mpSimplifier->mpUnfoldQueue->FindMin()->miNode;
        int cutID = mpCut->mpSimplifier->mpUnfoldQueue->FindMin()->CutID;
            
        VDS::TriIndex subtri = mpCut->mpSimplifier->mpCuts[cutID]->mpForest->
            mpNodes[refinenode].miFirstSubTri;
        int NumRefineSubTris = 0;
        while (subtri != VDS::Forest::iNIL_TRI)
        {
            subtri = mpCut->mpSimplifier->mpCuts[cutID]->mpForest->
                mpTris[subtri].miNextSubTri;
            ++NumRefineSubTris;
        }
        refineTris = currentNumTris + NumRefineSubTris;
    }
    else
    {
        refineTris = MAXINT;
    }
    
    //      if (coarsenErrorScreenSpace() < currentErrorScreenSpace())
    //      {
    //              fprintf(stderr, "VDS queues unbalanced -> may not be possible to balance GLOD queues.\n");
    //      }
}

xbsReal VDSCut::coarsenErrorObjectSpace(int area)
{
    if (mpCut->mpSimplifier->mpFoldQueue->Size > 0)
    {
        xbsReal t(mpCut->mpSimplifier->mpFoldQueue->FindMin()->mError);
        return t;
    }
    else
    {
        xbsReal t(MAXFLOAT);
        return t;
    }
}

xbsReal VDSCut::currentErrorObjectSpace(int area)
{
    if (mpCut->mpSimplifier->mpUnfoldQueue->Size > 0)
    {
        xbsReal t(mpCut->mpSimplifier->mpUnfoldQueue->FindMin()->mError);
        return t;
    }
    else
    {
        xbsReal t(0);
        return t;
    }
}

xbsReal VDSCut::coarsenErrorScreenSpace(int area)
{
    if (mpCut->mpSimplifier->mpFoldQueue->Size > 0)
    {
/*
  xbsVec3 v(mpCut->mpSimplifier->mpFoldQueue->FindMin()->mPosition.X,
  mpCut->mpSimplifier->mpFoldQueue->FindMin()->mPosition.Y,
  mpCut->mpSimplifier->mpFoldQueue->FindMin()->mPosition.Z);
  xbsReal t(view.computePixelsOfError(v,
  mpCut->mpSimplifier->mpFoldQueue->FindMin()->mRadius));
  return t;
*/
        // couldn't do that because the node on top of the fold queue could
        // belong to another vds cut, so its error would be calculated
        // erroneously if it used this cut's transformation (view).
        // instead just get error directly from budgetitem in queue
        xbsReal t(mpCut->mpSimplifier->mpFoldQueue->FindMin()->mError);
        return t;
    }
    else
    {
        xbsReal t(MAXFLOAT);
        return t;
    }
}

xbsReal VDSCut::currentErrorScreenSpace(int area)
{
    if (mpCut->mpSimplifier->mpUnfoldQueue->Size > 0)
    {
/*
  xbsVec3 v(mpCut->mpSimplifier->mpUnfoldQueue->FindMin()->mPosition.X,
  mpCut->mpSimplifier->mpUnfoldQueue->FindMin()->mPosition.Y,
  mpCut->mpSimplifier->mpUnfoldQueue->FindMin()->mPosition.Z);
  xbsReal t(view.computePixelsOfError(v,
  mpCut->mpSimplifier->mpUnfoldQueue->FindMin()->mRadius));
  return t;
*/
        // couldn't do that because the node on top of the fold queue could
        // belong to another vds cut, so its error would be calculated
        // erroneously if it used this cut's transformation (view).
        // instead just get error directly from budgetitem in queue
        xbsReal t(-mpCut->mpSimplifier->mpUnfoldQueue->FindMin()->mError);
        return t*((VDSHierarchy*)hierarchy)->quadricMultiplier;
    }
    else
    {
        xbsReal t(0);
        return t;
    }
}

// glod code addition: readback of the current cut... based on FastRenderCallback
void VDSCut::getReadbackSizes(int patch, GLuint* nindices, GLuint* nverts) {
    VDS::TriIndex NumTris = mpRenderer->mpPatchTriData[patch].NumTris ? (mpRenderer->mpPatchTriData[patch].LastActiveTri + 1) : 0;
    *nindices = NumTris*3;
    *nverts = mpRenderer->mNumVertices; // this is an 
    printf("Will produce %i indices, %i verts\n", *nindices, *nverts);
}

void VDSCut::readback(int PatchID, GLOD_RawPatch* raw) {
    bool HasColors = mpRenderer->mpPatchTriData[PatchID].ColorsPresent;
    bool HasNormals = mpRenderer->mpPatchTriData[PatchID].NormalsPresent;
    bool HasTexCoords = mpRenderer->mpCut->mpForest->mNumTextures > 0;

    VDS::TriIndex tri;
    VDS::TriIndex NumTris = mpRenderer->mpPatchTriData[PatchID].NumTris ? (mpRenderer->mpPatchTriData[PatchID].LastActiveTri + 1) : 0;

    VDS::TriProxy *tri_array = mpRenderer->mpPatchTriData[PatchID].TriProxiesArray;
    VDS::VertexRenderDatum *vertex_array = mpRenderer->mpVertexRenderData;

    int vprod = 0; int tprod = 0;

    // re-correct the mask of what attributes are present
    if((raw->data_flags & GLOD_HAS_VERTEX_NORMALS) && (!HasNormals))
        raw->data_flags = (raw->data_flags & (~GLOD_HAS_VERTEX_NORMALS));

    if((raw->data_flags & GLOD_HAS_VERTEX_COLORS_3) && (!HasColors))
        raw->data_flags = (raw->data_flags & (~GLOD_HAS_VERTEX_COLORS_3));

    if((raw->data_flags & GLOD_HAS_TEXTURE_COORDS_2) && (!HasTexCoords))
        raw->data_flags = (raw->data_flags & (~GLOD_HAS_TEXTURE_COORDS_2));

    if((raw->data_flags & GLOD_HAS_TEXTURE_COORDS_3))
        raw->data_flags = (raw->data_flags & (~GLOD_HAS_TEXTURE_COORDS_3));

    if((raw->data_flags & GLOD_HAS_VERTEX_COLORS_4))
        raw->data_flags = (raw->data_flags & (~GLOD_HAS_VERTEX_COLORS_4));

    // verification step not possible on the size of the raw object

    HashTable* v_src_to_raw = AllocHashtableBySize(16384); // vertex-index+1 in src --> index_in_raw+1

    // copy all the triangles
    int i; int s_v, d_v;
    vprod = 0;
    for(tri = 0; tri < NumTris; ++tri) {
        for(i = 0; i < 3; i++) {
            s_v = tri_array[tri][i];
            d_v = (int)HashtableSearch(v_src_to_raw, s_v + 1);
            if(d_v == 0) {// never been seen
                d_v = vprod++; // this is where it goes
                {       // copy this vertex over ... vprod counts how far we've packed the raw array
                    VertexRenderDatum* v = &vertex_array[s_v];
                    memcpy(raw->vertices + 3*d_v, (const float*)&v->Position, sizeof(float) * 3);
          
                    if(raw->data_flags & GLOD_HAS_VERTEX_NORMALS)
                        memcpy(raw->vertex_normals + 3*d_v, (const float*)&v->Normal, sizeof(float) * 3);
          
                    if(raw->data_flags & GLOD_HAS_TEXTURE_COORDS_2)
                        memcpy(raw->vertex_texture_coords + 2*d_v, (const float*)&v->TexCoords, sizeof(float) * 2);
          
                    if(raw->data_flags & GLOD_HAS_VERTEX_COLORS_3) {
                        for(int j = 0; j < 3; j++)
                            raw->vertex_colors[3*d_v + j] = ((float)v->Color[j]) / 255.0;
                    }
          
                }
        
                // now remember where it went to
                HashtableAdd(v_src_to_raw, s_v + 1, (void*)(d_v + 1));
                d_v++; // pretend like it came from the zero-bad hash
            }
      
            // set the index in the index array
            raw->triangles[3*tri + i] = d_v - 1;
            tprod++;
        }

    }
  
    FreeHashtableCautious(v_src_to_raw);
}

void VDSCut::initVBO()
{
    if (glodHasVBO())
        mpRenderer->SetRenderFunc(VBOFastRenderCallback);
    else{
        mpRenderer->SetRenderFunc(FastRenderCallback);
        return;
    }

    // This creates a buffer large enough to hold the fully-refined model;
    // We should instead initially create a small buffer and dynamically resize
    // it when the cut grows too big.
    unsigned int FastMemorySize = hierarchy->mpForest->mNumNodes * sizeof(VDS::VertexRenderDatum);

    _glGenBuffersARB(1, &VBO_id);
    _glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBO_id);
    _glBufferDataARB(GL_ARRAY_BUFFER_ARB, FastMemorySize, NULL, GL_STREAM_DRAW_ARB);
    mpRenderer->mpFastVertexRenderData = 
        (VertexRenderDatum *) _glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
    mpRenderer->mpOldFastVertexRenderData = mpRenderer->mpFastVertexRenderData;
    mpRenderer->SetUseFastMemory(true);
}

/*****************************************************************************\
  $Log: Continuous.C,v $
  Revision 1.18  2004/10/12 16:34:52  gfx_friends
  added a multiplier for error quadrics so the error can be more evenly distributed between the different error metrics. It can be changed during runtime

  Revision 1.17  2004/07/08 16:44:41  gfx_friends
  Removed tabs and did 4-space indentation on source files in xbs directory.

  Revision 1.16  2004/06/25 18:58:41  gfx_friends
  New hierarchy, DiscretePatch, which does discrete simplification on a per patch basis. Also added tile management/rendering to glod, but it is broken for vds, so it is disabled by default

  Revision 1.15  2004/06/11 19:25:12  gfx_friends
  Made GLOD keep quiet during execution.

  Revision 1.14  2004/06/11 19:21:38  gfx_friends
  Fixed Win32 build.

  Revision 1.13  2004/03/29 02:17:06  gfx_friends
  fixed a major bug with triangle budget mode, which would force only the root object from a instance list to be simplified.
  Also a bug fix which would infinately loop if some objects were outside the view frustrum.

  Revision 1.12  2004/03/12 17:00:52  gfx_friends
  First stab at using bounding boxes for error calculations. VDS can finally once again be checked out (i hope...)

  Revision 1.11  2004/02/05 20:34:08  gfx_friends
  Modified Discrete and Continuous and Hierarchy to use glod_glext.h instead of their own custom VBO code.

  Revision 1.10  2004/02/04 17:15:01  gfx_friends
  Adding apple makefiles, code changes, which _hopefully_ won't break anything else...

  Revision 1.9  2004/01/21 18:50:46  bms6s
  fudging the view frustum width so we don't see the edges of unsimplification in the viewport

  Revision 1.8  2004/01/21 18:00:20  bms6s
  turned view frustum simplification on (maybe we need to make a new group param to be able to control it?)

  changed view parameter extraction so forward vector seems to be captured ok

  Revision 1.7  2004/01/20 23:30:16  gfx_friends
  Minor change to prevent VBO functions from being called if the VBO extension is not present

  Revision 1.6  2004/01/20 07:26:37  gfx_friends
  Fixed cross platform build issues. Apparently noone has been checking to see if we build for Linux of late.

  Revision 1.5  2004/01/19 20:31:20  bms6s
  don't leave a vbo bound after rendering since it affects gl state

  Revision 1.4  2004/01/13 16:44:48  bms6s
  fixed bug in vbo rendering of discrete object instances
  removed vbo debug printfs

  Revision 1.3  2004/01/13 15:43:13  bms6s
  fixed vbo rendering of more than one continuous cut
  commented out some debug printfs

  Revision 1.2  2004/01/12 16:25:44  bms6s
  removed compile-time vbo flags

  moved discrete rendering/initialization from DiscreteObject to DiscreteCut

  added VBO initialization and rendering for continuous cuts (currently crashes due to glMapBuffer() returning null if more than one continuous cut is in use - still debugging this)

  Revision 1.1  2003/12/18 19:44:07  bms6s
  separated out VDSHierarchy and VDSCut into continuous.h/c


\*****************************************************************************/
