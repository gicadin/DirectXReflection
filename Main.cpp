// include the basic windows header files and the Direct3D header file
#include "includefiles.h"

// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")

// global declarations
DGraphics* graphics;

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	HWND hWnd;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	// Register a windows class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_TITLE;

	RegisterClassEx(&wc);

	// Create window 
	hWnd = CreateWindowEx(NULL,
		WINDOW_TITLE,
		WINDOW_TITLE,
		(WINDOWED) ? WS_OVERLAPPEDWINDOW : WS_EX_TOPMOST,		// Whether we want the window fullscreen or not
		CW_USEDEFAULT, CW_USEDEFAULT,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	// Adjust window to desired size after initialization but before displaying
	RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rect, GetWindowLong(hWnd, GWL_STYLE), FALSE);
	SetWindowPos(hWnd, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE); 

	graphics = new DGraphics(hWnd, hInstance);
	graphics->initDirectInput();
	graphics->loadMeshes();
	graphics->setupView();

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	
	MSG msg;

	while (TRUE)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		graphics->renderFrame();
	}

	return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	} break;
	
	case WM_PAINT:
		
		graphics->renderFrame();
		
		ValidateRect(hWnd, 0);
		return 0;
	
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		return 0;

	case WM_LBUTTONDOWN:
	{
		DGraphics::Ray ray = graphics->calcPickingRay(LOWORD(lParam), HIWORD(lParam));

		// transform the ray to world space
		D3DXMATRIX view;
		graphics->getDevice()->GetTransform(D3DTS_VIEW, &view);

		D3DXMATRIX viewInverse;
		D3DXMatrixInverse(&viewInverse, 0, &view);

		graphics->transformRay(&ray, &viewInverse);

		if (graphics->raySphereIntTest(&ray, &graphics->g_sphere))
		{
			int count = 0;
			count++;
		}
	} break;

	}



	return DefWindowProc(hWnd, message, wParam, lParam);
}
