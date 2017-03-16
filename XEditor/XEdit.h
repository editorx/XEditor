#ifndef  _X_EDIT_H
#define  _X_EDIT_H

class XEdit : public XWnd
{
	struct CARET                //插入符
	{
	private:
		int x;
		int y;
	public:
		int xOffset, yOffset;
		CARET() : x(0), y(0), xOffset(0), yOffset(0) {}
		~CARET() {}
		void GetPos(int &cx, int &cy);
		void SetPos(int cx, int cy);
	};

	struct SCROLLBAR             //滚动条
	{
	public:
		int iDeltaPerLine, iAccumDelta;
		ULONG ulScrollLines;
		SCROLLINFO si;
		int m_pos;

		SCROLLBAR() : iDeltaPerLine(0), iAccumDelta(0), ulScrollLines(0) {}
		~SCROLLBAR() {}
	};

	struct SELECTOR               //文本选择
	{
		BOOL start;
		BOOL selected;
		BOOL keySelected;
		int rows;
		struct
		{
			int x;
			int y;
		} Point0, point1;
		void Reset();
	};
public:
	XArticle article;
	
private:
	DWORD m_CharSet;
	int m_CxChar, m_CyChar;          //单字符
	int m_CxClient, m_CyClient;      //客户区

	CARET m_Caret;
	SCROLLBAR m_Scroll;
	SELECTOR m_selector;
	SELECTOR m_selectorStart;

	BOOL VIRTUAL_MK_LBUTTON;           //虚拟鼠标左键

	int xCaretPixelSave;               //插入符水平位置保存
	int xCaretPix, yCaretPix;          //插入符位置

	HDC hMemDC;                        //用于双缓冲
	RECT ClientRect;
	HBITMAP hMemBmp;
	HBITMAP hPreBmp;

	HDC hMemNumDC;                       //用于双缓冲（行号）
	HBITMAP hMemNumBmp;
	HBITMAP hPreNumBmp;

public:
	XEdit() : m_CharSet(DEFAULT_CHARSET),
		m_CxChar(0), m_CyChar(0), m_CxClient(0), m_CyClient(0),
		xCaretPixelSave(0), xCaretPix(0), yCaretPix(0),
		VIRTUAL_MK_LBUTTON(FALSE) 
	{}
	~XEdit() {}

	LRESULT OnCreat();
	void OnSettingChange();
	void OnSize(int width, int height);
	void OnKeyDown(WPARAM wParam, LPARAM lParam);
	void OnLButtonDown(WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(WPARAM wParam, LPARAM lParam);
	void OnMouseMove(WPARAM wParam, LPARAM lParam);
	void OnCommand(WPARAM wParam, LPARAM lParam);
	void OnChar(WPARAM wParam, LPARAM lParam);
	void OnFind(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	void OnVScroll(UINT nSBCode);
	void OnHScroll(UINT nSBCode);
	void OnMouseWheel(WPARAM wParam, LPARAM lParam);
	void OnPaint();
	void OnDestroy();

	virtual LRESULT WndProc(HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam);

	int GetRealCxChar(int line, int col);

	enum SCROLL_FALG
	{
		SCROLL_MAX,
		SCROLL_MIN,
		SCROLL_INC,
		SCROLL_DEC
	};

	void ScrollMove(int nBar, int Pos);
	void ScrollMove(int nBar, SCROLL_FALG Flag);
	void ScrollResize(int nBar, int nMax);

private:
	SeqStack<XArticleTag*> UndoArticleTagList;
	SeqStack<XArticleTag*> RedoArticleTagList;
	SeqStack<int> redoColPixList;
	SeqStack<int> redoLinePixList;
	SeqStack<int> undoColPixList;
	SeqStack<int> undoLinePixList;

	void DoCMD(RedoOrUndo select, LogElement log);
	bool Redo(void);
	bool Undo(void);
	void AddLog(LogElement log);
	void ClearLogAll(void);
};

#endif
