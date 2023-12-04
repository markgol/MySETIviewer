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
//
// This is the main functional part of MySETIviewer
// This handles all the display image operations
// 
// This is being replaced
// 
#include "framework.h"
#include "resource.h"
#include <shobjidl.h>
#include <winver.h>
#include <vector>
#include <atlstr.h>
#include <strsafe.h>
#include <shellapi.h>
#include "globals.h"
#include "AppFunctions.h"

//size of the bitmap for the entire display
int DisplayXextent = 0;
int DisplayYextent = 0;
// size of the Image extent (the size of the image after all images are overlyed)
int ImageXextent = 16;
int ImageYextent = 8;

// number of gaps major and minor
int NumberMajorXgap = 0;
int NumberMinorXgap = 0;
int NumberMajorYgap = 0;
int NumberMinorYgap = 0;

void CalculateDisplayExtent(void);

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
        CString csString = L"ImageWindow";
        RestoreWindowPlacement(hDlg, csString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDCANCEL:
            if (!DisplayResults) {
                DestroyWindow(hwndImage);
                hwndImage = NULL;
            }
            else {
                ShowWindow(hwndImage, SW_HIDE);
            }
            return (INT_PTR)TRUE;

        case IDC_GENERATE_BMP:
        {
            // This was the old way
            
            // Initialize COM
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

            // use the default Windows viewer for BMP file
            ShellExecute(hDlg, 0, szBMPFilename, 0, 0, SW_NORMAL);

            // release COM
            CoUninitialize();

            // The new way uses Direct2D to set and render the displau screen


            return (INT_PTR)TRUE;
        }

        default:
            return (INT_PTR)FALSE;
        }
    case WM_PAINT:
    {
        // redraw image as required
        PAINTSTRUCT ps;
        RECT rc;
        HDC hDC;
        if (GetUpdateRect(hDlg, &rc, 0)) {
            hDC = BeginPaint(hDlg, &ps);
            if (hDC == NULL) {
                break;
            }
            TextOut(hDC, 40, 20, szBMPFilename, (int)wcslen(szBMPFilename));
            TextOut(hDC, 40, 40, L"using external BMP viewer", 25);
            EndPaint(hDlg, &ps);
        }
        break;
    }

    case WM_DESTROY:
    {
        // save window position/size data
        CString csString = L"ImageWindow";
        SaveWindowPlacement(hDlg, csString);
        break;
    }

    }
    return (INT_PTR)FALSE;
}
