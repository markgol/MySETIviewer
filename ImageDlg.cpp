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
// V0.1.0.1 2023-11-27  Initial Pre Release
// V0.3.0.1 2023-12-14  Correcting behaviour of repaint, window position saving/restoring
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
        CString csString = L"ImageWindow";
        RestoreWindowPlacement(hDlg, csString);
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDCANCEL:
        {
            // save window position/size data
            CString csString = L"ImageWindow";
            SaveWindowPlacement(hDlg, csString);
            ShowWindow(hwndImage, SW_HIDE);
            return (INT_PTR)TRUE;
        }

        case IDC_GENERATE_BMP:
        {
            int xsize, ysize;
            COLORREF* Image;
            Displays->GetDisplay(&Image, &xsize, &ysize);
            
            if(ImgDlg->LoadCOLORREFimage(hwndImage, xsize, xsize,Image)) {
                ImgDlg->Repaint();
            }
            return (INT_PTR)TRUE;
        }

        default:
            return (INT_PTR)FALSE;
        } // end of WM_COMMAND
        
    case WM_WINDOWPOSCHANGED:
    {
        ImgDlg->Repaint();
    }

    case WM_WINDOWPOSCHANGING:
    {
        WINDOWPOS* wpos = (WINDOWPOS*)lParam;
        float AspectRatio;
        int x, y;

        // maintain aspect ratio
        if (Displays && Displays->GetSize(&x, &y)) {
            AspectRatio = (float)x / (float)y;
            wpos->cy = (int)((float)wpos->cx / AspectRatio);
        }
        else {
            // don't sohw window is ther is nothing to show
            wpos->flags &= ~SWP_SHOWWINDOW;
        }
        return 0;
    }

    case WM_MOUSEWHEEL:
    {
        ImgDlg->Rescale(GET_WHEEL_DELTA_WPARAM(wParam));
        ImgDlg->Repaint();
        break;
    }

    case WM_MOUSEMOVE:
    {
        //if (ImgDlg->PanImage(LOWORD(lParam), HIWORD(lParam))) {
        if (ImgDlg->PanImage(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))) {
            ImgDlg->Repaint();
        }
        break;
    }

    case WM_LBUTTONDOWN:
        ImgDlg->EnablePanning(TRUE);
        SetCapture(hDlg); // Capture the mouse input to receive WM_MOUSEMOVE even if the mouse is outside the window
        break;

    case WM_LBUTTONUP:
        ImgDlg->EnablePanning(FALSE);
        ReleaseCapture(); // Release the mouse input capture
        break;

    case WM_DESTROY:
    {
        // release Direct2D
        ImgDlg->ReleaseDirect2D();
        hwndImage = NULL;
        break;
    }

    }
    return (INT_PTR)FALSE;
}

