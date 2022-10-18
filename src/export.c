#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset */
#include <math.h> /* sqrt */
#include <mx/gl.h>
#include "common.h"
//#include "mdx.h"
//#include "md2.h"


float avertexnormals[NUMVERTEXNORMALS][3] = {
	{ -0.525731f, 0.000000f, 0.850651f },
	{ -0.442863f, 0.238856f, 0.864188f },
	{ -0.295242f, 0.000000f, 0.955423f },
	{ -0.309017f, 0.500000f, 0.809017f },
	{ -0.162460f, 0.262866f, 0.951056f },
	{ 0.000000f, 0.000000f, 1.000000f },
	{ 0.000000f, 0.850651f, 0.525731f },
	{ -0.147621f, 0.716567f, 0.681718f },
	{ 0.147621f, 0.716567f, 0.681718f },
	{ 0.000000f, 0.525731f, 0.850651f },
	{ 0.309017f, 0.500000f, 0.809017f },
	{ 0.525731f, 0.000000f, 0.850651f },
	{ 0.295242f, 0.000000f, 0.955423f },
	{ 0.442863f, 0.238856f, 0.864188f },
	{ 0.162460f, 0.262866f, 0.951056f },
	{ -0.681718f, 0.147621f, 0.716567f },
	{ -0.809017f, 0.309017f, 0.500000f },
	{ -0.587785f, 0.425325f, 0.688191f },
	{ -0.850651f, 0.525731f, 0.000000f },
	{ -0.864188f, 0.442863f, 0.238856f },
	{ -0.716567f, 0.681718f, 0.147621f },
	{ -0.688191f, 0.587785f, 0.425325f },
	{ -0.500000f, 0.809017f, 0.309017f },
	{ -0.238856f, 0.864188f, 0.442863f },
	{ -0.425325f, 0.688191f, 0.587785f },
	{ -0.716567f, 0.681718f, -0.147621f },
	{ -0.500000f, 0.809017f, -0.309017f },
	{ -0.525731f, 0.850651f, 0.000000f },
	{ 0.000000f, 0.850651f, -0.525731f },
	{ -0.238856f, 0.864188f, -0.442863f },
	{ 0.000000f, 0.955423f, -0.295242f },
	{ -0.262866f, 0.951056f, -0.162460f },
	{ 0.000000f, 1.000000f, 0.000000f },
	{ 0.000000f, 0.955423f, 0.295242f },
	{ -0.262866f, 0.951056f, 0.162460f },
	{ 0.238856f, 0.864188f, 0.442863f },
	{ 0.262866f, 0.951056f, 0.162460f },
	{ 0.500000f, 0.809017f, 0.309017f },
	{ 0.238856f, 0.864188f, -0.442863f },
	{ 0.262866f, 0.951056f, -0.162460f },
	{ 0.500000f, 0.809017f, -0.309017f },
	{ 0.850651f, 0.525731f, 0.000000f },
	{ 0.716567f, 0.681718f, 0.147621f },
	{ 0.716567f, 0.681718f, -0.147621f },
	{ 0.525731f, 0.850651f, 0.000000f },
	{ 0.425325f, 0.688191f, 0.587785f },
	{ 0.864188f, 0.442863f, 0.238856f },
	{ 0.688191f, 0.587785f, 0.425325f },
	{ 0.809017f, 0.309017f, 0.500000f },
	{ 0.681718f, 0.147621f, 0.716567f },
	{ 0.587785f, 0.425325f, 0.688191f },
	{ 0.955423f, 0.295242f, 0.000000f },
	{ 1.000000f, 0.000000f, 0.000000f },
	{ 0.951056f, 0.162460f, 0.262866f },
	{ 0.850651f, -0.525731f, 0.000000f },
	{ 0.955423f, -0.295242f, 0.000000f },
	{ 0.864188f, -0.442863f, 0.238856f },
	{ 0.951056f, -0.162460f, 0.262866f },
	{ 0.809017f, -0.309017f, 0.500000f },
	{ 0.681718f, -0.147621f, 0.716567f },
	{ 0.850651f, 0.000000f, 0.525731f },
	{ 0.864188f, 0.442863f, -0.238856f },
	{ 0.809017f, 0.309017f, -0.500000f },
	{ 0.951056f, 0.162460f, -0.262866f },
	{ 0.525731f, 0.000000f, -0.850651f },
	{ 0.681718f, 0.147621f, -0.716567f },
	{ 0.681718f, -0.147621f, -0.716567f },
	{ 0.850651f, 0.000000f, -0.525731f },
	{ 0.809017f, -0.309017f, -0.500000f },
	{ 0.864188f, -0.442863f, -0.238856f },
	{ 0.951056f, -0.162460f, -0.262866f },
	{ 0.147621f, 0.716567f, -0.681718f },
	{ 0.309017f, 0.500000f, -0.809017f },
	{ 0.425325f, 0.688191f, -0.587785f },
	{ 0.442863f, 0.238856f, -0.864188f },
	{ 0.587785f, 0.425325f, -0.688191f },
	{ 0.688191f, 0.587785f, -0.425325f },
	{ -0.147621f, 0.716567f, -0.681718f },
	{ -0.309017f, 0.500000f, -0.809017f },
	{ 0.000000f, 0.525731f, -0.850651f },
	{ -0.525731f, 0.000000f, -0.850651f },
	{ -0.442863f, 0.238856f, -0.864188f },
	{ -0.295242f, 0.000000f, -0.955423f },
	{ -0.162460f, 0.262866f, -0.951056f },
	{ 0.000000f, 0.000000f, -1.000000f },
	{ 0.295242f, 0.000000f, -0.955423f },
	{ 0.162460f, 0.262866f, -0.951056f },
	{ -0.442863f, -0.238856f, -0.864188f },
	{ -0.309017f, -0.500000f, -0.809017f },
	{ -0.162460f, -0.262866f, -0.951056f },
	{ 0.000000f, -0.850651f, -0.525731f },
	{ -0.147621f, -0.716567f, -0.681718f },
	{ 0.147621f, -0.716567f, -0.681718f },
	{ 0.000000f, -0.525731f, -0.850651f },
	{ 0.309017f, -0.500000f, -0.809017f },
	{ 0.442863f, -0.238856f, -0.864188f },
	{ 0.162460f, -0.262866f, -0.951056f },
	{ 0.238856f, -0.864188f, -0.442863f },
	{ 0.500000f, -0.809017f, -0.309017f },
	{ 0.425325f, -0.688191f, -0.587785f },
	{ 0.716567f, -0.681718f, -0.147621f },
	{ 0.688191f, -0.587785f, -0.425325f },
	{ 0.587785f, -0.425325f, -0.688191f },
	{ 0.000000f, -0.955423f, -0.295242f },
	{ 0.000000f, -1.000000f, 0.000000f },
	{ 0.262866f, -0.951056f, -0.162460f },
	{ 0.000000f, -0.850651f, 0.525731f },
	{ 0.000000f, -0.955423f, 0.295242f },
	{ 0.238856f, -0.864188f, 0.442863f },
	{ 0.262866f, -0.951056f, 0.162460f },
	{ 0.500000f, -0.809017f, 0.309017f },
	{ 0.716567f, -0.681718f, 0.147621f },
	{ 0.525731f, -0.850651f, 0.000000f },
	{ -0.238856f, -0.864188f, -0.442863f },
	{ -0.500000f, -0.809017f, -0.309017f },
	{ -0.262866f, -0.951056f, -0.162460f },
	{ -0.850651f, -0.525731f, 0.000000f },
	{ -0.716567f, -0.681718f, -0.147621f },
	{ -0.716567f, -0.681718f, 0.147621f },
	{ -0.525731f, -0.850651f, 0.000000f },
	{ -0.500000f, -0.809017f, 0.309017f },
	{ -0.238856f, -0.864188f, 0.442863f },
	{ -0.262866f, -0.951056f, 0.162460f },
	{ -0.864188f, -0.442863f, 0.238856f },
	{ -0.809017f, -0.309017f, 0.500000f },
	{ -0.688191f, -0.587785f, 0.425325f },
	{ -0.681718f, -0.147621f, 0.716567f },
	{ -0.442863f, -0.238856f, 0.864188f },
	{ -0.587785f, -0.425325f, 0.688191f },
	{ -0.309017f, -0.500000f, 0.809017f },
	{ -0.147621f, -0.716567f, 0.681718f },
	{ -0.425325f, -0.688191f, 0.587785f },
	{ -0.162460f, -0.262866f, 0.951056f },
	{ 0.442863f, -0.238856f, 0.864188f },
	{ 0.162460f, -0.262866f, 0.951056f },
	{ 0.309017f, -0.500000f, 0.809017f },
	{ 0.147621f, -0.716567f, 0.681718f },
	{ 0.000000f, -0.525731f, 0.850651f },
	{ 0.425325f, -0.688191f, 0.587785f },
	{ 0.587785f, -0.425325f, 0.688191f },
	{ 0.688191f, -0.587785f, 0.425325f },
	{ -0.955423f, 0.295242f, 0.000000f },
	{ -0.951056f, 0.162460f, 0.262866f },
	{ -1.000000f, 0.000000f, 0.000000f },
	{ -0.850651f, 0.000000f, 0.525731f },
	{ -0.955423f, -0.295242f, 0.000000f },
	{ -0.951056f, -0.162460f, 0.262866f },
	{ -0.864188f, 0.442863f, -0.238856f },
	{ -0.951056f, 0.162460f, -0.262866f },
	{ -0.809017f, 0.309017f, -0.500000f },
	{ -0.864188f, -0.442863f, -0.238856f },
	{ -0.951056f, -0.162460f, -0.262866f },
	{ -0.809017f, -0.309017f, -0.500000f },
	{ -0.681718f, 0.147621f, -0.716567f },
	{ -0.681718f, -0.147621f, -0.716567f },
	{ -0.850651f, 0.000000f, -0.525731f },
	{ -0.688191f, 0.587785f, -0.425325f },
	{ -0.587785f, 0.425325f, -0.688191f },
	{ -0.425325f, 0.688191f, -0.587785f },
	{ -0.425325f, -0.688191f, -0.587785f },
	{ -0.587785f, -0.425325f, -0.688191f },
	{ -0.688191f, -0.587785f, -0.425325f },
};


