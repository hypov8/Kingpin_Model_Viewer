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

//version 1.1.6.12
// - fix bbox display size

//version 1.1.6.13
// - limited framerate to ~30fps (fix high cpu usages)
// - frame name code cleanup
// - fix pause frames using 0 range
// - live framerate added
// - read players/skinfolder.txt for loading "players" from menu
// - changed speed slider to reflect fps
// - brightnedd slider updated realtime(re-load textures)
// - draw debug order changed. added wireframe over textured faces
// - KP_Viewer.ini changed to text file
// - added upscale of images(power of 2) so any size image loads correctly
// - added UV export to tga. 2k mode upscales but keeps aspect ratio. (note: odd sizes changed to even)
// - fixed screenshot
// - store light/grid/render/pause setting in KP_Viewer.ini. startup switches disabled
// - fix mouse wheel scroll(zoom) in 3D
// - add hotkeys. P=pause, 1-4=render modes, F5=refresh skinns. (note: hotkeys only work within 3D area)
// - added menu item. start paused. setting stored in .ini

//version 1.1.6.14
// - added debug wireframe view options. show glcommands(red=fan, green=strip)
// - fixed animation selector
// - added show HD to info tab


//todo 
//stop building models every frame (use buffer)
//add md2 software tex cords?
//focus in 3d. fixed


MDXViewer *g_mdxViewer = 0;

char startupModelsNames[MAX_MODELS][MAX_PATH_LEN];


#ifdef __cplusplus
extern "C" {
#endif
int SaveAsMD_(const char *filename, mdx_model_t *model);
#ifdef __cplusplus
}
#endif


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
MDXViewer::updateRecentPaths_(char paths[NUM_RECENT_FILES][MAX_PATH_LEN], char *newPath)
{
	int i;

	//check matching entry..
	for (i = 0; i < NUM_RECENT_FILES; i++)
	{
		if (!_stricmp(paths[i], newPath))
			break;
	}

	//no match
	if (i == NUM_RECENT_FILES)
		i--;

	// shift existing to +1 entry (remove last if list full)
	for (; i > 0; i--)
	{
		strcpy_s(paths[i], sizeof(paths[0]), paths[i - 1]);
	}

	//copy new to index 0
	strcpy_s(paths[0], sizeof(paths[0]), newPath);
}

void
MDXViewer::updateRecentPaths(int eventID, char *newPath)
{
	if (eventID >= IDC_MODEL_RECENTPAKFILES1 && eventID < IDC_MODEL_RECENTPAKFILES1 + NUM_RECENT_FILES)
		updateRecentPaths_(d_recentPakFiles, newPath);
	else if (eventID >= IDC_MODEL_RECENTMODELS1 && eventID < IDC_MODEL_RECENTMODELS1 + NUM_RECENT_FILES)
		updateRecentPaths_(d_recentModelFiles, newPath);
}


void
MDXViewer::updateRecentMenu_(char strings[NUM_RECENT_FILES][MAX_PATH_LEN], int eventID)
{
	for (int i = 0; i < NUM_RECENT_FILES; i++)
	{
		if (strlen (strings[i]))//d_recentModelFiles
		{
			UI_mb->modify (eventID + i, eventID + i, strings[i]); //d_recentModelFiles
			UI_mb->setEnabled (eventID + i, true);
		}
		else
		{
			UI_mb->modify (eventID + i, eventID + i, "(empty)");
			UI_mb->setEnabled (eventID + i, false);
		}
	}
}

void
MDXViewer::updateRecentMenu(int eventID)
{
	if (eventID >= IDC_MODEL_RECENTPAKFILES1 && eventID < IDC_MODEL_RECENTPAKFILES1 + NUM_RECENT_FILES)
		updateRecentMenu_(d_recentPakFiles, eventID);
	else if (eventID >= IDC_MODEL_RECENTMODELS1 && eventID < IDC_MODEL_RECENTMODELS1 + NUM_RECENT_FILES)
		updateRecentMenu_(d_recentModelFiles, eventID);
}


void
MDXViewer::initRecentFiles ()
{
	for (int i = 0; i < NUM_RECENT_FILES; i++)
	{
		//model files
		//UI_mb->modify (IDC_MODEL_RECENTMODELS1 + i, IDC_MODEL_RECENTMODELS1 + i, "(empty)");
		//UI_mb->setEnabled (IDC_MODEL_RECENTMODELS1 + i, false);
		d_recentModelFiles[i][0] = 0;
		//pak files
		//UI_mb->modify (IDC_MODEL_RECENTPAKFILES1 + i, IDC_MODEL_RECENTPAKFILES1 + i, "(empty)");
		//UI_mb->setEnabled (IDC_MODEL_RECENTPAKFILES1 + i, false);
		d_recentPakFiles[i][0] = 0;
	}
}


void
MDXViewer::loadConfigFile ()
{
	char path[MAX_PATH_LEN];
	char line[512];
	int countPak = 0;
	int countModel = 0;
	int mode = 0;

	strcpy_s(path, sizeof(path), mx::getApplicationPath());
	strcat_s(path, sizeof(path), "\\KP_Viewer.ini");

	FILE *file = NULL;
	if (fopen_s(&file, path, "r")==0)
	{
		while (fgets(line, sizeof(line), file))
		{
			if (!strlen(line))
				continue;
			if (line[0] == '[')
			{
				//change modes
				if (!strncmp(line,      "[model]", 7))
					mode = 1;
				else if (!strncmp(line, "[pak]", 5))
					mode = 2;
				else if (!strncmp(line, "[settings]", 10))
					mode = 3;
				else
					mode = 0;
				continue;
			}

			//clean line ending
			line[strcspn(line, "\n")] = 0;
			line[strcspn(line, "\r")] = 0;

			switch (mode)
			{
				case 1:
				{
					if (countModel < NUM_RECENT_FILES)
					{
						strcpy_s(d_recentModelFiles[countModel], sizeof(d_recentModelFiles[0]), line);
						countModel++;
					}
					break;
				}
				case 2:
				{
					if (countPak < NUM_RECENT_FILES)
					{
						strcpy_s(d_recentPakFiles[countPak], sizeof(d_recentPakFiles[0]), line);
						countPak++;
					}
					break;
				}
				case 3:
				{
					char *s;
					int val;
					if (!strncmp(line, "mode=", 5))
					{
						s = line + 5;
						val = atoi(s);
						if (val >= 0 && val <= 3)
							d_startMode = val; //cRenderMode->
					}
					else if (!strncmp(line, "pause=", 6))
					{
						val = atoi(line + 6);
						d_startPaused = (val > 0);
					}
					else if (!strncmp(line, "grid=", 5))
					{
						val = atoi(line + 5);
						cbGrid->setChecked((val > 0));
					}
					else if (!strncmp(line, "light=", 6))
					{
						val = atoi(line + 6);
						cbLight->setChecked((val > 0));
					}
				}
				break;
			}
		}

		fclose(file);
	}
}



void
MDXViewer::saveConfigFile ()
{
	char path[256];

	strcpy_s (path, sizeof(path), mx::getApplicationPath ());
	strcat_s (path,sizeof(path), "\\KP_Viewer.ini"); //hypov8 todo: .dat? .ini should be a text file

	FILE *file = NULL;
	if (fopen_s(&file, path, "w")==0)
	{
		bool grid = glw->getFlag(F_GRID);
		bool light = glw->getFlag(F_LIGHT);
		//ui settings
		fprintf(file, "[settings]\n");
		fprintf(file, "mode=%i\n",  glw->getRenderMode());
		fprintf(file, "grid=%d\n",  glw->getFlag(F_GRID));
		fprintf(file, "pause=%d\n", d_startPaused);
		fprintf(file, "light=%d\n", glw->getFlag(F_LIGHT));

		//recent models
		fprintf(file, "[model]\n");
		for (int i = 0; i < NUM_RECENT_FILES; i++)
		{
			if (d_recentModelFiles[i][0])
				fprintf(file, "%s\n", d_recentModelFiles[i]);
		}

		//recent pak files
		fprintf(file, "[pak]\n");
		for (int i = 0; i < NUM_RECENT_FILES; i++)
		{
			if (d_recentPakFiles[i][0])
				fprintf(file, "%s\n", d_recentPakFiles[i]);
		}


		fclose (file);
	}
}

