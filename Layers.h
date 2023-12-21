#pragma once
//
// MySETIviewer, a set tools for decoding bitstreams into various formats and manipulating those files
// Layers.h
// (C) 2023, Mark Stegall
// Author: Mark Stegall
//
// This file is part of MySETIviewer.
//
// MySETIviewer is free software : you can redistribute it and /or modify it under
// the terms of the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// MySETIviewer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with MySETIapp.
// If not, see < https://www.gnu.org/licenses/>.
//
// This file contains the forward declarations of the Layers class
// This class handles the conifguration data and image memory
// used in creating multiple image overlays for display
// 
// V1.0.1.0	2023-12-20	Initial release
// V1.0.2.0 2023-12-20  Added Y direction flag for which direction to move image
//
#include "framework.h"

#define MAX_LAYERS 8

class Layers {
private:
	// variables
	int* LayerImage[MAX_LAYERS] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
	int LayerXsize[MAX_LAYERS] = { 0,0,0,0,0,0,0,0 };
	int LayerYsize[MAX_LAYERS] = { 0,0,0,0,0,0,0,0 };
	COLORREF LayerColor[MAX_LAYERS] = { 0,0,0,0,0,0,0,0 }; // color to use for layer when pixel != 0
	int LayerX[MAX_LAYERS] = { 0,0,0,0,0,0,0,0 };
	int LayerY[MAX_LAYERS] = { 0,0,0,0,0,0,0,0 };
	BOOL Enabled[MAX_LAYERS] = { FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE };

	COLORREF rgbBackgroundColor = 0; // color used for Layer backgrounds when pixel is 0
	COLORREF rgbOverlayColor = 0; // color used for overlay background
	COLORREF rgbDefaultLayerColor = 0;
	COLORREF* OverlayImage = NULL;

	int NumLayers = 0;
	int CurrentLayer = 0;
	int ImageXextent = 0;
	int ImageYextent = 0;
	int Xextent0 = 0;
	int Yextent0 = 0;
	int minOverlaySizeX = 512;
	int minOverlaySizeY = 512;
	int yposDir = 0;

public:
	// variables
	WCHAR* LayerFilename[MAX_LAYERS] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
	WCHAR ConfigurationFile[MAX_PATH] = L"";

	BOOL OverlayValid = FALSE;

	// forward method/function declarations
	//	method/functions definition are done in Layers.cpp
	
	Layers();

	~Layers();

	int AddLayer(WCHAR* Filename);
	int ReleaseLayer(int LayerNum);

	int CreateOverlay(int xsize,int ysize);
	int ReleaseOverlay(void);
	int UpdateOverlay(void);
	int GetNewOverlaySize(int* x, int* y);
	int GetCurrentOverlaySize(int* x, int* y);

	int SaveConfiguration(void);
	int SaveConfiguration(WCHAR* Filename);
	int LoadConfiguration(WCHAR* Filename);

	int GetNumLayers(void);
	int SetCurrentLayer(int LayerNumber);
	int GetCurrentLayer(void);

	int GetSize(int Layer, int* x, int* y);
	int GetLocation(int Layer, int* x, int* y);
	int SetLocation(int Layer, int x, int y);

	COLORREF GetBackgroundColor(void);
	void SetBackgroundColor(COLORREF Color);

	COLORREF GetDefaultLayerColor(void);
	void SetDefaultLayerColor(COLORREF Color);

	COLORREF GetOverlayColor(void);
	void SetOverlayColor(COLORREF Color);

	COLORREF GetLayerColor(int Layer);
	int SetLayerColor(int Layer, COLORREF Color);
	
	int DisableLayer(int Layer);
	int EnableLayer(int Layer);
	BOOL IsLayerEnabled(int Layer);

	void GetMinOverlaySize(int* x,int* y);
	void SetMinOverlaySize(int x, int y);

	int GetYdir();
	void SetYdir(int ydir);


	int GetOverlayImage(COLORREF** OverlayImage, int* xsize, int* ysize);

	int SaveBMP(WCHAR* Filename);

};