//glcmds related
int	commands[65536];
int numcommands, numglverts;
int	used[MAX_TRIANGLES];
int	strip_xyz[128];
int	strip_st[128];
int	strip_tris[128];
int	stripcount;
int FanLength(int starttri, int startv, md2_model_t *md2);
int StripLength(int starttri, int startv, md2_model_t *md2);
void BuildGlCmds(md2_model_t *md2);

//


int SaveAsMD2(const char *filename, mdx_model_t *mdx)
{
	md2_model_t *md2;
	mdx_glCommandVertex_t v1, v2, v3;
	int pos = 0;
	int maxSkinName = 64;
	int something;
	int i, j, k;
	int z, x;
	int s = 0;
	int trick = 0;
	int first;//, extra
	int numStripVerts, numFanVerts;
	int isFinished = 0;
	int texCoordStart = 0;
	int totalTris = 0;
	int TrisCount = 0;
	int TexTrisCount = 0;
	short *glTris=NULL;
	short *glTexTris=NULL;
	float sf, tf;
	int handle;

	if (!mdx)
		return 0;

	//Open MD2 File for writing 
	handle = _open(filename, _O_BINARY | _O_WRONLY | _O_CREAT | _O_TRUNC, _S_IREAD | _S_IWRITE);

	//Check for error
	if (handle == -1)
		return 0; //return on error

	//Init MD2	
	md2 =(md2_model_t*) malloc(sizeof(md2_model_t));
	if (!md2)
	{
		_close(handle);
		return 0; //return on error
	}

	//Header data
	memset(md2, 0, sizeof(md2_model_t));
	md2->header.frameSize = mdx->header.frameSize;
	md2->header.magic = (int)(('2' << 24) + ('P' << 16) + ('D' << 8) + 'I');
	md2->header.numFrames = mdx->header.numFrames;
	md2->header.numGlCommands = mdx->header.numGlCommands;
	md2->header.numSkins = mdx->header.numSkins;
	md2->header.numTriangles = mdx->header.numTriangles;
	md2->header.numVertices = mdx->header.numVertices;
	md2->header.skinHeight = mdx->header.skinHeight;
	md2->header.skinWidth = mdx->header.skinWidth;
	md2->header.version = 8;

	//extra = 0;


	//Allocate memory for GLCommands
	md2->glCommandBuffer = (int *)malloc(sizeof(int) * md2->header.numGlCommands);
	if (md2->glCommandBuffer)
	{
		md2->texCoords = (md2_textureCoordinate_t*)malloc(sizeof(md2_textureCoordinate_t) * md2->header.numTriangles * 3); //sizeof(short) * 2
		if (md2->texCoords)
		{
			//tri vertex index number
			glTris = (short*)malloc(sizeof(short)*mdx->header.numTriangles * 3*3);
			if (glTris)
			{
				//texture index number
				glTexTris = (short*)malloc(sizeof(short)*mdx->header.numTriangles * 3*3);
				if (glTexTris)
				{
					i = j = k = 0;
					//rebuild GLCommands and create software skin UV cords..
					while (!isFinished )
					{
						first = md2->glCommandBuffer[k++] = mdx->glCommandBuffer[i++];

						//something that we have to get rid of in the glcommand conversion
						if (!mdx->isMD2)
							something = mdx->glCommandBuffer[i++]; //mdx. object number

						if (first > 0) // Triangle strip
						{
							numStripVerts = first;
							totalTris += numStripVerts - 2;

							//First vert
							*(float*)&md2->glCommandBuffer[k++] = sf = *(float*)&mdx->glCommandBuffer[i++];
							*(float*)&md2->glCommandBuffer[k++] = tf = *(float*)&mdx->glCommandBuffer[i++];

							md2->texCoords[j].s = (short)(sf*mdx->header.skinWidth);
							md2->texCoords[j++].t = (short)(tf*mdx->header.skinHeight);

							//vert index
							v2.vertexIndex = md2->glCommandBuffer[k++] = mdx->glCommandBuffer[i++];


							//Second vert
							*(float*)&md2->glCommandBuffer[k++] = sf = *(float*)&mdx->glCommandBuffer[i++];
							*(float*)&md2->glCommandBuffer[k++] = tf = *(float*)&mdx->glCommandBuffer[i++];

							md2->texCoords[j].s = (short)(sf*mdx->header.skinWidth);
							md2->texCoords[j++].t = (short)(tf*mdx->header.skinHeight);

							//vert index
							v3.vertexIndex = md2->glCommandBuffer[k++] = mdx->glCommandBuffer[i++];


							for (z = 1;z <= (numStripVerts - 2);z++)
							{
								v1.vertexIndex = v2.vertexIndex;
								v2.vertexIndex = v3.vertexIndex;

								*(float*)&md2->glCommandBuffer[k++] = sf = *(float*)&mdx->glCommandBuffer[i++];
								*(float*)&md2->glCommandBuffer[k++] = tf = *(float*)&mdx->glCommandBuffer[i++];

								md2->texCoords[j].s = (short)(sf*mdx->header.skinWidth);
								md2->texCoords[j++].t = (short)(tf*mdx->header.skinHeight);

								//vert index
								v3.vertexIndex = md2->glCommandBuffer[k++] = mdx->glCommandBuffer[i++];

								// As we create the strip, the vertex winding changes...
								if (!(z & 1))
								{
									//append glTris		[v1.vertexIndex, v2.vertexIndex, v3.vertexIndex]
									glTris[TrisCount++] = (short)v1.vertexIndex;
									glTris[TrisCount++] = (short)v2.vertexIndex;
									glTris[TrisCount++] = (short)v3.vertexIndex;

									//append glTexTris	[texCoordStart + z + 2, texCoordStart + z + 1, texCoordStart + z]
									glTexTris[TexTrisCount++] = (short)(texCoordStart + z + 2 - 1);
									glTexTris[TexTrisCount++] = (short)(texCoordStart + z + 1 - 1);
									glTexTris[TexTrisCount++] = (short)(texCoordStart + z - 1);
								}
								else
								{
									//append glTris		[v3.vertexIndex, v2.vertexIndex, v1.vertexIndex]
									glTris[TrisCount++] = (short)v3.vertexIndex;
									glTris[TrisCount++] = (short)v2.vertexIndex;
									glTris[TrisCount++] = (short)v1.vertexIndex;

									//append glTexTris	[texCoordStart + z, texCoordStart + z + 1, texCoordStart + z + 2]
									glTexTris[TexTrisCount++] = (short)(texCoordStart + z - 1);
									glTexTris[TexTrisCount++] = (short)(texCoordStart + z + 1 - 1);
									glTexTris[TexTrisCount++] = (short)(texCoordStart + z + 2 - 1);
								}
							}
							texCoordStart += numStripVerts;
						}

						else if (first < 0) // Triangle fan
						{
							numFanVerts = -first;
							totalTris += numFanVerts - 2;

							//First vert
							*(float*)&md2->glCommandBuffer[k++] = sf = *(float*)&mdx->glCommandBuffer[i++];
							*(float*)&md2->glCommandBuffer[k++] = tf = *(float*)&mdx->glCommandBuffer[i++];

							md2->texCoords[j].s = (short)(sf*mdx->header.skinWidth);
							md2->texCoords[j++].t = (short)(tf*mdx->header.skinHeight);

							//vert index
							v1.vertexIndex = md2->glCommandBuffer[k++] = mdx->glCommandBuffer[i++];


							//Second vert
							*(float*)&md2->glCommandBuffer[k++] = sf = *(float*)&mdx->glCommandBuffer[i++];
							*(float*)&md2->glCommandBuffer[k++] = tf = *(float*)&mdx->glCommandBuffer[i++];

							md2->texCoords[j].s = (short)(sf*mdx->header.skinWidth);
							md2->texCoords[j++].t = (short)(tf*mdx->header.skinHeight);

							//vert index
							v3.vertexIndex = md2->glCommandBuffer[k++] = mdx->glCommandBuffer[i++];

							for (z = 1;z <= (numFanVerts - 2);z++)
							{
								v2.vertexIndex = v3.vertexIndex;

								*(float*)&md2->glCommandBuffer[k++] = sf = *(float*)&mdx->glCommandBuffer[i++];
								*(float*)&md2->glCommandBuffer[k++] = tf = *(float*)&mdx->glCommandBuffer[i++];

								md2->texCoords[j].s = (short)(sf*mdx->header.skinWidth);
								md2->texCoords[j++].t = (short)(tf*mdx->header.skinHeight);

								//vert index
								v3.vertexIndex = md2->glCommandBuffer[k++] = mdx->glCommandBuffer[i++];

								//append glTris		[v3.vertexIndex, v2.vertexIndex, v1.vertexIndex]
								glTris[TrisCount++] = (short)v3.vertexIndex;
								glTris[TrisCount++] = (short)v2.vertexIndex;
								glTris[TrisCount++] = (short)v1.vertexIndex;

								//append glTexTris	[texCoordStart + 1, texCoordStart + z + 1, texCoordStart + z + 2]
								glTexTris[TexTrisCount++] = (short)(texCoordStart + 1 - 1);
								glTexTris[TexTrisCount++] = (short)(texCoordStart + z + 1 - 1);
								glTexTris[TexTrisCount++] = (short)(texCoordStart + z + 2 - 1);
							}
							texCoordStart += numFanVerts;
						}
						else if (first == 0)
						{
							isFinished = 1;
						}
					}

					//now we have glTris and glTexTris..now we need to write them & flip tris normals
					md2->triangles = (mdx_triangle_t *)malloc(sizeof(mdx_triangle_t)*totalTris * 3);
					if (md2->triangles)
					{
						memset(md2->triangles, -1, sizeof(mdx_triangle_t)*totalTris * 3);
						for (z = 0, x = 0;z < TrisCount;z++)
						{
							md2->triangles[z].vertexIndices[2] = glTris[x];
							md2->triangles[z].textureIndices[0] = glTexTris[x++];

							md2->triangles[z].vertexIndices[1] = glTris[x];
							md2->triangles[z].textureIndices[1] = glTexTris[x++];

							md2->triangles[z].vertexIndices[0] = glTris[x];
							md2->triangles[z].textureIndices[2] = glTexTris[x++];
						}

						//fixup header
						md2->header.numTexCoords = j;
						md2->header.numTriangles = totalTris;
						//md2->header.numGlCommands = mdx->header.numGlCommands - extra;
						md2->header.numGlCommands = k; //hypov8 use original gl commands

						//Header offsets
						pos += sizeof(md2_header_t);
						md2->header.offsetSkins = pos;

						pos += maxSkinName*md2->header.numSkins;
						md2->header.offsetTexCoords = pos;

						pos += md2->header.numTexCoords * sizeof(md2_textureCoordinate_t);
						md2->header.offsetTriangles = pos;

						pos += md2->header.numTriangles * sizeof(mdx_triangle_t);
						md2->header.offsetFrames = pos;

						pos += mdx->header.frameSize * mdx->header.numFrames;
						md2->header.offsetGlCommands = pos;

						//BuildGlCmds(md2); //hypov8 todo: this is using uv byte data
						//md2->header.numGlCommands = numcommands;

						pos += (md2->header.numGlCommands * 4);//-(4*extra);  //4 bytes (dword)
						md2->header.offsetEnd = pos;

						//Write header
						_write(handle, &md2->header, sizeof(md2_header_t));
						//Write skins
						_write(handle, mdx->skins, maxSkinName*mdx->header.numSkins);
						//Write TexCoords
						_write(handle, md2->texCoords, md2->header.numTexCoords * sizeof(md2_textureCoordinate_t));
						//Write Tris
						_write(handle, md2->triangles, md2->header.numTriangles * sizeof(mdx_triangle_t));
						//Write animation frames
						_write(handle, mdx->framesBuffer, (sizeof(byte) * (mdx->header.numFrames * mdx->header.frameSize)));
						//Write GLCommands //hypov8 note: this completly rebuilds gl commands and looses texture precision. was there a reason to rebuild? 
						//write(handle, commands, numcommands * 4);
						//Write GlCommands...hack really..need to generate new ones 
						_write(handle, md2->glCommandBuffer, (md2->header.numGlCommands * 4));


						_close(handle);
						md2_freeModel(md2);
						free(glTris);
						free(glTexTris);

						return 1;

					}//gl
				}//text cords
			}//glTris
		}//glTexTris
	}//md2->triangles

	//error
	_close(handle);
	md2_freeModel(md2);
	free(glTris);
	free(glTexTris);
	return 0;

}

