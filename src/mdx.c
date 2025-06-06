#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset */
#include <math.h> /* sqrt */
#include <mx/gl.h>
#include "common.h"
//#include "anorms.h"
//#include "mdx.h"

//static int g_glcmds = 1; /* use glcommands */
static int g_interp = 1; /* interpolate frames */


//md2 compat
int mdx_readFrameData(FILE *file, mdx_frame_t **pFrames, byte *buffer, 
	int numFrames, int numVertices, int frameSize, float*bboxMin, float*bboxMax,
	int offsetFrames, byte **framesBuffer,
	boolean isModel_HD) //HD
{
	int i, j, nIdx;
	mdx_frame_t *frames;
	char *v1, *v2, *fName;
	byte *v16;
	float fScale[3];
	mdx_alias_frame_t *frame;

	*pFrames = (mdx_frame_t *)calloc(numFrames, sizeof(mdx_frame_t));
	if (!pFrames)
		return 0;

	frames = (mdx_frame_t*)*pFrames;
	v1 = frames[0].name;
	v2 = frames[1].name;

	
	//hypov8 get bbox
	bboxMin[0] = bboxMin[1] = bboxMin[2] = 999999.0f;
	bboxMax[0] = bboxMax[1] = bboxMax[2] = -999999.0f;

	for (i = 0; i < numFrames; i++)
	{
		frame = (mdx_alias_frame_t *)buffer;
		fName = frames[i].name;

		fread(frame, 1, frameSize, file);
		memcpy(fName, frame->name, sizeof(frame->name)); //todo: null terminated?
		frames[i].vertices = (mdx_triangleVertex_t *)malloc(sizeof(mdx_triangleVertex_t) * numVertices);
		if (!frames[i].vertices)
			return 0;

		fScale[0] = frame->scale[0];
		fScale[1] = frame->scale[1];
		fScale[2] = frame->scale[2];
		if (isModel_HD == TRUE)//def KINGPIN_MDX_V5
		{
			v16 = (byte*)((byte*)frame->alias_vertices + numVertices * 4);
		}

		for (j = 0; j < numVertices; j++)
		{
			int v[3];
			v[0] = (int)frame->alias_vertices[j].vertex[0];
			v[1] = (int)frame->alias_vertices[j].vertex[1];
			v[2] = (int)frame->alias_vertices[j].vertex[2];

			if (isModel_HD == TRUE)//def KINGPIN_MDX_V5
			{		
				//copy vertex
				frames[i].vertices[j].vertex[2] = ((float)v[0]+((float)(char)v16[0] /256.0f)) * fScale[0] + frame->translate[0];
				frames[i].vertices[j].vertex[0] = ((float)v[1]+((float)(char)v16[1] /256.0f)) * fScale[1] + frame->translate[1];
				frames[i].vertices[j].vertex[1] = ((float)v[2]+((float)(char)v16[2] /256.0f)) * fScale[2] + frame->translate[2]; //z-far
				v16 += 3;
			}
			else
			{
				//copy vertex
				frames[i].vertices[j].vertex[2] = (float)v[0] * fScale[0] + frame->translate[0];
				frames[i].vertices[j].vertex[0] = (float)v[1] * fScale[1] + frame->translate[1];
				frames[i].vertices[j].vertex[1] = (float)v[2] * fScale[2] + frame->translate[2]; //z-far
			}

			//copy normal
			nIdx = frame->alias_vertices[j].lightNormalIndex;
			frames[i].vertices[j].normal[2] = avertexnormals[nIdx][0];
			frames[i].vertices[j].normal[0] = avertexnormals[nIdx][1];
			frames[i].vertices[j].normal[1] = avertexnormals[nIdx][2];

			//get model bbox min/max (frame0)
			if (i == 0)
			{
				bboxMin[0] = min(bboxMin[0], frames[0].vertices[j].vertex[2]);
				bboxMax[0] = max(bboxMax[0], frames[0].vertices[j].vertex[2]);
				bboxMin[1] = min(bboxMin[1], frames[0].vertices[j].vertex[0]); //z-far
				bboxMax[1] = max(bboxMax[1], frames[0].vertices[j].vertex[0]); //z-far
				bboxMin[2] = min(bboxMin[2], frames[0].vertices[j].vertex[1]);
				bboxMax[2] = max(bboxMax[2], frames[0].vertices[j].vertex[1]);
			}
		}
	}

	//hypov8 store frame data for later export
	fseek(file, offsetFrames, SEEK_SET);
	*framesBuffer = (byte *)malloc(sizeof(byte) * (frameSize * numFrames));
	if (!framesBuffer)
		return 0;
	fread(*framesBuffer, sizeof(byte), frameSize * numFrames, file);

	return 1;
}



