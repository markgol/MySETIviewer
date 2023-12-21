//
// MySETIviewer, a set tools for decoding bitstreams into various formats and manipulating those files
// SettingsDlg.cpp
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
// This file contains the dialog callback procedures for settings dialogs; Display and Layers
// 
// V1.0.1.0	2023-12-20	Initial release
// V1.0.2.0 2023-12-20  Added Y direction flag for which direction to move image
//
// Global Settings dialog box handler
// 
#include "framework.h"
#include "MySETIviewer.h"
#include <libloaderapi.h>
#include <Windows.h>
#include <shobjidl.h>
#include <winver.h>
#include <vector>
#include <atlstr.h>
#include <strsafe.h>
#include <Commdlg.h>
#include "Display.h"
#include "Globals.h"
#include "imageheader.h"
#include "FileFunctions.h"
#include "Appfunctions.h"

// local statics

// Display class brushes
static HBRUSH hbrBackground = NULL;
static HBRUSH hbrGapMajor = NULL;
static HBRUSH hbrGapMinor = NULL;

// Layer class brushes
static HBRUSH hbrSelectedLayer = NULL;
static HBRUSH hbrBackgroundLayer = NULL;
static HBRUSH hbrOverlayLayer = NULL;

static BOOL Touched = FALSE;
static HWND hLayerEnable = NULL;

int ReplaceListBoxEntry(HWND hDlg, int Control, int Selection, WCHAR* szString);
void ApplyLayers(HWND hDlg);
void ApplyDisplay(HWND hDlg);

