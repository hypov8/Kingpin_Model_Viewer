#include <mx/mx.h>
#include <mx/mxMessageBox.h>
#include <mx/mxTga.h>
#include <mx/mxPcx.h>
#include <mx/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "GlWindow.h"
//#include "common.h"
//#include "mdx.h"
//#include "md2.h"


//char modelFileNames[MAX_MODELS][256]; //hypov8 models
//char modelTexNames[MAX_TEXTURES][256]; //hypov8 textures


float fps; //todo. limit framerate


GlWindow::GlWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style)
: mxGlWindow (parent, x, y, w, h, label, style)
{
	d_rotX = d_rotY = 0;
	d_transX = d_transY = 0;
	d_transZ = 50;
	d_models[0] = 0;
	d_models[1] = 0;
	d_models[2] = 0;
	d_models[3] = 0;
	d_models[4] = 0;
	d_models[5] = 0;
	d_textureNames[0] = 0;
	d_textureNames[1] = 0;
	d_textureNames[2] = 0;
	d_textureNames[3] = 0;
	d_textureNames[4] = 0;
	d_textureNames[5] = 0;
	d_textureNames[6] = 0;
	d_textureNames[7] = 0;


	
	setFrameInfo (0, 0);
	setRenderMode (0); //reset view to wireframe
	setFlag (F_WATER, false);
	setFlag (F_LIGHT, false);
	setFlag (F_SHININESS, false);
	setFlag (F_INTERPOLATE, true);
	setFlag (F_GLCOMMANDS, true);
	setFlag (F_PAUSE, false); //hypov8
	setFlag (F_BACKGROUND, false);
	setFlag (F_VNORMS, false); //hypov8
	setFlag (F_GRID, false); //hypov8
	setFlag (F_HITBOX, false); //hypov8

	setPitch (125.0f);
	setBGColor (0.5f, 0.5f, 0.5f); //hypov8 background. was black
	setFGColor (1.0f, 1.0f, 1.0f);
	setWFColor (1.0f, 1.0f, 1.0f);
	setLightColor (1.0f, 1.0f, 1.0f);
	setDebugColor (1.0f, 0.0f, 0.0f); //hypov8 red
	setGridColor (0.35f, 0.35f, 0.35f); //hypov8 dark grey
	setBrightness (5);
	setTextureLimit (512);

	//setModelIndex(); //hypov8
	d_modelIndex = 0; //hypov8
	d_debugLoad = 0; //hypov8 debug model loading
	memset(modelFileNames, 0, sizeof(modelFileNames)); //hypov8
	memset(modelTexNames, 0, sizeof(modelTexNames)); //hypov8

	d_vertexIndex = -1; //HYPOVERTEX
	d_vertexUseFace = 0; //HYPOVERTEX

//	loadTexture ("water.tga", TEXTURE_WATER);

	glCullFace (GL_FRONT);

	mx::setIdleWindow (this);
}


//hypov8 updated:
GlWindow::~GlWindow ()
{
	mx::setIdleWindow (0);
	loadModel (0, TEXTURE_MODEL_0);
	loadModel(0, TEXTURE_MODEL_1);
	loadModel(0, TEXTURE_MODEL_2);
	loadModel(0, TEXTURE_MODEL_3);
	loadModel(0, TEXTURE_MODEL_4);
	loadModel(0, TEXTURE_MODEL_5);

	loadTexture (0, TEXTURE_MODEL_0); 
	loadTexture (0, TEXTURE_MODEL_1);
	loadTexture(0, TEXTURE_MODEL_2);
	loadTexture(0, TEXTURE_MODEL_3);
	loadTexture(0, TEXTURE_MODEL_4);
	loadTexture(0, TEXTURE_MODEL_5);

	loadTexture (0, TEXTURE_BACKGROUND);
	loadTexture (0, TEXTURE_WATER);

}