/*
 * load model
 */
mdx_model_t*
mdx_readModel (const char *filename, int debugLoad)
{
	FILE *file=NULL;
	mdx_model_t *model;
	byte *buffer; // [MDL_MAX_FRAMESIZE];
	long fLen;
	boolean isModel_HD = FALSE;//def KINGPIN_MDX_V5

	model = (mdx_model_t *)malloc(sizeof(mdx_model_t));
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
	memset (model, 0, sizeof (mdx_model_t));
	fread (&model->header, sizeof (mdx_header_t), 1, file);

	fseek(file, 0L, SEEK_END);
	fLen = ftell(file);

#if 0
	printf ("magic:\t\t%d\n", model->header.magic);
	printf ("version:\t\t%d\n", model->header.version);
	printf ("skinWidth:\t\t%d\n", model->header.skinWidth);
	printf ("skinHeight:\t\t%d\n", model->header.skinHeight);
	printf ("frameSize:\t\t%d\n", model->header.frameSize);
	printf ("numSkins:\t\t%d\n", model->header.numSkins);
	printf ("numTriangles:\t\t%d\n", model->header.numTriangles);
	printf ("numGlCommands:\t\t%d\n", model->header.numGlCommands);
	printf ("numFrames:\t\t%d\n", model->header.numFrames);
	printf ("offsetSkins:\t\t%d\n", model->header.offsetSkins);
	printf ("offsetTriangles:\t%d\n", model->header.offsetTriangles);
	printf ("offsetFrames:\t\t%d\n", model->header.offsetFrames);
	printf ("offsetGlCommands:\t%d\n", model->header.offsetGlCommands);
	printf ("offsetEnd:\t\t%d\n", model->header.offsetEnd);
#endif
	if (!debugLoad && (
		model->header.magic != (int)(('X' << 24) + ('P' << 16) + ('D' << 8) + 'I') ||
		//do more checks
		model->header.numGlCommands < 1 || //hypov8 was 4
		fLen < model->header.offsetEnd || model->header.offsetEnd <= 0 ||
		fLen < model->header.offsetFrames || model->header.offsetFrames <= 0 ||
		fLen < model->header.offsetGlCommands || model->header.offsetGlCommands <= 0 ||
		fLen < model->header.offsetSkins || model->header.offsetSkins <= 0 ||
		fLen < model->header.offsetTriangles || model->header.offsetTriangles <= 0 ||
		fLen < model->header.offsetBBoxFrames || model->header.offsetBBoxFrames <= 0 ||
		fLen < model->header.offsetVertexInfo || model->header.offsetVertexInfo <= 0))
	{
		fclose (file);
		free (model);
		return 0;
	}

	/* read skins */
	fseek (file, model->header.offsetSkins, SEEK_SET);
	if (model->header.numSkins > 0)
	{
		model->skins = (mdx_skin_t *)malloc(sizeof(mdx_skin_t) * model->header.numSkins);
		if (!model->skins)
		{
			fclose(file); //hypov8
			mdx_freeModel (model);
			return 0;
		}
		fread (model->skins, sizeof (mdx_skin_t), model->header.numSkins, file);
		//for (i = 0; i < model->header.numSkins; i++)
		//	fread (&model->skins[i], sizeof (mdx_skin_t), 1, file);
	}

	/* read triangles */
	fseek (file, model->header.offsetTriangles, SEEK_SET);
	if (model->header.numTriangles > 0)
	{
		model->triangles = (mdx_triangle_t *) malloc (sizeof (mdx_triangle_t) * model->header.numTriangles);
		if (!model->triangles)
		{
			fclose(file); //hypov8
			mdx_freeModel (model);
			return 0;
		}

		fread (model->triangles, sizeof (mdx_triangle_t), model->header.numTriangles, file);
		//for (i = 0; i < model->header.numTriangles; i++)
		//	fread (&model->triangles[i], sizeof (mdx_triangle_t), 1, file);
	}

	//def KINGPIN_MDX_V5
	if (model->header.frameSize == (int)(40 + model->header.numVertices*4 + model->header.numVertices*3))
		isModel_HD = TRUE;

	//hypov8 moved to dynamic buffer. no model size limit
	buffer = malloc(sizeof(mdx_alias_frame_t) + sizeof(mdx_alias_triangleVertex_t)*model->header.numVertices+ 
		sizeof(byte)*model->header.numVertices*3);	//def KINGPIN_MDX_V5
	if (!buffer)
	{
		fclose(file); //hypov8
		mdx_freeModel (model);
		return 0;
	}

	/* read alias frames */
	fseek (file, model->header.offsetFrames, SEEK_SET);
	if (model->header.numFrames > 0)
	{
		if (!mdx_readFrameData(file, &model->frames, buffer, model->header.numFrames, model->header.numVertices, 
			model->header.frameSize, model->bBoxMin, model->bBoxMax, model->header.offsetFrames, &model->framesBuffer, 
			isModel_HD)) //HD
		{
			fclose(file); //hypov8
			mdx_freeModel (model);
			free(buffer); //def KINGPIN_MDX_V5
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
			fclose(file); //hypov8
			mdx_freeModel (model);
			free(buffer); //def KINGPIN_MDX_V5
			return 0;
		}

		fread (model->glCommandBuffer, sizeof (int), model->header.numGlCommands, file);
	}

	/* read hitbox data */
	if (model->header.numSubObjects)
	{
		fseek(file, model->header.offsetBBoxFrames, SEEK_SET);
		model->hitBox = (float*)malloc(sizeof(float) * 6 * model->header.numFrames * model->header.numSubObjects);
		if (!model->hitBox)
		{
			fclose(file); //hypov8
			mdx_freeModel(model);
			free(buffer); //def KINGPIN_MDX_V5
			return 0;
		}
		fread(model->hitBox, sizeof(float)*6, model->header.numFrames*model->header.numSubObjects, file);
	}

	fclose (file);

	free(buffer); //def KINGPIN_MDX_V5

	model->isMD2 = 0;

	return model;
}