enum {
	CMD_NEW = 1,
	// other command IDs
};

MDXViewer::MDXViewer ()
: mxWindow (0, 0, 0, 0, 0, "Kingpin Model Viewer " KP_BUILD_VERSION "(beta)", mxWindow::Normal) //hypov8 version
{
	// create menu stuff
	UI_mb = new mxMenuBar (this);
	mxMenu *menuModel   = new mxMenu ();
	mxMenu *menuPack    = new mxMenu (); //hypov8
	mxMenu *menuSkin    = new mxMenu ();
	mxMenu *menuMake    = new mxMenu (); //hypov8
	mxMenu *menuOptions = new mxMenu ();
	mxMenu *menuView    = new mxMenu ();
	mxMenu *menuHelp    = new mxMenu ();

	UI_mb->addMenu ("Model",    menuModel);
	UI_mb->addMenu ("Pak ",     menuPack); //hypov8
	UI_mb->addMenu ("Textures", menuSkin);
	UI_mb->addMenu ("Make",     menuMake); //hypov8
	UI_mb->addMenu ("Options",  menuOptions);
	UI_mb->addMenu ("Help",     menuHelp);


	mxMenu *menuRecentModels = new mxMenu ();
	mxMenu *menuRecentPakFiles = new mxMenu ();
	for (int i = 0; i < NUM_RECENT_FILES; i++)	{
		menuRecentModels->add ("(empty)", IDC_MODEL_RECENTMODELS1 + i);
		menuRecentPakFiles->add ("(empty)", IDC_MODEL_RECENTPAKFILES1+i);
	}

	/////////////
	//menu model
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


	////////////
	//menu pack
	menuPack->add ("Open PAK file...", IDC_MODEL_OPENPAKFILE);
	menuPack->add ("Close PAK file", IDC_MODEL_CLOSEPAKFILE);
	menuPack->addSeparator ();
	menuPack->addMenu ("Recent PAK files", menuRecentPakFiles);

	////////////////
	//menu textures
	menuSkin->add ("Load Model 1 Skin...", IDC_SKIN_MODELSKIN1);
	menuSkin->add ("Load Model 2 Skin...", IDC_SKIN_MODELSKIN2);
	menuSkin->add ("Load Model 3 Skin...", IDC_SKIN_MODELSKIN3);
	menuSkin->add ("Load Model 4 Skin...", IDC_SKIN_MODELSKIN4);
	menuSkin->add ("Load Model 5 Skin...", IDC_SKIN_MODELSKIN5);
	menuSkin->add ("Load Model 6 Skin...", IDC_SKIN_MODELSKIN6);
	menuSkin->addSeparator ();
	menuSkin->add("Reload all skins\t(F5)",      IDC_SKIN_RELOAD);
	menuSkin->addSeparator();
	menuSkin->add ("Load Background Texture...", IDC_SKIN_BACKGROUND);
	menuSkin->add ("Load Water Texture...",      IDC_SKIN_WATER);

//------------------------------------\\
	//hotkeys
	mxAccelerator accel;
	accel.add(VK_F5, FVIRTKEY, IDC_KEYS_F5);
	accel.add('P',   FVIRTKEY, IDC_KEYS_PAUSE);
	accel.add('1',   FVIRTKEY, IDC_KEYS_MODE1);
	accel.add('2',   FVIRTKEY, IDC_KEYS_MODE2);
	accel.add('3',   FVIRTKEY, IDC_KEYS_MODE3);
	accel.add('4',   FVIRTKEY, IDC_KEYS_MODE4);

	accel.loadTable();

//------------------------------------\\


	////////////
	//menu make
#ifdef WIN32
	menuMake->add ("Screenshot...", IDC_MAKE_SCREENSHOT);	//hypov8 slow
	//menuSkin->add ("Make AVI File...", IDC_SKIN_AVI);				//hypov8 buggy
	menuMake->addSeparator();
#endif
	menuMake->add("VU Map. true size...", IDC_MAKE_UV_RES);
	menuMake->add("VU Map. 1k(square)...",  IDC_MAKE_UV_1K);
	menuMake->add("VU Map. 2k(keep A/R)...",  IDC_MAKE_UV_2K);

	///////////////
	//menu options
	menuOptions->add ("Background Color...", IDC_OPTIONS_BGCOLOR);
	menuOptions->add ("Wireframe Color...",  IDC_OPTIONS_WFCOLOR);
	menuOptions->add ("Face Color...",       IDC_OPTIONS_FACECOLOR);
	menuOptions->add ("Light Color...",      IDC_OPTIONS_LIGHTCOLOR);
	menuOptions->add ("Grid Color...",       IDC_OPTIONS_GRIDCOLOR);
	menuOptions->add ("Developer Color1...",  IDC_OPTIONS_DEBUGCOLOR1);
	menuOptions->add ("Developer Color2...",  IDC_OPTIONS_DEBUGCOLOR2);
	menuOptions->addSeparator ();
	menuOptions->add ("Generate Light Normals.", IDC_OPTIONS_GEN_NORMALS);
	menuOptions->addSeparator();
	menuOptions->add("Load invalid models?", IDC_OPTIONS_LOADINVALID); //DEBUG
	menuOptions->addSeparator();
	menuOptions->add("Load Animated Models Paused?", IDC_OPTIONS_LOAD_PAUSED); //DEBUG

#ifdef WIN32
	menuHelp->add ("Goto Homepage", IDC_HELP_GOTOHOMEPAGE);
	menuHelp->addSeparator ();
#endif
	menuHelp->add ("About", IDC_HELP_ABOUT);

	///////////////////////////////////////////
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
	cRenderMode = new mxChoice (wView, 3, 5, 125, 22, IDC_RENDERMODE); //view mode
	cRenderMode->add ("Wireframe");
	cRenderMode->add ("Flat shaded");
	cRenderMode->add ("Smooth Shaded");
	cRenderMode->add ("Textured");

	cRenderMode->select (0);
	cbWireFrame = new mxCheckBox(wView, 55, 30, 76, 22, "Wire #", IDC_WIREFRAME);

	//brightness slider
	mxSlider *slBrightness = new mxSlider (wView, 127, 5, 70, 22, IDC_BRIGHTNESS); //slider (d_bias)
	mxLabel *lblBrightness = new mxLabel (wView, 137, 28, 70, 22, "Brightness"); //label
	slBrightness->setRange (0, BRIGHTNESS_LEVELS_MAX);
	slBrightness->setValue (BRIGHTNESS_LEVELS_STD);

	cbWater =                 new mxCheckBox (wView, 3, 30, 50, 22, "Water", IDC_WATER);
	cbLight =                 new mxCheckBox (wView, 3, 55, 50, 22, "Light", IDC_LIGHT);
	mxCheckBox *cbShininess = new mxCheckBox (wView, 55, 55, 50, 22, "Shine", IDC_SHININESS);
	cbBackground =            new mxCheckBox (wView, 110, 55, 78, 22, "Background", IDC_BACKGROUND);
	cbGrid =                  new mxCheckBox (wView, 3, 80, 50, 22, "Grid", IDC_GRID);
	mxCheckBox *cbHitBox =    new mxCheckBox (wView, 55, 80, 50, 22, "HitBox", IDC_HITBOX);
	mxCheckBox *cbVNorms =    new mxCheckBox (wView, 110, 80, 90, 22, "Vertex Normals", IDC_VERTNORMS);
		
	cbLight->setChecked (false);
	cbShininess->setChecked (false);
	cbVNorms->setChecked (false);
	cbGrid->setChecked (true);
	cbHitBox->setChecked (false);

	mxToolTip::add (cbHitBox, "MDX hitbox. Player models use these.");
	mxToolTip::add (cbVNorms, "View vertex normal"); //
	mxToolTip::add (slBrightness, "Texture Brightness"); //



	////////////////////////////////////
	// Create widgets for the Animation
	////////////////////////////////////
	int ofT = 205; //ani alignment offset

	cAnim = new mxChoice (wView, 5+ofT, 5, 170, 22, IDC_ANIMATION_SETS); //dropdown
	cbInterp = new mxCheckBox (wView, 5 + ofT, 30, 70, 22, "Interpolate", IDC_INTERPOLATE);
	cbInterp->setChecked (true);
	lAnimSpeed = new mxLabel(wView, 5 + ofT, 59, 75, 14, "Speed: 10fps");
	mxSlider *slPitch = new mxSlider (wView, 5 + ofT, 77, 98, 22, IDC_PITCH); //slider
	mxToolTip::add (slPitch, "Frame Animation Speed (Pitch)");
	slPitch->setRange (1, 30); //changed to FPS
	slPitch->setValue (10); //default kp time. 10fps
	
	

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
	mxToolTip::add(leVertex, "Current Vertex/Face");
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
	//////////////////////////////////////
	// create the pakviewer window
	pakViewer = new PAKViewer (this);
	pakViewer->setVisible(0);

	initRecentFiles ();
	loadConfigFile ();
	updateRecentMenu(IDC_MODEL_RECENTMODELS1);
	updateRecentMenu(IDC_MODEL_RECENTPAKFILES1);

	//setBounds (20, 20, 690, 550);
	setBounds (20, 20, 520, 550); //hidden pak
	setVisible (true);

	reset_modelData();
	reset_viewData();

	//set startup option for render method
	cRenderMode->select(d_startMode);
	setRenderMode(cRenderMode->getSelectedIndex());

	//sync CB and opengl values
	glw->setFlag(F_WATER,       cbWater->isChecked());
	glw->setFlag(F_LIGHT,       cbLight->isChecked());
	glw->setFlag(F_SHININESS,   cbShininess->isChecked());
	glw->setFlag(F_INTERPOLATE, cbInterp->isChecked());
	glw->setFlag(F_PAUSE,       d_startPaused); //hypov8
	glw->setFlag(F_BACKGROUND,  cbBackground->isChecked());
	glw->setFlag(F_VNORMS,      cbVNorms->isChecked());
	glw->setFlag(F_GRID,        cbGrid->isChecked());
	glw->setFlag(F_HITBOX,      cbHitBox->isChecked());
	glw->setFlag(F_WIREFRAME,   cbWireFrame->isChecked());
	glw->setFlag(F_WIRE_OGL1,    false);
	glw->setFlag(F_WIRE_OGL2,    false);


	//hypov8 file->open with. load
	int pos = 0;
	for (int i = 0; i < MAX_MODELS; i++)
	{
		if (startupModelsNames[i][0] != NULL)
		{
			if (!loadModel_(startupModelsNames[i], pos, true))
			{
				char str[256];
				sprintf_s(str, sizeof(str), "Error reading model: %s", startupModelsNames[i]);
				mxMessageBox(this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
			}
			else
				pos++;
		}
	}
}



MDXViewer::~MDXViewer ()
{
	saveConfigFile ();
}


void
MDXViewer::reset_modelData(void)
{
	for (int i = 0; i < MAX_MODELS; i++)
	{
		//texture
		glw->d_textureValid[i] = false;
		glw->d_modelTexNames[i][0] = '\0'; //hypov8 textures
		
		//model
		mdx_freeModel(glw->d_models[i]);
		glw->d_models[i] = NULL;
		glw->d_textureValid[i] = false;
		glw->d_modelFileNames[i][0] = '\0'; //hypov8 models
		glw->d_modelTexNames[i][0] = '\0'; //hypov8 textures
	}
}
void MDXViewer::reset_viewData()
{
	//reset model to animate
	leVertex->setLabel("-1");
	leFrame->setLabel("0");

	setPauseMode(0);

	bDecFrame->setEnabled(0);
	leFrame->setEnabled(0);
	bIncFrame->setEnabled(0);
	bSetFrame->setEnabled(0);
	cAnim->setEnabled(1);
	cbInterp->setEnabled(1);
	cAnim->removeAll();

	//glw->setFlag(F_INTERPOLATE, cbInterp->isChecked());
	glw->setFrameInfo(0, 0);
	glw->redraw();
	setDisplayModelInfo(); // 0, TEXTURE_MODEL_0);
	glw->d_vertexIndex = -1; //HYPOVERTEX
	glw->d_vertexUseFace = 0; //HYPOVERTEX
}

void MDXViewer::setFrameDisplay(int frame)
{
	char str[32];

	sprintf_s(str, sizeof(str), "%d", frame);
	leFrame->setLabel(str);
}

void
MDXViewer::setPauseMode(int frames)
{
	if (d_startPaused || frames == 1)
	{
		glw->setFlag(F_PAUSE, false); //toggled below
		setStatePaused();
	}
	else
	{
		glw->setFlag(F_PAUSE, true); //toggled below
		setStatePaused();
	}

	if (frames == 1)
	{
		bPause->setEnabled(0);
		bPause->setLabel("No Frames");
		cAnim->setEnabled(0);
		cbInterp->setEnabled(0);
	}
	else
		bPause->setEnabled(1);
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
MDXViewer::importPlayerModelFolder(const char *ptr, int mode, int mIndex, bool getSkin)
{
	if (ptr)
	{
		char path_new[MAX_PATH_LEN], path_tmp[300];

		if (mode == IDC_MODEL_LOADMODEL)
		{
			reset_modelData(); //hypov8
			reset_viewData();
			mIndex = 0;
		}

		if (!loadModel_(ptr, mIndex, getSkin))
		{
			sprintf_s(path_tmp, sizeof(path_tmp), "Error reading model: %s", ptr);
			mxMessageBox(this, path_tmp, "ERROR", MX_MB_OK | MX_MB_ERROR);
			return 0; // break;
		}

		// update recent files list
		sprintf_s(path_new, sizeof(path_new), "%s", ptr);
		updateRecentPaths(IDC_MODEL_RECENTMODELS1, path_new);
		updateRecentMenu(IDC_MODEL_RECENTMODELS1);
		

		//initRecentFiles();
		return 1;
	}
	//else cancled?
	return 0;
}

#define KEY_DELAY 150
int
MDXViewer::handleEvent(mxEvent *event)
{
	switch (event->event)
	{
		case mxEvent::Action:
		{
			switch (event->action)
			{
				case IDC_KEYS_F5:
				{
					static int lastTime = 0;
					int curTime = mx::getTickCount();
					if (lastTime < curTime)
					{
						glw->reloadAllTextures();
					}
					lastTime = curTime + KEY_DELAY;
				}
				break;

				case IDC_KEYS_PAUSE:
				{
					static int lastTime = 0;
					int curTime = mx::getTickCount();
					if (lastTime < curTime)
					{
						setStatePaused(); //pause
					}
					lastTime = curTime + KEY_DELAY;
				}
				break;

				case IDC_KEYS_MODE1:
				case IDC_KEYS_MODE2:
				case IDC_KEYS_MODE3:
				case IDC_KEYS_MODE4:
				{
					static int lastTime = 0;
					int curTime = mx::getTickCount();
					if (lastTime < curTime)
					{
						int val = event->action - IDC_KEYS_MODE1;
						//cRenderMode->select(val);
						setRenderMode(val);
						glw->redraw();
					}
					lastTime = curTime + KEY_DELAY;
				}
				break;

				case IDC_MODEL_LOADMODEL:
				case IDC_MODEL_MERGEMODEL:
				{
					int mIndex = glw->getFreeModelIndex();
					if (mIndex < MAX_MODELS || event->action == IDC_MODEL_LOADMODEL)
					{
						const char *ptr = mxGetOpenFileName(this, 0, fileFilterString(FILE_TYPE_MDX, 1)); //.mdx
						if (ptr)
						{
							int tmp = importPlayerModelFolder(ptr, event->action, mIndex, true);
#if 0
							int i;
							char path[256];

							if (event->action == IDC_MODEL_LOADMODEL)
							{
								reset_modelData(); //hypov8
								reset_viewData();
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
					const char *ptr = mxGetFolderPath(this, mx_getcwd());
					if (ptr)
					{
						int idx = 0;
						static char *mdlName[4] = { "head.mdx",     "body.mdx",     "legs.mdx",     "w_tommygun.mdx" };
						static char *skiName[4] = { "head_001.tga", "body_001.tga", "legs_001.tga", "0" };
						char skins[3][256];

						reset_modelData(); //hypov8
						reset_viewData();
						mdx_read_playerINI(ptr, skins[0], skins[1], skins[2]);

						for (int i = 0; i < 4; i++)
						{
							char ptrFile[MAX_PATH];
							bool getSkin = (i == 3); //only get wep skin
							//load model
							sprintf_s(ptrFile, sizeof(ptrFile), "%s\\%s", ptr, mdlName[i]);
							if (importPlayerModelFolder((const char *)ptrFile, event->action, idx, getSkin))
							{
								//load player skins. head/body/legs
								if (i < 3)
								{
									if (skins[i][0])
										sprintf_s(ptrFile, sizeof(ptrFile), "%s\\%s", skins[i], skiName[i]);
									else
										sprintf_s(ptrFile, sizeof(ptrFile), "%s\\%s", ptr, skiName[i]);
									glw->loadTexture(ptrFile, idx);
								}
								idx += 1;
							}
						}
						mx_setcwd(ptr);
						setDisplayModelInfo();
						setRenderMode(3); //allow viewing skins
						centerModel(0, 0);//reset view to whole model
						glw->redraw();
					}
				}
				break;

				case IDC_MODEL_UNLOADMODEL:
				case IDC_MODEL_UNLOADWEAPON:
					reset_modelData(); //hypov8
					reset_viewData();
					break;

				case IDC_MODEL_OPENPAKFILE:
				//case IDC_MODEL_OPENPAKFILE2:
				{
					const char *ptr = mxGetOpenFileName(this, 0, fileFilterString(FILE_TYPE_PAK, 0)); //*.pak
					if (ptr)
					{
						pakViewer->setLoadEntirePAK(event->action == IDC_MODEL_OPENPAKFILE);
						if (!pakViewer->openPAKFile(ptr))
						{
							mxMessageBox(this, "Error loading PAK file", "ERROR", MX_MB_OK | MX_MB_ERROR);
							break;
						}

						// update recent pak file list
						char path_new[MAX_PATH_LEN];
						sprintf_s(path_new, sizeof(path_new), "%s", ptr);
						updateRecentPaths(IDC_MODEL_RECENTPAKFILES1, path_new);
						updateRecentMenu(IDC_MODEL_RECENTPAKFILES1);

						redraw();
					}
				}
				break;

				case IDC_MODEL_CLOSEPAKFILE:
				{
					pakViewer->closePAKFile();
					pakViewer->setVisible(0);
					redraw();
				}
				break;

				case IDC_MODEL_RECENTMODELS1:
				case IDC_MODEL_RECENTMODELS2:
				case IDC_MODEL_RECENTMODELS3:
				case IDC_MODEL_RECENTMODELS4:
				case IDC_MODEL_RECENTMODELS5:
				case IDC_MODEL_RECENTMODELS6:
				case IDC_MODEL_RECENTMODELS7:
				case IDC_MODEL_RECENTMODELS8:
				{
					int i = event->action - IDC_MODEL_RECENTMODELS1;
					char *ptr = &d_recentModelFiles[i][0];

					reset_modelData();
					reset_viewData();

					if (!loadModel_(ptr, 0, true))
					{
						char str[300];
						sprintf_s(str, sizeof(str), "Error reading model: %s", ptr);
						mxMessageBox(this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
						//d_recentModelFiles[i][0] = 0;
						break;
					}

					// update recent model list
					char path_new[MAX_PATH_LEN];
					sprintf_s(path_new, sizeof(path_new), "%s", ptr);
					updateRecentPaths(event->action, path_new);
					updateRecentMenu(IDC_MODEL_RECENTMODELS1);
				}
				break;

				case IDC_MODEL_RECENTPAKFILES1:
				case IDC_MODEL_RECENTPAKFILES2:
				case IDC_MODEL_RECENTPAKFILES3:
				case IDC_MODEL_RECENTPAKFILES4:
				case IDC_MODEL_RECENTPAKFILES5:
				case IDC_MODEL_RECENTPAKFILES6:
				case IDC_MODEL_RECENTPAKFILES7:
				case IDC_MODEL_RECENTPAKFILES8:
				{
					int i = event->action - IDC_MODEL_RECENTPAKFILES1;
					pakViewer->setLoadEntirePAK(true);
					if (!pakViewer->openPAKFile(d_recentPakFiles[i]))
					{
						char str[300];
						sprintf_s(str, sizeof(str), "Error reading pak: %s", d_recentPakFiles[i]);
						mxMessageBox(this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
						d_recentPakFiles[i][0] = 0;
						break;
					}

					char path_new[MAX_PATH_LEN];
					sprintf_s(path_new, sizeof(path_new), "%s", d_recentPakFiles[i]);
					updateRecentPaths(event->action, path_new);
					updateRecentMenu(IDC_MODEL_RECENTPAKFILES1);

					redraw();
				}
				break;

				case IDC_MODEL_SAVE:
				{
					//this convertes the model between mdx/md2
					mdx_model_t *model = glw->getModel(0);
					if (model)
					{
						int fileType[2] = { FILE_TYPE_MD2, FILE_TYPE_MDX };
						char *ext[2] = { ".md2", ".mdx" };
						const char *file = mxGetSaveFileName(this, 0, fileFilterString(fileType[model->isMD2], 0));
						if (file)
						{
							//add file extension if missing
							if (mx_strncasecmp(mx_getextension(file), ext[model->isMD2], 4))
								strcat_s((char *)file, _MAX_PATH, ext[model->isMD2]);

							if (!SaveAsMD_(file, model))
							{
								char str[300];
								sprintf_s(str, sizeof(str), "Error saving model: %s", file);
								mxMessageBox(this, str, "ERROR", MX_MB_OK | MX_MB_ERROR);
							}
						}
					}
				}
				break;

				case IDC_MODEL_EXIT:
					mx::setIdleWindow(0);
					mx::quit();
					break;

				case IDC_SKIN_MODELSKIN1:
				case IDC_SKIN_MODELSKIN2:
				case IDC_SKIN_MODELSKIN3:
				case IDC_SKIN_MODELSKIN4:
				case IDC_SKIN_MODELSKIN5:
				case IDC_SKIN_MODELSKIN6:
				{
					const char *ptr = mxGetOpenFileName(this, 0, fileFilterString(FILE_TYPE_TGA, 1)); //*.tga then *.pcx
					if (ptr)
					{
						glw->loadTexture(ptr, event->action - IDC_SKIN_MODELSKIN1); //TEXTURE_MODEL_0
						setDisplayModelInfo();
						setRenderMode(3); //allow viewing skins
						glw->redraw();
					}
				}
				break;

				case IDC_SKIN_RELOAD:
				{
					glw->reloadAllTextures();
				}
				break;

				case IDC_SKIN_BACKGROUND:
				case IDC_SKIN_WATER:
				{
					const char *ptr = mxGetOpenFileName(this, 0, fileFilterString(FILE_TYPE_TGA, 1));//*.tga *.pcx
					if (!ptr)
						break;

					if (glw->loadTexture(ptr, event->action == IDC_SKIN_BACKGROUND ? TEXTURE_BACKGROUND : TEXTURE_WATER))
					{
						if (event->action == IDC_SKIN_BACKGROUND)
						{
							cbBackground->setChecked(true);
							glw->setFlag(F_BACKGROUND, true);
						}
						else
						{
							cbWater->setChecked(true);
							glw->setFlag(F_WATER, true);
						}

						//setRenderMode (3);
						glw->redraw();
					}
					else
						mxMessageBox(this, "Error loading texture.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);
				}
				break;

#ifdef WIN32
				case IDC_MAKE_SCREENSHOT:
				{
					const char *ptr = mxGetSaveFileName(this, 0, fileFilterString(FILE_TYPE_TGA, 0)); //*.tga
					if (ptr)
					{
						if (mx_strncasecmp(mx_getextension(ptr), ".tga", 4))
							strcat_s((char *)ptr, _MAX_PATH, ".tga");
						makeScreenShot(ptr);
					}
				}
				break;

				case IDC_MAKE_AVI:
				{
					int index = cAnim->getSelectedIndex();
					if (index >= 0)
					{
						// set the animation
						int start, end, test;
						//FILE *te;
						initAVIAnimation(glw->getModel(0), index - 1, &start, &end);
						if (start == 0 && end == 0)
						{
							start = 1;
							end = glw->getModel(0)->header.numFrames;
						}
						//	te = fopen("test.txt", "w+");
						//	fprintf(te, "%d start, %d end\n", start, end);
						//	fclose(te);
						if ((test = MakeAVI(start, end)) == 0)
							mxMessageBox(this, "Error creating AVI File.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);

						glw->redraw();
					}
				}
				break;
#endif

				case IDC_MAKE_UV_RES:
				case IDC_MAKE_UV_1K:
				case IDC_MAKE_UV_2K:
				{
					const char *ptr = mxGetSaveFileName(this, 0, fileFilterString(FILE_TYPE_TGA, 0)); //*.tga
					if (ptr) //D:\Kingpin\main\models\props\boat
					{
						if (mx_strncasecmp(mx_getextension(ptr), ".tga", 4))
							strcat_s((char *)ptr, _MAX_PATH, ".tga");
						makeUVMapImage(ptr, event->action);
					}
				}
				break;


				case IDC_OPTIONS_BGCOLOR:
				case IDC_OPTIONS_FACECOLOR:
				case IDC_OPTIONS_WFCOLOR:
				case IDC_OPTIONS_LIGHTCOLOR:
				case IDC_OPTIONS_DEBUGCOLOR1:
				case IDC_OPTIONS_DEBUGCOLOR2:
				case IDC_OPTIONS_GRIDCOLOR:
				{
					float r, g, b;
					int ir, ig, ib;

					if (event->action == IDC_OPTIONS_BGCOLOR)
						glw->getBGColor(&r, &g, &b);
					else if (event->action == IDC_OPTIONS_FACECOLOR)
						glw->getFaceColor(&r, &g, &b);
					else if (event->action == IDC_OPTIONS_WFCOLOR)
						glw->getWFColor(&r, &g, &b);
					else if (event->action == IDC_OPTIONS_LIGHTCOLOR)
						glw->getLightColor(&r, &g, &b);
					else if (event->action == IDC_OPTIONS_DEBUGCOLOR1)
						glw->getDebugColor1(&r, &g, &b);
					else if (event->action == IDC_OPTIONS_DEBUGCOLOR2)
						glw->getDebugColor2(&r, &g, &b);
					else if (event->action == IDC_OPTIONS_GRIDCOLOR)
						glw->getGridColor(&r, &g, &b);

					ir = (int)(r * 255.0f);
					ig = (int)(g * 255.0f);
					ib = (int)(b * 255.0f);
					if (mxChooseColor(this, &ir, &ig, &ib))
					{
						if (event->action == IDC_OPTIONS_BGCOLOR)
						{
							glw->setBGColor((float)ir / 255.0f, (float)ig / 255.0f, (float)ib / 255.0f);
							glw->loadTexture(0, TEXTURE_BACKGROUND);
						}
						else if (event->action == IDC_OPTIONS_FACECOLOR)
							glw->setFaceColor((float)ir / 255.0f, (float)ig / 255.0f, (float)ib / 255.0f);
						else if (event->action == IDC_OPTIONS_WFCOLOR)
							glw->setWFColor((float)ir / 255.0f, (float)ig / 255.0f, (float)ib / 255.0f);
						else if (event->action == IDC_OPTIONS_LIGHTCOLOR)
							glw->setLightColor((float)ir / 255.0f, (float)ig / 255.0f, (float)ib / 255.0f);
						else if (event->action == IDC_OPTIONS_DEBUGCOLOR1)
							glw->setDebugColor1((float)ir / 255.0f, (float)ig / 255.0f, (float)ib / 255.0f);
						else if (event->action == IDC_OPTIONS_DEBUGCOLOR2)
							glw->setDebugColor2((float)ir / 255.0f, (float)ig / 255.0f, (float)ib / 255.0f);
						else if (event->action == IDC_OPTIONS_GRIDCOLOR)
							glw->setGridColor((float)ir / 255.0f, (float)ig / 255.0f, (float)ib / 255.0f);
						glw->redraw();
					}
				}
				break;

				case IDC_OPTIONS_CENTERMODEL1:
					centerModel(glw->getCurrFrame(), 0);
					glw->redraw();
					break;

				case IDC_OPTIONS_CENTERMODEL2:
					centerModel(glw->getCurrFrame(), 1);
					glw->redraw();
					break;

				case IDC_OPTIONS_GEN_NORMALS:
				{
					int i;
					for (i = 0; i < MAX_MODELS; i++)
					{
						mdx_generateLightNormals(glw->getModel(i));
					}
					glw->redraw();
				}
				break;

				case IDC_OPTIONS_LOADINVALID:
				{
					int val = glw->getLoadInvalid();
					if (val)
						glw->setLoadInvalid(false); //hypov8
					else
					{

						val = !mxMessageBox(this,
							"Use with caution.\n"
							"Load models with an invalid header?", "Caution:", MX_MB_YESNO | MX_MB_QUESTION);
						glw->setLoadInvalid(val); //hypov8
					}
				}
				break;

				case IDC_OPTIONS_LOAD_PAUSED:
				{
					d_startPaused = !d_startPaused;

					if (d_startPaused)
					{
						mxMessageBox(this,
							"Load animated models and pause animations.", "Pause Mode:", MX_MB_OK);
					}
					else
					{
						mxMessageBox(this,
							"load animated models and play animations.", "Play Mode:", MX_MB_OK);
					}
				}
				break;

#ifdef WIN32
				case IDC_HELP_GOTOHOMEPAGE:
					ShellExecute(0, "open", "http://Kingpin.info", 0, 0, SW_SHOW);
					break;
#endif

				case IDC_HELP_ABOUT:
					mxMessageBox(this,
						"Info:"
						"\tLoads mdx and md2 models. Support for 6 models.\n"
						"\tAbility to export .mdx models to .md2.\n"
						"\tAbility to export .md2 models to .mdx.\n"
						"\tPak viewer that loads models and textures.\n"
						"\n"
						"Keys:"
						"\tMouse-Left:      \tdrag to rotate.\n"
						"\tMouse-Right:     \tdrag to zoom.\n"
						"\tMouse-Middle:    \tdrag to pan x-y (slow).\n"
						"\tMouse-Left+Shift:\tdrag to pan x-y (fast).\n"
						"\t1-4:             \ttexture modes.\n"
						"\tF5:              \trefresh all textures.\n"
						"\tP:               \tpause.\n"
						"\n"

						"commandline:\n"
						"\t-1:  \tflat shaded.\n"
						"\t-2:  \tsmooth shaded.\n"
						"\t-3:  \ttextured.\n"
						"\t-p:  \tstart paused.\n"

						"\n"
						"bugs:"
						"\tNeed to left click first in PAK viewer.\n"
						"\n"
						"Thanks:"
						"\tTiCaL: Creator of MDX Viewer v1.1.\n"
						"\tMete Cirigan: for MD2 Viewer code.\n"
						"\tChris Cookson: for helping TiCaL.\n"
						"\n"
						"Build:\t" __DATE__ ".\n"
						"Email:\t" "hypov8@kingpin.info" "\n"
						"Web:\t" "http://Kingpin.info/",
						"Kingpin Model Viewer " KP_BUILD_VERSION " by Hypov8", //title //hypov8 version
						MX_MB_OK | MX_MB_INFORMATION
					);
					break;

					// 
					// widget actions
					//
					//

					//
					// Model Panel
					//

				case IDC_RENDERMODE:
					setRenderMode(cRenderMode->getSelectedIndex());
					glw->redraw();
					break;

				case IDC_WIREFRAME:
				{	//4 state check box
					bool modeWire = glw->getFlag(F_WIREFRAME);
					bool mode_OGL1 = glw->getFlag(F_WIRE_OGL1);
					bool mode_OGL2 = glw->getFlag(F_WIRE_OGL2);

					//toggle modes
					if (modeWire)
					{
						glw->setFlag(F_WIREFRAME, false);
						glw->setFlag(F_WIRE_OGL1, true); //enable
						glw->setFlag(F_WIRE_OGL2, false);
						cbWireFrame->setChecked(true);
						cbWireFrame->setLabel("Wire #2");
					}
					else if (mode_OGL1)
					{
						glw->setFlag(F_WIREFRAME, false);
						glw->setFlag(F_WIRE_OGL1, false);
						glw->setFlag(F_WIRE_OGL2, true); //enable
						cbWireFrame->setChecked(true);
						cbWireFrame->setLabel("Wire #3");
					}
					else if (mode_OGL2)
					{
						glw->setFlag(F_WIREFRAME, false);
						glw->setFlag(F_WIRE_OGL1, false);
						glw->setFlag(F_WIRE_OGL2, false);
						cbWireFrame->setChecked(false);
						cbWireFrame->setLabel("Wire #-");
					}
					else
					{
						glw->setFlag(F_WIREFRAME, true); //enable
						glw->setFlag(F_WIRE_OGL1, false);
						glw->setFlag(F_WIRE_OGL2, false);
						cbWireFrame->setChecked(true);
						cbWireFrame->setLabel("Wire #1");
					}
					glw->redraw();
				}
				break;

				case IDC_WATER:
					glw->setFlag(F_WATER, cbWater->isChecked());
					glw->redraw();
					break;

				case IDC_LIGHT:
					glw->setFlag(F_LIGHT, cbLight->isChecked());
					glw->redraw();
					break;

				case IDC_BRIGHTNESS:
					glw->setBrightness(((mxSlider *)event->widget)->getValue());
					break;

				case IDC_SHININESS:
					glw->setFlag(F_SHININESS, ((mxCheckBox *)event->widget)->isChecked());
					glw->redraw();
					break;

				case IDC_BACKGROUND:
					glw->setFlag(F_BACKGROUND, ((mxCheckBox *)event->widget)->isChecked());
					glw->redraw();
					break;

				case IDC_VERTNORMS: //hypov8
					glw->setFlag(F_VNORMS, ((mxCheckBox *)event->widget)->isChecked());
					glw->redraw();
					break;
				case IDC_GRID: //hypov8
					glw->setFlag(F_GRID, ((mxCheckBox *)event->widget)->isChecked());
					glw->redraw();
					break;
				case IDC_HITBOX: //hypov8
					glw->setFlag(F_HITBOX, ((mxCheckBox *)event->widget)->isChecked());
					glw->redraw();
					break;

					/*case IDC_TEXTURELIMIT:
					{
						int tl[3] = { 512, 256, 128 };
						int index = ((mxChoice *) event->widget)->getSelectedIndex ();
						if (index >= 0)
							glw->setTextureLimit (tl[index]);
					}
					break;*/

					//
					// Animation Panel
					//
				case IDC_ANIMATION_SETS:
				{
					int currAnimIndex = cAnim->getSelectedIndex();
					if (currAnimIndex >= 0)
					{
						int startFrame = 0, endFrame;
						mdx_model_t *model = glw->getModel(0);
						// set the animation
						//////initAnimation (glw->getModel (0), index - 1);

						if (!model)
						{
							glw->setFrameInfo(0, 0);
						}
						else
						{
							if (currAnimIndex <= 0)
								glw->setFrameInfo(startFrame, model->header.numFrames - 1);
							else
							{
								mdx_getAnimationFrames(model, currAnimIndex - 1, &startFrame, &endFrame);
								glw->setFrameInfo(startFrame, endFrame);
							}

							// if we pause, update current frame in leFrame
							if (glw->getFlag(F_PAUSE))
							{
								char str[32];
								//int frame = glw->getCurrFrame ();
								sprintf_s(str, sizeof(str), "%d", startFrame);
								leFrame->setLabel(str);
								glw->setFrameInfo(startFrame, startFrame);
							}
						}
						glw->redraw();
					}
				}
				break;

				case IDC_INTERPOLATE:
					glw->setFlag(F_INTERPOLATE, ((mxCheckBox *)event->widget)->isChecked());
					glw->redraw();
					break;

				case IDC_PITCH: //slPitch
				{
					char str[32];
					float speed = (float)((mxSlider *)event->widget)->getValue();
					glw->setPitch(/*200 -*/speed); //hypov8 UI: invert. d_pitch
					sprintf_s(str, sizeof(str), "Speed: %ifps", (int)(speed));
					lAnimSpeed->setLabel(str);
				}
				break;

				case IDC_PAUSE: // Pause/Play
				{
					bool pause = !glw->getFlag(F_PAUSE); //toggle (button pressed)

					glw->setFlag(F_PAUSE, pause);
					bDecFrame->setEnabled(pause);
					leFrame->setEnabled(pause);
					bIncFrame->setEnabled(pause);
					bSetFrame->setEnabled(pause);

					if (pause)
					{
						char str[32];
						sprintf_s(str, sizeof(str), "%d", glw->getCurrFrame());
						leFrame->setLabel(str);
						bPause->setLabel("Play");
					}
					else //play
					{
						int frame = glw->d_currFrame;
						const char *ptr = leFrame->getLabel();
						if (ptr)
							frame = atoi(ptr);

						mdx_model_t *model = glw->getModel(0);
						if (model)
						{
							int curAnimIndex = cAnim->getSelectedIndex();
							int startFrame, endFrame;
							if (curAnimIndex <= 0) //=-1
							{
								startFrame = 0;
								endFrame = model->header.numFrames - 1;
							}
							else
							{
								mdx_getAnimationFrames(model, curAnimIndex - 1, &startFrame, &endFrame);
							}
							//curent frame within sequence?
							if (frame <= endFrame && frame >= startFrame)
							{
								glw->d_startFrame = startFrame;
								glw->d_endFrame = endFrame;

								//resuming animation. force next frame
								glw->d_currFrame2 = glw->d_currFrame + 1;
								if (glw->d_currFrame2 > endFrame)
									glw->d_currFrame2 = startFrame;
							}
							else
							{
								glw->setFrameInfo(startFrame, endFrame);
							}
							//continue lerp
							glw->setResumePaused();
						}
						else
							glw->setFrameInfo(0, 0);

						bPause->setLabel("Pause");
						//glw->redraw();
					}
				}
				break;

				case IDC_DECFRAME:
				case IDC_BTN_SET_FRAME:
				case IDC_INCFRAME:
				{
					const char *ptr = leFrame->getLabel();
					int frame = glw->getCurrFrame();
					if (ptr)
						frame = atoi(ptr);

					if (event->action == IDC_INCFRAME)
						frame += 1;
					else if (event->action == IDC_DECFRAME)
						frame -= 1;

					glw->setFrameInfo(frame, frame);

					char str[32];
					sprintf_s(str, sizeof(str), "%d", glw->getCurrFrame());
					leFrame->setLabel(str);
					glw->redraw();
				}
				break;

				case IDC_1ST_PERSON:
					//hypov8 set viewport to 1st person
					glw->d_rotX = 0;
					glw->d_rotY = 180;
					glw->d_transX = 0;
					glw->d_transY = 0;
					glw->d_transZ = 3; //move fov back slightly
					glw->redraw();
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

					sprintf_s(str, sizeof(str), "%d", cutVertID);
					leVertex->setLabel(str);

					if (isFace)
						glw->d_vertexUseFace = 1;
					else
						glw->d_vertexUseFace = 0;

					glw->d_vertexIndex = cutVertID;
					glw->redraw();
				}
				break;
				//END_HYPOVERTEX

			}//end event switch
		} // mxEvent::Action
		break;

		case mxEvent::Size:
		{
			int w = event->width;
			int h = event->height;
			int y = UI_mb->getHeight();
#ifdef WIN32
			static const int tabHeight = 128; //104 //hypov8 was 120 with progress bar
#else
			static const int tabHeight = 140;
			h -= 40;
#endif

			//resize
			if (pakViewer->isVisible())
			{
				w -= 170;
				pakViewer->setBounds(w, y, 170, h); //hypov8 UI: 
			}
			//fix tga bug for avi file....always make glw even width
			if (w & 1)
				w += 1;
			if (h & 1)
				h += 1;

			glw->setBounds(0, y, w/*- pakW*/, h - tabHeight);
			//UI_bar->setBounds (0, y + h - 16, w-2, HEIGHT-105); //progress bar!!
			UI_tab->setBounds(0, y + h - tabHeight, w - 2, tabHeight);

		} //end mxEvent::Size:
		break;

		//case mxEvent::KeyUp:

		case mxEvent::KeyDown:
		{
			//hotkeys
			switch (event->key)
			{
				case VK_F5:
				{
					glw->reloadAllTextures();
				}
				break;

				case 'P':
				{
					setStatePaused();//pause
				}
				break;

				//IDC_RENDERMODE:
				case '1':
				case '2':
				case '3':
				case '4':
				{
					int val = event->key - 49;
					cRenderMode->select(val);
					setRenderMode(val);
					glw->redraw();
				}
			}
		} //end mxEvent::KeyUp:
		break;

		case mxEvent::MouseWheel:
		{
#if WIN32
			//SetFocus((HWND)glw->getHandle());
#endif
			if (event->zdelta<0)
				glw->d_transZ += 10;
			else
				glw->d_transZ -= 10;
			glw->redraw();
		} //mxEvent::MouseWheel:
		break;

		case mxEvent::MouseMove:
		{
#if 0 //WIN32
			int mouseX = event->x;
			int mouseY = event->y;

			int glWinX = 0; //glw->x() -g_mdxViewer->x();
			int glWinY = UI_mb->getHeight(); // glw->y() - g_mdxViewer->y() - UI_mb->y();

			int glWinW = glw->w()+glWinX;
			int glWinH = glw->h()+glWinY;

			//within opengl window
			if (mouseX > glWinX && mouseX < glWinW &&
				mouseY > glWinY && mouseY < glWinH)
			{
				// Grab focus as soon as the mouse is over the GL area
				// Using Win32 directly for compatibility
				if (GetFocus() != (HWND)glw->getHandle()) {
					SetFocus((HWND)glw->getHandle());
				}
			}
#endif

		} //end mxEvent::MouseMove:
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
MDXViewer::setStatePaused()
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
		//glReadBuffer (GL_FRONT);
		glReadBuffer (GL_BACK);
		glReadPixels (0, 0, w, h, GL_RGB , GL_UNSIGNED_BYTE, image->data); //GL_BGR_EXT //glGetString(GL_EXTENSIONS)

		//swap RGB to BGR
		byte *RGB = (byte*)image->data;
		byte r,b;
		for (int i = 0; i < w*h; i++)
		{
			r = RGB[0];
			b = RGB[2];
			RGB[0] = b;
			RGB[2] = r;
			RGB += 3;
		}
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
//Bresenham's Line Algorithm
void drawLine(byte *data, int width, int height, int x0, int y0, int x1, int y1, byte color[3]) 
{
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;
	int totalSize = width * height * 3;

	while (1) 
	{
		int index = (y0 * width + x0) * 3;
		if (index >= 0 && index < totalSize) // Ensure within bounds
		{ 
			data[index + 0] = color[2];  // B
			data[index + 1] = color[1];  // G
			data[index + 2] = color[0];  // R
		}
		else
		{
			int OOB = 1;
			OOB += 1;
		}

		if (x0 == x1 && y0 == y1) 
			break;
		int err2 = err * 2;
		if (err2 > -dy) 
		{ 
			err -= dy; 
			x0 += sx; 
		}
		if (err2 < dx) 
		{ 
			err += dx; 
			y0 += sy; 
		}
	}
}



void
MDXViewer::makeUVMapImage(const char *filename, int skinSize)
{
	int w;
	int h;
	float r, g, b;

	//get first model
	mdx_model_t*mdl = glw->getModel(0);
	if (!mdl)
		return;
	
	glw->getWFColor(&r, &g, &b);

	switch (skinSize)
	{
		case IDC_MAKE_UV_RES:
		case IDC_MAKE_UV_2K:
		{
			if (glw->d_textureValid[0])
			{	//use image0
				w = glw->d_textureRez[0][0];
				h = glw->d_textureRez[0][1];
			}
			else
			{	//fallback. use model size
				w = mdl->header.skinWidth;
				h = mdl->header.skinHeight;
			}
			//force min size
			if (w < 16) 
				w = 16;
			if (h < 16) 
				h = 16;

			//scale to 2k. keep aspect ratio
			if (skinSize == IDC_MAKE_UV_2K)
			{
				float scaleFactor = 1.0f;
				if (w > h)
					scaleFactor = 2048.f / w;
				else
					scaleFactor = 2048.f / h;

				w = (int)(w *  scaleFactor);
				h = (int)(h *  scaleFactor);
			}
			//fix odd size
			if (w & 1)
				w += 1;
			if (h & 1)
				h += 1;
		}
		break;

		default:
		case IDC_MAKE_UV_1K:
			w = 1024;
			h = 1024;
			break;
	}


	//create new  image
	mxImage *image = new mxImage ();
	if (!image)
		return;

	if (image->create (w, h, 24))
	{
		byte *data = (byte *) image->data;			
		byte wireColor[3] = { 
			(byte)(r * 255.0f), //line color. use UI wireframe colors
			(byte)(g * 255.0f),
			(byte)(b * 255.0f)};

		memset(data, 0, w*h*3); //all black?

		if (mdl->header.numGlCommands)
		{
			int isFinished = 0;
			int first, z, i = 0;
			int numStripVerts, numFanVerts;
			int *glCmds = mdl->glCommandBuffer;
			int TrisCount = 0;
			int TexTrisCount = 0;
			float st_v1[2], st_v2[2], st_v3[2];

			while (!isFinished)
			{
				first = glCmds[i++];

				if (!mdl->isMD2)
					i++; //mdx. object number. skip

				//////////////////
				// Triangle strip
				if (first > 0)
				{
					numStripVerts = first;
					//totalTris += numStripVerts - 2;

					//First vert UV pos
					st_v1[0] =        *(float*)&glCmds[i++]  * (w - 1);
					st_v1[1] = (1.f - *(float*)&glCmds[i++]) * (h - 1);
					i++; //vert index. skip 

					//Second vert
					st_v2[0] =        *(float*)&glCmds[i++]  * (w - 1);
					st_v2[1] = (1.f - *(float*)&glCmds[i++]) * (h - 1);
					i++; //vert index. skip 

					drawLine(data, w, h, (int)st_v1[0], (int)st_v1[1], (int)st_v2[0], (int)st_v2[1], wireColor);

					for (z = 1; z <= (numStripVerts - 2); z++)
					{
						st_v3[0] =        *(float*)&glCmds[i++]  * (w - 1);
						st_v3[1] = (1.f - *(float*)&glCmds[i++]) * (h - 1);
						i++;//vert index. skip 
						//draw line to start
						drawLine(data, w, h, (int)st_v1[0], (int)st_v1[1], (int)st_v3[0], (int)st_v3[1], wireColor);

						//draw line from previous vert to new vert
						drawLine(data, w, h, (int)st_v2[0], (int)st_v2[1], (int)st_v3[0], (int)st_v3[1], wireColor);

						//dupe to vert 1.
						st_v1[0] = st_v2[0];
						st_v1[1] = st_v2[1];
						//dupe to vert 2.
						st_v2[0] = st_v3[0];
						st_v2[1] = st_v3[1];
					}
				}
				////////////////
				// Triangle fan
				else if (first < 0)
				{
					numFanVerts = -first;
					//totalTris += numFanVerts - 2;

					//First vert
					st_v1[0] =        *(float*)&glCmds[i++]  * (w - 1);
					st_v1[1] = (1.f - *(float*)&glCmds[i++]) * (h - 1);
					i++;//vert index. skip 

					//Second vert
					st_v2[0] =        *(float*)&glCmds[i++]  * (w - 1);
					st_v2[1] = (1.f - *(float*)&glCmds[i++]) * (h - 1);
					i++;//vert index. skip 

					drawLine(data, w, h, (int)st_v1[0], (int)st_v1[1], (int)st_v2[0], (int)st_v2[1], wireColor);

					for (z = 1; z <= (numFanVerts - 2); z++)
					{
						st_v3[0] =        *(float*)&glCmds[i++]  * (w - 1);
						st_v3[1] = (1.f - *(float*)&glCmds[i++]) * (h - 1);
						i++;//vert index. skip 
						//draw line from previous vert to new vert
						drawLine(data, w, h, (int)st_v2[0], (int)st_v2[1], (int)st_v3[0], (int)st_v3[1], wireColor);

						//draw line back to start
						drawLine(data, w, h, (int)st_v1[0], (int)st_v1[1], (int)st_v3[0], (int)st_v3[1], wireColor);

						//dupe to vert 2.
						st_v2[0] = st_v3[0];
						st_v2[1] = st_v3[1];
					}
				}
				else if (first == 0)
				{
					isFinished = 1;
				}
			}
		}

		//write file
		if (!mxTgaWrite (filename, image))
			mxMessageBox (this, "Error writing screenshot.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);
	}

	delete image;

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
		float min_tmp[3], max_tmp[3];

		min[0] = min[1] = min[2] = 999999.0f;
		max[0] = max[1] = max[2] = -999999.0f;

		//todo loop all models?
		for (int i = 0; i < MAX_MODELS; i++)
		{
			mdx_model_t *mod = glw->getModel(i);
			if (mod)
			{
				mdx_getBoundingBox(mod, min_tmp, max_tmp, frame); // todo current frame?
				//get all model max bounds
				for (int j = 0; j < 3; j++)
				{
					if (min[j] > min_tmp[j])
						min[j] = min_tmp[j];
					if (max[j] < max_tmp[j])
						max[j] = max_tmp[j];
				}
			}
		}


		// adjust distance
		float dx = max[0] - min[0];
		float dy = max[1] - min[1];
		float dz = max[2] - min[2];
		float xPos = (max[0]/2) + (min[0]/2);
		float xPos2 = (max[0] + min[0]) / 2;
		float xPos3 = (max[0] - dx) / 2;

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
MDXViewer::loadModel_ (const char *ptr, int pos, bool getSkin)
{
	mdx_model_t *model;

	model = glw->loadModel (ptr,  pos, getSkin);
	if (!model)
		return false;

	if (pos == TEXTURE_MODEL_0)
	{	
		initAnimation (model, -1);
		centerModel (0, 0);	
		//if (model->header.numFrames == 1)
		setPauseMode(model->header.numFrames);
	}
	if (getSkin)
		glw->loadTexture(glw->d_modelTexNames[pos], pos);
	
	glw->redraw ();
	setDisplayModelInfo(); // model, pos); //hypov8 print details
	return true;
}

//#define getMin(a, b) ((a<b)? a:b)
//#define getMax(a, b) ((a>b)? a:b)

void
MDXViewer::setDisplayModelInfo() //mdx_model_t *model, int pos)
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
	int isHDModel = 0;
	mdx_model_t *model;

	for (int i = 0; i < MAX_MODELS; i++)
	{
		model = glw->getModel(i);
		if (model)
		{
			found = true;
			if (i == 0)
			{
				Frames = model->header.numFrames;
				if (model->header.frameSize == (int)(40 + model->header.numVertices * 4 + model->header.numVertices * 3))
					isHDModel = 1;
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
			"max (%5.1f, %5.1f, %5.1f)\n"
			"HD: %s",
			bMin[0], bMin[1], bMin[2],
			bMax[0], bMax[1], bMax[2],
			(isHDModel)? "Yes" : "No");

		sprintf_s(str3, sizeof(str3),
			"Model: %s\n"
			"Skin0: %s", 
			glw->d_modelFileNames[0],
			glw->d_modelTexNames[0]);
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
	for (int i = 0; i < model->header.numFrames; i++)
	//for (int i = 0; i < count; i++)
	{
		cAnim->add(mdx_getAnimationName(model, &i));
	}

	if (animation <= 0)
	{
		glw->setFrameInfo(0, model->header.numFrames - 1);
		cAnim->select (0);
	}
	else
	{
		int startFrame, endFrame;
		mdx_getAnimationFrames(model, animation-1, &startFrame, &endFrame);
		glw->setFrameInfo(startFrame, endFrame);
		cAnim->select (animation);
	}
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
-4=start textured+wire
-5=start wireframe backface-culled
folder/file name
*/
void ProcessArgs(int argc, char *argv[])
{
	int arg_Idx = 1;
	int file_Idx = 0;
	//d_startPaused = 0;
	//d_startMode = 0;
	while (arg_Idx < argc)
	{
		if (argv[arg_Idx][0] == '-')
		{
			char a = argv[arg_Idx][1];

			/*if (a == 'p') //-p
				d_startPaused = 1;
			//-0, -1, -2, -3 (render mode)
			else if (a == '0') //-0 wireframe
				d_startMode = 0;
			else if (a == '1') //-1 flat shadded
				d_startMode = 1;
			else if (a == '2') //-2 smooth shaded
				d_startMode = 2;
			else if (a == '3') //-3 textured
				d_startMode = 3;*/
			/*else if (a == '4') //-4 textured wireframe
				d_startMode = 4;
			else if (a == '5') //-5 wireframe backface-culled
				d_startMode = 5;*/
		}
		else if (file_Idx < MAX_MODELS)
		{	
			//copy any other args to filename
			strcpy_s(startupModelsNames[file_Idx], sizeof(startupModelsNames[0]), argv[arg_Idx]);
			file_Idx++;
		}
		arg_Idx++;
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