//*******************************************************************************
//
// Message handler for SettingsDisplayDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK SettingsDisplayDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        int x, y;

        Displays->GetGridMajor(&x, &y);
        // IDC_GRID_X_MAJOR
        SetDlgItemInt(hDlg, IDC_GRID_X_MAJOR, x, TRUE);
        // IDC_GRID_Y_MAJOR
        SetDlgItemInt(hDlg, IDC_GRID_Y_MAJOR, y, TRUE);

        Displays->GetGridMinor(&x, &y);
        // IDC_GRID_X_MINOR
        SetDlgItemInt(hDlg, IDC_GRID_X_MINOR, x, TRUE);
        // IDC_GRID_Y_MINOR
        SetDlgItemInt(hDlg, IDC_GRID_Y_MINOR, y, TRUE);

        Displays->GetGapMajor(&x, &y);
        // IDC_GAP_X_MAJOR
        SetDlgItemInt(hDlg, IDC_GAP_X_MAJOR, x, TRUE);
        // IDC_GAP_Y_MAJOR
        SetDlgItemInt(hDlg, IDC_GAP_Y_MAJOR, y, TRUE);

        Displays->GetGapMinor(&x, &y);
        // IDC_GAP_X_MINOR
        SetDlgItemInt(hDlg, IDC_GAP_X_MINOR, x, TRUE);
        // IDC_GAP_Y_MINOR
        SetDlgItemInt(hDlg, IDC_GAP_Y_MINOR, y, TRUE);

        // These controls are updated in the WM_CTLCOLORSTATIC message handler
        // IDC_BACKGROUND
        // IDC_GAP_MAJOR
        // IDC_GAP_MINOR
        
        CString csString = L"ImageDisplay";
        RestoreWindowPlacement(hDlg, csString);

        return (INT_PTR)TRUE;
    }

    case WM_DESTROY:

        // must destroy any brushes created

        if (hbrBackground) {
            DeleteObject(hbrBackground);
            hbrBackground = NULL;
        }
        if (hbrGapMajor) {
            DeleteObject(hbrGapMajor);
            hbrGapMajor = NULL;
        }
        if (hbrGapMinor) {
            DeleteObject(hbrGapMinor);
            hbrGapMinor = NULL;
        }
        hwndDisplay = NULL;

        return (INT_PTR)TRUE;

    case WM_CTLCOLORSTATIC:
    {
        // This is specifically to color the static controls for:
        // IDC_BACKGROUND
        // IDC_GRID
        // IDC_GAP
        HANDLE hControl;

        HDC hdcStatic = (HDC)wParam; // not used here
        hControl = (HANDLE)lParam;

        HANDLE hThis;

        hThis = GetDlgItem(hDlg, IDC_BACKGROUND);
        if (hThis == hControl) {
            if (hbrBackground) {
                DeleteObject(hbrBackground);
            }
            hbrBackground = CreateSolidBrush(Displays->GetBackgroundColor());
            return (INT_PTR)hbrBackground;
        }

        hThis = GetDlgItem(hDlg, IDC_GAP_MAJOR);
        if (hThis == hControl) {
            if (hbrGapMajor) {
                DeleteObject(hbrGapMajor);
            }
            hbrGapMajor = CreateSolidBrush(Displays->GetGapMajorColor());
            return (INT_PTR)hbrGapMajor;
        }

        hThis = GetDlgItem(hDlg, IDC_GAP_MINOR);
        if (hThis == hControl) {
            if (hbrGapMinor) {
                DeleteObject(hbrGapMinor);
            }
            hbrGapMinor = CreateSolidBrush(Displays->GetGapMinorColor());
            return (INT_PTR)hbrGapMinor;
        }
        return FALSE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDC_BACKGROUND_COLOR:
        {
            // use common dialog color picker
            // this is old school method but doesn't require MFC or XAML

            CHOOSECOLOR cc;                 // common dialog box structure 

            // Initialize CHOOSECOLOR 
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hDlg;
            cc.lpCustColors = (LPDWORD)CustomColorTable;
            cc.rgbResult = (DWORD)Displays->GetBackgroundColor();
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorW(&cc) == TRUE)
            {

                if (cc.rgbResult == Displays->GetGapMajorColor() || cc.rgbResult == Displays->GetGapMinorColor()) {
                    MessageBox(hDlg, L"Background color must be different from gaps color\nrecommend RGB = 2,2,2", L"Display", MB_OK);
                    return (INT_PTR)TRUE;
                }
                
                Displays->SetBackgroundColor((COLORREF)cc.rgbResult);
            }
            // force redraw static control
            SetDlgItemText(hDlg, IDC_BACKGROUND, L"");
            if (ImageLayers->GetNumLayers() > 0) {
                ApplyDisplay(hDlg);
            }
            return (INT_PTR)TRUE;
        }

        case IDC_GAP_MAJOR_COLOR:
        {
            // use common dialog color picker
            // this is old school method but doesn't require MFC or XAML

            CHOOSECOLOR cc;                 // common dialog box structure 

            // Initialize CHOOSECOLOR 
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hDlg;
            cc.lpCustColors = (LPDWORD)CustomColorTable;
            cc.rgbResult = (DWORD)Displays->GetGapMajorColor();
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorW(&cc) == TRUE) {
                if (cc.rgbResult == Displays->GetBackgroundColor()) {
                    MessageBox(hDlg, L"Major gap color must be different from Background color\nrecommend RGB = 81,0,40", L"Display", MB_OK);
                    return (INT_PTR)TRUE;
                }
                Displays->SetGapMajorColor((COLORREF)cc.rgbResult);
            }

            // force redraw static control
            SetDlgItemText(hDlg, IDC_GAP_MAJOR, L"");
            if (ImageLayers->GetNumLayers() > 0) {
                ApplyDisplay(hDlg);
            }
            return (INT_PTR)TRUE;
        }

        case IDC_GAP_MINOR_COLOR:
        {
            // use common dialog color picker
            // this is old school method but doesn't require MFC or XAML

            CHOOSECOLOR cc;                 // common dialog box structure 

            // Initialize CHOOSECOLOR 
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hDlg;
            cc.lpCustColors = (LPDWORD)CustomColorTable;
            cc.rgbResult = (DWORD)Displays->GetGapMinorColor();
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorW(&cc) == TRUE) {
                if (cc.rgbResult == Displays->GetBackgroundColor()) {
                    MessageBox(hDlg, L"Minor gap color must be different from Background color\nrecommend RGB = 15,15,15", L"Display", MB_OK);
                    return (INT_PTR)TRUE;
                }
                Displays->SetGapMinorColor((COLORREF)cc.rgbResult);
            }
            // force redraw static control
            SetDlgItemText(hDlg, IDC_GAP_MINOR, L"");
            if (ImageLayers->GetNumLayers() > 0) {
                ApplyDisplay(hDlg);
            }
            return (INT_PTR)TRUE;
        }

        case IDC_SCALE_RESET:
        {
            if (ImgDlg) {
                ImgDlg->SetScale(1.0f);
            }
        }

        case IDC_POS_RESET:
        {
            if (ImgDlg) {
                ImgDlg->SetPan(1.0f, 1.0f);
            }
        }

        case IDC_APPLY:
        {
            if (ImageLayers->GetNumLayers() <= 0) {
                MessageBox(hDlg, L"Nothing to display\nLoad and apply layers first", L"Display", MB_OK);
                return (INT_PTR)TRUE;
            }

            ApplyDisplay(hDlg);
            return (INT_PTR)TRUE;
        }

        case IDOK:
        {
            BOOL bSuccess;
            int x, y;

            // IDC_GRID_X_MAJOR
            GetDlgItemText(hDlg, IDC_GRID_X_MAJOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GridXmajor", szString, (LPCTSTR)strAppNameINI);
            x = GetDlgItemInt(hDlg, IDC_GRID_X_MAJOR, &bSuccess, TRUE);
            // IDC_GRID_Y_MAJOR
            GetDlgItemText(hDlg, IDC_GRID_Y_MAJOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GridYmajor", szString, (LPCTSTR)strAppNameINI);
            y = GetDlgItemInt(hDlg, IDC_GRID_Y_MAJOR, &bSuccess, TRUE);
            Displays->SetGridMajor(x, y);

            // IDC_GRID_X_MINOR
            GetDlgItemText(hDlg, IDC_GRID_X_MINOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GridXminor", szString, (LPCTSTR)strAppNameINI);
            x = GetDlgItemInt(hDlg, IDC_GRID_X_MINOR, &bSuccess, TRUE);
            // IDC_GRID_Y_MINOR
            GetDlgItemText(hDlg, IDC_GRID_Y_MINOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GridYminor", szString, (LPCTSTR)strAppNameINI);
            y = GetDlgItemInt(hDlg, IDC_GRID_Y_MINOR, &bSuccess, TRUE);
            Displays->SetGridMinor(x, y);

            // IDC_GAP_X_MAJOR
            GetDlgItemText(hDlg, IDC_GAP_X_MAJOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GapXmajor", szString, (LPCTSTR)strAppNameINI);
            x = GetDlgItemInt(hDlg, IDC_GAP_X_MAJOR, &bSuccess, TRUE);
            // IDC_GAP_Y_MAJOR
            GetDlgItemText(hDlg, IDC_GAP_Y_MAJOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GapYmajor", szString, (LPCTSTR)strAppNameINI);
            y = GetDlgItemInt(hDlg, IDC_GAP_Y_MAJOR, &bSuccess, TRUE);
            Displays->SetGapMajor(x, y);

            // IDC_GAP_X_MINOR
            GetDlgItemText(hDlg, IDC_GAP_X_MINOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GapXminor", szString, (LPCTSTR)strAppNameINI);
            x = GetDlgItemInt(hDlg, IDC_GAP_X_MINOR, &bSuccess, TRUE);
            // IDC_GAP_Y_MINOR
            GetDlgItemText(hDlg, IDC_GAP_Y_MINOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GapYminor", szString, (LPCTSTR)strAppNameINI);
            y = GetDlgItemInt(hDlg, IDC_GAP_Y_MINOR, &bSuccess, TRUE);
            Displays->SetGapMinor(x, y);

            // save color settings
            swprintf_s(szString, MAX_PATH, L"%u", Displays->GetBackgroundColor());
            WritePrivateProfileString(L"SettingsDisplayDlg", L"rgbBackground", szString, (LPCTSTR)strAppNameINI);

            swprintf_s(szString, MAX_PATH, L"%u", Displays->GetGapMajorColor());
            WritePrivateProfileString(L"SettingsDisplayDlg", L"rgbGapMajor", szString, (LPCTSTR)strAppNameINI);

            swprintf_s(szString, MAX_PATH, L"%u", Displays->GetGapMinorColor());
            WritePrivateProfileString(L"SettingsDisplayDlg", L"rgbGapMinor", szString, (LPCTSTR)strAppNameINI);

            for (int i = 0; i < 16; i++) {
                WCHAR CustomColor[20];
                swprintf_s(CustomColor, 20, L"CustomColorTable%d", i);
                swprintf_s(szString, MAX_PATH, L"%u", CustomColorTable[i]);
                WritePrivateProfileString(L"SettingsDlg", CustomColor, szString, (LPCTSTR)strAppNameINI);
            }

            CString csString = L"DisplayWindow";
            SaveWindowPlacement(hwndDisplay, csString);
            ShowWindow(hDlg, SW_HIDE);
            return (INT_PTR)TRUE;
        }

        case IDCANCEL:
        {
            // These are class Display settings
            COLORREF BackGroundColor = (COLORREF)GetPrivateProfileInt(L"SettingsDisplayDlg", L"rgbBackground", 0, (LPCTSTR)strAppNameINI);
            COLORREF GapMajorColor = (COLORREF)GetPrivateProfileInt(L"SettingsDisplayDlg", L"rgbGapMajor", 0, (LPCTSTR)strAppNameINI);
            COLORREF GapMinorColor = (COLORREF)GetPrivateProfileInt(L"SettingsDisplayDlg", L"rgbGapMinor", 0, (LPCTSTR)strAppNameINI);
            Displays->SetColors(BackGroundColor, GapMajorColor, GapMinorColor);

            int ValueX, ValueY;
            ValueX = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GridXmajor", 4, (LPCTSTR)strAppNameINI);
            ValueY = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GridYmajor", 2, (LPCTSTR)strAppNameINI);
            Displays->SetGridMajor(ValueX, ValueY);

            ValueX = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GridXminor", 2, (LPCTSTR)strAppNameINI);
            ValueY = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GridYminor", 2, (LPCTSTR)strAppNameINI);
            Displays->SetGridMinor(ValueX, ValueY);

            ValueX = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GapXmajor", 2, (LPCTSTR)strAppNameINI);
            ValueY = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GapYmajor", 2, (LPCTSTR)strAppNameINI);
            Displays->SetGapMajor(ValueX, ValueY);

            ValueX = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GapXminor", 1, (LPCTSTR)strAppNameINI);
            ValueY = GetPrivateProfileInt(L"SettingsDisplayDlg", L"GapYminor", 1, (LPCTSTR)strAppNameINI);
            Displays->SetGapMinor(ValueX, ValueY);

            int x, y;
            Displays->GetGridMajor(&x, &y);
            // IDC_GRID_X_MAJOR
            SetDlgItemInt(hDlg, IDC_GRID_X_MAJOR, x, TRUE);
            // IDC_GRID_Y_MAJOR
            SetDlgItemInt(hDlg, IDC_GRID_Y_MAJOR, y, TRUE);

            Displays->GetGridMinor(&x, &y);
            // IDC_GRID_X_MINOR
            SetDlgItemInt(hDlg, IDC_GRID_X_MINOR, x, TRUE);
            // IDC_GRID_Y_MINOR
            SetDlgItemInt(hDlg, IDC_GRID_Y_MINOR, y, TRUE);

            Displays->GetGapMajor(&x, &y);
            // IDC_GAP_X_MAJOR
            SetDlgItemInt(hDlg, IDC_GAP_X_MAJOR, x, TRUE);
            // IDC_GAP_Y_MAJOR
            SetDlgItemInt(hDlg, IDC_GAP_Y_MAJOR, y, TRUE);

            Displays->GetGapMinor(&x, &y);
            // IDC_GAP_X_MINOR
            SetDlgItemInt(hDlg, IDC_GAP_X_MINOR, x, TRUE);
            // IDC_GAP_Y_MINOR
            SetDlgItemInt(hDlg, IDC_GAP_Y_MINOR, y, TRUE);
            ShowWindow(hDlg, SW_HIDE);
            SetDlgItemText(hDlg, IDC_GAP_MINOR, L"");
            SetDlgItemText(hDlg, IDC_GAP_MAJOR, L"");
            SetDlgItemText(hDlg, IDC_BACKGROUND, L"");

            return (INT_PTR)TRUE;
        }

        } // end of WM_COMMAND
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Helper function for SettingsLayerDlg dialog box.
// 
//*******************************************************************************
void ApplyDisplay(HWND hDlg) 
{
    BOOL bSuccess;
    int x, y;

    // IDC_GRID_X_MAJOR
    x = GetDlgItemInt(hDlg, IDC_GRID_X_MAJOR, &bSuccess, TRUE);
    // IDC_GRID_Y_MAJOR
    y = GetDlgItemInt(hDlg, IDC_GRID_Y_MAJOR, &bSuccess, TRUE);
    Displays->SetGridMajor(x, y);

    // IDC_GRID_X_MINOR
    x = GetDlgItemInt(hDlg, IDC_GRID_X_MINOR, &bSuccess, TRUE);
    // IDC_GRID_Y_MINOR
    y = GetDlgItemInt(hDlg, IDC_GRID_Y_MINOR, &bSuccess, TRUE);
    Displays->SetGridMinor(x, y);

    // IDC_GAP_X_MAJOR
    x = GetDlgItemInt(hDlg, IDC_GAP_X_MAJOR, &bSuccess, TRUE);
    // IDC_GAP_Y_MAJOR
    y = GetDlgItemInt(hDlg, IDC_GAP_Y_MAJOR, &bSuccess, TRUE);
    Displays->SetGapMajor(x, y);

    // IDC_GAP_X_MINOR
    x = GetDlgItemInt(hDlg, IDC_GAP_X_MINOR, &bSuccess, TRUE);
    // IDC_GAP_Y_MINOR
    y = GetDlgItemInt(hDlg, IDC_GAP_Y_MINOR, &bSuccess, TRUE);
    Displays->SetGapMinor(x, y);

    COLORREF* Overlay;
    int xsize, ysize;
    int iRes;
    iRes = ImageLayers->GetOverlayImage(&Overlay, &xsize, &ysize);
    if (iRes == APP_SUCCESS) {
        int iRes;
        Displays->CalculateDisplayExtent(xsize, ysize);
        iRes = Displays->CreateDisplayImages();
        if (iRes != APP_SUCCESS) {
            MessageMySETIviewerError(hDlg, iRes, L"Display 0 gap parameter");
            return;
        }
        iRes = Displays->UpdateDisplay(Overlay, xsize, ysize);
        if (hwndImage != NULL) {
            PostMessage(hwndImage, WM_COMMAND, IDC_GENERATE_BMP, 0l);
            ShowWindow(hwndImage, SW_SHOW);
        }
    }
    else {
        MessageBox(hDlg, L"Nothing to display\nLoad and apply layers first", L"Display", MB_OK);
    }

    return;
}

//*******************************************************************************
//
// Message handler for SettingsLayersDlg dialog box.
// 
// Things needed to do to be able to make modeless
//      Changing yposDir requires updating the currently selected parameters field for y position
//      Move Add layer to Layers dialog, delete from menus
//      Load configuration must reload the the layer list in the Layer dialog and the
//        position parameters and color parmaters
//      Move New to Layers dialog, delete from menus
//      Delete Remove Layer from menu
//      Reload configuration must reload the layer list in the layer dialog and the 
//        position parameters and color parameters
//      Remove disabling controls in Layers dialog, change to reject changes if # Layers is 0
// 
//*******************************************************************************
INT_PTR CALLBACK SettingsLayersDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {

    case WM_INITDIALOG:
    {
        int NumLayers;
        NumLayers = ImageLayers->GetNumLayers();

        if (NumLayers == 0) {
            if (MessageBox(hDlg, L"No layers loaded\nLoad layer configuration", L"Layers", MB_YESNO) == IDYES) {
                // use files dialog to select config file
                int iRes;
                // todo: load last configuration file by default
                {
                    WCHAR Filename[MAX_PATH] = L"";

                    PWSTR pszFilename;
                    COMDLG_FILTERSPEC CFGType[] =
                    {
                         { L"Configuration files", L"*.cfg" },
                         { L"All Files", L"*.*" },
                    };
                 
                    if (!CCFileOpen(hDlg, Filename, &pszFilename, FALSE, 2, CFGType, L".cfg")) {
                        return (INT_PTR)TRUE;
                    }
                    wcscpy_s(Filename, MAX_PATH, pszFilename);
                    CoTaskMemFree(pszFilename);
                    
                    iRes = ImageLayers->LoadConfiguration(Filename);
                    if (iRes == APPERR_FILESIZE) {
                        MessageMySETIviewerError(hDlg, iRes, L"Partial configuration loaded");
                    } else if (iRes != APP_SUCCESS) {
                        MessageMySETIviewerError(hDlg, iRes, L"Load cfg failure");
                        return(INT_PTR)TRUE;
                    }
                }
                NumLayers = ImageLayers->GetNumLayers();
            }
            if (NumLayers == 0) {
                // disable everything except default color selection and 
                // minimum overlay size
                HWND hControl;

                //IDC_COMBO_LAYER_LIST
                hControl = GetDlgItem(hDlg, IDC_COMBO_LAYER_LIST);
                EnableWindow(hControl, FALSE);

                //IDC_X_POS
                hControl = GetDlgItem(hDlg, IDC_X_POS);
                EnableWindow(hControl, FALSE);

                //IDC_Y_POS
                hControl = GetDlgItem(hDlg, IDC_Y_POS);
                EnableWindow(hControl, FALSE);

                //IDC_X_MINUS
                hControl = GetDlgItem(hDlg, IDC_X_MINUS);
                EnableWindow(hControl, FALSE);

                //IDC_X_PLUS
                hControl = GetDlgItem(hDlg, IDC_X_PLUS);
                EnableWindow(hControl, FALSE);

                //IDC_Y_MINUS
                hControl = GetDlgItem(hDlg, IDC_Y_MINUS);
                EnableWindow(hControl, FALSE);

                //IDC_X_PLUS
                hControl = GetDlgItem(hDlg, IDC_X_PLUS);
                EnableWindow(hControl, FALSE);

                //IDC_ENABLE
                hControl = GetDlgItem(hDlg, IDC_ENABLE);
                EnableWindow(hControl, FALSE);

                //IDC_DELETE_LAYER
                hControl = GetDlgItem(hDlg, IDC_DELETE_LAYER);
                EnableWindow(hControl, FALSE);

                // set minimum size for overlay order image
                {
                    int x, y;
                    ImageLayers->GetMinOverlaySize(&x, &y);
                    SetDlgItemInt(hDlg, IDC_LAYERS_MIN_X, x, TRUE);
                    SetDlgItemInt(hDlg, IDC_LAYERS_MIN_Y, y, TRUE);
                }

                {
                    // save window position/size data
                    CString csString = L"LayerWindow";
                    RestoreWindowPlacement(hDlg, csString);
                    //SaveWindowPlacement(hDlg, csString);
                }
                return(INT_PTR)TRUE;
            }
        }
        // backup configuration in case user cancels
        WCHAR TempConfig[MAX_PATH];
        swprintf_s(TempConfig, MAX_PATH, L"%s\\MySETIviewerTmp.cfg", szTempDir);
        ImageLayers->SaveConfiguration(TempConfig);

        int CurrentLayer;
        int x, y;
        CurrentLayer = ImageLayers->GetCurrentLayer();

        // IDC_LAYER_LIST
        {
            HWND ListHwnd;
            WCHAR szString[MAX_PATH];

            ListHwnd = GetDlgItem(hDlg, IDC_LAYER_LIST);

            for (int i = 0; i < NumLayers; i++) {
                // split filename into components
                // add Layer number, fname.ext string, x,y size to combo box
                ImageLayers->GetSize(i, &x, &y);
                if (ImageLayers->IsLayerEnabled(i)) {
                    swprintf_s(szString, MAX_PATH, L"Enabled:  %dHx%dV, %s", x, y,
                        ImageLayers->LayerFilename[i]);
                }
                else {
                    swprintf_s(szString, MAX_PATH, L"Disabled: %dHx%dV, %s", x, y,
                        ImageLayers->LayerFilename[i]);
                }
                // AddString
                SendMessage(ListHwnd, LB_ADDSTRING, 0, (LPARAM)szString);
            }
            // Set current selection to Current layer
            SendMessage(ListHwnd, LB_SETCURSEL, CurrentLayer, (LPARAM)0);
            SendMessage(ListHwnd, LB_GETTEXT, CurrentLayer, (LPARAM)szString);
            SetDlgItemText(hDlg, IDC_LAYER_TEXT, szString);
        }
        // IDC_X_POS, IDC_Y_POS
        // set x to current layer X location value
        ImageLayers->GetLocation(CurrentLayer, &x, &y);
        
        SetDlgItemInt(hDlg, IDC_X_POS, x, TRUE);

        // set x to current layer X location value
        SetDlgItemInt(hDlg, IDC_Y_POS, y, TRUE);
        
        // set minimum size for overlay order image
        {
            int x, y;
            ImageLayers->GetMinOverlaySize(&x, &y);
            SetDlgItemInt(hDlg, IDC_LAYERS_MIN_X, x, TRUE);
            SetDlgItemInt(hDlg, IDC_LAYERS_MIN_Y, y, TRUE);
        }

        // 
        // IDC_LAYER_COLOR
        // set current color in WM_CTLCOLORSTATIC

        //IDC_ENABLE
        if (ImageLayers->IsLayerEnabled(CurrentLayer)) {
            CheckDlgButton(hDlg, IDC_ENABLE, BST_CHECKED);
        }
        hLayerEnable = GetDlgItem(hDlg, IDC_ENABLE);
        
        {
            // save window position/size data
            CString csString = L"LayerWindow";
            RestoreWindowPlacement(hDlg, csString);
            //SaveWindowPlacement(hwndImage, csString);
        }
        Touched = FALSE;
        return (INT_PTR)TRUE;
    }

    case WM_DESTROY:
        // must destroy any brushes created

        if (hbrSelectedLayer) {
            DeleteObject(hbrSelectedLayer);
            hbrSelectedLayer = NULL;
        }
        if (hbrBackgroundLayer) {
            DeleteObject(hbrBackgroundLayer);
            hbrBackgroundLayer = NULL;
        }
        hLayerEnable = NULL;
        return (INT_PTR)TRUE;

    case WM_CTLCOLORSTATIC:
    {
        // This is specifically to color the static controls for:
        // IDC_LAYER_COLOR
        HANDLE hControl;

        hControl = (HANDLE)lParam;

        HANDLE hThis;
        COLORREF ThisColor;

        if (ImageLayers->GetNumLayers() != 0) {
            hThis = GetDlgItem(hDlg, IDC_LAYER_COLOR);
            if (hThis == hControl) {
                if (hbrSelectedLayer) {
                    DeleteObject(hbrSelectedLayer);
                }
                int CurrentLayer = ImageLayers->GetCurrentLayer();
                ThisColor = ImageLayers->GetLayerColor(CurrentLayer);
                hbrSelectedLayer = CreateSolidBrush(ThisColor);
                return (INT_PTR)hbrSelectedLayer;
            }
        }
        else {
            hThis = GetDlgItem(hDlg, IDC_LAYER_COLOR);
            if (hThis == hControl) {
                if (hbrSelectedLayer) {
                    DeleteObject(hbrSelectedLayer);
                }
                ThisColor = ImageLayers->GetDefaultLayerColor();
                hbrSelectedLayer = CreateSolidBrush(ThisColor);
                return (INT_PTR)hbrSelectedLayer;
            }
        }

        hThis = GetDlgItem(hDlg, IDC_BACKGROUND_COLOR);
        if (hThis == hControl) {
            if (hbrBackgroundLayer) {
                DeleteObject(hbrBackgroundLayer);
            }
            ThisColor = ImageLayers->GetBackgroundColor();
            hbrBackgroundLayer = CreateSolidBrush(ThisColor);
            return (INT_PTR)hbrBackgroundLayer;
        }

        hThis = GetDlgItem(hDlg, IDC_OVERLAY_COLOR);
        if (hThis == hControl) {
            if (hbrOverlayLayer) {
                DeleteObject(hbrOverlayLayer);
            }
            ThisColor = ImageLayers->GetOverlayColor();
            hbrOverlayLayer = CreateSolidBrush(ThisColor);
            return (INT_PTR)hbrOverlayLayer;
        }

        return FALSE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDC_ENABLE:
        {
            int Selection;
            int x, y;
            WCHAR szString[MAX_PATH];
            
            Selection = ImageLayers->GetCurrentLayer();
            ImageLayers->GetSize(Selection, &x, &y);

            if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == BN_DOUBLECLICKED) {
                if (IsDlgButtonChecked(hDlg, IDC_ENABLE) == BST_CHECKED) {
                    ImageLayers->EnableLayer(Selection);
                    swprintf_s(szString, MAX_PATH, L"Enabled:  %dHx%dV, %s", x, y,
                        ImageLayers->LayerFilename[Selection]);
                    ReplaceListBoxEntry(hDlg, IDC_LAYER_LIST, Selection, szString);
                }
                else {
                    ImageLayers->DisableLayer(Selection);
                    swprintf_s(szString, MAX_PATH, L"Disabled: %dHx%dV, %s", x, y,
                        ImageLayers->LayerFilename[Selection]);
                    ReplaceListBoxEntry(hDlg, IDC_LAYER_LIST, Selection, szString);
                }
            }

            // set text description for layer
            HWND ListHwnd;
            ListHwnd = GetDlgItem(hDlg, IDC_LAYER_LIST);
            SendMessage(ListHwnd, LB_GETTEXT, Selection, (LPARAM)szString);
            SetDlgItemText(hDlg, IDC_LAYER_TEXT, szString);

            ApplyLayers(hDlg);
                        
            return (INT_PTR)FALSE;
        }


        case IDC_LAYER_LIST:
        {   
            // check if list box selection changed
            if (HIWORD(wParam) != LBN_SELCHANGE) {
                return (INT_PTR)FALSE;
            }
            // this should only happen when the user makes changes to the selected layer
            // it does not trigger when LB_SETCURSEL, LB_DELETESTRING, LB_ADDSTRING occurs
            int Layer;
            Layer = (int)SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
            if (Layer < 0) {
                return (INT_PTR)TRUE;
            }
            // set text description for layer
            HWND ListHwnd;
            WCHAR szString[MAX_PATH];

            ListHwnd = GetDlgItem(hDlg, IDC_LAYER_LIST);
            SendMessage(ListHwnd, LB_GETTEXT, Layer, (LPARAM)szString);
            SetDlgItemText(hDlg, IDC_LAYER_TEXT, szString);
            
            // set x,y and color for this layer
            int x, y;
            ImageLayers->SetCurrentLayer(Layer);
            ImageLayers->GetLocation(Layer,&x, &y);
            ImageLayers->GetLayerColor(Layer);
            SetDlgItemInt(hDlg, IDC_X_POS, x, TRUE);
            SetDlgItemInt(hDlg, IDC_Y_POS, y, TRUE);
            //IDC_ENABLE
            if (ImageLayers->IsLayerEnabled(Layer)) {
                CheckDlgButton(hDlg, IDC_ENABLE, BST_CHECKED);
            }
            else {
                CheckDlgButton(hDlg, IDC_ENABLE, BST_UNCHECKED);
            }
            SetDlgItemText(hDlg, IDC_LAYER_COLOR, L"");

            Touched = TRUE;

            ApplyLayers(hDlg);

            return (INT_PTR)TRUE;
        }

        case IDC_DELETE_LAYER:
        {
            int NumLayers;

            NumLayers = ImageLayers->GetNumLayers();
            if (NumLayers == 0) {
                MessageBox(hDlg, L"There are no layers to delete", L"No Layers", MB_OK);
                return (INT_PTR)TRUE;
            }

            WCHAR szString[128];
            HWND ListHwnd;
            int Selection;

            ListHwnd = GetDlgItem(hDlg, IDC_LAYER_LIST);
            Selection = (int)SendMessage(ListHwnd, LB_GETCURSEL, 0, 0);
            if (Selection < 0) {
                MessageBox(hDlg, L"No layer selected for deletion", L"Layers", MB_OK);
                return (INT_PTR)TRUE;
            }
            swprintf_s(szString, 128, L"Do you really want to delete\n Layer %d?", Selection);
            if (MessageBox(hDlg, szString, L"Layers", MB_YESNO| MB_DEFBUTTON2) != IDYES) {
                return (INT_PTR)TRUE;
            }

            // get current selection in combo box
            ImageLayers->ReleaseLayer(Selection);
            
            // remove from combo box list
            SendMessage(ListHwnd, LB_DELETESTRING, Selection, 0);

            NumLayers = ImageLayers->GetNumLayers();
            if (NumLayers == 0) {
                return (INT_PTR)TRUE;
            }
            if (Selection >= NumLayers) {
                Selection = NumLayers - 1;
            }
            
            SendMessage(ListHwnd, LB_SETCURSEL, Selection, 0);
            // set x,y and color for this layer
            int x, y;
            ImageLayers->SetCurrentLayer(Selection);
            ImageLayers->GetLocation(Selection, &x, &y);
            ImageLayers->GetLayerColor(Selection);
            SetDlgItemInt(hDlg, IDC_X_POS, x, TRUE);
            SetDlgItemInt(hDlg, IDC_Y_POS, y, TRUE);
            //IDC_ENABLE
            if (ImageLayers->IsLayerEnabled(Selection)) {
                CheckDlgButton(hDlg, IDC_ENABLE, BST_CHECKED);
            }
            else {
                CheckDlgButton(hDlg, IDC_ENABLE, BST_UNCHECKED);
            }
            SetDlgItemText(hDlg, IDC_LAYER_COLOR, L"");

            // set text description for layer
            ListHwnd = GetDlgItem(hDlg, IDC_LAYER_LIST);
            SendMessage(ListHwnd, LB_GETTEXT, Selection, (LPARAM)szString);
            SetDlgItemText(hDlg, IDC_LAYER_TEXT, szString);

            Touched = TRUE;
            
            ApplyLayers(hDlg);
            
            return (INT_PTR)TRUE;
        }

        case IDC_SELECT_LAYER_COLOR:
        {
            // use common dialog color picker
            // this is old school method but doesn't require MFC or XAML

            CHOOSECOLOR cc;                 // common dialog box structure 
            

            int CurrentLayer = ImageLayers->GetCurrentLayer();

            // Initialize CHOOSECOLOR 
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hDlg;
            cc.lpCustColors = (LPDWORD)CustomColorTable;
            if (CurrentLayer >= 0) {
                cc.rgbResult = (DWORD)ImageLayers->GetLayerColor(CurrentLayer);
            }
            else {
                cc.rgbResult = (DWORD)ImageLayers->GetDefaultLayerColor();
            }
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorW(&cc) == TRUE) {
                if (CurrentLayer >= 0) {
                    ImageLayers->SetLayerColor(CurrentLayer, (COLORREF)cc.rgbResult);
                }
                else {
                    ImageLayers->SetDefaultLayerColor((COLORREF)cc.rgbResult);
                }
            }
            // force redraw static control
            SetDlgItemText(hDlg, IDC_LAYER_COLOR, L"");

            Touched = TRUE;

            ApplyLayers(hDlg);
            
            return (INT_PTR)TRUE;
        }

        case IDC_SELECT_BACKGROUND_COLOR:
        {
            // use common dialog color picker
            // this is old school method but doesn't require MFC or XAML

            CHOOSECOLOR cc;                 // common dialog box structure 

            // Initialize CHOOSECOLOR 
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hDlg;
            cc.lpCustColors = (LPDWORD)CustomColorTable;
            cc.rgbResult = (DWORD)ImageLayers->GetBackgroundColor();
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorW(&cc) == TRUE)
            {
                ImageLayers->SetBackgroundColor((COLORREF)cc.rgbResult);
            }
            // force redraw static control
            SetDlgItemText(hDlg, IDC_BACKGROUND_COLOR, L"");

            Touched = TRUE;

            ApplyLayers(hDlg);
            
            return (INT_PTR)TRUE;
        }

        case IDC_SELECT_OVERLAY_COLOR:
        {
            // use common dialog color picker
            // this is old school method but doesn't require MFC or XAML

            CHOOSECOLOR cc;                 // common dialog box structure 

            // Initialize CHOOSECOLOR 
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hDlg;
            cc.lpCustColors = (LPDWORD)CustomColorTable;
            cc.rgbResult = (DWORD)ImageLayers->GetOverlayColor();
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorW(&cc) == TRUE)
            {
                ImageLayers->SetOverlayColor((COLORREF)cc.rgbResult);
            }
            // force redraw static control
            SetDlgItemText(hDlg, IDC_OVERLAY_COLOR, L"");

            Touched = TRUE;

            ApplyLayers(hDlg);
            
            return (INT_PTR)TRUE;
        }

        case IDC_APPLY:
        {
            if (ImageLayers->GetNumLayers() <= 0) {
                MessageBox(hDlg, L"Nothing to display\nLoad layers first", L"Display", MB_OK);
                return(INT_PTR)TRUE;
            }
            ApplyLayers(hDlg);
            
            return (INT_PTR)TRUE;
        }

        case IDC_X_MINUS:
        {
            int Value;
            BOOL bSuccess;
            Value = GetDlgItemInt(hDlg, IDC_X_POS, &bSuccess, TRUE);
            if (!bSuccess) {
                MessageBox(hDlg, L"bad X position, value reset", L"layers", MB_OK);
                SetDlgItemInt(hDlg, IDC_X_POS, 0, TRUE);
                Touched = TRUE;
                return (INT_PTR)TRUE;
            }
            Value--;
            SetDlgItemInt(hDlg, IDC_X_POS, Value, TRUE);
            // update layer value
            int Layer;
            int x, y;
            Layer = ImageLayers->GetCurrentLayer();
            ImageLayers->GetLocation(Layer, &x, &y);
            x = Value;
            ImageLayers->SetLocation(Layer, x, y);

            Touched = TRUE;

            ApplyLayers(hDlg);
            
            return (INT_PTR)TRUE;
        }

        case IDC_X_PLUS:
        {
            int Value;
            BOOL bSuccess;
            Value = GetDlgItemInt(hDlg, IDC_X_POS, &bSuccess, TRUE);
            if (!bSuccess) {
                MessageBox(hDlg, L"bad X position, value reset", L"layers", MB_OK);
                SetDlgItemInt(hDlg, IDC_X_POS, 0, TRUE);
                Touched = TRUE;
                return (INT_PTR)TRUE;
            }
            Value++;
            SetDlgItemInt(hDlg, IDC_X_POS, Value, TRUE);
            // update layer value
            int Layer;
            int x, y;
            Layer = ImageLayers->GetCurrentLayer();
            ImageLayers->GetLocation(Layer, &x, &y);
            x = Value;
            ImageLayers->SetLocation(Layer, x, y);

            Touched = TRUE;

            ApplyLayers(hDlg);
            
            return (INT_PTR)TRUE;
        }

        case IDC_Y_MINUS:
        {
            int Value;
            BOOL bSuccess;
            Value = GetDlgItemInt(hDlg, IDC_Y_POS, &bSuccess, TRUE);
            if (!bSuccess) {
                MessageBox(hDlg, L"bad Y position, value reset", L"layers", MB_OK);
                SetDlgItemInt(hDlg, IDC_Y_POS, 0, TRUE);
                Touched = TRUE;
                return (INT_PTR)TRUE;
            }
            Value--;
            SetDlgItemInt(hDlg, IDC_Y_POS, Value, TRUE);
            // update layer value
            int Layer;
            int x, y;
            Layer = ImageLayers->GetCurrentLayer();
            ImageLayers->GetLocation(Layer, &x, &y);
            y = Value;
            ImageLayers->SetLocation(Layer, x, y);

            Touched = TRUE;

            ApplyLayers(hDlg);
           
            return (INT_PTR)TRUE;
        }

        case IDC_Y_PLUS:
        {
            int Value;
            BOOL bSuccess;
            Value = GetDlgItemInt(hDlg, IDC_Y_POS, &bSuccess, TRUE);
            if (!bSuccess) {
                MessageBox(hDlg, L"bad Y position, value reset", L"layers", MB_OK);
                SetDlgItemInt(hDlg, IDC_Y_POS, 0, TRUE);
                Touched = TRUE;
                return (INT_PTR)TRUE;
            }
            Value++;
            SetDlgItemInt(hDlg, IDC_Y_POS, Value, TRUE);
            // update layer value
            int Layer;
            int x, y;
            Layer = ImageLayers->GetCurrentLayer();
            ImageLayers->GetLocation(Layer, &x, &y);
            y = Value;
            ImageLayers->SetLocation(Layer, x, y);

            Touched = TRUE;

            ApplyLayers(hDlg);
            
            return (INT_PTR)TRUE;
        }

        case IDOK:
        {
            WCHAR CustomColor[20];
            WCHAR szString[20];
            BOOL bSuccess;

            {
                int x, y;

                x = GetDlgItemInt(hDlg, IDC_LAYERS_MIN_X, &bSuccess, TRUE);
                if (!bSuccess) {
                    MessageBox(hDlg, L"Invalid X min. overlay size", L"Layers", MB_OK);
                    return (INT_PTR)TRUE;
                }
                y = GetDlgItemInt(hDlg, IDC_LAYERS_MIN_Y, &bSuccess, TRUE);
                if (!bSuccess) {
                    MessageBox(hDlg, L"Invalid Y min. overlay size", L"Layers", MB_OK);
                    return (INT_PTR)TRUE;
                }
                ImageLayers->SetMinOverlaySize(x, y);
            }

            for (int i = 0; i < 16; i++) {
                swprintf_s(CustomColor, 20, L"CustomColorTable%d", i);
                swprintf_s(szString, 20, L"%u", CustomColorTable[i]);
                WritePrivateProfileString(L"SettingsDlg", CustomColor, szString, (LPCTSTR)strAppNameINI);
            }

            COLORREF Color;

            if (ImageLayers->GetNumLayers() == 0) {
                Color = ImageLayers->GetDefaultLayerColor();
                swprintf_s(szString, 20, L"%u", Color);
                WritePrivateProfileString(L"SettingsDlg", L"DefaultLayerColor", szString, (LPCTSTR)strAppNameINI);
            }

            Color = ImageLayers->GetBackgroundColor();
            swprintf_s(szString, 20, L"%u", Color);
            WritePrivateProfileString(L"SettingsDlg", L"BackgroundColor", szString, (LPCTSTR)strAppNameINI);
            
            Color = ImageLayers->GetOverlayColor();
            swprintf_s(szString, 20, L"%u", Color);
            WritePrivateProfileString(L"SettingsDlg", L"OverlayColor", szString, (LPCTSTR)strAppNameINI);

            HWND ListHwnd;
            int CurrentLayer;

            ListHwnd = GetDlgItem(hDlg, IDC_LAYER_LIST);
            CurrentLayer = (int) SendMessage(ListHwnd, LB_GETCURSEL, 0, 0);
            if (CurrentLayer < 0) CurrentLayer = 0;
            ImageLayers->SetCurrentLayer(CurrentLayer);
            Touched = FALSE;

            {
                // save window position/size data
                CString csString = L"LayerWindow";
                //RestoreWindowPlacement(hDlg, csString);
                SaveWindowPlacement(hDlg, csString);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        case IDCANCEL:
            // backup configuration in case user cancels
            if (Touched) {
                WCHAR TempConfig[MAX_PATH];
                swprintf_s(TempConfig, MAX_PATH, L"%s\\MySETIviewerTmp.cfg", szTempDir);
                ImageLayers->LoadConfiguration(TempConfig);
                ApplyLayers(hDlg);
                Touched = FALSE;
            }
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }

    return (INT_PTR)FALSE;
}
//*******************************************************************************
//
// Helper function for SettingsLayerDlg dialog box.
// 
//*******************************************************************************
void ApplyLayers(HWND hDlg)
{
    int xnewsize, ynewsize;
    int iRes;
    int x, y;
    int Xsize, Ysize;
    BOOL bSuccess;

    if (ImageLayers->GetNumLayers() <= 0) {
        return;
    }

    x = GetDlgItemInt(hDlg, IDC_X_POS, &bSuccess, TRUE);
    if (!bSuccess) {
        MessageBox(hDlg, L"Invalid x position", L"Layers", MB_OK);
        return;
    }
    y = GetDlgItemInt(hDlg, IDC_Y_POS, &bSuccess, TRUE);
    if (!bSuccess) {
        MessageBox(hDlg, L"Invalid y position", L"Layers", MB_OK);
        return;
    }

    Xsize = GetDlgItemInt(hDlg, IDC_LAYERS_MIN_X, &bSuccess, TRUE);
    if (!bSuccess) {
        MessageBox(hDlg, L"Invalid X Min Overlay size", L"Layers", MB_OK);
        return;
    }
    Ysize = GetDlgItemInt(hDlg, IDC_LAYERS_MIN_Y, &bSuccess, TRUE);
    if (!bSuccess) {
        MessageBox(hDlg, L"Invalid Y Min Overlay size", L"Layers", MB_OK);
        return;
    }

    ImageLayers->SetLocation(ImageLayers->GetCurrentLayer(), x, y);
    ImageLayers->SetMinOverlaySize(Xsize, Ysize);

    iRes = ImageLayers->GetNewOverlaySize(&xnewsize, &ynewsize);
    if (iRes != APP_SUCCESS) {
        MessageBox(hDlg, L"Creating Overlay image failed", L"Layers", MB_OK);
        return;
    }
    iRes = ImageLayers->ReleaseOverlay();
    iRes = ImageLayers->CreateOverlay(xnewsize, ynewsize);
    if (iRes != APP_SUCCESS) {
        return;
    }

    iRes = ImageLayers->UpdateOverlay();

    COLORREF* Overlay;
    int xsize, ysize;

    iRes = ImageLayers->GetOverlayImage(&Overlay, &xsize, &ysize);
    if (iRes == APP_SUCCESS) {
        int iRes;
        Displays->CalculateDisplayExtent(xsize, ysize);
        iRes = Displays->CreateDisplayImages();
        if (iRes != APP_SUCCESS) {
            MessageMySETIviewerError(hDlg, iRes, L"Display 0 gap parameter");
            return;
        }
        iRes = Displays->UpdateDisplay(Overlay, xsize, ysize);
        if (hwndImage != NULL) {
            PostMessage(hwndImage, WM_COMMAND, IDC_GENERATE_BMP, 0l);
            ShowWindow(hwndImage, SW_SHOW);
        }
    }

    return;
}

//*******************************************************************************
//
// Message handler for SettingsGlobalDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK SettingsGlobalDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        int iRes;
        GetPrivateProfileString(L"SettingsGlobalDlg", L"TempDir", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMG_TEMP, szString);

        // todo: move to about box
        SetDlgItemText(hDlg, IDC_INI_FILE, strAppNameINI);
        SetDlgItemText(hDlg, IDC_EXE_FILE, strAppNameEXE);

        // IDC_SETTINGS_AUTO_PNG
        iRes = GetPrivateProfileInt(L"SettingsGlobalDlg", L"AutoPNG", 1, (LPCTSTR)strAppNameINI);
        if (iRes != 0) {
            CheckDlgButton(hDlg, IDC_SETTINGS_AUTO_PNG, BST_CHECKED);
        }

        // IDC_SETTINGS_START_LAST
        iRes = GetPrivateProfileInt(L"SettingsGlobalDlg", L"StartLast", 0, (LPCTSTR)strAppNameINI);
        if (iRes != 0) {
            CheckDlgButton(hDlg, IDC_SETTINGS_START_LAST, BST_CHECKED);
        }

        // Radio buttons
        int ydir = ImageLayers->GetYdir();
        if (ydir) {
            CheckRadioButton(hDlg, IDC_GLOBAL_YPOS_UP, IDC_GLOBAL_YPOS_DOWN, IDC_GLOBAL_YPOS_UP);
        }
        else {
            CheckRadioButton(hDlg, IDC_GLOBAL_YPOS_UP, IDC_GLOBAL_YPOS_DOWN, IDC_GLOBAL_YPOS_DOWN);
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMG_TEMP_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMG_TEMP, szString, MAX_PATH);
            COMDLG_FILTERSPEC imgType[] =
            {
                 { L"All Files", L"*.*" } };
            if (!CCFileOpen(hDlg, szString, &pszFilename, TRUE, 1, imgType, L"")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMG_TEMP, szString);

            return (INT_PTR)TRUE;
        }

        case IDOK:
        {
            GetDlgItemText(hDlg, IDC_IMG_TEMP, szString, MAX_PATH);
            size_t Length = wcslen(szString);

            if (wcscmp(szString + Length - 1, L"\\") == 0) {
                // If the last character is a backslash, delete
                szString[Length - 1] = L'\0';
            }

            wcscpy_s(szTempDir, szString);
            WritePrivateProfileString(L"SettingsGlobalDlg", L"TempDir", szString, (LPCTSTR)strAppNameINI);

            // IDC_SETTINGS_AUTO_PNG
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_AUTO_PNG) == BST_CHECKED) {
                WritePrivateProfileString(L"SettingsGlobalDlg", L"AutoPNG", L"1", (LPCTSTR)strAppNameINI);
                AutoPNG = 1;
            }
            else {
                WritePrivateProfileString(L"SettingsGlobalDlg", L"AutoPNG", L"0", (LPCTSTR)strAppNameINI);
                AutoPNG = 0;
            }

            // IDC_SETTINGS_START_LAST
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_START_LAST) == BST_CHECKED) {
                WritePrivateProfileString(L"SettingsGlobalDlg", L"StartLast", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"SettingsGlobalDlg", L"StartLast", L"0", (LPCTSTR)strAppNameINI);
            }

            // radio buttons
            if (IsDlgButtonChecked(hDlg, IDC_GLOBAL_YPOS_UP)) {
                ImageLayers->SetYdir(1);
                WritePrivateProfileString(L"SettingsGlobalDlg", L"yposDir", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                ImageLayers->SetYdir(0);
                WritePrivateProfileString(L"SettingsGlobalDlg", L"yposDir", L"0", (LPCTSTR)strAppNameINI);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        case IDCANCEL:
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        }
    }

    return (INT_PTR)FALSE;
}

