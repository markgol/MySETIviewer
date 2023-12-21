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
// V1.0.1.0	2023-12-20	Initial release
//
// Application standardized error numbers for functions:
//		See AppErrors.h
//
//	This is the Display class for handling the formatting, display, scaling of the overlayed bitmap
//
#include "framework.h"
#include <shtypes.h>
#include <string.h>
#include <stdio.h>
#include "AppErrors.h"
#include "imageheader.h"
#include "display.h"
#include "FileFunctions.h"

//*******************************************************************************
//
//  Display()
//  class constructor
// 
//*******************************************************************************
Display::Display()
{
	return;
}

//*******************************************************************************
//
//  ~Display()
//  class destructor
// 
//*******************************************************************************
Display::~Display()
{
    ReleaseDisplayImages();
	return;
}

//*******************************************************************************
//
//  CreateDisplayImage(void)
// 
//*******************************************************************************
int Display::CreateDisplayImages(void)
{
    if (DisplayXextent <= 0 || DisplayYextent <= 0) {
        return APPERR_PARAMETER;
    }

    ReleaseDisplayImages();

    int DisplaySize = DisplayXextent * DisplayYextent;

    DisplayImage = new COLORREF[DisplaySize];
    if (DisplayImage == NULL) {
        return APPERR_MEMALLOC;
    }

    DisplayReference = new COLORREF[DisplaySize];
    if (DisplayReference == NULL) {
        delete[] DisplayImage;
        DisplayImage = NULL;
        return APPERR_MEMALLOC;
    }

    // make sure the clusters are valid
    if (GridXmajor <= 0) GridXmajor = 1;
    if (GridYmajor <= 0) GridYmajor = 1;
    if (GridXminor <= 0) GridXminor = 1;
    if (GridYminor <= 0) GridYminor = 1;

    if (GapXmajor < 0) GapXmajor = 0;
    if (GapYmajor < 0) GapYmajor = 0;
    if (GapXminor < 0) GapXminor = 0;
    if (GapYminor < 0) GapYminor = 0;

    // there are four cases for the gaps when calculating the display extent
    // 
    // Gap  Major   Minor
    //      !=0     !=0
    //      ==0     !=0
    //      !=0     ==0
    //      ==0     ==0     
    //
    // The overall display is surrounded by a frame of MajorGap size

    int dAddress;
    int y = 0;
    if (GapYmajor != 0 && GapYminor != 0) {
        // starts with GapYmajor lines of rgbGapMajor
        for (int gap = 0; gap < GapYmajor && y < DisplayYextent; gap++, y++) {
            dAddress = y * DisplayXextent;
            // MajorGap line (just MajorGap colors1)
            CreateDisplayLine(dAddress, rgbGapMajor, rgbGapMajor, rgbGapMajor);
        }
        
        // then is goes in groups of GridYminorGridYGap lines of the image
        // followed by Gap
        while(y < DisplayYextent) {
            for (int i = 0; i < GridYminor && y < DisplayYextent; i++, y++) {
                dAddress = y * DisplayXextent;
                // regular line (all elements: Major,Minor,background for image)
                CreateDisplayLine(dAddress, rgbBackground, rgbGapMajor, rgbGapMinor);
            }

            // insert GridYminor lines of using X Grid/gap parameters

            // add Y major grid groups - a group of (minor gap Y image lines, minor grid Y lines)

            for (int Grid = 0; Grid < (GridYmajor - 1); Grid++) {
                // line of rgbGapMinor
                for (int gap = 0; gap < GapYminor && y < DisplayYextent; gap++, y++) {
                    dAddress = y * DisplayXextent;
                    // MinorGap line (just MajorGap, MinorGap colors)
                    CreateDisplayLine(dAddress, rgbGapMinor, rgbGapMajor, rgbGapMinor);
                }
                // insert GridYminor lines of using X Grid/gap parameters
                for (int i = 0; i < GridYminor && y < DisplayYextent; i++, y++) {
                    dAddress = y * DisplayXextent;
                    // regular line (all elements: Major,Minor,background for image)
                    CreateDisplayLine(dAddress, rgbBackground, rgbGapMajor, rgbGapMinor);
                }
            }

            // Add Major Gap lines
            for (int gap = 0; gap < GapYmajor && y < DisplayYextent; gap++, y++) {
                dAddress = y * DisplayXextent;
                // MajorGap line (just MajorGap colors1)
                CreateDisplayLine(dAddress, rgbGapMajor, rgbGapMajor, rgbGapMajor);
            }
        }
    }
    else if (GapYmajor == 0 && GapYminor != 0) {
        // no Major gap exists only Minor gaps

        while (y < DisplayYextent) {
            for (int i = 0; i < GridYminor && y < DisplayYextent; i++, y++) {
                dAddress = y * DisplayXextent;
                // regular line (all elements: Major,Minor,background for image)
                CreateDisplayLine(dAddress, rgbBackground, rgbGapMajor, rgbGapMinor);
            }


            // insert GridYminor lines of using X Grid/gap parameters

            // Add Minor Gap lines
            for (int gap = 0; gap < GapYminor && y < DisplayYextent; gap++, y++) {
                dAddress = y * DisplayXextent;
                // MinorGap line (just MajorGap, MinorGap colors)
                CreateDisplayLine(dAddress, rgbGapMinor, rgbGapMajor, rgbGapMinor);
            }
        }
    }
    else if (GapYmajor != 0 && GapYminor == 0) {
        // no Major gap exists only Minor gaps

            // Add Major Gap lines
        for (int gap = 0; gap < GapYmajor && y < DisplayYextent; gap++, y++) {
            dAddress = y * DisplayXextent;
            // MajorGap line (just MajorGap colors1)
            CreateDisplayLine(dAddress, rgbGapMajor, rgbGapMajor, rgbGapMajor);
        }

        while (y < DisplayYextent) {
            for (int i = 0; i < (GridYminor*GridYmajor) && y < DisplayYextent; i++, y++) {
                dAddress = y * DisplayXextent;
                // regular line (all elements: Major,Minor,background for image)
                CreateDisplayLine(dAddress, rgbBackground, rgbGapMajor, rgbGapMinor);
            }


            // insert GridYmajor lines of using X Grid/gap parameters

            // Add Major Gap lines
            for (int gap = 0; gap < GapYmajor && y < DisplayYextent; gap++, y++) {
                dAddress = y * DisplayXextent;
                // MajorGap line (just MajorGap colors1)
                CreateDisplayLine(dAddress, rgbGapMajor, rgbGapMajor, rgbGapMajor);
            }
        }
    }
    else {
        // 0,0 case just set the everything to Background image
        for (int i = 0; i < DisplaySize; i++) {
            DisplayReference[i] = rgbBackground;
        }
    }

    return APP_SUCCESS;
}

