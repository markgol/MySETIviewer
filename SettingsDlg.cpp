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
// V1.0.1	2023-12-20	Initial release
// V1.0.2   2023-12-20  Added Y direction flag for which direction to move image
//                      Separate Display and Layers dialogs from SettingsDlg file
// V1.1.1   2023-12-27  Added, Scale,Zoom ,pan settings
//                      Changed, moved .exe and .ini file information to About dialog
//                      Added, Show status bar flag 
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


        float sf, px, py;
        ImgDlg->GetScalePos(&sf, &px, &py);

        swprintf_s(szString, MAX_PATH, L"%.3f", sf);
        SetDlgItemText(hDlg, IDC_SCALE_FACTOR, szString);

        swprintf_s(szString, MAX_PATH, L"%.3f", px);
        SetDlgItemText(hDlg, IDC_PAN_OFFSET_X, szString);

        swprintf_s(szString, MAX_PATH, L"%.3f", py);
        SetDlgItemText(hDlg, IDC_PAN_OFFSET_Y, szString);

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

        // IDC_SETTINGS_STATUSBAR
        iRes = GetPrivateProfileInt(L"SettingsGlobalDlg", L"ShowStatusBar", 1, (LPCTSTR)strAppNameINI);
        if (iRes != 0) {
            CheckDlgButton(hDlg, IDC_SETTINGS_STATUSBAR, BST_CHECKED);
        }

        // Radio buttons
        int ydir = ImageLayers->GetYdir();
        if (ydir) {
            CheckRadioButton(hDlg, IDC_GLOBAL_YPOS_UP, IDC_GLOBAL_YPOS_DOWN, IDC_GLOBAL_YPOS_UP);
        }
        else {
            CheckRadioButton(hDlg, IDC_GLOBAL_YPOS_UP, IDC_GLOBAL_YPOS_DOWN, IDC_GLOBAL_YPOS_DOWN);
        }

        if (KeepOpen) {
            CheckRadioButton(hDlg, IDC_GLOBAL_KEEP_OPEN, IDC_GLOBAL_CLOSE_ON_OKCANCEL, IDC_GLOBAL_KEEP_OPEN);
        }
        else {
            CheckRadioButton(hDlg, IDC_GLOBAL_KEEP_OPEN, IDC_GLOBAL_CLOSE_ON_OKCANCEL, IDC_GLOBAL_CLOSE_ON_OKCANCEL);
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
            int iRes;
            float sf, px, py;
            
            GetDlgItemText(hDlg, IDC_SCALE_FACTOR, szString, MAX_PATH);
            iRes = swscanf_s(szString, L"%f", &sf);
            if (iRes != 1) {
                MessageBox(hDlg, L"Bad scale factor parameter", L"Globals", MB_OK);
                return (INT_PTR)TRUE;
            }

            GetDlgItemText(hDlg, IDC_PAN_OFFSET_X, szString, MAX_PATH);
            iRes = swscanf_s(szString, L"%f", &px);
            if (iRes != 1) {
                MessageBox(hDlg, L"Bad pan offset X parameter", L"Globals", MB_OK);
                return (INT_PTR)TRUE;
            }
            GetDlgItemText(hDlg, IDC_PAN_OFFSET_Y, szString, MAX_PATH);
            iRes = swscanf_s(szString, L"%f", &py);
            if (iRes != 1) {
                MessageBox(hDlg, L"Bad pan offset Y parameter", L"Globals", MB_OK);
                return (INT_PTR)TRUE;
            }
           
            ImgDlg->SetScalePos(sf, px, py);

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

            // IDC_SETTINGS_STATUSBAR
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_STATUSBAR) == BST_CHECKED) {
                WritePrivateProfileString(L"SettingsGlobalDlg", L"ShowStatusBar", L"1", (LPCTSTR)strAppNameINI);
                ShowStatusBar = TRUE;
                // create status bar
                if (hwndImage && ImgDlg->StatusBarExists()) {
                    ImgDlg->ShowStatusBar(TRUE);
                }
            }
            else {
                WritePrivateProfileString(L"SettingsGlobalDlg", L"ShowStatusBar", L"0", (LPCTSTR)strAppNameINI);
                ShowStatusBar = FALSE;
                ImgDlg->ShowStatusBar(FALSE);
            }

            // radio buttons
            if (IsDlgButtonChecked(hDlg, IDC_GLOBAL_YPOS_UP)) {
                ImageLayers->SetYdir(1);
                // (update the Layers dialog and apply to Image Window)
                SendMessage(hwndLayers, WM_COMMAND, ID_UPDATE, 1);
                WritePrivateProfileString(L"SettingsGlobalDlg", L"yposDir", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                ImageLayers->SetYdir(0);
                // (update the Layers dialog and apply to Image Window)
                SendMessage(hwndLayers, WM_COMMAND, ID_UPDATE, 1);
                WritePrivateProfileString(L"SettingsGlobalDlg", L"yposDir", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_GLOBAL_KEEP_OPEN)) {
                // This should set Display and Layers to to SW_SHOW
                KeepOpen = TRUE;
                WritePrivateProfileString(L"SettingsGlobalDlg", L"KeepOpen", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                KeepOpen = FALSE;
                WritePrivateProfileString(L"SettingsGlobalDlg", L"KeepOpen", L"0", (LPCTSTR)strAppNameINI);
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