/*
 * free model
 */
void
mdx_freeModel (mdx_model_t *model)
{
	if (model)
	{
		if (model->skins)
			free (model->skins);

		if (model->triangles)
			free (model->triangles);

		if (model->frames)
		{
			int i;

			for (i = 0; i < model->header.numFrames; i++)
			{
				if (model->frames[i].vertices)
					free (model->frames[i].vertices);
			}
			free (model->frames);
		}

		if (model->framesBuffer)
			free(model->framesBuffer);

		if (model->glCommandBuffer)
			free (model->glCommandBuffer);

		if (model->hitBox)
			free(model->hitBox);

		free (model);
	}
}



/*
 * set draw style, either with glcommands, interpolated
 */
void
mdx_setStyle (int glcmds, int interp)
{
	//g_glcmds = glcmds;
	g_interp = interp;
}



/*
 * center model 
 *
 */
void
mdx_getBoundingBox (mdx_model_t *model, float *outMin, float *outMax, int frame)
{
#if 1
	int i, j;
	float min[3], max[3];

	if (frame >= model->header.numFrames)
		frame = model->header.numFrames - 1;
	 
	min[0] = min[1] = min[2] =  999999.0f;
	max[0] = max[1] = max[2] = -999999.0f;

	/* get bounding box */
	for (i = 0; i < model->header.numVertices; i++)
	{
		mdx_triangleVertex_t *v = &model->frames[frame].vertices[i];

		for (j = 0; j < 3; j++)
		{
			if (v->vertex[j] < min[j])	
				min[j] = v->vertex[j];
			else if (v->vertex[j] > max[j])
				max[j] = v->vertex[j];

			//hypov8 properly centre model
			/*if (j !=1) //dont centre vertical (z is fov)
			{
				if (-v->vertex[j] < min[j])
					min[j] = -v->vertex[j];
				else if (-v->vertex[j] > max[j])
					max[j] = -v->vertex[j];
			}*/

		}
	}
	outMin[0] = min[0];
	outMin[1] = min[1];
	outMin[2] = min[2];

	outMax[0] = max[0];
	outMax[1] = max[1];
	outMax[2] = max[2];
#else
	int i;
	float minx, maxx;
	float miny, maxy;
	float minz, maxz;
	minx = miny = minz = 999999.0f;
	maxx = maxy = maxz = -999999.0f;

	/* get bounding box */
	for (i = 0; i < model->header.numVertices; i++)
	{
		mdx_triangleVertex_t *v = &model->frames[frame].vertices[i];

		if (v->vertex[0] < minx)
			minx = v->vertex[0];
		else if (v->vertex[0] > maxx)
			maxx = v->vertex[0];

		if (v->vertex[1] < miny)
			miny = v->vertex[1];
		else if (v->vertex[1] > maxy)
			maxy = v->vertex[1];

		if (v->vertex[2] < minz)
			minz = v->vertex[2];
		else if (v->vertex[2] > maxz)
			maxz = v->vertex[2];

	minmax[0] = minx;
	minmax[1] = maxx;
	minmax[2] = miny;
	minmax[3] = maxy;
	minmax[4] = minz;
	minmax[5] = maxz;
	}
#endif

}

