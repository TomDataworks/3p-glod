/*

The interface routines for reading and writing PLY polygon files.

Greg Turk, February 1994

---------------------------------------------------------------

A PLY file contains a single polygonal _object_.

An object is composed of lists of _elements_.  Typical elements are
vertices, faces, edges and materials.

Each type of element for a given object has one or more _properties_
associated with the element type.  For instance, a vertex element may
have as properties the floating-point values x,y,z and the three unsigned
chars representing red, green and blue.

---------------------------------------------------------------

Copyright (c) 1994 The Board of Trustees of The Leland Stanford
Junior University.  All rights reserved.   
  
Permission to use, copy, modify and distribute this software and its   
documentation for any purpose is hereby granted without fee, provided   
that the above copyright notice and this permission notice appear in   
all copies of this software and that you do not sell the software.   
  
THE SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,   
EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY   
WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.   

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "ply.h"

char *type_names[] = {
    "invalid",
    "char", "short", "int",
    "uchar", "ushort", "uint",
    "float", "double",
};

char *new_type_names[] = 
{
    "invalid",
    "int8", "int16", "int32",
    "uint8", "uint16", "uint32",
    "float32", "float64",
};

int ply_type_size[] = {
    0, 1, 2, 4, 1, 2, 4, 4, 8
};

typedef union
{
    int  int_value;
    char byte_values[sizeof(int)];
} endian_test_type;


static int native_binary_type = -1;
static int types_checked = 0;

#define NO_OTHER_PROPS  -1

#define DONT_STORE_PROP  0
#define STORE_PROP       1

#define OTHER_PROP       0
#define NAMED_PROP       1


/* returns 1 if strings are equal, 0 if not */
int equal_strings(char *, char *);

/* find a property in an element's list */
PlyProperty *find_property(PlyElement *, char *, int *);

/* write to a file the word describing a PLY file data type */
void write_scalar_type (FILE *, int);

/* read a line from a file and break it up into separate words */
char **get_words(FILE *, int *, char **);
char **old_get_words(FILE *, int *);

/* write an item to a file */
void write_binary_item(FILE *, int, int, unsigned int, double, int);
void write_ascii_item(FILE *, int, unsigned int, double, int);
double old_write_ascii_item(FILE *, char *, int);

/* copy a property */
void copy_property(PlyProperty *, PlyProperty *);

/* store a value into where a pointer and a type specify */
void store_item(char *, int, int, unsigned int, double);

/* return the value of a stored item */
void get_stored_item( void *, int, int *, unsigned int *, double *);

/* return the value stored in an item, given ptr to it and its type */
double get_item_value(char *, int);

/* get binary or ascii item and store it according to ptr and type */
void get_ascii_item(char *, int, int *, unsigned int *, double *);
void get_binary_item(FILE *, int, int, int *, unsigned int *, double *);

/* memory allocation */
char *my_alloc(int, int, char *);

/* byte ordering */
void get_native_binary_type();
void swap_bytes(char *, int);

void check_types();



/******************************************************************************
Make ready for "other" properties of an element-- those properties that
the user has not explicitly asked for, but that are to be stashed away
in a special structure to be carried along with the element's other
information.

Entry:
  plyfile - file identifier
  elem    - element for which we want to save away other properties
******************************************************************************/

void setup_other_props(PlyElement *elem)
{
    int i;
    PlyProperty *prop;
    int size = 0;
    int type_size;

    /* Examine each property in decreasing order of size. */
    /* We do this so that all data types will be aligned by */
    /* word, half-word, or whatever within the structure. */

    for (type_size = 8; type_size > 0; type_size /= 2) {

        /* add up the space taken by each property, and save this information */
        /* away in the property descriptor */

        for (i = 0; i < elem->nprops; i++) {

            /* don't bother with properties we've been asked to store explicitly */
            if (elem->store_prop[i])
                continue;

            prop = elem->props[i];

            /* internal types will be same as external */
            prop->internal_type = prop->external_type;
            prop->count_internal = prop->count_external;

            /* check list case */
            if (prop->is_list) {

                /* pointer to list */
                if (type_size == sizeof (void *)) {
                    prop->offset = size;
                    size += sizeof (void *);        /* always use size of a pointer here */
                }

                /* count of number of list elements */
                if (type_size == ply_type_size[prop->count_external]) {
                    prop->count_offset = size;
                    size += ply_type_size[prop->count_external];
                }
            }
            /* not list */
            else if (type_size == ply_type_size[prop->external_type]) {
                prop->offset = size;
                size += ply_type_size[prop->external_type];
            }
        }

    }

    /* save the size for the other_props structure */
    elem->other_size = size;
}