int SaveAsMDX(const char *filename, mdx_model_t *active_model)
{
	mdx_model_t *mdx_export;
	//mdx_glCommandVertex_t v1, v2, v3;
	int pos = 0;
	int i=0, j=0, k=0;
	int z;
	int s = 0;
	int trick = 0;
	int first;//, extra
	int numStripVerts, numFanVerts;
	int isFinished = 0;
	int texCoordStart = 0;
	int totalTris = 0;
	int TrisCount = 0;
	int TexTrisCount = 0;
	//short *glTris = NULL;
	//short *glTexTris = NULL;
	float *bboxFrames = NULL;
	int *vertInfo = NULL;
	float minmax[6];

	//float sf, tf;
	int handle;

	if (!active_model)
		return 0;
	if (!active_model->isMD2)
		return 0; //no need to save an mdx
	if (active_model->header.numGlCommands <3)
		return 0; //no gl commands

	//_sopen_s; //hypov8 todo:
	//Open MD2 File for writing 
	handle = _open(filename, _O_BINARY | _O_WRONLY | _O_CREAT | _O_TRUNC, _S_IREAD | _S_IWRITE);

	//Check for error
	if (handle == -1)
		return 0; //return on error

	//Init memory
	mdx_export = (mdx_model_t*)malloc(sizeof(mdx_model_t));
	if (!mdx_export)
	{
		_close(handle);
		return 0; //return on error
	}
	memset(mdx_export, 0, sizeof(mdx_model_t));
	
	bboxFrames = (float*)malloc(sizeof(float)*active_model->header.numFrames * 6);
	vertInfo = (int*)malloc(sizeof(int)*active_model->header.numVertices);
	mdx_export->glCommandBuffer = (int *)malloc(sizeof(int) * active_model->header.numTriangles * 5 + active_model->header.numTriangles * 2);//Allocate memory for GLCommands
	if (!mdx_export || !bboxFrames || !mdx_export->glCommandBuffer)
	{
		_close(handle);
		return 0; //return on error
	}


	//Header data
	mdx_export->header.magic = (int)(('X' << 24) + ('P' << 16) + ('D' << 8) + 'I');
	mdx_export->header.version = 4; // ? ?
	mdx_export->header.skinWidth = active_model->header.skinWidth;
	mdx_export->header.skinHeight = active_model->header.skinHeight;
	mdx_export->header.frameSize = active_model->header.frameSize;
	mdx_export->header.numSkins = active_model->header.numSkins;
	mdx_export->header.numVertices = active_model->header.numVertices;
	mdx_export->header.numTriangles = active_model->header.numTriangles;
	mdx_export->header.numGlCommands = active_model->header.numGlCommands; //todo: this is wrong
	mdx_export->header.numFrames = active_model->header.numFrames;
	mdx_export->header.numSfxDefines =  0; //
	mdx_export->header.numSfxEntries = 0;
	mdx_export->header.numSubObjects = 1; //active_model->header.numSubObjects;
	//todo: bbox
	//todo: ofs dummy


	//rebuild GLCommands and create software skin UV cords..
	while (!isFinished)
	{
		//read/write tris type (fan/strip)
		first = mdx_export->glCommandBuffer[k++] = active_model->glCommandBuffer[i++];

		//read/write object 0
		mdx_export->glCommandBuffer[k++] = 0;


		if (first > 0) // Triangle strip
		{
			numStripVerts = first;
			totalTris += numStripVerts - 2;

			//read/write First vert
			*(float*)&mdx_export->glCommandBuffer[k++] =  *(float*)&active_model->glCommandBuffer[i++];
			*(float*)&mdx_export->glCommandBuffer[k++] = *(float*)&active_model->glCommandBuffer[i++];


			//read/write vert index
			mdx_export->glCommandBuffer[k++] = active_model->glCommandBuffer[i++];


			//read/write Second vert
			*(float*)&mdx_export->glCommandBuffer[k++] = *(float*)&active_model->glCommandBuffer[i++];
			*(float*)&mdx_export->glCommandBuffer[k++] = *(float*)&active_model->glCommandBuffer[i++];

			//read/write vert index
			mdx_export->glCommandBuffer[k++] = active_model->glCommandBuffer[i++];


			for (z = 1;z <= (numStripVerts - 2);z++)
			{
				//read/write  vert
				*(float*)&mdx_export->glCommandBuffer[k++] = *(float*)&active_model->glCommandBuffer[i++];
				*(float*)&mdx_export->glCommandBuffer[k++] = *(float*)&active_model->glCommandBuffer[i++];

				//read/write vert index
				mdx_export->glCommandBuffer[k++] = active_model->glCommandBuffer[i++];
			}
			texCoordStart += numStripVerts;
		}

		else if (first < 0) // Triangle fan
		{
			numFanVerts = -first;
			totalTris += numFanVerts - 2;

			//read/write First vert
			*(float*)&mdx_export->glCommandBuffer[k++] = *(float*)&active_model->glCommandBuffer[i++];
			*(float*)&mdx_export->glCommandBuffer[k++] = *(float*)&active_model->glCommandBuffer[i++];

			//read/write vert index
			mdx_export->glCommandBuffer[k++] = active_model->glCommandBuffer[i++];


			//read/write Second vert
			*(float*)&mdx_export->glCommandBuffer[k++] = *(float*)&active_model->glCommandBuffer[i++];
			*(float*)&mdx_export->glCommandBuffer[k++] = *(float*)&active_model->glCommandBuffer[i++];
			//read/write Second vert index
			 mdx_export->glCommandBuffer[k++] = active_model->glCommandBuffer[i++];

			for (z = 1;z <= (numFanVerts - 2);z++)
			{
				//read/write vert
				*(float*)&mdx_export->glCommandBuffer[k++] = *(float*)&active_model->glCommandBuffer[i++];
				*(float*)&mdx_export->glCommandBuffer[k++] = *(float*)&active_model->glCommandBuffer[i++];

				//read/write  vert index
				 mdx_export->glCommandBuffer[k++] = active_model->glCommandBuffer[i++];
			}
			texCoordStart += numFanVerts;
		}
		else if (first == 0)
		{
			isFinished = 1;
		}
	}


	//fill hitbox
	for (i = 0; i < active_model->header.numFrames;i++)
	{
		mdx_getBoundingBoxExport(active_model, minmax, i);
		bboxFrames[i * 6 + 0] = minmax[0];
		bboxFrames[i * 6 + 1] = minmax[1];
		bboxFrames[i * 6 + 2] = minmax[2];
		bboxFrames[i * 6 + 3] = minmax[3];
		bboxFrames[i * 6 + 4] = minmax[4];
		bboxFrames[i * 6 + 5] = minmax[5];
	}

	//fill vertex info
	for (i = 0; i < active_model->header.numVertices;i++)
	{
		vertInfo[i] = 0;
	}


	//finish header
	mdx_export->header.numGlCommands = k; //hypov8 use original gl commands

	pos += sizeof(mdx_header_t);
	mdx_export->header.offsetSkins = pos;	//offsetSkins; 

	pos += mdx_export->header.numSkins*sizeof(mdx_skin_t);
	mdx_export->header.offsetTriangles = pos;	//offsetTriangles

	pos += mdx_export->header.numTriangles * sizeof(mdx_triangle_t);
	mdx_export->header.offsetFrames = pos;	//offsetFrames

	pos += active_model->header.numFrames*active_model->header.frameSize * sizeof(byte);
	mdx_export->header.offsetGlCommands = pos;	//offsetGlCommands

	pos += (mdx_export->header.numGlCommands * sizeof(int));
	mdx_export->header.offsetVertexInfo = pos;	//offsetVertexInfo

	pos += (mdx_export->header.numVertices * sizeof(int));
	mdx_export->header.offsetSfxDefines = pos;	//offsetSfxDefines
	mdx_export->header.offsetSfxEntries = pos;	//offsetSfxEntries
	mdx_export->header.offsetBBoxFrames = pos;	//offsetBBoxFrames
	pos += (active_model->header.numFrames * sizeof(int) * 6);
	mdx_export->header.offsetDummyEnd = pos;	//offsetDummyEnd
	mdx_export->header.offsetEnd = pos;	//offsetEnd

		
	_write(handle, &mdx_export->header, sizeof(mdx_header_t));//Write header
	_write(handle, active_model->skins, active_model->header.numSkins*sizeof(mdx_skin_t));//Write skins
	_write(handle, active_model->triangles, mdx_export->header.numTriangles * sizeof(mdx_triangle_t));//Write Tris
	_write(handle, active_model->framesBuffer, active_model->header.numFrames * active_model->header.frameSize * sizeof(byte));//Write animation frames
	_write(handle, mdx_export->glCommandBuffer, mdx_export->header.numGlCommands * sizeof(int));//Write GlCommands... todo:should we generate new ones?
	_write(handle, vertInfo, mdx_export->header.numVertices * sizeof(int));
	// offsetSfxDefines;
	// offsetSfxEntries;
	_write(handle, bboxFrames, mdx_export->header.numFrames * sizeof(float) * 6);
	// offsetBBoxFrames;
	// offsetDummyEnd;
	_close(handle);
	mdx_freeModel(mdx_export);

	return 1;
}

