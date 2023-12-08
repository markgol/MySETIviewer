#pragma once
//
// GLobal variables
//
#include "Layers.h"
#include "AppErrors.h"

// Version info
extern CString strProductName;
extern CString strProductVersion;
extern CString strName;
extern CString strCopyright;

// exe and ini file locations
extern CString strAppNameEXE;
extern CString strAppNameINI;

// working filenames
extern WCHAR szBMPFilename[MAX_PATH];
extern WCHAR szCurrentFilename[MAX_PATH];
extern WCHAR szBMPFilename[MAX_PATH];
extern WCHAR szTempDir[MAX_PATH];

// globals flags
extern int DisplayResults;
extern int AutoScaleResults;
extern int DefaultRBG;
extern int AutoSize;
extern int AutoPNG;

// Application handles and instances
extern HWND hwndMain;
extern HWND hwndImage;
extern HINSTANCE hInst;

extern Layers* ImageLayers;

// Display message handler
extern INT_PTR CALLBACK ImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// color tables for display
extern COLORREF rgbBackground;        // display background color
extern COLORREF rgbGapMajor;              // display Grid color
extern COLORREF rgbGapMinor;               // display Gap color

extern COLORREF CustomColorTable[16]; // array of custom colors 

// display image grid size
extern int GridXmajor;
extern int GridYmajor;
extern int GridXminor;
extern int GridYminor;

// display gap size
extern int GapXmajor;
extern int GapYmajor;
extern int GapXminor;
extern int GapYminor;