/******************************************************************************
Free up storage used by an "other" elements data structure.

Entry:
  other_elems - data structure to free up
******************************************************************************/

void ply_free_other_elements (PlyOtherElems *other_elems)
{
    other_elems = other_elems;
}



/*******************/
/*      Miscellaneous  */
/*******************/


/******************************************************************************
Compare two strings.  Returns 1 if they are the same, 0 if not.
******************************************************************************/

int equal_strings(char *s1, char *s2)
{

    while (*s1 && *s2)
        if (*s1++ != *s2++)
            return (0);

    if (*s1 != *s2)
        return (0);
    else
        return (1);
}


/******************************************************************************
Find a property in the list of properties of a given element.

Entry:
  elem          - pointer to element in which we want to find the property
  prop_name - name of property to find

Exit:
  index - index to position in list
  returns a pointer to the property, or NULL if not found
******************************************************************************/

PlyProperty *find_property(PlyElement *elem, char *prop_name, int *index)
{
    int i;

    for (i = 0; i < elem->nprops; i++)
        if (equal_strings (prop_name, elem->props[i]->name)) {
            *index = i;
            return (elem->props[i]);
        }

    *index = -1;
    return (NULL);
}


/******************************************************************************
  Reverse the order in an array of bytes.  This is the conversion from big
  endian to little endian and vice versa

Entry:
  bytes         - array of bytes to reverse (in place)
  num_bytes - number of bytes in array
******************************************************************************/

void swap_bytes(char *bytes, int num_bytes)
{
    int i;
    char temp;
        
    for (i=0; i < num_bytes/2; i++)
    {
        temp = bytes[i];
        bytes[i] = bytes[(num_bytes-1)-i];
        bytes[(num_bytes-1)-i] = temp;
    }
}

/******************************************************************************
  Find out if this machine is big endian or little endian

  Exit:
        set global variable, native_binary_type =
                                                          either PLY_BINARY_BE or PLY_BINARY_LE

******************************************************************************/

void get_native_binary_type()
{
    endian_test_type test;

    test.int_value = 0;
    test.int_value = 1;
    if (test.byte_values[0] == 1)
        native_binary_type = PLY_BINARY_LE;
    else if (test.byte_values[sizeof(int)-1] == 1)
        native_binary_type = PLY_BINARY_BE;
    else
    {
        fprintf(stderr, "ply: Couldn't determine machine endianness.\n");
        fprintf(stderr, "ply: Exiting...\n");
        exit(1);
    }
}

/******************************************************************************
  Verify that all the native types are the sizes we need


******************************************************************************/

void check_types()
{
    if ((ply_type_size[PLY_CHAR] != sizeof(char)) ||
        (ply_type_size[PLY_SHORT] != sizeof(short)) ||  
        (ply_type_size[PLY_INT] != sizeof(int)) ||      
        (ply_type_size[PLY_UCHAR] != sizeof(unsigned char)) ||  
        (ply_type_size[PLY_USHORT] != sizeof(unsigned short)) ||        
        (ply_type_size[PLY_UINT] != sizeof(unsigned int)) ||    
        (ply_type_size[PLY_FLOAT] != sizeof(float)) ||  
        (ply_type_size[PLY_DOUBLE] != sizeof(double)))
    {
        fprintf(stderr, "ply: Type sizes do not match built-in types\n");
        fprintf(stderr, "ply: Exiting...\n");
        exit(1);
    }
        
    types_checked = 1;
}

/******************************************************************************
Return the value of an item, given a pointer to it and its type.

Entry:
  item - pointer to item
  type - data type that "item" points to

Exit:
  returns a double-precision float that contains the value of the item
******************************************************************************/

