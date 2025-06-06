#ifndef INCLUDED_MDXVIEWER
#define INCLUDED_MDXVIEWER


#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif

//#ifndef INCLUDED_MDX
#include "mdx.h"

//#include "common.h"
//#endif

#define KP_BUILD_VERSION "1.1.6.12"


#define IDC_MODEL_LOADMODEL			1001
#define IDC_MODEL_MERGEMODEL		1002
#define IDC_MODEL_UNLOADMODEL		1003
#define IDC_MODEL_UNLOADWEAPON		1004
#define IDC_MODEL_OPENPAKFILE		1005
#define IDC_MODEL_OPENPAKFILE2		1006
#define IDC_MODEL_CLOSEPAKFILE		1007
#define IDC_MODEL_RECENTMODELS1		1008
#define IDC_MODEL_RECENTMODELS2		1009
#define IDC_MODEL_RECENTMODELS3		1010
#define IDC_MODEL_RECENTMODELS4		1011
#define IDC_MODEL_RECENTPAKFILES1	1012
#define IDC_MODEL_RECENTPAKFILES2	1013
#define IDC_MODEL_RECENTPAKFILES3	1014
#define IDC_MODEL_RECENTPAKFILES4	1015
#define IDC_MODEL_EXIT				1016
#define IDC_MODEL_MD2				1017
#define IDC_MODEL_SAVE				1018
#define IDC_MODEL_LOAD_PMODEL		1019 //HYPOV8 OPEN MODEL FOLDER

#define IDC_SKIN_MODELSKIN1			1021
#define IDC_SKIN_MODELSKIN2			1022
#define IDC_SKIN_MODELSKIN3			1023
#define IDC_SKIN_MODELSKIN4			1024
#define IDC_SKIN_MODELSKIN5			1025
#define IDC_SKIN_MODELSKIN6			1026
#define IDC_SKIN_BACKGROUND			1027
#define IDC_SKIN_WATER				1028
#define IDC_SKIN_SCREENSHOT			1029

#define IDC_SKIN_RELOAD				1030 //HYPOV8
#define IDC_SKIN_AVI				1031
#define IDC_SKIN_UV					1032


#define IDC_OPTIONS_BGCOLOR			1033
#define IDC_OPTIONS_WFCOLOR			1034
#define IDC_OPTIONS_FGCOLOR			1035
#define IDC_OPTIONS_LIGHTCOLOR		1036
#define IDC_OPTIONS_GRIDCOLOR		1037 //grid color
#define IDC_OPTIONS_DEBUGCOLOR		1038 //vertex norms, hitbox
#define IDC_OPTIONS_CENTERMODEL1	1039
#define IDC_OPTIONS_CENTERMODEL2	1040
#define IDC_OPTIONS_GEN_NORMALS		1041
#define  IDC_OPTIONS_LOADINVALID	1042 //ignore header size

#define IDC_HELP_GOTOHOMEPAGE		1051
#define IDC_HELP_ABOUT				1052

#define IDC_RENDERMODE				2001
#define IDC_WATER					2002
#define IDC_LIGHT					2003
#define IDC_BRIGHTNESS				2004
#define IDC_SHININESS				2005
#define IDC_BACKGROUND				2006
#define IDC_TEXTURELIMIT			2007
#define IDC_VERTNORMS				2008
#define IDC_GRID					2009
#define IDC_HITBOX					2010

#define IDC_ANIMATION_SETS			3001
#define IDC_INTERPOLATE				3002
#define IDC_GLCOMMANDS				3003
#define IDC_PITCH					3004
#define IDC_PAUSE					3005
#define IDC_BTN_SET_FRAME			3006
#define IDC_INCFRAME				3007
#define IDC_DECFRAME				3008
#define IDC_1ST_PERSON				3009 //hypov8
#define IDC_VERTEX_SET				3010 //HYPOVERTEX vertex number input
#define IDC_VERTEX_NEXT				3011 //HYPOVERTEX increase FRAME
#define IDC_VERTEX_PREV				3012 //HYPOVERTEX decrease FRAME
#define IDC_VERT_USEFACE			3013 //HYPOVERTEX switch modes

#define MAX_MODELS 6 //HYPOV8
#define MAX_TEXTURES 6+2 //1->6 = model, 7 = background, 8 = water 

class mxProgressBar;
class mxTab;
class mxMenuBar;
class mxButton;
class mxLineEdit;
class mxLabel;
class mxChoice;
class mxCheckBox;
class mxSlider;
class GlWindow;
class PAKViewer;

class MDXViewer : public mxWindow
{
	mxMenuBar *UI_mb;
	mxTab *UI_tab;
	mxProgressBar *UI_bar;

	mxChoice *cRenderMode;
	mxCheckBox *cbWater, *cbLight, *cbBackground, *cbInterp;

	mxChoice *cAnim; //dropdown animation sets
	mxButton *bPause; //button pause
	mxLineEdit *leFrame; //inputbox current frame
	mxLineEdit *leVertex; //HYPOVERTEX //hypov8 showVertex
	mxButton *bDecFrame; //button decrease frame num
	mxButton *bIncFrame;  //button increase frame num
	mxButton *bSetFrame; //button set  frame num
	mxButton *b1sPerson; //button 1sPerson
	mxButton *bCamToGrid; //button cam to grid
	mxButton *bCamToModel; //button to model mid section

	mxButton *bDecVertID, *bIncVertID, *bSetVertID; //HYPOVERTEX //hypov8 showVertex
	mxCheckBox *cbVertUseFace; //HYPOVERTEX

	mxLabel *lModelInfo1;
	mxLabel *lModelInfo2; //hypov8 bbox
	mxLabel *lModelInfo3; //mesh name
	mxLabel *lModelInfo4; //HYPOVERTEX
	mxLabel *lAnimSpeed; //speed
	
	mxLineEdit *leWidth, *leHeight;
	mxCheckBox *cb3dfxOpenGL, *cbCDS;

	GlWindow *glw;
	PAKViewer *pakViewer;

	mdx_model_t *mdxModel;
	mdx_model_t *mdxWeapon;

	void loadRecentFiles ();
	void saveRecentFiles ();
	void initRecentFiles ();

	bool loadModel (const char *ptr, int pos);
	void setModelInfo(); // mdx_model_t *model, int pos);
	void initAnimation (mdx_model_t *model, int animation);
	void initAVIAnimation (mdx_model_t *model, int animation, int *start, int *end);
	int MakeAVI(int start, int end);

	//void ProcessArgs(int argc, char *argv[]);
	
public:
	friend PAKViewer;

	// CREATORS
	MDXViewer ();
	~MDXViewer ();

	// MANIPULATORS
	virtual int handleEvent (mxEvent *event);
	void redraw ();
	void makeScreenShot (const char *filename);
	void makeUVMapImage(const char *filename);
	void setRenderMode (int index);
	void centerModel (int frame, int pos2);

	void setStartPaused(); //hypov8

	void reset_modelData(); //hypov8
	void setPauseMode(int frames); //hypov8
	int importPlayerModelFolder(const char *ptr, int mode, int mIndex);
	// ACCESSORS
	mxMenuBar *getMenuBar () const { return UI_mb; }

	//void ProcessArgs(int argc, char *argv[]); //hypov8
	const char * fileFilterString(int type, int multi); //hypov8
};



extern MDXViewer *g_mdxViewer;



#endif // INCLUDED_MDXVIEWER