int
GlWindow::handleEvent (mxEvent *event)
{
	static float oldrx = 0, oldry = 0, oldtz = 50, oldtx = 0, oldty = 0;
	static int oldx, oldy;

	switch (event->event)
	{
	case mxEvent::MouseDown:
		oldrx = d_rotX;
		oldry = d_rotY;
		oldtx = d_transX;
		oldty = d_transY;
		oldtz = d_transZ;
		oldx = event->x;
		oldy = event->y;
		//setFlag (F_PAUSE, false);

		break;

	case mxEvent::MouseWheel: //hypov8 not working when other gui elements are active
		{
			if (event->zdelta<0)
				d_transZ += 10;
			else
				d_transZ -= 10;
			redraw();
		}
		break;

	case mxEvent::MouseDrag:
		if (event->buttons & mxEvent::MouseLeftButton)
		{
			if (event->modifiers & mxEvent::KeyShift)
			{
				d_transX = oldtx - (float) (event->x - oldx);
				d_transY = oldty + (float) (event->y - oldy);
			}
			else
			{
				d_rotX = oldrx + (float) (event->y - oldy);
				d_rotY = oldry + (float) (event->x - oldx);
			}
		}
		else if (event->buttons & mxEvent::MouseRightButton)
		{
			d_transZ = oldtz + ((float)(event->y - oldy) / 2);
		}
		else if (event->buttons & mxEvent::MouseMiddleButton)
		{
			d_transX = oldtx - ((float)(event->x - oldx) / 10);
			d_transY = oldty + ((float)(event->y - oldy) / 10);
		}

		redraw ();
		break;

	case mxEvent::Idle:
	{
		static DWORD timer = 0, lastTimer = 0;

		if (getFlag (F_PAUSE))
			return 0;
		DWORD tmp = mx::getTickCount();
		if (tmp - 30 < lastTimer) //hypov8 limit to 60 fps?
			return 0;

		lastTimer = timer;
		timer = mx::getTickCount ();

		float diff = (float) (timer - lastTimer);
		fps = 1 / (diff / 1000.0f);
		d_pol += diff / d_pitch;

		if (d_pol < 0.0f)
			d_pol = 0.0f; //hypov8 failsafe

		if (d_pol > 1.0f)
		{
			d_pol = 0.0f;
			d_currFrame++;
			d_currFrame2++;

			if (d_currFrame > d_endFrame)
				d_currFrame = d_startFrame;

			if (d_currFrame2 > d_endFrame)
				d_currFrame2 = d_startFrame;
		}

		redraw ();
	}
	break;

	case mxEvent::KeyDown:
	{
		//nothing
	}
	break;

	}
	return 1;
}



void
GlWindow::draw ()
{
	int mdlIdx;
	glClearColor (d_bgColor[0], d_bgColor[1], d_bgColor[2], 0.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport (0, 0, w (), h ());

	if (getFlag (F_BACKGROUND) && d_textureNames[TEXTURE_BACKGROUND])
	{
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
		glOrtho (0.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f);

		glMatrixMode (GL_MODELVIEW);
		glPushMatrix ();
		glLoadIdentity ();

		glDisable (GL_LIGHTING);
		glDisable (GL_CULL_FACE);
		glDisable (GL_DEPTH_TEST);
		glEnable (GL_TEXTURE_2D);

		glColor4f (1.0f, 1.0f, 1.0f, 0.3f);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_BACKGROUND]);
		
		glBegin (GL_QUADS);

		glTexCoord2f (0, 0);
		glVertex2f (0, 0);

		glTexCoord2f (0, 1);
		glVertex2f (0, 1);

		glTexCoord2f (1, 1);
		glVertex2f (1, 1);

		glTexCoord2f (1, 0);
		glVertex2f (1, 0);

	
		
		glEnd ();

		glPopMatrix ();
	}

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (65.0f, (GLfloat) w () / (GLfloat) h (), 1.0f, 1024.0f); //hypov8 todo: increase for heli?

	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();

	glLoadIdentity ();

	if (getFlag (F_LIGHT))
	{
		GLfloat lp[4] = { 0, 0, d_transZ, 1 };
		GLfloat lc[4] = { d_lightColor[0], d_lightColor[1], d_lightColor[2], 1.0f };

		glLightfv (GL_LIGHT0, GL_POSITION, lp);
		glLightfv (GL_LIGHT0, GL_DIFFUSE, lc);
	}

	glPixelTransferf (GL_RED_SCALE, 1.0f + 5.0f * d_bias);
	glPixelTransferf (GL_GREEN_SCALE, 1.0f + 5.0f * d_bias);
	glPixelTransferf (GL_BLUE_SCALE, 1.0f + 5.0f * d_bias);


