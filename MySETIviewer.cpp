//
// MySETIviewer, an application for viewing, overlaying and manipulating images for display
// relative to eath other for the purpose of examining images generated by MySETIapp
// MySETIviewer.cpp
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
// Background:
// From the 'A Sign in Space' website, https://asignin.space/:
// A Sign in Space is an interdisciplinary project by media artist Daniela de Paulis,
// in collaboration with the SETI Institute, the European Space Agency,
// the Green Bank Observatory and INAF, the Italian National Institute for Astrophysics.
// The project consists in transmitting a simulated extraterrestrial message as part
// of a live performance, using an ESA spacecraft as celestial source.The objective
// of the project is to involve the world - wide Search for Extraterrestrial
// Intelligence community, professionals from different fieldsand the broader public
// in the reception, decodingand interpretation of the message.This process will
// require global cooperation, bridging a conversation around the topics of SETI,
// space researchand society, across multiple culturesand fields of expertise.
// https://www.seti.org/event/sign-space
// 
// The message was transmitted from the ESA's ExoMars Trace Gas Orbiter (TGO)
//   on May 24 at 19:16 UTC/12:15 pm PDT.
// 
// It was received by three radio telescopes on earth May 24,2023.
// A group of individuals in the Discord 'A Sign in Space' channel
// unscrambled the message from the radio telemetry.
// The message published as Data17.bin is the correctly transcribed
// bitstream of the message payload given to ESA.
// 
// The next step in the problem is the decoding of the payload bitstream into
// the next level of the message.
// 
// The MySETI programs is a set of tools to help in this process.
// This conssists of:
//      MySETIapp    Addresses various messgae extraction and image manipultation
//                   methods the have been tried or are being tried for the purpose
//                   of fruther decoding the 'A Sign In Space' message
//      MySETIviewer Viewer program to load the image files (.raw, .bmp(8 bit)) that the MySETIapp   
//                   application generates.  It allows the overlay, alignment, and
//                   display of theses file with an interactrive GUI.
// 
// Future expansion:
//      Change Layers, display dialog to modeless to support better live interaction
//      Add .bin files, binary file used on Discord channel
//      Add overlay shapes, rectangle, circle, line, text to annotate image
// 
// The MySETIviewer covers many of the problems people have had in the Discord group visualizing
// the image data being generated in the decoding process.
//
// V1.0.1	2023-12-20	Initial release
// V1.0.2   2023-12-20  Added Y direction flag for which direction to move image
//						Changed color mixing formula when pixels are overlapped.
// V1.1.0   2023-12-20  Changed the Layer dialog to be modeless
//                      Moved Add layer to Layers dialog
//                      Deleted Remove Layer from menu 
//                      Correction, do not save anything when there are no layers
// V1.1.1   2023-12-27  Fixed parameter in IDC_BMP_GENERATE when x,y sizes are the same
//                      Added, status bar to Image window
//                      Added, bitmap position to status bar
//                      Changed, zoom, pan behaviour of bitmap
//                      Changed window resize of image display
//                      Added reset window positions on restart
// V1.1.2   2024-01-07  Added option to turn the grid on or off
//                      Added window position reset
//                      Added Action menu to Image Window
//                          Reset Pan
//                          Reset Zoom
//                          Close
//                      Changed Image Window, does not close with ESC or Enter keys
// 
//  This appliction stores user parameters in a Windows style .ini file
//  The MySETIviewer.ini file must be in the same directory as the exectable
//
//  A lot of this application code is really to support the framework
//  of the application for the user interface.
// 
//  The application code that does the image overlays and formatting
//  are implemented in the Display class and the Layers class:
//  Display.h,Display.cpp, Layers.h, Layers.cpp
// 
//  Everything else is support code
//
#include "framework.h"
#include <atlstr.h>
#include <strsafe.h>
#include <atlpath.h>
#include <string.h>
#include "AppErrors.h"
#include "MySETIviewer.h"
#include "Layers.h"
#include "Display.h"
#include "ImageDialog.h"
#include "AppFunctions.h"
#include "imageheader.h"
#include "FileFunctions.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// working filenames
WCHAR szBMPFilename[MAX_PATH] = L"";    // used to save last results file
WCHAR szCurrentFilename[MAX_PATH] = L"";    // used to save last results file
WCHAR szTempDir[MAX_PATH] = L"";        // used as a temporary storage location

