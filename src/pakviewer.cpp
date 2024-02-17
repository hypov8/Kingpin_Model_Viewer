#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mx/mx.h>
#include "pakviewer.h"
//#include "common.h"
#include "GlWindow.h"
#include "mdxviewer.h"


int
pak_ExtractFile (const char *pakFile, const char *lumpName, char *outFile)
{
	FILE *file =NULL;
	if (fopen_s(&file, pakFile, "rb"))
		return 0;

	int ident, dirofs, dirlen;

	fread (&ident, sizeof (int), 1, file);
	if (ident != (int) (('K' << 24) + ('C' << 16) + ('A' << 8) + 'P'))
	{
		fclose (file);
		return 0;
	}

	fread (&dirofs, sizeof (int), 1, file);
	fread (&dirlen, sizeof (int), 1, file);

	fseek (file, dirofs, SEEK_SET);
	int numLumps = dirlen / 64;

	for (int i = 0; i < numLumps; i++)
	{
		char name[56];
		int filepos, filelen;

		fread (name, 56, 1, file);
		fread (&filepos, sizeof (int), 1, file);
		fread (&filelen, sizeof (int), 1, file);

		if (!strcmp (name, lumpName))
		{
			FILE *out = NULL;
			if (fopen_s(&out, outFile, "wb"))
			{
				fclose (file);
				return 0;
			}

			fseek (file, filepos, SEEK_SET);

			while (filelen--)
				fputc (fgetc (file), out);

			fclose (out);
			fclose (file);

			return 1;
		}
	}

	fclose (file);

	return 0;
}



PAKViewer::PAKViewer (mxWindow *window)
: mxWindow (window, 0, 0, 0, 0)
{
	d_pakFile[0] = '\0';//strcpy_s (d_pakFile, sizeof(d_pakFile), "");
	d_currLumpName[0] = '\0';//strcpy_s (d_currLumpName, sizeof(d_currLumpName), "");
	
	
	UI_tvPAK = new mxTreeView (this, 0, 0, 0, 0, IDC_PAKVIEWER);
	pmMenu = new mxPopupMenu ();
	pmMenu->add ("Load Model/Tex", 1);
	pmMenu->add ("Merge Model/Tex", 2);
	pmMenu->addSeparator ();
	pmMenu->add ("Load Background", 3);
	pmMenu->add ("Load Water", 4);
	pmMenu->addSeparator ();
	pmMenu->add ("Extract...", 5);

	setLoadEntirePAK (false);

	UI_pakTab = new mxTab (this, 0, 0, 0, 0);
	UI_pakTab->add (UI_tvPAK, "PAK");
}



PAKViewer::~PAKViewer ()
{
	closePAKFile ();
}



void
_makeTempFileName (char *str, const char *prefix)
{
	char path2[256];

	GetTempPath (256, path2);

	strcpy_s (str, 256, path2);
	strcat_s (str, 256, "mdxtemp"); //hypov8 removed fdSlash
	strcat_s (str, 256, prefix);
}



int
PAKViewer::handleEvent (mxEvent *event)
{
	switch (event->event)
	{
	case mxEvent::Action:
	{
		switch (event->action)
		{
		case IDC_PAKVIEWER: // tvPAK
			if (event->flags & mxEvent::RightClicked)
			{
				OnPAKViewer ();
				char ext[256];

				strcpy_s(ext, sizeof(ext), mx_getextension(d_currLumpName));
				bool isMdx = (!mx_strcasecmp(ext, ".mdx")); //hypov8 capitals?
				bool isMd2 = (!mx_strcasecmp(ext, ".md2"));
				bool isPcx = (!mx_strcasecmp(ext, ".pcx"));
				bool isTga = (!mx_strcasecmp(ext, ".tga"));
				pmMenu->setEnabled (1, isMdx|| isMd2|| isPcx || isTga);
				pmMenu->setEnabled (2, isMdx|| isMd2|| isPcx || isTga);
				pmMenu->setEnabled (3, isPcx || isTga);
				pmMenu->setEnabled (4, isPcx || isTga);

				int ret = pmMenu->popup (UI_tvPAK, event->x, event->y);
				int tess = g_mdxViewer->glw->getModelIndex();
				switch (ret)
				{
				case 1: // load model
					if (isMdx|| isMd2)
						OnLoadModel (0);
					else if (isTga|| isPcx)
						OnLoadTexture (TEXTURE_MODEL_0, isTga? true: false);
					break;

				case 2: // merge model
					if (isMdx || isMd2)
					{
						HWND hwID = (HWND)g_mdxViewer->getHandle();
						if (tess >= 6)//max models		
							MessageBox(hwID, "Limit of 6 models reached.\nUse \"Load Model\" instead ", "Note", MB_OK);
						else
							OnLoadModel(tess);
					}
					else if (tess > 0 && (isTga || isPcx))//allow setting model 0 skins
						OnLoadTexture(tess - 1, isTga ? true : false);
					break;

				case 3: //load bg
				case 4: //load water
					if (isTga || isPcx)
						OnLoadTexture(ret == 3 ? TEXTURE_BACKGROUND : TEXTURE_WATER, isTga ? true : false);
					break;

				case 5:
					OnExtract ();
				}
			}

			else if (event->flags & mxEvent::DoubleClicked)
			{
				OnPAKViewer ();
				char ext[256];

				strcpy_s(ext, sizeof(ext), mx_getextension(d_currLumpName));
				bool isMdx = (!mx_strcasecmp(ext, ".mdx"));
				bool isMd2 = (!mx_strcasecmp(ext, ".md2"));
				bool isTga = (!mx_strcasecmp(ext, ".tga"));
				bool isPcx = (!mx_strcasecmp(ext, ".pcx"));
				
				if (isMdx || isMd2)
					OnLoadModel (0);

				else if (isTga || isPcx)
					OnLoadTexture (TEXTURE_BACKGROUND, isTga ? true : false);

				else if (!mx_strcasecmp(ext, ".wav"))
					OnPlaySound ();

				return 1;
			}

			return 1;
		} // event->action
	} // mxEvent::Action
	break;

	case mxEvent::Size:
	{
		UI_pakTab->setBounds (0, 0, event->width, event->height);
	} // mxEvent::Size
	break;

	} // event->event

	return 1;
}



