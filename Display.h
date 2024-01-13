#pragma once
//
// MySETIviewer, a set tools for decoding bitstreams into various formats and manipulating those files
// Display.cpp
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
// V0.2.0.1 2023-12-08  Added the Display class to provide the on screen display without the use of
//                      a BMP file viewer.
//
// Application standardized error numbers for functions:
//		See AppErrors.h
//
//	This is the Display class for handling the formatting, display, scaling of the overlayed bitmap
//

class Display {
public:
	Display();
	~Display();

	int GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor);
	int SetColors(COLORREF BackgroundColor, COLORREF GapMajorColor, COLORREF GapMinorColor);
	int SetBackgroundColor(COLORREF BackgroundColor);
	int SetGapMajorColor(COLORREF GapMajorColor);
	int SetGapMinorColor(COLORREF GapMinorColor);
	COLORREF GetBackgroundColor(void);
	COLORREF GetGapMajorColor(void);
	COLORREF GetGapMinorColor(void);

	int GetGridMajor(int* x, int* y);
	int GetGridMinor(int* x, int* y);
	int SetGridMajor(int x, int y);
	int SetGridMinor(int x, int y);

	int GetGapMajor(int* xwidth, int* ywidth);
	int GetGapMinor(int* xwidth, int* ywidth);
	int SetGapMajor(int xwidth, int ywidth);
	int SetGapMinor(int xwidth, int ywidth);

	int CreateDisplayImages(void);
	int ReleaseDisplayImages(void);
	int UpdateDisplay(COLORREF* OverlayImage, int xsize, int ysize);

	void CalculateDisplayExtent(int ImageXextent, int ImageYextent);
	int SaveBMP(WCHAR* Filename, int Select);

	void LoadConfiguration(WCHAR* szFilename);
	int SaveConfiguration(WCHAR* szFilename);

	int GetDisplay(COLORREF** Image, int* xsize, int* ysize);
	BOOL GetSize(int* x, int* y);
	BOOL IsGridEnabled(void);
	void EnableGrid(BOOL Enable);

private:
	// grid are on by default
	BOOL GridEnabled = TRUE;

	// color setting tables for Display
	COLORREF rgbBackground = 0;      // display background color
	COLORREF rgbGapMajor = 0;            // display Grid color
	COLORREF rgbGapMinor = 0;             // display Gap color

	// display image grid size
	int GridXmajor = 0;
	int GridYmajor = 0;
	int GridXminor = 0;
	int GridYminor = 0;

	// display gap size
	int GapXmajor = 0;
	int GapYmajor = 0;
	int GapXminor = 0;
	int GapYminor = 0;

	//size of the bitmap for the entire display
	int DisplayXextent = 0;
	int DisplayYextent = 0;

	// number of gaps major and minor
	int NumberMajorXgap = 0;
	int NumberMinorXgap = 0;
	int NumberMajorYgap = 0;
	int NumberMinorYgap = 0;

	COLORREF* DisplayReference = NULL;	// This contains the gridded reference image with gaps
										// This isused to facilitate processing of the Display Image
										// This is recreated when the grid, gap or display color changes

	COLORREF* DisplayImage = NULL;		// this the reference image merged with the Overlay image
										// This is what is acutally displayed.
										// This is updated whenever the Reference image changes or
										// the Overlay image changes.

	void CreateDisplayLine(int StartAddress,
		COLORREF rgbBackground,
		COLORREF rgbGapMajor,
		COLORREF rgbGapMinor);

};