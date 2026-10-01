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
#include <math.h> //sin/cos

#include "GlWindow.h"



GlWindow::GlWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style)
: mxGlWindow (parent, x, y, w, h, label, style)
{
	d_rotX = d_rotY = 0;
	d_transX = d_transY = 0;
	d_transZ = 50;

	glGenTextures(MAX_TEXTURES, d_textureGLID);
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		d_textureValid[i] = false;
		d_textureRez[i][0] = 0;
		d_textureRez[i][1] = 0;
		d_modelTexNames[i][0] = '\0';
	}
	for (int i = 0; i < MAX_MODELS; i++)
	{
		d_models[i] = 0;
		d_modelFileNames[i][0] = '\0';
	}
	setFrameInfo (0, 0);
	setRenderMode (0); //reset view to wireframe
	setPitch (10.0f); //10fps
	setBGColor (RGB_COLOR_GREY_BLUE); //hypov8 background. was black
	setFaceColor(RGB_COLOR_WHITE);
	setWFColor (RGB_COLOR_WHITE);
	setLightColor (RGB_COLOR_GREY_LIGHT); //slight grey, so wireframe stands out and shine works
	setDebugColor1 (RGB_COLOR_RED);       //hypov8 red
	setDebugColor2 (RGB_COLOR_GREEN);     //hypov8 green
	setGridColor (RGB_COLOR_GREY_DARK);   //hypov8 dark grey

	setBrightness (BRIGHTNESS_LEVELS_STD);

	d_modelIndex = 0; //hypov8
	d_debugLoad = 0; //hypov8 debug model loading
	d_vertexIndex = -1; //HYPOVERTEX
	d_vertexUseFace = 0; //HYPOVERTEX

	glCullFace (GL_FRONT);

	mx::setIdleWindow (this);
}


//hypov8 updated:
GlWindow::~GlWindow ()
{
	mx::setIdleWindow (0);
	for (int i = 0; i < MAX_MODELS ; i++) //TEXTURE_MODEL_5
	{
		mdx_freeModel(d_models[i]);
	}
	
	/*loadModel(0, TEXTURE_MODEL_0, false);
	loadModel(0, TEXTURE_MODEL_1, false);
	loadModel(0, TEXTURE_MODEL_2, false);
	loadModel(0, TEXTURE_MODEL_3, false);
	loadModel(0, TEXTURE_MODEL_4, false);
	loadModel(0, TEXTURE_MODEL_5, false);*/

	/*loadTexture(0, TEXTURE_MODEL_0); 
	loadTexture(0, TEXTURE_MODEL_1);
	loadTexture(0, TEXTURE_MODEL_2);
	loadTexture(0, TEXTURE_MODEL_3);
	loadTexture(0, TEXTURE_MODEL_4);
	loadTexture(0, TEXTURE_MODEL_5);

	loadTexture(0, TEXTURE_BACKGROUND);
	loadTexture(0, TEXTURE_WATER);*/

	glDeleteTextures(MAX_TEXTURES, d_textureGLID);
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		//d_textureValid[i] = -1;
	}
}



