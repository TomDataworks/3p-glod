/*
 *	tomesh - 
 *		Convert independent triangles to large triangle meshes
 *
 *			    Kurt Akeley - 24 March 1988
 *		     		Paul Haeberli - 1990	
 *			   Derrick Burns - September 1990
 *
 *
 *	The algorithm attempts to avoid leaving isolated triangles by
 *	choosing as the next triangle in a growing mesh that adjacent
 *	triangle with the minimum number of neighbors.  It will frequently
 *	produce a single, large mesh for a well behaved set of triangles.
 *
 *	To use, include the file mesh.h.
 *
 *	exports four routines:	
 *
 *		Meshobj *newMeshobj( ... )
 *		void in_ambegin( Meshobj *, int )
 *		void in_amnewtri( Meshobj * )
 *		void in_amvert( Meshobj *, unsigned int )
 *		void in_amend( Meshobj * )
 *		void freeMeshobj( Meshobj * )
 *
 *	calls back these routines:	
 *		void out_ambegin( unsigned int nverts, unsigned int ntris )
 *		void out_amend( void );
 * 		int out_amhashvert( unsigned int v )
 *		int out_amvertsame( unsigned int v1, unsigned int v2 )
 *		void out_amvertdata( unsigned int fptr )
 *		void out_ambgntmesh( void ) 
 *		void out_amendtmesh( void ) 
 *		void out_amswaptmesh( void ) 
 *		void out_amvert( unsigned int index )
 *
 *	on output the package calls:
 *	
 *	void out_ambegin( unsigned int nverts, unsigned int ntris )
 *	calls repeatedly:
 *		int out_amhashvert( unsigned int v )
 *		int out_amvertsame( unsigned int v1, unsigned int v2 )
 *	calls nverts times:
 *		void out_amvertdata( unsigned int fptr )
 *	calls these in mixed sequence:
 *		void out_ambgntmesh( void ) 
 *		  void out_amvert( unsigned int index )
 *		  void out_amswaptmesh( void )
 *		void out_amendtmesh( void ) 
 *	void out_amend()
 *
 *
 *      In clearer words:
 *
 *        The user of this package should call newMeshobj() and specify
 *        all callback functions. Then the user should call something like:
 *
 *           in_ambegin(m, nverts)
 *           for (n=0; n<numTris; n++) {
 *              t = triangle[n];
 *              in_amnewtri(m);
 *              in_amvert(m, t->vertexIndex[0]);
 *              in_amvert(m, t->vertexIndex[1]);
 *              in_amvert(m, t->vertexIndex[2]);
 *           }
 *           in_amend(m);   <-- will call the specified callbacks.
 *           freeMeshobj(m);
 *
 *      The callbacks will be called as follows:
 *           
 *         out_ambegin(nverts, ntris)   setup for hashing nverts and outputting
 *                                      meshes from ntris
 *         out_amvertdata(fptr)         called for each vertex
 *         while (meshing)
 *            out_amhashvert(v)         hash for vertex v (if shared vertices
 *                                      are used, hash reduces to table lookup?)
 *            out_amvertsame(v0,v1)     called to compare two verts
 *            
 *            (calls these in mixed sequence:)
 *            void out_ambgntmesh( void )
 *              void out_amvert( unsigned int index )
 *              void out_amswaptmesh( void )
 *            void out_amendtmesh( void )
 *         endwhile
 *
 *         out_amend()                  end of outputting
 */

#include <stdlib.h>
#include <stdio.h>
#include "tmesh.h"

/* #define dprintf if (1) fprintf */
#define dprintf if (0) fprintf

#define INITMESH	replaceB = false
#define SWAPMESH	replaceB = !replaceB
#define REPLACEVERT(v)	{vert[replaceB ? 1 : 0] = (v); SWAPMESH;}

#define TABLE_SIZE0 2003
#define TABLE_SIZE1 5003
#define TABLE_SIZE2 17003
#define TABLE_SIZE3 53003
#define TABLE_SIZE4 153003
#define TABLE_SIZE5 553003
#define TABLE_SIZE6 1000003
#define TABLE_SIZE7 1500007
#define TABLE_SIZE8 2000003


