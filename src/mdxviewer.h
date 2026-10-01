#ifndef INCLUDED_MDXVIEWER
#define INCLUDED_MDXVIEWER


#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif

#ifndef INCLUDED_COMMON
	#include "common.h"
#endif
#ifndef INCLUDED_MDX
	#include "mdx.h"
#endif
#ifndef INCLUDED_MD2
	#include "md2.h"
#endif


#define KP_BUILD_VERSION "1.1.6.14"
//#define MAX_PATH_LEN 256

enum idControlEvents1
{
	IDC_MODEL_LOADMODEL = 1001,
	IDC_MODEL_MERGEMODEL,
	IDC_MODEL_UNLOADMODEL,
	IDC_MODEL_UNLOADWEAPON,
	IDC_MODEL_OPENPAKFILE,
	//IDC_MODEL_OPENPAKFILE2,
	IDC_MODEL_CLOSEPAKFILE,
	IDC_MODEL_EXIT,
	IDC_MODEL_MD2,
	IDC_MODEL_SAVE,
	IDC_MODEL_LOAD_PMODEL, //HYPOV8 OPEN MODEL FOLDER

	IDC_SKIN_MODELSKIN1,
	IDC_SKIN_MODELSKIN2,
	IDC_SKIN_MODELSKIN3,
	IDC_SKIN_MODELSKIN4,
	IDC_SKIN_MODELSKIN5,
	IDC_SKIN_MODELSKIN6,
	IDC_SKIN_BACKGROUND,
	IDC_SKIN_WATER,

	IDC_SKIN_RELOAD, //HYPOV8

	//file output
	IDC_MAKE_SCREENSHOT,
	IDC_MAKE_AVI,
	IDC_MAKE_UV_RES,
	IDC_MAKE_UV_1K,
	IDC_MAKE_UV_2K,

	//options
	IDC_OPTIONS_BGCOLOR,
	IDC_OPTIONS_WFCOLOR,
	IDC_OPTIONS_FACECOLOR,
	IDC_OPTIONS_LIGHTCOLOR,
	IDC_OPTIONS_GRIDCOLOR, //grid color
	IDC_OPTIONS_DEBUGCOLOR1, //vertex norms, hitbox, wireframe, glCommands
	IDC_OPTIONS_DEBUGCOLOR2,
	IDC_OPTIONS_CENTERMODEL1,
	IDC_OPTIONS_CENTERMODEL2,
	IDC_OPTIONS_GEN_NORMALS,
	IDC_OPTIONS_LOADINVALID, //ignore header size
	IDC_OPTIONS_LOAD_PAUSED, //ini option

	IDC_HELP_GOTOHOMEPAGE,
	IDC_HELP_ABOUT,

	IDC_MODEL_RECENTMODELS_FIRST,
	IDC_MODEL_RECENTMODELS1 = IDC_MODEL_RECENTMODELS_FIRST,
	IDC_MODEL_RECENTMODELS2,
	IDC_MODEL_RECENTMODELS3,
	IDC_MODEL_RECENTMODELS4,
	IDC_MODEL_RECENTMODELS5,
	IDC_MODEL_RECENTMODELS6,
	IDC_MODEL_RECENTMODELS7,
	IDC_MODEL_RECENTMODELS8,
	IDC_MODEL_RECENTMODELS_LAST = IDC_MODEL_RECENTMODELS8,

	IDC_MODEL_RECENTPAKFILES1,
	IDC_MODEL_RECENTPAKFILES2,
	IDC_MODEL_RECENTPAKFILES3,
	IDC_MODEL_RECENTPAKFILES4,
	IDC_MODEL_RECENTPAKFILES5,
	IDC_MODEL_RECENTPAKFILES6,
	IDC_MODEL_RECENTPAKFILES7,
	IDC_MODEL_RECENTPAKFILES8,

	IDC_KEYS_F5,
	IDC_KEYS_PAUSE,
	IDC_KEYS_MODE1,
	IDC_KEYS_MODE2,
	IDC_KEYS_MODE3,
	IDC_KEYS_MODE4,
};

