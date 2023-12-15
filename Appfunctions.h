#pragma once

void LastErrorMsg(LPCTSTR lpszFunction);
void SaveWindowPlacement(HWND hWnd, CString& Section);
BOOL RestoreWindowPlacement(HWND hWnd, CString& Section);

BOOL GetProductAndVersion(CString* strProductName,
    CString* strProductVersion,
    CString* strName,
    CString* strCopyright,
    CString* strAppNameEXE
);

INT GetEncoderClsid(const WCHAR* format, CLSID* pClsid);  // helper function
void MessageMySETIviewerError(HWND hWnd, int ErrNo, const wchar_t* Title);
int ReplaceListBoxEntry(HWND hDlg, int Control, int Selection, WCHAR* szString);

