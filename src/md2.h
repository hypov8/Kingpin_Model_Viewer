
#ifndef INCLUDED_MD2
#define INCLUDED_MD2

typedef struct 
{ 
   int magic; 
   int version; 
   int skinWidth; 
   int skinHeight; 
   int frameSize; 

   int numSkins; 
   int numVertices;
   int numTexCoords; 
   int numTriangles; 
   int numGlCommands; 
   int numFrames; 

   int offsetSkins; 
   int offsetTexCoords; 
   int offsetTriangles; 
   int offsetFrames; 
   int offsetGlCommands; 
   int offsetEnd; 
} md2_header_t;


typedef struct
{
	short s;
	short t;
} md2_textureCoordinate_t; //md2 only. software tex cords(per pixel)

typedef struct
{
	md2_header_t			header;
	mdx_skin_t				*skins;
	md2_textureCoordinate_t	*texCoords;
	mdx_triangle_t			*triangles;
	mdx_frame_t				*frames;
	byte					*framesBuffer;
	int						*glCommandBuffer;
	int						isMD2;
	float					bBoxMin[3];
	float					bBoxMax[3];
} md2_model_t;


#ifdef __cplusplus
extern "C" {
#endif

mdx_model_t *md2_Parse_readModel(const char *filename, int debugLoad);
void md2_freeModel (md2_model_t *model);

#ifdef __cplusplus
}
#endif



#endif /* INCLUDED_MD2 */