//*******************************************************************************
//
//  void CreateDisplayLine(COLORREF* dAddress)
// 
//  Helper function for CreateDisplayImage()
// 
//*******************************************************************************
void Display::CreateDisplayLine(int StartAddress,
    COLORREF rgbBackground,
    COLORREF rgbGapMajor,
    COLORREF rgbGapMinor)
{
    // there are four cases for the gaps when calculating the display extent
    // 
    // Gap  Major   Minor
    //      !=0     !=0
    //      ==0     !=0
    //      !=0     ==0
    //      ==0     ==0     
    //
    // The overall display is surrounded by a frame of MajorGap size

    int x = 0;
    if (GapXmajor != 0 && GapXminor != 0) {
        // starts with GapXmajor pixels of rgbGapMajor
        for (int gap = 0; gap < GapXmajor && x < DisplayXextent; gap++, x++) {
            // pixels of rgbGapMajor
            DisplayReference[StartAddress + x] = rgbGapMajor;
        }

        // then is goes in groups of GridXminor,GridXGap pixels of the image
        // followed by Gap
        while (x < DisplayXextent) {
            for (int i = 0; i < GridXminor && x < DisplayXextent; i++, x++) {
                DisplayReference[StartAddress + x] = rgbBackground;
            }

            // insert GridXminor pixels of using X Grid/gap parameters

            // add Y major grid groups - a group of (minor gap Y image lines, minor grid Y lines)

            for (int Grid = 0; Grid < (GridXmajor - 1); Grid++) {
                // Pixels(s) of rgbGapMajor
                for (int gap = 0; gap < GapXminor && x < DisplayXextent; gap++, x++) {
                    DisplayReference[StartAddress + x] = rgbGapMinor;
                }
                // insert GridXminor pixel(s) of using X Grid/gap parameters
                for (int i = 0; i < GridXminor && x < DisplayXextent; i++, x++) {
                    DisplayReference[StartAddress + x] = rgbBackground;
                }
            }

            // Add Major Gap pixels
            for (int gap = 0; gap < GapXmajor && x < DisplayXextent; gap++, x++) {
                DisplayReference[StartAddress + x] = rgbGapMajor;
            }
        }
    }
    else if (GapXmajor == 0 && GapXminor != 0) {
        // no Major gap exists only Minor gaps

        while (x < DisplayXextent) {
            for (int i = 0; i < GridXminor && x < DisplayXextent; i++, x++) {
                DisplayReference[StartAddress + x] = rgbBackground;
            }

            // insert GridYminor lines of using X Grid/gap parameters

            // Add Minor Gap lines
            for (int gap = 0; gap < GapXminor && x < DisplayXextent; gap++, x++) {
                DisplayReference[StartAddress + x] = rgbGapMinor;
            }
        }
    }
    else if (GapXmajor != 0 && GapXminor == 0) {
        // Major gap > 0, no minor gaps

        // Add Major Gap lines
        for (int gap = 0; gap < GapXmajor && x < DisplayXextent; gap++, x++) {
            DisplayReference[StartAddress + x] = rgbGapMajor;
        }

        while (x < DisplayXextent) {
            for (int i = 0; i < (GridXminor*GridXmajor) && x < DisplayXextent; i++, x++) {
                DisplayReference[StartAddress + x] = rgbBackground;
            }

            // insert GridYmajor lines of using X Grid/gap parameters

            for (int gap = 0; gap < GapXmajor && x < DisplayXextent; gap++, x++) {
                DisplayReference[StartAddress + x] = rgbGapMajor;
            }
        }
    }
    // 0,0 case has nothing to do

    return;
}

