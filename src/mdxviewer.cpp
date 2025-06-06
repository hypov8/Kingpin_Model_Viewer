#define WIN32_LEAN_AND_MEAN
#define ERR(x) error(__FILE__, __LINE__, x)
#define AVIERR(x) if ((x) != AVIERR_OK) ERR(#x)

#include <windows.h>
#include <vfw.h> /* don't forget to link with vfw32.lib and user32.lib! */
#include <stdio.h>
#include <stdlib.h>
#include <shellapi.h> 
#include <malloc.h>
#include <mx/mx.h>
#include <mx/gl.h>
#include <mx/mxTga.h>
#include "mdxviewer.h"
#include "GlWindow.h"
#include "pakviewer.h"


//version 1.1.4
// - added vertex viewer to assist adding sprites
// - fixed pause animation. loading a new model would still use old frames
// - additional model with less animations are now always drawn on last frame (red wireframe)
// - fixed some texture searching issues
// - added scrollwheel to zoom view (buggy. looses focus)

//version 1.1.5
// - fixed missing skins causing a crash
// - added searching for player skins eg.. head.mdx searches for head_001.tga

//version 1.1.6
// - fixed light normals
// - added option. show vertex normals
// - added option to show mdx hitbox
// - added menu reload textures
// - added option to show grid
// - added custom color to new dev items
// - camera rotated. this shows player front view
// - fixed missing file extension on export
// - fixed lerp counter going negative (DWORD)
// - updated to latest mxtk from github (+added kingpin file support)
// - background image can be removed with setting color
// - removed pakview, until needed.
// - fix md2 export bug. glCommands buffer overflow
// - loading 6 models with startup string
// - added startup switch. 
// - 	'-0'=stat wireframe
// - 	'-1'=start flat shade
// - 	'-2'=smooth shaded
// - 	'-3'=start textured
// -	'-p'=pause
// - aded high deff suport

//version 1.1.6.11
// - Updated HD model support
// - Fixed crash for large models. Added dynamic memory for frame buffer

//todo 
//framerate limit (currently set to 60)
//stop building models every frame (use buffer)
//add md2 software tex cords?
//fix high cpu usage
//mouse wheel bug. no focus in 3d.
//no ainms. default to paused



MDXViewer *g_mdxViewer = 0;
char loadmod[6][256];

int d_startMode = 0; //hypov8 todo: c++ way?
bool  d_startPaused = 0; //local
char d_recentPath[MAX_PATH] = {'\0'};

#ifdef __cplusplus
extern "C" {
#endif
int SaveAsMD_(const char *filename, mdx_model_t *model);
#ifdef __cplusplus
}
#endif

static char recentFiles[8][256] = { "", "", "", "", "", "", "", "" };

/*long Skins=0;
long Vertices=0;
long Triangles=0;
long GlCommands=0;
long Frames=0;*/
//HYPOVERTEX
//int uiVertexID = -1;


void error(char *file, int line, char *err)
{
	FILE *fEerrorLog=NULL;
	if (fopen_s(&fEerrorLog, "error.log", "w") == 0)
	{
		fprintf(fEerrorLog, "%s:%d: %s\n", file, line, err);
		fclose(fEerrorLog);
		exit(1);
	}
}

void
MDXViewer::initRecentFiles ()
{
	for (int i = 0; i < 8; i++)
	{
		if (strlen (recentFiles[i]))
		{
			UI_mb->modify (IDC_MODEL_RECENTMODELS1 + i, IDC_MODEL_RECENTMODELS1 + i, recentFiles[i]);
		}
		else
		{
			UI_mb->modify (IDC_MODEL_RECENTMODELS1 + i, IDC_MODEL_RECENTMODELS1 + i, "(empty)");
			UI_mb->setEnabled (IDC_MODEL_RECENTMODELS1 + i, false);
		}
	}
}



void
MDXViewer::loadRecentFiles ()
{
	char tmpStr[256], path[256];

	strcpy_s(path, sizeof(path), mx::getApplicationPath());
	strcat_s(path, sizeof(path), "\\KP_Viewer.ini"); //hypov8 todo: .dat? .ini should be a text file

	FILE *file = NULL;
	if (fopen_s(&file, path, "rb")==0)
	{
		//hypov8 fail safe read of ini
		for (int i = 0; i < 8; i++) 
		{
			int rByte = fread(tmpStr, sizeof(recentFiles[i]), 1, file);
			if (rByte==1) //make sure its 1
			{
				tmpStr[255] = 0; //null term string
				memcpy(recentFiles[i], tmpStr, sizeof(recentFiles[i]));
			}
		}
		fclose(file);
	}
}



void
MDXViewer::saveRecentFiles ()
{
	char path[256];
	strcpy_s (path, sizeof(path), mx::getApplicationPath ());
	strcat_s (path,sizeof(path), "\\KP_Viewer.ini"); //hypov8 todo: .dat? .ini should be a text file

	FILE *file = NULL;
	if (fopen_s(&file, path, "wb")==0)
	{
		fwrite (recentFiles, sizeof recentFiles, 1, file);
		fclose (file);
	}
}


MDXViewer::MDXViewer ()
: mxWindow (0, 0, 0, 0, 0, "Kingpin Model Viewer " KP_BUILD_VERSION "(beta)", mxWindow::Normal) //hypov8 version
{
	// create menu stuff
	UI_mb = new mxMenuBar (this);
	mxMenu *menuModel = new mxMenu ();
	mxMenu *menuPack = new mxMenu (); //hypov8
	mxMenu *menuSkin = new mxMenu ();
	mxMenu *menuOptions = new mxMenu ();
	mxMenu *menuView = new mxMenu ();
	mxMenu *menuHelp = new mxMenu ();

	UI_mb->addMenu ("Model", menuModel);
	UI_mb->addMenu ("Pak ", menuPack); //hypov8
	UI_mb->addMenu ("Textures", menuSkin);
	UI_mb->addMenu ("Options", menuOptions);
	UI_mb->addMenu ("Help", menuHelp);

	mxMenu *menuRecentModels = new mxMenu ();
	menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS1);
	menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS2);
	menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS3);
	menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS4);

	mxMenu *menuRecentPakFiles = new mxMenu ();
	menuRecentPakFiles->add ("(empty)", IDC_MODEL_RECENTPAKFILES1);
	menuRecentPakFiles->add ("(empty)", IDC_MODEL_RECENTPAKFILES2);
	menuRecentPakFiles->add ("(empty)", IDC_MODEL_RECENTPAKFILES3);
	menuRecentPakFiles->add ("(empty)", IDC_MODEL_RECENTPAKFILES4);


	menuModel->add ("Load Model...", IDC_MODEL_LOADMODEL);
	menuModel->add ("Merge Model...", IDC_MODEL_MERGEMODEL);
	menuModel->add ("Load Player...", IDC_MODEL_LOAD_PMODEL);
	//menuModel->add ("Unload Models", IDC_MODEL_UNLOADMODEL); //hypov8 not needed
	menuModel->addSeparator ();
	menuModel->addMenu ("Recent Models", menuRecentModels);
	menuModel->addSeparator ();
	//menuModel-> add ("Save as MD2...", IDC_MODEL_MD2);
	menuModel-> add ("Save as MD2/MDX...", IDC_MODEL_SAVE);
	menuModel->addSeparator ();
	menuModel->add ("Exit", IDC_MODEL_EXIT);

	menuPack->add ("Open PAK file...", IDC_MODEL_OPENPAKFILE);
	menuPack->add ("Close PAK file", IDC_MODEL_CLOSEPAKFILE);
	menuPack->addSeparator ();
	menuPack->addMenu ("Recent PAK files", menuRecentPakFiles);

	menuSkin->add ("Load Model 1 Skin...", IDC_SKIN_MODELSKIN1);
	menuSkin->add ("Load Model 2 Skin...", IDC_SKIN_MODELSKIN2);
	menuSkin->add ("Load Model 3 Skin...", IDC_SKIN_MODELSKIN3);
	menuSkin->add ("Load Model 4 Skin...", IDC_SKIN_MODELSKIN4);
	menuSkin->add ("Load Model 5 Skin...", IDC_SKIN_MODELSKIN5);
	menuSkin->add ("Load Model 6 Skin...", IDC_SKIN_MODELSKIN6);
	menuSkin->addSeparator ();
	menuSkin->add("Reload all skins", IDC_SKIN_RELOAD);
	menuSkin->addSeparator();
		menuSkin->add ("Load Background Texture...", IDC_SKIN_BACKGROUND);
	menuSkin->add ("Load Water Texture...", IDC_SKIN_WATER);