/*	glPixelTransferf (GL_RED_SCALE, 99.0f);
	glPixelTransferf (GL_GREEN_SCALE, 99.0f);
	glPixelTransferf (GL_BLUE_SCALE, 99.0f);

	glPixelTransferf (GL_RED_BIAS, 99.0f);
	glPixelTransferf (GL_GREEN_BIAS, 99.0f);
	glPixelTransferf (GL_BLUE_BIAS, 99.0f);*/

	glTranslatef (-d_transX, -d_transY, -d_transZ);

	glRotatef (d_rotX, 1, 0, 0);
	glRotatef (d_rotY, 0, 1, 0);

	if (getFlag (F_SHININESS))
	{
		GLfloat ms[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, ms);
		glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 128.0f);
	}
	else
	{
		GLfloat ms[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, ms);
		glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
	}

	if (getFlag (F_LIGHT))
	{
		glEnable (GL_LIGHTING);
		glEnable (GL_LIGHT0);
	}
	else
	{
		glDisable (GL_LIGHTING);
		glDisable (GL_LIGHT0);
	}

	if (d_renderMode == RM_WIREFRAME)
	{
		GLfloat md[4] = { d_wfColor[0], d_wfColor[1], d_wfColor[2], 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);
		glColor3f (d_wfColor[0], d_wfColor[1], d_wfColor[2]);

		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_CULL_FACE);
		glDisable (GL_DEPTH_TEST);
	}
	else if (d_renderMode == RM_FLATSHADED ||
			d_renderMode == RM_SMOOTHSHADED)
	{
		glColor3f (d_fgColor[0], d_fgColor[1], d_fgColor[2]);
		GLfloat md[4] = { d_fgColor[0], d_fgColor[1], d_fgColor[2], 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);

		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glDisable (GL_TEXTURE_2D);
		glEnable (GL_CULL_FACE);
		glEnable (GL_DEPTH_TEST);

		if (d_renderMode == RM_FLATSHADED)
			glShadeModel (GL_FLAT);
		else
			glShadeModel (GL_SMOOTH);
	}
	else if (d_renderMode == RM_TEXTURED)
	{
		glColor3f (1.0f, 1.0f, 1.0f);
		GLfloat md[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, md);
		
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable (GL_TEXTURE_2D);
		glEnable (GL_CULL_FACE);
		glEnable (GL_DEPTH_TEST);
		glShadeModel (GL_SMOOTH);
		
	}

	int hasTexture = 0;

	if (d_models[0])
	{
		if (d_renderMode == RM_TEXTURED)
		{
			if (!d_textureNames[TEXTURE_MODEL_0])
			{
				GLfloat md[4] = { d_fgColor[0], d_fgColor[1], d_fgColor[2], 1.0f };
				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, md);
				//glDisable(GL_TEXTURE_2D);
			}
			else
			{
				hasTexture = 1;
			}
		}

		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_MODEL_0]);
		if (d_currFrame < d_models[0]->header.numFrames &&
			d_currFrame2 < d_models[0]->header.numFrames)
		{
			mdx_drawModel(d_models[0], d_currFrame, d_currFrame2, d_pol, 0, 0);
			mdx_drawModel_dev(d_models[0], d_currFrame, d_currFrame2, d_pol, 1,
				d_vertexIndex, d_vertexUseFace, //HYPOVERTEX
				getFlag(F_VNORMS), getFlag(F_GRID), getFlag(F_HITBOX), 
				d_debugColor, d_gridColor);
		}
	}

	//loop though all models. render textured?
	for (mdlIdx = 1; mdlIdx < MAX_MODELS; mdlIdx++)
	{
		if (d_models[mdlIdx])
		{
			int numFrm = d_models[mdlIdx]->header.numFrames;
			if (d_renderMode == RM_TEXTURED)
			{
				
				if (!d_textureNames[mdlIdx]) //TEXTURE_MODEL_1
				{
					GLfloat md[4] = { d_fgColor[0], d_fgColor[1], d_fgColor[2], 1.0f };
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, md);
					//glDisable(GL_TEXTURE_2D);
				}
				else
				{
					GLfloat md[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, md);
					hasTexture = 1;
				}
			}

			glBindTexture(GL_TEXTURE_2D, d_textureNames[mdlIdx]); //TEXTURE_MODEL_1

			// multi model with differn animation count?
			if (d_currFrame < numFrm && d_currFrame2 < numFrm)
			{
				mdx_drawModel(d_models[mdlIdx], d_currFrame, d_currFrame2, d_pol, 0, 0);
				mdx_drawModel_dev(d_models[mdlIdx], d_currFrame, d_currFrame2, d_pol, 1,
					-1, 0, getFlag(F_VNORMS), getFlag(F_GRID), getFlag(F_HITBOX),
					d_debugColor, d_gridColor);
			}
			else
			{
				mdx_drawModel(d_models[mdlIdx], numFrm-1, d_currFrame2, 0, 1, 1); // static, show in red
				mdx_drawModel_dev(d_models[mdlIdx], numFrm-1, -1, 0, 0,
					-1, 0, getFlag(F_VNORMS), getFlag(F_GRID), getFlag(F_HITBOX),
					d_debugColor, d_gridColor);
			}
		}
	}
	//hypov8 only disable if no valid skin
	if (d_renderMode == RM_TEXTURED && !hasTexture)
		glDisable(GL_TEXTURE_2D);


	if (getFlag (F_WATER) && d_textureNames[TEXTURE_WATER])
	{
		glDisable (GL_LIGHTING);

		glNormal3f (0, 1, 0);
		glDisable (GL_CULL_FACE);
		glEnable (GL_BLEND);
		glColor4f (1.0f, 1.0f, 1.0f, 0.3f);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture (GL_TEXTURE_2D, d_textureNames[TEXTURE_WATER]);

		glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 0.0f);
		glVertex3f (-100.0f, 0.0f, -100.0f);
		glTexCoord2f (1.0f, 0.0f);
		glVertex3f (100.0f, 0.0f, -100.0f);
		glTexCoord2f (1.0f, 1.0f);
		glVertex3f (100.0f, 0.0f, 100.0f);
		glTexCoord2f (0.0f, 1.0f);
		glVertex3f (-100.0f, 0.0f, 100.0f);
		glEnd ();
		glDisable (GL_BLEND);
	}

	glPopMatrix ();
}



