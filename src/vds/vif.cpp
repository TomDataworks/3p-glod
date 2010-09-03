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
#ifdef _WIN32
#pragma warning(disable: 4530)
#endif

#include "vif.h"
#include "vds.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <set>

using namespace std;
using namespace VDS;

char *get_line(ifstream &rIStream);
bool is_blank_line(char *pLine);
bool read_header(istream &rIStream, unsigned int &rMajor, unsigned int &rMinor, unsigned int &rFormat, unsigned int &rNumTextures, unsigned int &rNumVertexPositions, unsigned int &rNumNodes, unsigned int &rNumTris, VDS::PatchIndex &rNumPatches, unsigned int &rNumMerges, unsigned int &rNumErrorParams, int &rErrorParamSize);
void eat(istream &rIStream);
const int MAX_LINE_SIZE = 500;

Vec3 gDefaultNormal(1.0 / sqrt(3.0), 1.0 / sqrt(3.0), 1.0 / sqrt(3.0));

const int POSITION = 1;
const int NORMAL = 4;
const int COLORMASK = 2;
const int TEXTURE = 8;

Vif::Vif()
{
	NumVertexPositions = 0;
	NumVerts = 0;
	NumTris = 0;
	NumPatches = 0;
	
	ColorsPresent = false;
	NormalsPresent = false;
	NumTextures = 0;
	
	VertexPositions = NULL;
	Vertices = NULL;
	TextureCoords = NULL;
	
	Triangles = NULL;
	
	NumMerges = 0;
	Merges = NULL;

	NumErrorParams = 0;
	ErrorParamSize = 0;
	ErrorParams = NULL;
	
	maxVertexPositions = maxVerts = maxTris = maxMerges = 0;
}

Vif::~Vif()
{
    unsigned int i;
    
    if (VertexPositions != NULL)
		delete[] VertexPositions;
    if (Vertices != NULL)
		delete[] Vertices;
    if (TextureCoords != NULL)
    {
		for (i = 0; i < NumVertexPositions; ++i)
			delete[] TextureCoords[i];
		delete[] TextureCoords;
    }
    if (Triangles != NULL)
		delete[] Triangles;
    if (Merges != NULL)
    {
		for (i=0; i<NumMerges; i++)
			delete Merges[i].NodesBeingMerged;
		delete[] Merges;
    }
}

unsigned int
Vif::addVertPos(Point3& coord, ByteColorA& color,
			 Vec3& normal, Point2*& texcoord)
{
    // allocate space
    if (NumVertexPositions == maxVertexPositions)
    {
		if (maxVertexPositions == 0)
		{
			VertexPositions = new VDS::VertexRenderDatum[1];
			if (NumTextures > 0)
			{
			    TextureCoords = new Point2 *[1];
			}
			
			maxVertexPositions = 1;
		}
		else
		{
			VertexRenderDatum *newVertPositions = new VertexRenderDatum[maxVertexPositions*2];
			for (unsigned int i=0; i<NumVertexPositions; i++)
				newVertPositions[i] = VertexPositions[i];
			delete [] VertexPositions;
			VertexPositions = newVertPositions;

			if (NumTextures > 0)
			{
				Point2 **newTexCoord = new Point2 *[maxVertexPositions*2];
				for (unsigned int i=0; i<NumVertexPositions; i++)
					newTexCoord[i] = TextureCoords[i];
				delete [] TextureCoords;
				TextureCoords = newTexCoord;
			}
			
			maxVertexPositions *= 2;
		}
    }

    VertexPositions[NumVertexPositions].Position = coord;
	VertexPositions[NumVertexPositions].Color = color;
	VertexPositions[NumVertexPositions].Normal = normal;
    if (NumTextures > 0)
		TextureCoords[NumVertexPositions] = texcoord;
	
    // notice that we use the array of texcoord handed by the caller,
    // so the caller had better not free it!
    
    NumVertexPositions++;
    return NumVertexPositions-1;
}

