#ifndef INCLUDED_GLWINDOW
#define INCLUDED_GLWINDOW

#ifndef INCLUDED_MXGLWINDOW
#include <mx/mxGlWindow.h>
#endif

//#ifndef INCLUDED_MDXVIEWER
#include "mdxviewer.h"
//#endif

//#ifndef INCLUDED_MDX
//#include "common.h"
#include "mdx.h"
#include "md2.h"
//#endif



enum // GlWindow Flags
{
	F_WATER = 1,
	F_LIGHT = 2,
	F_SHININESS = 4,
	F_INTERPOLATE = 8,
	F_GLCOMMANDS = 16,
	F_PAUSE = 32,
	F_BACKGROUND = 64,
	F_VNORMS = 128,
	F_GRID = 256,
	F_HITBOX = 512
};


enum // texture names
{
	TEXTURE_MODEL_0,
	TEXTURE_MODEL_1,
	TEXTURE_MODEL_2,
	TEXTURE_MODEL_3,
	TEXTURE_MODEL_4,
	TEXTURE_MODEL_5,

	TEXTURE_BACKGROUND,
	TEXTURE_WATER
};

//duplicate mxFileDialog
enum // FileTypes
{
	FILE_TYPE_NONE,
	FILE_TYPE_MDX,
	FILE_TYPE_MD2,
	FILE_TYPE_TGA,
	FILE_TYPE_PCX,
	FILE_TYPE_PAK, 
	FILE_TYPE_AVI
};



enum // render modes
{
	RM_WIREFRAME,
	RM_FLATSHADED,
	RM_SMOOTHSHADED,
	RM_TEXTURED
};



class GlWindow : public mxGlWindow
{
	float d_rotX, d_rotY;
	float d_transX, d_transY, d_transZ;
	mdx_model_t *d_models[MAX_MODELS]; //6 models max

	//1->6 = model, 7 = background, 8 = water 
	unsigned int d_textureNames[MAX_TEXTURES];	// index0 = texture 0
	int d_textureLimit;		

	char modelFileNames[MAX_MODELS][256]; //hypov8 models
	char modelTexNames[MAX_TEXTURES][256]; //hypov8 textures

	int d_renderMode;

	float d_pol; // interpolate value 0.0f - 1.0f
	int d_currFrame, d_currFrame2, d_startFrame, d_endFrame;
	float d_pitch;

	float d_bgColor[3];
	float d_fgColor[3];
	float d_wfColor[3];
	float d_lightColor[3];
	float d_debugColor[3]; //debug color
	float d_gridColor[3]; //grid color
	int d_debugLoad; //load invalid models
	//int d_startMode; //startup switch


	float d_bias;

	int d_flags;

	int d_modelIndex; //hypov8
	int d_vertexIndex; //HYPOVERTEX
	int d_vertexUseFace; //HYPOVERTEX

public:
	friend MDXViewer;

	// CREATORS
	GlWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style);
	~GlWindow ();

	// MANIPULATORS
	virtual int handleEvent (mxEvent *event);
	virtual void draw ();

	mdx_model_t *loadModel (const char *filename, int pos);
	int loadTexture (const char *filename, int name);

	void setRenderMode (int mode);
	void setFrameInfo (int startFrame, int endFrame);
	void setPitch (float pitch);
	void setBGColor (float r, float g, float b);
	void setFGColor (float r, float g, float b);
	void setWFColor (float r, float g, float b);
	void setLightColor (float r, float g, float b);
	void setDebugColor (float r, float g, float b); //hypov8
	void setGridColor (float r, float g, float b); //hypov8
	void setFlag (int flag, bool enable);
	void setBrightness (int value);
	void setTextureLimit (int limit) { d_textureLimit = limit; }
	void setModelIndex(void);//hypov8

	void setLoadInvalid(int value)  { d_debugLoad = value; }; //hypov8

	// ACCESSORS
	mdx_model_t *getModel (int pos) const { return d_models[pos]; }
	int getRenderMode () const { return d_renderMode; }
	int getCurrFrame () const { return d_currFrame; }
	int getCurrFrame2 () const { return d_currFrame2; }
	int getStartFrame () const { return d_startFrame; }
	int getEndFrame () const { return d_endFrame; }
	void getBGColor (float *r, float *g, float *b) { *r = d_bgColor[0]; *g = d_bgColor[1]; *b = d_bgColor[2]; }
	void getFGColor (float *r, float *g, float *b) { *r = d_fgColor[0]; *g = d_fgColor[1]; *b = d_fgColor[2]; }
	void getWFColor (float *r, float *g, float *b) { *r = d_wfColor[0]; *g = d_wfColor[1]; *b = d_wfColor[2]; }
	void getLightColor (float *r, float *g, float *b) { *r = d_lightColor[0]; *g = d_lightColor[1]; *b = d_lightColor[2]; }
	void getDebugColor (float *r, float *g, float *b) { *r = d_debugColor[0]; *g = d_debugColor[1]; *b = d_debugColor[2]; } //hypov8
	void getGridColor  (float *r, float *g, float *b) { *r = d_gridColor[0];  *g = d_gridColor[1];  *b = d_gridColor[2]; } //hypov8
	bool getFlag (int flag) const { return ((d_flags & flag) == flag); }
	int getFlags () const { return d_flags; }
	int getTextureLimit () const { return d_textureLimit; }
	int getModelIndex() const { return d_modelIndex; }//hypov8

};



//extern char modelFileNames[MAX_MODELS][256]; //hypov8 models
//extern char modelTexNames[MAX_TEXTURES][256]; //hypov8 textures




#endif // INCLUDED_GLWINDOW