mdx_model_t * GlWindow::loadModel (const char *filename, int pos)
{
	char ext[256];

	if (d_models[pos] != 0)
	{
		mdx_freeModel (d_models[pos]);
		d_models[pos] = 0;
		setModelIndex(); //hypov8

		modelFileNames[pos][0] = '\0'; //hypov8 models
		modelTexNames[pos][0] = '\0'; //hypov8 textures
	}

	if (!filename || !strlen (filename))
		return 0;


	strcpy_s(ext, sizeof(ext), mx_getextension(filename));
	if (!mx_strncasecmp(ext, ".mdx", 4))
	{
		d_models[pos] = mdx_readModel(filename, d_debugLoad);
		setModelIndex(); //hypov8
		if (!d_models[pos])
			return 0;
	}
	else if (!mx_strncasecmp(ext, ".md2", 4))
	{
		d_models[pos] = md2_Parse_readModel(filename, d_debugLoad);
		setModelIndex(); //hypov8
		if (!d_models[pos])
			return 0;
	}
	else
		return 0;
	
	//set skins to main/ and use the models internal file name
	if (d_models[pos]->skins && d_models[pos]->skins[0][0] != '\0')
	{
		int idx = 0;
		char in[256]; 
		char out[256];
		FILE *file = NULL;

		strcpy_s(in, sizeof(in), filename);
		mx_strlower(in);

		const char *isModel = strstr(in, "models\\"); //hypov8 todo: os path..
		const char *isPlayr = strstr(in, "players\\");
		const char *isTextr = strstr(in, "textures\\");
		//hypov8 todo: should we also check main for assets?
	

		if (isModel || isPlayr || isTextr)
		{
			if (isModel)
				idx = isModel - in;
			else if (isPlayr)
				idx = isPlayr - in;
			else //if (isTextr)
				idx = isTextr - in;

			//combine file path + model skin
			strncpy_s(out, sizeof(out), in, idx);
			strcat_s(out, sizeof(out), d_models[pos]->skins[0]);

			//check if it exists
			if (fopen_s(&file, out, "rb") == 0)
			{
				fclose(file);
			}
			else if (isPlayr )//failed.. check for player model skins 
			{
				char fName[256];
				strcpy_s(fName, sizeof(fName), mx_getfilename(filename));
				if (!mx_strcasecmp(fName, "head.mdx"))
				{
					strcpy_s(out, sizeof(out), mx_getpath(in));
					strcat_s(out, sizeof(out), "head_001.tga");
				}
				else if (!mx_strcasecmp(fName, "body.mdx"))
				{
					strcpy_s(out, sizeof(out), mx_getpath(in));
					strcat_s(out, sizeof(out), "body_001.tga");
				}
				else if (!mx_strcasecmp(fName, "legs.mdx"))
				{
					strcpy_s(out, sizeof(out), mx_getpath(in));
					strcat_s(out, sizeof(out), "legs_001.tga");
				}
			}
		}//end key folder names
		else
		{	//get texture in model directory
			strcpy_s(out, sizeof(out), mx_getpath(in));
			strcat_s(out, sizeof(out), mx_getfilename(d_models[pos]->skins[0]));
		}
		//set skin name
		strcpy_s(modelTexNames[pos], sizeof(modelTexNames[pos]), out); //hypov8 textures. 
	}

	//copy fie name
	strcpy_s (modelFileNames[pos], sizeof(modelFileNames[pos]), filename);

	return d_models[pos];
}