typedef struct Vert {
    struct Vert *next,*prev;
    struct Vert *hnext;
    unsigned int id, index;
} Vert;

typedef struct Vertlist {
    Vert *first,*last;
} Vertlist;

typedef struct Edge {
    struct Edge *hnext;
    struct Tri *tri;
    unsigned int index0, index1;
} Edge;

typedef struct Tri {
    struct Tri *next,*prev;
    struct Tri *hnext;
    struct Tri *(adj[3]);
    Vert *vert[3];
    int drawvert[3];
    int adjcount;
    int used;
    int id;
} Tri;

typedef struct Trilist {
    Tri *first,*last;
} Trilist;

static int vertHashSize = -1;
static int edgeHashSize = -1;
static int triHashSize  = -1;

/* local functions */
static Tri *	hashtrifind( Meshobj *, Tri * );
static Vert *	hashvertfind( Meshobj *, Vert * );
static void	beginoutputobj( Meshobj *, Vert *, Tri * );
static void	freelists( Meshobj * );
static void	hashedgebegin( Meshobj *, int );
static void	hashedgeadd( Meshobj *, Tri *, Vert *, Vert * );
static Edge *	hashedgefind( Meshobj *, Vert *, Vert * );
static void	hashedgeend( Meshobj * );
static void	hashtriadd( Meshobj *, Tri * );
static void	hashtribegin( Meshobj * );
static void	hashtriend( Meshobj * );
static int	hashvert( Meshobj *, Vert * );
static void	hashvertadd( Meshobj *, Vert* );
static void	hashvertbegin( Meshobj * );
static void	hashvertend( Meshobj * );
static void	makelists( Meshobj * );
static void	outmesh( Meshobj *, Trilist * );
static void	removeadjacencies( Meshobj *, Tri * );

static Tri *	maketri( void ) ;
static Tri *	minadj( Tri * );
static Trilist *maketrilist( void ) ;
static Vert *	makevert( void ) ;
static Vertlist *makevertlist( void );
static int	hashedge( unsigned int, unsigned int );
static int	hashtri( Tri * );
static int	ismember( Vert *, Tri * );
static int	notcommon( Tri *, Tri * );
static int	triequal( Tri *, Tri * ) ;
static void	deletetri( Trilist *, Tri * );
static void	freetri( Tri * );
static void	freetrilist( Trilist * );
static void	freevert( Vert * );
static void	freevertlist( Vertlist * );
static void	inserttri( Trilist *, Tri * );
static void	insertvert( Vertlist *, Vert * );

int makered = 0;


Meshobj *
newMeshobj ( 
    void	(*out_ambegin)( int, int ),
    void	(*out_amend)( void ),
    int		(*out_amhashvert)( unsigned int ),
    int		(*out_amvertsame)( unsigned int, unsigned int ),
    void	(*out_amvertdata)( unsigned int ),
    void	(*out_ambgntmesh)( void ),
    void	(*out_amendtmesh)( void ),
    void	(*out_amswaptmesh)( void ),
    void	(*out_amvert)( unsigned int ) )
{
    Meshobj *m;

    m = (Meshobj*)malloc(sizeof(Meshobj));
    m->connectcount	= 0;
    m->independentcount	= 0;
    m->ambegin		= out_ambegin;
    m->amend		= out_amend;
    m->amhashvert	= out_amhashvert;
    m->amvertsame	= out_amvertsame;
    m->amvertdata	= out_amvertdata;
    m->ambgntmesh	= out_ambgntmesh;
    m->amendtmesh	= out_amendtmesh;
    m->amswaptmesh	= out_amswaptmesh;
    m->amvert		= out_amvert;
    return m;
}

void
freeMeshobj( Meshobj *m )
{
    free( m );
}

/* 
 *	Vertex hashing
 *
 */
static void hashvertbegin( Meshobj *m )
{
    int i;

    m->verthashlist = (Vert **)malloc(sizeof(Vert *)*vertHashSize);
    for(i=0; i<vertHashSize; i++) 
	m->verthashlist[i] = 0;
}