/*
* center model *
*/
void
mdx_getBoundingBoxExport(mdx_model_t *model, float *minmax, int frame)
{

	int i;
	float min[3] = { 999999.0f, 999999.0f, 999999.0f};
	float max[3] = { -999999.0f, -999999.0f, -999999.0f};

	/* get bounding box */
	for (i = 0; i < model->header.numVertices; i++)
	{
		mdx_triangleVertex_t *v = &model->frames[frame].vertices[i];

		if (v->vertex[0] < min[0])
			min[0] = v->vertex[0];
		else if (v->vertex[0] > max[0])
			max[0] = v->vertex[0];

		if (v->vertex[1] < min[1])
			min[1] = v->vertex[1];
		else if (v->vertex[1] > max[1])
			max[1] = v->vertex[1];

		if (v->vertex[2] < min[2])
			min[2] = v->vertex[2];
		else if (v->vertex[2] > max[2])
			max[2] = v->vertex[2];
	}
	minmax[0] = min[2];
	minmax[1] = min[0];
	minmax[2] = min[1];

	minmax[3] = max[2];
	minmax[4] = max[0];
	minmax[5] = max[1];

	/*minmax[0] = minx;
	minmax[1] = maxx;
	minmax[2] = miny;
	minmax[3] = maxy;
	minmax[4] = minz;
	minmax[5] = maxz;*/
}


/*
 * draw with glcommands
 *
 */
void
mdx_drawModel_gl(mdx_model_t *model, int frame1, int frame2, float pol, int lerp, int isMissingFrame)
{
	int i = 0;
	int something;
	int val = model->glCommandBuffer[i++];
	float rgba[4];
	float x1, y1, z1;
	float x2, y2, z2;
	int count;

	glGetFloatv(GL_CURRENT_COLOR, rgba);
	if (isMissingFrame && frame2 > model->header.numFrames)
		glColor3f(1.0f, 0.0f, 0.0f); //set red

#if 0 //disabled. software texture loading
	//todo:
	// cant export mdx without glCommands.. disabled
	if (model->isMD2 && !model->header.numGlCommands)
	{
		glBegin(GL_TRIANGLES);
		for (i = 0; i < model->header.numTriangles; i++)
		{
			int vIdcX = model->triangles[i].vertexIndices[0];
			int vIdcY = model->triangles[i].vertexIndices[1];
			int vIdcZ = model->triangles[i].vertexIndices[2];

			float *v_X = model->frames[frame1].vertices[vIdcX].vertex;
			float *n_X = model->frames[frame1].vertices[vIdcX].normal;
			float *v_Y = model->frames[frame1].vertices[vIdcY].vertex;
			float *n_Y = model->frames[frame1].vertices[vIdcY].normal;
			float *v_Z = model->frames[frame1].vertices[vIdcZ].vertex;
			float *n_Z = model->frames[frame1].vertices[vIdcZ].normal;
			float s = model->;
			float t = ;

			glTexCoord2f((float)model->texCoords[t->textureIndices[2]].s / (float)model->header.skinWidth,
				(float)model->texCoords[t->textureIndices[2]].t / (float)model->header.skinHeight);

			glTexCoord2f(s, t);
			glNormal3f(x1, y1, z1);
			glVertex3f(x2, y2, z2);


			for (j = 0; j < 3; j++)
			{ }

				//normal dir
			x1 = model->frames[frame1].vertices[i].normal[0];
			y1 = model->frames[frame1].vertices[index].normal[1];
			z1 = model->frames[frame1].vertices[index].normal[2];
			//vertex pos			
			x2 = model->frames[frame1].vertices[index].vertex[0];
			y2 = model->frames[frame1].vertices[index].vertex[1];
			z2 = model->frames[frame1].vertices[index].vertex[2];
		}
		glEnd();
	}
#endif

	while (val != 0)
	{
		if (!model->isMD2)
			something = model->glCommandBuffer[i++]; //mdx. read object number
		
		if (val > 0 && val <= model->header.numGlCommands) //hypov8 add check
		{
			glBegin(GL_TRIANGLE_STRIP);
			count = val;
		}
		else if (val < 0 && -val <= model->header.numGlCommands) //hypov8 add check
		{
			glBegin(GL_TRIANGLE_FAN);
			count = -val;
		}
		else 
			break;

		while (count--)
		{
			float s = *(float *) &model->glCommandBuffer[i++];
			float t = *(float *) &model->glCommandBuffer[i++];
			int index = model->glCommandBuffer[i++];

			//hypov8 invalid model
			if (index < 0 || index > model->header.numVertices)
			{
				glEnd();
				return;
			}

			//normal dir
			x1 = model->frames[frame1].vertices[index].normal[0];
			y1 = model->frames[frame1].vertices[index].normal[1];
			z1 = model->frames[frame1].vertices[index].normal[2];
			//vertex pos			
			x2 = model->frames[frame1].vertices[index].vertex[0];
			y2 = model->frames[frame1].vertices[index].vertex[1];
			z2 = model->frames[frame1].vertices[index].vertex[2];

			//interpolate
			if (lerp)	
			{	//normal dir
				x1 += pol * (model->frames[frame2].vertices[index].normal[0] - x1);
				y1 += pol * (model->frames[frame2].vertices[index].normal[1] - y1);
				z1 += pol * (model->frames[frame2].vertices[index].normal[2] - z1);
				//vertex pos				
				x2 += pol * (model->frames[frame2].vertices[index].vertex[0] - x2);
				y2 += pol * (model->frames[frame2].vertices[index].vertex[1] - y2);
				z2 += pol * (model->frames[frame2].vertices[index].vertex[2] - z2);
			}

			glTexCoord2f (s, t);
			glNormal3f(x1, y1, z1);
			glVertex3f(x2, y2, z2);
		}

		glEnd ();
		val = model->glCommandBuffer[i++];
	}


	if (isMissingFrame)
		glColor3f(rgba[0], rgba[1], rgba[2]); //reset to white
}