int
GlWindow::loadTexture (const char *filename, int imgIndex)
{
	if (!filename || !strlen (filename))
	{
		if (d_textureNames[imgIndex])
			d_textureNames[imgIndex] = 0;

		modelTexNames[imgIndex][0] = '\0'; //hypov8 textures

		return 0;
	}

	mxImage *image = 0;
	char ext[256];
	int isTga = 0;
	strcpy_s(ext, sizeof(ext), mx_getextension(filename));

	if (!mx_strcasecmp(ext, ".pcx"))
		image = mxPcxRead(filename);
	else if (!mx_strcasecmp(ext, ".tga"))
		image = mxTgaRead(filename);

	if (image)
	{
		byte *out = NULL;
		//hypov8 store texture names
		strcpy_s (modelTexNames[imgIndex], sizeof(modelTexNames[imgIndex]), filename);

		d_textureNames[imgIndex] = (unsigned int)imgIndex+1; //hypov8

		//convert pallet to 24 it
		if (image->bpp == 8)
		{
			int ptr = 0;
			byte *pal = (byte *)image->palette;
			byte *data = (byte *)image->data;
			out = (byte *)malloc(image->width * image->height * 3);

			for (int y = 0; y < image->height; y++)
			{
				for (int x = 0; x < image->width; x++)
				{
					if (image->bpp == 8)
					{
						out[ptr++] = pal[data[y * image->width + x] * 3 + 0];
						out[ptr++] = pal[data[y * image->width + x] * 3 + 1];
						out[ptr++] = pal[data[y * image->width + x] * 3 + 2];
					}
					else if (image->bpp == 24)
					{	//hypov8 todo: this is not 100% working (ingame invalid to)
						out[ptr++] = data[(y* (image->width * 3) + x)]; //red
						out[ptr++] = data[(y* (image->width * 3) + (image->width * 1) + x)]; //green
						out[ptr++] = data[(y* (image->width * 2) + (image->width * 2) + x)]; //blue
					}
				}
			}
		}

		glBindTexture(GL_TEXTURE_2D, d_textureNames[imgIndex]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, (image->bpp == 8)? out: image->data);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (out) free(out);

		delete image;
		return imgIndex+1;
	}

	return 0;
}



