#pragma once
//
// GLobal variables
//
#include "Layers.h"
#include "Display.h"
#include "ImageDialog.h"
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
extern BOOL AutoPNG;
extern BOOL KeepOpen;

// Application handles and instances
extern HWND hwndMain;
extern HWND hwndImage;
extern HWND hwndDisplay;
extern HWND hwndLayers;
extern HINSTANCE hInst;

extern Layers* ImageLayers;
extern Display* Displays;

// Display message handler
extern INT_PTR CALLBACK ImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

extern COLORREF CustomColorTable[16]; // array of custom colors

extern ImageDialog* ImgDlg;