#ifdef WIN32
	menuSkin->addSeparator ();
	menuSkin->add ("Make Screenshot...", IDC_SKIN_SCREENSHOT);	//hypov8 slow
	//menuSkin->add ("Make AVI File...", IDC_SKIN_AVI);				//hypov8 buggy
	//menuSkin->add("Make VU Map...", IDC_SKIN_UV);	//hypov8 todo
#endif
	menuOptions->add ("Background Color...", IDC_OPTIONS_BGCOLOR);
	menuOptions->add ("Wireframe Color...", IDC_OPTIONS_WFCOLOR);
	menuOptions->add ("Shade Color...", IDC_OPTIONS_FGCOLOR);
	menuOptions->add ("Light Color...", IDC_OPTIONS_LIGHTCOLOR);
	menuOptions->add ("Grid Color...", IDC_OPTIONS_GRIDCOLOR);
	menuOptions->add ("Developer Color...", IDC_OPTIONS_DEBUGCOLOR);

	/*menuOptions->addSeparator ();
	menuOptions->add ("Center on Model.", IDC_OPTIONS_CENTERMODEL1);
	menuOptions->add ("Center on Grid.", IDC_OPTIONS_CENTERMODEL2);*/ //moved to buttons
	menuOptions->addSeparator ();
	menuOptions->add ("Generate Light Normals.", IDC_OPTIONS_GEN_NORMALS);
	menuOptions->addSeparator();
	menuOptions->add("Load invalid models?", IDC_OPTIONS_LOADINVALID); //DEBUG
#ifdef WIN32
	menuHelp->add ("Goto Homepage...", IDC_HELP_GOTOHOMEPAGE);
	menuHelp->addSeparator ();
#endif
	menuHelp->add ("About", IDC_HELP_ABOUT);

	// create tabcontrol with subdialog windows
	UI_tab = new mxTab (this, 0, 0, 0, 0);
#ifdef WIN32
	SetWindowLong ((HWND)UI_tab->getHandle (), GWL_EXSTYLE, 0/* WS_EX_CLIENTEDGE*/); //hypov8
#endif

	mxWindow *wInfo = new mxWindow (this, 0, 0, 0, 0);
	mxWindow *wView = new mxWindow (this, 0, 0, 0, 0);

	// and add them to the tabcontrol
	UI_tab->add (wView, "View");
	//_tab->add (wAnim, "Animation");
	UI_tab->add (wInfo, "Model Info");

	/////////////////////////////////////
	// Create widgets for the 'View Tab'
	/////////////////////////////////////
	cRenderMode = new mxChoice (wView, 5, 5, 125, 22, IDC_RENDERMODE); //view mode
	cRenderMode->add ("Wireframe");
	cRenderMode->add ("Flat shaded");
	cRenderMode->add ("Smooth Shaded");
	cRenderMode->add ("Textured");
	cRenderMode->select (0);
	mxChoice *cTextureLimit = new mxChoice(wView, 130, 5, 60, 22, IDC_TEXTURELIMIT); //max texture rez
	cTextureLimit->add("512");
	cTextureLimit->add("256");
	cTextureLimit->add("128");
	cTextureLimit->select(0);
	cTextureLimit->setEnabled(0);


	//brightness slider
	mxLabel *lblBrightness = new mxLabel (wView, 65, 34, 60, 22, "Brightness"); //label
	mxSlider *slBrightness = new mxSlider (wView, 115, 31, 80, 22, IDC_BRIGHTNESS); //slider
	slBrightness->setRange (0, 100);
	slBrightness->setValue (5);


	cbWater =					new mxCheckBox (wView, 5, 30, 50, 22, "Water", IDC_WATER);
	cbLight =					new mxCheckBox (wView, 5, 55, 50, 22, "Light", IDC_LIGHT);
	mxCheckBox *cbShininess =	new mxCheckBox (wView, 55, 55, 50, 22, "Shine", IDC_SHININESS);
	cbBackground =				new mxCheckBox (wView, 110, 55, 78, 22, "Background", IDC_BACKGROUND);
	mxCheckBox *cbGrid =		new mxCheckBox (wView, 5, 80, 50, 22, "Grid", IDC_GRID);
	mxCheckBox *cbHitBox =		new mxCheckBox (wView, 55, 80, 50, 22, "HitBox", IDC_HITBOX);
	mxCheckBox *cbVNorms =		new mxCheckBox (wView, 110, 80, 90, 22, "Vertex Normals", IDC_VERTNORMS);
		
	cbLight->setChecked (false);
	cbShininess->setChecked (false);
	cbVNorms->setChecked (false);
	cbGrid->setChecked (true);
	cbHitBox->setChecked (false);


	mxToolTip::add (slBrightness, "Brightness - don't forget to reload the skin!");
	mxToolTip::add (cTextureLimit, "Choose Texture Size Limit");
	mxToolTip::add (cbVNorms, "MDX hitbox. Player models use these.");

	////////////////////////////////////
	// Create widgets for the Animation
	////////////////////////////////////
	int ofT = 205; //ani alignment offset

	cAnim = new mxChoice (wView, 5+ofT, 5, 170, 22, IDC_ANIMATION_SETS); //dropdown
	cbInterp = new mxCheckBox (wView, 5 + ofT, 30, 70, 22, "Interpolate", IDC_INTERPOLATE);
	cbInterp->setChecked (true);
	lAnimSpeed = new mxLabel(wView, 5 + ofT, 59, 75, 14, "Speed: 125");
	mxSlider *slPitch = new mxSlider (wView, 5 + ofT, 77, 98, 22, IDC_PITCH); //slider
	mxToolTip::add (slPitch, "Frame Animation Speed (Pitch)");
	slPitch->setRange (1, 200);
	slPitch->setValue (125);
	
	

	///////////////////////////////////
	// animation navigator
	///////////////////////////////////
	int ofT2 = 303;
	bPause = new mxButton (wView, 0 + ofT2, 30, 77, 22, "Pause", IDC_PAUSE);
	bDecFrame = new mxButton (wView, 0 + ofT2, 53, 20, 22, "<", IDC_DECFRAME);
	leFrame = new mxLineEdit (wView, 21 + ofT2, 53, 35, 22, "0"); //21
	bIncFrame = new mxButton (wView, 57 + ofT2, 53, 20, 22, ">", IDC_INCFRAME); //57
	bSetFrame = new mxButton (wView, 21 + ofT2, 76, 35, 22, "Set", IDC_BTN_SET_FRAME);
	bDecFrame->setEnabled (false);
	leFrame->setEnabled (false);
	bIncFrame->setEnabled (false);
	bSetFrame->setEnabled (false);
	mxToolTip::add (bDecFrame, "Decrease Current Frame");
	mxToolTip::add (leFrame, "Current Frame");
	mxToolTip::add (bSetFrame, "Set Current Frame");
	mxToolTip::add (bIncFrame, "Increase Current Frame");



	///////////////////////////////////
	// camera buttons group
	///////////////////////////////////
	ofT = 390;
	mxGroupBox * gbCamera = new mxGroupBox(wView, ofT, 0, 95, 100, "Camera");
	b1sPerson = new mxButton(wView, 10 + ofT, 20, 75, 22, "1st Person", IDC_1ST_PERSON); // button 1st person
	bCamToGrid = new mxButton(wView, 10 + ofT, 45, 75, 22, "Model", IDC_OPTIONS_CENTERMODEL2); // button 1st person
	bCamToModel = new mxButton(wView, 10 + ofT, 70, 75, 22, "Reset", IDC_OPTIONS_CENTERMODEL1); // button 1st person
	b1sPerson->setEnabled(true);
	bCamToGrid->setEnabled(true);
	bCamToModel->setEnabled(true);
	mxToolTip::add(b1sPerson, "Set viewport to match ingame");
	mxToolTip::add(bCamToGrid, "Set viewport to model centre");
	mxToolTip::add(bCamToModel, "Set viewport to default view");



	///////////////////////////////////////////
	// Create widgets for the 'Model Info' Tab
	///////////////////////////////////////////
	int ofT3 = 140; //shift input group
	lModelInfo1 = new mxLabel (wInfo, 5, 5, 150, 70, "No Model."); // model stats
	lModelInfo2 = new mxLabel (wInfo, 150, 5, 150, 70, "");		// bbox
	lModelInfo3 = new mxLabel (wInfo, 5, 70, 500, 42, "");			// model path

