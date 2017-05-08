#ifndef _X_EDITOR_H
#define  _X_EDITOR_H

class XEditor : public XWnd 
{
private:
	HINSTANCE    m_hInst;
	HACCEL       m_hAccel;

	XEdit        m_edit;
	XFindDlg     m_findDialog;
	XAboutDlg    m_aboutDlg;
	TCHAR        InitFilePath[MAX_PATH];
	virtual void GetWndClassEx(WNDCLASSEX &wc);

public:
	XEditor(HINSTANCE hInstance);
	virtual ~XEditor();
	void SetInitFilePath(TCHAR *path);
	void SetInitFilePath(CHAR *path);

public:
	LRESULT OnCreate();
	void OnSize(int width, int height);
	void OnDraw(HDC hdc);
	void OnDragFiles(WPARAM wParam, LPARAM lParam);
	void OnCommand(WPARAM Wparam, LPARAM lParam);

	virtual bool CreateEX(DWORD dwExStyle, LPCTSTR lpszClass,     // ÖØÐ´ CreateExº¯Êý
		LPCTSTR lpszName, DWORD dwStyle, int x, int y,
		int nWidth, int nHeight, HWND hParent, HMENU hMenu, HINSTANCE hInst);

	virtual WPARAM MessageLoop(void);
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

};
#endif
