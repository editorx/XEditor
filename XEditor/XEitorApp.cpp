#include <stdio.h>
#include "common.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance,
	LPSTR lpCmd, int nShow)
{
	XEditor xEditor(hInst);

#ifdef _DEBUG
	DEBUG::RedirectIOToConsole();
#endif
	TCHAR szProgram[40];
	LoadString(hInst, IDS_APP_NAME, szProgram, 40);
	xEditor.SetInitFilePath(lpCmd);
	xEditor.CreateEX(0, szProgram, szProgram, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, LoadMenu(hInst, MAKEINTRESOURCE(IDR_MAIN)), hInst
		);
	xEditor.ShowWindow(nShow);
	xEditor.UpdateWindow();

	return xEditor.MessageLoop();
}