static int hashvert( Meshobj *m, Vert *vert )
{
    unsigned int val;

    val = (*m->amhashvert)(vert->id);
    return (int) (val%vertHashSize);
}

static Vert *hashvertfind( Meshobj *m, Vert *vert )
{
    int pos;
    Vert *vptr;

    pos = hashvert(m,vert);
    for( vptr = m->verthashlist[pos]; vptr; vptr = vptr->hnext )
	if((*m->amvertsame)(vert->id,vptr->id)) 
	    return vptr;
    return 0;
}

static void hashvertadd( Meshobj *m, Vert* vert )
{
    int pos;

    pos = hashvert(m,vert);
    vert->hnext = m->verthashlist[pos];
    m->verthashlist[pos] = vert;
}

static void hashvertend( Meshobj *m )
{
    free(m->verthashlist);
}

/* 
 *	Triangle hashing
 *
 */
static void hashtribegin( Meshobj *m )
{
    int i;

    m->trihashlist = (Tri **)malloc(sizeof(Tri *)*triHashSize);
    for(i=0; i<triHashSize; i++) 
	m->trihashlist[i] = 0;
}

static int hashtri( Tri *tri )
{
    unsigned int val, l;

    l = (unsigned int)tri->vert[0];
    val = l;
    l = (unsigned int)tri->vert[1];
    val = val^l;
    l = (unsigned int)tri->vert[2];
    val = val^l;
    return (int) (val%triHashSize);
}

static Tri *hashtrifind( Meshobj *m, Tri *tri )
{
    int pos;
    Tri *tptr;

    pos = hashtri(tri);
    for( tptr = m->trihashlist[pos]; tptr; tptr = tptr->hnext ) 
	if(triequal(tri,tptr)) 
	    return tptr;
    return 0;
}

static void hashtriadd( Meshobj *m, Tri *tri )
{
    int pos;

    pos = hashtri(tri);
    tri->hnext = m->trihashlist[pos];
    m->trihashlist[pos] = tri;
}

static void hashtriend( Meshobj *m )
{
    free(m->trihashlist);
}

/* 
 *	Edge hashing
 *
 */
static void hashedgebegin( Meshobj *m, int np )
{
    int i;

    m->edgehashlist = (Edge **)malloc(sizeof(Edge *)*edgeHashSize);
    m->freeedges = (Edge *)malloc(sizeof(Edge)*3*np);
    m->edgearray = m->freeedges;
    for(i=0; i<edgeHashSize; i++) 
	m->edgehashlist[i] = 0;
}

static int hashedge( unsigned int index0, unsigned int index1 )
{
    unsigned int val;

    val = index0*index1;
    val = val&0x7fffffff;
    return (int) (val%edgeHashSize);
}

static Edge *hashedgefind( Meshobj *m, Vert *v0, Vert *v1 )
{
    int pos;
    unsigned int index0, index1;
    Edge *tptr;

    index0 = v0->index;
    index1 = v1->index;
    pos = hashedge(index0,index1);
    tptr = m->edgehashlist[pos];
    return tptr;
}

static void hashedgeadd( Meshobj *m, Tri *tri, Vert *v0, Vert *v1 )
{
    int pos;
    unsigned int index0, index1;
    Edge *edge;

    index0 = v0->index;
    index1 = v1->index;
    pos = hashedge(index0,index1);
    edge = m->freeedges++;
    edge->index0 = index0;
    edge->index1 = index1;
    edge->tri = tri;
    edge->hnext = m->edgehashlist[pos];
    m->edgehashlist[pos] = edge;
}

static void hashedgeend( Meshobj *m )
{
    free(m->edgehashlist);
    free(m->edgearray);
}

static Vertlist *makevertlist( void ) 
{
    /* allocate space for and initialize a vert list */
    Vertlist *tmp;

    tmp = (Vertlist *)malloc(sizeof(Vertlist));
    tmp->first = tmp->last = 0;
    return tmp;
}

static void freevertlist( Vertlist *vl )
{
    Vert *vert, *nvert = NULL;

    for( vert = vl->first; vert; vert = nvert ) {
	nvert = vert->next;
	freevert(vert);
    }
    free(vl);
}