//HYPOVERTEX
void mdx_drawModel_dev_gl(mdx_model_t *model, int frame, int frame2, float pol, int lerp, int vertID, int useFace, int showVN, int showGrid, int showHitBox, 
	float *rgb, float *grid_rgb)
{
	float x1, y1, z1;
	float x2, y2, z2;
	float nx, ny, nz;
	int i, j, reset = 0;
	GLboolean isLight[1], isLight0[1], isTex[1];
	float rgba[4];

	//skip
	if (vertID == -1 && !showVN && !showGrid && !showHitBox)
		return;
	
	// get previous values	
	glGetBooleanv(GL_LIGHTING, isLight);
	glGetBooleanv(GL_LIGHT0, isLight0);
	glGetBooleanv(GL_TEXTURE_2D, isTex);
	glGetFloatv(GL_CURRENT_COLOR, rgba);

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_TEXTURE_2D);
	glPointSize(5);

	glColor3f(rgb[0], rgb[1], rgb[2]); //set red
	if (vertID > -1 && 
		((!useFace && vertID < model->header.numVertices) || ( useFace && vertID < model->header.numTriangles)) )
	{
		reset = 1;
		glBegin(GL_POINTS);


		if (!useFace)
		{
			//use vertex pos
			x1 = model->frames[frame].vertices[vertID].vertex[0];
			y1 = model->frames[frame].vertices[vertID].vertex[1];
			z1 = model->frames[frame].vertices[vertID].vertex[2];
			
			if (lerp && frame2 > -1)
			{	//interpolate
				x1 += pol * (model->frames[frame2].vertices[vertID].vertex[0] - x1);
				y1 += pol * (model->frames[frame2].vertices[vertID].vertex[1] - y1);
				z1 += pol * (model->frames[frame2].vertices[vertID].vertex[2] - z1);
			}
		}
		else
		{
			//get triangle middle
			int id0 = model->triangles[vertID].vertexIndices[0];
			int id1 = model->triangles[vertID].vertexIndices[1];
			int id2 = model->triangles[vertID].vertexIndices[2];
			float *v1 = model->frames[frame].vertices[id0].vertex;
			float *v2 = model->frames[frame].vertices[id1].vertex;
			float *v3 = model->frames[frame].vertices[id2].vertex;

			x1 = (v1[0]+v2[0]+v3[0]) / 3.0f;
			y1 = (v1[1]+v2[1]+v3[1]) / 3.0f;
			z1 = (v1[2]+v2[2]+v3[2]) / 3.0f;
			
			if (frame2 > -1)
			{	//interpolate
				float *v1_ = model->frames[frame2].vertices[id0].vertex;
				float *v2_ = model->frames[frame2].vertices[id1].vertex;
				float *v3_ = model->frames[frame2].vertices[id2].vertex;
				x2 = (v1_[0]+v2_[0]+v3_[0]) / 3.0f;
				y2 = (v1_[1]+v2_[1]+v3_[1]) / 3.0f;
				z2 = (v1_[2]+v2_[2]+v3_[2]) / 3.0f;

				x1 += pol * (x2 - x1);
				y1 += pol * (y2 - y1);
				z1 += pol * (z2 - z1);
			}
		}
		glVertex3f(x1, y1, z1);
		glEnd();
	}

	//hypov8 vertexnorms
	if (showVN)
	{
		reset = 1;	
		glBegin(GL_LINES);
		//glColor3f(1.0f, 0.2f, 0.2f); //set red
		for (i = 0; i < model->header.numVertices; i++)
		{
			for (j = 0; j < 3; j++)
			{
				//start poing of normal(vertex)
				x1 = model->frames[frame].vertices[i].vertex[0];
				y1 = model->frames[frame].vertices[i].vertex[1];
				z1 = model->frames[frame].vertices[i].vertex[2];
				//normal direction
				nx = model->frames[frame].vertices[i].normal[0];
				ny = model->frames[frame].vertices[i].normal[1];
				nz = model->frames[frame].vertices[i].normal[2];

				if (lerp && frame2 > -1)
				{	//interpolate
					x1 += pol * (model->frames[frame2].vertices[i].vertex[0] - x1);
					y1 += pol * (model->frames[frame2].vertices[i].vertex[1] - y1);
					z1 += pol * (model->frames[frame2].vertices[i].vertex[2] - z1);
					//
					nx += pol * (model->frames[frame2].vertices[i].normal[0] - nx);
					ny += pol * (model->frames[frame2].vertices[i].normal[1] - ny);
					nz += pol * (model->frames[frame2].vertices[i].normal[2] - nz);
				}

				//end point
				x2 = x1 + nx;
				y2 = y1 + ny;
				z2 = z1 + nz;

				glVertex3f(x1, y1, z1); //normal start pos
				glVertex3f(x2, y2, z2); //normal end pos
			}
		}
		glEnd();
	}

	//hitbox
	if (showHitBox && !model->isMD2)
	{
		int i, j;
		int iObj = model->header.numSubObjects;
		int fSize = model->header.numFrames * 6;
		int fOfs1 = frame * 6; //frame1
		int fOfs2= frame2 * 6; //frame2
		float *bbox, minMax[6];
		static int cube[6][12] = {
			//vert 1		vert2			vert 3			vert 4
			{ 0, 1, 2,		0, 4, 2,		0, 4, 5,		0, 1, 5 }, //face 1
			{ 0, 1, 2,		0, 1, 5,		3, 1, 5,		3, 1, 2 }, //face 2
			{ 0, 1, 2,		3, 1, 2,		3, 4, 2,		0, 4, 2 }, //face 3
			{ 3, 4, 5,		0, 4, 5,		0, 4, 2,		3, 4, 2 }, //face 4
			{ 3, 4, 5,		3, 4, 2,		3, 1, 2,		3, 1, 5 }, //face 5
			{ 3, 4, 5,		3, 1, 5,		0, 1, 5,		0, 4, 5 }, //face 6
		};

		glColor4f(rgb[0], rgb[1], rgb[2], 0.3f); //set red
		//glEnable(GL_BLEND); //Enable blending.
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glBegin(GL_QUADS);
		for (i = 0; i < iObj; i++)
		{
			bbox = &model->hitBox[fSize * i + fOfs1];
			//convert cordanates. Y up
			minMax[2] = bbox[0]; 
			minMax[0] = bbox[1];
			minMax[1] = bbox[2];
			minMax[5] = bbox[3];
			minMax[3] = bbox[4];
			minMax[4] = bbox[5];
			//lerp?
			if (lerp && frame2 > -1)
			{
				bbox = &model->hitBox[fSize * i + fOfs2];
				minMax[2] += pol * (bbox[0]- minMax[2]); 
				minMax[0] += pol * (bbox[1] - minMax[0]);
				minMax[1] += pol * (bbox[2] - minMax[1]);
				minMax[5] += pol * (bbox[3] - minMax[5]);
				minMax[3] += pol * (bbox[4] - minMax[3]);
				minMax[4] += pol * (bbox[5] - minMax[4]);
			}

			for (j = 0; j < 6; j++)
			{
				glBegin(GL_LINE_LOOP);
				glVertex3f(minMax[cube[j][0]], minMax[cube[j][1]], minMax[cube[j][2]]);
				glVertex3f(minMax[cube[j][3]], minMax[cube[j][4]], minMax[cube[j][5]]);
				glVertex3f(minMax[cube[j][6]], minMax[cube[j][7]], minMax[cube[j][8]]);
				glVertex3f(minMax[cube[j][9]], minMax[cube[j][10]], minMax[cube[j][11]]);
				glEnd();
			}
		}
		//glEnd();
		//glDisable(GL_BLEND);
	} //end hitbox


	if (showGrid)
	{
		float ofs = -40.0f;
		glBegin(GL_LINES);
		
		glColor3f(grid_rgb[0], grid_rgb[1], grid_rgb[2]); //set grey

		//draw x/z lines
		for (i = 0; i < 9; i++)
		{
			if (ofs == 0.0f) //set darker
				glColor3f((grid_rgb[0] * 0.5f), (grid_rgb[1] * 0.5f), (grid_rgb[2] * 0.5f));

			glVertex3f(ofs, 0.0f, -40.0f);
			glVertex3f(ofs, 0.0f, 40.0f);
			glVertex3f(-40.0f, 0.0f, ofs);
			glVertex3f(40.0f, 0.0f, ofs);
	
			if (ofs == 0.0f) //reset color
				glColor3f(grid_rgb[0], grid_rgb[1], grid_rgb[2]);

			ofs += 10.0f;
		}
		glEnd();
	}

	//reset values to previous
	if (isLight[0] != 0)
		glEnable(GL_LIGHTING);
	if (isLight0[0] != 0)
		glEnable(GL_LIGHT0);
	if (isTex[0] != 0)
		glEnable(GL_TEXTURE_2D);
	glColor3f(rgba[0], rgba[1], rgba[2]); //reset to white
}
//END_HYPOVERTEX

