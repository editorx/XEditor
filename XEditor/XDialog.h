#ifndef _XDIALOG_H 
#define _XDIALOG_H

class XDialog
{
	virtual BOOL OnInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
	{
		m_hWnd = hWnd;
		return TRUE;
	}

	virtual BOOL OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) 
	{
		return TRUE;
	}
	virtual BOOL DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:
	static  BOOL CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	typedef enum { XDIALOG_MARKER = 31415927 } ;
	typedef struct
	{
		unsigned marker;
		XDialog *pDialog;
	} _DialogCreatRecord;
public:
	HWND m_hWnd;
	XDialog();
	~XDialog();

	HWND Creat(HINSTANCE hInst, int ID_DIALOG, HWND hWndParent)
	{
		_DialogCreatRecord ccr;

		ccr.marker = XDIALOG_MARKER;
		ccr.pDialog = this;

		m_hWnd = ::CreateDialogParam(hInst, MAKEINTRESOURCE(ID_DIALOG), hWndParent,
			DialogProc, (LPARAM)& ccr);
		return m_hWnd;
	}
	void ShowWindow()
	{
		::ShowWindow(m_hWnd, SW_SHOW);
	}

	int DoModal(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent = NULL)
	{
		_DialogCreatRecord ccr;
		ccr.marker = XDIALOG_MARKER;
		ccr.pDialog = this;

		return ::DialogBoxParam(hInstance, lpTemplate, hWndParent, DialogProc, (LPARAM)& ccr);
	}

};
#endif