static Trilist *maketrilist( void ) 
{
    /* allocate space for and initialize a tri list */
    Trilist *tmp;

    tmp = (Trilist *)malloc(sizeof(Trilist));
    tmp->first = tmp->last = 0;
    return tmp;
}

static void freetrilist( Trilist *tl )
{
    Tri *tri, *ntri = NULL;

    for( tri = tl->first; tri; tri = ntri ) {
	ntri = tri->next;
	freetri(tri);
    }
    free(tl);
}

static Vert *makevert( void ) 
{
    /* allocate space for and initialize a vert */
    Vert *tmp;

    tmp = (Vert *)malloc(sizeof(Vert));
    tmp->prev = tmp->next = 0;
    tmp->index = 0;
    return tmp;
}

static void freevert( Vert *v )
{
    free(v);
}

static Tri *maketri( void ) 
{
    /* allocate space for and initialize a tri */
    Tri *tmp;
    register int i;

    tmp = (Tri *)malloc(sizeof(Tri));
    tmp->prev = tmp->next = 0;
    for (i=0; i<3; i++) {
	tmp->adj[i] = 0;
	tmp->vert[i] = 0;
    }
    tmp->drawvert[0] = 0;
    tmp->drawvert[1] = 1;
    tmp->drawvert[2] = 2;
    tmp->adjcount = 0;
    tmp->used = false;
    return tmp;
}

static void freetri( Tri *tri )
{
    free(tri);
}

static void insertvert( Vertlist *list, Vert *item )
{
    /* insert the new item at the top of the list */
    if (list->first) {
	item->next = list->first;
	item->prev = 0;
	item->next->prev = item;
	list->first = item;
    } else {
	list->first = list->last = item;
	item->prev = item->next = 0;
    }
}

static void inserttri( Trilist *list, Tri *item )
{
    /* insert the new item at the top of the list */
    if (list->first) {
	item->next = list->first;
	item->prev = 0;
	item->next->prev = item;
	list->first = item;
    } else {
	list->first = list->last = item;
	item->prev = item->next = 0;
    }
}

static void deletetri( Trilist *list, Tri *item )
{
    /* delete the item from the list */
    if (list->first == item) {
	if (list->last == item) {
	    /* this is the only item in the list */
	    list->first = list->last = 0;
	} else {
	    /* this is the first item in the list */
	    list->first = item->next;
	    list->first->prev = 0;
	}
    } else if (list->last == item) {
	/* this is the last item in the list */
	list->last = item->prev;
	list->last->next = 0;
    } else {
	item->prev->next = item->next;
	item->next->prev = item->prev;
    }
    item->prev = item->next = 0;
}

static int triequal( Tri *tri1, Tri *tri2 ) 
{
    int i, j, k;

    k = 0;
    for (i=0; i<3; i++) {
	for (j=0; j<3; j++) {
	    if (tri1->vert[i] == tri2->vert[j])
		k += 1;
	}
    }
    if (k == 3) 
	return 1;
    else 
	return 0;
}

static void makelists( Meshobj *m )
{
    int i;

    m->vertlist = makevertlist();
    m->trilist = maketrilist();
    m->newtrilist = maketrilist();
    m->duptrilist = maketrilist();
    m->donetrilist = maketrilist();
    for (i=0; i<4; i++)
	m->adjtrilist[i] = maketrilist();
}

static void freelists( Meshobj *m )
{
    int i;

    freevertlist(m->vertlist);
    freetrilist(m->trilist);
    freetrilist(m->newtrilist);
    freetrilist(m->duptrilist);
    freetrilist(m->donetrilist);
    for (i=0; i<4; i++)
	freetrilist(m->adjtrilist[i]);
}

/*
 *	actual exported routines
 *
 *
 */
