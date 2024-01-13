//
// MySETIviewer, a set tools for decoding bitstreams into various formats and manipulating those files
// DisplayDlg.cpp
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
// V1.0.1	2023-12-20	Initial release
// V1.0.2   2023-12-20  Added Y direction flag for which direction to move image
// V1.1.0   2023-12-21  Separate Display and Layers dialogs from SettingsDlg file
//                      Added ID_UPDATE to allow menu command to cause the Display dialog
//                      update the entire dialog
// V1.1.1   2023-12-27  Added, window position reset
//                      Correction, reset pan poistion to 0,0 instead of 1,1
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

void ApplyDisplay(HWND hDlg);
void LoadDisplaySettings(HWND hDlg);

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
        LoadDisplaySettings(hDlg);

        int ResetWindows = GetPrivateProfileInt(L"GlobalSettings", L"ResetWindows", 0, (LPCTSTR)strAppNameINI);
        if (!ResetWindows) {
            CString csString = L"ImageDisplay";
            RestoreWindowPlacement(hDlg, csString);
        }

        if (!Displays->IsGridEnabled()) {
            CheckDlgButton(hDlg, IDC_ENABLE, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_ENABLE, BST_CHECKED);
        }

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

        case IDC_ENABLE:
        {
            if (IsDlgButtonChecked(hDlg, IDC_ENABLE) == BST_CHECKED) {
                Displays->EnableGrid(TRUE);
            }
            else {
                Displays->EnableGrid(FALSE);
            }
            ApplyDisplay(hDlg);
            return (INT_PTR)TRUE;
        }

        case IDC_SCALE_RESET:
        {
            if (ImgDlg) {
                ImgDlg->SetScale(1.0f);
            }
            ApplyDisplay(hDlg);
            return (INT_PTR)TRUE;
        }

        case IDC_POS_RESET:
        {
            if (ImgDlg) {
                ImgDlg->SetPan(0.0f, 0.0f);
            }
            ApplyDisplay(hDlg);
            return (INT_PTR)TRUE;
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

        case ID_UPDATE:
            LoadDisplaySettings(hDlg);
            if (LOWORD(lParam)) {
                ApplyDisplay(hDlg);
            }
            return (INT_PTR)TRUE;

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
            
            if (!KeepOpen) {
                ShowWindow(hDlg, SW_HIDE);
            }
            
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

            SetDlgItemText(hDlg, IDC_GAP_MINOR, L"");
            SetDlgItemText(hDlg, IDC_GAP_MAJOR, L"");
            SetDlgItemText(hDlg, IDC_BACKGROUND, L"");

            if (!KeepOpen) {
                ShowWindow(hDlg, SW_HIDE);
            }

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
void LoadDisplaySettings(HWND hDlg)
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
