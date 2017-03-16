#ifdef _XWND_H
#define _XWND_H

class XWnd {
protected:
	virtual void OnDraw(HDC hdc) {}                               // 窗口重绘函数
	virtual void OnKeyDown(WPARAM wParam, LPARAM lParam) {}       // 键盘响应函数
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg,                 // 窗口过程
		WPARAM wParam, LPARAM lParam);
	static  LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg,     // 回调到 WinAPI的窗口过程(静态)
		WPARAM wParam, LPARAM lParam);
	virtual void GetWndClassEx(WNDCLASSEX &wc);                   // 初始化 WNDCLASSEX结构到默认值
public:
	HWND m_hWnd;                                                  // 窗口句柄
	XWnd(void) { m_hWnd = NULL; }                                 // 构造函数
	virtual ~XWnd(void) {}                                        // 析构函数
	virtual bool CreateEx(DWORD dwExStyle,                        // 创建窗体
		LPCTSTR lpszClass, LPCTSTR lpszName, DWORD dwStyle,
		int x, int y, int nWidth, int nHeight, HWND hParent,
		HMENU hMenu, HINSTANCE hInst);
	bool RegisterClass(LPCTSTR lpszClass, HINSTANCE hInst);       // 注册窗体
	virtual WPARAM MessageLoop(void);                             // 消息循环
	BOOL ShowWindow(int nCmdShow) const                           // 显示窗体
	{
		return ::ShowWindow(m_hWnd, nCmdShow);
	}
	BOOL UpdateWindow(void) const                                 // 更新窗体
	{
		return ::UpdateWindow(m_hWnd);
	}
};
#endif