void in_ambegin( Meshobj *m, int nverts )
{

    makelists(m);

   /* estimate hash table sizes */
   if (nverts < TABLE_SIZE1) {
     vertHashSize = TABLE_SIZE1;
     edgeHashSize = TABLE_SIZE0;
     triHashSize  = TABLE_SIZE0;
   } else if (nverts < TABLE_SIZE2) {
     vertHashSize = TABLE_SIZE2;
     edgeHashSize = TABLE_SIZE1;
     triHashSize  = TABLE_SIZE1;
   } else if (nverts < TABLE_SIZE3) {
     vertHashSize = TABLE_SIZE3;
     edgeHashSize = TABLE_SIZE2;
     triHashSize  = TABLE_SIZE2;
   } else if (nverts < TABLE_SIZE4) {
     vertHashSize = TABLE_SIZE4;
     edgeHashSize = TABLE_SIZE3;
     triHashSize  = TABLE_SIZE3;
   } else if (nverts < TABLE_SIZE5) {
     vertHashSize = TABLE_SIZE5;
     edgeHashSize = TABLE_SIZE4;
     triHashSize  = TABLE_SIZE4;
   } else if (nverts < TABLE_SIZE6) {
     vertHashSize = TABLE_SIZE6;
     edgeHashSize = TABLE_SIZE5;
     triHashSize  = TABLE_SIZE4;
   } else if (nverts < TABLE_SIZE7) {
     vertHashSize = TABLE_SIZE7;
     edgeHashSize = TABLE_SIZE6;
     triHashSize  = TABLE_SIZE5;
   } else {
     vertHashSize = TABLE_SIZE8;
     edgeHashSize = TABLE_SIZE6;
     triHashSize  = TABLE_SIZE5;
   }

    m->vertcount = 0;
    hashvertbegin( m );
    m->tmpvert = makevert();
    m->npolys = 0;
    m->vertno = 0;
}


void in_amnewtri( Meshobj *m, int tId )
{
    m->vertno = 0;
    m->curtri = maketri();
    m->curtri->id = tId; 
    inserttri(m->trilist,m->curtri);
    m->npolys++;
}

void in_amvert( Meshobj *m, unsigned int fptr )
{
    Vert *vert;

    if(m->vertno > 2) {
	fprintf(stderr,"in_amvert: can't have more than 3 verts in triangle\n");
	return;
    }
    m->curtri->vert[m->vertno] = 0;
    m->tmpvert->id = fptr;
    vert = hashvertfind(m,m->tmpvert);
    if(vert)
    {
	m->curtri->vert[m->vertno] = vert;
    }
    else  {
	hashvertadd(m,m->tmpvert);
/* add a new vertex to the list */
	m->tmpvert->index = m->vertcount;
	m->vertcount++;
	m->curtri->vert[m->vertno] = m->tmpvert;
	insertvert(m->vertlist,m->tmpvert);
	m->tmpvert = makevert();
    }
    m->vertno++;
}