//HYPOVERTEX
	// show vertex number
	ofT3 = 330; // 140; //shift input group
	lModelInfo4 = new mxLabel (wInfo, 1 + ofT3, 5, 150, 22, "Find Vertex Index (0-Based)");
	bDecVertID = new mxButton(wInfo, 1 + ofT3, 24, 20, 22, "<", IDC_VERTEX_PREV);
	leVertex = new mxLineEdit(wInfo, 22 + ofT3, 23, 35, 22, "-1");
	bIncVertID = new mxButton(wInfo, 58 + ofT3, 24, 20, 22, ">", IDC_VERTEX_NEXT);
	bSetVertID = new mxButton(wInfo, 22 + ofT3, 46, 35, 22, "Set", IDC_VERTEX_SET);
	bSetVertID->setEnabled(true);
	leVertex->setEnabled (true);
	bDecVertID->setEnabled(true);
	bIncVertID->setEnabled(true);
	cbVertUseFace = new mxCheckBox(wInfo, 82 + ofT3, 24, 70, 22, "Face?", IDC_VERT_USEFACE);
	cbVertUseFace->setEnabled(true);
	mxToolTip::add(bSetFrame, "Set Current Vertex/Face");
	mxToolTip::add (leVertex, "Current Vertex/Face");
	mxToolTip::add(bDecVertID, "Previous Vertex/Face");
	mxToolTip::add(bIncVertID, "Next Vertex/Face");
	mxToolTip::add(cbVertUseFace, "Use face centre");
//END


	///////////////////////////
	// create the OpenGL window
	glw = new GlWindow (this, 0, 0, 0, 0, "", mxWindow::Normal);
#ifdef WIN32
	SetWindowLong ((HWND) glw->getHandle (), GWL_EXSTYLE, WS_EX_CLIENTEDGE);
#endif

	// finally create the pakviewer window
	pakViewer = new PAKViewer (this);
	pakViewer->setVisible(0);

	loadRecentFiles ();
	initRecentFiles ();

	//setBounds (20, 20, 690, 550);
	setBounds (20, 20, 520, 550); //hidden pak
	setVisible (true);

	//hypov8 file->open with
	int pos = 0;
	for (int i = 0; i < 6; i++)
	{
		if (loadmod[i][0] != NULL)
		{
			if (!loadModel(loadmod[i], pos))
			{
				char str[256];
				sprintf_s(str, sizeof(str), "Error reading model: %s", loadmod[i]);
				mxMessageBox(this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
			}
			else
				pos++;
		}
	}

	//set startup option for render method
	cRenderMode->select(d_startMode);
	setRenderMode(cRenderMode->getSelectedIndex());
}



MDXViewer::~MDXViewer ()
{
	saveRecentFiles ();
}

void
MDXViewer::reset_modelData(void)
{
	/*Skins = 0;
	Vertices = 0;
	Triangles = 0;
	GlCommands = 0;
	Frames = 0;*/

	glw->loadModel(0, TEXTURE_MODEL_0);
	glw->loadModel(0, TEXTURE_MODEL_1);
	glw->loadModel(0, TEXTURE_MODEL_2);
	glw->loadModel(0, TEXTURE_MODEL_3);
	glw->loadModel(0, TEXTURE_MODEL_4);
	glw->loadModel(0, TEXTURE_MODEL_5);

	glw->loadTexture(0, TEXTURE_MODEL_0);
	glw->loadTexture(0, TEXTURE_MODEL_1);
	glw->loadTexture(0, TEXTURE_MODEL_2);
	glw->loadTexture(0, TEXTURE_MODEL_3);
	glw->loadTexture(0, TEXTURE_MODEL_4);
	glw->loadTexture(0, TEXTURE_MODEL_5);
	setModelInfo(); // 0, TEXTURE_MODEL_0);
	glw->d_vertexIndex = -1; //HYPOVERTEX
	glw->d_vertexUseFace = 0; //HYPOVERTEX

	//glw->loadTexture(0, TEXTURE_BACKGROUND);
	//glw->loadTexture(0, TEXTURE_WATER);

	//reset model to animate
	char str[32];
	sprintf_s(str, sizeof(str), "%s", "-1");
	leVertex->setLabel(str);//HYPOVERTEX
	sprintf_s(str, sizeof(str), "%s", "0");
	leFrame->setLabel(str);
	glw->setFlag(F_PAUSE, 0);
	bDecFrame->setEnabled(0);
	leFrame->setEnabled(0);
	bIncFrame->setEnabled(0);
	bSetFrame->setEnabled(0);
	bPause->setEnabled(1);
	bPause->setLabel("Pause");
	cAnim->setEnabled(1);
	cbInterp->setEnabled(1);
	//
	cAnim->removeAll();
	glw->setFrameInfo(0, 0);
	glw->redraw();
	//mIndex = 0;
}

void
MDXViewer::setPauseMode(int frames)
{
	if (d_startPaused || frames == 1)
		setStartPaused();

	if (frames == 1)
	{
		bPause->setEnabled(0);
		bPause->setLabel("No Frames");
		cAnim->setEnabled(0);
		cbInterp->setEnabled(0);
	}

}

const char* 
MDXViewer::fileFilterString(int type, int multiType)
{
	const char *out = "";
	if (multiType)
	{
		switch (type)
		{
		case FILE_TYPE_MDX:
		case FILE_TYPE_MD2:
			out =	"Kingpin_Models(*.mdx;*.md2);;"
					"MDX_Model(*.mdx);;"
					"MD2_Model(*.md2)";
			break;
		case FILE_TYPE_TGA:
		case FILE_TYPE_PCX:
			out =	"Kingpin_Images(*.tga;*.pcx);;"
					"TGA_Image(*.tga);;"
					"PCX_File(*.pcx)";
			break;
		case FILE_TYPE_PAK:
			out =	"Pack_Files(*.pak)";
			break;
		}
	}
	else
	{
		switch (type)
		{
		case FILE_TYPE_MDX:
			out = "MDX_Model(*.mdx)";	break;
		case FILE_TYPE_MD2:
			out = "MD2_Model(*.md2)";	break;
		case FILE_TYPE_TGA:
			out = "TGA_Image(*.tga)";	break;
		case FILE_TYPE_PCX:
			out = "PCX_Files(*.pcx)";	break;
		case FILE_TYPE_PAK:
			out = "Pack_Files(*.pak)";	break;
		}
	}

	return out;
}