double get_item_value(char *item, int type)
{
    unsigned char *puchar;
    char *pchar;
    short int *pshort;
    unsigned short int *pushort;
    int *pint;
    unsigned int *puint;
    float *pfloat;
    double *pdouble;
    int int_value;
    unsigned int uint_value;
    double double_value;

    switch (type) {
        case PLY_CHAR:
            pchar = (char *) item;
            int_value = *pchar;
            return ((double) int_value);
        case PLY_UCHAR:
            puchar = (unsigned char *) item;
            int_value = *puchar;
            return ((double) int_value);
        case PLY_SHORT:
            pshort = (short int *) item;
            int_value = *pshort;
            return ((double) int_value);
        case PLY_USHORT:
            pushort = (unsigned short int *) item;
            int_value = *pushort;
            return ((double) int_value);
        case PLY_INT:
            pint = (int *) item;
            int_value = *pint;
            return ((double) int_value);
        case PLY_UINT:
            puint = (unsigned int *) item;
            uint_value = *puint;
            return ((double) uint_value);
        case PLY_FLOAT:
            pfloat = (float *) item;
            double_value = *pfloat;
            return (double_value);
        case PLY_DOUBLE:
            pdouble = (double *) item;
            double_value = *pdouble;
            return (double_value);
        default:
            fprintf (stderr, "get_item_value: bad type = %d\n", type);
            exit (-1);
    }
}


/******************************************************************************
Get the value of an item that is in memory, and place the result
into an integer, an unsigned integer and a double.

Entry:
  ptr  - pointer to the item
  type - data type supposedly in the item

Exit:
  int_val        - integer value
  uint_val       - unsigned integer value
  double_val - double-precision floating point value
******************************************************************************/

void get_stored_item(
    void *ptr,
    int type,
    int *int_val,
    unsigned int *uint_val,
    double *double_val
    )
{
    switch (type) {
        case PLY_CHAR:
            *int_val = *((char *) ptr);
            *uint_val = *int_val;
            *double_val = *int_val;
            break;
        case PLY_UCHAR:
            *uint_val = *((unsigned char *) ptr);
            *int_val = *uint_val;
            *double_val = *uint_val;
            break;
        case PLY_SHORT:
            *int_val = *((short int *) ptr);
            *uint_val = *int_val;
            *double_val = *int_val;
            break;
        case PLY_USHORT:
            *uint_val = *((unsigned short int *) ptr);
            *int_val = *uint_val;
            *double_val = *uint_val;
            break;
        case PLY_INT:
            *int_val = *((int *) ptr);
            *uint_val = *int_val;
            *double_val = *int_val;
            break;
        case PLY_UINT:
            *uint_val = *((unsigned int *) ptr);
            *int_val = *uint_val;
            *double_val = *uint_val;
            break;
        case PLY_FLOAT:
            *double_val = *((float *) ptr);
            *int_val = (int) *double_val;
            *uint_val = (unsigned int) *double_val;
            break;
        case PLY_DOUBLE:
            *double_val = *((double *) ptr);
            *int_val = (int) *double_val;
            *uint_val = (unsigned int) *double_val;
            break;
        default:
            fprintf (stderr, "get_stored_item: bad type = %d\n", type);
            exit (-1);
    }
}


/******************************************************************************
Extract the value of an item from an ascii word, and place the result
into an integer, an unsigned integer and a double.

Entry:
  word - word to extract value from
  type - data type supposedly in the word

Exit:
  int_val        - integer value
  uint_val       - unsigned integer value
  double_val - double-precision floating point value
******************************************************************************/

void get_ascii_item(
    char *word,
    int type,
    int *int_val,
    unsigned int *uint_val,
    double *double_val
    )
{
    switch (type) {
        case PLY_CHAR:
        case PLY_UCHAR:
        case PLY_SHORT:
        case PLY_USHORT:
        case PLY_INT:
            *int_val = atoi (word);
            *uint_val = (unsigned int) *int_val;
            *double_val = (double) *int_val;
            break;

        case PLY_UINT:
            *uint_val = strtol (word, (char **) NULL, 10);
            *int_val = (int) *uint_val;
            *double_val = (double) *uint_val;
            break;

        case PLY_FLOAT:
        case PLY_DOUBLE:
            *double_val = atof (word);
            *int_val = (int) *double_val;
            *uint_val = (unsigned int) *double_val;
            break;

        default:
            fprintf (stderr, "get_ascii_item: bad type = %d\n", type);
            exit (-1);
    }
}