int in_amend( Meshobj *m )
{
    Vert *vert0,*vert1;
    register Tri *tri;
    register Tri *tmptri;
    register Tri *nexttri;
    register Edge *edge;
    int i, j, k;
    int degeneratecount;
    int equivalentcount;
    int count;
    int adjcount[4];
    int adjnumber;

    freevert(m->tmpvert);
    if(m->vertno != 3) {
	(void) fprintf(stderr,"in_amend: incomplete triangle\n");
	return 0;
    }
    hashvertend(m);
    dprintf(stderr,"%d triangles read\n",m->npolys);
    dprintf(stderr,"%d vertexes created\n",m->vertcount);

    beginoutputobj(m,m->vertlist->first,m->trilist->first);

/*** eliminate degenerate triangles ***/
    dprintf(stderr,"eliminating degenerate triangles\n");
    degeneratecount = 0;
    for (tri=m->trilist->first; tri;) {
	if ((tri->vert[0] == tri->vert[1]) ||
	    (tri->vert[0] == tri->vert[2]) ||
	    (tri->vert[1] == tri->vert[2])) {
	    degeneratecount += 1;
	    tmptri = tri->next;
	    deletetri(m->trilist,tri);
	    freetri(tri);
	    tri = tmptri;
	} else 
	    tri = tri->next;
    }
    dprintf(stderr,"%d degenerate triangles eliminated\n",degeneratecount);

/*** eliminate equivalent triangles ***/
    dprintf(stderr,"eliminating equivalent triangles\n");
    count = 0;
    equivalentcount = 0;

    hashtribegin(m);
    for (tri=m->trilist->first; tri;) {
	count += 1;
	if(hashtrifind(m,tri)) {
	    equivalentcount += 1;
	    nexttri = tri->next;
	    deletetri(m->trilist,tri);
            inserttri(m->duptrilist,tri);
	    tri = nexttri;
	} else {
	    hashtriadd(m,tri);
	    tri = tri->next;
	}
    }
    hashtriend(m);
    dprintf(stderr,"%d equivalent triangles eliminated\n",equivalentcount);

/*** compute triangle adjacencies ***/
    dprintf(stderr,"computing adjacent triangles\n");
    hashedgebegin(m,m->npolys);
    dprintf(stderr,"adding to hash table . . ");
    for (tri=m->trilist->first; tri; tri=tri->next) {
	for(i=0; i<3; i++) {
	    vert0 = tri->vert[(i+0)%3];
	    vert1 = tri->vert[(i+1)%3];
	    hashedgeadd(m,tri,vert0,vert1);
	}
    }
    dprintf(stderr,"done\n");
    count = 0;
    for (tri=m->trilist->first; tri; tri=tri->next) {
	count += 1;
	for (i=0; i<3; i++) {
	    if (tri->adj[i])
		continue;
	    vert0 = tri->vert[i];
	    vert1 = tri->vert[(i+1)%3];
	    for (edge=hashedgefind(m,vert0,vert1); edge; edge=edge->hnext) {
		nexttri = edge->tri;
		if(nexttri == tri) 
		    continue;
		for (j=0; j<3; j++) {
		    if (vert0 == nexttri->vert[j]) {
			for (k=0; k<3; k++) {
			    if (k==j)
				continue;
			    if (vert1 == nexttri->vert[k]) {
				switch (j+k) {
				    case 1:
					adjnumber = 0;
					break;
				    case 2:
					adjnumber = 2;
					break;
				    case 3:
					adjnumber = 1;
					break;
				    default:
					(void) fprintf(stderr,
					    "ERROR: bad adjnumber\n");
					break;
				}
				if (tri->adj[i]||nexttri->adj[adjnumber]) {
				} else {	
				    tri->adj[i] = nexttri;
				    nexttri->adj[adjnumber] = tri;
				}
			    }
			}
		    }
		}
	    }
	}
    }
    hashedgeend(m);
    dprintf(stderr," done\n");

/*** test adjacency pointers for consistency ***/
    for (tri=m->trilist->first; tri; tri=tri->next) {
	for (i=0; i<3; i++) {
	    if (nexttri = tri->adj[i]) {
		for (j=0,k=0; j<3; j++) {
		    if (tri == nexttri->adj[j])
			k += 1;
		}
		if (k != 1) {
		    (void) fprintf(stderr," ERROR: %x to %x k = %d\n",tri,nexttri,k);
		}
	    }
	}
    }

/*** compute adjacency statistics ***/
    for (i=0; i<4; i++)
	adjcount[i] = 0;
    for (tri=m->trilist->first; tri;) {
	for (i=0,count=0; i<3; i++) {
	    if (tri->adj[i])
		count += 1;
	}
	tri->adjcount = count;
	adjcount[count] += 1;
	nexttri = tri->next;
	deletetri(m->trilist,tri);
	inserttri(m->adjtrilist[count],tri);
	tri = nexttri;
    }
    dprintf(stderr,"adjacencies: 0:%d, 1:%d, 2:%d, 3:%d\n",
	adjcount[0],adjcount[1],adjcount[2],adjcount[3]);

/*** search for connected triangles and output ***/

    /* output dup-tris as independent tris */
    makered = 0;
    while (tri = m->duptrilist->first) {
        deletetri(m->duptrilist,tri);
        inserttri(m->newtrilist,tri);
        tri->used = true;
        outmesh(m,m->newtrilist);
        dprintf(stderr,"outputted equivalent triangle\n");
    }
    makered = 0;
  
    while (1) {
    /*** output singular triangles, if any ***/
	while (tri = m->adjtrilist[0]->first) {
	    deletetri(m->adjtrilist[0],tri);
	    inserttri(m->newtrilist,tri);
	    tri->used = true;
	    outmesh(m,m->newtrilist);
	}
    /*** choose a seed triangle with the minimum number of adjacencies ***/
	if (tri = m->adjtrilist[1]->first)
	    deletetri(m->adjtrilist[1],tri);
	else if (tri = m->adjtrilist[2]->first)
	    deletetri(m->adjtrilist[2],tri);
	else if (tri = m->adjtrilist[3]->first)
	    deletetri(m->adjtrilist[3],tri);
	else
	    break;
	inserttri(m->newtrilist,tri);
	removeadjacencies(m,tri);

    /*** extend in one direction using triangles with min adjacencies ***/
	while (tri = minadj(tri)) {
	    deletetri(m->adjtrilist[tri->adjcount],tri);
	    inserttri(m->newtrilist,tri);
	    removeadjacencies(m,tri);
	}

    /*** if seed has two or more adjacencies, extend in other direction **/
	tri = m->newtrilist->first;
	nexttri = 0;
	for (i=0; i<3; i++) {
	    if (tri->adj[i] &&
		(tri->adj[i] != tri->next) &&
		(!(tri->adj[i]->used))) {
		nexttri = tri->adj[i];
		break;
	    }
	}
	for( tri = nexttri; tri; tri=minadj(tri) ) {
	    deletetri(m->adjtrilist[tri->adjcount],tri);
	    inserttri(m->newtrilist,tri);
	    removeadjacencies(m,tri);
	}

    /*** output the resulting mesh ***/
	outmesh(m,m->newtrilist);
    }
    if (m->connectcount)
	dprintf(stderr,"%d triangle mesh%s output\n",
	    m->connectcount,m->connectcount==1?"":"es");
    if (m->independentcount)
	dprintf(stderr,"%d independent triangle%s output\n",
	    m->independentcount,m->independentcount==1?"":"s");
    (*m->amend)();
    freelists(m);
    return 1;
}

