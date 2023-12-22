//
// MySETIviewer, a set tools for decoding bitstreams into various formats and manipulating those files
// BinaryInput.cpp
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
//
// Global Settings dialog box handler
// 

//#define IDM_BITTOOLS_TEXT2BITSTREAM     32861
//#define IDM_BITTOOLS_BINARYIMAGE        32803

#include "framework.h"
#include "MySETIviewer.h"
#include <windows.h>
#include <libloaderapi.h>
#include <shtypes.h>
#include <stdio.h>
#include <atlstr.h>
#include <strsafe.h>
#include "imageheader.h"
#include "FileFunctions.h"
#include "Appfunctions.h"
#include "globals.h"

int BitStream2Image(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int BlockHeaderBits, int NumBlockBodyBits, int BlockNum, int xsize,
    int BitDepth, int BitOrder, int BitScale, int Invert, int InputBitOrder);

int ConvertText2BitStream(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int BitOrder);

//*******************************************************************************
//
// Message handler for BitImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK BitImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        int BitOrder;
        int BitScale;
        int Invert;
        int InputBitOrder;

        GetPrivateProfileString(L"BitImageDlg", L"BinaryInput", L"OriginalSource\\data17.bin", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);

        // get filesize
        // IDC_FILESIZE
        int FileSize;
        FileSize = GetFileSize(szString) * 8;
        if (FileSize >= 0) {
            SetDlgItemInt(hDlg, IDC_FILESIZE, FileSize, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_FILESIZE, 0, TRUE);
        }

        GetPrivateProfileString(L"BitImageDlg", L"ImageOutput", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"BitImageDlg", L"PrologueSize", L"80", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_PROLOGUE_SIZE, szString);

        GetPrivateProfileString(L"BitImageDlg", L"BlockHeaderBits", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BLOCK_HEADER_BITS, szString);

        GetPrivateProfileString(L"BitImageDlg", L"BlockBits", L"65536", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BLOCK_BITS, szString);

        GetPrivateProfileString(L"BitImageDlg", L"xsize", L"256", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XSIZE, szString);

        GetPrivateProfileString(L"BitImageDlg", L"BlockNum", L"1", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BLOCK_NUM, szString);

        GetPrivateProfileString(L"BitImageDlg", L"BitDepth", L"1", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BIT_DEPTH, szString);

        BitOrder = GetPrivateProfileInt(L"BitImageDlg", L"BitOrder", 0, (LPCTSTR)strAppNameINI);
        if (!BitOrder) {
            CheckDlgButton(hDlg, IDC_BITORDER, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_BITORDER, BST_CHECKED);
        }

        InputBitOrder = GetPrivateProfileInt(L"BitImageDlg", L"InputBitOrder", 0, (LPCTSTR)strAppNameINI);
        if (!InputBitOrder) {
            CheckDlgButton(hDlg, IDC_INPUT_BITORDER, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_INPUT_BITORDER, BST_CHECKED);
        }

        BitScale = GetPrivateProfileInt(L"BitImageDlg", L"BitScale", 0, (LPCTSTR)strAppNameINI);
        if (!BitScale) {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_CHECKED);
        }

        Invert = GetPrivateProfileInt(L"BitImageDlg", L"Invert", 0, (LPCTSTR)strAppNameINI);
        if (!Invert) {
            CheckDlgButton(hDlg, IDC_INVERT, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_INVERT, BST_CHECKED);
        }

        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC bitType[] =
            {
                 { L"bit stream files", L"*.bin" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, bitType, L"*.bin")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);

            // get filesize
            // IDC_FILESIZE
            int FileSize;
            FileSize = GetFileSize(szString) * 8;
            if (FileSize >= 0) {
                SetDlgItemInt(hDlg, IDC_FILESIZE, FileSize, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_FILESIZE, 0, TRUE);
            }

            return (INT_PTR)TRUE;
        }
        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L".raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_CONVERT:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int PrologueSize;
            int xsize;
            int BlockNum;
            int NumBlockBodyBits;
            int BlockHeaderBits;
            int BitDepth;
            int BitOrder = 0;
            int BitScale = 0;
            int Invert = 0;
            int InputBitOrder = 0;
            int iRes;

            GetDlgItemText(hDlg, IDC_BINARY_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);

            PrologueSize = GetDlgItemInt(hDlg, IDC_PROLOGUE_SIZE, &bSuccess, TRUE);

            BlockHeaderBits = GetDlgItemInt(hDlg, IDC_BLOCK_HEADER_BITS, &bSuccess, TRUE);

            NumBlockBodyBits = GetDlgItemInt(hDlg, IDC_BLOCK_BITS, &bSuccess, TRUE);

            xsize = GetDlgItemInt(hDlg, IDC_XSIZE, &bSuccess, TRUE);

            BlockNum = GetDlgItemInt(hDlg, IDC_BLOCK_NUM, &bSuccess, TRUE);

            BitDepth = GetDlgItemInt(hDlg, IDC_BIT_DEPTH, &bSuccess, TRUE);

            if (IsDlgButtonChecked(hDlg, IDC_BITORDER) == BST_CHECKED) {
                BitOrder = 1;
            }

            if (IsDlgButtonChecked(hDlg, IDC_INPUT_BITORDER) == BST_CHECKED) {
                InputBitOrder = 1;
            }

            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                BitScale = 1;
            }

            if (IsDlgButtonChecked(hDlg, IDC_INVERT) == BST_CHECKED) {
                Invert = 1;
            }

            iRes = BitStream2Image(hDlg, InputFile, OutputFile,
                    PrologueSize, BlockHeaderBits, NumBlockBodyBits, BlockNum, xsize,
                    BitDepth, BitOrder, BitScale, Invert, InputBitOrder);
            if (iRes != APP_SUCCESS) {
                MessageMySETIviewerError(hDlg, iRes, L"Convert");
                return (INT_PTR)TRUE;
            }
            
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"BinaryInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_PROLOGUE_SIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"PrologueSize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BLOCK_HEADER_BITS, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"BlockHeaderBits", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BLOCK_BITS, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"BlockBits", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"xsize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BLOCK_NUM, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"BlockNum", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BIT_DEPTH, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"BitDepth", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_BITORDER) == BST_CHECKED) {
                WritePrivateProfileString(L"BitImageDlg", L"BitOrder", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"BitImageDlg", L"BitOrder", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_INPUT_BITORDER) == BST_CHECKED) {
                WritePrivateProfileString(L"BitImageDlg", L"InputBitOrder", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"BitImageDlg", L"InputBitOrder", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                WritePrivateProfileString(L"BitImageDlg", L"BitScale", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"BitImageDlg", L"BitScale", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_INVERT) == BST_CHECKED) {
                WritePrivateProfileString(L"BitImageDlg", L"Invert", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"BitImageDlg", L"Invert", L"0", (LPCTSTR)strAppNameINI);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for Text2StreamDlg dialog box.
// 
// This converts a text file into a binary bitstream file (like data17.bin)
// 
//*******************************************************************************

INT_PTR CALLBACK Text2StreamDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        int BitOrder;

        GetPrivateProfileString(L"Text2StreamDlg", L"TextInput", L"data17.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);

        GetPrivateProfileString(L"Text2StreamDlg", L"BinaryOutput", L"bitstream.bin", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BINARY_OUTPUT, szString);

        BitOrder = GetPrivateProfileInt(L"Text2StreamDlg", L"BitOrder", 0, (LPCTSTR)strAppNameINI);
        if (!BitOrder) {
            CheckDlgButton(hDlg, IDC_BITORDER, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_BITORDER, BST_CHECKED);
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_BINARY_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC bitType[] =
            {
                 { L"bit stream files", L"*.bin" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, bitType, L".bin")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BINARY_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }
        case IDC_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text stream files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, textType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_CONVERT:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int BitOrder = 0;
            int iRes;

            GetDlgItemText(hDlg, IDC_TEXT_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_BINARY_OUTPUT, OutputFile, MAX_PATH);

            if (IsDlgButtonChecked(hDlg, IDC_BITORDER) == BST_CHECKED) {
                BitOrder = 1;
            }

            iRes = ConvertText2BitStream(hDlg, InputFile, OutputFile, BitOrder);
            if (iRes != APP_SUCCESS) {
                MessageMySETIviewerError(hDlg, iRes, L"Convert");
                return (INT_PTR)TRUE;
            }

            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"Text2StreamDlg", L"TextInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BINARY_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"Text2StreamDlg", L"BinaryOutput", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_BITORDER) == BST_CHECKED) {
                WritePrivateProfileString(L"Text2StreamDlg", L"BitOrder", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"Text2StreamDlg", L"BitOrder", L"0", (LPCTSTR)strAppNameINI);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//******************************************************************************
//
// BitStream2Image
// 
// Convert packed binary bitstream file to image file
// 
// Parameters:
//  HWND hDlg               Handle of calling window or dialog
//  WCHAR* InputFile        Packed Binary Btstream input file
//  CHAR* OutputFile        Name of text file to output
//  int PrologueSize        # of bits to skip in prologue
//  int BlockHeaderBits     # of block header bits to skip
//  int NumBlockBodyBits    # of bits in block (each block is converted to a frame
//                          in the output image file) 
//  int BlockNum            # of block in bitstream (becomes # of frames
//                          in the output image file
//  int xsize               # of pixels in a row
//  int BitDepth            # of bits converted per pixel
//  int BitOrder            0 - LSB to MSB, 1 - MSB to LSB
//  int BitScale            Scale binary output, 0,1 -> 0,255
// 
//  The Ysize of the image is calculated as Ysize = NumBlockBodyBits/(xsize*bitdepth)
//
//******************************************************************************
int BitStream2Image(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int BlockHeaderBits, int NumBlockBodyBits, int BlockNum, int xsize,
    int BitDepth, int BitOrder, int BitScale, int Invert, int InputBitOrder)
{
    FILE* In;
    FILE* OutRaw;
    int CurrentBit = 0;
    int CurrentPrologueBit = 0;
    int CurrentByteBit;
    int CurrentPage = 0;
    int CurrentHeaderBit = 0;
    int CurrentPageBit = 0;
    int CurrentPageCol = 0;
    int CurentX = 0;
    int CurrentY = 0;
    int CurrentPixelBit;
    errno_t ErrNum;
    union PIXEL {
        unsigned char bit8;
        unsigned short bit16;
        long bit32;
    } Pixel;

    if (xsize <= 0) {
        MessageBox(hDlg, L"x size must be >= 1", L"File I/O", MB_OK);
        return 0;
    }

    if (NumBlockBodyBits <= 0) {
        MessageBox(hDlg, L"# bits in block >= 1", L"File I/O", MB_OK);
        return 0;
    }

    if (BitDepth <= 0 || BitDepth > 32) {
        MessageBox(hDlg, L"1 <= Image bit depth <= 32", L"File I/O", MB_OK);
        return 0;
    }

    if (BitDepth != 1 && BitScale) {
        MessageBox(hDlg, L"Scale Binary can only be used if Image bit depth is 1", L"File I/O", MB_OK);
        return 0;
    }

    ErrNum = _wfopen_s(&In, InputFile, L"rb");
    if (In == NULL) {
        MessageBox(hDlg, L"Could not open input file", L"File I/O", MB_OK);
        return -2;
    }

    ErrNum = _wfopen_s(&OutRaw, OutputFile, L"wb");
    if (OutRaw == NULL) {
        fclose(In);
        MessageBox(hDlg, L"Could not open raw output file", L"File I/O", MB_OK);
        return -2;
    }

    // Initialize image file header
    IMAGINGHEADER ImgHeader;

    ImgHeader.Endian = (short)-1;  // PC format
    ImgHeader.HeaderSize = (short)sizeof(IMAGINGHEADER);
    ImgHeader.ID = (short)0xaaaa;
    ImgHeader.Version = (short)1;
    ImgHeader.NumFrames = (short)BlockNum;
    if (BitDepth <= 8) {
        ImgHeader.PixelSize = (short)1;
    }
    else if (BitDepth <= 16) {
        ImgHeader.PixelSize = (short)2;
    }
    else {
        ImgHeader.PixelSize = (short)4;
    }
    ImgHeader.Xsize = xsize;
    ImgHeader.Ysize = (NumBlockBodyBits / (xsize * BitDepth));
    ImgHeader.Padding[0] = 0;
    ImgHeader.Padding[1] = 0;
    ImgHeader.Padding[2] = 0;
    ImgHeader.Padding[3] = 0;
    ImgHeader.Padding[4] = 0;
    ImgHeader.Padding[5] = 0;

    // write header to file
    fwrite(&ImgHeader, sizeof(IMAGINGHEADER), 1, OutRaw);

    CurrentPixelBit = BitDepth - 1;
    Pixel.bit32 = 0;

    while (!feof(In)) {
        char CurrentByte;
        int BitValue;
        size_t NumRead;
        CurrentByteBit = 0;

        NumRead = fread(&CurrentByte, 1, 1, In);
        if (NumRead <= 0) {
            break;
        }

        // process data bit by bit in the order of the bit transmission message
        // input file is byte oriented MSB to LSB representing the bit order that the message was received
        // This does not imply any bit ordering in the message itself.
        while (CurrentByteBit < 8) {
            // MSB to LSB in input data file 
            if (InputBitOrder) {
                BitValue = CurrentByte & (0x01 << CurrentByteBit);
            }
            else {
                BitValue = CurrentByte & (0x80 >> CurrentByteBit);
            }
            if (Invert == 1) {
                if (BitValue == 0) {
                    BitValue = 1;
                }
                else {
                    BitValue = 0;
                }
            }
            // Prologue block processing
            // skip prologue, PrologueSize bits
            if (PrologueSize > 0) {  // 0 based index
                if (CurrentPrologueBit < PrologueSize) {
                    CurrentByteBit++;
                    CurrentPrologueBit++;
                    CurrentBit++;
                    continue;
                }
                else {
                    if (CurrentPrologueBit == PrologueSize) {
                        // mark so don't process in this block again
                        CurrentPrologueBit++;
                    }
                }
            }
            // end of prologque processing

            // Page header block processing, one header per page
            // skip page headers
            if (BlockHeaderBits > 0) {
                if (CurrentHeaderBit < BlockHeaderBits) {
                    // process header bits
                    CurrentByteBit++;
                    CurrentHeaderBit++;
                    CurrentBit++;
                    continue;
                }
            }
            // end of page header block processing

            // these bits belong in page/tile/image

            if (CurrentPageBit < NumBlockBodyBits) {
                // this assumes that there could be 1 to 16 bits per pixel in the resulting image
                if (BitValue) {
                    // add to image byte if if bit depth is <=8, add to image int16 is bit depth >8
                    // right now don't worry about pixel bit depth > 16
                    // PixelByte, Pixel are a union.

                    // data is always stored in resulting image MSB to LSB in byte/short
                    // since Pixel value starts as 0 only need to set bits not clear them
                    // Decode using Endian flag for LSB-MSB or MSB-LSB order
                    if (BitDepth <= 8) {
                        if (BitOrder) {
                            // bit stream order is MSB to LSB
                            Pixel.bit8 = Pixel.bit8 | (1 << CurrentPixelBit);
                        }
                        else {
                            // bit stream order is LSB to MSB
                            Pixel.bit8 = Pixel.bit8 | (1 << ((BitDepth - 1) - CurrentPixelBit));
                        }
                    }
                    else if (BitDepth <= 16) {
                        if (BitOrder) {
                            // bit stream order is MSB to LSB
                            Pixel.bit16 = Pixel.bit16 | (1 << CurrentPixelBit);
                        }
                        else {
                            // bit stream order is LSB to MSB
                            Pixel.bit16 = Pixel.bit16 | (1 << ((BitDepth - 1) - CurrentPixelBit));
                        }
                    }
                    else {
                        if (BitOrder) {
                            // bit stream order is MSB to LSB
                            Pixel.bit32 = Pixel.bit32 | (1 << CurrentPixelBit);
                        }
                        else {
                            // bit stream order is LSB to MSB
                            Pixel.bit32 = Pixel.bit32 | (1 << ((BitDepth - 1) - CurrentPixelBit));
                        }
                    }
                }
                CurrentPixelBit--;
                if (CurrentPixelBit < 0) {
                    if (BitDepth <= 8) {
                        if (BitScale && Pixel.bit8 && (BitDepth == 1)) {
                            Pixel.bit8 = 255;
                        }
                        fwrite(&Pixel.bit8, 1, 1, OutRaw);
                    }
                    else if (BitDepth <= 16) {
                        fwrite(&Pixel.bit16, 2, 1, OutRaw);
                    }
                    else {
                        fwrite(&Pixel.bit32, 4, 1, OutRaw);

                    }
                    Pixel.bit16 = 0;
                    CurrentPixelBit = BitDepth - 1;
                }
                CurrentPageCol++;
                if (CurrentPageCol >= xsize) {
                    CurrentPageCol = 0;
                }
                CurrentByteBit++;
                CurrentPageBit++;
                CurrentBit++;
                continue;
            }
            else {
                // start on next page
                CurrentHeaderBit = 0;
                CurrentPageBit = 0;
                CurrentPixelBit = BitDepth - 1;
                CurrentPage++;
                if (CurrentPage == BlockNum) {
                    fclose(In);
                    fclose(OutRaw);
                    return 1;
                }
            }
        }
    }
    fclose(In);
    fclose(OutRaw);


    return 1;
}

//*******************************************************************
//
// ConvertText2BitStream
// 
// Convert textfile to a packed BitStream binary file
// 
// Parameters:
//  HWND hDlg               handle of calling window/dialog
//  WCHAR* InputFile        text file with space delmited list of values
//                          value < 0 , cause error to be returned
//                          value = 0 is taken as bit with value 0
//                          value > 0 is taken as bit with value 1
//                          (file is multiple always of 8 bits)
//  WCHAR* OutputFile       Packed Binary bit stream file
//
//*******************************************************************
int ConvertText2BitStream(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int BitOrder)
{
    FILE* In;
    FILE* Out;
    int BitNumber;
    int BitValue;
    int ByteValue;
    int iRes;
    int TotalBits;
    int TotalOneBits;
    int TotalBytes;
    errno_t ErrNum;

    BitNumber = 0;
    ByteValue = 0;
    TotalBits = 0;
    TotalOneBits = 0;
    TotalBytes = 0;

    ErrNum = _wfopen_s(&In, InputFile, L"r");
    if (In == NULL) {
        MessageBox(hDlg, L"Could not open input file", L"File I/O", MB_OK);
        return -2;
    }

    ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
    if (Out == NULL) {
        fclose(In);
        MessageBox(hDlg, L"Could not open raw output file", L"File I/O", MB_OK);
        return -2;
    }

    while (!feof(In)) {
        iRes = fscanf_s(In, "%d", &BitValue);
        if (iRes != 1) {
            fclose(Out);
            fclose(In);
            return -3;
        }
        if (BitValue < 0) {
            fclose(Out);
            fclose(In);
            return -3;
        }
        if (BitValue > 0) {
            if (BitOrder) {
                ByteValue = ByteValue | (0x01 << BitNumber);
            }
            else {
                ByteValue = ByteValue | (0x80 >> BitNumber);
            }
            TotalOneBits++;
        }
        TotalBits++;
        BitNumber++;
        if (BitNumber >= 8) {
            fwrite(&ByteValue, 1, 1, Out);
            ByteValue = 0;
            BitNumber = 0;
            TotalBytes++;
        }
    }

    if (BitNumber != 0) {
        fwrite(&ByteValue, 1, 1, Out);
    }

    TCHAR pszMessageBuf[MAX_PATH];
    StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH, TEXT("Bitsream properties\n# of bits: %d\n# of set bits: %d\nBytes writtten: %d"),
        TotalBits, TotalOneBits, TotalBytes);
    MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);

    fclose(In);
    fclose(Out);
    return 1;
}

