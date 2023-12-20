//
// MySETIviewer, a set tools for decoding bitstreams into various formats and manipulating those files
// Layers.cpp
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
// This file contains the definitions of the Layers class methods/functions
// This class handles the conifguration data and image memory
// used in creating multiple image overlays for display
// 
// V0.1.0.1 2023-12-04	Initial pre release
// V0.3.0.1 2023-12-13	Changes to help integrate Image Dialog requirements
// V0.4.1.1 2023-12-15	Added minimum Overlay Size
//
// Application standardized error numbers for functions:
//		See AppErrors.h
//
#include "framework.h"
#include <shtypes.h>
#include <string.h>
#include <stdio.h>
#include "AppErrors.h"
#include "imageheader.h"
#include "FileFunctions.h"
#include "Layers.h"

//*******************************************************************************
//
//  Layers()
//	Class constructor
// 
//*******************************************************************************
Layers::Layers() {
	// constructor
};

//*******************************************************************************
//
//  ~Layers()
//	Class destuctor
// 
//*******************************************************************************
Layers::~Layers() {
	// destuctor
	ReleaseOverlay();
	for (int i = NumLayers - 1; i >= 0; i--) {
		ReleaseLayer(i);
	}
};

//*******************************************************************************
//
//  ReleaseOverlay(void)
// 
//*******************************************************************************
int Layers::ReleaseOverlay(void) {
	if (OverlayImage != NULL) {
		delete[] OverlayImage;
		OverlayImage = NULL;
	}
	OverlayValid = FALSE;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  int AddLayer(WCHAR* Filename)
// 
// This adds a layer into the conifguration
// Currently all files are treated as a binary (0,1) image
// Future expansion will distinguish between binary image, monochrome, color image
// 
// WCHAR* Filename		Image, or BMP file to add as a layer
// 
// return
// int					APP_SUCCESS, 1,	Success
//						APPERR_PARAMETER, max layers already reached
//
//*******************************************************************************
int Layers::AddLayer(WCHAR* Filename) {
	int iRes;
	int* Image;
	IMAGINGHEADER ImageHeader;

	if (NumLayers > MAX_LAYERS) {
		return APPERR_PARAMETER;
	}

	// try loading as .img file
	iRes = LoadImageFile(&Image, Filename, &ImageHeader);
	if (iRes != APP_SUCCESS) {
		iRes = LoadBMPfile(&Image, Filename, &ImageHeader);
		if (iRes != APP_SUCCESS) {
			return iRes;
		}
	}

	// save results in Layers class variables
	LayerImage[NumLayers] = Image;
	LayerXsize[NumLayers] = ImageHeader.Xsize;
	LayerYsize[NumLayers] = ImageHeader.Ysize;

	WCHAR* FileAdded;
	FileAdded = new WCHAR[MAX_PATH];
	wcscpy_s(FileAdded, MAX_PATH,Filename);
	LayerFilename[NumLayers] = FileAdded;

	LayerColor[NumLayers] = 0xffffff;
	LayerX[NumLayers] = 0;
	LayerY[NumLayers] = 0;
	Enabled[NumLayers] = TRUE;

	NumLayers++;
	OverlayValid = FALSE;

	return APP_SUCCESS;
};

//*******************************************************************************
//
//  int ReleaseLayer(int LayerNum)
// 
// This deletes a layer from the conifguration
// 
// int LayerNum			Layer number to delete
// 
// return
// int					1	Success
//						!=1	Standard application error number
//
//*******************************************************************************
int Layers::ReleaseLayer(int LayerNum) {
	if (LayerNum < 0 || LayerNum >= NumLayers) {
		return APPERR_PARAMETER;
	}
	// release allocated memory
	delete[] LayerImage[LayerNum];
	delete[] LayerFilename[LayerNum];

	NumLayers--;

	if (LayerNum == NumLayers) {
		// last layer was the one deleted
		OverlayValid = FALSE;
		return APP_SUCCESS;
	}
	// move the rest of the layers down 1 slot
	for (int i = LayerNum; i < NumLayers; i++) {
		LayerImage[i] = LayerImage[i+1];
		LayerXsize[i] = LayerXsize[i+1];
		LayerYsize[i] = LayerYsize[i+1];
		LayerColor[i] = LayerColor[i+1];
		LayerX[i] = LayerX[i+1];
		LayerY[i] = LayerY[i+1];
		Enabled[i] = Enabled[i + 1];
		LayerFilename[i] = LayerFilename[i+1];
	}
	OverlayValid = FALSE;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  int CreateOverlay(void)
// 
// This generates the overlay image from the current Layer configuration
// 
// return
// int					1	Success
//						!=1	Standard application error number
//
//*******************************************************************************
int Layers::CreateOverlay(int xsize, int ysize) {
	OverlayImage = new COLORREF[xsize * ysize];
	if (OverlayImage == NULL) {
		ImageXextent = 0;
		ImageYextent = 0;
		return APPERR_MEMALLOC;
	}

	for (int i = 0; i < (xsize * ysize); i++) {
		OverlayImage[i] = rgbOverlayColor;
	}

	ImageXextent = xsize;
	ImageYextent = ysize;
	// Overlay isn't valid until it is updated using UpdateOverlay()
	OverlayValid = FALSE;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  int UpdateOverlay(void)
// 
// This generates the overlay image from the current Layer configuration
// 
// return
// int					1	Success
//						!=1	Standard application error number
//
//*******************************************************************************
int Layers::UpdateOverlay(void) {
	// process each layer
	int oAddress;
	int oOffset;
	int iAddress;
	int ImageXsize;
	int ImageYsize;
	int Pixel;
	int* Image;

	union {
		COLORREF Color;
		RGBQUAD rgb;
	} OverlayPixel;

	union {
		COLORREF Color;
		RGBQUAD rgb;
	} NewColor;

	union {
		COLORREF Color;
		RGBQUAD rgb;
	} iColor;

	int Colorsum;

	for (int Layer = 0; Layer < NumLayers; Layer++) {
		ImageXsize = LayerXsize[Layer];
		ImageYsize = LayerYsize[Layer];
		Image = LayerImage[Layer];
		if (!Enabled[Layer] || LayerColor[Layer] == rgbOverlayColor) {
			// if current layer is not enabled, do no add layer to overlay
			// if current layer color is the overlay color, do not add layer to overlay
			continue;
		}
		iColor.Color = LayerColor[Layer];

		oAddress = ((Yextent0+ LayerY[Layer])- (LayerYsize[Layer]/2)) * ImageXextent;
		
		iAddress = 0;

		for (int y = 0; y < ImageYsize;
			 y++, iAddress += ImageXsize, oAddress += ImageXextent) {

			oOffset = ((Xextent0 + LayerX[Layer]) - (LayerXsize[Layer] / 2));

			for (int x = 0; x < ImageXsize; x++, oOffset++) {
				Pixel = Image[iAddress + x];
				OverlayPixel.Color = OverlayImage[oAddress+ oOffset];
				if (Pixel == 0) {
					// if pixel is already set ignore
					if (OverlayPixel.Color != rgbBackgroundColor &&
						OverlayPixel.Color != rgbOverlayColor) {
						continue;
					}
					OverlayImage[oAddress + oOffset] = rgbBackgroundColor;
				}
				else {
					// pixel is not zero
					if (OverlayPixel.Color == rgbBackgroundColor ||
						OverlayPixel.Color == rgbOverlayColor) {
						// pixel was not previously set high
						// set pixel to layer color
						OverlayImage[oAddress + oOffset] = LayerColor[Layer];
					}
					else {
						// pixel already set, add the 2 COLORREF values
						Colorsum = (int)OverlayPixel.rgb.rgbRed + (int)iColor.rgb.rgbRed;
						if (Colorsum > 255) Colorsum = 255;
						NewColor.rgb.rgbRed = Colorsum;

						Colorsum = (int)OverlayPixel.rgb.rgbGreen + (int)iColor.rgb.rgbGreen;
						if (Colorsum > 255) Colorsum = 255;
						NewColor.rgb.rgbGreen = Colorsum;

						Colorsum = (int)OverlayPixel.rgb.rgbBlue + (int)iColor.rgb.rgbBlue;
						if (Colorsum > 255) Colorsum = 255;
						NewColor.rgb.rgbBlue = Colorsum;

						OverlayImage[oAddress + oOffset] = NewColor.Color;
					}
				}
			}
		}
	}
	OverlayValid = TRUE;

	return APP_SUCCESS;
};

//*******************************************************************************
//
//  int GetNewOverlaySize(int* x, int* y)
// 
// calculate the size of the image overlay
// 
// It is based on the sizes of all the layers plus the layer location.
// All layers are included in the calcualtion even if layer is not enabled for display.
// 
// return
// int					1	Success
//						!=1	Standard application error number
//
//*******************************************************************************
int Layers::GetNewOverlaySize(int* x, int* y) {
	// The image location 0,0 is centric
	// The position of 0,0 is calculated from the min, max annd resultings extent size
	// The image location in the of a given layers is based on it size and x,y pos
	// The size of the overlay is the Delta X max and the Delta Y Max of all the
	// layers.
	// A minimum overlay size is also used
	//
	int xmin = -minOverlaySizeX / 2, xmax = minOverlaySizeX / 2;
	int ymin = -minOverlaySizeY / 2, ymax = minOverlaySizeY / 2;
	int xlow = 0, xhigh = 0;
	int ylow = 0, yhigh = 0;
	int xnew = 0, ynew = 0;

	for (int Layer = 0; Layer < NumLayers; Layer++) {
		// half the size plus the position offset from center
		xlow = (-LayerXsize[Layer] / 2) + LayerX[Layer];
		xhigh = LayerXsize[Layer] + xlow;
		ylow = (-LayerYsize[Layer] / 2) + LayerY[Layer];
		yhigh = LayerYsize[Layer] + ylow;
	
		if (xlow < xmin) xmin = xlow;
		if (ylow < ymin) ymin = ylow;
		if (xhigh > xmax) xmax = xhigh;
		if (yhigh > ymax) ymax = yhigh;
	}

	xnew = (xmax - xmin);
	ynew = (ymax - ymin);


	// Index where 0,0 is in the ImageExtent
	// This is needed to porperly insert and image
	// relative to the othe images
	Xextent0 = (-xmin);
	Yextent0 = (-ymin);

	if (xnew <= 0 || ynew <= 0) {
		*x = 0;
		*y = 0;
		return APPERR_PARAMETER;
	}

	*x = xnew;
	*y = ynew;

	return APP_SUCCESS;
};

//*******************************************************************************
//
//  int GetCurrentOverlaySize(int* x, int* y)
// 
// This generates the overlay image from the current Layer configuration
// 
// return
// int					1	Success
//						!=1	Standard application error number
//
//*******************************************************************************
int Layers::GetCurrentOverlaySize(int* x, int* y) {
	*x = ImageXextent;
	*y = ImageYextent;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  int SaveConfiguration(WCHAR* Filename)
// 
// This generates the overlay image from the current Layer configuration
// 
// WCHAR* Filename		Save current congifuration in this file
// 
// return
// int					1	Success
//						!=1	Standard application error number
//
//*******************************************************************************
int Layers::SaveConfiguration(WCHAR* Filename) {

	// Save all this
	WCHAR szString[MAX_PATH];
	WCHAR AppName[MAX_PATH];
	int  iRes;

	swprintf_s(szString, MAX_PATH, L"%d", NumLayers);
	iRes = WritePrivateProfileString(L"Layers", L"NumLayers", szString, Filename);
	if (iRes == 0) {
		return APPERR_FILEOPEN;
	}

	swprintf_s(szString, MAX_PATH, L"%u", rgbBackgroundColor);
	iRes = WritePrivateProfileString(L"Layers", L"BackgroundColor", szString, Filename);

	swprintf_s(szString, MAX_PATH, L"%u", rgbDefaultLayerColor);
	iRes = WritePrivateProfileString(L"Layers", L"DefaultLayerColor", szString, Filename);

	swprintf_s(szString, MAX_PATH, L"%u", rgbOverlayColor);
	iRes = WritePrivateProfileString(L"Layers", L"OverlayColor", szString, Filename);

	swprintf_s(szString, MAX_PATH, L"%d", CurrentLayer);
	iRes = WritePrivateProfileString(L"Layers", L"CurrentLayer", szString, Filename);

	swprintf_s(szString, MAX_PATH, L"%d", minOverlaySizeX);
	iRes = WritePrivateProfileString(L"Layers", L"minOverlaySizeX", szString, Filename);
	
	swprintf_s(szString, MAX_PATH, L"%d", minOverlaySizeY);
	iRes = WritePrivateProfileString(L"Layers", L"minOverlaySizeY", szString, Filename);

	for (int i = 0; i < NumLayers; i++) {
		swprintf_s(AppName, MAX_PATH, L"Layers-%d", i);

		iRes = WritePrivateProfileString(AppName, L"LayerFilename", LayerFilename[i], Filename);

		swprintf_s(szString, MAX_PATH, L"%u", LayerColor[i]);
		iRes = WritePrivateProfileString(AppName, L"LayerColor", szString, Filename);

		swprintf_s(szString, MAX_PATH, L"%d", LayerX[i]);
		iRes = WritePrivateProfileString(AppName, L"LayerX", szString, Filename);

		swprintf_s(szString, MAX_PATH, L"%d", LayerY[i]);
		iRes = WritePrivateProfileString(AppName, L"LayerY", szString, Filename);

		swprintf_s(szString, MAX_PATH, L"%d", Enabled[i]);
		iRes = WritePrivateProfileString(AppName, L"Enabled", szString, Filename);
	}

	return APP_SUCCESS;
};

//*******************************************************************************
//
//  int SaveConfiguration(void)
// 
// This generates the overlay image from the current Layer configuration
// 
// return
// int					1	Success
//						!=1	Standard application error number
//
//*******************************************************************************
int Layers::SaveConfiguration() {

	int iRes;
	iRes = SaveConfiguration(ConfigurationFile);
	return iRes;
}

//*******************************************************************************
//
//  int LoadConfiguration(WCHAR* Filename)
// 
// This loads the Layer configuration from a cfg file
//
// WCHAR* Filename		Load conifiguration from this file
// 
// return
// int					APP_SUCESS,	Success
//						APPERR_FILESIZE, not all layers successfully loaded
//						APPERR_FILEOPEN, configuration file could not be read
//
//*******************************************************************************
int Layers::LoadConfiguration(WCHAR* Filename) {
	// Save all this
	WCHAR szString[MAX_PATH];
	WCHAR AppName[MAX_PATH];
	int  iRes;
	int TotalLayers;
	int LayerCount;

	ReleaseOverlay();
	// release all current layers
	for (int i = NumLayers-1; i >=0; i--) {
		ReleaseLayer(i);
	}

	TotalLayers = GetPrivateProfileInt(L"Layers", L"NumLayers", -1, Filename);
	if (TotalLayers == -1) {
		return APPERR_FILEOPEN;
	}

	rgbBackgroundColor = GetPrivateProfileInt(L"Layers", L"BackgroundColor", 1, Filename);
	rgbDefaultLayerColor = GetPrivateProfileInt(L"Layers", L"DefautLayerColor", 1, Filename);
	rgbOverlayColor = GetPrivateProfileInt(L"Layers", L"OverlayColor", 1, Filename);

	CurrentLayer = GetPrivateProfileInt(L"Layers", L"CurrentLayer", 0, Filename);
	
	minOverlaySizeX = GetPrivateProfileInt(L"Layers", L"minOverlaySizeX", 512, Filename);
	minOverlaySizeY = GetPrivateProfileInt(L"Layers", L"minOverlaySizeY", 512, Filename);

	LayerCount = 0;
	for (int i = 0; i < TotalLayers; i++) {
		swprintf_s(AppName, MAX_PATH, L"Layers-%d", i);

		GetPrivateProfileString(AppName, L"LayerFilename",L"", szString, MAX_PATH, Filename);
		// initially added as a new layer
		iRes = AddLayer(szString);
		if (iRes != APP_SUCCESS) {
			continue;
		}

		// then update the color, and x,y positions
		LayerColor[LayerCount] = GetPrivateProfileInt(AppName, L"LayerColor", 1, Filename);

		LayerX[LayerCount] = GetPrivateProfileInt(AppName, L"LayerX", 0, Filename);
		LayerY[LayerCount] = GetPrivateProfileInt(AppName, L"LayerY", 0, Filename);

		Enabled[LayerCount] = GetPrivateProfileInt(AppName, L"Enabled", 1, Filename);

		LayerCount++;
	}

	wcscpy_s(ConfigurationFile, MAX_PATH, Filename);

	if (LayerCount != TotalLayers) {
		// failed to load all layers
		return APPERR_FILESIZE;
	}
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  int Layers::GetSize(int Layer, int* x, int* y)
// 
// This returns the image size for the specified layer
//
//*******************************************************************************
int Layers::GetSize(int Layer, int* x, int* y) {
	if (Layer < 0 || Layer >= NumLayers) {
		return APPERR_PARAMETER;
	}
	*x = LayerXsize[Layer];
	*y = LayerYsize[Layer];

	return APP_SUCCESS;
};

//*******************************************************************************
//
//  int Layers::GetLocation(int Layer, int* x, int* y)
// 
// This returns the image location for the specified layer
//
//*******************************************************************************
int Layers::GetLocation(int Layer, int* x, int* y) {
	if (Layer < 0 || Layer >= NumLayers) {
		return APPERR_PARAMETER;
	}

	*x = LayerX[Layer];
	*y = LayerY[Layer];

	return APP_SUCCESS;
};

//*******************************************************************************
//
//  int Layers::SetLocation(int Layer, int* x, int* y)
// 
// This returns the image location for the specified layer
//
//*******************************************************************************
int Layers::SetLocation(int Layer, int x, int y) {
	if (Layer < 0 || Layer >= NumLayers) {
		return APPERR_PARAMETER;
	}
	
	LayerX[Layer] = x;
	LayerY[Layer] = y;

	return APP_SUCCESS;
}

//*******************************************************************************
//
//  int Layers::GetNumLayers(void)
// 
// This returns the current layer number
//
//*******************************************************************************
int Layers::GetNumLayers(void) {
	return NumLayers;
};

//*******************************************************************************
//
//  int Layers::SetCurrentLayer(int Layer)
// 
// Set the current layer
//
//*******************************************************************************
int Layers::SetCurrentLayer(int Layer) {
	if (Layer < 0 || Layer >= NumLayers) {
		return APPERR_PARAMETER;
	}

	CurrentLayer = Layer;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  int Layers::GetCurrentLayer(void)
// 
// This returns the current layer
//
//*******************************************************************************
int Layers::GetCurrentLayer(void) {
	if (NumLayers <= 0) {
		return -1;
	}

	return CurrentLayer;
};

//*******************************************************************************
//
//  int Layers::GetBackgroundColor(void)
// 
// This returns the current layer
//
//*******************************************************************************
COLORREF Layers::GetBackgroundColor(void) {
	return rgbBackgroundColor;
};

//*******************************************************************************
//
//  int Layers::SetBackgroundColor(void)
// 
// This returns the current layer
//
//*******************************************************************************
void Layers::SetBackgroundColor(COLORREF Color) {
	rgbBackgroundColor = Color;
};

//*******************************************************************************
//
//  int Layers::GetBackgroundColor(void)
// 
// This returns the current layer
//
//*******************************************************************************
COLORREF Layers::GetDefaultLayerColor(void) {
	return rgbDefaultLayerColor;
};

//*******************************************************************************
//
//  int Layers::SetBackgroundColor(void)
// 
// This returns the current layer
//
//*******************************************************************************
void Layers::SetDefaultLayerColor(COLORREF Color) {
	rgbDefaultLayerColor = Color;
};

//*******************************************************************************
//
//  int Layers::GetOverlayColor(void)
// 
// This returns the current layer
//
//*******************************************************************************
COLORREF Layers::GetOverlayColor(void) {
	return rgbOverlayColor;
};

//*******************************************************************************
//
//  int Layers::SetOverlayColor(void)
// 
// This returns the current layer
//
//*******************************************************************************
void Layers::SetOverlayColor(COLORREF Color) {
	rgbOverlayColor = Color;
};

//*******************************************************************************
//
//  int Layers::GetLayerColor(void)
// 
// This returns the current layer
//
//*******************************************************************************
COLORREF Layers::GetLayerColor(int Layer) {
	return LayerColor[Layer];
};

//*******************************************************************************
//
//  int Layers::GetLayerColor(void)
// 
// This returns the current layer
//
//*******************************************************************************
int Layers::SetLayerColor(int Layer, COLORREF Color) {
	if (Layer < 0 || Layer >= NumLayers) {
		return APPERR_PARAMETER;
	}

	LayerColor[Layer] = Color;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  DisableLayer(int Layer)
// 
// This disables the layers in the overlay image
//
//*******************************************************************************
int Layers::DisableLayer(int Layer) {
	if (Layer < 0 || Layer >= NumLayers) {
		return APPERR_PARAMETER;
	}
	Enabled[Layer] = FALSE;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  EnableLayer(int Layer)
// 
// This disables the layers in the overlay image
//
//*******************************************************************************
int Layers::EnableLayer(int Layer) {
	if (Layer < 0 || Layer >= NumLayers) {
		return APPERR_PARAMETER;
	}
	Enabled[Layer] = TRUE;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  IsLayerEnabled(int Layer)
// 
// This disables the layers in the overlay image
//
//*******************************************************************************
BOOL Layers::IsLayerEnabled(int Layer) {
	if (Layer < 0 || Layer >= NumLayers) {
		return FALSE;
	}
	
	return Enabled[Layer];
};

//*******************************************************************************
//
//  int Layers::SaveBMP(WCHAR* Filename)
// 
// This returns the current layer
//
//*******************************************************************************
int Layers::SaveBMP(WCHAR* Filename) {
	if (wcslen(Filename) == 0) {
		return APPERR_PARAMETER;
	}
	if (OverlayImage == NULL) {
		return APPERR_PARAMETER;
	}

	int iRes;
	iRes = SaveImageBMP(Filename, OverlayImage, ImageXextent, ImageYextent);

	return iRes;
}

//*******************************************************************************
//
//  GetMinOverlaySize()
//	Class constructor
// 
//*******************************************************************************
void Layers::GetMinOverlaySize(int*x, int* y) {
	*x = minOverlaySizeX;
	*y = minOverlaySizeY;
};

//*******************************************************************************
//
//  SetMinOverlaySize()
//	Class constructor
// 
//*******************************************************************************
void Layers::SetMinOverlaySize(int x, int y) {
	minOverlaySizeX = x;
	minOverlaySizeY = y;
};

//*******************************************************************************
//
//  int GetOverlayImage(COLORREF* OverlayImage, int* xsize, int* ysize)
// 
// parameter returns:
// 
// COLORREF* OverlayImage	pointer to the overlay image
// int* xsize				x size of Overlay image
// int* ysize				y size of Overlay image
//
// function return:
// APP_SUCCESS				valid Overlay image
// APPERR_PARAMETER			The Overlay image is not valid, parameter returns
//							should not be used.
// 
//*******************************************************************************
int Layers::GetOverlayImage(COLORREF** Image, int* xsize, int* ysize)
{
	if (OverlayImage == NULL ||
		ImageXextent <= 0 || ImageYextent <= 0||
		!OverlayValid) {

		*Image = NULL;
		*xsize = 0;
		*ysize = 0;
		return APPERR_PARAMETER;
	}

	*Image = OverlayImage;
	*xsize = ImageXextent;
	*ysize = ImageYextent;
	return APP_SUCCESS;
}


