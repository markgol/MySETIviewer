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
// V0.1.0.1 2023-12-08  Initial Pre Release
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
#include "Appfunctions.h"

// local statics
static HBRUSH hbrBackground = NULL;
static HBRUSH hbrGapMajor = NULL;
static HBRUSH hbrGapMinor = NULL;
static HBRUSH hbrSelectedLayer = NULL;
static HBRUSH hbrDefaultLayer = NULL;
static BOOL Touched = FALSE;
static HWND hLayerEnable = NULL;

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

            return (INT_PTR)TRUE;
        }

        case IDOK:
        {
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
                WritePrivateProfileString(L"SettingsDlg", CustomColor, szString, (LPCTSTR)strAppNameINI);
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
                // disable everything except default color selection
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

                //IDC_SELECT_LAYER_COLOR
                hControl = GetDlgItem(hDlg, IDC_SELECT_LAYER_COLOR);
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

        // IDC_COMBO_LAYER_LIST
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
        }
        // IDC_X_POS, IDC_Y_POS
        // set x to current layer X location value
        ImageLayers->GetLocation(CurrentLayer, &x, &y);
        
        SetDlgItemInt(hDlg, IDC_X_POS, x, TRUE);

        // set x to current layer X location value
        SetDlgItemInt(hDlg, IDC_Y_POS, y, TRUE);
        // IDC_LAYER_COLOR
        // set current color in WM_CTLCOLORSTATIC

        //IDC_ENABLE
        if (ImageLayers->IsLayerEnabled(CurrentLayer)) {
            CheckDlgButton(hDlg, IDC_ENABLE, BST_CHECKED);
        }
        hLayerEnable = GetDlgItem(hDlg, IDC_ENABLE);

        Touched = FALSE;
        return (INT_PTR)TRUE;
    }

    case WM_DESTROY:
        // must destroy any brushes created

        if (hbrSelectedLayer) {
            DeleteObject(hbrSelectedLayer);
            hbrSelectedLayer = NULL;
        }
        if (hbrDefaultLayer) {
            DeleteObject(hbrDefaultLayer);
            hbrDefaultLayer = NULL;
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

        hThis = GetDlgItem(hDlg, IDC_DEFAULT_COLOR);
        if (hThis == hControl) {
            if (hbrDefaultLayer) {
                DeleteObject(hbrDefaultLayer);
            }
            ThisColor = ImageLayers->GetDefaultColor();
            hbrDefaultLayer = CreateSolidBrush(ThisColor);
            return (INT_PTR)hbrDefaultLayer;
        }

        hThis = GetDlgItem(hDlg, IDC_OVERLAY_COLOR);
        if (hThis == hControl) {
            if (hbrDefaultLayer) {
                DeleteObject(hbrDefaultLayer);
            }
            ThisColor = ImageLayers->GetOverlayColor();
            hbrDefaultLayer = CreateSolidBrush(ThisColor);
            return (INT_PTR)hbrDefaultLayer;
        }

        return FALSE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDC_ENABLE:
        {
            if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == BN_DOUBLECLICKED) {
                if (IsDlgButtonChecked(hDlg, IDC_ENABLE) == BST_CHECKED) {
                    ImageLayers->EnableLayer(ImageLayers->GetCurrentLayer());
                }
                else {
                    ImageLayers->DisableLayer(ImageLayers->GetCurrentLayer());
                }
            }
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

            Touched = TRUE;
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
            cc.rgbResult = (DWORD)ImageLayers->GetLayerColor(CurrentLayer);
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorW(&cc) == TRUE)
            {
                ImageLayers->SetLayerColor(CurrentLayer, (COLORREF)cc.rgbResult);
            }
            // force redraw static control
            SetDlgItemText(hDlg, IDC_LAYER_COLOR, L"");

            Touched = TRUE;
            return (INT_PTR)TRUE;
        }

        case IDC_SELECT_DEFAULT_COLOR:
        {
            // use common dialog color picker
            // this is old school method but doesn't require MFC or XAML

            CHOOSECOLOR cc;                 // common dialog box structure 

            // Initialize CHOOSECOLOR 
            ZeroMemory(&cc, sizeof(cc));
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hDlg;
            cc.lpCustColors = (LPDWORD)CustomColorTable;
            cc.rgbResult = (DWORD)ImageLayers->GetDefaultColor();
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorW(&cc) == TRUE)
            {
                ImageLayers->SetDefaultColor((COLORREF)cc.rgbResult);
            }
            // force redraw static control
            SetDlgItemText(hDlg, IDC_DEFAULT_COLOR, L"");

            Touched = TRUE;
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
            return (INT_PTR)TRUE;
        }

        case IDC_APPLY:
        {
            int xnewsize, ynewsize;
            int iRes;

            iRes = ImageLayers->GetNewOverlaySize(&xnewsize, &ynewsize);
            if (iRes != APP_SUCCESS) {
                MessageBox(hDlg, L"Creating Overlay image failed", L"Layers", MB_OK);
                return (INT_PTR)TRUE;
            }
            iRes = ImageLayers->ReleaseOverlay();
            iRes = ImageLayers->CreateOverlay(xnewsize, ynewsize);
            if (iRes == APP_SUCCESS) {
                iRes = ImageLayers->UpdateOverlay();
            }
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
            return (INT_PTR)TRUE;
        }

        case IDOK:
        {
            WCHAR CustomColor[20];
            WCHAR szString[20];

            for (int i = 0; i < 16; i++) {
                swprintf_s(CustomColor, 20, L"CustomColorTable%d", i);
                swprintf_s(szString, 20, L"%u", CustomColorTable[i]);
                WritePrivateProfileString(L"SettingsDlg", CustomColor, szString, (LPCTSTR)strAppNameINI);
            }

            COLORREF Color;
            Color = ImageLayers->GetDefaultColor();
            swprintf_s(szString, 20, L"%u", Color);
            WritePrivateProfileString(L"SettingsDlg", L"DefaultColor", szString, (LPCTSTR)strAppNameINI);
            
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

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        case IDCANCEL:
            // backup configuration in case user cancels
            if (Touched) {
                WCHAR TempConfig[MAX_PATH];
                swprintf_s(TempConfig, MAX_PATH, L"%s\\MySETIviewerTmp.cfg", szTempDir);
                ImageLayers->LoadConfiguration(TempConfig);
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
        GetPrivateProfileString(L"SettingsGlobalDlg", L"BMPresults", L"BMP files\\last.bmp", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BMP_RESULTS, szString);

        GetPrivateProfileString(L"SettingsGlobalDlg", L"TempDir", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMG_TEMP, szString);

        // todo: move to about box
        SetDlgItemText(hDlg, IDC_INI_FILE, strAppNameINI);
        SetDlgItemText(hDlg, IDC_EXE_FILE, strAppNameEXE);

        //IDC_SETTINGS_DISPLAY_RESULTS
        iRes = GetPrivateProfileInt(L"SettingsGlobalDlg", L"DisplayResults", 1, (LPCTSTR)strAppNameINI);
        if (iRes != 0) {
            CheckDlgButton(hDlg, IDC_SETTINGS_DISPLAY_RESULTS, BST_CHECKED);
        }

        // IDC_SETTINGS_AUTOSCALE_RESULTS
        iRes = GetPrivateProfileInt(L"SettingsGlobalDlg", L"AutoScaleResults", 1, (LPCTSTR)strAppNameINI);
        if (iRes != 0) {
            CheckDlgButton(hDlg, IDC_SETTINGS_AUTOSCALE_RESULTS, BST_CHECKED);
        }

        // IDC_SETTINGS_AUTO_PNG
        iRes = GetPrivateProfileInt(L"SettingsGlobalDlg", L"AutoPNG", 1, (LPCTSTR)strAppNameINI);
        if (iRes != 0) {
            CheckDlgButton(hDlg, IDC_SETTINGS_AUTO_PNG, BST_CHECKED);
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BMP_RESULTS_BROWSE:
        {
            GetDlgItemText(hDlg, IDC_BMP_RESULTS, szString, MAX_PATH);

            PWSTR pszFilename;
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
                 { L"All Files", L"*.*" }            };
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
            GetDlgItemText(hDlg, IDC_BMP_RESULTS, szString, MAX_PATH);
            wcscpy_s(szBMPFilename, szString);
            WritePrivateProfileString(L"SettingsGlobalDlg", L"BMPresults", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMG_TEMP, szString, MAX_PATH);
            size_t Length = wcslen(szString);

            if (wcscmp(szString + Length - 1, L"\\") == 0) {
                // If the last character is a backslash, delete
                szString[Length - 1] = L'\0';
            }

            wcscpy_s(szTempDir, szString);
            WritePrivateProfileString(L"SettingsGlobalDlg", L"TempDir", szString, (LPCTSTR)strAppNameINI);

            //IDC_SETTINGS_DISPLAY_RESULTS
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_DISPLAY_RESULTS) == BST_CHECKED) {
                WritePrivateProfileString(L"SettingsGlobalDlg", L"DisplayResults", L"1", (LPCTSTR)strAppNameINI);
                DisplayResults = 1;
            }
            else {
                WritePrivateProfileString(L"SettingsGlobalDlg", L"DisplayResults", L"0", (LPCTSTR)strAppNameINI);
                DisplayResults = 0;
            }

            // IDC_SETTINGS_AUTOSCALE_RESULTS
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_AUTOSCALE_RESULTS) == BST_CHECKED) {
                WritePrivateProfileString(L"SettingsGlobalDlg", L"AutoScaleResults", L"1", (LPCTSTR)strAppNameINI);
                AutoScaleResults = 1;
            }
            else {
                WritePrivateProfileString(L"SettingsGlobalDlg", L"AutoScaleResults", L"0", (LPCTSTR)strAppNameINI);
                AutoScaleResults = 0;
            }

            // IDC_SETTINGS_AUTO_PNG
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_AUTO_PNG) == BST_CHECKED) {
                WritePrivateProfileString(L"SettingsGlobalDlg", L"AutoPNG", L"1", (LPCTSTR)strAppNameINI);
                AutoPNG = 1;
            }
            else {
                WritePrivateProfileString(L"SettingsGlobalDlg", L"AutoPNG", L"0", (LPCTSTR)strAppNameINI);
                AutoPNG = 0;
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
