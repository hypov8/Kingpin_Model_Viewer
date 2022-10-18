#ifndef INCLUDED_COMMON
#define INCLUDED_COMMON


#include "mdx.h"
#include "md2.h"

#define NUMVERTEXNORMALS 162
#define MAX_TRIANGLES 4096

#ifndef byte
typedef unsigned char byte;
#endif /* byte */


#ifdef __cplusplus
extern "C" {
#endif

float avertexnormals[NUMVERTEXNORMALS][3];

#ifdef __cplusplus
}
#endif

#endif