/*
 * draw model
 */
void mdx_drawModel (mdx_model_t *model, int frame1, int frame2, float pol, int noLerp, int noData)
{
	if (g_interp && !noLerp && frame1 != frame2)
		mdx_drawModel_gl(model, frame1, frame2, pol, 1, 0);
	else
		mdx_drawModel_gl(model, frame1, frame2, pol, 0, noData);
}

//HYPOVERTEX
void mdx_drawModel_dev (mdx_model_t *model, int frame1, int frame2, float pol, int lerp, int vertID, int useFace, int showVN, int showGrid, int showHitBox, 
	float *rgb, float *grid_rgb)
{
	if (g_interp && lerp)
		mdx_drawModel_dev_gl(model, frame1, frame2, pol, 1, vertID, useFace, showVN, showGrid, showHitBox, rgb, grid_rgb);
	else
		mdx_drawModel_dev_gl(model, frame1,     -1,  -1, 0, vertID, useFace, showVN, showGrid, showHitBox, rgb, grid_rgb);
}
//END

void mdx_makeFacetNormal (mdx_frame_t *frames, mdx_triangle_t *t, int fIdx, float *fn)
{
	float v1[3], v2[3];
	double angle;

	v1[0] = frames[fIdx].vertices[t->vertexIndices[1]].vertex[0] - frames[fIdx].vertices[t->vertexIndices[0]].vertex[0];
	v1[1] = frames[fIdx].vertices[t->vertexIndices[1]].vertex[1] - frames[fIdx].vertices[t->vertexIndices[0]].vertex[1];
	v1[2] = frames[fIdx].vertices[t->vertexIndices[1]].vertex[2] - frames[fIdx].vertices[t->vertexIndices[0]].vertex[2];

	v2[0] = frames[fIdx].vertices[t->vertexIndices[2]].vertex[0] - frames[fIdx].vertices[t->vertexIndices[0]].vertex[0];
	v2[1] = frames[fIdx].vertices[t->vertexIndices[2]].vertex[1] - frames[fIdx].vertices[t->vertexIndices[0]].vertex[1];
	v2[2] = frames[fIdx].vertices[t->vertexIndices[2]].vertex[2] - frames[fIdx].vertices[t->vertexIndices[0]].vertex[2];

	angle = 1;

	fn[0] = (v1[1] * v2[2] - v1[2] * v2[1]) * (float) angle;
	fn[1] = (v1[2] * v2[0] - v1[0] * v2[2]) * (float) angle;
	fn[2] = (v1[0] * v2[1] - v1[1] * v2[0]) * (float) angle;
}



