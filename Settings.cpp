//
// MySETIviewer, a set tools for decoding bitstreams into various formats and manipulating those files
// Settings.cpp
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
// V0.1.0.1 2023-11-27  Initial Pre Release
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
#include "Globals.h"
#include "imageheader.h"
#include "FileFunctions.h"

// local statics
static HBRUSH hbrBackground = NULL;
static HBRUSH hbrGapMajor = NULL;
static HBRUSH hbrGapMinor = NULL;
static HBRUSH hbrSelectedLayer = NULL;

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
        int iRes;
        GetPrivateProfileString(L"SettingsDisplayDlg", L"BMPresults", L"BMP files\\last.bmp", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BMP_RESULTS, szString);

        GetPrivateProfileString(L"SettingsDisplayDlg", L"TempImageFilename", L"working.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMG_TEMP, szString);

        // todo: move to about box
        SetDlgItemText(hDlg, IDC_INI_FILE, strAppNameINI);
        SetDlgItemText(hDlg, IDC_EXE_FILE, strAppNameEXE);

        //IDC_SETTINGS_DISPLAY_RESULTS
        iRes = GetPrivateProfileInt(L"SettingsDisplayDlg", L"DisplayResults", 1, (LPCTSTR)strAppNameINI);
        if (iRes != 0) {
            CheckDlgButton(hDlg, IDC_SETTINGS_DISPLAY_RESULTS, BST_CHECKED);
        }

        // IDC_SETTINGS_AUTOSCALE_RESULTS
        iRes = GetPrivateProfileInt(L"SettingsDisplayDlg", L"AutoScaleResults", 1, (LPCTSTR)strAppNameINI);
        if (iRes != 0) {
            CheckDlgButton(hDlg, IDC_SETTINGS_AUTOSCALE_RESULTS, BST_CHECKED);
        }
        
        // IDC_SETTINGS_AUTO_PNG
        iRes = GetPrivateProfileInt(L"SettingsDisplayDlg", L"AutoPNG", 1, (LPCTSTR)strAppNameINI);
        if (iRes != 0) {
            CheckDlgButton(hDlg, IDC_SETTINGS_AUTO_PNG, BST_CHECKED);
        }

        // IDC_GRID_X_MAJOR
        SetDlgItemInt(hDlg, IDC_GRID_X_MAJOR, GridXmajor, TRUE);
        // IDC_GRID_X_MINOR
        SetDlgItemInt(hDlg, IDC_GRID_X_MINOR, GridXminor, TRUE);

        // IDC_GRID_Y_MAJOR
        SetDlgItemInt(hDlg, IDC_GRID_Y_MAJOR, GridYmajor, TRUE);
        // IDC_GRID_Y_MINOR
        SetDlgItemInt(hDlg, IDC_GRID_Y_MINOR, GridYminor, TRUE);

        // IDC_GAP_X_MAJOR
        SetDlgItemInt(hDlg, IDC_GAP_X_MAJOR, GapXmajor, TRUE);
        // IDC_GAP_X_MINOR
        SetDlgItemInt(hDlg, IDC_GAP_X_MINOR, GapXminor, TRUE);

        // IDC_GAP_Y_MAJOR
        SetDlgItemInt(hDlg, IDC_GAP_Y_MAJOR, GapYmajor, TRUE);
        // IDC_GAP_Y_MINOR
        SetDlgItemInt(hDlg, IDC_GAP_Y_MINOR, GapYminor, TRUE);

        // These controls are updated in the WM_CTLCOLORSTATIC message handler
        // IDC_BACKGROUND
        // IDC_GAP_MAJOR
        // IDC_GAP_MINOR

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
            hbrBackground = CreateSolidBrush(rgbBackground);
            return (INT_PTR)hbrBackground;
        }

        hThis = GetDlgItem(hDlg, IDC_GAP_MAJOR);
        if (hThis == hControl) {
            if (hbrGapMajor) {
                DeleteObject(hbrGapMajor);
            }
            hbrGapMajor = CreateSolidBrush(rgbGapMajor);
            return (INT_PTR)hbrGapMajor;
        }

        hThis = GetDlgItem(hDlg, IDC_GAP_MINOR);
        if (hThis == hControl) {
            if (hbrGapMinor) {
                DeleteObject(hbrGapMinor);
            }
            hbrGapMinor = CreateSolidBrush(rgbGapMinor);
            return (INT_PTR)hbrGapMinor;
        }
        return FALSE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BMP_RESULTS_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_BMP_RESULTS, szString, MAX_PATH);
            COMDLG_FILTERSPEC BMPType[] =
            {
                 { L"BMP files", L"*.bmp" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, BMPType, L".bmp")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BMP_RESULTS, szString);

            return (INT_PTR)TRUE;
        }

        case IDC_IMG_TEMP_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMG_TEMP, szString, MAX_PATH);
            COMDLG_FILTERSPEC imgType[] =
            {
                 { L"Image Files", L" * .raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, imgType, L".raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMG_TEMP, szString);

            return (INT_PTR)TRUE;
        }

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
            cc.rgbResult = (DWORD)rgbBackground;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorW(&cc) == TRUE)
            {
                rgbBackground = (COLORREF)cc.rgbResult;
            }
            // force redraw static control
            SetDlgItemText(hDlg, IDC_BACKGROUND, L"");

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
            cc.rgbResult = (DWORD)rgbGapMajor;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorW(&cc) == TRUE)
            {
                rgbGapMajor = (COLORREF)cc.rgbResult;
            }
            // force redraw static control
            SetDlgItemText(hDlg, IDC_GAP_MAJOR, L"");

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
            cc.rgbResult = (DWORD)rgbGapMinor;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorW(&cc) == TRUE)
            {
                rgbGapMinor = (COLORREF)cc.rgbResult;
            }
            // force redraw static control
            SetDlgItemText(hDlg, IDC_GAP_MINOR, L"");

            return (INT_PTR)TRUE;
        }
        case IDC_APPLY:
        {
            GetDlgItemText(hDlg, IDC_BMP_RESULTS, szString, MAX_PATH);
            wcscpy_s(szBMPFilename, szString);

            GetDlgItemText(hDlg, IDC_IMG_TEMP, szString, MAX_PATH);
            wcscpy_s(szTempImageFilename, szString);

            //IDC_SETTINGS_DISPLAY_RESULTS
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_DISPLAY_RESULTS) == BST_CHECKED) {
                DisplayResults = 1;
            }
            else {
                DisplayResults = 0;
            }

            // IDC_SETTINGS_AUTOSCALE_RESULTS
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_AUTOSCALE_RESULTS) == BST_CHECKED) {
                AutoScaleResults = 1;
            }
            else {
                AutoScaleResults = 0;
            }

            // IDC_SETTINGS_AUTO_PNG
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_AUTO_PNG) == BST_CHECKED) {
                AutoPNG = 1;
            }
            else {
                AutoPNG = 0;
            }

            BOOL bSuccess;

            // IDC_GRID_X_MAJOR
            GridXmajor = GetDlgItemInt(hDlg, IDC_GRID_X_MAJOR, &bSuccess, TRUE);

            // IDC_GRID_X_MINOR
            GridXminor = GetDlgItemInt(hDlg, IDC_GRID_X_MINOR, &bSuccess, TRUE);

            // IDC_GRID_Y_MAJOR
            GridYmajor = GetDlgItemInt(hDlg, IDC_GRID_Y_MAJOR, &bSuccess, TRUE);

            // IDC_GRID_Y_MINOR
            GridYminor = GetDlgItemInt(hDlg, IDC_GRID_Y_MINOR, &bSuccess, TRUE);

            // IDC_GAP_X_MAJOR
            GapXmajor = GetDlgItemInt(hDlg, IDC_GAP_X_MAJOR, &bSuccess, TRUE);

            // IDC_GAP_X_MINOR
            GapXminor = GetDlgItemInt(hDlg, IDC_GAP_X_MINOR, &bSuccess, TRUE);

            // IDC_GAP_Y_MAJOR
            GapYmajor = GetDlgItemInt(hDlg, IDC_GAP_Y_MAJOR, &bSuccess, TRUE);

            // IDC_GAP_Y_MINOR
            GapYminor = GetDlgItemInt(hDlg, IDC_GAP_Y_MINOR, &bSuccess, TRUE);

            // CalculateDisplayExtent();

            if (hwndImage != NULL) {
                PostMessage(hwndImage, WM_PAINT, IDC_GENERATE_BMP, 0l);
                ShowWindow(hwndImage, SW_SHOW);
            }

            return (INT_PTR)TRUE;
        }

        case IDOK:
        {
            GetDlgItemText(hDlg, IDC_BMP_RESULTS, szString, MAX_PATH);
            wcscpy_s(szBMPFilename, szString);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"BMPresults", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMG_TEMP, szString, MAX_PATH);
            wcscpy_s(szTempImageFilename, szString);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"TempImageFilename", szString, (LPCTSTR)strAppNameINI);

            //IDC_SETTINGS_DISPLAY_RESULTS
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_DISPLAY_RESULTS) == BST_CHECKED) {
                WritePrivateProfileString(L"SettingsDisplayDlg", L"DisplayResults", L"1", (LPCTSTR)strAppNameINI);
                DisplayResults = 1;
            }
            else {
                WritePrivateProfileString(L"SettingsDisplayDlg", L"DisplayResults", L"0", (LPCTSTR)strAppNameINI);
                DisplayResults = 0;
            }

            // IDC_SETTINGS_AUTOSCALE_RESULTS
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_AUTOSCALE_RESULTS) == BST_CHECKED) {
                WritePrivateProfileString(L"SettingsDisplayDlg", L"AutoScaleResults", L"1", (LPCTSTR)strAppNameINI);
                AutoScaleResults = 1;
            }
            else {
                WritePrivateProfileString(L"SettingsDisplayDlg", L"AutoScaleResults", L"0", (LPCTSTR)strAppNameINI);
                AutoScaleResults = 0;
            }

            // IDC_SETTINGS_AUTO_PNG
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_AUTO_PNG) == BST_CHECKED) {
                WritePrivateProfileString(L"SettingsDisplayDlg", L"AutoPNG", L"1", (LPCTSTR)strAppNameINI);
                AutoPNG = 1;
            }
            else {
                WritePrivateProfileString(L"SettingsDisplayDlg", L"AutoPNG", L"0", (LPCTSTR)strAppNameINI);
                AutoPNG = 0;
            }

            BOOL bSuccess;

            // IDC_GRID_X_MAJOR
            GetDlgItemText(hDlg, IDC_GRID_X_MAJOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GridXmajor", szString, (LPCTSTR)strAppNameINI);
            GridXmajor = GetDlgItemInt(hDlg, IDC_GRID_X_MAJOR, &bSuccess, TRUE);

            // IDC_GRID_X_MINOR
            GetDlgItemText(hDlg, IDC_GRID_X_MINOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GridXminor", szString, (LPCTSTR)strAppNameINI);
            GridXminor = GetDlgItemInt(hDlg, IDC_GRID_X_MINOR, &bSuccess, TRUE);

            // IDC_GRID_Y_MAJOR
            GetDlgItemText(hDlg, IDC_GRID_Y_MAJOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GridYmajor", szString, (LPCTSTR)strAppNameINI);
            GridYmajor = GetDlgItemInt(hDlg, IDC_GRID_Y_MAJOR, &bSuccess, TRUE);

            // IDC_GRID_Y_MINOR
            GetDlgItemText(hDlg, IDC_GRID_Y_MINOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GridYminor", szString, (LPCTSTR)strAppNameINI);
            GridYminor = GetDlgItemInt(hDlg, IDC_GRID_Y_MINOR, &bSuccess, TRUE);

            // IDC_GAP_X_MAJOR
            GetDlgItemText(hDlg, IDC_GAP_X_MAJOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GapXmajor", szString, (LPCTSTR)strAppNameINI);
            GapXmajor = GetDlgItemInt(hDlg, IDC_GAP_X_MAJOR, &bSuccess, TRUE);

            // IDC_GAP_X_MINOR
            GetDlgItemText(hDlg, IDC_GAP_X_MINOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GapXminor", szString, (LPCTSTR)strAppNameINI);
            GapXminor = GetDlgItemInt(hDlg, IDC_GAP_X_MINOR, &bSuccess, TRUE);

            // IDC_GAP_Y_MAJOR
            GetDlgItemText(hDlg, IDC_GAP_Y_MAJOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GapYmajor", szString, (LPCTSTR)strAppNameINI);
            GapYmajor = GetDlgItemInt(hDlg, IDC_GAP_Y_MAJOR, &bSuccess, TRUE);

            // IDC_GAP_Y_MINOR
            GetDlgItemText(hDlg, IDC_GAP_Y_MINOR, szString, MAX_PATH);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"GapYminor", szString, (LPCTSTR)strAppNameINI);
            GapYminor = GetDlgItemInt(hDlg, IDC_GAP_Y_MINOR, &bSuccess, TRUE);

            // save color settings
            swprintf_s(szString, MAX_PATH, L"%u", rgbBackground);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"rgbBackground", szString, (LPCTSTR)strAppNameINI);

            swprintf_s(szString, MAX_PATH, L"%u", rgbGapMajor);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"rgbGapMajor", szString, (LPCTSTR)strAppNameINI);

            swprintf_s(szString, MAX_PATH, L"%u", rgbGapMinor);
            WritePrivateProfileString(L"SettingsDisplayDlg", L"rgbGapMinor", szString, (LPCTSTR)strAppNameINI);

            for (int i = 0; i < 16; i++) {
                WCHAR CustomColor[20];
                swprintf_s(CustomColor, 20, L"CustomColorTable%d", i);
                swprintf_s(szString, MAX_PATH, L"%u", CustomColorTable[i]);
                WritePrivateProfileString(L"SettingsDisplayDlg", CustomColor, szString, (LPCTSTR)strAppNameINI);
            }

            // CalculateDisplayExtent();

            if (hwndImage != NULL) {
                PostMessage(hwndImage, WM_PAINT, IDC_GENERATE_BMP, 0l);
                ShowWindow(hwndImage, SW_SHOW);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }

    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for SettingsLayersDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK SettingsLayersDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        return (INT_PTR)TRUE;
    }

    case WM_DESTROY:
        // must destroy any brushes created

        if (hbrSelectedLayer) {
            DeleteObject(hbrSelectedLayer);
            hbrSelectedLayer = NULL;
        }
        return (INT_PTR)TRUE;

    case WM_CTLCOLORSTATIC:
    {
        // This is specifically to color the static controls for:
        // IDC_GAP
        HANDLE hControl;

        HDC hdcStatic = (HDC)wParam; // not used here
        hControl = (HANDLE)lParam;

        HANDLE hThis;

        hThis = GetDlgItem(hDlg, IDC_LAYER_COLOR);
        if (hThis == hControl) {
            if (hbrSelectedLayer) {
                DeleteObject(hbrSelectedLayer);
            }
            // todo: replace this with actual layer color selection;
            //       This is skeleton only
            hbrSelectedLayer = CreateSolidBrush(rgbGapMinor);
            return (INT_PTR)hbrSelectedLayer;
        }
        return FALSE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_SELECT_LAYER_COLOR:
        {
            // use common dialog color picker
            // this is old school method but doesn't require MFC or XAML

            CHOOSECOLOR cc;                 // common dialog box structure 
            
            // todo: The rgbGapMinor needs to be replaced by Layer color

            // Initialize CHOOSECOLOR 
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hDlg;
            cc.lpCustColors = (LPDWORD)CustomColorTable;
            cc.rgbResult = (DWORD)rgbGapMinor;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorW(&cc) == TRUE)
            {
                rgbGapMinor = (COLORREF)cc.rgbResult;
            }
            // force redraw static control
            SetDlgItemText(hDlg, IDC_LAYER_COLOR, L"");

            return (INT_PTR)TRUE;
        }
        case IDC_APPLY:
        {
            // todo:  update layer settings on display

            return (INT_PTR)TRUE;
        }

        case IDOK:
        {
            for (int i = 0; i < 16; i++) {
                WCHAR CustomColor[20];
                swprintf_s(CustomColor, 20, L"CustomColorTable%d", i);
                swprintf_s(szString, MAX_PATH, L"%u", CustomColorTable[i]);
                WritePrivateProfileString(L"SettingsLayersDlg", CustomColor, szString, (LPCTSTR)strAppNameINI);
            }

            // todo:  update layer settings on display

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }

    return (INT_PTR)FALSE;
}