// global Class declarations
Layers* ImageLayers = NULL;     // This class loads the image layers, combines them into the Overlay image
Display* Displays = NULL;       // This class creates a refernce image based on the display format paraneters
                                // and then inserts the Overlay image to create the Dislay image
ImageDialog* ImgDlg = NULL;     // This class is used to support displaying the Display image in a window
                                // on the desktop.  This also includes scaling and panning of the displayed image

// global flags
BOOL AutoPNG = FALSE;                // generate a PNG file when a BMP file is saved
BOOL KeepOpen = TRUE;           // keep Display and Layers dialog open when clicking OK or Cancel
BOOL ShowStatusBar = TRUE;     // Display Status bar

                               // handles for modeless dialogs and windows
HWND hwndImage = NULL;   // Handle for modeless Image Dialog window (this displays the image in a window)
HWND hwndDisplay = NULL; // Handle for the Display dialog (sets the grid and gap, and
                         //    creates the Display image used in the Image Dialog
HWND hwndLayers = NULL;  // Handle for the Layers dialog, this creates the Overlayed image from
                         //    the specified layer, colors amd position,  the overlay image is
                         //    then used in the Display dialog

// color picker dialog custom color list
COLORREF CustomColorTable[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };    // array of custom colors

// The following is from the version information in the resource file
CString strProductName;
CString strProductVersion;
CString strName;
CString strCopyright;
CString strAppNameEXE;
CString strAppNameINI;

// handle to the main application window
//  needed to do things like check or uncheck a menu item in the main app
HWND hwndMain = NULL;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// Declaration for callback dialog procedures in other modules
INT_PTR CALLBACK    AboutDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SettingsDisplayDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SettingsLayersDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SettingsGlobalDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Text2StreamDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    BitImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

//*******************************************************************************
//
// int APIENTRY wWinMain
//
//*******************************************************************************
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    // 
    // Get version information, executable name including path to executable location
    if (!GetProductAndVersion(&strProductName,
        &strProductVersion,
        &strName,
        &strCopyright,
        &strAppNameEXE))
        return FALSE;

    // create INI file name for application settings
    // The INI file has the same name as the executable.
    // It is located in the same directory as the exectable
    CPath path(strAppNameEXE);
    path.RenameExtension(_T(".ini"));
    strAppNameINI.SetString(path);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MYSETIVIEWER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYSETIVIEWER));

    MSG msg;
    BOOL bRet;

    // Main message loop:
    while ((bRet = GetMessage(&msg, nullptr, 0, 0))!=0)
    {
        if (bRet == -1) {
            // error handling if required
        }
        else {
            // Modeless dialog need to process messages intended for the modeless dialog only.
            // Check that message is not for each modeless dialog box.
            
            // Modeless Image Dialog window
            if (IsWindow(hwndImage) && IsDialogMessage(hwndImage, &msg)) {
                continue;
            }

            // Modeless Display window
            if (IsWindow(hwndDisplay) && IsDialogMessage(hwndDisplay, &msg)) {
                continue;
            }

            // Modeless Layer window ( not yet implemented )
            if (IsWindow(hwndLayers) && IsDialogMessage(hwndLayers, &msg)) {
                continue;
            }

            // process all other applications messages
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    return (int) msg.wParam;
}


//*******************************************************************************
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.  This is not the same as a c++ class
//
//*******************************************************************************
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYSETIVIEWER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MYSETIVIEWER);
    wcex.lpszClassName  = szWindowClass;  // loaded from the resource file IDC_MYSETIVIEWER string
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MYSETIVIEWER));

    return RegisterClassExW(&wcex);
}