void
GlWindow::setRenderMode (int mode)
{
	d_renderMode = mode;
}



void
GlWindow::setFrameInfo (int startFrame, int endFrame)
{
	if (d_models[0])
	{
		d_startFrame = startFrame;
		d_endFrame = endFrame;

		if (d_startFrame >= d_models[0]->header.numFrames)
			d_startFrame = d_models[0]->header.numFrames - 1;
		else if (d_startFrame < 0)
			d_startFrame = 0;

		if (d_endFrame >= d_models[0]->header.numFrames)
			d_endFrame = d_models[0]->header.numFrames - 1;
		else if (d_endFrame < 0)
			d_endFrame = 0;

		d_currFrame = d_startFrame;
		d_currFrame2 = d_startFrame + 1;

		if (d_currFrame >= d_models[0]->header.numFrames)
			d_currFrame = d_models[0]->header.numFrames - 1;

		if (d_currFrame2 >= d_models[0]->header.numFrames)
			d_currFrame2 = 0;
	}
	else
	{
		d_startFrame = d_endFrame = d_currFrame = d_currFrame2 = 0;
	}

	d_pol = 0;
}



void
GlWindow::setPitch (float pitch)
{
	d_pitch = pitch;
	if (d_pitch < 1.0f) //fix devide by zero
		d_pitch = 1.0f;
}



void
GlWindow::setBGColor (float r, float g, float b)
{
	d_bgColor[0] = r;
	d_bgColor[1] = g;
	d_bgColor[2] = b;
}



void
GlWindow::setFGColor (float r, float g, float b)
{
	d_fgColor[0] = r;
	d_fgColor[1] = g;
	d_fgColor[2] = b;
}


void
GlWindow::setWFColor (float r, float g, float b)
{
	d_wfColor[0] = r;
	d_wfColor[1] = g;
	d_wfColor[2] = b;
}


void
GlWindow::setLightColor (float r, float g, float b)
{
	d_lightColor[0] = r;
	d_lightColor[1] = g;
	d_lightColor[2] = b;
}


void
GlWindow::setDebugColor (float r, float g, float b)
{
	d_debugColor[0] = r;
	d_debugColor[1] = g;
	d_debugColor[2] = b;
}

void
GlWindow::setGridColor(float r, float g, float b)
{
	d_gridColor[0] = r;
	d_gridColor[1] = g;
	d_gridColor[2] = b;
}

/*void
GlWindow::setLoadInvalid(int value)
{
	d_debugLoad = value;
}*/

void
GlWindow::setFlag (int flag, bool enable)
{
	if (enable)
		d_flags |= flag;	// set flag
	else
		d_flags &= ~flag;	// clear flag

	mdx_setStyle ((int) getFlag (F_GLCOMMANDS), (int) getFlag (F_INTERPOLATE));
}



void
GlWindow::setBrightness (int value)
{
	d_bias = (float) value / 100.0f;
	redraw ();
}


void
GlWindow::setModelIndex (void)
{
	int i;

	d_modelIndex = 6;

	for (i = 0;i < MAX_MODELS; i++)
	{
		if (!d_models[i]) {
			d_modelIndex = i;
			break;
		}
	}
}