void mdx_normalize (float *n)
{
	float l = (float) sqrt (n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);

	if (l != 0.0f)
	{
		n[0] /= l;
		n[1] /= l;
		n[2] /= l;
	}
}



void
mdx_generateLightNormals (mdx_model_t *model)
{
	int i;

	if (!model)
		return;


	for (i = 0; i < model->header.numFrames; i++)
	{
		int j;

		/* clear all normals */
		for (j = 0; j < model->header.numVertices; j++)
		{
			model->frames[i].vertices[j].normal[0] = 0.0f;
			model->frames[i].vertices[j].normal[1] = 0.0f;
			model->frames[i].vertices[j].normal[2] = 0.0f;
		}

		/* calc normals */
		for (j = 0; j < model->header.numTriangles; j++)
		{
			int k;
			float facetnormal[3];

			mdx_makeFacetNormal (model->frames, &model->triangles[j], i, facetnormal);

			for (k = 0; k < 3; k++)
			{
				model->frames[i].vertices[model->triangles[j].vertexIndices[k]].normal[0] += -facetnormal[0];
				model->frames[i].vertices[model->triangles[j].vertexIndices[k]].normal[1] += -facetnormal[1];
				model->frames[i].vertices[model->triangles[j].vertexIndices[k]].normal[2] += -facetnormal[2];
			}
		}

		/* normalize normals */
		for (j = 0; j < model->header.numVertices; j++)
			mdx_normalize (model->frames[i].vertices[j].normal);
	}
}



