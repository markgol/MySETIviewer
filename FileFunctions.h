#pragma once
// 
// function prototypes
//
BOOL CCFileSave(HWND hWnd, LPWSTR pszCurrentFilename, LPWSTR* pszFilename,
BOOL bSelectFolder, int NumTypes, COMDLG_FILTERSPEC* FileTypes, LPCWSTR szDefExt);
BOOL CCFileOpen(HWND hWnd, LPWSTR pszCurrentFilename, LPWSTR* pszFilename,
BOOL bSelectFolder, int NumTypes, COMDLG_FILTERSPEC* FileTypes, LPCWSTR szDefExt);
int ReadImageHeader(WCHAR* Filename, IMAGINGHEADER* ImageHeader);
int LoadImageFile(int** ImagePtr, WCHAR* ImagingFilename, IMAGINGHEADER* Header);
int SaveBMP(WCHAR* Filename, WCHAR* InputFile, int RGBframes, int AutoScale);
int SaveTXT(WCHAR* Filename, WCHAR* InputFile);
int DisplayImage(WCHAR* Filename);
int ImportBMP(HWND hWnd);
int HEX2Binary(HWND hWnd);
int CamIRaImport(HWND hWnd);
int GetFileSize(WCHAR* szString);
int SaveBMP2PNG(WCHAR* Filename);
