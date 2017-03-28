#ifndef _X_EDITOR_H
#define  _X_EDITOR_H

class XEditor : public XWnd 
{
private:
	HINSTANCE m_hInst;
	HACCEL m_hAccel;
	
	TCHAR InitFilePath[MAX_PATH];

	
	virtual void GetWndClassEx(WNDCLASSEX &wc);

public:
	XEditor(HINSTANCE hInstance);
	virtual ~XEditor();

};
#endif