static int ismember( Vert *vert, Tri *tri )
{
    /*** return true if vert is one of the vertexes in tri, otherwise false ***/
    register int i;

    for (i=0; i<3; i++)
	if (vert == tri->vert[i])
	    return true;
    return false;
}

static int notcommon( Tri *tri, Tri *tri2 )
{
    /*** returns the index of the vertex in tri that is not in tri2 ***/
    int i;

    for (i=0; i<3; i++)
	if (!ismember(tri->vert[i],tri2))
	    return i;
    return -1;
}

static void outmesh( Meshobj *m, Trilist *tris )
{
    Tri *tri;
    int i;
    Vert *vert[2];
    Vert *nextvert;
    int nextid;
    bool replaceB;

    /*** output trilist - transfer to donelist ***/
    tri = tris->first;
    if (tri == tris->last) {
	/*** there is only one triangle in the mesh - use polygon command ***/
	m->independentcount += 1;
	(*m->ambgntmesh)();
	(*m->amvert)((tri->id*3)+0);
	(*m->amvert)((tri->id*3)+1);
	(*m->amvert)((tri->id*3)+2);
	(*m->amendtmesh)();
	deletetri(tris,tri);
	inserttri(m->donetrilist,tri);
    } else {
	/*** a real mesh output is required ***/
	m->connectcount += 1;
	/*** start output with vertex that is not in the second triangle ***/
	i = notcommon(tri,tri->next);
	INITMESH;
	(*m->ambgntmesh)();

	(*m->amvert)((tri->id*3)+i);

	REPLACEVERT(tri->vert[i]);
	(*m->amvert)((tri->id*3)+((i+1)%3));

	REPLACEVERT(tri->vert[(i+1)%3]);
	(*m->amvert)((tri->id*3)+((i+2)%3));

	REPLACEVERT(tri->vert[(i+2)%3]);
	/*** compute vertex of second triangle that is not in the first ***/

	i = notcommon(tri->next,tri);
	nextvert = (tri->next)->vert[i];
	nextid   = ((tri->next)->id*3) + i;
	/*** transfer triangle to done list ***/
	deletetri(tris,tri);
	inserttri(m->donetrilist,tri);
	for( tri = tris->first; tri->next; tri=tris->first ) {
	    /*** check for errors ***/
	    if ((!ismember(vert[0],tri)) || (!ismember(vert[1],tri)) ||
		(!ismember(nextvert,tri))) {
		(void) fprintf(stderr,"ERROR in mesh generation\n");
	    }
	    if ((vert[0] == vert[1]) || (vert[0] == nextvert)) {
		(void) fprintf(stderr,"ERROR in mesh generation\n");
	    }
	    /*** decide whether to swap or not ***/
	    if (ismember(vert[replaceB],tri->next)) {
		(*m->amswaptmesh)();
		SWAPMESH;
	    }
	    /*** output the next vertex ***/

	    (*m->amvert)(nextid);
	    REPLACEVERT(nextvert);

	    /*** determine the next output vertex ***/
	    i = notcommon(tri->next,tri);
	    nextvert = (tri->next)->vert[i];   
	    nextid   = ((tri->next)->id*3) + i;
	    /*** transfer tri to the done list ***/
	    deletetri(tris,tri);
	    inserttri(m->donetrilist,tri);
	}
	/*** output the last vertex ***/
	(*m->amvert)(nextid);
	REPLACEVERT(nextvert);
	deletetri(tris,tri);
	inserttri(m->donetrilist,tri);
	(*m->amendtmesh)();
    }
}

