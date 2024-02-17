#ifndef INCLUDED_COMMON
#define INCLUDED_COMMON


#include "mdx.h"
#include "md2.h"


#if 1 //def KINGPIN_MDX_V5
#define MDL_MAX_TRIANGLES		(4096*2)
#define MDL_MAX_VERTICES		(2048*2)
#define MDL_MAX_FRAMES			1028
#define MDL_MAX_SKINS			32
#define MDL_MAX_FRAMESIZE		(MDL_MAX_VERTICES * 7 + 128)
#else
#define MDL_MAX_TRIANGLES		4096
#define MDL_MAX_VERTICES		2048
#define MDL_MAX_FRAMES			1028
#define MDL_MAX_SKINS			32
#define MDL_MAX_FRAMESIZE		(MDL_MAX_VERTICES * 4 + 128)
#endif

#define NUMVERTEXNORMALS 162

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