int
mdx_getAnimationCount (mdx_model_t *model)
{
	int i, j, pos;
	int count;
	int lastId;
	char name[32], last[32];

	//strcpy_s (last, sizeof(model->frames[0].name), model->frames[0].name);
	memcpy(last, model->frames[0].name, sizeof(model->frames[0].name));
	pos = strlen (last) - 1;
	j = 0;
	while (last[pos] >= '0' && last[pos] <= '9' && j < 2)
	{
		pos--;
		j++;
	}
	last[pos + 1] = '\0';

	lastId = 0;
	count = 0;

	for (i = 0; i <= model->header.numFrames; i++)
	{
		if (i == model->header.numFrames)
			name[0] = '\0'; // some kind of a sentinel
		else
		{
			//strcpy_s (name, sizeof(model->frames[i].name), model->frames[i].name);
			memcpy(name, model->frames[i].name, sizeof(model->frames[i].name));
			name[16] = '\0'; //str not allways null terminated
		}
		pos = strlen (name) - 1;
		j = 0;
		while (name[pos] >= '0' && name[pos] <= '9' && j < 2)
		{
			pos--;
			j++;
		}
		name[pos + 1] = '\0';

		if (strcmp (last, name))
		{
			strcpy_s (last, sizeof(last), name);
			count++;
		}
	}

	return count;
}



const char *
mdx_getAnimationName (mdx_model_t *model, int animation)
{
	int i, j, pos;
	int count, prev;
	int lastId;
	static char last[32];
	char name[32];

	//strcpy_s (last, sizeof(model->frames[0].name), model->frames[0].name);
	memcpy(last, model->frames[0].name, sizeof(model->frames[0].name));
	pos = strlen (last) - 1;
	j = 0;
	while (last[pos] >= '0' && last[pos] <= '9' && j < 2)
	{
		pos--;
		j++;
	}
	last[pos + 1] = '\0';

	lastId = 0;
	count = 0;
	prev = 0;

	for (i = 0; i <= model->header.numFrames; i++)
	{
		if (i == model->header.numFrames)
			name[0] = '\0';// some kind of a sentinel
		else
		{
			//strcpy_s (name, sizeof(model->frames[i].name), model->frames[i].name);
			memcpy(name, model->frames[i].name, sizeof(model->frames[i].name));
			name[16] = '\0'; //str not allways null terminated
		}
		pos = strlen (name) - 1;
		j = 0;
		while (name[pos] >= '0' && name[pos] <= '9' && j < 2)
		{
			pos--;
			j++;
		}
		name[pos + 1] = '\0';

		//if (animation == count - 1)
		//{
		//	if (prev == -1)
		//		prev = i;
		//	else
		//		prev += 1;
		//}

		if (strcmp (last, name))
		{
			if (count == animation)
			{
				sprintf_s(last, sizeof(last), "%s  (%i-%i)", last, prev, i - 1); //hypov8 add
				return last;
			}

			strcpy_s (last, sizeof(last), name);
			count++;
			prev = i;
		}
	}

	return 0;
}



void
mdx_getAnimationFrames (mdx_model_t *model, int animation, int *startFrame, int *endFrame)
{
	int i, j, pos;
	int count, numFrames, frameCount;
	int lastId;
	char name[32], last[32];

	//strcpy_s (last, sizeof(model->frames[0].name), model->frames[0].name);
	memcpy(last, model->frames[0].name, sizeof(model->frames[0].name));
	last[16] = '\0'; //str not allways null terminated

	pos = strlen (last) - 1;
	j = 0;
	while (last[pos] >= '0' && last[pos] <= '9' && j < 2)
	{
		pos--;
		j++;
	}
	last[pos + 1] = '\0';

	lastId = 0;
	count = 0;
	numFrames = 0;
	frameCount = 0;

	for (i = 0; i <= model->header.numFrames; i++)
	{
		if (i == model->header.numFrames)
			name[0] = '\0'; // some kind of a sentinel
		else
		{
			//strcpy_s (name, sizeof(model->frames[i].name), model->frames[i].name);
			memcpy(name, model->frames[i].name, sizeof(model->frames[i].name));
			name[16] = '\0'; //str not allways null terminated
		}
		pos = strlen (name) - 1;
		j = 0;
		while (name[pos] >= '0' && name[pos] <= '9' && j < 2)
		{
			pos--;
			j++;
		}
		name[pos + 1] = '\0';

		if (strcmp (last, name))
		{
			strcpy_s (last, sizeof(last), name);

			if (count == animation)
			{
				*startFrame = frameCount - numFrames;
				*endFrame = frameCount - 1;
				return;
			}

			count++;
			numFrames = 0;
		}
		frameCount++;
		numFrames++;
	}


	*startFrame = *endFrame = 0;
}


