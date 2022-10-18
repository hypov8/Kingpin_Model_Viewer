#ifndef INCLUDED_MDX
#define INCLUDED_MDX

#define MDX_MAX_TRIANGLES		4096
#define MDX_MAX_VERTICES		2048
#define MDX_MAX_FRAMES			1028
#define MDX_MAX_SKINS			32
#define MDX_MAX_FRAMESIZE		(MDX_MAX_VERTICES * 4 + 128)


typedef struct 
{ 
   int magic; 
   int version; 
   int skinWidth; 
   int skinHeight; 
   int frameSize; 

   int numSkins; 
   int numVertices;
   int numTriangles; 
   int numGlCommands; 
   int numFrames; 
   int numSfxDefines;
   int numSfxEntries;
   int numSubObjects;

   int offsetSkins; 
   int offsetTriangles; 
   int offsetFrames; 
   int offsetGlCommands;
   int offsetVertexInfo; 
   int offsetSfxDefines; 
   int offsetSfxEntries; 
   int offsetBBoxFrames; 
   int offsetDummyEnd; 
   int offsetEnd; 
} mdx_header_t;

typedef struct
{
   byte vertex[3];
   byte lightNormalIndex;
} mdx_alias_triangleVertex_t; //md2 compat

typedef struct
{
   float vertex[3]; //xyz pos
   float normal[3]; //xyz direction
} mdx_triangleVertex_t; //md2 compat

typedef struct
{
   short vertexIndices[3];
   short textureIndices[3];
} mdx_triangle_t; //md2 compat

typedef struct
{
   float scale[3];
   float translate[3];
   char name[16];
   mdx_alias_triangleVertex_t alias_vertices[1];
} mdx_alias_frame_t; //md2 compat

typedef struct
{
	float scale[3]; //hypov8
	float translate[3]; //hypov8
	char name[16];
	mdx_triangleVertex_t *vertices;
} mdx_frame_t; //md2 compat

typedef char mdx_skin_t[64];

typedef struct
{
	float s;
	float t;
   int vertexIndex;
} mdx_glCommandVertex_t; //md2 compat

typedef struct
{
	mdx_header_t			header;
	mdx_skin_t				*skins;
	mdx_triangle_t			*triangles;
	mdx_frame_t				*frames;
	byte					*framesBuffer;
	int						*glCommandBuffer;
	int						isMD2;
	float					min[3];
	float					max[3];
	float					*hitBox;
} mdx_model_t;

#ifdef __cplusplus
extern "C" {
#endif

mdx_model_t *mdx_readModel (const char *filename, int debugLoad);
void mdx_freeModel (mdx_model_t *model);
void mdx_setStyle (int glcmds, int interp);
void mdx_getBoundingBox (mdx_model_t *model, float *minmax, int frame);
void mdx_getBoundingBoxExport (mdx_model_t *model, float *minmax, int frame);
void mdx_drawModel (mdx_model_t *model, int frame1, int frame2, float pol, int noLerp, int isMissingFrame);
//void mdx_drawModel_gl (mdx_model_t *model, int frame1, int isMissingFrame); //hypov8 add: render models without animations
void mdx_generateLightNormals (mdx_model_t *model);
int mdx_getAnimationCount (mdx_model_t *model);
const char *mdx_getAnimationName (mdx_model_t *model, int animation);
void mdx_getAnimationFrames (mdx_model_t *model, int animation, int *startFrame, int *endFrame);

//md2 compatable
void mdx_makeFacetNormal(mdx_frame_t *frame, mdx_triangle_t *t, int fIdx, float *fn);
void mdx_normalize(float *n);

//HYPOVERTEX
void mdx_drawModel_dev(mdx_model_t *model, int frame1, int frame2, float pol, int lerp, int vertID, int useFace, int showVN, int showGrid, int showBBox, float *rgb, float *grid_rgb);
int mdx_readFrameData(FILE *file, mdx_frame_t **frames, byte *buffer, int numFrames, int numVertices, int frameSize, float*min, float*max, int offsetFrames, byte **framesBuffer);
//END

#ifdef __cplusplus
}
#endif



#endif /* INCLUDED_MDX */