int SaveAsMD_(const char *filename, mdx_model_t *model)
{
	if (model->isMD2)
		return SaveAsMDX(filename, model);
	else
		return SaveAsMD2(filename, model);
}

// from models.c in the Quake 2 source.
void BuildGlCmds(md2_model_t *md2)
{
	int		i, j, k;
	int		startv;
	float		s, t;
	int		len, bestlen, besttype;
	int		best_xyz[1024];
	int		best_st[1024];
	int		best_tris[1024];
	int		type;

	//
	// build tristrips
	//
	numcommands = 0;
	numglverts = 0;
	memset(used, 0, sizeof(used));
	for (i = 0; i<md2->header.numTriangles; i++)
	{
		// pick an unused triangle and start the trifan
		if (used[i])
			continue;
		bestlen = 0;
		for (type = 0; type < 2; type++)
		{
			for (startv = 0; startv < 3; startv++)
			{
				if (type == 1)
					len = StripLength(i, startv, md2);
				else
					len = FanLength(i, startv, md2);
				if (len > bestlen)
				{
					besttype = type;
					bestlen = len;
					for (j = 0; j<bestlen + 2; j++)
					{
						best_st[j] = strip_st[j];
						best_xyz[j] = strip_xyz[j];
					}
					for (j = 0; j<bestlen; j++)
						best_tris[j] = strip_tris[j];
				}
			}
		}

		// mark the tris on the best strip/fan as used
		for (j = 0; j<bestlen; j++)
			used[best_tris[j]] = 1;

		if (besttype == 1)
			commands[numcommands++] = (bestlen + 2);
		else
			commands[numcommands++] = -(bestlen + 2);

		numglverts += bestlen + 2;

		for (j = 0; j<bestlen + 2; j++)
		{
			// emit a vertex into the reorder buffer
			k = best_st[j];

			// emit s/t coords into the commands stream
			s = md2->texCoords[k].s;
			t = md2->texCoords[k].t;

			s = (float)((s + 0.5) / md2->header.skinWidth);
			t = (float)((t + 0.5) / md2->header.skinHeight);

			if (numcommands>16510)
			{
				int x = 25;
				x += numcommands - 4 * 10;
				k = best_st[j];
			}

			*(float*)&commands[numcommands++] = s;
			*(float*)&commands[numcommands++] = t;
			*(int *)&commands[numcommands++] = best_xyz[j];
		}
	}

	commands[numcommands++] = 0;		// end of list marker
}