static void removeadjacencies( Meshobj *m, Tri *tri )
{
    register int i,j;
    Tri *adjtri;

    tri->used = true;
    for (i=0; i<3; i++) {
	adjtri = tri->adj[i];
	if (adjtri) {
	    deletetri(m->adjtrilist[adjtri->adjcount],adjtri);
	    adjtri->adjcount -= 1;
	    for (j=0; j<3; j++) {
		if (tri == adjtri->adj[j]) {
		    adjtri->adj[j] = 0;
#ifdef I_THINK_THIS_IS_A_BUG
		    freetri(adjtri);
#endif
		    break;
		}
	    }
	    inserttri(m->adjtrilist[adjtri->adjcount],adjtri);
	}
    }
}

static Tri *minadj( Tri *tri )
{
    int min0,min1;

    switch (tri->adjcount) {
	case 0:
	    return 0;
	case 1:
	    if (tri->adj[0])
		return tri->adj[0];
	    else if (tri->adj[1])
		return tri->adj[1];
	    else
		return tri->adj[2];
	case 2:
	    if (tri->adj[0]) {
		min0 = 0;
		if (tri->adj[1])
		    min1 = 1;
		else
		    min1 = 2;
	    } else {
		min0 = 1;
		min1 = 2;
	    }
	    if ((tri->adj[min0])->adjcount <= (tri->adj[min1])->adjcount)
		return tri->adj[min0];
	    else
		return tri->adj[min1];
	case 3:
	default:
	    if ((tri->adj[0])->adjcount <= (tri->adj[1])->adjcount)
		min0 = 0;
	    else
		min0 = 1;
	    min1 = 2;
	    if ((tri->adj[min0])->adjcount <= (tri->adj[min1])->adjcount)
		return tri->adj[min0];
	    else
		return tri->adj[min1];
    }
}

static void beginoutputobj( Meshobj *m, Vert *verts, Tri *tris ) 
{
    Vert *vert;
    Tri *tri;
    int nverts, ntris;
    int i;

/* count vertices and triangles */
    nverts = 0;
    for( vert = verts; vert; vert = vert->next )
	nverts++;

    ntris = 0;
    for( tri = tris; tri; tri = tri->next )
	ntris++;

    dprintf(stderr,"makeoutput: vertexes in %d out %d\n",m->npolys*3,nverts);
    dprintf(stderr,"makeoutput: tris     in %d out %d\n",m->npolys,ntris);
    (*m->ambegin)(nverts,ntris);

    for( i=0, vert = verts; vert; vert = vert->next, i++ ) {
	vert->index = i;
	(*m->amvertdata)(vert->id);
    }
}
