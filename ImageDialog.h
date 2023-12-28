#pragma once
#include "framework.h"
#include <windows.h>
#include <d2d1.h>

class ImageDialog
{
private:
	ID2D1Factory* pFactory = nullptr;
	ID2D1HwndRenderTarget* pRenderTarget = nullptr;
	ID2D1Bitmap* pBitmap = nullptr;
	D2D1_BITMAP_PROPERTIES bitmapProperties = { D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM,
														 D2D1_ALPHA_MODE_IGNORE),
												96.0f, 96.0f };

	// bitmap display scaling
	float scaleFactor = 1.0f;
	// bitmap display panning
	D2D1_POINT_2F panOffset = { 0.0f, 0.0f };
	D2D1_POINT_2F BitmapSize = { 0.0f,0.0f };
	bool isPanning = false;
	POINT lastMousePos = { 0, 0 };
	POINT BitMapMousePos = { 0,0 };
	HWND hwndStatusBar = NULL;
	int StatusBarRight = -1;
	int ClientHeightOffset = 23;
	int BorderX = 0;
	int BorderY = 0;
	int DisplayXsize = 0;
	int DisplayYsize = 0;
	WINDOWPOS WindowPos = { NULL,NULL,0,0,0,0,0 };

public:
	ImageDialog() {
	};

	~ImageDialog() {
	};

	int InitializeDirect2D(void);
	void ReleaseDirect2D(void);
	void ReleaseBitmapRender(void);
	BOOL LoadCOLORREFimage(HWND hWnd, int xsize, int ysize, COLORREF* Image);
	BOOL Repaint(void);

	void Rescale(int Delta);
	void SetScale(float Scale);
	float GetScale();

	void SetPan(float x, float y);
	void GetPan(float* x, float*y);
	void EnablePanning(BOOL Enable);
	BOOL PanImage(HWND hwndParent,int x, int y);
	HWND CreateStatusBar(HWND hwndParent, int idStatus, HINSTANCE hinst);
	void UpdateStatusBar(HWND ParentWindow);
	BOOL StatusBarExists(void);
	void ShowStatusBar(BOOL Show);
	void DestroyStatusBar();
	void ResizeStatusBar(HWND hParentWindow);
	void UpdateMousePos(HWND ParentWindow, int x, int y);
	void GetBorderSize(int* x, int* y);
	void SetReportedWindowPos(HWND hwndWindow, WINDOWPOS* wpos);
	void GetScalePos(float* Scale, float* Xoff, float* Yoff);
	void SetScalePos(float Scale, float Xoff, float Yoff);
};