//*******************************************************************************
//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
//*******************************************************************************
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   // Just using default size and position since this will be overwritten
   // when restorewindow is called
   HWND hWnd = CreateWindowW(szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, // initial x,y position
        CW_USEDEFAULT, 0, // initial x,ysize
        nullptr, nullptr,
        hInstance, nullptr);

   if (!hWnd)
   {
       // if can not create window just die
      return FALSE;
   }

   int ResetWindows = GetPrivateProfileInt(L"GlobalSettings", L"ResetWindows", 0, (LPCTSTR)strAppNameINI);

   // restore main window position from last execution
   if (!ResetWindows) {
       CString csString = L"MainWindow";
       RestoreWindowPlacement(hWnd, csString);
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // load globals

   // this must be done before ImageDialog class created
   ShowStatusBar = GetPrivateProfileInt(L"SettingsGlobalDlg", L"ShowStatusBar", 1, (LPCTSTR)strAppNameINI);

   hwndMain = hWnd;
   ImgDlg = new ImageDialog;
   ImageLayers = new Layers;
   Displays = new Display;

   // create display window
   hwndImage = CreateDialog(hInst, MAKEINTRESOURCE(IDD_IMAGE), hwndMain, ImageDlg);

   // strings
   WCHAR szString[MAX_PATH];

   GetPrivateProfileString(L"SettingsGlobalDlg", L"TempDir", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
   wcscpy_s(szTempDir, szString);

   GetPrivateProfileString(L"SettingsGlobalDlg", L"CurrentFIlename", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
   wcscpy_s(szCurrentFilename, szString);

   // variables
   AutoPNG = GetPrivateProfileInt(L"SettingsGlobalDlg", L"AutoPNG", 1, (LPCTSTR)strAppNameINI);
   KeepOpen = GetPrivateProfileInt(L"SettingsGlobalDlg", L"KeepOpen", 0, (LPCTSTR)strAppNameINI);
   int ydir = GetPrivateProfileInt(L"SettingsGlobalDlg", L"yposDir", 1, (LPCTSTR)strAppNameINI);
   ImageLayers->SetYdir(ydir);

   for (int i = 0; i < 16; i++) {
       WCHAR CustomColor[20];
       swprintf_s(CustomColor, 20, L"CustomColorTable%d", i);
       CustomColorTable[i] = (COLORREF) GetPrivateProfileInt(L"SettingsDlg", CustomColor, 0, (LPCTSTR)strAppNameINI);
   }

   // These are Display class settings
   COLORREF BackGroundColor = (COLORREF)GetPrivateProfileInt(L"SettingsDisplayDlg", L"rgbBackground", 131586, (LPCTSTR)strAppNameINI);
   COLORREF GapMajorColor = (COLORREF)GetPrivateProfileInt(L"SettingsDisplayDlg", L"rgbGapMajor", 2621521, (LPCTSTR)strAppNameINI);
   COLORREF GapMinorColor = (COLORREF)GetPrivateProfileInt(L"SettingsDisplayDlg", L"rgbGapMinor", 1973790, (LPCTSTR)strAppNameINI);
   Displays->SetColors(BackGroundColor, GapMajorColor, GapMinorColor);

   int ValueX, ValueY;
   ValueX = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GridXmajor", 8, (LPCTSTR)strAppNameINI);
   ValueY = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GridYmajor", 8, (LPCTSTR)strAppNameINI);
   Displays->SetGridMajor(ValueX, ValueY);

   ValueX = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GridXminor", 4, (LPCTSTR)strAppNameINI);
   ValueY = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GridYminor", 4, (LPCTSTR)strAppNameINI);
   Displays->SetGridMinor(ValueX, ValueY);

   ValueX = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GapXmajor", 1, (LPCTSTR)strAppNameINI);
   ValueY = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GapYmajor", 1, (LPCTSTR)strAppNameINI);
   Displays->SetGapMajor(ValueX, ValueY);

   ValueX = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GapXminor", 1, (LPCTSTR)strAppNameINI);
   ValueY = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GapYminor", 1, (LPCTSTR)strAppNameINI);
   Displays->SetGapMinor(ValueX, ValueY);

   int Enable = GetPrivateProfileInt(L"SettingsDisplayDlg", L"EnableGrid", 1, (LPCTSTR)strAppNameINI);
   Displays->EnableGrid(Enable);

   // These are Layer class settings
   COLORREF Color;
   // This is the color of the overlay background anywhere it hasn't been replaced by image data
   Color = (COLORREF) GetPrivateProfileInt(L"LayersDlg", L"BackGround", 2763306, (LPCTSTR)strAppNameINI);
   ImageLayers->SetBackgroundColor(Color);
   Color = (COLORREF)GetPrivateProfileInt(L"LayersDlg", L"OverlayColor", 65793, (LPCTSTR)strAppNameINI);
   ImageLayers->SetOverlayColor(Color);
   Color = (COLORREF)GetPrivateProfileInt(L"LayersDlg", L"DefaultLayerColor", 16777215, (LPCTSTR)strAppNameINI);
   ImageLayers->SetDefaultLayerColor(Color);

   hwndDisplay = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SETTINGS_DISPLAY), hWnd, SettingsDisplayDlg);

   if (GetPrivateProfileInt(L"SettingsGlobalDlg", L"StartLast", 0, (LPCTSTR)strAppNameINI) != 0) {
       // load the last layer cinfiguration
       WCHAR szString[MAX_PATH];

       GetPrivateProfileString(L"GlobalSettings", L"LastConfigFile", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
       //wcscpy_s(ImageLayers->ConfigurationFile, MAX_PATH, szString);
       int iRes = ImageLayers->LoadConfiguration(szString);
       if (iRes != APP_SUCCESS) {
           wcscpy_s(ImageLayers->ConfigurationFile, MAX_PATH, L"");
       }
   }
   else {
       wcscpy_s(ImageLayers->ConfigurationFile, MAX_PATH, L"");
   }

   hwndLayers = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SETTINGS_LAYERS), hWnd, SettingsLayersDlg);

   // clear windows reset
   WritePrivateProfileString(L"GlobalSettings", L"ResetWindows", L"0", (LPCTSTR)strAppNameINI);
   return TRUE;
}