int
GlWindow::handleEvent (mxEvent *event)
{
	static float oldrx = 0, oldry = 0, oldtz = 50, oldtx = 0, oldty = 0;
	static int oldx, oldy;
	static DWORD lastTimeIdle = 0, lastTimeMouse = 0;
	DWORD currentTimeIdle, currentTimeMouse;

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

			break;

		case mxEvent::MouseWheel:
		{
			if (event->zdelta < 0)
				d_transZ += 10;
			else
				d_transZ -= 10;
			redraw();
		}
		break;

		case mxEvent::MouseDrag:
		{
			//limit fps
			currentTimeMouse = mx::getTickCount();
			if (currentTimeMouse < lastTimeMouse + 30)
			{
				mx::sleep(2); //reduce cpu usage
				return 0;
			}
			lastTimeMouse = currentTimeMouse;


			if (event->buttons & mxEvent::MouseLeftButton)
			{
				if (event->modifiers & mxEvent::KeyShift)
				{
#if 0
					d_transX = oldtx - (float)(event->x - oldx);
					d_transY = oldty + (float)(event->y - oldy);

#else //todo fix this
					/*float radAngle = -radians(degree);// "-" - clockwise
					float x = point.x;
					float y = point.y;
					float rX = pivot.x + (x - pivot.x) * cos(radAngle) - (y - pivot.y) * sin(radAngle);
					float rY = pivot.y + (x - pivot.x) * sin(radAngle) + (y - pivot.y) * cos(radAngle);*/

					float rotX = (float)(event->x - oldx) / 180;
					float s = (float)sin(rotX);
					float c = (float)cos(rotX);
					float modelX = d_modelOriginX;
					float modelY = d_modelOriginZ;
					float cameraX = (float)oldtx;
					float cameraY = (float)oldtz;

					float rotatedX = (float)cos(rotX) * (cameraX - modelX) - (float)sin(rotX) * (cameraY - modelY) + modelX;
					float rotatedZ = (float)sin(rotX) * (cameraX - modelX) + (float)cos(rotX) * (cameraY - modelY) + modelY;

					d_transX = rotatedX;
					d_transZ = rotatedZ;

					//d_rotX = oldrx + (float) (event->y - oldy);
					//d_rotY = oldry + (float) (event->x - oldx);
					//}
#endif
				}
				else
				{
					d_rotX = oldrx + (float)(event->y - oldy);
					d_rotY = oldry + (float)(event->x - oldx);
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

			redraw();
		}
		break;

		case mxEvent::Idle:
		{
			if (getFlag(F_PAUSE))
			{
				mx::sleep(10); //reduce cpu usage
				return 0;
			}

			//limit fps
			currentTimeIdle = mx::getTickCount();
			if (currentTimeIdle < lastTimeIdle + 30)
			{
				mx::sleep(2); //reduce cpu usage
				return 0;
			}

			float diff = (float)(currentTimeIdle - lastTimeIdle);
			lastTimeIdle = currentTimeIdle;

			if (!d_resumePaused)
			{
				d_pol += diff * (d_pitch / 1000.f); //IDC_PITCH: //slPitch
				if (d_pol < 0.0f)
					d_pol = 0.0f; //hypov8 failsafe
			}
			else //paused used. resume lerp
			{
				if (d_pol < 0.0f && d_pol > 1.0f)
					d_pol = 0.0f;
				d_resumePaused = false;
			}

			if (d_pol > 1.0f)
			{
				//try sync with framerate
				d_pol -= 1.0f;
				if (d_pol > 1.0f) //extended time...
					d_pol = 0.0f;

				d_currFrame++;
				d_currFrame2++; //should this be fr1+1. and set below?

				if (d_currFrame > d_endFrame)
					d_currFrame = d_startFrame;

				if (d_currFrame2 > d_endFrame)
					d_currFrame2 = d_startFrame;

				//update gui. current frame
				g_mdxViewer->setFrameDisplay(d_currFrame);
			}

			redraw();
		}
		break;

		case mxEvent::MouseMove:
		{
#if WIN32
			int mouseX = event->x;
			int mouseY = event->y;

			int glWinX = 0; //glw->x() -g_mdxViewer->x();
			int glWinY = 0; //g_mdxViewer->UI_mb->getHeight(); // glw->y() - g_mdxViewer->y() - UI_mb->y();

			int glWinW = w() + glWinX;
			int glWinH = h() + glWinY;

			//within opengl window
			if (mouseX > glWinX && mouseX < glWinW &&
				mouseY > glWinY && mouseY < glWinH)
			{
				// Grab focus as soon as the mouse is over the GL area
				// Using Win32 directly for compatibility
				if (GetFocus() != (HWND)getHandle()) {
					SetFocus((HWND)getHandle());
				}
			}
#endif
		}
		break;

		//case mxEvent::KeyUp:

		case mxEvent::KeyDown:
		{
#if 1
			//hotkeys
			switch (event->key)
			{
				case VK_F5:
				{
					reloadAllTextures();
				}
				break;

				case 'P':
				{
					g_mdxViewer->setStatePaused();
				}
				break;

				//IDC_RENDERMODE:
				case '1':
				case '2':
				case '3':
				case '4':
				{
					int val = event->key - 49;
					g_mdxViewer->setRenderMode(val);
					redraw();
				}
				break;
			}
#endif
		} //end mxEvent::KeyUp:
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

	if (getFlag (F_BACKGROUND) && d_textureValid[TEXTURE_BACKGROUND])
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
		glBindTexture (GL_TEXTURE_2D, d_textureGLID[TEXTURE_BACKGROUND]);
		
		//start plane
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
	gluPerspective (65.0f, (GLfloat) w () / (GLfloat) h (), 1.0f, 4096.0f); //hypov8 todo: zfar increase for heli? was 2048

	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();

	if (getFlag (F_LIGHT))
	{
		GLfloat lp[4] = { 0, 0, 1.0, 0.0f }; //directional light
		GLfloat lc[4] = { d_lightColor[0], d_lightColor[1], d_lightColor[2], 1.0f };
		GLfloat amb[4] = {
		(d_lightColor[0]+ d_bias)*0.5f,
		(d_lightColor[1]+ d_bias)*0.5f,
		(d_lightColor[2]+ d_bias)*0.5f,
		1.0f };

		glLightfv(GL_LIGHT0, GL_POSITION, lp);
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lc);
	}


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
		glCullFace(GL_FRONT); //reverse windindings=back?
		
		glEnable(GL_DEPTH_TEST);
		//glDisable (GL_DEPTH_TEST);
	}
	else if (d_renderMode == RM_FLATSHADED ||
			d_renderMode == RM_SMOOTHSHADED)
	{
		glColor3f (d_faceColor[0], d_faceColor[1], d_faceColor[2]);
		GLfloat md[4] = { d_faceColor[0], d_faceColor[1], d_faceColor[2], 1.0f };
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
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable (GL_TEXTURE_2D);
		glEnable (GL_CULL_FACE);
		glEnable (GL_DEPTH_TEST);
		glShadeModel (GL_SMOOTH);
	}


	//loop though all models. render mesh
	for (mdlIdx = 0; mdlIdx < MAX_MODELS; mdlIdx++)
	{
		if (d_models[mdlIdx] == 0)
			continue;

		if (d_renderMode == RM_WIREFRAME
			&& (getFlag(F_WIREFRAME) || getFlag(F_WIRE_OGL1) || getFlag(F_WIRE_OGL2)))
			continue; //break...

		int numFrames = d_models[mdlIdx]->header.numFrames;
		if (d_renderMode == RM_TEXTURED)
		{
			GLfloat md[4];
			glBindTexture(GL_TEXTURE_2D, d_textureGLID[mdlIdx]);
			if (!d_textureValid[mdlIdx])
				vec4Set(md, d_faceColor[0], d_faceColor[1], d_faceColor[2], 1.0f);
			else
				vec4Set(md, 1.0f, 1.0f, 1.0f, 1.0f);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, md);
		}

		if (d_currFrame < numFrames && d_currFrame2 < numFrames)
		{
			mdx_drawModel(d_models[mdlIdx], 
				d_currFrame, d_currFrame2, 
				d_pol, //fraction
				getFlag(F_INTERPOLATE),
				0);
		}
		else
		{
			mdx_drawModel(d_models[mdlIdx], 
				numFrames - 1, d_currFrame2, 
				0, 
				0, 
				1); // static, show in red
		}

	}

	//loop though all models. render dev
	for (mdlIdx = 0; mdlIdx < MAX_MODELS; mdlIdx++)
	{
		if (d_models[mdlIdx])
		{
			int numFrames = d_models[mdlIdx]->header.numFrames;

			// multi model with differn animation count?
			if (d_currFrame < numFrames && d_currFrame2 < numFrames)
			{
				mdx_drawModel_dev(
					d_models[mdlIdx], 
					d_currFrame, d_currFrame2, 
					d_pol,  //fraction
					getFlag(F_INTERPOLATE), //lerp?
					d_vertexIndex, d_vertexUseFace,
					getFlag(F_VNORMS), getFlag(F_GRID), getFlag(F_HITBOX), 
					getFlag(F_WIREFRAME), getFlag(F_WIRE_OGL1), getFlag(F_WIRE_OGL2),
					d_debugColor1, d_debugColor2, d_gridColor, d_wfColor);
			}
			else
			{
				mdx_drawModel_dev(
					d_models[mdlIdx], 
					numFrames -1, -1, 
					0, //fraction
					0, //no lerp
					d_vertexIndex, d_vertexUseFace,
					getFlag(F_VNORMS), getFlag(F_GRID), getFlag(F_HITBOX), 
					getFlag(F_WIREFRAME), getFlag(F_WIRE_OGL1), getFlag(F_WIRE_OGL2),
					d_debugColor1, d_debugColor2, d_gridColor, d_wfColor);
			}
		}
	}


	if (getFlag (F_WATER) && d_textureValid[TEXTURE_WATER])
	{
		glDisable (GL_LIGHTING);

		glNormal3f (0, 1, 0);
		glDisable (GL_CULL_FACE);
		glEnable (GL_BLEND);
		glColor4f (1.0f, 1.0f, 1.0f, 0.3f);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture (GL_TEXTURE_2D, d_textureGLID[TEXTURE_WATER]);

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



mdx_model_t * 
GlWindow::loadModel (const char *filename, int pos, bool getSkin)
{
	char ext[MAX_PATH_LEN];

	if (d_models[pos] != 0)
	{
		mdx_freeModel (d_models[pos]);
		d_models[pos] = NULL;
		d_textureValid[pos] = false;
		d_modelFileNames[pos][0] = '\0'; //hypov8 models
		d_modelTexNames[pos][0] = '\0'; //hypov8 textures
	}

	if (!filename || !strlen (filename))
		return 0;

	strcpy_s(ext, sizeof(ext), mx_getextension(filename));
	if (!mx_strncasecmp(ext, ".mdx", 4))
	{
		d_models[pos] = mdx_readModel(filename, d_debugLoad);
		if (!d_models[pos])
			return 0;
	}
	else if (!mx_strncasecmp(ext, ".md2", 4))
	{
		d_models[pos] = md2_readModel_to_mdx(filename, d_debugLoad);
		if (!d_models[pos])
			return 0;
	}
	else
		return 0;
	
	//set skins to main/ and use the models internal file name
	if (getSkin && d_models[pos]->skins && d_models[pos]->skins[0][0] != '\0')
	{
		int idx = 0;
		char in[MAX_PATH_LEN];
		char out[MAX_PATH_LEN];
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
				char fName[MAX_PATH_LEN];
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
		strcpy_s(d_modelTexNames[pos], sizeof(d_modelTexNames[pos]), out); //hypov8 textures. 
	}

	//copy fie name
	strcpy_s (d_modelFileNames[pos], sizeof(d_modelFileNames[pos]), filename);

	return d_models[pos];
}


void
GlWindow::bindWhiteImage(int imgIndex)
{
	byte data[4] = {255, 255, 255, 255};

	glBindTexture(GL_TEXTURE_2D, d_textureGLID[imgIndex]);
	glTexImage2D(GL_TEXTURE_2D,
		0, //mip 
		GL_RGBA8, //
		1, 1, // w\h
		0, GL_RGBA, GL_UNSIGNED_BYTE, data
	);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void 
GlWindow::reloadAllTextures()
{
	int i;
	for (i = 0; i < MAX_TEXTURES; i++)
	{
		loadTexture(d_modelTexNames[i], i);
	}

	g_mdxViewer->setRenderMode(3); //set mode to view skins
	redraw();
}


//bilinear interpolation
int 
GlWindow::clamp(float value)
{
	if (value < 0.01) 
		return 0;
	if (value > 255.01f) 
		return 255;
	return (int)value;
}

void 
GlWindow::ResampleTexture(byte *in, int inwidth, int inheight, byte *out, int outwidth, int outheight)
{
	float x_ratio = (float)(inwidth - 1) / outwidth;
	float y_ratio = (float)(inheight - 1) / outheight;

	for (int y = 0; y < outheight; y++) 
	{
		for (int x = 0; x < outwidth; x++) 
		{
			// Determine the position in the input image
			float gx = x * x_ratio;
			float gy = y * y_ratio;

			int gxi = (int)gx;
			int gyi = (int)gy;

			// Calculate the fractional parts
			float x_diff = gx - (float)gxi;
			float y_diff = gy - (float)gyi;

			// Get the indices of the surrounding pixels
			int index1 = (gyi * inwidth + gxi) * 3;           // Top-left
			int index2 = (gyi * inwidth + gxi + 1) * 3;       // Top-right
			int index3 = ((gyi + 1) * inwidth + gxi) * 3;     // Bottom-left
			int index4 = ((gyi + 1) * inwidth + gxi + 1) * 3; // Bottom-right

			// Interpolate for R, G, B channels
			for (int i = 0; i < 3; i++) 
			{
				float top =    (float)in[index1 + i] * (1.f - x_diff) + (float)in[index2 + i] * x_diff;
				float bottom = (float)in[index3 + i] * (1.f - x_diff) + (float)in[index4 + i] * x_diff;
				out[(y * outwidth + x) * 3 + i] = clamp(top * (1.f - y_diff) + bottom * y_diff);
			}
		}
	}
}


void 
GlWindow::pixelTransfer_set(float value)
{
	glPixelTransferf(GL_RED_SCALE, value);
	glPixelTransferf(GL_GREEN_SCALE, value);
	glPixelTransferf(GL_BLUE_SCALE, value);
}

void 
GlWindow::pixelTransfer_reset()
{
	glPixelTransferf(GL_RED_SCALE, 1.0f);
	glPixelTransferf(GL_GREEN_SCALE, 1.0f);
	glPixelTransferf(GL_BLUE_SCALE, 1.0f);
}


//upload 24 bit image, resized to power of 2 if neded.
int
GlWindow::gl_uploadTexture24(byte *data, int width, int height, int gl_ID)
{
	int scaled_width, scaled_height;
	byte *uploadData = data;

	// convert to exact power of 2 sizes
	for (scaled_width = 4; scaled_width < width; scaled_width <<= 1)	
	{	;	}
	for (scaled_height = 4; scaled_height < height; scaled_height <<= 1)	
	{	;	}


	if (scaled_width != width || scaled_height != height)
	{
		byte *scaledData = (byte*)malloc(scaled_width*scaled_height*3);
		if (!scaledData)
			return 0;
		ResampleTexture((byte *)data, width, height, scaledData, scaled_width, scaled_height);

		//set new sizes
		uploadData = scaledData;
		width = scaled_width ;
		height = scaled_height;
	}

	pixelTransfer_set(1.0f + 2.0f * d_bias);

	//upload image
	glBindTexture(GL_TEXTURE_2D, gl_ID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, uploadData);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	pixelTransfer_reset();

	//free new image data
	if (uploadData != data)
		free(uploadData);

	return 1;
}

int
GlWindow::loadTexture (const char *filename, int imgIndex)
{
	if (!filename || !strlen (filename))
	{
		d_textureValid[imgIndex] = false;
		d_modelTexNames[imgIndex][0] = '\0'; //hypov8 textures
		bindWhiteImage(imgIndex);
		return 0;
	}

	mxImage *image = 0;
	char ext[MAX_PATH_LEN];

	strcpy_s(ext, sizeof(ext), mx_getextension(filename));

	if (!mx_strcasecmp(ext, ".pcx"))
		image = mxPcxRead(filename);
	else if (!mx_strcasecmp(ext, ".tga"))
		image = mxTgaRead(filename);

	if (image)
	{
		//convert palette image to 24bit if needed
		if (!image->convert8to24bit())
		{
			delete image;
		}
		else
		{
			d_textureValid[imgIndex] = true;
			d_textureRez[imgIndex][0] = image->width;
			d_textureRez[imgIndex][1] = image->height;

			strcpy_s (d_modelTexNames[imgIndex], sizeof(d_modelTexNames[0]), filename);
			gl_uploadTexture24((byte*)image->data, image->width, image->height, d_textureGLID[imgIndex]);

			delete image;
			return imgIndex + 1;
		}
	}

	//white image
	d_textureValid[imgIndex] = false;
	//modelTexNames[imgIndex][0] = 0;
	bindWhiteImage(imgIndex);

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
GlWindow::setFaceColor(float r, float g, float b)
{
	d_faceColor[0] = r;
	d_faceColor[1] = g;
	d_faceColor[2] = b;
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
GlWindow::setDebugColor1 (float r, float g, float b)
{
	d_debugColor1[0] = r;
	d_debugColor1[1] = g;
	d_debugColor1[2] = b;
}

void
GlWindow::setDebugColor2 (float r, float g, float b)
{
	d_debugColor2[0] = r;
	d_debugColor2[1] = g;
	d_debugColor2[2] = b;
}

void
GlWindow::setGridColor(float r, float g, float b)
{
	d_gridColor[0] = r;
	d_gridColor[1] = g;
	d_gridColor[2] = b;
}


void
GlWindow::setFlag (int flag, bool enable)
{
	if (enable)
		d_flags |= flag;	// set flag
	else
		d_flags &= ~flag;	// clear flag
}


void
GlWindow::setBrightness (int value)
{
	static int lastValue = -1;

	if (lastValue != value)
	{
		d_bias = (float) value / (float)BRIGHTNESS_LEVELS_MAX;

		if (lastValue != -1) //ignore init
		{
			//reload images
			for (int i = 0; i < MAX_TEXTURES; i++)
			{
				loadTexture(d_modelTexNames[i], i);
			}

			redraw();
			Sleep(20); //update has no rate limit
		}

		//prevent constant updates
		lastValue = value;
	}
}


int 
GlWindow::getFreeModelIndex()
{
	int i = 0;

	for (; i < MAX_MODELS; i++)
	{
		if (!d_models[i])
			break;
	}
	return i;
}

