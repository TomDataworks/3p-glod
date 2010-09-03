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
#ifndef GENTSTRIPS
#define GENTSTRIPS

typedef struct Meshobj {
    int			connectcount;
    int			independentcount;
    int			npolys;
    int			vertcount;
    int			vertno;
    struct Edge *	edgearray;
    struct Edge *	freeedges;
    struct Edge **	edgehashlist;
    struct Tri *	curtri;
    struct Tri **	trihashlist;
    struct Trilist *	adjtrilist[4];
    struct Trilist *	donetrilist;
    struct Trilist *	newtrilist;
    struct Trilist *	duptrilist;
    struct Trilist *	trilist;
    struct Vert *	tmpvert;
    struct Vert **	verthashlist;
    struct Vertlist *	vertlist;
    void		(*ambegin)( int, int );
    void		(*amend)( void );
    int			(*amhashvert)( unsigned int );
    int			(*amvertsame)( unsigned int, unsigned int );
    void		(*amvertdata)( unsigned int );
    void		(*ambgntmesh)( void );
    void		(*amendtmesh)( void );
    void		(*amswaptmesh)( void );
    void		(*amvert)( unsigned int );
} Meshobj;


Meshobj * 	newMeshobj ( 
    void (*)( int, int ), void (*)( void ),
    int	 (*)( unsigned int ),     int  (*)( unsigned int, unsigned int ),
    void (*)( unsigned int ),     void (*)( void ),
    void (*)( void ),     void (*)( void ),
    void (*)( unsigned int ) );
void		freeMeshobj( Meshobj * );
void		in_ambegin( Meshobj *, int );
void		in_amnewtri( Meshobj *, int );
void		in_amvert( Meshobj *, unsigned int );
int 		in_amend( Meshobj * );

#endif
