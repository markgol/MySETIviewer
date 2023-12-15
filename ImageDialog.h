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
	D2D1_BITMAP_PROPERTIES bitmapProperties;

	// bitmap display scaling
	float scaleFactor = 1.0f;
	// bitmap display panning
	D2D1_POINT_2F panOffset = { 0.0f, 0.0f };
	bool isPanning = false;
	POINT lastMousePos = { 0, 0 };

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
	BOOL PanImage(int x, int y);
};