unsigned int
Vif::addVert(unsigned int vpos, VDS::PatchIndex patchid,
			 bool coincidentvertflag, unsigned int coincidentvert)
{
    // allocate space
    if (NumVerts == maxVerts)
    {
		if (maxVerts == 0)
		{
			Vertices = new VifVertex[1];

			maxVerts = 1;
		}
		else
		{
			VifVertex *newVerts = new VifVertex[maxVerts*2];
			for (unsigned int i=0; i<NumVerts; i++)
				newVerts[i] = Vertices[i];
			delete [] Vertices;
			Vertices = newVerts;
			
			maxVerts *= 2;
		}
    }
	
    Vertices[NumVerts].VertexPosition = vpos;
    Vertices[NumVerts].PatchID = patchid;
    Vertices[NumVerts].CoincidentVertexFlag = coincidentvertflag;
    Vertices[NumVerts].CoincidentVertex = coincidentvert;
	
    NumVerts++;
    return NumVerts-1;
}

unsigned int
Vif::addTri(unsigned int v1, unsigned int v2, unsigned int v3, 
			VDS::PatchIndex patchid)
{
    if (NumTris == maxTris)
    {
		if (maxTris == 0)
		{
			Triangles = new VifTri[1];
			maxTris = 1;
		}
		else
		{
			VifTri *newTris = new VifTri[maxTris*2];
			for (unsigned int i=0; i<NumTris; i++)
				newTris[i] = Triangles[i];
			delete [] Triangles;
			Triangles = newTris;
			maxTris *= 2;
		}
    }
	
    Triangles[NumTris].Corners[0] = v1;
	Triangles[NumTris].Corners[1] = v2;
	Triangles[NumTris].Corners[2] = v3;
	Triangles[NumTris].PatchID = patchid;
    NumTris++;
    return NumTris-1;
}

unsigned int
Vif::addMerge(VifMerge& merge)
{
    if (NumMerges == maxMerges)
    {
		if (maxMerges == 0)
		{
			Merges = new VifMerge[1];
			maxMerges = 1;
		}
		else
		{
			VifMerge *newMerges = new VifMerge[maxMerges*2];
			for (unsigned int i=0; i<NumMerges; i++)
				newMerges[i] = Merges[i];
			delete [] Merges;
			Merges = newMerges;
			maxMerges *= 2;
		}
		
    }
	
    Merges[NumMerges++] = merge;
    return NumMerges-1;
}

bool Vif::WriteVif2_2(const char *Filename)
{
    FILE *OutputFile;
	unsigned int i,j;
	
	OutputFile = fopen(Filename, "w");
	if (OutputFile == NULL)
	{
		fprintf(stderr, "Error opening file %s\n", Filename);
		return false;
	}
	
	fprintf(OutputFile, "# VIF file created automatically by Vif::WriteVif2_2()\n");
	
	fprintf(OutputFile, "VIF2.2\n");
	fprintf(OutputFile, "format: p");
	if (ColorsPresent)
		fprintf(OutputFile, "c");
	if (NormalsPresent)
		fprintf(OutputFile, "n");
	if (NumTextures > 0)
		fprintf(OutputFile, "x%i", NumTextures);
	fprintf(OutputFile, "\nvertex positions: %i\n", NumVertexPositions);
	fprintf(OutputFile, "vertices: %i\n", NumVerts);
	fprintf(OutputFile, "triangles: %i\n", NumTris);
	fprintf(OutputFile, "patches: %i\n", NumPatches);
	fprintf(OutputFile, "merges: %i\n", NumMerges);
//	fprintf(OutputFile, "\n");
	
	for (i = 0; i < NumVertexPositions; ++i)
	{
		fprintf(OutputFile, "p%i %f %f %f\n", i, VertexPositions[i].Position.X, VertexPositions[i].Position.Y, VertexPositions[i].Position.Z);
		if (ColorsPresent)
			fprintf(OutputFile, "c %u %u %u %u\n", VertexPositions[i].Color.R, VertexPositions[i].Color.G, VertexPositions[i].Color.B, VertexPositions[i].Color.A);
		if (NormalsPresent)
			fprintf(OutputFile, "n %f %f %f\n", VertexPositions[i].Normal.X, VertexPositions[i].Normal.Y, VertexPositions[i].Normal.Z);
		if (NumTextures > 0)
		{
			for (j = 0; j < NumTextures; ++j)
			{
				fprintf(OutputFile, "x%i %f %f\n", j, TextureCoords[i][j].X, TextureCoords[i][j].Y);
			}
		}
	}

	for (i = 0; i < NumVerts; ++i)
	{
		fprintf(OutputFile, "v%i %u %u", i, Vertices[i].VertexPosition, Vertices[i].PatchID);
		if (Vertices[i].CoincidentVertexFlag)
			fprintf(OutputFile, " %u", Vertices[i].CoincidentVertex);
		fprintf(OutputFile, "\n");
	}
	
	for (i = 0; i < NumTris; ++i)
	{
		fprintf(OutputFile, "t %u %u %u %u\n", Triangles[i].Corners[0], Triangles[i].Corners[1], Triangles[i].Corners[2], Triangles[i].PatchID);
	}	
	
	for (i = 0; i < NumMerges; ++i)
	{
		fprintf(OutputFile, "m%i", Merges[i].ParentNode);
		for (j = 0; j < Merges[i].NumNodesInMerge; ++j)
		{
			fprintf(OutputFile, " %i", Merges[i].NodesBeingMerged[j]);
		}
		fprintf(OutputFile, "\n");
	}
	
	fclose(OutputFile);
	return true;
}