int
PAKViewer::OnPAKViewer ()
{
	mxTreeViewItem *tvi = UI_tvPAK->getSelectedItem ();
	if (tvi)
	{
		strcpy_s (d_currLumpName, sizeof(d_currLumpName), UI_tvPAK->getLabel (tvi));

		// find the full lump name
		mxTreeViewItem *tviParent = UI_tvPAK->getParent (tvi);
		char tmp[128];
		while (tviParent)
		{
			strcpy_s (tmp, sizeof(tmp), d_currLumpName);
			strcpy_s (d_currLumpName, sizeof(d_currLumpName), UI_tvPAK->getLabel (tviParent));
			strcat_s (d_currLumpName, sizeof(d_currLumpName), "/");
			strcat_s (d_currLumpName, sizeof(d_currLumpName), tmp);
			tviParent = UI_tvPAK->getParent (tviParent);
		}

		if (!d_loadEntirePAK)
		{
			// finally insert "models/"
			strcpy_s (tmp, sizeof(tmp), d_currLumpName);
			strcpy_s (d_currLumpName, sizeof(d_currLumpName), "models/");
			strcat_s (d_currLumpName, sizeof(d_currLumpName), tmp);
		}
	}

	return 1;
}


int
PAKViewer::OnLoadModel (int pos)
{
	static char str2[256];
	static char ext[256];
	char prefix[16];

	strcpy_s(ext, sizeof(ext), mx_getextension(d_currLumpName));

	if (!mx_strncasecmp(ext, ".mdx", 4))
		sprintf_s (prefix, sizeof(prefix), "_%d%s", pos, ".mdx");
	else 
		sprintf_s (prefix, sizeof(prefix), "_%d%s", pos, ".md2");

	_makeTempFileName (str2, prefix);

	// extract the file from the pak file and give it a temp name
	if (!pak_ExtractFile (d_pakFile, d_currLumpName, str2))
	{
		mxMessageBox (this, "Error extracting from PAK file.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);
		return 1;
	}

	if (pos == TEXTURE_MODEL_0)
		g_mdxViewer->reset_modelData();

	// now load the things
	mdx_model_t *model = g_mdxViewer->glw->loadModel (str2, pos);
	if (model)
	{
		if (pos == TEXTURE_MODEL_0)
		{
			g_mdxViewer->initAnimation(model, -1);
			g_mdxViewer->centerModel(0, 0); //hypov8 add
			//if (model->header.numFrames == 1)
			g_mdxViewer->setPauseMode(model->header.numFrames);
		}
		//g_mdxViewer->setModelInfo (model, pos);

		// try to load skin
		if (model->header.numSkins > 0)
		{
			static char ext2[256];

			strcpy_s(ext2, sizeof(ext2), mx_getextension(model->skins[0]));
			if (!mx_strncasecmp(ext2, ".tga", 4))
				sprintf_s (prefix, sizeof(prefix), "_%d%s", pos, ".tga");
			else
				sprintf_s(prefix, sizeof(prefix), "_%d%s", pos, ".pcx");

			_makeTempFileName (str2, prefix);

			if (pak_ExtractFile (d_pakFile, model->skins[0], str2))
				g_mdxViewer->glw->loadTexture(str2, pos);
			else //missing skin in pack. try oudside pack
			{
				if (d_pakFile)
				{
					strcpy_s(str2, sizeof(str2), mx_getpath(d_pakFile));
					strcat_s(str2, sizeof(str2), model->skins[0]);
					g_mdxViewer->glw->loadTexture(str2, pos);					
				}
			}
		}

		if (pos == 0)
			g_mdxViewer->centerModel (0, 0);

		g_mdxViewer->setModelInfo();//moved down

		g_mdxViewer->glw->redraw ();
	}
	else
		mxMessageBox (this, "Error loading model.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);

	return 1;
}



int
PAKViewer::OnLoadTexture (int pos, bool isTarga)
{
	char str2[256];
	char prefix[16];


	sprintf_s (prefix, sizeof(prefix), "_%d%s", pos, isTarga ? ".tga":".pcx");
	_makeTempFileName (str2, prefix);

	// extract the file from the pak file and give it a temp name
	if (!pak_ExtractFile (d_pakFile, d_currLumpName, str2))
	{
		mxMessageBox (this, "Error extracting from PAK file.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);
		return 1;
	}

	// now load the things
	if (g_mdxViewer->glw->loadTexture (str2, pos))
	{
		if (pos == TEXTURE_BACKGROUND)
		{
			g_mdxViewer->cbBackground->setChecked (true);
			g_mdxViewer->glw->setFlag (F_BACKGROUND, true);
		}
		else if (pos == TEXTURE_WATER)
		{
			g_mdxViewer->cbWater->setChecked (true);
			g_mdxViewer->glw->setFlag (F_WATER, true);
		}
		g_mdxViewer->setModelInfo();
		g_mdxViewer->glw->redraw ();
	}
	else
		mxMessageBox (this, "Error loading skin.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);

	return 1;
}



int
PAKViewer::OnPlaySound ()
{
#ifdef WIN32
	static char str2[256];
	char suffix[16] = "";

	// stop any playing sound
	PlaySound (0, 0, SND_FILENAME | SND_ASYNC);

	sprintf_s (suffix, sizeof(suffix), "_%d%s", 44, ".wav");

	_makeTempFileName (str2, suffix);

	if (!pak_ExtractFile (d_pakFile, d_currLumpName, str2))
	{
		mxMessageBox (this, "Error extracting from PAK file.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);
		return 1;
	}

	PlaySound (str2, 0, SND_FILENAME | SND_ASYNC);

#endif
	return 1;
}



int
PAKViewer::OnExtract ()
{
	char *ptr = (char *) mxGetSaveFileName (this, 0, 0);
	if (ptr)
	{
		if (!pak_ExtractFile (d_pakFile, d_currLumpName, ptr))
			mxMessageBox (this, "Error extracting from PAK file.", "Kingpin Model Viewer", MX_MB_OK | MX_MB_ERROR);
	}

	return 1;
}



int
_compare(const void *arg1, const void *arg2)
{
	if (strchr ((char *) arg1, '/') && !strchr ((char *) arg2, '/'))
		return -1;

	else if (!strchr ((char *) arg1, '/') && strchr ((char *) arg2, '/'))
		return 1;

	else
		return strcmp ((char *) arg1, (char *) arg2);
}



bool
PAKViewer::openPAKFile (const char *pakFile)
{
	FILE *file = NULL;
	if (fopen_s(&file, pakFile, "rb"))
		return false;

	int ident, dirofs, dirlen;

	// check for id
	fread (&ident, sizeof (int), 1, file);
	if (ident != (int) (('K' << 24) + ('C' << 16) + ('A' << 8) + 'P'))
	{
		fclose (file);
		return false;
	}

	// load lumps
	fread (&dirofs, sizeof (int), 1, file);
	fread (&dirlen, sizeof (int), 1, file);
	int numLumps = dirlen / 64;

	fseek (file, dirofs, SEEK_SET);
	lump_t *lumps = new lump_t[numLumps];
	if (!lumps)
	{
		fclose (file);
		return false;
	}

	fread (lumps, sizeof (lump_t), numLumps, file);
	fclose (file);

	qsort (lumps, numLumps, sizeof (lump_t), _compare);

	// save pakFile for later
	strcpy_s (d_pakFile, sizeof(d_pakFile), pakFile);

	UI_tvPAK->remove (0);

	char namestack[32][32];
	mxTreeViewItem *tvistack[32];
	for (int k = 0; k < 32; k++)
	{
		namestack[k][0] = '\0';
		//strcpy_s (namestack[k], sizeof(namestack[k]), "");
		tvistack[k] = 0;
	}

	for (int i = 0; i < numLumps; i++)
	{
		if (d_loadEntirePAK || !strncmp (lumps[i].name, "models", 6))
		{
			char *tok;
			if (d_loadEntirePAK)
				tok = &lumps[i].name[0];
			else
				tok = &lumps[i].name[7];

			int i = 1;
			while (tok)
			{
				char *end = strchr (tok, '/');
				if (end)
					*end = '\0';

				if (strcmp (namestack[i], tok))
				{
					strcpy_s (namestack[i], sizeof(namestack[i]), tok);

					tvistack[i] = UI_tvPAK->add (tvistack[i - 1], tok);

					for (int j = i + 1; j < 32; j++)
					{
						namestack[j][0] = '\0';
						//strcpy_s (namestack[j], sizeof(namestack[j]), "");
						tvistack[j] = 0;
					}
				}

				++i;

				if (end)
					tok = end + 1;
				else
					tok = 0;
			}
		}
	}

	delete[] lumps;

	setVisible(1);
	return true;
}



void
PAKViewer::closePAKFile ()
{
	d_pakFile[0] = '\0';
	//strcpy_s (d_pakFile, sizeof(d_pakFile), "");
	UI_tvPAK->remove (0);
}
