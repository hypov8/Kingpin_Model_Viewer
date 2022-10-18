/*#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <string.h> // memset
#include <math.h>*/ // sqrt

#include <stdio.h>
#include <mx/gl.h>
#include "common.h"
//#include "anorms.h"



/*
 * load model
 */
md2_model_t* md2_readModel (const char *filename, int debugLoad)
{
	FILE *file=NULL;
	md2_model_t *model;
	byte buffer[MD2_MAX_FRAMESIZE];
	long fLen;

	model = (md2_model_t *) malloc (sizeof (md2_model_t));
	if (!model)
		return 0;

	if (fopen_s(&file,filename, "rb"))
	{
		free (model);
		return 0;
	}

	//g_glcmds = 0; /* use glcommands */
	//g_interp = 1; /* interpolate frames */

	/* initialize model and read header */
	memset (model, 0, sizeof (md2_model_t));
	fread (&model->header, sizeof (md2_header_t), 1, file);

	fseek(file, 0L, SEEK_END);
	fLen = ftell(file);

#if 0
	printf("magic:\t\t%d\n", model->header.magic);
	printf("version:\t\t%d\n", model->header.version);
	printf("skinWidth:\t\t%d\n", model->header.skinWidth);
	printf("skinHeight:\t\t%d\n", model->header.skinHeight);
	printf("frameSize:\t\t%d\n", model->header.frameSize);
	printf("numSkins:\t\t%d\n", model->header.numSkins);
	printf("numVertices:\t\t%d\n", model->header.numVertices);
	printf("numTexCoords:\t\t%d\n", model->header.numTexCoords);
	printf("numTriangles:\t\t%d\n", model->header.numTriangles);
	printf("numGlCommands:\t\t%d\n", model->header.numGlCommands);
	printf("numFrames:\t\t%d\n", model->header.numFrames);
	printf("offsetSkins:\t\t%d\n", model->header.offsetSkins);
	printf("offsetTexCoords:\t%d\n", model->header.offsetTexCoords);
	printf("offsetTriangles:\t%d\n", model->header.offsetTriangles);
	printf("offsetFrames:\t\t%d\n", model->header.offsetFrames);
	printf("offsetGlCommands:\t%d\n", model->header.offsetGlCommands);
	printf("offsetEnd:\t\t%d\n", model->header.offsetEnd);
#endif
	if (!debugLoad && (
		model->header.magic != (int)(('2' << 24) + ('P' << 16) + ('D' << 8) + 'I') ||
		//do more checks
		model->header.numGlCommands < 4 ||
		fLen < model->header.offsetEnd || model->header.offsetEnd <= 0 ||
		fLen < model->header.offsetFrames || model->header.offsetFrames <= 0 ||
		fLen < model->header.offsetGlCommands || model->header.offsetGlCommands <= 0 ||
		fLen < model->header.offsetSkins || model->header.offsetSkins <= 0 ||
		fLen < model->header.offsetTexCoords || model->header.offsetTexCoords <= 0 ||
		fLen < model->header.offsetTriangles || model->header.offsetTriangles <= 0))
	{
		fclose (file);
		free (model);
		return 0;
	}


	/* read skins */
	fseek (file, model->header.offsetSkins, SEEK_SET);
	if (model->header.numSkins > 0)
	{
		model->skins = (mdx_skin_t *) malloc (sizeof (mdx_skin_t) * model->header.numSkins);
		if (!model->skins)
		{
			fclose(file); //hypov8
			md2_freeModel (model);
			return 0;
		}

		fread(model->skins, sizeof(mdx_skin_t), model->header.numSkins, file);
		//for (i = 0; i < model->header.numSkins; i++)
		//	fread (&model->skins[i], sizeof (md2_skin_t), 1, file);
	}

	/* read software texture coordinates */
	fseek (file, model->header.offsetTexCoords, SEEK_SET);
	if (model->header.numTexCoords > 0)
	{
		model->texCoords = (md2_textureCoordinate_t *) malloc (sizeof (md2_textureCoordinate_t) * model->header.numTexCoords);
		if (!model->texCoords)
		{
			fclose(file);
			md2_freeModel (model);
			return 0;
		}
		fread (model->texCoords, sizeof (md2_textureCoordinate_t), model->header.numTexCoords, file);

		//hypov8 todo:
		//for (i = 0; i < model->header.numTexCoords; i++)
		//	fread (&model->texCoords[i], sizeof (md2_textureCoordinate_t), 1, file);
	}

	/* read triangles */
	fseek (file, model->header.offsetTriangles, SEEK_SET);
	if (model->header.numTriangles > 0)
	{
		model->triangles = (mdx_triangle_t *) malloc (sizeof (mdx_triangle_t) * model->header.numTriangles);
		if (!model->triangles)
		{
			fclose(file); //hypov8
			md2_freeModel (model);
			return 0;
		}
		fread (model->triangles, sizeof (mdx_triangle_t), model->header.numTriangles, file);
		//for (i = 0; i < model->header.numTriangles; i++)
		//	fread (&model->triangles[i], sizeof (md2_triangle_t), 1, file);
	}

	/* read alias frames */
	fseek (file, model->header.offsetFrames, SEEK_SET);
	if (model->header.numFrames > 0)
	{
		if (!mdx_readFrameData(file, &model->frames, buffer, model->header.numFrames, model->header.numVertices,
			 model->header.frameSize, model->min, model->max, model->header.offsetFrames, &model->framesBuffer))
		{
			fclose(file); //hypov8
			md2_freeModel(model);
			return 0;
		}
	}
	

	/* read gl commands */
	fseek (file, model->header.offsetGlCommands, SEEK_SET);
	if (model->header.numGlCommands)
	{
		model->glCommandBuffer = (int *) malloc (sizeof (int) * model->header.numGlCommands);
		if (!model->glCommandBuffer)
		{
			fclose(file);
			md2_freeModel (model);
			return 0;
		}
		fread (model->glCommandBuffer, sizeof (int), model->header.numGlCommands, file);
	}

	fclose (file);

	return model;
}