bool Vif::WriteVif2_3(const char *Filename)
{
    FILE *OutputFile;
	unsigned int i,j;
    int k;
	
	OutputFile = fopen(Filename, "w");
	if (OutputFile == NULL)
	{
		fprintf(stderr, "Error opening file %s\n", Filename);
		return false;
	}
	
	fprintf(OutputFile, "# VIF file created automatically by Vif::WriteVif2_3()\n");
	
	fprintf(OutputFile, "VIF2.3\n");
	fprintf(OutputFile, "format: p");
	if (ColorsPresent)
		fprintf(OutputFile, "c");
	if (NormalsPresent)
		fprintf(OutputFile, "n");
	if (NumTextures > 0)
		fprintf(OutputFile, "x%i", NumTextures);
	fprintf(OutputFile, "\nvertex positions: %i\n", NumVertexPositions);
	fprintf(OutputFile, "vertices: %i\n", NumVerts);
	fprintf(OutputFile, "triangles: %i\n", NumTris);
	fprintf(OutputFile, "patches: %i\n", NumPatches);
	fprintf(OutputFile, "errorparams: %i\n", NumErrorParams);
	if (NumErrorParams > 0)
		fprintf(OutputFile, "errorparam size: %i\n", ErrorParamSize);
	fprintf(OutputFile, "merges: %i\n", NumMerges);
//	fprintf(OutputFile, "\n");
	
	for (i = 0; i < NumVertexPositions; ++i)
	{
		fprintf(OutputFile, "p%i %f %f %f\n", i, VertexPositions[i].Position.X, VertexPositions[i].Position.Y, VertexPositions[i].Position.Z);
		if (ColorsPresent)
			fprintf(OutputFile, "c %u %u %u %u\n", VertexPositions[i].Color.R, VertexPositions[i].Color.G, VertexPositions[i].Color.B, VertexPositions[i].Color.A);
		if (NormalsPresent)
			fprintf(OutputFile, "n %f %f %f\n", VertexPositions[i].Normal.X, VertexPositions[i].Normal.Y, VertexPositions[i].Normal.Z);
		if (NumTextures > 0)
		{
			for (j = 0; j < NumTextures; ++j)
			{
				fprintf(OutputFile, "x%i %f %f\n", j, TextureCoords[i][j].X, TextureCoords[i][j].Y);
			}
		}
	}

	for (i = 0; i < NumVerts; ++i)
	{
		fprintf(OutputFile, "v%i %u %u", i, Vertices[i].VertexPosition, Vertices[i].PatchID);
		if (Vertices[i].CoincidentVertexFlag)
			fprintf(OutputFile, " %u", Vertices[i].CoincidentVertex);
		fprintf(OutputFile, "\n");
	}
	
	for (i = 0; i < NumTris; ++i)
	{
		fprintf(OutputFile, "t %u %u %u %u\n", Triangles[i].Corners[0], Triangles[i].Corners[1], Triangles[i].Corners[2], Triangles[i].PatchID);
	}	
	
	if (NumErrorParams > 0)
	{
		unsigned int errorvalueindex = 0;
		for (i = 0; i < NumErrorParams; ++i)
		{
			fprintf(OutputFile, "e%u", i);
			for (k = 0; k < ErrorParamSize; ++k)
			{
				fprintf(OutputFile, " %f", ErrorParams[errorvalueindex]);
				++errorvalueindex;
			}
			fprintf(OutputFile, "\n");
		}	
	}
	
	for (i = 0; i < NumMerges; ++i)
	{
		fprintf(OutputFile, "m%i", Merges[i].ParentNode);
		if (NumErrorParams > 0)
			fprintf(OutputFile, " e%i", Merges[i].ErrorParamIndex);
		for (j = 0; j < Merges[i].NumNodesInMerge; ++j)
		{
			fprintf(OutputFile, " %i", Merges[i].NodesBeingMerged[j]);
		}
		fprintf(OutputFile, "\n");
	}
	
	fclose(OutputFile);
	return true;
}