//*******************************************************************************
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
//*******************************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {

        case IDM_LOAD_CONFIGURATION:
        {
            int iRes;
            WCHAR szFilename[MAX_PATH];

            PWSTR pszFilename;
            COMDLG_FILTERSPEC AllType[] =
            {
                 { L"Image files", L"*.cfg" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileOpen(hWnd, ImageLayers->ConfigurationFile, &pszFilename, FALSE, 2, AllType, L".cfg")) {
                break;
            }

            wcscpy_s(szFilename, pszFilename);
            CoTaskMemFree(pszFilename);

            iRes = ImageLayers->LoadConfiguration(szFilename);
            if (iRes != APP_SUCCESS) {
                MessageBox(hWnd, L"Load configuration file failed\nCheck file for correct filenames in file", L"Layers", MB_OK);
                break;
            }
            Displays->LoadConfiguration(szFilename);
            SendMessage(hwndDisplay, WM_COMMAND, ID_UPDATE,0); // do not apply
            SendMessage(hwndLayers, WM_COMMAND, ID_UPDATE, 1); // apply 
            
            break;
        }

        case IDM_SAVE_CONFIGURATION:
        {
            int iRes;

            if (ImageLayers->GetNumLayers() == 0) {
                MessageBox(hWnd, L"No layers loaded", L"Layers", MB_OK);
                break;
            }

            if (wcslen(ImageLayers->ConfigurationFile) != 0) {
                iRes = ImageLayers->SaveConfiguration();
                iRes = Displays->SaveConfiguration(ImageLayers->ConfigurationFile);
                if (iRes == APP_SUCCESS) {
                    break;
                }
            }
            // go ahead with IDM_SAVE_AS_CONFIGURATION
        }
        case IDM_SAVE_AS_CONFIGURATION:
        {
            int iRes;

            if (ImageLayers->GetNumLayers() == 0) {
                MessageBox(hWnd, L"No layers loaded", L"Layers", MB_OK);
                break;
            }

            WCHAR szFilename[MAX_PATH];

            PWSTR pszFilename;
            COMDLG_FILTERSPEC AllType[] =
            {
                 { L"Image files", L"*.cfg" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileSave(hWnd, ImageLayers->ConfigurationFile, &pszFilename, FALSE, 2, AllType, L".cfg")) {
                break;
            }

            wcscpy_s(szFilename, pszFilename);
            CoTaskMemFree(pszFilename);

            iRes = ImageLayers->SaveConfiguration(szFilename);
            if (iRes == APP_SUCCESS) {
                wcscpy_s(ImageLayers->ConfigurationFile, MAX_PATH, szFilename);
            }
            iRes = Displays->SaveConfiguration(szFilename);
            break;
        }

        case IDM_LAYER_SAVEBMP:
        {
            if (ImageLayers->GetNumLayers() == 0) {
                MessageBox(hWnd, L"No layers, nothing to save", L"Layers", MB_OK); 
                break;
            }

            if (!ImageLayers->OverlayValid) {
                MessageBox(hWnd, L"Overlay is not valid to save\nmake sure to 'Apply' current layers configuration", L"Layers", MB_OK);
                break;
            }

            PWSTR pszFilename;
            COMDLG_FILTERSPEC AllType[] =
            {
                 { L"BMP files", L"*.bmp" },
                 { L"All Files", L"*.*" }
            };

            if (!CCFileSave(hWnd, szCurrentFilename, &pszFilename, FALSE, 2, AllType, L".bmp")) {
                break;
            }

            wcscpy_s(szCurrentFilename, pszFilename);
            CoTaskMemFree(pszFilename);

            int iRes;
            iRes = ImageLayers->SaveBMP(szCurrentFilename);
            if (iRes != APP_SUCCESS) {
                MessageBox(hWnd, L"Save Failed", L"Layers", MB_OK);
            }
            break;
        }

        case IDM_DISPLAY_SAVEBMP:
        {
            if (ImageLayers->GetNumLayers() == 0) {
                MessageBox(hWnd, L"No layers, nothing to save", L"Display", MB_OK);
                break;
            }

            PWSTR pszFilename;
            COMDLG_FILTERSPEC AllType[] =
            {
                 { L"BMP files", L"*.bmp" },
                 { L"All Files", L"*.*" }
            };

            if (!CCFileSave(hWnd, szCurrentFilename, &pszFilename, FALSE, 2, AllType, L".bmp")) {
                break;
            }

            wcscpy_s(szCurrentFilename, pszFilename);
            CoTaskMemFree(pszFilename);

            int iRes;
            iRes = Displays->SaveBMP(szCurrentFilename, 0);
            if (iRes != APP_SUCCESS) {
                MessageBox(hWnd, L"Save Failed", L"Layers", MB_OK);
            }
            break;
        }

        case IDM_REFERENCE_SAVEBMP:
        {
            PWSTR pszFilename;
            COMDLG_FILTERSPEC AllType[] =
            {
                 { L"BMP files", L"*.bmp" },
                 { L"All Files", L"*.*" }
            };

            if (!CCFileSave(hWnd, szCurrentFilename, &pszFilename, FALSE, 2, AllType, L".bmp")) {
                break;
            }

            wcscpy_s(szCurrentFilename, pszFilename);
            CoTaskMemFree(pszFilename);

            int iRes;
            iRes = Displays->SaveBMP(szCurrentFilename, 1);
            if (iRes != APP_SUCCESS) {
                MessageBox(hWnd, L"Save Failed", L"Layers", MB_OK);
            }
            break;
        }

        case IDM_BITTOOLS_TEXT2BITSTREAM:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_TEXT2STREAM), hWnd, Text2StreamDlg);
            break;

        case IDM_BITTOOLS_BINARYIMAGE:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_BINARYIMAGE), hWnd, BitImageDlg);
            break;

        case IDM_EXIT:
        {
            if (hwndImage) {
                // save window position/size data
                CString csString = L"ImageWindow";
                SaveWindowPlacement(hwndImage, csString);
                // this must be done to ensure Direct2D factory is released
                DestroyWindow(hwndImage);
            }
            DestroyWindow(hWnd);
            break;
        }

        case IDM_SETTINGS_GLOBAL:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS_GLOBAL), hWnd, SettingsGlobalDlg);
            break;

        case IDM_SETTINGS_DISPLAY:
        {
            if (wcslen(szTempDir) == 0) {
                MessageBox(hWnd, L"Working file folder needs to be set\nin Settings->Global first", L"Layers", MB_OK);
                break;
            }
            if (hwndDisplay) {
                ShowWindow(hwndDisplay, SW_SHOW);
            }
            
            break;
        }

        case IDM_SETTINGS_LAYERS:
        {
            if (wcslen(szTempDir) == 0) {
                MessageBox(hWnd, L"Working file folder needs to be set\nin Settings->Global first", L"Layers", MB_OK);
                break;
            }
            if (hwndLayers) {
                ShowWindow(hwndLayers, SW_SHOW);
            }

            //DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS_LAYERS), hWnd, SettingsLayersDlg);
            break;
        }

        case IDM_SETTINGS_RESET_WINDOWS:
            WritePrivateProfileString(L"GlobalSettings", L"ResetWindows", L"1", (LPCTSTR)strAppNameINI);
            break;

        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDlg);
            break;


        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        break;
    } // This is the end of WM_COMMAND
  
    case WM_CLOSE:
    {
        if (hwndImage) {
            // save window position/size data
            CString csString = L"ImageWindow";
            SaveWindowPlacement(hwndImage, csString);
            // this must be done to ensure Direct2D factory is released
            DestroyWindow(hwndImage);
        }
        if (hwndDisplay) {
            // save window position/size data for the Image Dialog window
            CString csString = L"ImageDisplay";
            SaveWindowPlacement(hwndDisplay, csString);
            DestroyWindow(hwndDisplay);
        }
        if (hwndLayers) {
            // save window position/size data for the Image Dialog window
            CString csString = L"LayerWindow";
            SaveWindowPlacement(hwndLayers, csString);
            DestroyWindow(hwndLayers);
        }

        DestroyWindow(hWnd);
        break;
    }

    case WM_DESTROY:
    {   
        WritePrivateProfileString(L"GlobalSettings", L"CurrentFilename", szCurrentFilename, (LPCTSTR)strAppNameINI);
        WritePrivateProfileString(L"GlobalSettings", L"LastConfigFile", ImageLayers->ConfigurationFile, (LPCTSTR)strAppNameINI);
        // save window position/size data for the Main window
        CString csString = L"MainWindow";
        SaveWindowPlacement(hWnd, csString);

        // release Image Dialog
        if (hwndImage) {
            // save window position/size data for the Image Dialog window
            CString csString = L"ImageWindow";
            SaveWindowPlacement(hwndImage, csString);
            DestroyWindow(hwndImage);
        }

        if (hwndDisplay) {
            // save window position/size data for the Image Dialog window
            CString csString = L"ImageDisplay";
            SaveWindowPlacement(hwndDisplay, csString);
            DestroyWindow(hwndDisplay);
        }

        if (hwndLayers) {
            // save window position/size data for the Image Dialog window
            CString csString = L"LayerWindow";
            SaveWindowPlacement(hwndDisplay, csString);
            DestroyWindow(hwndLayers);
        }

        // delete the global classes;
        if (ImageLayers != NULL) delete ImageLayers;
        if (Displays != NULL) delete Displays;
        if (ImgDlg != NULL) delete ImgDlg;

        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);

    }
    return 0;
}