//hypov8 convert md2 to mdx compatable
mdx_model_t * md2_Parse_readModel(const char * filename, int debugLoad)
{
	int i;
	md2_model_t *md2Src;
	mdx_model_t *mdxOut;

	md2Src = md2_readModel(filename, debugLoad);
	if (!md2Src)
		return 0;

	//convert md2 to mdx viewer usable format
	mdxOut = (mdx_model_t *)malloc(sizeof(mdx_model_t));
	if (!mdxOut)
		return 0;


	//header
	memset(mdxOut, 0, sizeof(mdx_model_t));
	memcpy(&mdxOut->header, &md2Src->header, sizeof(int) * 7);
	mdxOut->header.numTriangles = md2Src->header.numTriangles;
	mdxOut->header.numGlCommands = md2Src->header.numGlCommands;
	mdxOut->header.numFrames = md2Src->header.numFrames;

	memcpy(mdxOut->min, md2Src->min, sizeof(float) * 3);
	memcpy(mdxOut->max, md2Src->max, sizeof(float) * 3);

	//skins
	mdxOut->skins = (mdx_skin_t *)malloc(sizeof(mdx_skin_t)*md2Src->header.numSkins);
	if (mdxOut->skins)
	{
		memcpy(mdxOut->skins, md2Src->skins, sizeof(mdx_skin_t)*md2Src->header.numSkins);

		//tex cords

		//triangles
		mdxOut->triangles = (mdx_triangle_t *)malloc(sizeof(mdx_triangle_t) * md2Src->header.numTriangles);
		if (mdxOut->triangles)
		{
			memcpy(mdxOut->triangles, md2Src->triangles, sizeof(mdx_triangle_t)*md2Src->header.numTriangles);

			//frames
			mdxOut->frames = (mdx_frame_t *)malloc(sizeof(mdx_frame_t) * md2Src->header.numFrames);
			if (mdxOut->frames)
			{
				for (i = 0;i < md2Src->header.numFrames; i++)
				{
					memcpy(mdxOut->frames[i].name, md2Src->frames[i].name, sizeof(md2Src->frames[i].name));
					
					//vertex	
					mdxOut->frames[i].vertices = (mdx_triangleVertex_t *)malloc(sizeof(mdx_triangleVertex_t) * md2Src->header.numVertices);
					if (!mdxOut->frames[i].vertices)
					{
						mdx_freeModel(mdxOut);
						md2_freeModel(md2Src);
						return 0;
					}
					memcpy(mdxOut->frames[i].vertices, md2Src->frames[i].vertices, sizeof(mdx_triangleVertex_t)*md2Src->header.numVertices);
				}

				//frameBuffer
				mdxOut->framesBuffer = (byte *)malloc(sizeof(byte) * (md2Src->header.frameSize * md2Src->header.numFrames));
				if (mdxOut->framesBuffer)
				{
					memcpy(mdxOut->framesBuffer, md2Src->framesBuffer, sizeof(byte) * (md2Src->header.frameSize * md2Src->header.numFrames));
					
					//glCommand buffer
					mdxOut->glCommandBuffer = (int *)malloc(sizeof(int) * md2Src->header.numGlCommands);
					if (mdxOut->glCommandBuffer)
					{
						memcpy(mdxOut->glCommandBuffer, md2Src->glCommandBuffer, sizeof(int) * md2Src->header.numGlCommands);

						mdxOut->isMD2 = 1; //stop reading object index in glCommands
						//setModelIndex(); //hypov8

						md2_freeModel(md2Src);

						return mdxOut;
					}
				}
			}
		}
	}


	//must have failed!!
	mdx_freeModel(mdxOut);
	md2_freeModel(md2Src);
	return 0;
}



/*
 * free model
 */
void
md2_freeModel (md2_model_t *model)
{
	if (model)
	{
		if (model->skins)
			free (model->skins);

		if (model->texCoords)
			free (model->texCoords);

		if (model->triangles)
			free (model->triangles);

		if (model->frames)
		{
			int i;

			for (i = 0; i < model->header.numFrames; i++)
			{
				if (model->frames[i].vertices)
					free(model->frames[i].vertices);
			}
			free(model->frames);
		}

		if (model->framesBuffer)
			free(model->framesBuffer);

		if (model->glCommandBuffer)
			free (model->glCommandBuffer);


		free (model);
	}
}