bool Vif::ReadVif2_2(const char *Filename)
{
	char letter;
	unsigned int i, j;
    ifstream fin;
	Point3 pos;
	Vec3 normal;
	ByteColorA color;
    unsigned int major, minor;
//    bool is_merge_tree;
//    bool is_node_over;
    unsigned int format;
	NodeIndex node, user_index;
	TriIndex tri;
	NodeIndex merge, parent, child;
	NodeIndex children[128];

	fin.open(Filename);
	if (!fin.is_open())
	{
		fprintf(stderr, "Error opening file \"%s\"\n", Filename);
		return false;
	}

    cout << "Reading VIF Header...";
    if (!read_header(fin, major, minor, format, NumTextures, NumVertexPositions, NumVerts, NumTris, NumPatches, NumMerges, NumErrorParams, ErrorParamSize))
    {
		cerr << "Incorrect header in \"" << Filename << "\"" << endl;
		return false;
    }

    cout << "finished." << endl;    
    if ((format & POSITION) == 0)
    {
        cerr << "VIF files must contain position" << endl;
        return false;
    }

	cout << "Reading Vertex Positions...";

	VertexPositions = new VertexRenderDatum[NumVertexPositions];

	if (format & COLORMASK)
		ColorsPresent = true;
	else
		ColorsPresent = false;

	if (format & NORMAL)
		NormalsPresent = true;
	else
		NormalsPresent = false;

    if (NumTextures > 0)
    {
		TextureCoords = new Point2 *[NumVertexPositions];
		for (i = 0; i < NumVertexPositions; ++i)
			TextureCoords[i] = new Point2[NumTextures];
    }

	for (i = 0; i < NumVertexPositions; ++i)
	{
        eat(fin);
        fin.get(letter);
        if (letter != 'p')
        {
            cerr << "Error on vertex " << i << ", expected vertex" << endl;
            return false;
        }
        if (!isspace(fin.peek()))
        {
            fin >> user_index;
            if (user_index != i)
            {
                cerr << "Error, vertex index agreement." << endl;
                return false;
            }
        }
        fin >> pos;

		if (ColorsPresent)
		{
            eat(fin);        
            fin.get(letter);
            if (letter != 'c')
            {
				cerr << "Error - expected vertex color." << endl;
			}
			fin >> color;
		}
		if (NormalsPresent)
		{
            eat(fin);        
            fin.get(letter);
            if (letter != 'n')
            {
				cerr << "Error - expected vertex normal." << endl;
			}
			fin >> normal;
			if (normal.LengthSquared() == 0.0)
				normal = gDefaultNormal;
			else
				normal.Normalize();
		}
		for (j = 0; j < NumTextures; ++j)
		{
            eat(fin);        
            fin.get(letter);
            if (letter != 'x')
            {
				cerr << "Error - expected texture coordinate " << j << "." << endl;
			}
			if (isspace(fin.peek()))
			{
				cerr << "Error - expected texture coordinate " << j << "'s index." << endl;
			}
			fin >> user_index;
			if (user_index != j)
			{
				cerr << "Error, texture coordinate " << j << " index agreement." << endl;
				return false;
			}
			fin >> TextureCoords[i][j];
        }

		VertexPositions[i].Position = pos;

		if (ColorsPresent)
		{
			VertexPositions[i].Color = color;
		}
		if (NormalsPresent)
		{
			VertexPositions[i].Normal = normal;
		}
	}

	cout << "finished." << endl;
	cout << "Reading Nodes...";

	Vertices = new VifVertex[NumVerts];
//	char *line;
	bool coincidentflag;
	unsigned int vp, patchid, coincidentvert;

    for (node = 0; node < NumVerts; node++)
    {
        eat(fin);
        fin.get(letter);
        if (letter != 'v')
        {
            cerr << "Error on vertex " << node << ", expected vertex" << endl;
            return false;
        }
        if (!isspace(fin.peek()))
        {
            fin >> user_index;
            if (user_index != node)
            {
                cerr << "Error, vertex index agreement." << endl;
                return false;
            }
        }

		fin >> vp >> patchid;
		eat(fin);
		char c = fin.peek();
		if ((c >= '0') && (c <= '9'))
		{
			coincidentflag = true;
			fin >> coincidentvert;
		}
		else
		{
			coincidentflag = false;
			coincidentvert = 666666;
		}

		if (patchid > NumPatches)
		{
			cerr << "Invalid patch ID specified in vertex " << node << endl;
			return false;
		}
		Vertices[node].VertexPosition = vp;
		Vertices[node].PatchID = patchid;
		Vertices[node].CoincidentVertexFlag = coincidentflag;
		Vertices[node].CoincidentVertex = coincidentvert;
    }

    for (node = 0; node < NumVerts; node++)
    {
		if (Vertices[node].CoincidentVertexFlag)
		{
			i = node;
			if (Vertices[i].CoincidentVertex == i)
			{
				cerr << "Error - Coincident vertex points to self." << endl;
				return false;
			}
			while (Vertices[i].CoincidentVertex != node)
			{
				if (!Vertices[i].CoincidentVertexFlag)
				{
					cerr << "Error - Coincident vertex doesn't have coincident vertex flag set." << endl;
					return false;
				}
				i = Vertices[i].CoincidentVertex;
			}
		}
	}

	cout << "finished." << endl;

    cout << "Reading Tris..." << flush;

	Triangles = new VifTri[NumTris];
	unsigned int v0, v1, v2;

	for (tri = 0; tri < NumTris; ++tri)
	{
        eat(fin);
        if (fin.get() != 't')
        {
            cerr << "Error, expecting tri." << endl;
            return false;
        }

		fin >> v0 >> v1 >> v2 >> patchid;

        if ((v0 >= NumVerts) || (v1 >= NumVerts) || (v2 >= NumVerts))
        {
            cerr << "Invalid vertex specified in tri number " << tri << endl;
            return false;
        }
		if (patchid > NumPatches)
		{
			cerr << "Invalid patch ID specified in tri number " << tri << endl;
			return false;
		}
		if ((patchid != Vertices[v0].PatchID) || (patchid != Vertices[v1].PatchID) || (patchid != Vertices[v2].PatchID))
		{
			cerr << "Error - tri number " << tri << " has patch ID different than one of its corner vertices' patch ID." << endl;
			return false;
		}
		Triangles[tri].Corners[0] = v0;
		Triangles[tri].Corners[1] = v1;
		Triangles[tri].Corners[2] = v2;
		Triangles[tri].PatchID = patchid;
	}
	cout << "finished." << endl;

	cout << "Reading Merges..." << flush;
	Merges = new VifMerge[NumMerges];
	
	for (merge = 0; merge < NumMerges; ++merge)
	{
		eat(fin);
		if (fin.get() != 'm')
		{
			cerr << "Error, expecting merge." << endl;
			return false;
		}
		fin >> parent;
		if (parent >= NumVerts)
		{
			cerr << "Attempt to merge invalid vertex in VIF." << endl;
			return false;
		}
		Merges[merge].ParentNode = parent;
		i = 0;
		eat(fin);
		while (fin.peek() != 'm' && !fin.eof())
		{
			fin >> child;
			if (child >= NumVerts)
			{
				cerr << "Attempt to merge invalid node." << endl;
				return false;
			}
			children[i] = child;
			++i;
			eat(fin);
		}
		Merges[merge].NumNodesInMerge = i;
		Merges[merge].NodesBeingMerged = new unsigned int[Merges[merge].NumNodesInMerge];
		for (i = 0; i < Merges[merge].NumNodesInMerge; ++i)
			Merges[merge].NodesBeingMerged[i] = children[i];
	}
	cout << "finished." << endl;
	return true;
}

