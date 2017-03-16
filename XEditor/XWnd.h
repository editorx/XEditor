#ifdef _XWND_H
#define _XWND_H

class XWnd {
protected:
	virtual void OnDraw(HDC hdc) {}                               // �����ػ溯��
	virtual void OnKeyDown(WPARAM wParam, LPARAM lParam) {}       // ������Ӧ����
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg,                 // ���ڹ���
		WPARAM wParam, LPARAM lParam);
	static  LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg,     // �ص��� WinAPI�Ĵ��ڹ���(��̬)
		WPARAM wParam, LPARAM lParam);
	virtual void GetWndClassEx(WNDCLASSEX &wc);                   // ��ʼ�� WNDCLASSEX�ṹ��Ĭ��ֵ
public:
	HWND m_hWnd;                                                  // ���ھ��
	XWnd(void) { m_hWnd = NULL; }                                 // ���캯��
	virtual ~XWnd(void) {}                                        // ��������
	virtual bool CreateEx(DWORD dwExStyle,                        // ��������
		LPCTSTR lpszClass, LPCTSTR lpszName, DWORD dwStyle,
		int x, int y, int nWidth, int nHeight, HWND hParent,
		HMENU hMenu, HINSTANCE hInst);
	bool RegisterClass(LPCTSTR lpszClass, HINSTANCE hInst);       // ע�ᴰ��
	virtual WPARAM MessageLoop(void);                             // ��Ϣѭ��
	BOOL ShowWindow(int nCmdShow) const                           // ��ʾ����
	{
		return ::ShowWindow(m_hWnd, nCmdShow);
	}
	BOOL UpdateWindow(void) const                                 // ���´���
	{
		return ::UpdateWindow(m_hWnd);
	}
};
#endif