enum idControlEvents2
{
	IDC_RENDERMODE=2001,
	IDC_WATER,
	IDC_LIGHT,
	IDC_BRIGHTNESS,
	IDC_SHININESS,
	IDC_BACKGROUND,
	IDC_VERTNORMS,
	IDC_GRID,
	IDC_HITBOX,
	IDC_WIREFRAME,
};


enum idControlEvents3
{
	IDC_ANIMATION_SETS = 3001, //animation dropdown event
	IDC_INTERPOLATE,
	IDC_GLCOMMANDS,
	IDC_PITCH,
	IDC_PAUSE,
	IDC_BTN_SET_FRAME,
	IDC_INCFRAME,
	IDC_DECFRAME,
	IDC_1ST_PERSON,   //hypov8
	IDC_VERTEX_SET,   //HYPOVERTEX vertex number input
	IDC_VERTEX_NEXT,  //HYPOVERTEX increase FRAME
	IDC_VERTEX_PREV,  //HYPOVERTEX decrease FRAME
	IDC_VERT_USEFACE, //HYPOVERTEX switch modes
};



#define MAX_MODELS 6 //HYPOV8
#define MAX_TEXTURES MAX_MODELS+2 //1-6=model, 7=background, 8=water 

#define vec4Set(out, in1, in2, in3, in4)   (out[0]=(in1),out[1]=(in2),out[2]=(in3),out[3]=(in4))
#define BRIGHTNESS_LEVELS_MAX 9
#define BRIGHTNESS_LEVELS_STD 1
#define SET_BIT(x) (1<<(x))

#define RGB_COLOR_WHITE      1.0f, 1.0f, 1.0f
#define RGB_COLOR_GREY       0.45f, 0.45f, 0.45f //slight darker. helps with light and paint tools.
#define RGB_COLOR_GREY_DARK  0.35f, 0.35f, 0.35f
#define RGB_COLOR_GREY_LIGHT 0.85f, 0.85f, 0.85f
#define RGB_COLOR_GREY_BLUE  0.35f, 0.45f, 0.55f

#define RGB_COLOR_RED        1.0f, 0.0f, 0.0f
#define RGB_COLOR_GREEN      0.0f, 1.0f, 0.0f
#define RGB_COLOR_BLUE       0.0f, 0.0f, 1.0f


#define NUM_RECENT_FILES (IDC_MODEL_RECENTMODELS_LAST - IDC_MODEL_RECENTMODELS_FIRST +1)


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
	mxCheckBox *cbWireFrame, *cbWater, *cbLight, *cbBackground, *cbInterp, *cbGrid;

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

	int  d_startMode;
	bool d_startPaused;

	char d_recentModelFiles[NUM_RECENT_FILES][MAX_PATH_LEN];
	char d_recentPakFiles[NUM_RECENT_FILES][MAX_PATH_LEN];

	void loadConfigFile ();
	void saveConfigFile ();
	void initRecentFiles ();

	void updateRecentPaths (int eventID, char *str2);
	void updateRecentPaths_(char paths[NUM_RECENT_FILES][MAX_PATH_LEN], char *newPath);

	void updateRecentMenu (int eventID);
	void updateRecentMenu_(char paths[NUM_RECENT_FILES][MAX_PATH_LEN], int eventID);

	bool loadModel_ (const char *ptr, int pos, bool getSkin);
	void setDisplayModelInfo(); // mdx_model_t *model, int pos);
	void initAnimation (mdx_model_t *model, int animation);
	void initAVIAnimation (mdx_model_t *model, int animation, int *start, int *end);
	int  MakeAVI(int start, int end);

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
	void makeUVMapImage(const char *filename, int skinSize);
	void setRenderMode (int index);
	void centerModel (int frame, int pos2);

	void setStatePaused(); //hypov8

	void reset_modelData(); //hypov8
	void reset_viewData(); //hypov8
	void setPauseMode(int frames); //hypov8
	void setFrameDisplay(int frame); //hypov8
	int importPlayerModelFolder(const char *ptr, int mode, int mIndex, bool getSkin);

	// ACCESSORS
	mxMenuBar *getMenuBar () const { return UI_mb; }

	//void ProcessArgs(int argc, char *argv[]); //hypov8
	const char * fileFilterString(int type, int multi); //hypov8
};



extern MDXViewer *g_mdxViewer;



#endif // INCLUDED_MDXVIEWER