/******************************************************************************
Store a value into a place being pointed to, guided by a data type.

Entry:
  item           - place to store value
  type           - data type
  int_val        - integer version of value
  uint_val       - unsigned integer version of value
  double_val - double version of value

Exit:
  item - pointer to stored value
******************************************************************************/

void store_item (
    char *item,
    int type,
    int int_val,
    unsigned int uint_val,
    double double_val
    )
{
    unsigned char *puchar;
    short int *pshort;
    unsigned short int *pushort;
    int *pint;
    unsigned int *puint;
    float *pfloat;
    double *pdouble;

    switch (type) {
        case PLY_CHAR:
            *item = int_val;
            break;
        case PLY_UCHAR:
            puchar = (unsigned char *) item;
            *puchar = uint_val;
            break;
        case PLY_SHORT:
            pshort = (short *) item;
            *pshort = int_val;
            break;
        case PLY_USHORT:
            pushort = (unsigned short *) item;
            *pushort = uint_val;
            break;
        case PLY_INT:
            pint = (int *) item;
            *pint = int_val;
            break;
        case PLY_UINT:
            puint = (unsigned int *) item;
            *puint = uint_val;
            break;
        case PLY_FLOAT:
            pfloat = (float *) item;
            *pfloat = (float) double_val;
            break;
        case PLY_DOUBLE:
            pdouble = (double *) item;
            *pdouble = double_val;
            break;
        default:
            fprintf (stderr, "store_item: bad type = %d\n", type);
            exit (-1);
    }
}

/******************************************************************************
Return the type of a property, given the name of the property.

Entry:
  name - name of property type

Exit:
  returns integer code for property, or 0 if not found
******************************************************************************/

int get_prop_type(char *type_name)
{
    int i;

    for (i = PLY_START_TYPE + 1; i < PLY_END_TYPE; i++)
        if (equal_strings (type_name, type_names[i]) ||
            equal_strings (type_name, new_type_names[i]))
            return (i);

    /* if we get here, we didn't find the type */
    return (0);
}


/******************************************************************************
Copy a property.
******************************************************************************/

void copy_property(PlyProperty *dest, PlyProperty *src)
{
    dest->name = strdup (src->name);
    dest->external_type = src->external_type;
    dest->internal_type = src->internal_type;
    dest->offset = src->offset;

    dest->is_list = src->is_list;
    dest->count_external = src->count_external;
    dest->count_internal = src->count_internal;
    dest->count_offset = src->count_offset;
}


/******************************************************************************
Allocate some memory.

Entry:
  size  - amount of memory requested (in bytes)
  lnum  - line number from which memory was requested
  fname - file name from which memory was requested
******************************************************************************/

char *my_alloc(int size, int lnum, char *fname)
{
    char *ptr;

    ptr = (char *) malloc (size);

    if (ptr == 0) {
        fprintf(stderr, "Memory allocation bombed on line %d in %s\n", lnum, fname);
    }

    return (ptr);
}


/***************************************************************************
 Cleanu crud
***************************************************************************/
void ply_free_property(PlyProperty* p) {
    free(p->name);    
}

void ply_free_other_property(PlyOtherProp* p){
    int i;
    free(p->name);
    if(p->props != NULL) {
        for(i = 0; i < p->nprops; i++) {
            ply_free_property(p->props[i]);
            free(p->props[i]);
        }
        free(p->props);
    }
}

void ply_free_element(PlyElement* p) {
    int i;
    free(p->name);
    if(p->props != NULL) {
        for(i = 0; i < p->nprops; i++) {
            ply_free_property(p->props[i]);
            free(p->props[i]);
        }
        free(p->props);
        if(p->store_prop != NULL)
            free(p->store_prop);
    }
}
