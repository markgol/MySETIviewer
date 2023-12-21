//
// MySETIapp, a set tools for decoding bitstreams into various formats and manipulating those files
// AboutDlg.cpp
// (C) 2023, Mark Stegall
// Author: Mark Stegall
//
// This file is part of MySETIapp.
//
// MySETIapp is free software : you can redistribute it and /or modify it under
// the terms of the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// MySETIapp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with MySETIapp.
// If not, see < https://www.gnu.org/licenses/>. 
// 
// V1.0.1.0	2023-12-20	Initial release
//
// About box handler
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
#include "Globals.h"
#include "AppFunctions.h"
#include "imageheader.h"
#include "FileFunctions.h"

// Message handler for about box.
INT_PTR CALLBACK AboutDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        if (GetProductAndVersion(&strProductName,
            &strProductVersion,
            &strName,
            &strCopyright,
            &strAppNameEXE))
        {
            const WCHAR* pszProductName = (const WCHAR*)strProductName;
            const WCHAR* pszProductVersion = (const WCHAR*)strProductVersion;
            const WCHAR* pszName = (const WCHAR*)strName;
            const WCHAR* pszCopyright = (const WCHAR*)strCopyright;

            SetDlgItemText(hDlg, IDC_ABOUT_NAME, pszProductName);
            SetDlgItemText(hDlg, IDC_ABOUT_VERSION, pszProductVersion);
            SetDlgItemText(hDlg, IDC_ABOUT_LICENSE, pszName);
            SetDlgItemText(hDlg, IDC_ABOUT_COPYRIGHT, pszCopyright);
        }
        else
        {
            SetDlgItemText(hDlg, IDC_ABOUT_NAME, (LPCWSTR)L"MySETIapp");
            SetDlgItemText(hDlg, IDC_ABOUT_VERSION, (LPCWSTR)L"2.0.0.1");
            SetDlgItemText(hDlg, IDC_ABOUT_AUTHOR, (LPCWSTR)L"GNU GPL V3.0 or later");
            SetDlgItemText(hDlg, IDC_ABOUT_COPYRIGHT, (LPCWSTR)L"(C) 2023, Mark Stegall");
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
