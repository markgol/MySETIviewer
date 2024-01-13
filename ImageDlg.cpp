//
// MySETIviewer, a set tools for decoding bitstreams into various formats and manipulating those files
// ImageDlg.cpp
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
// If not, see < https://www.gnu.org/licenses/>. x
// 
// This file contains the dialog callback procedures for the image dialog
// 
// V1.0.1	2023-12-20	Initial release
// V1.1.2   2023-12-27  Fixed parameter in IDC_BMP_GENERATE when x,y sizes are the same
//                      Added window position reset
//                      Added Action menu to Image Window
//                          Reset Pan
//                          Reset Zoom
//                          Close
//                      Changed Image Window, does not close with ESC or Enter keys
// 
// This handles all the actual display of the bitmap generated
//
#include "framework.h"
#include "resource.h"
#include <shobjidl.h>
#include <winver.h>
#include <windowsx.h>
#include <vector>
#include <atlstr.h>
#include <strsafe.h>
#include <shellapi.h>
#include <d2d1.h>
#include <d2d1_1.h>
#pragma comment(lib, "d2d1.lib")
#include "globals.h"
#include "AppFunctions.h"
#include "ImageDialog.h"

extern void ApplyDisplay(HWND hDlg);

//*******************************************************************************
//
// Message handler for ImageDlg dialog box.
// This window is used to display an image or BMP file
// It is a modeless dialog.
// 
//*******************************************************************************
INT_PTR CALLBACK ImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    //    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {

    case WM_INITDIALOG:
    {
        ImgDlg->InitializeDirect2D();

        int ResetWindows = GetPrivateProfileInt(L"GlobalSettings", L"ResetWindows", 0, (LPCTSTR)strAppNameINI);
        if (!ResetWindows) {
            CString csString = L"ImageWindow";
            RestoreWindowPlacement(hDlg, csString);
        }

        ImgDlg->CreateStatusBar(hDlg, ID_IMG_STATUSBAR, hInst);
        if (ShowStatusBar) {
            ImgDlg->ShowStatusBar(TRUE);
        }
        else {
            ImgDlg->ShowStatusBar(FALSE);
        }
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDCANCEL:
        case IDOK:
            return (INT_PTR)TRUE;

        case IDC_EXIT:
        {
            // save window position/size data
            CString csString = L"ImageWindow";
            SaveWindowPlacement(hDlg, csString);
            ShowWindow(hwndImage, SW_HIDE);
            return (INT_PTR)TRUE;
        }

        case IDM_RESET_ZOOM:
        {
            if (ImgDlg) {
                ImgDlg->SetScale(1.0f);
            }
            ImgDlg->Repaint();
            ImgDlg->UpdateStatusBar(hDlg);
            return (INT_PTR)TRUE;
        }

        case IDM_RESET_PAN:
        {
            if (ImgDlg) {
                ImgDlg->SetPan(0.0f, 0.0f);
            }
            ImgDlg->Repaint();
            ImgDlg->UpdateStatusBar(hDlg);
            return (INT_PTR)TRUE;
        }

        case IDC_GENERATE_BMP:
        {
            int xsize, ysize;
            COLORREF* Image;
            Displays->GetDisplay(&Image, &xsize, &ysize);
            
            if(ImgDlg->LoadCOLORREFimage(hwndImage, xsize, ysize,Image)) {
                ImgDlg->Repaint();
                ImgDlg->UpdateStatusBar(hDlg);
            }
            return (INT_PTR)TRUE;
        }

        default:
            return (INT_PTR)FALSE;
        } // end of WM_COMMAND

    case WM_SYSCOMMAND:
    {
        if (wParam == SC_CLOSE) {
            // save window position/size data
            CString csString = L"ImageWindow";
            SaveWindowPlacement(hDlg, csString);
            ShowWindow(hwndImage, SW_HIDE);
            return (INT_PTR)TRUE;
        }
        return (INT_PTR)FALSE;
    }

    case WM_WINDOWPOSCHANGING:
    {
        WINDOWPOS* wpos = (WINDOWPOS*)lParam;
        int x, y;

        if (!Displays || !Displays->GetSize(&x, &y)) {
            // don't show window if there is nothing to show
            wpos->flags &= ~SWP_SHOWWINDOW;
        }
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        ImgDlg->Rescale(GET_WHEEL_DELTA_WPARAM(wParam));
        ImgDlg->Repaint();
        ImgDlg->UpdateStatusBar(hDlg);
        break;
    }

    case WM_MOUSEMOVE:
    {
        int x, y;
        x = GET_X_LPARAM(lParam);
        y = GET_Y_LPARAM(lParam);

        if (ImgDlg->PanImage(hDlg,x,y)) {
            ImgDlg->Repaint();
        }

        ImgDlg->UpdateMousePos(hDlg, x, y);
        ImgDlg->UpdateStatusBar(hDlg);
        break;
    }

    case WM_LBUTTONDOWN:
        ImgDlg->EnablePanning(TRUE);
        SetCapture(hDlg); //ResizeStatusBar Capture the mouse input to receive WM_MOUSEMOVE even if the mouse is outside the window
        break;

    case WM_LBUTTONUP:
        ImgDlg->EnablePanning(FALSE);
        ReleaseCapture(); // Release the mouse input capture
        break;

    case WM_SIZE:
    {
        int xsize, ysize;
        COLORREF* Image;
        Displays->GetDisplay(&Image, &xsize, &ysize);

        if (ImgDlg->LoadCOLORREFimage(hwndImage, xsize, ysize, Image)) {
            ImgDlg->Repaint();
            ImgDlg->UpdateStatusBar(hDlg);
        }

        ImgDlg->ResizeStatusBar(hDlg);
        ImgDlg->UpdateStatusBar(hDlg);
        break;
    }

    case WM_DESTROY:
    {
        ImgDlg->DestroyStatusBar();

        // release Direct2D
        ImgDlg->ReleaseDirect2D();

        hwndImage = NULL;
        break;
    }

    }
    return (INT_PTR)FALSE;
}