bool Vif::ReadVif2_3(const char *Filename)
{
	char letter;
	unsigned int i, j;
    int k;
	int errorparamindex;
    ifstream fin;
	Point3 pos;
	Vec3 normal;
	ByteColorA color;
    unsigned int major, minor;
//    bool is_merge_tree, is_node_over;
    unsigned int format;
	NodeIndex node, user_index;
	TriIndex tri;
	NodeIndex merge, parent, child;
	NodeIndex children[128];

	fin.open(Filename);
	if (!fin.is_open())
	{
		fprintf(stderr, "Error opening file \"%s\"\n", Filename);
		return false;
	}

    cout << "Reading VIF Header...";
    if (!read_header(fin, major, minor, format, NumTextures, NumVertexPositions, NumVerts, NumTris, NumPatches, NumMerges, NumErrorParams, ErrorParamSize))
    {
		cerr << "Incorrect header in \"" << Filename << "\"" << endl;
		return false;
    }

    cout << "finished." << endl;    
    if ((format & POSITION) == 0)
    {
        cerr << "VIF files must contain position" << endl;
        return false;
    }

	cout << "Reading Vertex Positions...";

	VertexPositions = new VertexRenderDatum[NumVertexPositions];

	if (format & COLORMASK)
		ColorsPresent = true;
	else
		ColorsPresent = false;

	if (format & NORMAL)
		NormalsPresent = true;
	else
		NormalsPresent = false;

    if (NumTextures > 0)
    {
		TextureCoords = new Point2 *[NumVertexPositions];
		for (i = 0; i < NumVertexPositions; ++i)
			TextureCoords[i] = new Point2[NumTextures];
    }

	for (i = 0; i < NumVertexPositions; ++i)
	{
        eat(fin);
        fin.get(letter);
        if (letter != 'p')
        {
            cerr << "Error on vertex " << i << ", expected vertex" << endl;
            return false;
        }
        if (!isspace(fin.peek()))
        {
            fin >> user_index;
            if (user_index != i)
            {
                cerr << "Error, vertex index agreement." << endl;
                return false;
            }
        }
        fin >> pos;

		if (ColorsPresent)
		{
            eat(fin);        
            fin.get(letter);
            if (letter != 'c')
            {
				cerr << "Error - expected vertex color." << endl;
			}
			fin >> color;
		}
		if (NormalsPresent)
		{
            eat(fin);        
            fin.get(letter);
            if (letter != 'n')
            {
				cerr << "Error - expected vertex normal." << endl;
			}
			fin >> normal;
			if (normal.LengthSquared() == 0.0)
				normal = gDefaultNormal;
			else
				normal.Normalize();
		}
		for (j = 0; j < NumTextures; ++j)
		{
            eat(fin);        
            fin.get(letter);
            if (letter != 'x')
            {
				cerr << "Error - expected texture coordinate " << j << "." << endl;
			}
			if (isspace(fin.peek()))
			{
				cerr << "Error - expected texture coordinate " << j << "'s index." << endl;
			}
			fin >> user_index;
			if (user_index != j)
			{
				cerr << "Error, texture coordinate " << j << " index agreement." << endl;
				return false;
			}
			fin >> TextureCoords[i][j];
        }

		VertexPositions[i].Position = pos;

		if (ColorsPresent)
		{
			VertexPositions[i].Color = color;
		}
		if (NormalsPresent)
		{
			VertexPositions[i].Normal = normal;
		}
	}

	cout << "finished." << endl;
	cout << "Reading Nodes...";

	Vertices = new VifVertex[NumVerts];
//	char *line;
	bool coincidentflag;
	unsigned int vp, patchid, coincidentvert;

    for (node = 0; node < NumVerts; node++)
    {
        eat(fin);
        fin.get(letter);
        if (letter != 'v')
        {
            cerr << "Error on vertex " << node << ", expected vertex" << endl;
            return false;
        }
        if (!isspace(fin.peek()))
        {
            fin >> user_index;
            if (user_index != node)
            {
                cerr << "Error, vertex index agreement." << endl;
                return false;
            }
        }

		fin >> vp >> patchid;
		eat(fin);
		char c = fin.peek();
		if ((c >= '0') && (c <= '9'))
		{
			coincidentflag = true;
			fin >> coincidentvert;
		}
		else
		{
			coincidentflag = false;
			coincidentvert = 666666;
		}

		if (patchid > NumPatches)
		{
			cerr << "Invalid patch ID specified in vertex " << node << endl;
			return false;
		}
		Vertices[node].VertexPosition = vp;
		Vertices[node].PatchID = patchid;
		Vertices[node].CoincidentVertexFlag = coincidentflag;
		Vertices[node].CoincidentVertex = coincidentvert;
    }

    for (node = 0; node < NumVerts; node++)
    {
		if (Vertices[node].CoincidentVertexFlag)
		{
			i = node;
			if (Vertices[i].CoincidentVertex == i)
			{
				cerr << "Error - Coincident vertex points to self." << endl;
				return false;
			}
			while (Vertices[i].CoincidentVertex != node)
			{
				if (!Vertices[i].CoincidentVertexFlag)
				{
					cerr << "Error - Coincident vertex doesn't have coincident vertex flag set." << endl;
					return false;
				}
				i = Vertices[i].CoincidentVertex;
			}
		}
	}

	cout << "finished." << endl;

    cout << "Reading Tris..." << flush;

	Triangles = new VifTri[NumTris];
	unsigned int v0, v1, v2;

	for (tri = 0; tri < NumTris; ++tri)
	{
        eat(fin);
        if (fin.get() != 't')
        {
            cerr << "Error, expecting tri." << endl;
            return false;
        }

		fin >> v0 >> v1 >> v2 >> patchid;

        if ((v0 >= NumVerts) || (v1 >= NumVerts) || (v2 >= NumVerts))
        {
            cerr << "Invalid vertex specified in tri number " << tri << endl;
            return false;
        }
		if (patchid > NumPatches)
		{
			cerr << "Invalid patch ID specified in tri number " << tri << endl;
			return false;
		}
		if ((patchid != Vertices[v0].PatchID) || (patchid != Vertices[v1].PatchID) || (patchid != Vertices[v2].PatchID))
		{
			cerr << "Error - tri number " << tri << " has patch ID different than one of its corner vertices' patch ID." << endl;
			return false;
		}
		Triangles[tri].Corners[0] = v0;
		Triangles[tri].Corners[1] = v1;
		Triangles[tri].Corners[2] = v2;
		Triangles[tri].PatchID = patchid;
	}
	cout << "finished." << endl;

	if (NumErrorParams)
	{
		cout << "Reading Errors..." << flush;

		if (ErrorParamSize < 1)
		{
			cerr << "Error - ErrorParamSize (" << ErrorParamSize << ") expected to be a positive integer." << endl;
			return false;
		}

		ErrorParams = new float[NumErrorParams * ErrorParamSize];
		errorparamindex = 0;
		for (i = 0; i < NumErrorParams; ++i)
		{
			eat(fin);
			fin.get(letter);
			if (letter != 'e')
			{
				cerr << "Error on error param " << i << ", expected error param" << endl;
				return false;
			}
			fin >> user_index;
			if (user_index != i)
			{
				cerr << "Error in error param index agreement." << endl;
				return false;
			}
			for (k = 0; k < ErrorParamSize; ++k)
			{
				eat(fin);
				if (fin.peek() == 'e' || fin.peek() == 'm' || fin.eof())
				{
					cerr << "Error in expected number of floats in error param." << endl;
					return false;
				}
				fin >> ErrorParams[errorparamindex];
				++errorparamindex;
			}
		}
		cout << "finished." << endl;
	}

	cout << "Reading Merges..." << flush;
	Merges = new VifMerge[NumMerges];
	
	for (merge = 0; merge < NumMerges; ++merge)
	{
		eat(fin);
		if (fin.get() != 'm')
		{
			cerr << "Error, expecting merge." << endl;
			return false;
		}
		fin >> parent;
		if (parent >= NumVerts)
		{
			cerr << "Attempt to merge invalid vertex in VIF." << endl;
			return false;
		}
		Merges[merge].ParentNode = parent;

		if (NumErrorParams > 0)
		{
			eat(fin);
			if (fin.get() != 'e')
			{
				cerr << "Error - expected error param index in merge " << merge << endl;
				return false;
			}
			fin >> errorparamindex;
			if (errorparamindex < 1)
			{
				cerr << "Error - error param index of merge " << merge << " is less than 1." << endl;
				return false;
			}
		}

		i = 0;
		eat(fin);
		while (fin.peek() != 'm' && !fin.eof())
		{
			fin >> child;
			if (child >= NumVerts)
			{
				cerr << "Attempt to merge invalid node." << endl;
				return false;
			}
			children[i] = child;
			++i;
			eat(fin);
		}
		Merges[merge].NumNodesInMerge = i;
		Merges[merge].NodesBeingMerged = new unsigned int[Merges[merge].NumNodesInMerge];
		for (i = 0; i < Merges[merge].NumNodesInMerge; ++i)
			Merges[merge].NodesBeingMerged[i] = children[i];
		Merges[merge].ErrorParamIndex = errorparamindex;
	}
	cout << "finished." << endl;
	return true;
}

