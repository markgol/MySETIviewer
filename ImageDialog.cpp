//
// MySETIviewer, a set tools for decoding bitstreams into various formats and manipulating those files
// ImageDialog.cpp
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
//
// This handles all the actual display of the bitmap generated
//
#include "framework.h"
#include <d2d1.h>
#include <d2d1_1.h>
#pragma comment(lib, "d2d1.lib")
#include "AppErrors.h"
#include "ImageDialog.h"

//*******************************************************************************
//
// 
// 
//*******************************************************************************
int ImageDialog::InitializeDirect2D(void)
{
    HRESULT hResult;
    hResult = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
    if (FAILED(hResult)) {
        return APPERR_MEMALLOC;
    }
    return APP_SUCCESS;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::ReleaseDirect2D(void)
{
    // release resources
    if (pBitmap) {
        pBitmap->Release();
        pBitmap = nullptr;
    }

    if (pRenderTarget) {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
    }

    // factory is last
    if (pFactory) {
        pFactory->Release();
        pFactory = nullptr;
    }
    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::ReleaseBitmapRender(void)
{
    if (pBitmap) {
        pBitmap->Release();
        pBitmap = nullptr;
    }
    if (pRenderTarget) {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
    }
    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
BOOL ImageDialog::LoadCOLORREFimage(HWND hWnd, int xsize, int ysize, COLORREF* Image)
{
    // delete old data first
    if (pBitmap) {
        pBitmap->Release();
        pBitmap = nullptr;
    }

    if (pRenderTarget) {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
    }

    // create render target
    pFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(xsize, ysize)),
        &pRenderTarget
    );

    if (pRenderTarget) {
        bitmapProperties.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM,
                                                         D2D1_ALPHA_MODE_IGNORE);
        HRESULT hRes = pRenderTarget->CreateBitmap(D2D1::SizeU(xsize, ysize), Image,
                                        xsize * sizeof(COLORREF),
                                        bitmapProperties, &pBitmap);
        if (FAILED(hRes)) {
            if (pBitmap) {
                pBitmap->Release();
                pBitmap = nullptr;
            }
            return FALSE;
        }
        if (pBitmap) {
            return TRUE;
        }
    }
    return FALSE;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
BOOL ImageDialog::Repaint()
{
    // only do this if target exists
    if (pRenderTarget) {
        pRenderTarget->BeginDraw();
        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Scale(scaleFactor, scaleFactor,
            D2D1::Point2F(panOffset.x, panOffset.y)));
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SeaShell));
        D2D1_SIZE_F RectSize = pRenderTarget->GetSize();
        D2D1_RECT_F Rectf = D2D1::RectF(0.0f, 0.0f, RectSize.width, RectSize.height);
        pRenderTarget->DrawBitmap(pBitmap, Rectf, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        pRenderTarget->EndDraw();
        return TRUE;
    }
    return FALSE;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::Rescale(int Delta)
{
    scaleFactor += (Delta > 0) ? 0.1f : -0.1f;
    scaleFactor = max(0.5f, min(10.0f, scaleFactor));
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::SetScale(float Scale)
{
    scaleFactor = max(0.5f, min(10.0f, Scale));
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
float ImageDialog::GetScale()
{
    return scaleFactor;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::SetPan(float x, float y)
{
    panOffset.x = x;
    panOffset.y = y;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::GetPan(float* x, float* y)
{
    *x = panOffset.x;
    *y = panOffset.y;
}


//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::EnablePanning(BOOL Enable)
{
    isPanning = Enable;
    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
BOOL ImageDialog::PanImage(int x, int y)
{
    if (isPanning) {
        if (x > 0) {
            float deltaX = static_cast<float>(x - lastMousePos.x) / scaleFactor;
            panOffset.x -= deltaX;
        }
        if (y > 0) {
            float deltaY = static_cast<float>(y - lastMousePos.y) / scaleFactor;
            panOffset.y -= deltaY;
        }       
    }
    
    lastMousePos.x = x;
    lastMousePos.y = y;
 
    return isPanning;
}