//===========
//StripLength
//===========
int StripLength(int starttri, int startv, md2_model_t *md2)
{
	int				m1, m2;
	int				st1, st2;
	int				j;
	//	TRealTriangle	*last, *check;
	mdx_triangle_t  *last, *check;
	int				k;

	used[starttri] = 2;

	//	last = &Tris[starttri];
	last = &md2->triangles[starttri];

	strip_xyz[0] = last->vertexIndices[(startv) % 3];
	strip_xyz[1] = last->vertexIndices[(startv + 1) % 3];
	strip_xyz[2] = last->vertexIndices[(startv + 2) % 3];
	strip_st[0] = last->textureIndices[(startv) % 3];
	strip_st[1] = last->textureIndices[(startv + 1) % 3];
	strip_st[2] = last->textureIndices[(startv + 2) % 3];

	strip_tris[0] = starttri;
	stripcount = 1;

	m1 = last->vertexIndices[(startv + 2) % 3];
	st1 = last->textureIndices[(startv + 2) % 3];
	m2 = last->vertexIndices[(startv + 1) % 3];
	st2 = last->textureIndices[(startv + 1) % 3];

	// look for a matching triangle
nexttri:
	for (j = starttri + 1, check = &md2->triangles[starttri + 1]
		; j<md2->header.numTriangles; j++, check++)
	{
		for (k = 0; k<3; k++)
		{
			if (check->vertexIndices[k] != m1)
				continue;
			if (check->textureIndices[k] != st1)
				continue;
			if (check->vertexIndices[(k + 1) % 3] != m2)
				continue;
			if (check->textureIndices[(k + 1) % 3] != st2)
				continue;

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if (used[j])
				goto done;

			// the new edge
			if (stripcount & 1)
			{
				m2 = check->vertexIndices[(k + 2) % 3];
				st2 = check->textureIndices[(k + 2) % 3];
			}
			else
			{
				m1 = check->vertexIndices[(k + 2) % 3];
				st1 = check->textureIndices[(k + 2) % 3];
			}

			strip_xyz[stripcount + 2] = check->vertexIndices[(k + 2) % 3];
			strip_st[stripcount + 2] = check->textureIndices[(k + 2) % 3];
			strip_tris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}
done:

	// clear the temp used flags
	for (j = starttri + 1; j<md2->header.numTriangles; j++)
		if (used[j] == 2)
			used[j] = 0;

	return stripcount;
}


//===========
//FanLength
//===========
int FanLength(int starttri, int startv, md2_model_t *md2)
{
	int		m1, m2;
	int		st1, st2;
	int		j;
	//	TRealTriangle	*last, *check;
	mdx_triangle_t  *last, *check;
	int		k;

	used[starttri] = 2;

	last = &md2->triangles[starttri];

	strip_xyz[0] = last->vertexIndices[(startv) % 3];
	strip_xyz[1] = last->vertexIndices[(startv + 1) % 3];
	strip_xyz[2] = last->vertexIndices[(startv + 2) % 3];
	strip_st[0] = last->textureIndices[(startv) % 3];
	strip_st[1] = last->textureIndices[(startv + 1) % 3];
	strip_st[2] = last->textureIndices[(startv + 2) % 3];

	strip_tris[0] = starttri;
	stripcount = 1;

	m1 = last->vertexIndices[(startv + 0) % 3];
	st1 = last->textureIndices[(startv + 0) % 3];
	m2 = last->vertexIndices[(startv + 2) % 3];
	st2 = last->textureIndices[(startv + 2) % 3];


	// look for a matching triangle
nexttri:
	for (j = starttri + 1, check = &md2->triangles[starttri + 1]
		; j<md2->header.numTriangles; j++, check++)
	{
		for (k = 0; k<3; k++)
		{
			if (check->vertexIndices[k] != m1)
				continue;
			if (check->textureIndices[k] != st1)
				continue;
			if (check->vertexIndices[(k + 1) % 3] != m2)
				continue;
			if (check->textureIndices[(k + 1) % 3] != st2)
				continue;

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if (used[j])
				goto done;

			// the new edge
			m2 = check->vertexIndices[(k + 2) % 3];
			st2 = check->textureIndices[(k + 2) % 3];

			strip_xyz[stripcount + 2] = m2;
			strip_st[stripcount + 2] = st2;
			strip_tris[stripcount] = j;
			stripcount++;

			used[j] = 2;
			goto nexttri;
		}
	}
done:

	// clear the temp used flags
	for (j = starttri + 1; j<md2->header.numTriangles; j++)
		if (used[j] == 2)
			used[j] = 0;

	return stripcount;
}