//return the next non-blank, non-comment line
char *get_line(istream &rIStream)
{
    static char line[MAX_LINE_SIZE];
    char *p;

    do
    {
        rIStream.getline(line, MAX_LINE_SIZE);
    } 
    while (is_blank_line(line));
    
    p = line;
    do
    {
        if (*p == '#' || *p == '\n')
        {
            *p = '\0';
        }
        p++;
    }
    while (*p != '\0');
    return line;
}

//check to see if line contains nothing but comments and white space
bool is_blank_line(char *pLine)
{
    char *p = pLine;    
    do 
    {
        if (*p == '#')
        {
            return true;
        }
        if (!isspace(*p))
        {            
            return false;
        }
        p++;
    }
    while (*p != '\0');
        
    return true;
}

void eat(istream &rIStream)
{
    static char line[MAX_LINE_SIZE];
    while (isspace(rIStream.peek()) || rIStream.peek() == '#' || rIStream.peek() == '\n' || rIStream.peek() == '\r' || rIStream.peek() == '\f')
    {
        if (rIStream.peek() == '#')
        {
            rIStream.getline(line, MAX_LINE_SIZE);
        }
        else
        {
            rIStream.ignore(1);
        }
    }
}

// Parse and read VIF header information, returning true if good header
bool read_header(istream &rIStream, unsigned int &rMajor, unsigned int &rMinor, unsigned int &rFormat, unsigned int &rNumTextures, unsigned int &rNumVertexPositions, unsigned int &rNumNodes, unsigned int &rNumTris, VDS::PatchIndex &rNumPatches, unsigned int &rNumMerges, unsigned int &rNumErrorParams, int &rErrorParamSize)
{
    char *line;    
    line = get_line(rIStream);
    if (sscanf(line, "VIF%d.%u", &rMajor, &rMinor) != 2)
    {
		cerr << "Error reading VIF version" << endl;
	    return false;
    }
    rNumTextures = 0;
    if (rMajor < 2 || (rMajor == 2 && ((rMinor != 2) && (rMinor != 3))))
    {
        cerr << "Support for older VIF format not implemented yet." << endl;
        return false;
    }
    else
    {
		cout << "Reading VIF" << rMajor << "." << rMinor << " file." << endl;
        string instr;
        rFormat = 0;
        rIStream >> instr;
        if (instr != "format:")
        {
			cerr << "Error reading VIF format" << endl;
            return false;
        }
        bool finished = false;
        eat(rIStream);
        while (!finished)
        {
            switch (rIStream.peek())
            {
            case 'p':
                rIStream.ignore(1);
                rFormat |= POSITION;
                break;
            case 'c':
                rIStream.ignore(1);
                rFormat |= COLORMASK;
                break;
            case 'n':
                rIStream.ignore(1);
                rFormat |= NORMAL;
                break;
            case 'x':
                rIStream.ignore(1);
                rFormat |= TEXTURE;
                eat(rIStream);
                rIStream >> rNumTextures;
                break;
            default:
                finished = true;
                break;
            }
            eat(rIStream);
        }
    }
    line = get_line(rIStream);
    if (sscanf(line, "vertex positions: %u", &rNumVertexPositions) != 1) 
    {
	    return false;
    }
    line = get_line(rIStream);
    if (sscanf(line, "vertices: %u", &rNumNodes) != 1) 
    {
	    return false;
    }
    line = get_line(rIStream);
    if (sscanf(line, "triangles: %u", &rNumTris) != 1)
    {
	    return false;
    }
    line = get_line(rIStream);
    if (sscanf(line, "patches: %hu", &rNumPatches) != 1)
    {
	    return false;
    }
	if (rMinor >= 3)
	{
		line = get_line(rIStream);
		if (sscanf(line, "errorparams: %u", &rNumErrorParams) != 1)
		{
			return false;
		}
		if (rNumErrorParams > 0)
		{
			line = get_line(rIStream);
			if (sscanf(line, "errorparam size: %u", &rErrorParamSize) != 1)
			{
				return false;
			}
		}
	}
	else
	{
		rNumErrorParams = 0;
		rErrorParamSize = 0;
	}
    eat(rIStream);
    rNumMerges = 0;

	line = get_line(rIStream);
	if (sscanf(line, "merges: %u", &rNumMerges) != 1)
	{
		cerr << "Error - cannot find number of merges." << endl;
		return false;
	}
    return true;
}