//*******************************************************************************
//
//  UpdateDisplay(COLORREF*, int xsize, int ysize)
// 
//*******************************************************************************
int Display::UpdateDisplay(COLORREF* OverlayImage, int xsize, int ysize)
{
    int ix=0, iy=0;
    int Daddress;
    int Iaddress;
    BOOL Found = FALSE;

    for (int dy = 0; dy <DisplayYextent; dy++) {
        Daddress = dy * DisplayXextent;
        Iaddress = iy * xsize;
        Found = FALSE;
        ix = 0;
        for (int dx = 0; dx < DisplayXextent; dx++) {
            if (DisplayReference[Daddress + dx] == rgbBackground &&
                    ix<xsize && iy<ysize) {
                DisplayImage[Daddress + dx] = OverlayImage[Iaddress + ix];
                ix++;
                Found = TRUE;
            }
            else {
                DisplayImage[Daddress + dx] = DisplayReference[Daddress + dx];
            }
        }
        if (Found) {
            iy++;
        }
    }

    return APP_SUCCESS;
}

//*******************************************************************************
//
//  ReleaseDisplayImages(void)
// 
//*******************************************************************************
int Display::ReleaseDisplayImages(void) {
    if (DisplayImage != NULL) {
        delete[] DisplayImage;
        DisplayImage = NULL;
    }
    if (DisplayReference != NULL) {
        delete[] DisplayReference;
        DisplayReference = NULL;
    }
    return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
{
	*BackgroundColor = rgbBackground;      // display background color
	*GapMajorColor = rgbGapMajor;            // display Grid color
	*GapMinorColor = rgbGapMinor;             // display Gap color
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::SetColors(COLORREF BackgroundColor, COLORREF GapMajorColor, COLORREF GapMinorColor)
{
	rgbBackground = BackgroundColor;      // display background color
	rgbGapMajor = GapMajorColor;            // display Grid color
	rgbGapMinor = GapMinorColor;             // display Gap color
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::SetBackgroundColor(COLORREF BackgroundColor)
{
	rgbBackground = BackgroundColor;      // display background color
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::SetGapMajorColor(COLORREF GapMajorColor)
{
	rgbGapMajor = GapMajorColor;            // display Grid color
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::SetGapMinorColor(COLORREF GapMinorColor)
{
	rgbGapMinor = GapMinorColor;             // display Gap color
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
COLORREF Display::GetBackgroundColor(void)
{
	return rgbBackground;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
COLORREF Display::GetGapMajorColor(void)
{
	return rgbGapMajor;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
COLORREF Display::GetGapMinorColor(void)
{
	return rgbGapMinor;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::GetGridMajor(int* x, int* y)
{
	*x = GridXmajor;
	*y = GridYmajor;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::GetGridMinor(int* x, int* y)
{
	*x = GridXminor;
	*y = GridYminor;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::SetGridMajor(int x, int y)
{
	GridXmajor = x;
	GridYmajor = y;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::SetGridMinor(int x, int y) {
	GridXminor = x;
	GridYminor = y;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::GetGapMajor(int* xwidth, int* ywidth)
{
	*xwidth = GapXmajor;
	*ywidth = GapYmajor;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::GetGapMinor(int* xwidth, int* ywidth) {
	*xwidth = GapXminor;
	*ywidth = GapYminor;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::SetGapMajor(int xwidth, int ywidth) {
	GapXmajor = xwidth;
	GapYmajor = ywidth;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
int Display::SetGapMinor(int xwidth, int ywidth) {
	GapXminor = xwidth;
	GapYminor = ywidth;
	return APP_SUCCESS;
};

//*******************************************************************************
//
//  GetColors(COLORREF* BackgroundColor, COLORREF* GapMajorColor, COLORREF* GapMinorColor)
// 
//*******************************************************************************
void Display::CalculateDisplayExtent(int ImageXextent, int ImageYextent) {
 
    // make sure the clusters are valid
    if (GridXmajor <= 0) GridXmajor = 1;
    if (GridYmajor <= 0) GridYmajor = 1;
    if (GridXminor <= 0) GridXminor = 1;
    if (GridYminor <= 0) GridYminor = 1;

    if (GapXmajor < 0) GapXmajor = 0;
    if (GapYmajor < 0) GapYmajor = 0;
    if (GapXminor < 0) GapXminor = 0;
    if (GapYminor < 0) GapYminor = 0;

    // change ImageExtent to make sure clusters can be accomodated.
    // This is only used in this calculation, it does not actually change
    // the Overlay image extent.
    // This may result in extra space on the right and bottom of the
    // displayed image that doesn't have image overlay data.

    // ImageXextent adjusted to be multiple of GridXmajor*GridXminor
    if (ImageXextent % (GridXmajor * GridXminor) != 0) {
        // adjust ImageExtent size
        ImageXextent = ImageXextent + ((GridXmajor * GridXminor) - (ImageXextent % (GridXmajor * GridXminor)));
    }
    // ImageYextent adjusted to be multiple of GridYmajor*GridYminor
    if (ImageYextent % (GridYmajor * GridYminor) != 0) {
        // adjust ImageExtent size
        ImageYextent = ImageYextent + ((GridYmajor * GridYminor) - (ImageYextent % (GridYmajor * GridYminor)));
    }

    // there are four cases for the gaps when calculating the display extent
    // 
    // Gap  Major   Minor
    //      ==0     ==0
    //      ==0     !=0
    //      !=0     ==0
    //      !=0     !=0
    //
    // The overall display is surrounded by a frame of MajorGap size
    //
    if (GapYmajor == 0 && GapYminor == 0) {
        NumberMajorYgap = 0;
        NumberMinorYgap = 0;
        DisplayYextent = ImageYextent;
    }
    else if (GapYmajor == 0 && GapYminor != 0) {
        NumberMajorYgap = 0;
        NumberMinorYgap = (ImageYextent/GridYminor)-1;
        DisplayYextent = ImageYextent + NumberMinorYgap * GapYminor;
    }
    else if (GapYmajor != 0 && GapYminor == 0) {
        NumberMajorYgap = (ImageYextent / (GridYmajor*GridYminor)) + 1;
        NumberMinorYgap = 0;
        DisplayYextent = ImageYextent + NumberMajorYgap * GapYmajor;
    }
    else { // GapYmajor != 0 && GapYminor != 0
        NumberMajorYgap = (ImageYextent / (GridYmajor * GridYminor)) + 1;
        NumberMinorYgap = (GridYmajor-1) * (NumberMajorYgap-1);
        DisplayYextent = ImageYextent + 
                        (NumberMajorYgap * GapYmajor) +
                        (NumberMinorYgap * GapYminor);
    }

    if (GapXmajor == 0 && GapXminor == 0) {
        NumberMajorXgap = 0;
        NumberMinorXgap = 0;
        DisplayXextent = ImageXextent;
    }
    else if (GapXmajor == 0 && GapXminor != 0) {
        NumberMajorXgap = 0;
        NumberMinorXgap = (ImageXextent / GridXminor) - 1;
        DisplayXextent = ImageXextent + NumberMinorXgap * GapXminor;
    }
    else if (GapXmajor != 0 && GapXminor == 0) {
        NumberMajorXgap = (ImageXextent / (GridXmajor * GridXminor)) + 1;
        NumberMinorXgap = 0;
        DisplayXextent = ImageXextent + NumberMajorXgap * GapXmajor;
    }
    else { // GapYmajor != 0 && GapYminor != 0
        NumberMajorXgap = (ImageXextent / (GridXmajor * GridXminor)) + 1;
        NumberMinorXgap = (GridXmajor - 1) * (NumberMajorXgap - 1);
        DisplayXextent = ImageXextent +
            (NumberMajorXgap * GapXmajor) +
            (NumberMinorXgap * GapXminor);
    }

    return;
}

//*******************************************************************************
//
//  int SaveBMP(WCHAR* Filename)
// 
// This returns the current layer
//
//*******************************************************************************
int Display::SaveBMP(WCHAR* Filename, int Select) {
    if (wcslen(Filename) == 0) {
        return APPERR_PARAMETER;
    }
    int iRes;
    if (Select == 0) {
        if (DisplayImage == NULL) {
            return APPERR_PARAMETER;
        }
        iRes = SaveImageBMP(Filename, DisplayImage, DisplayXextent, DisplayYextent);
    } else {
        if (DisplayReference == NULL) {
            return APPERR_PARAMETER;
        }
        iRes = SaveImageBMP(Filename, DisplayReference, DisplayXextent, DisplayYextent);
    }
    return iRes;
}

//*******************************************************************************
//
//  int LoadConfiguration(WCHAR* szFilename)
// 
// This returns the current layer
//
//*******************************************************************************
void Display::LoadConfiguration(WCHAR* szFilename)
{
    int x, y;
    COLORREF Color;

    x = (int) GetPrivateProfileInt(L"Display", L"GridXmajor",1, (LPCTSTR)szFilename);
    y = (int) GetPrivateProfileInt(L"Display", L"GridYmajor",1, (LPCTSTR)szFilename);
    SetGridMajor(x, y);

    x = (int) GetPrivateProfileInt(L"Display", L"GridXminor", 1, (LPCTSTR)szFilename);
    y = (int) GetPrivateProfileInt(L"Display", L"GridYminor", 1, (LPCTSTR)szFilename);
    SetGridMinor(x, y);

    x = (int) GetPrivateProfileInt(L"Display", L"GapXmajor", 0, (LPCTSTR)szFilename);
    y = (int) GetPrivateProfileInt(L"Display", L"GapYmajor", 0, (LPCTSTR)szFilename);
    SetGapMajor(x, y);

    x = (int)GetPrivateProfileInt(L"Display", L"GapXminor", 0, (LPCTSTR)szFilename);
    y = (int)GetPrivateProfileInt(L"Display", L"GapYminor", 0, (LPCTSTR)szFilename);
    SetGapMinor(x, y);

    Color = GetPrivateProfileInt(L"Display", L"rgbBackground", 10, (LPCTSTR)szFilename);
    SetBackgroundColor(Color);

    Color = GetPrivateProfileInt(L"Display", L"rgbGapMajor", 30, (LPCTSTR)szFilename);
    SetGapMajorColor(Color);

    Color = GetPrivateProfileInt(L"Display", L"rgbGapMinor", 20, (LPCTSTR)szFilename);
    SetGapMinorColor(Color);

    return;
}

//*******************************************************************************
//
//  SaveConfiguration(WCHAR* szFilename)
// 
// This returns the current layer
//
//*******************************************************************************
int Display::SaveConfiguration(WCHAR* szFilename)
{
    int x, y;
    COLORREF Color;
    WCHAR szString[40];

    GetGridMajor(&x, &y);
    swprintf_s(szString, 40, L"%d", x);
    WritePrivateProfileString(L"Display", L"GridXmajor", szString, (LPCTSTR)szFilename);
    swprintf_s(szString, 40, L"%d", y);
    WritePrivateProfileString(L"Display", L"GridYmajor", szString, (LPCTSTR)szFilename);

    GetGridMinor(&x, &y);
    swprintf_s(szString, 40, L"%d", x);
    WritePrivateProfileString(L"Display", L"GridXminor", szString, (LPCTSTR)szFilename);
    swprintf_s(szString, 40, L"%d", y);
    WritePrivateProfileString(L"Display", L"GridYminor", szString, (LPCTSTR)szFilename);

    GetGapMajor(&x, &y);
    swprintf_s(szString, 40, L"%d", x);
    WritePrivateProfileString(L"Display", L"GapXmajor", szString, (LPCTSTR)szFilename);
    swprintf_s(szString, 40, L"%d", y);
    WritePrivateProfileString(L"Display", L"GapYmajor", szString, (LPCTSTR)szFilename);

    GetGapMinor(&x, &y);
    swprintf_s(szString, 40, L"%d", x);
    WritePrivateProfileString(L"Display", L"GapXminor", szString, (LPCTSTR)szFilename);
    swprintf_s(szString, 40, L"%d", y);
    WritePrivateProfileString(L"Display", L"GapYminor", szString, (LPCTSTR)szFilename);

    Color = GetBackgroundColor();
    swprintf_s(szString, 40, L"%u", Color);
    WritePrivateProfileString(L"Display", L"rgbBackground", szString, (LPCTSTR)szFilename);

    Color = GetGapMajorColor();
    swprintf_s(szString, 40, L"%u", Color);
    WritePrivateProfileString(L"Display", L"rgbGapMajor", szString, (LPCTSTR)szFilename);

    Color = GetGapMinorColor();
    swprintf_s(szString, 40, L"%u", Color);
    WritePrivateProfileString(L"Display", L"rgbGapMinor", szString, (LPCTSTR)szFilename);

    return APP_SUCCESS;
}

//*******************************************************************************
//
//  int GetDisplay(COLORREF** Image, int* xsize, int* ysize)
//
//*******************************************************************************
int Display::GetDisplay(COLORREF** Image, int* xsize, int* ysize)
{
    *Image = DisplayImage;
    *xsize = DisplayXextent;
    *ysize = DisplayYextent;

    return APP_SUCCESS;
}

//*******************************************************************************
//
//  BOOL Display::GetSize(int* x, int* y)
//
//*******************************************************************************
BOOL Display::Display::GetSize(int* x, int* y)
{
    if (DisplayYextent == 0) {
        *x = 0;
        *y = 0;
        return FALSE;
    }

    *x = DisplayXextent;
    *y = DisplayYextent;
    return TRUE;
}