int
MDXViewer::importPlayerModelFolder(const char *ptr, int mode, int mIndex)
{
	if (ptr)
	{
		int i;
		char path[256];

		if (mode == IDC_MODEL_LOADMODEL)
		{
			reset_modelData(); //hypov8
			mIndex = 0;
		}

		if (!loadModel(ptr, mIndex))
		{
			char str[300];

			sprintf_s(str, sizeof(str), "Error reading model: %s", ptr);
			mxMessageBox(this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
			return 0; // break;
		}

		// now update recent files list
		strcpy_s(path, sizeof(path), "[m] ");
		strcat_s(path, sizeof(path), ptr);
		path[255] = 0; //hypov8 null

		for (i = 0; i < 4; i++)
		{
			if (!_stricmp(recentFiles[i], path))
				break;
		}

		// swap existing recent file
		if (i < 4)
		{
			char tmp[256];
			strcpy_s(tmp, 256, recentFiles[0]);
			strcpy_s(recentFiles[0], 256, recentFiles[i]);
			strcpy_s(recentFiles[i], 256, tmp);
		}

		// insert recent file
		else
		{
			for (i = 3; i > 0; i--)
				strcpy_s(recentFiles[i], 256, recentFiles[i - 1]);

			strcpy_s(recentFiles[0], 256, path);
		}

		initRecentFiles();
		return 1;
	}
	//else cancled?
	return 0;

}

int
MDXViewer::handleEvent (mxEvent *event)
{
	switch (event->event)
	{

	case mxEvent::Action:
	{
		switch (event->action)
		{
		case IDC_MODEL_LOADMODEL:
		case IDC_MODEL_MERGEMODEL:

		{
			int mIndex = glw->getModelIndex();
			if (mIndex < MAX_MODELS || event->action == IDC_MODEL_LOADMODEL)
			{
				const char *ptr = mxGetOpenFileName(this, 0, fileFilterString(FILE_TYPE_MDX, 1)); //.mdx
				if (ptr)
				{
					int tmp = importPlayerModelFolder(ptr, event->action, mIndex);
#if 0
					int i;
					char path[256];

					if (event->action == IDC_MODEL_LOADMODEL)
					{
						reset_modelData(); //hypov8
						mIndex = 0;
					}

					if (!loadModel(ptr, mIndex))
					{
						char str[300];

						sprintf_s(str, sizeof(str), "Error reading model: %s", ptr);
						mxMessageBox(this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
						break;
					}


					// now update recent files list
					strcpy_s(path, sizeof(path), "[m] ");
					strcat_s(path, sizeof(path), ptr);
					path[255] = 0; //hypov8 null

					for (i = 0; i < 4; i++)
					{
						if (!_stricmp(recentFiles[i], path))
							break;
					}

					// swap existing recent file
					if (i < 4)
					{
						char tmp[256];
						strcpy_s(tmp, 256, recentFiles[0]);
						strcpy_s(recentFiles[0], 256, recentFiles[i]);
						strcpy_s(recentFiles[i], 256, tmp);
					}

					// insert recent file
					else
					{
						for (i = 3; i > 0; i--)
							strcpy_s(recentFiles[i], 256, recentFiles[i - 1]);

						strcpy_s(recentFiles[0], 256, path);
					}

					initRecentFiles();
#endif
				}
				//else cancled?
			}
			else 
			{
				HWND hwID = (HWND)g_mdxViewer->getHandle();
				//max models
				MessageBox(hwID, "Limit of 6 models reached.\nUse \"Load Model\" instead ", "Note", MB_OK);
			}
		}
		break;

		case IDC_MODEL_LOAD_PMODEL:
		{
			//int mIndex = glw->getModelIndex();
			//if (mIndex < MAX_MODELS || event->action == IDC_MODEL_LOADMODEL)
			{
				//if (d_recentPath[0] == '\0')
				//	strcpy_s(d_recentPath, MAX_PATH, mx_getcwd()); // mx_getpath(mx::getApplicationPath()));

				const char *ptr = mxGetFolderPath(this, mx_getcwd()); // d_recentPath);
				if (ptr)
				{
					int idx = 0;
					static char *mdlName[4] = { "head.mdx", "body.mdx", "legs.mdx", "w_tommygun.mdx" };
					static char *skiName[4] = { "head_001.tga", "body_001.tga", "legs_001.tga", "0" }; //todo read .ini?

					reset_modelData(); //hypov8

					for (int i = 0; i < 4; i++)
					{
						char ptrFile[MAX_PATH];
						sprintf_s(ptrFile, sizeof(ptrFile), "%s/%s", ptr, mdlName[i]);
						if (importPlayerModelFolder((const char *)ptrFile, event->action, idx))
						{
							if (i < 3) {
								sprintf_s(ptrFile, sizeof(ptrFile), "%s/%s", ptr, skiName[i]);
								glw->loadTexture(ptrFile, idx);
							}
							idx += 1;
						}
						else
							break;
					}
					mx_setcwd(ptr);
					setModelInfo();
					setRenderMode(3); //allow viewing skins
					glw->redraw();
				}
			}
		}
		break;

		case IDC_MODEL_UNLOADMODEL:
		case IDC_MODEL_UNLOADWEAPON:
			reset_modelData(); //hypov8
			break;

		case IDC_MODEL_OPENPAKFILE:
		case IDC_MODEL_OPENPAKFILE2:
		{
			const char *ptr = mxGetOpenFileName (this, 0, fileFilterString(FILE_TYPE_PAK, 0)); //*.pak
			if (ptr)
			{
				pakViewer->setLoadEntirePAK (event->action == IDC_MODEL_OPENPAKFILE);
				if (!pakViewer->openPAKFile (ptr))
				{
					mxMessageBox (this, "Error loading PAK file", "ERROR", MX_MB_OK | MX_MB_ERROR);
					break;
				}

				// update recent pak file list

				int i;

				for (i = 4; i < 8; i++)
				{
					if (!_stricmp(recentFiles[i], ptr))
						break;
				}

				// swap existing recent file
				if (i < 8)
				{
					char tmp[256];
					strcpy_s (tmp, 256, recentFiles[4]);
					strcpy_s (recentFiles[4], 256, recentFiles[i]);
					strcpy_s (recentFiles[i], 256, tmp);
				}

				// insert recent file
				else
				{
					for (i = 7; i > 4; i--)
						strcpy_s (recentFiles[i], 256, recentFiles[i - 1]);

					strcpy_s (recentFiles[4], 256, ptr);
				}

				initRecentFiles ();

				redraw ();
			}
		}
		break;

		case IDC_MODEL_CLOSEPAKFILE:
		{
			pakViewer->closePAKFile ();
			pakViewer->setVisible(0);
			redraw ();
		}
		break;

		case IDC_MODEL_RECENTMODELS1:
		case IDC_MODEL_RECENTMODELS2:
		case IDC_MODEL_RECENTMODELS3:
		case IDC_MODEL_RECENTMODELS4:
		{
			int i = event->action - IDC_MODEL_RECENTMODELS1;
			bool isModel = recentFiles[i][1] == 'm';
			char *ptr = &recentFiles[i][4];

			if (isModel)
				reset_modelData();

			if (!loadModel (ptr, isModel ? 0:1))
			{
				char str[300];

				sprintf_s (str, sizeof(str), "Error reading model: %s", ptr);
				mxMessageBox (this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
				break;
			}

			// update recent model list

			char tmp[256];			
			strcpy_s (tmp, sizeof(tmp), recentFiles[0]);
			strcpy_s (recentFiles[0], 256, recentFiles[i]);
			strcpy_s (recentFiles[i], 256, tmp);

			initRecentFiles ();
		}
		break;

		case IDC_MODEL_RECENTPAKFILES1:
		case IDC_MODEL_RECENTPAKFILES2:
		case IDC_MODEL_RECENTPAKFILES3:
		case IDC_MODEL_RECENTPAKFILES4:
		{
			int i = event->action - IDC_MODEL_RECENTPAKFILES1 + 4;
			pakViewer->setLoadEntirePAK (true);
			pakViewer->openPAKFile (recentFiles[i]);

			char tmp[256];			
			strcpy_s (tmp, recentFiles[4]);
			strcpy_s (recentFiles[4], 256, recentFiles[i]);
			strcpy_s (recentFiles[i], 256, tmp);

			initRecentFiles ();

			redraw ();
		}
		break;

		case IDC_MODEL_SAVE:
		{
			mdx_model_t *model = glw->getModel(0);
			if (model)
			{
				int fileType[2] = {FILE_TYPE_MD2, FILE_TYPE_MDX };
				char *ext[2] = { ".md2", ".mdx"};
				const char *file = mxGetSaveFileName(this, 0, fileFilterString(fileType[model->isMD2], 0));
				if (file)
				{	
					//add file extension if missing
					if (mx_strncasecmp(mx_getextension(file), ext[model->isMD2], 4))
						strcat_s((char *)file, _MAX_PATH,  ext[model->isMD2]);

					if (!SaveAsMD_(file, model))
					{
						char str[256];
						sprintf_s(str, sizeof(str), "Error saving model: %s", file);
						mxMessageBox(this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
					}
				}
			}
		}
		break;

		case IDC_MODEL_EXIT:
			mx::setIdleWindow (0);
			mx::quit ();
			break;

		case IDC_SKIN_MODELSKIN1:
		case IDC_SKIN_MODELSKIN2:
		case IDC_SKIN_MODELSKIN3:
		case IDC_SKIN_MODELSKIN4:
		case IDC_SKIN_MODELSKIN5:
		case IDC_SKIN_MODELSKIN6:
		{
			const char *ptr = mxGetOpenFileName (this, 0, fileFilterString(FILE_TYPE_TGA, 1)); //*.tga then *.pcx
			if (ptr)
			{
				glw->loadTexture(ptr, event->action - IDC_SKIN_MODELSKIN1); //TEXTURE_MODEL_0
				setModelInfo();				
				setRenderMode (3); //allow viewing skins
				glw->redraw ();
			}
		}
		break;

		case IDC_SKIN_RELOAD:
		{
			int i;
			for (i = 0; i < MAX_TEXTURES; i++)
			{
				glw->loadTexture(glw->modelTexNames[i], i);
			}

			setRenderMode(3); //set mode to view skins
			glw->redraw();
		}
		break;

		case IDC_SKIN_BACKGROUND:
		case IDC_SKIN_WATER:
		{
			const char *ptr = mxGetOpenFileName (this, 0, fileFilterString(FILE_TYPE_TGA, 1));//*.tga *.pcx
			if (!ptr)
				break;

			if (glw->loadTexture (ptr, event->action == IDC_SKIN_BACKGROUND ? TEXTURE_BACKGROUND:TEXTURE_WATER))
			{
				if (event->action == IDC_SKIN_BACKGROUND)
				{
					cbBackground->setChecked (true);
					glw->setFlag (F_BACKGROUND, true);
				}
				else
				{
					cbWater->setChecked (true);
					glw->setFlag (F_WATER, true);
				}

				//setRenderMode (3);
				glw->redraw ();
			}
			else
				mxMessageBox (this, "Error loading texture.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);
		}
		break;

#ifdef WIN32
		case IDC_SKIN_SCREENSHOT:
		case IDC_SKIN_UV:
		{
			const char *ptr = mxGetSaveFileName(this, 0, fileFilterString(FILE_TYPE_TGA, 0)); //*.tga
			if (ptr)
			{
				if(mx_strncasecmp(mx_getextension(ptr), ".tga", 4))
					strcat_s((char *)ptr, _MAX_PATH,  ".tga");
				if (event->action == IDC_SKIN_UV)
					makeUVMapImage (ptr);
				else
					makeScreenShot (ptr);
			}
		//	int test = MakeAVI();
		}
		break;

		case IDC_SKIN_AVI:
		{
			int index = cAnim->getSelectedIndex ();
			if (index >= 0)
			{
				// set the animation
				int start, end, test;
				//FILE *te;
				initAVIAnimation (glw->getModel (0), index - 1, &start, &end);
				if(start==0 && end==0)
				{
					start = 1; 
					end = glw->getModel (0)->header.numFrames;
				}
			//	te = fopen("test.txt", "w+");
			//	fprintf(te, "%d start, %d end\n", start, end);
			//	fclose(te);
				if((test = MakeAVI(start, end))==0)
					mxMessageBox (this, "Error creating AVI File.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);
				
				glw->redraw ();
			}

		}
		break;
#endif

		case IDC_OPTIONS_BGCOLOR:
		case IDC_OPTIONS_FGCOLOR:
		case IDC_OPTIONS_WFCOLOR:
		case IDC_OPTIONS_LIGHTCOLOR:
		case IDC_OPTIONS_DEBUGCOLOR:
		case IDC_OPTIONS_GRIDCOLOR:
		{
			float r, g, b;
			int ir, ig, ib;

			if (event->action == IDC_OPTIONS_BGCOLOR)
				glw->getBGColor (&r, &g, &b);
			else if (event->action == IDC_OPTIONS_FGCOLOR)
				glw->getFGColor (&r, &g, &b);
			else if (event->action == IDC_OPTIONS_WFCOLOR)
				glw->getWFColor (&r, &g, &b);
			else if (event->action == IDC_OPTIONS_LIGHTCOLOR)
				glw->getLightColor (&r, &g, &b);
			else if (event->action == IDC_OPTIONS_DEBUGCOLOR)
				glw->getDebugColor (&r, &g, &b);
			else if (event->action == IDC_OPTIONS_GRIDCOLOR)
				glw->getGridColor (&r, &g, &b);

			ir = (int) (r * 255.0f);
			ig = (int) (g * 255.0f);
			ib = (int) (b * 255.0f);
			if (mxChooseColor (this, &ir, &ig, &ib))
			{
				if (event->action == IDC_OPTIONS_BGCOLOR)
				{
					glw->setBGColor((float)ir / 255.0f, (float)ig / 255.0f, (float)ib / 255.0f);
					glw->loadTexture(0, TEXTURE_BACKGROUND);
				}
				else if (event->action == IDC_OPTIONS_FGCOLOR)
					glw->setFGColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);
				else if (event->action == IDC_OPTIONS_WFCOLOR)
					glw->setWFColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);
				else if (event->action == IDC_OPTIONS_LIGHTCOLOR)
					glw->setLightColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);
				else if (event->action == IDC_OPTIONS_DEBUGCOLOR)
					glw->setDebugColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);
				else if (event->action == IDC_OPTIONS_GRIDCOLOR)
					glw->setGridColor ((float) ir / 255.0f, (float) ig / 255.0f, (float) ib / 255.0f);
				glw->redraw ();
			}
		}
		break;

		case IDC_OPTIONS_CENTERMODEL1:
			centerModel (glw->getCurrFrame(), 0);
			glw->redraw ();
		break;

		case IDC_OPTIONS_CENTERMODEL2:
			centerModel (glw->getCurrFrame(), 1);
			glw->redraw ();
		break;

		case IDC_OPTIONS_GEN_NORMALS:
			int i;
			for (i = 0; i < MAX_MODELS; i++) 
			{
				mdx_generateLightNormals(glw->getModel(i));
			}
			glw->redraw ();
			break;

		case IDC_OPTIONS_LOADINVALID:
			int val;
			val = !mxMessageBox(this, 
				"Use with caution.\n"
				"Load models with an invalid header?", "Caution:", MX_MB_YESNO | MX_MB_QUESTION);
			glw->setLoadInvalid(val); //hypov8
			break;

#ifdef WIN32
		case IDC_HELP_GOTOHOMEPAGE:
			ShellExecute (0, "open", "http://Kingpin.info", 0, 0, SW_SHOW);
			break;
#endif

		case IDC_HELP_ABOUT:
			mxMessageBox (this,
				"Info:" 
				"\tLoads mdx and md2 models. Support for 6 models.\n"
				"\tAbility to export .mdx models to .md2.\n"
				"\tAbility to export .md2 models to .mdx.\n"
				"\tPak viewer that loads models and textures.\n"
				"\n"
				"Keys:"
				"\tMouse-Left:\tdrag to rotate.\n"
				"\tMouse-Right:\tdrag to zoom.\n"
				"\tMouse-Middle:\tdrag to pan x-y (slow).\n"
				"\tMouse-Left+Shift:\tdrag to pan x-y (fast).\n"
				"\n"
				"bugs:"
				"\tOdd size TGA files not supported(eg 129* 64).\n"
				"\tNeed to left click first in PAK viewer.\n"
				"\n"
				"Thanks:"
				"\tTiCaL: Creator of MDX Viewer v1.1.\n"
				"\tMete Cirigan: for MD2 Viewer code.\n"
				"\tChris Cookson: for helping TiCaL.\n"
				"\n"
				"Build:\t" __DATE__ ".\n"
				"Email:\thypov8@kingpin.info\n"
				"Web:\thttp://Kingpin.info/",
				"Kingpin Model Viewer " KP_BUILD_VERSION " by Hypov8", //title //hypov8 version
				MX_MB_OK | MX_MB_INFORMATION);
			break;

		// 
		// widget actions
		//
		//

		//
		// Model Panel
		//

		case IDC_RENDERMODE:
			setRenderMode (cRenderMode->getSelectedIndex ());
			glw->redraw ();
			break;

		case IDC_WATER:
			glw->setFlag (F_WATER, cbWater->isChecked ());
			glw->redraw ();
			break;

		case IDC_LIGHT:
			glw->setFlag (F_LIGHT, cbLight->isChecked ());
			glw->redraw ();
			break;

		case IDC_BRIGHTNESS:
			glw->setBrightness (((mxSlider *) event->widget)->getValue ());
			break;

		case IDC_SHININESS:
			glw->setFlag (F_SHININESS, ((mxCheckBox *) event->widget)->isChecked ());
			glw->redraw ();
			break;

		case IDC_BACKGROUND:
			glw->setFlag (F_BACKGROUND, ((mxCheckBox *) event->widget)->isChecked ());
			glw->redraw ();
			break;

		case IDC_VERTNORMS: //hypov8
			glw->setFlag (F_VNORMS, ((mxCheckBox *) event->widget)->isChecked ());
			glw->redraw ();
			break;
		case IDC_GRID: //hypov8
			glw->setFlag (F_GRID, ((mxCheckBox *) event->widget)->isChecked ());
			glw->redraw ();
			break;
		case IDC_HITBOX: //hypov8
			glw->setFlag (F_HITBOX, ((mxCheckBox *) event->widget)->isChecked ());
			glw->redraw ();
			break;

		case IDC_TEXTURELIMIT:
		{
			int tl[3] = { 512, 256, 128 };
			int index = ((mxChoice *) event->widget)->getSelectedIndex ();
			if (index >= 0)
				glw->setTextureLimit (tl[index]);
		}
		break;

		//
		// Animation Panel
		//
		case IDC_ANIMATION_SETS:
		{
			int index = cAnim->getSelectedIndex ();
			if (index >= 0)
			{
				// set the animation
				initAnimation (glw->getModel (0), index - 1);

				// if we pause, update current frame in leFrame
				if (glw->getFlag (F_PAUSE))
				{
					char str[32];
					int frame = glw->getCurrFrame ();
					sprintf_s (str, sizeof(str), "%d", frame);
					leFrame->setLabel (str);
					glw->setFrameInfo (frame, frame);
				}

				glw->redraw ();
			}
		}
		break;

		case IDC_INTERPOLATE:
			glw->setFlag (F_INTERPOLATE, ((mxCheckBox *) event->widget)->isChecked ());
			glw->redraw();
			break;

		case IDC_PITCH:
		{
			char str[32];
			float speed = (float)((mxSlider *)event->widget)->getValue();
			glw->setPitch(200 -speed); //hypov8 UI: invert
			sprintf_s(str, sizeof(str), "Speed: %i", (int)(speed));
			lAnimSpeed->setLabel(str);
		}
		break;

		case IDC_PAUSE: // Pause/Play
		{
			bool pause = !glw->getFlag (F_PAUSE);
			static int startFrame = 0, endFrame = 0, currFrame = 0, currFrame2 = 0;
			static float pol = 0;
			static int index;

			glw->setFlag (F_PAUSE, pause);
			bDecFrame->setEnabled (pause);
			leFrame->setEnabled (pause);
			bIncFrame->setEnabled (pause);
			bSetFrame->setEnabled (pause);

			if (pause)
			{
				char str[32];

				// store current settings
				startFrame = glw->getStartFrame ();
				endFrame = glw->getEndFrame ();
				currFrame = glw->getCurrFrame ();
				currFrame2 = glw->getCurrFrame2 ();
				pol = glw->d_pol;

				sprintf_s (str, sizeof(str), "%d", glw->getCurrFrame ());
				leFrame->setLabel (str);
				bPause->setLabel ("Play");

				index = cAnim->getSelectedIndex ();
			}
			else
			{ //todo: this is wrong when resuming after a new model loaded or manual frame change
				const char *ptr = leFrame->getLabel ();
				if (ptr)
				{
					int frame = atoi(ptr);
					if (frame != currFrame)
					{
						currFrame = frame;
						currFrame2 = frame+1;

						if (currFrame > endFrame)
							currFrame = endFrame;
						if (currFrame2 > endFrame)
							currFrame2 = startFrame;
						pol = 0;
					}
				}
				glw->d_startFrame = startFrame;
				glw->d_endFrame = endFrame;
				glw->d_currFrame = currFrame;
				glw->d_currFrame2 = currFrame2;
				glw->d_pol = pol;

				bPause->setLabel ("Pause");
				//frame set changes
				int index2 = cAnim->getSelectedIndex ();
				if (index2 >= 0 && index != index2)
				{
					initAnimation(glw->getModel(0), index2 - 1);
				}
			}
		}
		break;

		case IDC_DECFRAME:
		case IDC_BTN_SET_FRAME:
		case IDC_INCFRAME:
		{
			const char *ptr = leFrame->getLabel ();
			int frame = glw->getCurrFrame ();
			if (ptr)
				 frame = atoi (ptr);

			if (event->action == IDC_INCFRAME)
				frame += 1;
			else if (event->action == IDC_DECFRAME)
				frame -= 1;

			glw->setFrameInfo (frame, frame);

			char str[32];
			sprintf_s (str, sizeof(str), "%d", glw->getCurrFrame ());
			leFrame->setLabel (str);
			glw->redraw ();
		}
		break;

		case IDC_1ST_PERSON:
		{	//hypov8 set viewport to 1st person
			glw->d_rotX =0;
			glw->d_rotY =180;
			glw->d_transX=0;
			glw->d_transY=0;
			glw->d_transZ=3; //move fov back slightly
			glw->redraw ();
		}
		break;

//HYPOVERTEX
		case IDC_VERTEX_NEXT:
		case IDC_VERTEX_PREV:
		case IDC_VERTEX_SET:
		case IDC_VERT_USEFACE: //checkbox clicked
		{
			char str[32];
			int maxIdx = -1;
			int cutVertID = atoi(leVertex->getLabel());
			int isFace = cbVertUseFace->isChecked();

			mdx_model_t *model = glw->getModel(0);
			if (model)
			{
				if (isFace)
					maxIdx = glw->getModel(0)->header.numTriangles - 1; // : MDL_MAX_TRIANGLES;
				else
					maxIdx = glw->getModel(0)->header.numVertices - 1; // : MDL_MAX_VERTICES;
			}

			if (event->action == IDC_VERTEX_NEXT)
				cutVertID += 1;
			else if (event->action == IDC_VERTEX_PREV)
				cutVertID -= 1;

			if (cutVertID < -1)
				cutVertID = -1;
			else if (cutVertID > maxIdx)
				cutVertID = maxIdx;

			sprintf_s (str, sizeof(str), "%d", cutVertID);
			leVertex->setLabel (str);

			if (isFace)
				glw->d_vertexUseFace = 1;
			else
				glw->d_vertexUseFace = 0;

			glw->d_vertexIndex = cutVertID;
			glw->redraw();
		}
		break;
//END_HYPOVERTEX

		}//end switch
	} // mxEvent::Action
	break;

	case mxEvent::Size:
	{
		int w = event->width;
		int h = event->height;
		int y = UI_mb->getHeight ();
#ifdef WIN32
#define HEIGHT 128 //104 //hypov8 was 120 with progress bar
#else
#define HEIGHT 140
		h -= 40;
#endif

		//resize
		if (pakViewer->isVisible ())
		{
			w -= 170;
			pakViewer->setBounds (w, y, 170, h); //hypov8 UI: 
		}
		//fix tga bug for avi file....always make glw even width
		if (w&1) 
			w+=1;

		glw->setBounds (0, y, w/*- pakW*/, h - HEIGHT);
		//UI_bar->setBounds (0, y + h - 16, w-2, HEIGHT-105); //progress bar!!
		UI_tab->setBounds (0, y + h - HEIGHT, w-2, HEIGHT);
		
	}
	break;
	} // event->event


	return 1;
}



void
MDXViewer::redraw ()
{
	mxEvent event;
	event.event = mxEvent::Size;
	event.width = w2 ();
	event.height = h2 ();
	handleEvent (&event);
}


void
MDXViewer::setStartPaused()
{
	mxEvent event;
	event.event = mxEvent::Action;
	event.action = IDC_PAUSE;
	handleEvent (&event);
}



void
MDXViewer::makeScreenShot (const char *filename)
{
#ifdef WIN32
	glw->redraw ();
	int w = glw->w2 ();
	int h = glw->h2 ();

	mxImage *image = new mxImage ();
	if (image->create (w, h, 24))
	{
#if KINGPIN
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadBuffer (GL_FRONT);
		glReadPixels (0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, image->data);
#else
		HDC hdc = GetDC ((HWND) glw->getHandle ());
		byte *data = (byte *) image->data;
		int i = 0;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				COLORREF cref = GetPixel (hdc, x, y);
				data[i++] = (byte) ((cref >> 0)& 0xff);
				data[i++] = (byte) ((cref >> 8) & 0xff);
				data[i++] = (byte) ((cref >> 16) & 0xff);
			}
		}
		ReleaseDC ((HWND) glw->getHandle (), hdc);
#endif
		if (!mxTgaWrite (filename, image))
			mxMessageBox (this, "Error writing screenshot.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);

		delete image;
	}
#endif
}


#if KINGPIN
void
MDXViewer::makeUVMapImage(const char *filename)
{
	int w = 1024;
	int h = 1024;

	mxImage *image = new mxImage ();
	if (image->create (w, h, 24))
	{
		byte *data = (byte *) image->data;
		int i = 0;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{	//fill black
				data[i++] = (byte)0x00;
				data[i++] = (byte)0x00;
				data[i++] = (byte)0x00;
			}
		}
		mdx_model_t*mdl = glw->getModel(0);
		if (mdl)
		{
			int tri, triCnt = mdl->header.numTriangles;
			for (tri = 0; tri < triCnt; tri++)
			{
				// todo mdl->
				for (int y = 0; y < h; y++)
				{
					for (int x = 0; x < w; x++)
					{	//fill black
						data[i++] = (byte)0x00;
						data[i++] = (byte)0x00;
						data[i++] = (byte)0x00;
					}
				}
			}
		}
		

		if (!mxTgaWrite (filename, image))
			mxMessageBox (this, "Error writing screenshot.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);

		delete image;
	}
}
#endif


void
MDXViewer::setRenderMode (int mode)
{
	if (mode >= 0)
	{
		cRenderMode->select (mode);
		glw->setRenderMode (mode);

	//  disable light, if not needed
	//	glw->setFlag (F_LIGHT, mode != 0);
	//	cbLight->setChecked (mode != 0);
	}
}



void
MDXViewer::centerModel (int frame, int pos2)
{
	if (glw->getModel (0))
	{
		float min[3], max[3];

		mdx_getBoundingBox (glw->getModel(0), min, max, frame); // todo current frame?

		// adjust distance
		float dx = max[0] - min[0];
		float dy = max[1] - min[1];
		float dz = max[2] - min[2];
		float xPos = (max[0]/2) + (min[0]/2);
		float xPos2 = (max[0] + min[0]) / 2;
		float xPos3 = (max[0] - dx) / 2;

		if (0) 0;

		float d = dx;
		if (dy > d)
			d = dy;

		if (dz > d)
			d = dz;

		if (xPos3 > d)
			d = xPos3;
		/*if ((max[0]/2) > d)
			d = (max[0]/2);
		float minHalve = (float)(abs((long)min[0]) / 2);
		if (minHalve > d)
			d = minHalve;*/

		// center on model
		if (pos2)
		{
			glw->d_transX = (max[0] + min[0]) / 2; // dy; //centre on model //grid
			glw->d_transY = (max[1] + min[1]) / 2; //min[1] + (dy / 2);
			glw->d_transZ = max[2]; // +(dy / 2); // d * 1.2f; //z-depth
		}
		else //centre on grid
		{
			glw->d_transX = xPos3; //centre on grid
			glw->d_transY = (max[1] + min[1]) / 2;
			glw->d_transZ = d * 1.2f; //z-depth
		}
		glw->d_rotX = glw->d_rotY = 0.0f;
		glw->d_modelOriginX = dx;
		glw->d_modelOriginZ = dz;
	}
}



bool
MDXViewer::loadModel (const char *ptr, int pos)
{
	mdx_model_t *model;

	model = glw->loadModel (ptr,  pos);
	if (!model)
		return false;

	if (pos == TEXTURE_MODEL_0)
	{	
		initAnimation (model, -1);
		centerModel (0, 0);	
		//if (model->header.numFrames == 1)
		setPauseMode(model->header.numFrames);
	}

	glw->loadTexture(glw->modelTexNames[pos], pos);
	setModelInfo(); // model, pos); //hypov8 print details
	glw->redraw ();

	return true;
}

//#define getMin(a, b) ((a<b)? a:b)
//#define getMax(a, b) ((a>b)? a:b)

void
MDXViewer::setModelInfo () //mdx_model_t *model, int pos)
{
	static char str1[1024];
	static char str2[1024];
	static char str3[1024];

	bool found = false;
	long Skins = 0;
	long Vertices = 0;
	long Triangles = 0;
	long GlCommands = 0;
	long Frames = 0;
	float bMin[3] = {9999.9f, 9999.9f};
	float bMax[3] = {-9999.9f, -9999.9f};
	mdx_model_t *model;

	for (int i = 0; i < 6; i++)
	{
		model = glw->getModel(i);
		if (model)
		{
			found = true;
			if (i == 0)
			{
				Frames = model->header.numFrames;
			}
			Skins += model->header.numSkins;
			Vertices += model->header.numVertices;
			Triangles += model->header.numTriangles;
			GlCommands += model->header.numGlCommands;

			bMin[0] = min(model->bBoxMin[0], bMin[0]);
			bMin[1] = min(model->bBoxMin[1], bMin[1]);
			bMin[2] = min(model->bBoxMin[2], bMin[2]);

			bMax[0] = max(model->bBoxMax[0], bMax[0]);
			bMax[1] = max(model->bBoxMax[1], bMax[1]);
			bMax[2] = max(model->bBoxMax[2], bMax[2]);
		}
	}

	if (!found)
	{
		strcpy_s(str1, sizeof(str1), "No Models.");
		str2[0] = '\0';
		str3[0] = '\0';
	}
	else
	{
		sprintf_s(str1, sizeof(str1),
			"Skins: %d\n"
			"Vertices: %d\n"
			"Triangles: %d\n"
			"GlCommands: %d\n"
			"Frames: %d\n",
			Skins,
			Vertices,
			Triangles,
			GlCommands,
			Frames);

		sprintf_s(str2, sizeof(str2),
			"BBox (Frame 0):\n"
			"min (%5.1f, %5.1f, %5.1f)\n"
			"max (%5.1f, %5.1f, %5.1f)",
			bMin[0], bMin[1], bMin[2],
			bMax[0], bMax[1], bMax[2]);

		sprintf_s(str3, sizeof(str3),
			"Model: %s\n"
			"Skin0: %s", 
			glw->modelFileNames[0], 
			glw->modelTexNames[0]);
	}

	lModelInfo1->setLabel(str1); //model stats
	lModelInfo2->setLabel(str2); //bbox
	lModelInfo3->setLabel(str3); //model path/texture
}



void
MDXViewer::initAnimation (mdx_model_t *model, int animation)
{
	cAnim->removeAll ();

	if (!model)
		return;

	int count = mdx_getAnimationCount (model);

	cAnim->add ("<All Animations>");

	for (int i = 0; i < count; i++)
		cAnim->add (mdx_getAnimationName (model, i));

	int startFrame, endFrame;
	mdx_getAnimationFrames (model, animation, &startFrame, &endFrame);
	glw->setFrameInfo (startFrame, endFrame);

	if (animation == -1)
		glw->setFrameInfo (0, model->header.numFrames - 1);

	cAnim->select (animation + 1);
}

void
MDXViewer::initAVIAnimation (mdx_model_t *model, int animation, int *startFrame, int *endFrame)
{
	if (!model)
		return;

	mdx_getAnimationFrames (model, animation, startFrame, endFrame);
	
}


int MDXViewer::MakeAVI(int start, int end)
{
#if 1 //hypov8
	return 0;
#else
	if(!glw->getModel(0)) return 0;
		
	char *outfn = (char *)malloc(256);
	int fps = 0;
	int img_width, img_height;
	int frame_num, avi_frame;
	char pattern[256];
	int frame_begin, frame_end, frame_step;
	char fn[1024];

	mxImage *in = new mxImage ();
	mxImage *tga = new mxImage ();

	LPBITMAPINFOHEADER bih;
	PAVIFILE af = NULL;
	AVISTREAMINFO asi;
	PAVISTREAM as = NULL, ascomp = NULL;
	AVICOMPRESSOPTIONS opts;
	AVICOMPRESSOPTIONS FAR * aopts[1] = {&opts};
	HANDLE dib;
	int memsize;
	int test2 = NULL;
	char *test3 = NULL;

	outfn = (char *)mxGetSaveFileName (this, 0, fileFilterString(FILE_TYPE_AVI/*, "*.avi"*/, 0));
	if(!outfn)
		return 0;

	if((test3 = strstr(outfn, ".avi"))==NULL)
 			strcat_s(outfn, 256, ".avi");
	
	fps = 10;
	//strcpy_s(pattern, sizeof(pattern), "shot%04d.tga"); //hypov8 moved below

	//frame_begin = 1;
	frame_begin = start;
	//frame_end = glw->getModel(0)->header.numFrames;
	frame_end = end;
	frame_step = 1;

	if (frame_end < frame_begin) 
		ERR("Invalid end frame!");
	if (frame_step < 1) 
		ERR("Invalid frame step!");
	
	//check for existance and format of first frame 
	//sprintf_s(fn, sizeof(fn),pattern, frame_begin);
	sprintf_s(fn, sizeof(fn), "shot%04d.tga", frame_begin);

	char *test = (char *)malloc(256);
	GetTempPath (256, test);
	strcat_s(test,256, "/");
	strcat_s(test,256, fn);
	makeScreenShot (test);
	
	tga = mxTgaRead(test);
	if (!tga) return 0;

	img_width = tga->width;
	img_height = tga->height;
	remove (test);

	// allocate DIB 
	memsize = sizeof(BITMAPINFOHEADER) + (img_width*img_height*3);
	dib = GlobalAlloc(GHND,memsize);
	bih = (LPBITMAPINFOHEADER)GlobalLock(dib);

	bih->biSize = sizeof(BITMAPINFOHEADER);
	bih->biWidth = img_width;
	bih->biHeight = img_height; // negative = top-down 
	bih->biPlanes = 1;
	bih->biBitCount = 24; // B,G,R 
	bih->biCompression = BI_RGB;
	bih->biSizeImage = memsize - sizeof(BITMAPINFOHEADER);
	bih->biXPelsPerMeter = 0;
	bih->biYPelsPerMeter = 0;
	bih->biClrUsed = 0;
	bih->biClrImportant = 0;

	if (HIWORD(VideoForWindowsVersion()) < 0x010a)
		ERR("VFW is too old");

	AVIFileInit();
	AVIERR( AVIFileOpen(&af, outfn, OF_WRITE | OF_CREATE, NULL) );

	// header 
	memset(&asi, 0, sizeof(asi));
	asi.fccType	= streamtypeVIDEO;// stream type
	asi.fccHandler	= 0;
	asi.dwScale	= 1;
	asi.dwRate	= fps;
	asi.dwSuggestedBufferSize  = bih->biSizeImage;
	SetRect(&asi.rcFrame, 0, 0, img_width, img_height);

	AVIERR( AVIFileCreateStream(af, &as, &asi) );

	memset(&opts, 0, sizeof(opts));

	if (!AVISaveOptions(NULL, 0, 1, &as,
		(LPAVICOMPRESSOPTIONS FAR *) &aopts))
	{
		return 0;
	}

	AVIERR( AVIMakeCompressedStream(&ascomp, as, &opts, NULL) );

	AVIERR( AVIStreamSetFormat(ascomp, 0,
			       bih,	    // stream format
			       bih->biSize) ); // format size

	avi_frame = 0;
	frame_num = 1;

	int current = glw->getCurrFrame ();
	
	//bar->setTotalSteps(glw->getModel(0)->header.numFrames);
	bar->setTotalSteps(end-start);

	mxImage *flip = new mxImage ();

	int cnt=0;
	//for (int frame = 1;frame<=glw->getModel(0)->header.numFrames;frame++,frame_num++)
	for (int frame = start;frame<=end;frame++,frame_num++, cnt++)
	{		
		//Increment progress bar
		bar->setValue(cnt);
				
		//init strings
		char *filename = (char *)malloc(256);
		char *str = (char *)malloc(32);
		char *path = (char *)malloc(256);

		//Set to first frame and take a shot
		glw->setFrameInfo (frame, frame);
		sprintf_s (str, sizeof(byte) * 32, "%d", glw->getCurrFrame ());
		leFrame->setLabel (str);
		glw->redraw ();
		sprintf_s (filename, sizeof(byte)*256, "/shot%04d.tga", frame);
		GetTempPath (256, path);
		strcat_s (path, 256, filename);
		makeScreenShot (path);

		//Create an AVI file
		in = mxTgaRead(path);
		if (!in) return 0;

		//Flip the image
		flip->data = (unsigned char *)malloc(in->height*in->width*3);
		flip->height = in->height;
		flip->width = in->width;

		int len = (flip->height*flip->width*3)-1;

		for (int i = 0;i <= (flip->height*flip->width * 3) - 1;i++, len--)
			flip->data[i] = in->data[len];
		
		for (long y=0; y<flip->height; y++) 
		{ 
			long left = y*flip->width*3; 
			long right = left + flip->width*3 - 3; 
			for (long x=0; x<flip->width*0.5; x++) 
			{ 
				unsigned char tempRed   = flip->data[left]; 
				unsigned char tempGreen = flip->data[left+1]; 
				unsigned char tempBlue  = flip->data[left+2]; 
				
				flip->data[left]   = flip->data[right]; 
				flip->data[left+1] = flip->data[right+1]; 
				flip->data[left+2] = flip->data[right+2]; 
				
				flip->data[right]   = tempRed; 
				flip->data[right+1] = tempGreen; 
				flip->data[right+2] = tempBlue; 
				
				left  += 3; 
				right -= 3; 
			} 
		} 

		memcpy((LPBYTE)(bih) + sizeof(BITMAPINFOHEADER), 
			flip->data, bih->biSizeImage);

		AVIERR( AVIStreamWrite(ascomp,	// stream pointer
			avi_frame,		// time of this frame
			1,			// number of frames to write
			(LPBYTE)(bih) + sizeof(BITMAPINFOHEADER),
			bih->biSizeImage,	// size of this frame
			0,			// flags
			NULL,
			NULL) );
	
		avi_frame++;

		//Clean up the mess
		remove (path);
		delete [] str;
		delete [] filename;
		delete [] path;

	}

	//Close AVI Stuff
	if (ascomp) AVIStreamClose(ascomp);
	if (as) AVIStreamClose(as);
	if (af) AVIFileClose(af);
	GlobalUnlock(dib);
	AVIFileExit();

	//Set to initial frame
	char *str = (char *)malloc(32);
	glw->setFrameInfo (current, current);
	sprintf_s (str, sizeof(byte) * 32, "%d", glw->getCurrFrame ());
	leFrame->setLabel (str);
	glw->redraw ();
	delete [] str;

	free(in->data);
	free(tga->data);
	free(flip->data);
	
	in->~mxImage();
	tga->~mxImage();
	flip->~mxImage();

	delete in;
	delete tga;
	delete flip;
	
	bar->setValue(0);
	int n = _heapmin();
	return 1;
#endif
}

/*
argumens search
-0=stat wireframe
-1=start flat shade
-2=smooth shaded
-3=start textured
folder/file name
*/
void ProcessArgs(int argc, char *argv[])
{
	int aIdx = 1;
	int fIdx = 0;
	d_startPaused = 0;
	d_startMode = 0;
	while (aIdx < argc)
	{
		if (argv[aIdx][0] == '-')
		{
			byte a = (byte)argv[aIdx][1] - 48;
			if (a == 64) //-p
				d_startPaused = 1;
			else
			{	//-0, -1, -2, -3 (render mode)
				if (a < 0)
					a = 0;
				if (a > 3)
					a = 3;
				d_startMode = (int)a;
			}
		}
		else if (fIdx < 6)
		{	
			//copy any other args to filename
			strcpy_s(loadmod[fIdx], sizeof(loadmod[fIdx]), argv[aIdx]);
			fIdx++;
		}
		aIdx++;
	}
}


int
main (int argc, char *argv[])
{
	int ret;
	//
	// make sure, we start in the right directory
	//
	SetCurrentDirectory (mx::getApplicationPath ());

	mx::init (argc, argv);

	if(argc>1)
		ProcessArgs(argc, argv);

	g_mdxViewer = new MDXViewer ();
	g_mdxViewer->setMenuBar (g_mdxViewer->getMenuBar ());

	ret =  mx::run ();
	mx::cleanup();

	return ret;
}
