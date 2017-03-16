#include <windows.h>
#include <assert.h>
#include "XWnd.h"


LRESULT XWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
		OnKeyDown(wParam, lParam);
		return 0;
	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(m_hWnd, &ps);
		OnDraw(ps.hdc);
		EndPaint(m_hWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK XWnd::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	XWnd *pWindow;
	if (uMsg == WM_NCCREATE)
	{
		assert(!IsBadReadPtr((void *)lParam, sizeof(CREATESTRUCT)));
		MDICREATESTRUCT *pMDIC = (MDICREATESTRUCT *)((LPCREATESTRUCT)lParam)->lpCreateParams;
		pWindow = (XWnd *)(pMDIC->lParam);

		assert(!IsBadReadPtr(pWindow, sizeof(XWnd)));
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)pWindow);
	} 
	else
	{
		pWindow = (XWnd *)GetWindowLong(hWnd, GWL_USERDATA);
	}

	if (pWindow)
		return pWindow->WndProc(hWnd, uMsg, wParam, lParam);
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool XWnd::RegisterClass(LPCTSTR lpszClass, HINSTANCE hInst)
{
	WNDCLASSEX wc;

	if (!GetClassInfoEx(hInst, lpszClass, &wc))
	{
		GetWndClassEx(wc);
		wc.hInstance = hInst;
		wc.lpszClassName = lpszClass;
		if (!RegisterClassEx(&wc))
		{
			return false;
		}
	}
	return true;
}

bool XWnd::CreateEx(DWORD dwExStyle, LPCTSTR lpszClass, LPCTSTR lpszName, DWORD dwStyle, 
	int x, int y, int nWidth, int nHeight, HWND hParent, HMENU hMenu, HINSTANCE hInst)
{
	if (!RegisterClass(lpszClass, hInst))
		return false;

	MDICREATESTRUCT mdic;
	memset(&mdic, 0, sizeof(mdic));
	mdic.lParam = (LPARAM)this;
	m_hWnd = CreateWindowEx(dwExStyle, lpszClass, lpszName, dwStyle,
		x, y, nWidth, nHeight, hParent, hMenu, hInst, &mdic);

	return (m_hWnd != NULL);
}

void XWnd::GetWndClassEx(WNDCLASSEX &wc)
{
	memset(&wc, 0, sizeof(wc));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = NULL;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = NULL;
	wc.hIconSm = NULL;
}

WPARAM XWnd::MessageLoop(void)
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}


