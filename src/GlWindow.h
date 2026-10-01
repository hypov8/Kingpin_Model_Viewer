#ifndef INCLUDED_GLWINDOW
#define INCLUDED_GLWINDOW

#ifndef INCLUDED_MXGLWINDOW
#include <mx/mxGlWindow.h>
#endif

#ifndef INCLUDED_MDXVIEWER
#include "mdxviewer.h"
#endif


enum // GlWindow Flags
{
	F_WATER =       SET_BIT(0),
	F_LIGHT =       SET_BIT(1),
	F_SHININESS =   SET_BIT(2),
	F_INTERPOLATE = SET_BIT(3),
	F_PAUSE =       SET_BIT(4),
	F_BACKGROUND =  SET_BIT(5),
	F_VNORMS =      SET_BIT(6),
	F_GRID =        SET_BIT(7),
	F_HITBOX =      SET_BIT(8),
	F_WIREFRAME =   SET_BIT(9),
	F_WIRE_OGL1 =   SET_BIT(10),
	F_WIRE_OGL2 =   SET_BIT(11),
	//F_GLCOMMANDS =  SET_BIT(10),
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
	RM_TEXTURED,
};



class GlWindow : public mxGlWindow
{
	float d_rotX, d_rotY;
	float d_transX, d_transY, d_transZ;
	float d_modelOriginX, d_modelOriginZ; //rotate screen around model

	mdx_model_t *d_models[MAX_MODELS]; //6 models max

	//1-6=model, 7=background, 8=water 
	bool  d_textureValid[MAX_TEXTURES]; // valid texture loaded?
	UINT  d_textureGLID[MAX_TEXTURES];  // opengl texture ID //GLuint
	int   d_textureRez[MAX_TEXTURES][2]; // w/h
	//int          d_textureLimit;

	char  d_modelFileNames[MAX_MODELS][MAX_PATH_LEN]; //hypov8 models
	char  d_modelTexNames[MAX_TEXTURES][MAX_PATH_LEN]; //hypov8 textures

	int   d_renderMode;
	bool  d_resumePaused;
	float d_pol; // interpolate value 0.0f - 1.0f
	int   d_currFrame, d_currFrame2, d_startFrame, d_endFrame;
	float d_pitch;

	float d_bgColor[3];    //background  color
	float d_faceColor[3];  //face color (no textures)
	float d_wfColor[3];    //wireframe color
	float d_lightColor[3]; //pointlight color
	float d_debugColor1[3]; //debug color
	float d_debugColor2[3]; //debug color
	float d_gridColor[3];  //grid color

	int  d_debugLoad;     //load invalid models
	//int d_startMode; //startup switch
	//bool  d_startPaused; //startup switch

	float d_bias;

	int   d_flags;

	int d_modelIndex; //hypov8
	int d_vertexIndex; //HYPOVERTEX
	int d_vertexUseFace; //HYPOVERTEX

private:
	void pixelTransfer_set(float value);
	void pixelTransfer_reset();
	void bindWhiteImage(int imgIndex);
	int  gl_uploadTexture24(byte *data, int width, int height, int gl_ID);
	void ResampleTexture(byte *in, int inwidth, int inheight, byte *out, int outwidth, int outheight);
	int  clamp(float value);


public:
	friend MDXViewer;

	// CREATORS
	GlWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style);
	~GlWindow ();

	// MANIPULATORS
	virtual int handleEvent (mxEvent *event);
	virtual void draw ();

	mdx_model_t *loadModel (const char *filename, int pos, bool getSkin);
	void reloadAllTextures(); //hypov8. f5 refresh textures
	int  loadTexture (const char *filename, int pos);

	void setRenderMode (int mode);
	void setFrameInfo (int startFrame, int endFrame);
	void setPitch (float pitch);
	void setBGColor (float r, float g, float b);
	void setFaceColor(float r, float g, float b);
	void setWFColor (float r, float g, float b);
	void setLightColor (float r, float g, float b);
	void setDebugColor1 (float r, float g, float b); //hypov8
	void setDebugColor2 (float r, float g, float b); //hypov8
	void setGridColor (float r, float g, float b); //hypov8
	void setFlag (int flag, bool enable);
	void setBrightness (int value);
	//void setTextureLimit (int limit) { d_textureLimit = limit; }
	int  getFreeModelIndex();

	void setLoadInvalid(int value)  { d_debugLoad = value; }; //hypov8
	void setResumePaused() { d_resumePaused = true; };

	// ACCESSORS
	mdx_model_t *getModel (int pos) const { return d_models[pos]; }
	int  getRenderMode () const { return d_renderMode; }
	int  getCurrFrame () const { return d_currFrame; }
	int  getCurrFrame2 () const { return d_currFrame2; }
	int  getStartFrame () const { return d_startFrame; }
	int  getEndFrame () const { return d_endFrame; }
	void getBGColor (float *r, float *g, float *b) { *r = d_bgColor[0]; *g = d_bgColor[1]; *b = d_bgColor[2]; }
	void getFaceColor (float *r, float *g, float *b) { *r = d_faceColor[0]; *g = d_faceColor[1]; *b = d_faceColor[2]; }
	void getWFColor (float *r, float *g, float *b) { *r = d_wfColor[0]; *g = d_wfColor[1]; *b = d_wfColor[2]; }
	void getLightColor (float *r, float *g, float *b) { *r = d_lightColor[0]; *g = d_lightColor[1]; *b = d_lightColor[2]; }
	void getDebugColor1 (float *r, float *g, float *b) { *r = d_debugColor1[0]; *g = d_debugColor1[1]; *b = d_debugColor1[2]; } //hypov8
	void getDebugColor2 (float *r, float *g, float *b) { *r = d_debugColor2[0]; *g = d_debugColor2[1]; *b = d_debugColor2[2]; } //hypov8
	void getGridColor  (float *r, float *g, float *b) { *r = d_gridColor[0];  *g = d_gridColor[1];  *b = d_gridColor[2]; } //hypov8
	bool getFlag (int flag) const { return ((d_flags & flag) == flag); }
	int  getFlags () const { return d_flags; }
	//int  getTextureLimit () const { return d_textureLimit; }
	int  getLoadInvalid() { return d_debugLoad; }; //hypov8

};


#endif // INCLUDED_GLWINDOW
