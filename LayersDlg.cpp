//
// MySETIviewer, a set tools for decoding bitstreams into various formats and manipulating those files
// LayersDlg.cpp
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
// This file contains the dialog callback procedure for Layers and dialog help functions
// 
// V1.0.1	2023-12-20	Initial release
// V1.0.2   2023-12-20  Added Y direction flag for which direction to move image
// V1.1.0   2023-12-21  Separate Display and Layers dialogs from SettingsDlg file
//                      Moved New, Add, Remove, Reload from menus to Layers dialog
//                      Changed Layers dialog to be modeless.
//                      Added ID_UPDATE to allow menu command to casue the Layers dialog
//                      update the entire dialog
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

// Layer class brushes
static HBRUSH hbrSelectedLayer = NULL;
static HBRUSH hbrBackgroundLayer = NULL;
static HBRUSH hbrOverlayLayer = NULL;

static BOOL Touched = FALSE;

int ReplaceListBoxEntry(HWND hDlg, int Control, int Selection, WCHAR* szString);
void SetCurrentLayerSettings(HWND hDlg, int Layer);
void LoadLayerList(HWND hDlg, int CurrentLayer);
void DeleteAllLayers(HWND hDlg);

void ApplyLayers(HWND hDlg);

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
        // The backup needs to possibly change
        // backup configuration in case user cancels
        WCHAR TempConfig[MAX_PATH];
        swprintf_s(TempConfig, MAX_PATH, L"%s\\MySETIviewerTmp.cfg", szTempDir);
        ImageLayers->SaveConfiguration(TempConfig);

        int CurrentLayer;
        CurrentLayer = ImageLayers->GetCurrentLayer();
        
        // set minimum size for overlay order image
        {
            int x, y;
            ImageLayers->GetMinOverlaySize(&x, &y);
            SetDlgItemInt(hDlg, IDC_LAYERS_MIN_X, x, TRUE);
            SetDlgItemInt(hDlg, IDC_LAYERS_MIN_Y, y, TRUE);
        }

        // set minimum size for overlay order image
        {
            int x, y;
            ImageLayers->GetMinOverlaySize(&x, &y);
            SetDlgItemInt(hDlg, IDC_LAYERS_MIN_X, x, TRUE);
            SetDlgItemInt(hDlg, IDC_LAYERS_MIN_Y, y, TRUE);
        }

        LoadLayerList(hDlg, CurrentLayer);
        SetCurrentLayerSettings(hDlg, CurrentLayer);

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
        hwndLayers = NULL;
        
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
            
            SetCurrentLayerSettings(hDlg, Layer);
            Touched = TRUE;

            ApplyLayers(hDlg);

            return (INT_PTR)TRUE;
        }

        case IDC_ADD_LAYER:
        {
            if (ImageLayers->GetNumLayers() >= MAX_LAYERS) {
                MessageBox(hDlg, L"Max layers reached", L"Layers", MB_OK);
                break;
            }

            PWSTR pszFilename;
            COMDLG_FILTERSPEC AllType[] =
            {
                 { L"Image files", L"*.raw" },
                 { L"BMP files", L"*.bmp" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileOpen(hDlg, szCurrentFilename, &pszFilename, FALSE, 3, AllType, L".raw")) {
                break;
            }

            wcscpy_s(szCurrentFilename, pszFilename);
            CoTaskMemFree(pszFilename);

            int iRes = ImageLayers->AddLayer(szCurrentFilename);
            if (iRes != APP_SUCCESS) {
                MessageMySETIviewerError(hDlg, iRes, L"Add layer failure");
                break;
            }

            // add to listbox and set as current layer
            int Layer = ImageLayers->GetNumLayers()-1;
            ImageLayers->SetCurrentLayer(Layer);

            {
                HWND ListHwnd = GetDlgItem(hDlg, IDC_LAYER_LIST);
                WCHAR szString[MAX_PATH];
                int x, y;

                ImageLayers->GetSize(Layer, &x, &y);
                if (ImageLayers->IsLayerEnabled(Layer)) {
                    swprintf_s(szString, MAX_PATH, L"Enabled:  %dHx%dV, %s", x, y,
                        ImageLayers->LayerFilename[Layer]);
                }
                else {
                    swprintf_s(szString, MAX_PATH, L"Disabled: %dHx%dV, %s", x, y,
                        ImageLayers->LayerFilename[Layer]);
                }
                // AddString
                SendMessage(ListHwnd, LB_ADDSTRING, 0, (LPARAM)szString);
                SendMessage(ListHwnd, LB_SETCURSEL, Layer, (LPARAM)0);
            }

            SetCurrentLayerSettings(hDlg,Layer);
            ApplyLayers(hDlg);
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
            if (MessageBox(hDlg, szString, L"Layers", MB_YESNO | MB_DEFBUTTON2) != IDYES) {
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

            ImageLayers->SetCurrentLayer(Selection);

            SetCurrentLayerSettings(hDlg, Selection);

            Touched = TRUE;

            ApplyLayers(hDlg);

            return (INT_PTR)TRUE;
        }

        case IDC_NEW:
        {
            DeleteAllLayers(hDlg);

            Touched = TRUE;
            ShowWindow(hwndImage, SW_HIDE);
            return (INT_PTR)TRUE;
        }

        case IDC_RELOAD:
        {
            int iRes;
            WCHAR TempFile[MAX_PATH];

            // Save this conifguration in temp dir as "MySETIviewerReload.cfg"
            swprintf_s(TempFile, MAX_PATH, L"%s\\MySETIviewerReload.cfg", szTempDir);
            iRes = ImageLayers->SaveConfiguration(TempFile);
            if (iRes != APP_SUCCESS) {
                MessageBox(hDlg, L"Save configuration for reload failed", L"Layers", MB_OK);
                return (INT_PTR)TRUE;
            }
            
            iRes = ImageLayers->LoadConfiguration(TempFile);
            if (iRes != APP_SUCCESS) {
                MessageBox(hDlg, L"Could not reload configuration file", L"Layers", MB_OK);
                return (INT_PTR)TRUE;
            }
            
            SetCurrentLayerSettings(hDlg, ImageLayers->GetCurrentLayer());
            
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

        case ID_UPDATE:
        {
            LoadLayerList(hDlg, ImageLayers->GetCurrentLayer());
            SetCurrentLayerSettings(hDlg, ImageLayers->GetCurrentLayer());
            if (LOWORD(lParam)) {
                ApplyLayers(hDlg);
            }
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
            CurrentLayer = (int)SendMessage(ListHwnd, LB_GETCURSEL, 0, 0);
            if (CurrentLayer < 0) CurrentLayer = 0;
            ImageLayers->SetCurrentLayer(CurrentLayer);
            Touched = FALSE;

            {
                // save window position/size data
                CString csString = L"LayerWindow";
                //RestoreWindowPlacement(hDlg, csString);
                SaveWindowPlacement(hDlg, csString);
            }

            WCHAR TempConfig[MAX_PATH];
            swprintf_s(TempConfig, MAX_PATH, L"%s\\MySETIviewerTmp.cfg", szTempDir);

            if(ImageLayers->GetNumLayers() >0) {
                ImageLayers->SaveConfiguration(TempConfig);
            }
            else {
                // delete MySETIviewerTmp.cfg
                DeleteFile(TempConfig);
            }

            Touched = FALSE;
            if (!KeepOpen) {
                ShowWindow(hDlg, SW_HIDE);
            }

            return (INT_PTR)TRUE;
        }

        case IDCANCEL:
            // backup configuration in case user cancels
            if (Touched) {
                WCHAR TempConfig[MAX_PATH];
                int iRes;

                swprintf_s(TempConfig, MAX_PATH, L"%s\\MySETIviewerTmp.cfg", szTempDir);
                iRes = ImageLayers->LoadConfiguration(TempConfig);
                if (iRes == APP_SUCCESS) {
                    LoadLayerList(hDlg, ImageLayers->GetCurrentLayer());
                    SetCurrentLayerSettings(hDlg, ImageLayers->GetCurrentLayer());
                    ApplyLayers(hDlg);
                }
                else {
                    DeleteAllLayers(hDlg);
                }

                Touched = FALSE;
            }

            if (!KeepOpen) {
                ShowWindow(hDlg, SW_HIDE);
            }

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
void DeleteAllLayers(HWND hDlg)
{
    int NumLayers = ImageLayers->GetNumLayers();
    if (NumLayers == 0) {
        return;
    }
    HWND ListHwnd = GetDlgItem(hDlg, IDC_LAYER_LIST);
    ImageLayers->ReleaseOverlay();
    for (int i = NumLayers - 1; i >= 0; i--) {
        ImageLayers->ReleaseLayer(i);
        // remove from combo box list
        SendMessage(ListHwnd, LB_DELETESTRING, i, 0);
    }

    SetCurrentLayerSettings(hDlg, 0);
    return;
}

//*******************************************************************************
//
// Helper function for SettingsLayerDlg dialog box.
// 
//*******************************************************************************
void LoadLayerList(HWND hDlg, int CurrentLayer)
{
    HWND ListHwnd;
    int NumLayers;
    
    ListHwnd = GetDlgItem(hDlg, IDC_LAYER_LIST);
    NumLayers = ImageLayers->GetNumLayers();

    // clear layer list
    SendMessage(ListHwnd, LB_RESETCONTENT, 0, 0);

    if (CurrentLayer < 0 || NumLayers <= 0) {
        return;
    }
    // IDC_LAYER_LIST
    {
        WCHAR szString[MAX_PATH];
        int x, y;

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

    return;
}

//*******************************************************************************
//
// Helper function for SettingsLayerDlg dialog box.
// 
//*******************************************************************************
void SetCurrentLayerSettings(HWND hDlg, int Layer)
{
    HWND ListHwnd;
    WCHAR szString[MAX_PATH];

    if (ImageLayers->GetNumLayers() != 0) {
        ListHwnd = GetDlgItem(hDlg, IDC_LAYER_LIST);
        SendMessage(ListHwnd, LB_GETTEXT, Layer, (LPARAM)szString);
        SetDlgItemText(hDlg, IDC_LAYER_TEXT, szString);

        // set x,y and color for this layer
        int x, y;
        ImageLayers->SetCurrentLayer(Layer);
        ImageLayers->GetLocation(Layer, &x, &y);
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
    }
    else {
        // There are no layers so blanks current parameter fields
        SetDlgItemText(hDlg, IDC_LAYER_TEXT, L"");

        // set x,y to blank
        SetDlgItemText(hDlg, IDC_X_POS, L"");
        SetDlgItemText(hDlg, IDC_Y_POS, L"");
        //IDC_ENABLE
        CheckDlgButton(hDlg, IDC_ENABLE, BST_UNCHECKED);
        
        // Set Layer background color to default layer background color
        SetDlgItemText(hDlg, IDC_LAYER_COLOR, L"");
    }
    return;
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

