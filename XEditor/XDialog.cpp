#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include "XWnd.h"
#include "XDialog.h"

XDialog::XDialog() {}
XDialog::~XDialog() {}

BOOL XDialog::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

BOOL CALLBACK XDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	XDialog *pDialog;
	if (message == WM_INITDIALOG)
	{
		PROPSHEETPAGE *pPSP = (PROPSHEETPAGE *) lParam;
		_DialogCreatRecord *pDCR = (_DialogCreatRecord *)lParam;

		if (!IsBadReadPtr(pPSP, sizeof(PROPSHEETPAGE)) && (pPSP->dwSize == sizeof(PROPSHEETPAGE)))
		{
			pDialog = (XDialog *)pPSP->lParam;
			assert(!IsBadReadPtr(pDialog, sizeof(XDialog)));
			SetWindowLong(hDlg, GWL_USERDATA, (LONG)pDialog);
		}
		else if (!IsBadReadPtr(pDCR, sizeof(_DialogCreatRecord)) && (pDCR->marker == XDIALOG_MARKER))
		{
			pDialog = pDCR->pDialog;
			assert(!IsBadReadPtr(pDialog, sizeof(XDialog)));
			SetWindowLong(hDlg, GWL_USERDATA, (LONG)pDialog);
		}
		else
			assert(false);
	}
	else
	{
		switch (message)
		{
		case WM_SETFONT:
		case WM_NOTIFYFORMAT:
		case WM_QUERYUISTATE:
		case 131:
			return FALSE;
		}

		pDialog = (XDialog *)GetWindowLong(hDlg, GWL_USERDATA);
		assert(IsBadReadPtr(pDialog, sizeof(XDialog)));
	}

	if (pDialog)
	{
		return pDialog->DlgProc(hDlg, message, wParam, lParam);
	}
	else
	{
		return FALSE;
	}
}