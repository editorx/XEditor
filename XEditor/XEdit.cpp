#include <windows.h>
#include <iostream>
#include <stdio.h>
#include "XWnd.h"
#include "XArticle.h"
#include "XArticleTag.h"
#include "Debug.h"
#include "resource.h"
#include "XDialog.h"
#include "XFindDlg.h"
#include "SeqStack.h"
#include "RedoUndo.h"
#include "XEdit.h"


#define EDIT_MARGIN_LEFT        (8 * 6)

LRESULT XEdit::OnCreate()
{
	HDC hdc = GetDC(m_hWnd);
	SelectObject(hdc, CreateFont(0, 0, 0, 0, 0, 0, 0, 0,
		m_CharSet, 0, 0, 0, FIXED_PITCH, NULL));

	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	m_CxChar = tm.tmAveCharWidth;
	//   m_CxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * m_CxChar / 2;
	m_CyChar = tm.tmHeight;// + tm.tmExternalLeading;

	m_Caret.xOffset = 0;
	m_Caret.yOffset = 0;
	xCaretPixelSave = 0;                      // 初始化插入符位置
	xCaretPix = 0;
	yCaretPix = 0;
	m_Caret.SetPos(xCaretPix, yCaretPix);

	m_selector.Reset();                       // 初始化块选择结构
	m_selectorStart.Reset();

	TCHAR *newArticle = TEXT("");
	article.Initialize(newArticle);     // 默认情况新建一个空文本

	XArticleTag *undoArticle = new XArticleTag;
	int undoLinePix = 0, undoColPix = 0;
	undoArticle->Initialize(article.Format());
	undoArticle->LineBegin = 0;
	undoArticle->LineCount = 1;
	undoArticle->LineBeginPre = 0;
	undoArticle->LineCountPre = 1;
	undoLinePix = 0;
	undoColPix = 0;
	UndoArticleTagList.Push(undoArticle);
	undoLinePixList.Push(undoLinePix);
	undoColPixList.Push(undoColPix);

	DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
	ReleaseDC(m_hWnd, hdc);

	return 0;
}

void XEdit::OnSettingChange()
{
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &(m_Scroll.ulScrollLines), 0);
	if (m_Scroll.ulScrollLines)
		m_Scroll.iDeltaPerLine = WHEEL_DELTA / m_Scroll.ulScrollLines;
	else
		m_Scroll.iDeltaPerLine = 0;
}

void XEdit::OnSize(int width, int height)
{
	RECT rect;

	m_CxClient = width;
	m_CyClient = height;
	// 竖直滚动条配置
	m_Scroll.si.cbSize = sizeof(m_Scroll.si);
	m_Scroll.si.fMask = SIF_RANGE | SIF_PAGE;
	m_Scroll.si.nMin = 0;
	m_Scroll.si.nMax = (article.GetRows() > 0) ? (article.GetRows() - 1) : 0;
	//     yPage = m_CyClient / m_CyChar;
	//     si.nPage = yPage;
	m_Scroll.si.nPage = m_CyClient / m_CyChar;
	SetScrollInfo(m_hWnd, SB_VERT, &m_Scroll.si, TRUE);
	// 水平滚动条配置
	m_Scroll.si.cbSize = sizeof(m_Scroll.si);
	m_Scroll.si.fMask = SIF_RANGE | SIF_PAGE;
	m_Scroll.si.nMin = 0;
	int nMax = article.GetColMax() + EDIT_MARGIN_LEFT / 8;
	//   nMax += EDIT_MARGIN_LEFT/8;
	m_Scroll.si.nMax = (nMax > 0) ? (nMax - 1) : 0;
	//     xPage = m_CxClient / m_CxChar;
	//     si.nPage = xPage;
	m_Scroll.si.nPage = (m_CxClient - EDIT_MARGIN_LEFT) / m_CxChar;
	SetScrollInfo(m_hWnd, SB_HORZ, &m_Scroll.si, TRUE);

	GetClientRect(m_hWnd, &rect);
	InvalidateRect(m_hWnd, &rect, FALSE);
	UpdateWindow();
}

void XEdit::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	//   if (article.GetSize() == 0)
	//   {
	//     m_Caret.SetPos(0, 0);
	//     return;
	//   }
	switch (wParam)
	{
	case VK_BACK:                       // Backspace键响应
	{
		if (!m_selector.selected && m_selector.start)             // 没有块选中，单字符删除
		{
			// 定位
			m_Caret.GetPos(xCaretPix, yCaretPix);
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			//         BOOL isNormalDel = FALSE;

			// 加一条UN_BACKSPACE纪录
			//         ClearLogAll();
			LogElement log = { LogElement::UN_BACKSPACE, yCaretPix, xCaretPix, NULL };
			AddLog(log);

			// 删除
			if (col == 0)                               // 在行首时则连接到上一行
			{
				if (line > 0)
				{
					m_Caret.SetPos(article.GetCol(line - 1) * m_CxChar, yCaretPix - m_CyChar);
					article.CombineRow(line - 1);
					//           article.GetRows() = article.GetRows();
					//           article.GetColMax() = article.GetColMax();
					//           ScrollResize(SB_VERT, article.GetRows());
					//           ScrollResize(SB_HORZ, article.GetColMax());

					//             // 添加BackSpace纪录（这里相当于换行）
					// //             TCHAR ch = article.GetDataByIndex(line, index);
					//             LogElement log = {LogElement::BACKSPACE, line, 0, NULL};
					//             AddLog(log);
				}
			}
			else                                       // 不在行首时正常删除
			{
				//         m_Caret.SetPos(xCaretPix - m_CxChar, yCaretPix);
				::SendMessage(m_hWnd, WM_KEYDOWN, VK_LEFT, NULL);   // 先将插入符左移，然后删除
				int index = article.GetDataIndex(line, col);

				//           // 添加BackSpace纪录
				//           TCHAR ch = article.GetDataByIndex(line, index);
				//           LogElement log = {LogElement::BACKSPACE, line, index, ch};
				//           AddLog(log);

				article.DeleteChar(line, index - 1);
				//         article.GetRows() = article.GetRows();
				//         article.GetColMax() = article.GetColMax();
				//         ScrollResize(SB_VERT, article.GetRows());
				//         ScrollResize(SB_HORZ, article.GetColMax());
				//           isNormalDel = TRUE;
				//         ::SendMessage(m_hWnd, WM_KEYDOWN, VK_LEFT, NULL);
			}
			m_Caret.GetPos(xCaretPix, yCaretPix);
			// 加一条BACKSPACE纪录
			//         ClearLogAll();
			LogElement log2 = { LogElement::BACKSPACE, yCaretPix, xCaretPix, NULL };
			AddLog(log2);
		}
		else                                  // 块删除
		{
			m_Caret.GetPos(xCaretPix, yCaretPix);

			if (m_selector.Point0.y > m_selector.Point1.y)  // 如果向上选择则交换坐标
			{
				int temp = m_selector.Point0.y;
				m_selector.Point0.y = m_selector.Point1.y;
				m_selector.Point1.y = temp;
				temp = m_selector.Point0.x;
				m_selector.Point0.x = m_selector.Point1.x;
				m_selector.Point1.x = temp;
			}
			else if (m_selector.Point0.y == m_selector.Point1.y
				&& m_selector.Point0.x > m_selector.Point1.x)
			{
				int temp = m_selector.Point0.x;
				m_selector.Point0.x = m_selector.Point1.x;
				m_selector.Point1.x = temp;
			}

			int x0 = m_selector.Point0.x;
			int y0 = m_selector.Point0.y;
			int x1 = m_selector.Point1.x;
			int y1 = m_selector.Point1.y;

			// 加一条UN_BACKSPACE纪录
			//         ClearLogAll();
			LogElement log = { LogElement::UN_BACKSPACE, y0 * m_CyChar, x0 * m_CxChar, y1 - y0 + 1 };
			AddLog(log);

			m_Caret.SetPos(article.GetCol(y0, x0) * m_CxChar, y0 * m_CyChar);
			article.DeleteMultiString(y0, x0, y1, x1);

			m_Caret.GetPos(xCaretPix, yCaretPix);
			// 加一条BACKSPACE纪录
			//         ClearLogAll();
			LogElement log2 = { LogElement::BACKSPACE, yCaretPix, xCaretPix, NULL };
			AddLog(log2);
		}

		//       article.GetRows() = article.GetRows();
		//       article.GetColMax() = article.GetColMax();
		//       ScrollResize(SB_VERT, article.GetRows());
		//       ScrollResize(SB_HORZ, article.GetColMax());

		// 若超出客户区范围，则刷新滚动条
		m_Caret.GetPos(xCaretPix, yCaretPix);
		if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
		{
			//         article.GetColMax() = article.GetColMax();
			ScrollMove(SB_HORZ, xCaretPix / m_CxChar);
			ScrollResize(SB_HORZ, article.GetColMax());
		}
		if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
		{
			//         article.GetRows() = article.GetRows();
			ScrollMove(SB_VERT, yCaretPix / m_CyChar);
			ScrollResize(SB_VERT, article.GetRows());
		}

		//      if (isNormalDel)
		//      {
		////         ::SendMessage(m_hWnd, WM_KEYDOWN, VK_LEFT, NULL);
		//      }

		m_selector.Reset();
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		InvalidateRect(m_hWnd, &rect, FALSE);   // 窗口重绘
		UpdateWindow();
		break;
	}
	case VK_DELETE:                       // Delete键响应
	{
		if (!m_selector.selected && m_selector.start)             // 没有块选中，单字符删除
		{
			// 定位
			m_Caret.GetPos(xCaretPix, yCaretPix);
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;

			// 加一条UN_DELETE纪录
			//         ClearLogAll();
			LogElement log = { LogElement::UN_DEL_CHAR, yCaretPix, xCaretPix, NULL };
			AddLog(log);

			// 删除
			if (col == article.GetCol(line))          // 在行尾时则连接到下一行
			{
				if (line < article.GetRows() - 1)
				{
					//m_Caret.SetPos(article.GetCount(line-1) * m_CxChar, yCaretPix - m_CyChar);
					article.CombineRow(line);
				}
			}
			else                                       // 不在行尾时正常删除
			{
				//m_Caret.SetPos(xCaretPix - m_CxChar, yCaretPix);
				int index = article.GetDataIndex(line, col);

				//           // 添加DEL_CHAR纪录
				//           LogElement log = {LogElement::DEL_CHAR, line, index, NULL};
				//           AddLog(log);

				article.DeleteChar(line, index);
			}
			m_Caret.GetPos(xCaretPix, yCaretPix);
			// 加一条BACKSPACE纪录
			//         ClearLogAll();
			LogElement log2 = { LogElement::DEL_CHAR, yCaretPix, xCaretPix, NULL };
			AddLog(log2);
		}
		else                                  // 块删除
		{
			if (m_selector.Point0.y > m_selector.Point1.y)  // 如果向上选择则交换坐标
			{
				int temp = m_selector.Point0.y;
				m_selector.Point0.y = m_selector.Point1.y;
				m_selector.Point1.y = temp;
				temp = m_selector.Point0.x;
				m_selector.Point0.x = m_selector.Point1.x;
				m_selector.Point1.x = temp;
			}
			else if (m_selector.Point0.y == m_selector.Point1.y
				&& m_selector.Point0.x > m_selector.Point1.x)
			{
				int temp = m_selector.Point0.x;
				m_selector.Point0.x = m_selector.Point1.x;
				m_selector.Point1.x = temp;
			}

			int x0 = m_selector.Point0.x;
			int y0 = m_selector.Point0.y;
			int x1 = m_selector.Point1.x;
			int y1 = m_selector.Point1.y;

			// 加一条UN_BACKSPACE纪录
			//         ClearLogAll();
			LogElement log = { LogElement::UN_DEL_CHAR, y0 * m_CyChar, x0 * m_CxChar, y1 - y0 + 1 };
			AddLog(log);

			m_Caret.SetPos(article.GetCol(y0, x0) * m_CxChar, y0 * m_CyChar);
			article.DeleteMultiString(y0, x0, y1, x1);

			m_Caret.GetPos(xCaretPix, yCaretPix);
			// 加一条BACKSPACE纪录
			//         ClearLogAll();
			LogElement log2 = { LogElement::DEL_CHAR, yCaretPix, xCaretPix, NULL };
			AddLog(log2);
		}

		//       article.GetRows() = article.GetRows();
		//       article.GetColMax() = article.GetColMax();
		//       ScrollResize(SB_VERT, article.GetRows());
		//       ScrollResize(SB_HORZ, article.GetColMax());
		// 若超出客户区范围，则刷新滚动条
		m_Caret.GetPos(xCaretPix, yCaretPix);
		if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
		{
			//         article.GetColMax() = article.GetColMax();
			ScrollMove(SB_HORZ, xCaretPix / m_CxChar);
			ScrollResize(SB_HORZ, article.GetColMax());
		}
		if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
		{
			//         article.GetRows() = article.GetRows();
			ScrollMove(SB_VERT, yCaretPix / m_CyChar);
			ScrollResize(SB_VERT, article.GetRows());
		}

		m_selector.Reset();
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		InvalidateRect(m_hWnd, &rect, FALSE);   // 窗口重绘
		UpdateWindow();
		break;
	}
	case VK_HOME:                     // 插入符到行首
	{
		m_Caret.GetPos(xCaretPix, yCaretPix);
		//       xCaret = 0;
		xCaretPix = 0;
		xCaretPixelSave = xCaretPix;

		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		{
			//////////////////////////////////////////////////////////////////////////
			//         int x_pos = (LOWORD(lParam) + m_Caret.xOffset) / m_CxChar;
			//         int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
			// 
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);
			//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			//         yCaretPix = y_pos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}

			m_selector.Point1.x = index;
			m_selector.Point1.y = line;   // 块选择结构结束点

			if (m_selector.Point1.y >= m_selectorStart.Point0.y)
			{
				m_selector.rows = m_selector.Point1.y - m_selectorStart.Point0.y + 1;
			}
			else
			{
				m_selector.rows = m_selector.Point0.y - m_selectorStart.Point1.y + 1;
			}
			m_selector.selected = TRUE;
			//         m_selector.start = FALSE;
			m_selector.Point0.x = m_selectorStart.Point0.x;
			m_selector.Point0.y = m_selectorStart.Point0.y;

			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);
			UpdateWindow();
			//////////////////////////////////////////////////////////////////////////
		}
		else
		{
			//         int x_pos = (LOWORD(lParam) + m_Caret.xOffset) / m_CxChar;
			//         int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
			// 
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);
			//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			//         yCaretPix = y_pos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}
			m_Caret.SetPos(xCaretPix, yCaretPix);         // 设定坐标

			//           if (m_selector.selected)
			//           {
			//             m_selector.start    = false;
			//             m_selector.selected = false;
			//           }
			//           else
			//           {
			m_selector.start = TRUE;
			m_selector.selected = FALSE;
			//           }
			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);
			UpdateWindow();
			//   m_selector.start = true;

			//   m_selector.Point0.x = index;
			//   m_selector.Point0.y = line;   // 块选择结构初始点
			m_selectorStart.Point0.x = index;
			m_selectorStart.Point0.y = line;
		}

		m_Caret.SetPos(xCaretPix, yCaretPix);

		if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
		{
			ScrollMove(SB_HORZ, SCROLL_MIN);
		}
		if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
		{
			ScrollMove(SB_VERT, yCaretPix / m_CyChar);
		}
		break;
	}
	case VK_END:                      // 插入符到行尾  // TODO
	{
		m_Caret.GetPos(xCaretPix, yCaretPix);

		int line = yCaretPix / m_CyChar;
		int sLen = article.GetCol(line);
		xCaretPix = sLen * m_CxChar;
		xCaretPixelSave = xCaretPix;

		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		{
			//////////////////////////////////////////////////////////////////////////
			//         int x_pos = (LOWORD(lParam) + m_Caret.xOffset) / m_CxChar;
			//         int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
			// 
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);
			//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			//         yCaretPix = y_pos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}

			m_selector.Point1.x = index;
			m_selector.Point1.y = line;   // 块选择结构结束点

			if (m_selector.Point1.y >= m_selectorStart.Point0.y)
			{
				m_selector.rows = m_selector.Point1.y - m_selectorStart.Point0.y + 1;
			}
			else
			{
				m_selector.rows = m_selector.Point0.y - m_selectorStart.Point1.y + 1;
			}
			m_selector.selected = TRUE;
			//         m_selector.start = FALSE;
			m_selector.Point0.x = m_selectorStart.Point0.x;
			m_selector.Point0.y = m_selectorStart.Point0.y;

			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);
			UpdateWindow();
			//////////////////////////////////////////////////////////////////////////
		}
		else
		{
			//         int x_pos = (LOWORD(lParam) + m_Caret.xOffset) / m_CxChar;
			//         int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
			// 
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);
			//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			//         yCaretPix = y_pos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}
			m_Caret.SetPos(xCaretPix, yCaretPix);         // 设定坐标

			//           if (m_selector.selected)
			//           {
			//             m_selector.start    = false;
			//             m_selector.selected = false;
			//           }
			//           else
			//           {
			m_selector.start = TRUE;
			m_selector.selected = FALSE;
			//           }
			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);
			UpdateWindow();
			//   m_selector.start = true;

			//   m_selector.Point0.x = index;
			//   m_selector.Point0.y = line;   // 块选择结构初始点
			m_selectorStart.Point0.x = index;
			m_selectorStart.Point0.y = line;
		}

		m_Caret.SetPos(xCaretPix, yCaretPix);
		if (xCaretPix >= m_Caret.xOffset && xCaretPix <= (m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT)   // 插入符在客户区时
			&& yCaretPix >= m_Caret.yOffset && yCaretPix <= (m_Caret.yOffset + m_CyClient - m_CyChar))
		{
			//////             xCaret = xPage;
			//////             xCaretPix = Caret.xOffset + xCaret * m_CxChar;
			////TCHAR *pBegin = GetTextPtr(m_pBuffer, yCaretPix / 16);
			////int sLen = GetTextPtrEnd(m_pBuffer, yCaretPix / 16) - pBegin;
			//int line = yCaretPix/m_CyChar;
			//int sLen = article.GetCount(line);
			//xCaretPix = sLen * m_CxChar;
			//xCaretPixelSave = xCaretPix;
			//m_Caret.SetPos(xCaretPix, yCaretPix);

			//ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
		}
		else                                                    // 不在客户区时
		{
			ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
		}
		break;
	}
	case VK_LEFT:                     // 步进 ←
	{
		int repeatTimes = LOWORD(lParam);
		if (repeatTimes < 1)
			repeatTimes = 1;
		m_Caret.GetPos(xCaretPix, yCaretPix);
		for (int i = 0; i<repeatTimes; i++)
		{
			if (xCaretPix <= 0)     // 向左移动到行首时继续移动则到上行行尾
			{
				int line = yCaretPix / 16;
				if (line > 0)
				{
					//           TCHAR *pBegin = GetTextPtr(m_pBuffer, collum - 1);     // 竖直方向
					//           int sLen = GetTextPtrEnd(m_pBuffer, collum - 1) - pBegin;
					int sLen = article.GetCol(line - 1);
					int pixelLen = sLen * m_CxChar;
					xCaretPix = pixelLen;
					yCaretPix = yCaretPix - m_CyChar;

					if (xCaretPix >= m_Caret.xOffset && xCaretPix <= (m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT)   // 插入符在客户区时
						&& yCaretPix >= m_Caret.yOffset && yCaretPix <= (m_Caret.yOffset + m_CyClient - m_CyChar))
					{
						//////             xCaret = xPage;
						//////             xCaretPix = Caret.xOffset + xCaret * m_CxChar;
						////TCHAR *pBegin = GetTextPtr(m_pBuffer, yCaretPix / 16);
						////int sLen = GetTextPtrEnd(m_pBuffer, yCaretPix / 16) - pBegin;
						//int line = yCaretPix/m_CyChar;
						//int sLen = article.GetCount(line);
						//xCaretPix = sLen * m_CxChar;
						//xCaretPixelSave = xCaretPix;
						//m_Caret.SetPos(xCaretPix, yCaretPix);

						//ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
					}
					else                                                    // 不在客户区时
					{
						ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
					}
				}
				else                  // 已处于第一行
				{
					xCaretPix = 0;
					yCaretPix = yCaretPix;
				}
			}
			else
			{
				int line = yCaretPix / m_CyChar;
				int col = xCaretPix / m_CxChar;

				int dataIndex = article.GetDataIndex(line, col);
				if (article.GetDataType(article.GetDataByIndex(line, dataIndex - 1)))
				{
					xCaretPix = max(xCaretPix - m_CxChar * 2, 0);
				}
				else
				{
					xCaretPix = max(xCaretPix - m_CxChar, 0);
				}

				//           xCaretPix = max(xCaretPix - m_CxChar, 0);
				//           xCaretPix = max(xCaretPix - realCxChar, 0);
				//           ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
			}

			xCaretPixelSave = xCaretPix;
		}

		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		{
			//////////////////////////////////////////////////////////////////////////
			//         int x_pos = (LOWORD(lParam) + m_Caret.xOffset) / m_CxChar;
			//         int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
			// 
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);
			//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			//         yCaretPix = y_pos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}

			m_selector.Point1.x = index;
			m_selector.Point1.y = line;   // 块选择结构结束点

			if (m_selector.Point1.y >= m_selectorStart.Point0.y)
			{
				m_selector.rows = m_selector.Point1.y - m_selectorStart.Point0.y + 1;
			}
			else
			{
				m_selector.rows = m_selector.Point0.y - m_selectorStart.Point1.y + 1;
			}
			m_selector.selected = TRUE;
			//         m_selector.start = FALSE;
			m_selector.Point0.x = m_selectorStart.Point0.x;
			m_selector.Point0.y = m_selectorStart.Point0.y;

			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);
			UpdateWindow();
			//////////////////////////////////////////////////////////////////////////
		}
		else
		{
			//         int x_pos = (LOWORD(lParam) + m_Caret.xOffset) / m_CxChar;
			//         int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
			// 
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);
			//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			//         yCaretPix = y_pos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}
			m_Caret.SetPos(xCaretPix, yCaretPix);         // 设定坐标

			//           if (m_selector.selected)
			//           {
			//             m_selector.start    = false;
			//             m_selector.selected = false;
			//           }
			//           else
			//           {
			m_selector.start = TRUE;
			m_selector.selected = FALSE;
			//           }
			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);
			UpdateWindow();
			//   m_selector.start = true;

			//   m_selector.Point0.x = index;
			//   m_selector.Point0.y = line;   // 块选择结构初始点
			m_selectorStart.Point0.x = index;
			m_selectorStart.Point0.y = line;
		}

		m_Caret.SetPos(xCaretPix, yCaretPix);

		if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
		{
			ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - 5));
		}
		if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
		{
			ScrollMove(SB_VERT, (yCaretPix / m_CyChar));
		}
		//       if (!GetKeyState(VK_SHIFT))
		//       {
		//         m_Caret.GetPos(xCaretPix, yCaretPix);
		//         int x_pos = xCaretPix / m_CxChar;
		//         int y_pos = yCaretPix / m_CyChar;
		// 
		//         int line = y_pos;
		//         int col  = x_pos;
		//         int index = article.GetDataIndex(line, col);
		//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
		//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
		//         yCaretPix = y_pos * m_CyChar;
		//         //   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
		//         if (yCaretPix/m_CyChar > (article.GetRows() - 1))
		//         {
		//           yCaretPix = (article.GetRows() - 1) * m_CyChar;
		//         }
		//         if (xCaretPix/m_CxChar > article.GetCol(yCaretPix/m_CyChar))
		//         {
		//           xCaretPix = article.GetCol(yCaretPix/m_CyChar) * m_CxChar;
		//         }
		//         m_Caret.SetPos(xCaretPix, yCaretPix);         // 设定坐标
		// 
		//         if (m_selector.selected)
		//         {
		//           m_selector.selected = false;
		//           RECT rect;
		//           GetClientRect(m_hWnd, &rect);
		//           InvalidateRect(m_hWnd, &rect, FALSE);
		//           UpdateWindow();
		//         }
		//         //   m_selector.selected = false;
		//         m_selector.start = true;
		//         //   m_selector.Point0.x = xCaretPix / m_CxChar;
		// 
		//         m_selector.Point0.x = index;
		//         m_selector.Point0.y = line;   // 块选择结构初始点
		//       }
		//       if (GetKeyState(VK_SHIFT))
		//       {
		//         printf("Shift\n");
		//         static int xPosSave=0, yPosSave=0;
		// 
		//         m_Caret.GetPos(xCaretPix, yCaretPix);
		//         int x_pos = xCaretPix / m_CxChar;
		//         int y_pos = yCaretPix / m_CyChar;
		// 
		//         int line = y_pos;
		//         int col  = x_pos;
		//         int index = article.GetDataIndex(line, col);
		//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
		//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
		//         yCaretPix = y_pos * m_CyChar;
		//         //   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
		//         if (yCaretPix/m_CyChar > (article.GetRows() - 1))
		//         {
		//           yCaretPix = (article.GetRows() - 1) * m_CyChar;
		//         }
		//         if (xCaretPix/m_CxChar > article.GetCol(yCaretPix/m_CyChar))
		//         {
		//           xCaretPix = article.GetCol(yCaretPix/m_CyChar) * m_CxChar;
		//         }
		// 
		//         if (m_selector.start == TRUE)
		//         {
		//           m_selector.Point1.x = index;
		//           m_selector.Point1.y = line;   // 块选择结构结束点
		//           if (m_selector.Point0.y > m_selector.Point1.y)  // 如果向上选择则交换坐标
		//           {
		//             xPosSave = m_selector.Point0.x;
		//             yPosSave = m_selector.Point0.y;
		//             int temp = m_selector.Point0.y;
		//             m_selector.Point0.y = m_selector.Point1.y;
		//             m_selector.Point1.y = temp;
		//             temp = m_selector.Point0.x;
		//             m_selector.Point0.x = m_selector.Point1.x;
		//             m_selector.Point1.x = temp;
		//           }
		//           else if (m_selector.Point0.y == m_selector.Point1.y 
		//             && m_selector.Point0.x > m_selector.Point1.x)
		//           {
		//             xPosSave = m_selector.Point0.x;
		//             yPosSave = m_selector.Point0.y;
		//             int temp = m_selector.Point0.x;
		//             m_selector.Point0.x = m_selector.Point1.x;
		//             m_selector.Point1.x = temp;
		//           }
		//           else
		//           {
		//             xPosSave = 0;
		//             yPosSave = 0;
		//           }
		//           m_selector.rows = m_selector.Point1.y - m_selector.Point0.y + 1;
		//           //      m_selector.selected = TRUE;
		//           ////       m_selector.start = FALSE;
		//           if ( m_selector.Point0.x == m_selector.Point1.x
		//             && m_selector.Point0.y == m_selector.Point1.y)
		//           {
		//             m_selector.selected = FALSE;
		//           }
		//           else
		//           {
		//             m_selector.selected = TRUE;
		//             //         m_selector.start = FALSE;
		// 
		//             RECT rect;
		//             GetClientRect(m_hWnd, &rect);
		//             InvalidateRect(m_hWnd, &rect, FALSE);
		//             UpdateWindow();
		//           }
		// 
		//           if (xPosSave == 0 && yPosSave == 0)
		//           {
		//           }
		//           else
		//           {
		//             int temp = m_selector.Point0.y;
		//             m_selector.Point0.y = m_selector.Point1.y;
		//             m_selector.Point1.y = temp;
		//             temp = m_selector.Point0.x;
		//             m_selector.Point0.x = m_selector.Point1.x;
		//             m_selector.Point1.x = temp;
		//           }
		//         }
		//       }
		break;
	}
	case VK_RIGHT:                    // 步进 →
	{
		int repeatTimes = LOWORD(lParam);
		if (repeatTimes < 1)
			repeatTimes = 1;
		m_Caret.GetPos(xCaretPix, yCaretPix);
		for (int i = 0; i<repeatTimes; i++)
		{
			//      TCHAR *pBegin = GetTextPtr(m_pBuffer, yCaretPix / 16);
			//      int sLen = GetTextPtrEnd(m_pBuffer, yCaretPix / 16) - pBegin;
			int sLen = article.GetCol(yCaretPix / m_CyChar);
			int pixelLen = sLen * m_CxChar;

			if (xCaretPix >= pixelLen)     // 向右移动到行尾时继续移动则到下行行首
			{
				int line = yCaretPix / 16;
				if (line < article.GetRows() - 1)
				{
					//           TCHAR *ppBegin = GetTextPtr(m_pBuffer, collum + 1);
					//           int ssLen = GetTextPtrEnd(m_pBuffer, collum + 1) - ppBegin;
					int ssLen = article.GetCol(line + 1);
					int ppixelLen = ssLen * m_CxChar;
					xCaretPix = 0;
					yCaretPix = yCaretPix + m_CyChar;
				}
				else
				{
					xCaretPix = pixelLen;
					yCaretPix = yCaretPix;
				}
			}
			else
			{
				int line = yCaretPix / m_CyChar;
				int col = xCaretPix / m_CxChar;
				int realCxChar = GetRealCxChar(line, col);
				xCaretPix = min(xCaretPix + realCxChar, pixelLen);
			}

			xCaretPixelSave = xCaretPix;
		}

		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		{
			//////////////////////////////////////////////////////////////////////////
			//         int x_pos = (LOWORD(lParam) + m_Caret.xOffset) / m_CxChar;
			//         int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
			// 
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);
			//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			//         yCaretPix = y_pos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}

			m_selector.Point1.x = index;
			m_selector.Point1.y = line;   // 块选择结构结束点

			if (m_selector.Point1.y >= m_selectorStart.Point0.y)
			{
				m_selector.rows = m_selector.Point1.y - m_selectorStart.Point0.y + 1;
			}
			else
			{
				m_selector.rows = m_selector.Point0.y - m_selectorStart.Point1.y + 1;
			}
			m_selector.selected = TRUE;
			//         m_selector.start = FALSE;
			m_selector.Point0.x = m_selectorStart.Point0.x;
			m_selector.Point0.y = m_selectorStart.Point0.y;

			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);
			UpdateWindow();
			//////////////////////////////////////////////////////////////////////////
		}
		else
		{
			//         int x_pos = (LOWORD(lParam) + m_Caret.xOffset) / m_CxChar;
			//         int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
			// 
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);
			//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			//         yCaretPix = y_pos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}
			m_Caret.SetPos(xCaretPix, yCaretPix);         // 设定坐标

			//           if (m_selector.selected)
			//           {
			//             m_selector.start    = false;
			//             m_selector.selected = false;
			//           }
			//           else
			//           {
			m_selector.start = TRUE;
			m_selector.selected = FALSE;
			//           }
			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);
			UpdateWindow();
			//   m_selector.start = true;

			//   m_selector.Point0.x = index;
			//   m_selector.Point0.y = line;   // 块选择结构初始点
			m_selectorStart.Point0.x = index;
			m_selectorStart.Point0.y = line;
		}

		m_Caret.SetPos(xCaretPix, yCaretPix);

		if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
		{
			ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
		}
		if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
		{
			ScrollMove(SB_VERT, (yCaretPix / m_CyChar - m_CyClient / m_CyChar + 1));
		}
		break;
	}
	case VK_UP:                       // 步进 ↑
	{
		m_Caret.GetPos(xCaretPix, yCaretPix);
		yCaretPix = max(yCaretPix - m_CyChar, 0);

		//TCHAR *pBegin = GetTextPtr(m_pBuffer, yCaretPix / 16);
		//int sLen = GetTextPtrEnd(m_pBuffer, yCaretPix / 16) - pBegin;
		int sLen = article.GetCol(yCaretPix / m_CyChar);
		int pixelLen = sLen * m_CxChar;

		if (xCaretPixelSave >= pixelLen)
		{
			xCaretPix = pixelLen;
		}
		else
		{
			int line = yCaretPix / m_CyChar;
			int col = xCaretPixelSave / m_CxChar;
			int index = article.GetDataIndex(line, col);
			xCaretPix = article.GetCol(line, index) * m_CxChar;
		}

		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		{
			//////////////////////////////////////////////////////////////////////////
			//         int x_pos = (LOWORD(lParam) + m_Caret.xOffset) / m_CxChar;
			//         int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
			// 
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);
			//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			//         yCaretPix = y_pos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}

			m_selector.Point1.x = index;
			m_selector.Point1.y = line;   // 块选择结构结束点

			if (m_selector.Point1.y >= m_selectorStart.Point0.y)
			{
				m_selector.rows = m_selector.Point1.y - m_selectorStart.Point0.y + 1;
			}
			else
			{
				m_selector.rows = m_selector.Point0.y - m_selectorStart.Point1.y + 1;
			}
			m_selector.selected = TRUE;
			//         m_selector.start = FALSE;
			m_selector.Point0.x = m_selectorStart.Point0.x;
			m_selector.Point0.y = m_selectorStart.Point0.y;

			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);
			UpdateWindow();
			//////////////////////////////////////////////////////////////////////////
		}
		else
		{
			//         int x_pos = (LOWORD(lParam) + m_Caret.xOffset) / m_CxChar;
			//         int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
			// 
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);
			//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			//         yCaretPix = y_pos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}
			m_Caret.SetPos(xCaretPix, yCaretPix);         // 设定坐标

			//           if (m_selector.selected)
			//           {
			//             m_selector.start    = false;
			//             m_selector.selected = false;
			//           }
			//           else
			//           {
			m_selector.start = TRUE;
			m_selector.selected = FALSE;
			//           }
			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);
			UpdateWindow();
			//   m_selector.start = true;

			//   m_selector.Point0.x = index;
			//   m_selector.Point0.y = line;   // 块选择结构初始点
			m_selectorStart.Point0.x = index;
			m_selectorStart.Point0.y = line;
		}

		m_Caret.SetPos(xCaretPix, yCaretPix);

		if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
		{
			ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
		}
		if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
		{
			ScrollMove(SB_VERT, (yCaretPix / m_CyChar));
		}
		break;
	}
	case VK_DOWN:                     // 步进 ↓
	{
		m_Caret.GetPos(xCaretPix, yCaretPix);
		//       article.GetRows() = article.GetRows();
		yCaretPix = min(yCaretPix + m_CyChar, (article.GetRows() - 1) * m_CyChar);

		//       TCHAR *pBegin = GetTextPtr(m_pBuffer, yCaretPix / 16);
		//       int sLen = GetTextPtrEnd(m_pBuffer, yCaretPix / 16) - pBegin;
		int sLen = article.GetCol(yCaretPix / m_CyChar);
		int pixelLen = sLen * m_CxChar;

		if (xCaretPixelSave >= pixelLen)
		{
			xCaretPix = pixelLen;
		}
		else
		{
			int line = yCaretPix / m_CyChar;
			int col = xCaretPixelSave / m_CxChar;
			int index = article.GetDataIndex(line, col);
			xCaretPix = article.GetCol(line, index) * m_CxChar;
		}

		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		{
			//////////////////////////////////////////////////////////////////////////
			//         int x_pos = (LOWORD(lParam) + m_Caret.xOffset) / m_CxChar;
			//         int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
			// 
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);
			//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			//         yCaretPix = y_pos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}

			m_selector.Point1.x = index;
			m_selector.Point1.y = line;   // 块选择结构结束点

			if (m_selector.Point1.y >= m_selectorStart.Point0.y)
			{
				m_selector.rows = m_selector.Point1.y - m_selectorStart.Point0.y + 1;
			}
			else
			{
				m_selector.rows = m_selector.Point0.y - m_selectorStart.Point1.y + 1;
			}
			m_selector.selected = TRUE;
			//         m_selector.start = FALSE;
			m_selector.Point0.x = m_selectorStart.Point0.x;
			m_selector.Point0.y = m_selectorStart.Point0.y;

			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);
			UpdateWindow();
			//////////////////////////////////////////////////////////////////////////
		}
		else
		{
			//         int x_pos = (LOWORD(lParam) + m_Caret.xOffset) / m_CxChar;
			//         int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
			// 
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);
			//         xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//         //   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			//         yCaretPix = y_pos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}
			m_Caret.SetPos(xCaretPix, yCaretPix);         // 设定坐标

			//           if (m_selector.selected)
			//           {
			//             m_selector.start    = false;
			//             m_selector.selected = false;
			//           }
			//           else
			//           {
			m_selector.start = TRUE;
			m_selector.selected = FALSE;
			//           }
			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);
			UpdateWindow();
			//   m_selector.start = true;

			//   m_selector.Point0.x = index;
			//   m_selector.Point0.y = line;   // 块选择结构初始点
			m_selectorStart.Point0.x = index;
			m_selectorStart.Point0.y = line;
		}

		m_Caret.SetPos(xCaretPix, yCaretPix);

		if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
		{
			ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
		}
		if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
		{
			//           ScrollMove(SB_VERT, (yCaretPix / m_CyChar));
			ScrollMove(SB_VERT, (yCaretPix / m_CyChar - m_CyClient / m_CyChar + 1));
		}
		break;
	}
	case VK_PRIOR:                    // 翻页 ↑
		SendMessage(m_hWnd, WM_VSCROLL, SB_PAGEUP, 0);
		SendMessage(m_hWnd, WM_LBUTTONDOWN, 0, 0);
		break;
	case VK_NEXT:                     // 翻页 ↓
		SendMessage(m_hWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
		SendMessage(m_hWnd, WM_LBUTTONDOWN, 0, 0);
		break;
	}
	//   cout << "X: " << xCaretPix << '\t' << "Y: " << yCaretPix << endl;
	//   cout << "列号: " << xCaretPix / m_CxChar << '\t' << "行号: " << '\t' << yCaretPix / 16 << endl;
}

void XEdit::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	short x_pos = LOWORD(lParam);
	short y_pos = HIWORD(lParam);

	if (x_pos < 0)
	{
		x_pos = 0;
	}
	if (y_pos < 0)
	{
		y_pos = 0;
	}

	x_pos = (x_pos + m_Caret.xOffset - EDIT_MARGIN_LEFT) / m_CxChar;
	y_pos = (y_pos + m_Caret.yOffset) / m_CyChar;

	if (y_pos > article.GetRows() - 1)
	{
		y_pos = article.GetRows() - 1;
	}
	int line = y_pos;
	int col = x_pos;
	int index = article.GetDataIndex(line, col);
	xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
	//   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
	yCaretPix = y_pos * m_CyChar;
	//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
	if (yCaretPix / m_CyChar > (article.GetRows() - 1))
	{
		yCaretPix = (article.GetRows() - 1) * m_CyChar;
	}
	if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
	{
		xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
	}
	m_Caret.SetPos(xCaretPix, yCaretPix);         // 设定坐标

	if (m_selector.selected)
	{
		m_selector.start = false;
		m_selector.selected = false;
	}
	else
	{
		m_selector.start = TRUE;
	}
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	InvalidateRect(m_hWnd, &rect, FALSE);
	UpdateWindow();
	//   m_selector.start = true;

	//   m_selector.Point0.x = index;
	//   m_selector.Point0.y = line;   // 块选择结构初始点
	m_selectorStart.Point0.x = index;
	m_selectorStart.Point0.y = line;
}

void XEdit::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	short x_pos = LOWORD(lParam);
	short y_pos = HIWORD(lParam);

	if (x_pos < 0)
	{
		x_pos = 0;
	}
	if (y_pos < 0)
	{
		y_pos = 0;
	}

	x_pos = (x_pos + m_Caret.xOffset - EDIT_MARGIN_LEFT) / m_CxChar;
	y_pos = (y_pos + m_Caret.yOffset) / m_CyChar;

	if (y_pos > article.GetRows() - 1)
	{
		y_pos = article.GetRows() - 1;
	}
	int line = y_pos;
	int col = x_pos;
	int index = article.GetDataIndex(line, col);
	xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
	//   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
	yCaretPix = y_pos * m_CyChar;
	//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
	if (yCaretPix / m_CyChar > (article.GetRows() - 1))
	{
		yCaretPix = (article.GetRows() - 1) * m_CyChar;
	}
	if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
	{
		xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
	}
	m_Caret.SetPos(xCaretPix, yCaretPix);         // 设定坐标

	if (m_selector.selected)
	{
		m_selector.start = false;
		m_selector.selected = false;
	}
	else
	{
		m_selector.start = TRUE;
	}
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	InvalidateRect(m_hWnd, &rect, FALSE);
	UpdateWindow();
	//   m_selector.start = true;

	//   m_selector.Point0.x = index;
	//   m_selector.Point0.y = line;   // 块选择结构初始点
	m_selectorStart.Point0.x = index;
	m_selectorStart.Point0.y = line;
}

void XEdit::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	//   static int xPosSave=0, yPosSave=0;

	//   cout << "X: " << (short)LOWORD(lParam) << "  Y: " << (short)HIWORD(lParam) << endl;

	if ((wParam & MK_LBUTTON) || VIRTUAL_MK_LBUTTON)
	{
		//     int x_pos = (LOWORD(lParam) + m_Caret.xOffset - EDIT_MARGIN_LEFT) / m_CxChar;
		//     int y_pos = (HIWORD(lParam) + m_Caret.yOffset) / m_CyChar;
		short x_pos = LOWORD(lParam);
		short y_pos = HIWORD(lParam);
		//     cout << "X: " << x_pos << "  Y: " << y_pos << endl;

		if (x_pos < 0)
		{
			x_pos = 0;
		}
		if (y_pos < 0)
		{
			y_pos = 0;
		}

		x_pos = (x_pos + m_Caret.xOffset - EDIT_MARGIN_LEFT) / m_CxChar;
		y_pos = (y_pos + m_Caret.yOffset) / m_CyChar;

		if (y_pos > article.GetRows() - 1)
		{
			y_pos = article.GetRows() - 1;
		}

		int line = y_pos;
		int col = x_pos;
		int index = article.GetDataIndex(line, col);
		xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
		//   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
		yCaretPix = y_pos * m_CyChar;
		//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
		if (yCaretPix / m_CyChar > (article.GetRows() - 1))
		{
			yCaretPix = (article.GetRows() - 1) * m_CyChar;
		}
		if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
		{
			xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
		}

		//     if (m_selector.start == TRUE)
		//     {
		m_selector.Point1.x = index;
		m_selector.Point1.y = line;   // 块选择结构结束点
		//     m_selector.Point1.x = xCaretPix / m_CxChar;
		//     m_selector.Point1.y = yCaretPix / m_CyChar;   // 块选择结构结束点
		//       if (m_selector.Point0.y > m_selector.Point1.y)  // 如果向上选择则交换坐标
		//       {
		//         xPosSave = m_selector.Point0.x;
		//         yPosSave = m_selector.Point0.y;
		//         int temp = m_selector.Point0.y;
		//         m_selector.Point0.y = m_selector.Point1.y;
		//         m_selector.Point1.y = temp;
		//         temp = m_selector.Point0.x;
		//         m_selector.Point0.x = m_selector.Point1.x;
		//         m_selector.Point1.x = temp;
		//       }
		//       else if (m_selector.Point0.y == m_selector.Point1.y 
		//         && m_selector.Point0.x > m_selector.Point1.x)
		//       {
		//         xPosSave = m_selector.Point0.x;
		//         yPosSave = m_selector.Point0.y;
		//         int temp = m_selector.Point0.x;
		//         m_selector.Point0.x = m_selector.Point1.x;
		//         m_selector.Point1.x = temp;
		//       }
		//       else
		//       {
		//         xPosSave = 0;
		//         yPosSave = 0;
		//       }
		if (m_selector.Point1.y >= m_selectorStart.Point0.y)
		{
			m_selector.rows = m_selector.Point1.y - m_selectorStart.Point0.y + 1;
		}
		else
		{
			m_selector.rows = m_selector.Point0.y - m_selectorStart.Point1.y + 1;
		}
		//      m_selector.selected = TRUE;
		////       m_selector.start = FALSE;
		//       if ( m_selectorStart.Point0.x == m_selector.Point1.x
		//         && m_selectorStart.Point0.y == m_selector.Point1.y)
		//       {
		//         m_selector.selected = FALSE;
		//       }
		//       else
	  {
		  m_selector.selected = TRUE;
		  //         m_selector.start = FALSE;
		  m_selector.Point0.x = m_selectorStart.Point0.x;
		  m_selector.Point0.y = m_selectorStart.Point0.y;

		  RECT rect;
		  GetClientRect(m_hWnd, &rect);
		  InvalidateRect(m_hWnd, &rect, FALSE);
		  UpdateWindow();
	  }

	  //       if (xPosSave == 0 && yPosSave == 0)
	  //       {
	  //       }
	  //       else
	  //       {
	  //         int temp = m_selector.Point0.y;
	  //         m_selector.Point0.y = m_selector.Point1.y;
	  //         m_selector.Point1.y = temp;
	  //         temp = m_selector.Point0.x;
	  //         m_selector.Point0.x = m_selector.Point1.x;
	  //         m_selector.Point1.x = temp;
	  //       }
	  //     }
	}
}

void XEdit::OnCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef UNICODE
#define CF_TCHAR    CF_UNICODETEXT
#else
#define CF_TCHAR    CF_TEXT
#endif // UNICODE

	//   MessageBox(m_hWnd, TEXT("OnCommand"), TEXT("Notice"), NULL);
	switch (LOWORD(wParam))
	{
	case IDM_EDIT_COPY:
	{
		if (m_selector.Point0.y > m_selector.Point1.y)  // 如果向上选择则交换坐标
		{
			int temp = m_selector.Point0.y;
			m_selector.Point0.y = m_selector.Point1.y;
			m_selector.Point1.y = temp;
			temp = m_selector.Point0.x;
			m_selector.Point0.x = m_selector.Point1.x;
			m_selector.Point1.x = temp;
		}
		else if (m_selector.Point0.y == m_selector.Point1.y
			&& m_selector.Point0.x > m_selector.Point1.x)
		{
			int temp = m_selector.Point0.x;
			m_selector.Point0.x = m_selector.Point1.x;
			m_selector.Point1.x = temp;
		}

		int x0 = m_selector.Point0.x;
		int y0 = m_selector.Point0.y;
		int x1 = m_selector.Point1.x;
		int y1 = m_selector.Point1.y;
		m_Caret.SetPos(article.GetCol(y0, x0) * m_CxChar, y0 * m_CyChar);

		int num = article.GetRealLength(x0, y0, x1, y1);
		if (!num)
			return;

		TCHAR *str = (TCHAR *)malloc((num + 1) * sizeof(TCHAR));
		article.CopyMultiString(str, x0, y0, x1, y1, num);
		str[num] = TEXT('\0');
		//       printf("num = %d\n", num);
		//       MessageBox(m_hWnd, str, TEXT("OnCommand"), NULL);
		//       MessageBox(m_hWnd, TEXT("Copy"), TEXT("OnCommand"), NULL);
		HGLOBAL hGlobal = GlobalAlloc(GHND | GMEM_SHARE,
			(num + 1) * sizeof(TCHAR));
		PTSTR pGlobal = (PTSTR)GlobalLock(hGlobal);
		lstrcpy(pGlobal, str);
		GlobalUnlock(hGlobal);

		OpenClipboard(m_hWnd);
		EmptyClipboard();
		SetClipboardData(CF_TCHAR, hGlobal);
		CloseClipboard();
		free(str);

		RECT rect;
		GetClientRect(m_hWnd, &rect);
		InvalidateRect(m_hWnd, &rect, FALSE);
		UpdateWindow();
		return;
	}
	case IDM_EDIT_CUT:
	{
		if (m_selector.Point0.y > m_selector.Point1.y)  // 如果向上选择则交换坐标
		{
			int temp = m_selector.Point0.y;
			m_selector.Point0.y = m_selector.Point1.y;
			m_selector.Point1.y = temp;
			temp = m_selector.Point0.x;
			m_selector.Point0.x = m_selector.Point1.x;
			m_selector.Point1.x = temp;
		}
		else if (m_selector.Point0.y == m_selector.Point1.y
			&& m_selector.Point0.x > m_selector.Point1.x)
		{
			int temp = m_selector.Point0.x;
			m_selector.Point0.x = m_selector.Point1.x;
			m_selector.Point1.x = temp;
		}

		int x0 = m_selector.Point0.x;
		int y0 = m_selector.Point0.y;
		int x1 = m_selector.Point1.x;
		int y1 = m_selector.Point1.y;
		m_Caret.SetPos(article.GetCol(y0, x0) * m_CxChar, y0 * m_CyChar);

		m_Caret.GetPos(xCaretPix, yCaretPix);
		// 添加一条UN_CUT纪录
		//         ClearLogAll();
		LogElement log = { LogElement::UN_CUT, yCaretPix, xCaretPix, y1 - y0 + 1 };
		AddLog(log);

		int num = article.GetRealLength(x0, y0, x1, y1);
		TCHAR *str = (TCHAR *)malloc((num + 1) * sizeof(TCHAR));
		article.CopyMultiString(str, x0, y0, x1, y1, num);
		article.DeleteMultiString(y0, x0, y1, x1);

		m_Caret.GetPos(xCaretPix, yCaretPix);
		// 添加一条CUT纪录
		//         ClearLogAll();
		LogElement log2 = { LogElement::CUT, yCaretPix, xCaretPix, NULL };
		AddLog(log2);

		//       printf("num = %d\n", num);
		//       MessageBox(m_hWnd, str, TEXT("OnCommand"), NULL);
		//       MessageBox(m_hWnd, TEXT("Copy"), TEXT("OnCommand"), NULL);
		HGLOBAL hGlobal = GlobalAlloc(GHND | GMEM_SHARE,
			(num + 1) * sizeof(TCHAR));
		PTSTR pGlobal = (PTSTR)GlobalLock(hGlobal);
		lstrcpy(pGlobal, str);
		GlobalUnlock(hGlobal);

		OpenClipboard(m_hWnd);
		EmptyClipboard();
		SetClipboardData(CF_TCHAR, hGlobal);
		CloseClipboard();
		free(str);

		m_selector.Reset();
		//       article.GetRows() = article.GetRows();
		//       article.GetColMax() = article.GetColMax();
		ScrollResize(SB_VERT, article.GetRows());
		ScrollResize(SB_HORZ, article.GetColMax());
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		InvalidateRect(m_hWnd, &rect, FALSE);
		UpdateWindow();
		return;
	}
	case IDM_EDIT_PASTE:
	{
		m_Caret.GetPos(xCaretPix, yCaretPix);

		int x0 = 0, y0 = 0, x1 = 0, y1 = 0;

		if (m_selector.selected || !m_selector.start)
		{
			if (m_selector.Point0.y > m_selector.Point1.y)  // 如果向上选择则交换坐标
			{
				int temp = m_selector.Point0.y;
				m_selector.Point0.y = m_selector.Point1.y;
				m_selector.Point1.y = temp;
				temp = m_selector.Point0.x;
				m_selector.Point0.x = m_selector.Point1.x;
				m_selector.Point1.x = temp;
			}
			else if (m_selector.Point0.y == m_selector.Point1.y
				&& m_selector.Point0.x > m_selector.Point1.x)
			{
				int temp = m_selector.Point0.x;
				m_selector.Point0.x = m_selector.Point1.x;
				m_selector.Point1.x = temp;
			}

			x0 = m_selector.Point0.x;
			y0 = m_selector.Point0.y;
			x1 = m_selector.Point1.x;
			y1 = m_selector.Point1.y;
			m_Caret.SetPos(article.GetCol(y0, x0) * m_CxChar, y0 * m_CyChar);
			m_Caret.GetPos(xCaretPix, yCaretPix);
			// 添加一条UN_CUT纪录
			//         ClearLogAll();
			LogElement log = { LogElement::UN_CUT, yCaretPix, xCaretPix, y1 - y0 + 1 };
			AddLog(log);

			article.DeleteMultiString(y0, x0, y1, x1);
			//         m_Caret.SetPos(m_selector.Point0.x * m_CxChar, m_selector.Point0.y * m_CyChar);
			m_selector.Reset();
		}
		OpenClipboard(m_hWnd);
		HGLOBAL hGlobal;
		PTSTR   pGlobal;
		PTSTR   pText = NULL;
		if (hGlobal = GetClipboardData(CF_TCHAR))
		{
			pGlobal = (PTSTR)GlobalLock(hGlobal);
			pText = (PTSTR)malloc(GlobalSize(hGlobal));
			lstrcpy(pText, pGlobal);
		}
		CloseClipboard();

		int realLength = lstrlen(pText);
		int tempLen = realLength;
		int lineCountTemp = 1;
		for (int i = 0; i<tempLen; i++)
		{
			if (pText[i] == TEXT('\r'))
			{
				realLength--;
				lineCountTemp++;
			}
		}

		// 添加一条UN_PASTE纪录
		//         ClearLogAll();
		LogElement log = { LogElement::UN_PASTE, yCaretPix, xCaretPix, lineCountTemp };
		AddLog(log);

		//       MessageBox(m_hWnd, pText, TEXT("OnCommand"), NULL);
		m_Caret.GetPos(xCaretPix, yCaretPix);
		//       int rowsPre = article.GetRows();
		article.InsertMultiString(yCaretPix / m_CyChar,
			article.GetDataIndex(yCaretPix / m_CyChar, xCaretPix / m_CxChar),
			pText, lstrlen(pText));
		//       int rowsNow = article.GetRows();
		//       int rowsDelta = rowsNow - rowsPre;
		//       for (int i=0; i<lstrlen(pText)-2*rowsDelta; i++)
		//       {
		//         ::SendMessage(m_hWnd, WM_KEYDOWN, VK_RIGHT, NULL);
		//       }

		//       article.GetRows() = article.GetRows();
		//       article.GetColMax() = article.GetColMax();
		ScrollResize(SB_VERT, article.GetRows());
		ScrollResize(SB_HORZ, article.GetColMax());

		::SendMessage(m_hWnd, WM_KEYDOWN, VK_RIGHT, realLength);

		if (pText != NULL)
		{
			free(pText);
		}

		m_Caret.GetPos(xCaretPix, yCaretPix);
		// 添加一条PASTE纪录
		//         ClearLogAll();
		LogElement log2 = { LogElement::PASTE, yCaretPix, xCaretPix, NULL };
		AddLog(log2);

		RECT rect;
		GetClientRect(m_hWnd, &rect);
		InvalidateRect(m_hWnd, &rect, FALSE);
		UpdateWindow();
		return;
	}
	case IDM_EDIT_ALL:          // 全选
	{
		m_selector.Point0.y = 0;
		m_selector.Point0.x = 0;
		m_selector.Point1.y = article.GetRows() - 1;
		m_selector.Point1.x = article.GetCol(article.GetRows() - 1);
		if (m_selector.Point1.y >= m_selector.Point0.y)
		{
			m_selector.rows = m_selector.Point1.y - m_selector.Point0.y + 1;
		}
		else
		{
			m_selector.rows = m_selector.Point0.y - m_selector.Point1.y + 1;
		}
		m_selector.selected = TRUE;
		m_selector.start = FALSE;
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		InvalidateRect(m_hWnd, &rect, FALSE);
		UpdateWindow();
		return;
	}
	case IDM_EDIT_UNDO:
	{
		Undo();
		return;
	}
	case IDM_EDIT_REDO:
	{
		Redo();
		return;
	}
	}
}

void XEdit::OnChar(WPARAM wParam, LPARAM lParam)
{
	//   m_selector.Reset();     // 取消选中
	//   printf("KEY_NUM = %d\n", wParam);
	for (int i = 0; i < (int)LOWORD(lParam); ++i)
	{
		switch (wParam)
		{
		case TEXT('\b'):               // backspace
		{
			break;
		}
		case TEXT('\t'):              // tab
		{
			m_selector.Reset();     // 取消选中
			//         printf("tab\n");
			m_Caret.GetPos(xCaretPix, yCaretPix);
			int tabUseSpace = 4 - (xCaretPix / m_CxChar % 4);   // 1*Tab = 4*Space
			::SendMessage(m_hWnd, WM_CHAR, TEXT(' '), tabUseSpace);
			break;
		}
		case '\n':              // line feed
		{
			break;
		}
		case TEXT('\r'):              // carriage return (Enter键)
		{
			if (!m_selector.selected && m_selector.start)             // 没有块选中，单字符删除
			{
			}
			else
			{
				::SendMessage(m_hWnd, WM_KEYDOWN, VK_BACK, NULL);
			}

			// 定位
			m_Caret.GetPos(xCaretPix, yCaretPix);

			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);

			// 按下回车时，清空之前的log，添加一条ENTER纪录
			//         ClearLogAll();
			LogElement log = { LogElement::UN_ENTER, yCaretPix, xCaretPix, NULL };
			AddLog(log);

			//         undoArticle.Initialize(article.Format());   // undo纪录

			// 分割字符串，换行
			article.SeparateRow(line, index);
			//         redoArticle.Initialize(article.Format());   // redo纪录

			m_Caret.SetPos(0, yCaretPix + m_CyChar);

			// 按下回车时，清空之前的log，添加一条ENTER纪录
			//         ClearLogAll();
			LogElement log2 = { LogElement::ENTER, yCaretPix + m_CyChar, 0, NULL };
			AddLog(log2);

			xCaretPixelSave = 0;    // 回车时不记忆上一行列号
			// 新增行后，若超出客户区范围，则刷新滚动条
			m_Caret.GetPos(xCaretPix, yCaretPix);
			if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
			{
				//           article.GetColMax() = article.GetColMax();
				ScrollResize(SB_HORZ, article.GetColMax());
				ScrollMove(SB_HORZ, SCROLL_MIN);
			}
			if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
			{
				//           article.GetRows() = article.GetRows();
				ScrollResize(SB_VERT, article.GetRows());
				::SendMessage(m_hWnd, WM_KEYDOWN, VK_UP, NULL);   // 插入换行后插入符向上移动(这两步只是为了滚动页面)
				::SendMessage(m_hWnd, WM_KEYDOWN, VK_DOWN, NULL); // 插入换行后插入符向下移动
			}

			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);   // 窗口重绘
			UpdateWindow();
			break;
		}
		case TEXT('\x1B'):            // escape
		{
			printf("escape\n");
			break;
		}
		default:                // character codes
		{
			m_selector.Reset();     // 取消选中
			// 定位
			m_Caret.GetPos(xCaretPix, yCaretPix);
			//         int line = yCaretPix / m_CyChar;
			//         int col  = xCaretPix / m_CxChar;
			int line = yCaretPix / m_CyChar;
			int col = xCaretPix / m_CxChar;
			int index = article.GetDataIndex(line, col);

			// 添加字符时，加入一条添加字符纪录
			LogElement log = { LogElement::UN_ADD_CHAR, yCaretPix, xCaretPix, NULL };
			AddLog(log);

			// 插入
			article.InsertChar(line, index, (TCHAR)wParam);
			//         redoArticle.Initialize(article.Format());   // redo纪录

			//////////////////////////////////////////////////////////////////////////
			//      插入符右移一位
			int sLen = article.GetCol(yCaretPix / m_CyChar);
			int pixelLen = sLen * m_CxChar;

			if (xCaretPix >= pixelLen)     // 向右移动到行尾时继续移动则到下行行首
			{
				int line = yCaretPix / 16;
				if (line < article.GetRows() - 1)
				{
					//           TCHAR *ppBegin = GetTextPtr(m_pBuffer, collum + 1);
					//           int ssLen = GetTextPtrEnd(m_pBuffer, collum + 1) - ppBegin;
					int ssLen = article.GetCol(line + 1);
					int ppixelLen = ssLen * m_CxChar;
					xCaretPix = 0;
					yCaretPix = yCaretPix + m_CyChar;
				}
				else
				{
					xCaretPix = pixelLen;
					yCaretPix = yCaretPix;
				}
			}
			else
			{
				int line = yCaretPix / m_CyChar;
				int col = xCaretPix / m_CxChar;
				int realCxChar = GetRealCxChar(line, col);
				xCaretPix = min(xCaretPix + realCxChar, pixelLen);
			}
			xCaretPixelSave = xCaretPix;
			m_Caret.SetPos(xCaretPix, yCaretPix);
			if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
			{
				ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
			}
			if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
			{
				ScrollMove(SB_VERT, (yCaretPix / m_CyChar - m_CyClient / m_CyChar + 1));
			}
			//////////////////////////////////////////////////////////////////////////
			//         ::SendMessage(m_hWnd, WM_KEYDOWN, VK_RIGHT, NULL); // 插入符向右移动
			//         xCaretPix = article.GetCol(line, index+1);    // 插入符右移
			//         m_Caret.SetPos(xCaretPix, yCaretPix);
			// 新增行后，若超出客户区范围，则刷新滚动条
			m_Caret.GetPos(xCaretPix, yCaretPix);
			if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
			{
				//           article.GetColMax() = article.GetColMax();
				ScrollResize(SB_HORZ, article.GetColMax());
			}
			//         if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
			//         {
			//           article.GetRows() = article.GetRows();
			//           ScrollResize(SB_VERT, article.GetRows());
			//         }

			//         article.GetRows() = article.GetRows();
			//         article.GetColMax() = article.GetColMax();
			//         ScrollResize(SB_VERT, article.GetRows());
			//         ScrollResize(SB_HORZ, article.GetColMax());

			//////////////////////////////////////////////////////////////////////////
			m_Caret.GetPos(xCaretPix, yCaretPix);
			if (xCaretPix <= 0)     // 向左移动到行首时继续移动则到上行行尾
			{
				int line = yCaretPix / 16;
				if (line > 0)
				{
					//           TCHAR *pBegin = GetTextPtr(m_pBuffer, collum - 1);     // 竖直方向
					//           int sLen = GetTextPtrEnd(m_pBuffer, collum - 1) - pBegin;
					int sLen = article.GetCol(line - 1);
					int pixelLen = sLen * m_CxChar;
					xCaretPix = pixelLen;
					yCaretPix = yCaretPix - m_CyChar;

					if (xCaretPix >= m_Caret.xOffset && xCaretPix <= (m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT)   // 插入符在客户区时
						&& yCaretPix >= m_Caret.yOffset && yCaretPix <= (m_Caret.yOffset + m_CyClient - m_CyChar))
					{
						//////             xCaret = xPage;
						//////             xCaretPix = Caret.xOffset + xCaret * m_CxChar;
						////TCHAR *pBegin = GetTextPtr(m_pBuffer, yCaretPix / 16);
						////int sLen = GetTextPtrEnd(m_pBuffer, yCaretPix / 16) - pBegin;
						//int line = yCaretPix/m_CyChar;
						//int sLen = article.GetCount(line);
						//xCaretPix = sLen * m_CxChar;
						//xCaretPixelSave = xCaretPix;
						//m_Caret.SetPos(xCaretPix, yCaretPix);

						//ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
					}
					else                                                    // 不在客户区时
					{
						ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
					}
				}
				else                  // 已处于第一行
				{
					xCaretPix = 0;
					yCaretPix = yCaretPix;
				}
			}
			else
			{
				int line = yCaretPix / m_CyChar;
				int col = xCaretPix / m_CxChar;

				int dataIndex = article.GetDataIndex(line, col);
				if (article.GetDataType(article.GetDataByIndex(line, dataIndex - 1)))
				{
					xCaretPix = max(xCaretPix - m_CxChar * 2, 0);
				}
				else
				{
					xCaretPix = max(xCaretPix - m_CxChar, 0);
				}

				//           xCaretPix = max(xCaretPix - m_CxChar, 0);
				//           xCaretPix = max(xCaretPix - realCxChar, 0);
				//           ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
			}
			xCaretPixelSave = xCaretPix;
			m_Caret.SetPos(xCaretPix, yCaretPix);

			if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
			{
				ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - 5));
			}
			if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
			{
				ScrollMove(SB_VERT, (yCaretPix / m_CyChar));
			}
			//         ::SendMessage(m_hWnd, WM_KEYDOWN, VK_LEFT, 1);
			//////////////////////////////////////////////////////////////////////////
			m_Caret.GetPos(xCaretPix, yCaretPix);
			//      TCHAR *pBegin = GetTextPtr(m_pBuffer, yCaretPix / 16);
			//      int sLen = GetTextPtrEnd(m_pBuffer, yCaretPix / 16) - pBegin;
			sLen = article.GetCol(yCaretPix / m_CyChar);
			pixelLen = sLen * m_CxChar;

			if (xCaretPix >= pixelLen)     // 向右移动到行尾时继续移动则到下行行首
			{
				int line = yCaretPix / 16;
				if (line < article.GetRows() - 1)
				{
					//           TCHAR *ppBegin = GetTextPtr(m_pBuffer, collum + 1);
					//           int ssLen = GetTextPtrEnd(m_pBuffer, collum + 1) - ppBegin;
					int ssLen = article.GetCol(line + 1);
					int ppixelLen = ssLen * m_CxChar;
					xCaretPix = 0;
					yCaretPix = yCaretPix + m_CyChar;
				}
				else
				{
					xCaretPix = pixelLen;
					yCaretPix = yCaretPix;
				}
			}
			else
			{
				int line = yCaretPix / m_CyChar;
				int col = xCaretPix / m_CxChar;
				int realCxChar = GetRealCxChar(line, col);
				xCaretPix = min(xCaretPix + realCxChar, pixelLen);
			}
			xCaretPixelSave = xCaretPix;
			m_Caret.SetPos(xCaretPix, yCaretPix);

			// 添加字符时，加入一条添加字符纪录
			//         line = yCaretPix / m_CyChar;
			//         col  = xCaretPix / m_CxChar;
			//         index = article.GetDataIndex(line, col);
			LogElement log2 = { LogElement::ADD_CHAR, yCaretPix, xCaretPix, NULL };
			AddLog(log2);

			//         if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
			if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - 0 - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
			{
				ScrollMove(SB_HORZ, (xCaretPix / m_CxChar - m_CxClient / m_CxChar + 1));
			}
			if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
			{
				ScrollMove(SB_VERT, (yCaretPix / m_CyChar - m_CyClient / m_CyChar + 1));
			}
			//         ::SendMessage(m_hWnd, WM_KEYDOWN, VK_RIGHT, 1);
			//////////////////////////////////////////////////////////////////////////

			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);   // 窗口重绘
			UpdateWindow();
			break;
		}
		}
	}
}

void XEdit::OnFind(WPARAM wParam, LPARAM lParam)
{
	WPARAM findParam = wParam;
	TCHAR *srcData = (TCHAR *)lParam;
	int    srcLen = lstrlen(srcData);

	m_Caret.GetPos(xCaretPix, yCaretPix);
	int lineStart = yCaretPix / m_CyChar;
	int colStart = article.GetDataIndex(lineStart, xCaretPix / m_CxChar);

	if (findParam & FP_BEGIN_OF_FILE)     // 如果从文件头开始
	{
		lineStart = 0;
		colStart = 0;
		m_Caret.SetPos(0, 0);
		m_Caret.GetPos(xCaretPix, yCaretPix);
		// 若超出客户区范围，则刷新滚动条
		m_Caret.GetPos(xCaretPix, yCaretPix);
		if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
		{
			ScrollMove(SB_HORZ, SCROLL_MIN);
		}
		if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
		{
			::SendMessage(m_hWnd, WM_KEYDOWN, VK_DOWN, NULL); // 插入换行后插入符向下移动
			::SendMessage(m_hWnd, WM_KEYDOWN, VK_UP, NULL);   // 插入换行后插入符向上移动(这两步只是为了滚动页面)
		}
	}

	BOOL  isFind = FALSE;
	int   index = 0;
	int   rstLine = 0, rstIndex = 0;

	if (findParam & FP_REPLACE_ALL)   // 全部替换
	{
		if (IDOK == MessageBox(m_hWnd, TEXT("是否全部替换！"), TEXT("提示"), MB_OKCANCEL))
		{
			printf("OK");
		}
		else
		{
			printf("Calcel");         // 取消全部替换
			return;
		}
		TCHAR *rpData = srcData + 100;
		int    rpLen = lstrlen(rpData);
		//     article.InsertString(rstLine, rstIndex, rpData, rpLen);

		m_selector.Reset();
		for (int i = 0; i<article.GetRows(); i++)
		{
			if (findParam & FP_CASE_SENSITIVE)    // 如果区分大小写
			{
				if (findParam & FP_WHOLE_WORD)
				{
					while (article.SearchText(i, 0, srcData, &index, TRUE, TRUE))
					{
						article.DeleteString(i, index, srcLen);
						article.InsertString(i, index, rpData, rpLen);
						isFind = TRUE;
					}
				}
				else
				{
					while (article.SearchText(i, 0, srcData, &index, TRUE, FALSE))
					{
						article.DeleteString(i, index, srcLen);
						article.InsertString(i, index, rpData, rpLen);
						isFind = TRUE;
					}
				}
			}
			else                                  // 不区分大小写
			{
				if (findParam & FP_WHOLE_WORD)
				{
					while (article.SearchText(i, 0, srcData, &index, FALSE, TRUE))
					{
						article.DeleteString(i, index, srcLen);
						article.InsertString(i, index, rpData, rpLen);
						isFind = TRUE;
					}
				}
				else
				{
					while (article.SearchText(i, 0, srcData, &index, FALSE, FALSE))
					{
						article.DeleteString(i, index, srcLen);
						article.InsertString(i, index, rpData, rpLen);
						isFind = TRUE;
					}
				}
			}
		}

		RECT rect;
		GetClientRect(m_hWnd, &rect);
		InvalidateRect(m_hWnd, &rect, FALSE);   // 窗口重绘
		UpdateWindow();
		if (isFind == TRUE)
		{
			MessageBox(m_hWnd, TEXT("全部替换完成"), TEXT("提示"), NULL);
		}
		else
		{
			MessageBox(m_hWnd, TEXT("未找到目标"), TEXT("提示"), NULL);
		}
		return;
	}
	if (findParam & FP_FIND_NEXT)     // 查找下一个
	{
		for (int i = lineStart; i<article.GetRows(); i++)
		{
			if (i == lineStart)     // 第一行从插入符所在位置开始
			{
				if (findParam & FP_CASE_SENSITIVE)    // 如果区分大小写
				{
					if (findParam & FP_WHOLE_WORD)
					{
						if (article.SearchText(i, colStart, srcData, &index, TRUE, TRUE))
							isFind = TRUE;
					}
					else
					{
						if (article.SearchText(i, colStart, srcData, &index, TRUE, FALSE))
							isFind = TRUE;
					}
				}
				else                                  // 不区分大小写
				{
					if (findParam & FP_WHOLE_WORD)
					{
						if (article.SearchText(i, colStart, srcData, &index, FALSE, TRUE))
							isFind = TRUE;
					}
					else
					{
						if (article.SearchText(i, colStart, srcData, &index, FALSE, FALSE))
							isFind = TRUE;
					}
				}
			}
			else
			{
				if (findParam & FP_CASE_SENSITIVE)        // 如果区分大小写
				{
					if (findParam & FP_WHOLE_WORD)
					{
						if (article.SearchText(i, 0, srcData, &index, TRUE, TRUE))
							isFind = TRUE;
					}
					else
					{
						if (article.SearchText(i, 0, srcData, &index, TRUE, FALSE))
							isFind = TRUE;
					}
				}
				else                                      // 不区分大小写
				{
					if (findParam & FP_WHOLE_WORD)
					{
						if (article.SearchText(i, 0, srcData, &index, FALSE, TRUE))
							isFind = TRUE;
					}
					else
					{
						if (article.SearchText(i, 0, srcData, &index, FALSE, FALSE))
							isFind = TRUE;
					}
				}
			}

			if (isFind == TRUE)
			{
				rstLine = i;
				rstIndex = index;
				break;
			}
		}
		if (isFind)
		{
			TCHAR strTemp[100];
			wsprintf(strTemp, TEXT("line  = %d\nindex = %d"), rstLine, rstIndex);
			//     MessageBox(m_hWnd, strTemp, TEXT("XEdit"), NULL);

			short xPos = 0, yPos = 0;
			xPos = article.GetCol(rstLine, rstIndex) * m_CxChar - m_Caret.xOffset + EDIT_MARGIN_LEFT;
			yPos = rstLine * m_CyChar - m_Caret.yOffset;
			WPARAM xyPos = (yPos << 16) | xPos;
			::SendMessage(m_hWnd, WM_LBUTTONDOWN, NULL, xyPos);
			xPos = article.GetCol(rstLine, rstIndex + srcLen) * m_CxChar - m_Caret.xOffset + EDIT_MARGIN_LEFT;
			xyPos = (yPos << 16) | xPos;
			VIRTUAL_MK_LBUTTON = TRUE;
			::SendMessage(m_hWnd, WM_MOUSEMOVE, NULL, xyPos);
			VIRTUAL_MK_LBUTTON = FALSE;
			::SendMessage(m_hWnd, WM_LBUTTONUP, NULL, xyPos);

			// 新增行后，若超出客户区范围，则刷新滚动条
			m_Caret.GetPos(xCaretPix, yCaretPix);
			if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
			{
				//           article.GetColMax() = article.GetColMax();
				//         ScrollResize(SB_HORZ, article.GetColMax());
				ScrollMove(SB_HORZ, SCROLL_MIN);
			}
			if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
			{
				//           article.GetRows() = article.GetRows();
				//         ScrollResize(SB_VERT, article.GetRows());
				::SendMessage(m_hWnd, WM_KEYDOWN, VK_UP, NULL);   // 插入换行后插入符向上移动(这两步只是为了滚动页面)
				::SendMessage(m_hWnd, WM_KEYDOWN, VK_DOWN, NULL); // 插入换行后插入符向下移动
			}

			xPos = article.GetCol(rstLine, rstIndex) * m_CxChar - m_Caret.xOffset + EDIT_MARGIN_LEFT;
			yPos = rstLine * m_CyChar - m_Caret.yOffset;
			xyPos = (yPos << 16) | xPos;
			::SendMessage(m_hWnd, WM_LBUTTONDOWN, NULL, xyPos);
			xPos = article.GetCol(rstLine, rstIndex + srcLen) * m_CxChar - m_Caret.xOffset + EDIT_MARGIN_LEFT;
			xyPos = (yPos << 16) | xPos;
			VIRTUAL_MK_LBUTTON = TRUE;
			::SendMessage(m_hWnd, WM_MOUSEMOVE, NULL, xyPos);
			VIRTUAL_MK_LBUTTON = FALSE;
			::SendMessage(m_hWnd, WM_LBUTTONUP, NULL, xyPos);

			if (findParam & FP_REPLACE)               // 如果需要替换
			{
				TCHAR *rpData = srcData + 100;
				int    rpLen = lstrlen(rpData);
				::SendMessage(m_hWnd, WM_KEYDOWN, VK_BACK, 1);
				article.InsertString(rstLine, rstIndex, rpData, rpLen);

				xPos = article.GetCol(rstLine, rstIndex) * m_CxChar - m_Caret.xOffset + EDIT_MARGIN_LEFT;
				yPos = rstLine * m_CyChar - m_Caret.yOffset;
				xyPos = (yPos << 16) | xPos;
				::SendMessage(m_hWnd, WM_LBUTTONDOWN, NULL, xyPos);
				xPos = article.GetCol(rstLine, rstIndex + rpLen) * m_CxChar - m_Caret.xOffset + EDIT_MARGIN_LEFT;
				xyPos = (yPos << 16) | xPos;
				VIRTUAL_MK_LBUTTON = TRUE;
				::SendMessage(m_hWnd, WM_MOUSEMOVE, NULL, xyPos);
				VIRTUAL_MK_LBUTTON = FALSE;
				::SendMessage(m_hWnd, WM_LBUTTONUP, NULL, xyPos);
			}

			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);   // 窗口重绘
			UpdateWindow();
		}
		else
		{
			MessageBox(m_hWnd, TEXT("Not Found!"), TEXT("XEdit"), NULL);
		}
		return;
	}
	if (findParam & FP_FIND_PREV)     // 查找上一个
	{
		for (int i = lineStart; i >= 0; i--)
		{
			if (i == lineStart)     // 第一行从插入符所在位置开始
			{
				if (findParam & FP_CASE_SENSITIVE)    // 如果区分大小写
				{
					if (findParam & FP_WHOLE_WORD)
					{
						if (article.SearchTextRev(i, colStart, srcData, &index, TRUE, TRUE))
							isFind = TRUE;
					}
					else
					{
						if (article.SearchTextRev(i, colStart, srcData, &index, TRUE, FALSE))
							isFind = TRUE;
					}
				}
				else                                  // 不区分大小写
				{
					if (findParam & FP_WHOLE_WORD)
					{
						if (article.SearchTextRev(i, colStart, srcData, &index, FALSE, TRUE))
							isFind = TRUE;
					}
					else
					{
						if (article.SearchTextRev(i, colStart, srcData, &index, FALSE, FALSE))
							isFind = TRUE;
					}
				}
			}
			else
			{
				if (findParam & FP_CASE_SENSITIVE)        // 如果区分大小写
				{
					if (findParam & FP_WHOLE_WORD)
					{
						if (article.SearchTextRev(i, article.GetCount(i), srcData, &index, TRUE, TRUE))
							isFind = TRUE;
					}
					else
					{
						if (article.SearchTextRev(i, article.GetCount(i), srcData, &index, TRUE, FALSE))
							isFind = TRUE;
					}
				}
				else                                      // 不区分大小写
				{
					if (findParam & FP_WHOLE_WORD)
					{
						if (article.SearchTextRev(i, article.GetCount(i), srcData, &index, FALSE, TRUE))
							isFind = TRUE;
					}
					else
					{
						if (article.SearchTextRev(i, article.GetCount(i), srcData, &index, FALSE, FALSE))
							isFind = TRUE;
					}
				}
			}

			if (isFind == TRUE)
			{
				rstLine = i;
				rstIndex = index;
				break;
			}
		}
		if (isFind)
		{
			TCHAR strTemp[100];
			wsprintf(strTemp, TEXT("line  = %d\nindex = %d"), rstLine, rstIndex);
			wcout << strTemp << endl;
			//     MessageBox(m_hWnd, strTemp, TEXT("XEdit"), NULL);

			short xPos = 0, yPos = 0;
			xPos = article.GetCol(rstLine, rstIndex);// * m_CxChar;// - m_Caret.xOffset;
			yPos = rstLine;// * m_CyChar;// - m_Caret.yOffset;
			//       LPARAM xyPos = (yPos << 16) | xPos;

			int line = yPos;
			int col = xPos;
			int index = article.GetDataIndex(line, col);
			xCaretPix = article.GetCol(line, index) * m_CxChar;   // 新坐标
			//   xCaretPix = m_Caret.xOffset + x_pos * m_CxChar;
			yCaretPix = yPos * m_CyChar;
			//   yCaretPix = m_Caret.yOffset + y_pos * m_CyChar;
			if (yCaretPix / m_CyChar > (article.GetRows() - 1))
			{
				yCaretPix = (article.GetRows() - 1) * m_CyChar;
			}
			if (xCaretPix / m_CxChar > article.GetCol(yCaretPix / m_CyChar))
			{
				xCaretPix = article.GetCol(yCaretPix / m_CyChar) * m_CxChar;
			}
			m_Caret.SetPos(xCaretPix, yCaretPix);         // 设定坐标

			//       ::SendMessage(m_hWnd, WM_LBUTTONDOWN, NULL, xyPos);
			//       xPos = article.GetCol(rstLine, rstIndex+srcLen) * m_CxChar - m_Caret.xOffset;
			//       xyPos = (yPos << 16) | xPos;
			//       ::SendMessage(m_hWnd, WM_LBUTTONUP, NULL, xyPos);

			// 新增行后，若超出客户区范围，则刷新滚动条
			m_Caret.GetPos(xCaretPix, yCaretPix);
			if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
			{
				//           article.GetColMax() = article.GetColMax();
				//         ScrollResize(SB_HORZ, article.GetColMax());
				ScrollMove(SB_HORZ, SCROLL_MIN);
			}
			if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
			{
				//           article.GetRows() = article.GetRows();
				//         ScrollResize(SB_VERT, article.GetRows());
				//         ::SendMessage(m_hWnd, WM_KEYDOWN, VK_UP, NULL);   // 插入换行后插入符向上移动(这两步只是为了滚动页面)
				//         ::SendMessage(m_hWnd, WM_KEYDOWN, VK_DOWN, NULL); // 插入换行后插入符向下移动
				::SendMessage(m_hWnd, WM_KEYDOWN, VK_DOWN, NULL); // 插入换行后插入符向下移动
				::SendMessage(m_hWnd, WM_KEYDOWN, VK_UP, NULL);   // 插入换行后插入符向上移动(这两步只是为了滚动页面)
			}

			xPos = article.GetCol(rstLine, rstIndex + srcLen) * m_CxChar - m_Caret.xOffset + EDIT_MARGIN_LEFT;
			yPos = rstLine * m_CyChar - m_Caret.yOffset;
			WPARAM xyPos = (yPos << 16) | xPos;
			::SendMessage(m_hWnd, WM_LBUTTONDOWN, NULL, xyPos);
			xPos = article.GetCol(rstLine, rstIndex) * m_CxChar - m_Caret.xOffset + EDIT_MARGIN_LEFT;
			xyPos = (yPos << 16) | xPos;
			VIRTUAL_MK_LBUTTON = TRUE;
			::SendMessage(m_hWnd, WM_MOUSEMOVE, NULL, xyPos);
			VIRTUAL_MK_LBUTTON = FALSE;
			::SendMessage(m_hWnd, WM_LBUTTONUP, NULL, xyPos);

			if (findParam & FP_REPLACE)               // 如果需要替换
			{
				TCHAR *rpData = srcData + 100;
				int    rpLen = lstrlen(rpData);
				::SendMessage(m_hWnd, WM_KEYDOWN, VK_BACK, 1);
				article.InsertString(rstLine, rstIndex, rpData, rpLen);

				xPos = article.GetCol(rstLine, rstIndex) * m_CxChar - m_Caret.xOffset + EDIT_MARGIN_LEFT;
				yPos = rstLine * m_CyChar - m_Caret.yOffset;
				xyPos = (yPos << 16) | xPos;
				::SendMessage(m_hWnd, WM_LBUTTONDOWN, NULL, xyPos);
				xPos = article.GetCol(rstLine, rstIndex + rpLen) * m_CxChar - m_Caret.xOffset + EDIT_MARGIN_LEFT;
				xyPos = (yPos << 16) | xPos;
				VIRTUAL_MK_LBUTTON = TRUE;
				::SendMessage(m_hWnd, WM_MOUSEMOVE, NULL, xyPos);
				VIRTUAL_MK_LBUTTON = FALSE;
				::SendMessage(m_hWnd, WM_LBUTTONUP, NULL, xyPos);
			}

			RECT rect;
			GetClientRect(m_hWnd, &rect);
			InvalidateRect(m_hWnd, &rect, FALSE);   // 窗口重绘
			UpdateWindow();
		}
		else
		{
			MessageBox(m_hWnd, TEXT("Not Found!"), TEXT("XEdit"), NULL);
		}
		return;
	}
}

LRESULT XEdit::OnSetText(WPARAM wParam, LPARAM lParam)
{
	RECT rect;

	m_Caret.xOffset = 0;
	m_Caret.yOffset = 0;
	xCaretPixelSave = 0;                      // 初始化插入符位置
	xCaretPix = 0;
	yCaretPix = 0;
	m_Caret.SetPos(xCaretPix, yCaretPix);

	m_selector.Reset();                       // 初始化块选择结构
	m_selectorStart.Reset();
	//////////////////////////////////////////////////////////////////////////
	//    文本处理
	OnDestroy();
	article.Destroy();
	article.Initialize((TCHAR *)lParam);
	//   redoArticle.Destroy();
	//   redoArticle.Initialize((TCHAR *)lParam);
	//   undoArticle.Destroy();
	//   undoArticle.Initialize((TCHAR *)lParam);

	//   m_selector.Point0.x = 0;
	//   m_selector.Point0.y = 0;
	//   m_selector.Point1.x = 0;
	//   m_selector.Point1.y = 0;
	//   m_selector.selected = FALSE;
	//   m_selector.start    = TRUE;

	//   int index = 0;
	//   if (article.SearchTest(TEXT("Hello"), 16, &index))
	//   {
	//     TCHAR strTemp[100];
	//     wsprintf(strTemp, TEXT("Find Text at: %d"), index);
	//     MessageBox(m_hWnd, strTemp, TEXT("Find"), NULL);
	//   }
	/*  article.Print();
	printf("GetColMax = %d\n", article.GetColMax());
	printf("Size = %d\n", article.GetSize());
	printf("rows = %d\n", article.GetRows());*/
	/*  article.InsertChar(0, 0, TEXT('H'));
	article.InsertChar(0, 1, TEXT('e'));
	article.InsertChar(0, 2, TEXT('l'));
	article.InsertChar(0, 3, TEXT('l'));
	article.InsertChar(0, 4, TEXT('o'));
	article.InsertChar(0, 5, TEXT(' '));*/
	/*  for(int i=0; i<article.rows; i++)
	{
	article.InsertCharBegin(i, TEXT(' '));    // 行首插入
	article.InsertCharBegin(i, TEXT('n'));
	article.InsertCharBegin(i, TEXT('i'));
	article.InsertCharBegin(i, TEXT('g'));
	article.InsertCharBegin(i, TEXT('e'));
	article.InsertCharBegin(i, TEXT('B'));
	article.InsertCharEnd(i, TEXT(' '));      // 行尾插入
	article.InsertCharEnd(i, TEXT('E'));
	article.InsertCharEnd(i, TEXT('n'));
	article.InsertCharEnd(i, TEXT('d'));
	}*/
	//   article.InsertString(4, 2, TEXT(" [InsertString] "), 16);
	/*  for(int i=0; i<article.rows; i++)
	{
	article.InsertStringBegin(i, TEXT("InStrBegin "), 11);  // 行首插入
	article.InsertStringEnd(i, TEXT(" InStrEnd"), 9);       // 行尾插入
	}*/
	/*  article.DeleteCharBegin(0);
	article.DeleteCharBegin(0);
	article.DeleteCharEnd(0);
	article.DeleteCharEnd(0);*/
	/*  article.InsertRowBegin(TEXT("----- Begin0 -----"));    // 页首插入
	article.InsertRowBegin(TEXT("----- Begin1 -----"));
	article.InsertRowBegin(TEXT("----- Begin2 -----"));
	article.InsertRowEnd(TEXT("----- End0 -----"));        // 页尾插入
	article.InsertRowEnd(TEXT("----- End1 -----"));
	article.InsertRowEnd(TEXT("----- End2 -----"));*/
	//   article.DeleteMultiString(13, 8, 14, 15);
	//   article.SeparateRow(1, 10);
	//   article.InsertMultiString(14 ,16, TEXT("Hello, world\r\nOh, Yes\r\nHahaha\r\n"), 31);
	/*  article.Print();
	printf("GetColMax = %d\n", article.GetColMax());
	printf("Size = %d\n", article.GetSize());
	printf("rows = %d\n", article.GetRows());*/
	/*  article.CombineRow(2);
	article.CombineRow(article.rows-4);*/
	//   article.Format();
	//////////////////////////////////////////////////////////////////////////

	//  //int sLen = lstrlen((LPCTSTR)lParam);
	//  int sLen = lstrlen(article.pBufSave);
	////       TCHAR *pBuf = (TCHAR *)lParam;
	//  if (m_pBuffer != NULL)
	//  {
	//    free(m_pBuffer);
	//    m_pBuffer = NULL;
	//  }
	//  m_pBuffer = (TCHAR *)malloc((sLen + 2) * sizeof(TCHAR));
	//  lstrcpy((PTSTR)m_pBuffer, (PTSTR)article.pBufSave);
	//  m_pBuffer[sLen] = TEXT('\0');
	//  m_pBuffer[sLen + 1] = TEXT('\0');
	//  // 计算行数
	////   article.GetRows() = CalcTextRows(m_pBuffer);
	//  article.GetRows() = article.GetRows();
	////   linePos = 0;
	//   article.GetRows() = article.GetRows();
	//   article.GetColMax() = article.GetColMax();
	ScrollResize(SB_VERT, article.GetRows());
	ScrollResize(SB_HORZ, article.GetColMax());
	//   m_Scroll.si.cbSize = sizeof(m_Scroll.si);
	//   m_Scroll.si.fMask = SIF_RANGE;
	//   m_Scroll.si.nMin = 0;
	//   m_Scroll.si.nMax = (article.GetRows() > 0) ? (article.GetRows() - 1) : 0;
	//   SetScrollInfo(m_hWnd, SB_VERT, &m_Scroll.si, TRUE);
	//   SetScrollPos(m_hWnd, SB_VERT, 0, TRUE);             // 竖直滚动条复位
	// 计算列数（准确讲应该是字符串宽度）
	//   article.GetColMax() = CalcTextCols(m_pBuffer);
	//   colsPos = 0;
	//   m_Scroll.si.cbSize = sizeof(m_Scroll.si);
	//   m_Scroll.si.fMask = SIF_RANGE;
	//   m_Scroll.si.nMin = 0;
	//   m_Scroll.si.nMax = (article.GetColMax() > 0) ? (article.GetColMax() - 1) : 0;
	//   SetScrollInfo(m_hWnd, SB_HORZ, &m_Scroll.si, TRUE);
	//   SetScrollPos(m_hWnd, SB_HORZ, 0, TRUE);             // 水平滚动条复位

	//   m_Caret.xOffset = 0;
	//   m_Caret.yOffset = 0;

	//   readComplete = TRUE;

	GetClientRect(m_hWnd, &rect);
	InvalidateRect(m_hWnd, &rect, TRUE);   // 使窗口客户区无效，完全重绘
	UpdateWindow();

	return 0;
}

void XEdit::OnVScroll(UINT nSBCode)
{
	RECT rect;
	int  iVertPos;

	if (article.GetSize() == NULL)
		return;

	// 获取垂直滚动条所有信息
	m_Scroll.si.cbSize = sizeof(m_Scroll.si);
	m_Scroll.si.fMask = SIF_ALL;
	GetScrollInfo(m_hWnd, SB_VERT, &m_Scroll.si);
	// 保存当前位置以便后面作比较
	iVertPos = m_Scroll.si.nPos;
	switch (nSBCode)
	{
	case SB_LINEUP:
		m_Scroll.si.nPos -= 1;
		break;
	case SB_LINEDOWN:
		m_Scroll.si.nPos += 1;
		break;
	case SB_PAGEUP:
		m_Scroll.si.nPos -= m_Scroll.si.nPage;
		break;
	case SB_PAGEDOWN:
		m_Scroll.si.nPos += m_Scroll.si.nPage;
		break;
	case SB_THUMBTRACK:
		m_Scroll.si.nPos = m_Scroll.si.nTrackPos;
		break;
	default:
		break;
	}
	m_Scroll.si.fMask = SIF_POS;
	SetScrollInfo(m_hWnd, SB_VERT, &m_Scroll.si, TRUE);
	GetScrollInfo(m_hWnd, SB_VERT, &m_Scroll.si);
	// 如果位置变化了，则移动窗口并重绘
	if (m_Scroll.si.nPos != iVertPos)
	{
		m_Caret.yOffset = (m_Scroll.si.nPos * m_CyChar);

		//         cout << "Caret.xOffset: " << Caret.xOffset << '\t' << "Caret.yOffset: " << Caret.yOffset << endl;
		m_Caret.SetPos(xCaretPix, yCaretPix);
		GetClientRect(m_hWnd, &rect);
		InvalidateRect(m_hWnd, &rect, FALSE);
		UpdateWindow();
	}
}

void XEdit::OnHScroll(UINT nSBCode)
{
	RECT rect;
	int  iHorzPos;

	if (article.GetSize() == 0)
		return;

	m_Scroll.si.cbSize = sizeof(m_Scroll.si);
	m_Scroll.si.fMask = SIF_ALL;
	GetScrollInfo(m_hWnd, SB_HORZ, &m_Scroll.si);

	iHorzPos = m_Scroll.si.nPos;
	switch (nSBCode)
	{
	case SB_LINELEFT:
		m_Scroll.si.nPos -= 1;
		break;
	case SB_LINERIGHT:
		m_Scroll.si.nPos += 1;
		break;
	case SB_PAGELEFT:
		m_Scroll.si.nPos -= m_Scroll.si.nPage;
		break;
	case SB_PAGERIGHT:
		m_Scroll.si.nPos += m_Scroll.si.nPage;
		break;
	case SB_THUMBTRACK:
		m_Scroll.si.nPos = m_Scroll.si.nTrackPos;
		break;
	default:
		break;
	}
	m_Scroll.si.fMask = SIF_POS;
	SetScrollInfo(m_hWnd, SB_HORZ, &m_Scroll.si, TRUE);
	GetScrollInfo(m_hWnd, SB_HORZ, &m_Scroll.si);

	if (m_Scroll.si.nPos != iHorzPos)
	{
		m_Caret.xOffset = (m_Scroll.si.nPos * m_CxChar);

		m_Caret.SetPos(xCaretPix, yCaretPix);
		GetClientRect(m_hWnd, &rect);
		InvalidateRect(m_hWnd, &rect, FALSE);
		UpdateWindow();
	}
}

void XEdit::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	if (m_Scroll.iDeltaPerLine == 0)
		return;

	m_Scroll.iAccumDelta += (short)HIWORD(wParam);         // 120 or -120
	while (m_Scroll.iAccumDelta >= m_Scroll.iDeltaPerLine)
	{
		if (wParam & MK_SHIFT)                    // 如果按下Shift键则水平滚动
			SendMessage(m_hWnd, WM_HSCROLL, SB_LINELEFT, 0);
		else
			SendMessage(m_hWnd, WM_VSCROLL, SB_LINEUP, 0);
		m_Scroll.iAccumDelta -= m_Scroll.iDeltaPerLine;
	}
	while (m_Scroll.iAccumDelta <= -m_Scroll.iDeltaPerLine)
	{
		if (wParam & MK_SHIFT)                    // 如果按下Shift键则水平滚动
			SendMessage(m_hWnd, WM_HSCROLL, SB_LINERIGHT, 0);
		else
			SendMessage(m_hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
		m_Scroll.iAccumDelta += m_Scroll.iDeltaPerLine;
	}
}

void XEdit::OnPaint()
{
	HDC             hdc;
	PAINTSTRUCT     ps;
	RECT            rect;
	SELECTOR        selector_temp;

	hdc = BeginPaint(m_hWnd, &ps);

	//   if (readComplete)
	//   {
	GetClientRect(m_hWnd, &rect);

	// 双缓冲
	hMemDC = CreateCompatibleDC(hdc);
	RECT rectMem;
	rectMem.left = 0;
	rectMem.right = (article.GetColMax() + 2 + EDIT_MARGIN_LEFT / 8) * m_CxChar;
	if (rectMem.right <= m_Caret.xOffset + rect.right)
	{
		ScrollMove(SB_HORZ, 0);
	}
	rectMem.right = (rectMem.right > rect.right) ? rectMem.right : (rect.right);
	rectMem.top = 0;
	rectMem.bottom = rect.bottom;
	hMemBmp = CreateCompatibleBitmap(hdc, rectMem.right, rectMem.bottom);

	hPreBmp = (HBITMAP)SelectObject(hMemDC, hMemBmp);
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
	FillRect(hMemDC, &rectMem, hBrush);
	DeleteObject(hBrush);
	// 绘图
	SelectObject(hMemDC, CreateFont(0, 0, 0, 0, 0, 0, 0, 0,
		m_CharSet, 0, 0, 0, FIXED_PITCH, NULL));
	RECT rectRow;     // 文本行部分
	int  lineClient = m_CyClient / m_CyChar;
	int  lineRows = article.GetRows() - m_Caret.yOffset / m_CyChar;
	int  lineShow = (lineClient > lineRows) ? lineRows : lineClient;
	SetBkMode(hMemDC, OPAQUE);
	//     SetBkColor(hMemDC, RGB(0, 255, 255));
	// 获取块选择信息
	selector_temp.rows = m_selector.rows;
	selector_temp.start = m_selector.start;
	selector_temp.selected = m_selector.selected;
	selector_temp.Point0.x = m_selector.Point0.x;
	selector_temp.Point0.y = m_selector.Point0.y;
	selector_temp.Point1.x = m_selector.Point1.x;
	selector_temp.Point1.y = m_selector.Point1.y;
	if (selector_temp.Point0.y > selector_temp.Point1.y)  // 如果向上选择则交换坐标
	{
		int temp = selector_temp.Point0.y;
		selector_temp.Point0.y = selector_temp.Point1.y;
		selector_temp.Point1.y = temp;
		temp = selector_temp.Point0.x;
		selector_temp.Point0.x = selector_temp.Point1.x;
		selector_temp.Point1.x = temp;
	}
	else if (selector_temp.Point0.y == selector_temp.Point1.y
		&& selector_temp.Point0.x > selector_temp.Point1.x)
	{
		int temp = selector_temp.Point0.x;
		selector_temp.Point0.x = selector_temp.Point1.x;
		selector_temp.Point1.x = temp;
	}

	// 用于显示行号
	// 双缓冲
	hMemNumDC = CreateCompatibleDC(hdc);
	RECT rectMemNum;
	rectMemNum.left = 0;
	rectMemNum.right = EDIT_MARGIN_LEFT;
	rectMemNum.top = 0;
	rectMemNum.bottom = rect.bottom;
	hMemNumBmp = CreateCompatibleBitmap(hdc, rectMemNum.right, rectMemNum.bottom);

	hPreNumBmp = (HBITMAP)SelectObject(hMemNumDC, hMemNumBmp);
	HBRUSH hBrushNum = CreateSolidBrush(RGB(220, 220, 220));
	FillRect(hMemNumDC, &rectMemNum, hBrushNum);
	DeleteObject(hBrushNum);
	// 绘图
	SelectObject(hMemNumDC, CreateFont(0, 0, 0, 0, 0, 0, 0, 0,
		m_CharSet, 0, 0, 0, FIXED_PITCH, NULL));
	RECT rectRowNum;

	UINT TextFormat = DT_LEFT | DT_NOPREFIX | DT_NOCLIP | DT_EXPANDTABS;
	// 循环打印行
	for (int i = 0; i<lineShow; i++)
	{
		if (selector_temp.selected == FALSE && selector_temp.start)       // 如果没有文本块被选中
		{
			rectRow.left = 0;
			rectRow.right = (article.GetCol(i) + 2) * m_CxChar;
			rectRow.top = i * m_CyChar;
			rectRow.bottom = (i + 1) * m_CyChar;
			DrawText(hMemDC, article.GetContent(i + m_Caret.yOffset / m_CyChar),
				article.GetCount(i + m_Caret.yOffset / m_CyChar),
				&rectRow, TextFormat);
		}
		else// if (selector_temp.selected || !selector_temp.start)      // 有文本块被选中，则选中部分高亮
		{
			int x0 = selector_temp.Point0.x;
			int y0 = selector_temp.Point0.y;
			int x1 = selector_temp.Point1.x;
			int y1 = selector_temp.Point1.y;
			int rowsSel = selector_temp.rows;

			int x_offset = m_Caret.xOffset;
			int y_offset = m_Caret.yOffset;

			if ((i + m_Caret.yOffset / m_CyChar) >= y0 &&
				(i + m_Caret.yOffset / m_CyChar) <= y1)     // 绘制被选中行
			{
				if (rowsSel == 1)     // 如果只有一行
				{
					rectRow.left = 0;
					rectRow.right = article.GetCol(y0, x0) * m_CxChar;
					rectRow.top = i * m_CyChar;
					rectRow.bottom = (i + 1) * m_CyChar;
					SetBkColor(hMemDC, RGB(255, 255, 255));
					DrawText(hMemDC, article.GetContent(i + y_offset / m_CyChar),
						x0,
						&rectRow, TextFormat);
					rectRow.left = article.GetCol(y0, x0) * m_CxChar;
					rectRow.right = article.GetCol(y0, x1) * m_CxChar;
					rectRow.top = i * m_CyChar;
					rectRow.bottom = (i + 1) * m_CyChar;
					SetBkColor(hMemDC, RGB(0, 255, 255));
					DrawText(hMemDC, article.GetContent(i + y_offset / m_CyChar) + x0,
						x1 - x0,
						&rectRow, TextFormat);
					rectRow.left = article.GetCol(y0, x1) * m_CxChar;
					rectRow.right = (article.GetCol(y0) + 2) * m_CxChar;
					rectRow.top = i * m_CyChar;
					rectRow.bottom = (i + 1) * m_CyChar;
					SetBkColor(hMemDC, RGB(255, 255, 255));
					DrawText(hMemDC, article.GetContent(i + y_offset / m_CyChar) + x1,
						article.GetCount(i + y_offset / m_CyChar) - x1,
						&rectRow, TextFormat);
				}
				else                            // 如果有多行同时选中
				{
					if ((i + y_offset / m_CyChar) == y0)    // 第一行
					{
						rectRow.left = 0;
						rectRow.right = article.GetCol(y0, x0) * m_CxChar;
						rectRow.top = i * m_CyChar;
						rectRow.bottom = (i + 1) * m_CyChar;
						SetBkColor(hMemDC, RGB(255, 255, 255));
						DrawText(hMemDC, article.GetContent(i + y_offset / m_CyChar),
							x0,
							&rectRow, TextFormat);
						rectRow.left = article.GetCol(y0, x0) * m_CxChar;
						rectRow.right = (article.GetCol(y0) + 2) * m_CxChar;
						rectRow.top = i * m_CyChar;
						rectRow.bottom = (i + 1) * m_CyChar;
						SetBkColor(hMemDC, RGB(0, 255, 255));
						DrawText(hMemDC, article.GetContent(i + y_offset / m_CyChar) + x0,
							article.GetCount(i + y_offset / m_CyChar) - x0,
							&rectRow, TextFormat);
						SetBkColor(hMemDC, RGB(255, 255, 255));
					}
					else if ((i + y_offset / m_CyChar) == y1)   // 最后一行
					{
						rectRow.left = 0;
						rectRow.right = article.GetCol(y1, x1) * m_CxChar;
						rectRow.top = i * m_CyChar;
						rectRow.bottom = (i + 1) * m_CyChar;
						SetBkColor(hMemDC, RGB(0, 255, 255));
						DrawText(hMemDC, article.GetContent(i + y_offset / m_CyChar),
							x1,
							&rectRow, TextFormat);
						rectRow.left = article.GetCol(y1, x1) * m_CxChar;
						rectRow.right = (article.GetCol(y1) + 2) * m_CxChar;
						rectRow.top = i * m_CyChar;
						rectRow.bottom = (i + 1) * m_CyChar;
						SetBkColor(hMemDC, RGB(255, 255, 255));
						DrawText(hMemDC, article.GetContent(i + y_offset / m_CyChar) + x1,
							article.GetCount(i + y_offset / m_CyChar) - x1,
							&rectRow, TextFormat);
						SetBkColor(hMemDC, RGB(255, 255, 255));
					}
					else
					{
						rectRow.left = 0;
						rectRow.right = (article.GetCol(i + y_offset / m_CyChar) + 2) * m_CxChar;
						rectRow.top = i * m_CyChar;
						rectRow.bottom = (i + 1) * m_CyChar;
						SetBkColor(hMemDC, RGB(0, 255, 255));
						DrawText(hMemDC, article.GetContent(i + y_offset / m_CyChar),
							article.GetCount(i + y_offset / m_CyChar),
							&rectRow, TextFormat);
						SetBkColor(hMemDC, RGB(255, 255, 255));
					}
				}
			}
			else                          // 绘制未被选中行
			{
				rectRow.left = 0;
				rectRow.right = (article.GetCol(i) + 2) * m_CxChar;
				rectRow.top = i * m_CyChar;
				rectRow.bottom = (i + 1) * m_CyChar;
				DrawText(hMemDC, article.GetContent(i + m_Caret.yOffset / m_CyChar),
					article.GetCount(i + m_Caret.yOffset / m_CyChar),
					&rectRow, TextFormat);
			}
		}
		//////////////////////////////////////////////////////////////////////////
		//  显示行号
		//////////////////////////////////////////////////////////////////////////
		rectRowNum.left = 0;
		rectRowNum.right = EDIT_MARGIN_LEFT;
		rectRowNum.top = i * m_CyChar;
		rectRowNum.bottom = (i + 1) * m_CyChar;
		TCHAR strTemp[30];
		int   rowNum = i + 1 + m_Caret.yOffset / m_CyChar;
		wsprintf(strTemp, TEXT("%5d"), rowNum);
		SetBkColor(hMemNumDC, RGB(220, 220, 220));
		SetTextColor(hMemNumDC, RGB(0, 180, 200));
		DrawText(hMemNumDC, strTemp,
			6,
			&rectRowNum, DT_LEFT | DT_NOPREFIX | DT_NOCLIP | DT_EXPANDTABS);
		SetBkColor(hMemNumDC, RGB(255, 255, 255));
		SetTextColor(hMemNumDC, RGB(255, 255, 255));
		//////////////////////////////////////////////////////////////////////////
	}
	DeleteObject(SelectObject(hMemDC, GetStockObject(SYSTEM_FONT)));
	// 绘图 End

	BitBlt(hdc, EDIT_MARGIN_LEFT, 0, rect.right, rect.bottom, hMemDC, m_Caret.xOffset, 0, SRCCOPY); // 内容
	BitBlt(hdc, 0, 0, EDIT_MARGIN_LEFT, rect.bottom, hMemNumDC, 0, 0, SRCCOPY);                     // 行号
	// 双缓冲 End
	SelectObject(hMemDC, hPreBmp);
	DeleteObject(hMemBmp);
	DeleteDC(hMemDC);
	SelectObject(hMemNumDC, hPreNumBmp);
	DeleteObject(hMemNumBmp);
	DeleteDC(hMemNumDC);
	//   }
	EndPaint(m_hWnd, &ps);
}

void XEdit::OnDestroy()
{
	article.Destroy();

	//   while (!UndoArticleList.Empty())
	//   {
	//     CArticle* temp = UndoArticleList.Pop();
	//     temp->Destroy();
	//     delete temp;
	//   }
	while (!UndoArticleTagList.Empty())
	{
		CArticleTag* temp = UndoArticleTagList.Pop();
		temp->Destroy();
		delete temp;
	}
	//   while (!RedoArticleList.Empty())
	//   {
	//     CArticle* temp = RedoArticleList.Pop();
	//     temp->Destroy();
	//     delete temp;
	//   }
	while (!RedoArticleTagList.Empty())
	{
		CArticleTag* temp = RedoArticleTagList.Pop();
		temp->Destroy();
		delete temp;
	}

	//   for (list<CArticle*>::iterator iter = UndoArticleList.begin(); iter != UndoArticleList.end(); iter++)
	//   {
	//     (*iter)->Destroy();
	//   }
	//   for (list<CArticle*>::iterator iter = RedoArticleList.begin(); iter != RedoArticleList.end(); iter++)
	//   {
	//     (*iter)->Destroy();
	//   }

	//   undoArticle.Destroy();
	//   redoArticle.Destroy();
	//   if (showReady)
	//   {
	//     showReady = FALSE;
	//     SelectObject(hMemDC, hPreBmp);
	//     DeleteObject(hMemBmp);
	//     DeleteDC(hMemDC);
	//   }
}

LRESULT XEdit::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INPUTLANGCHANGE:
		m_CharSet = wParam;
		return 0;
	case WM_CREATE:
		OnCreate();
		// 直接往下执行
	case WM_SETTINGCHANGE:
	{
		OnSettingChange();
		return 0;
	}
	case WM_SIZE:
	{
		OnSize(LOWORD(lParam), HIWORD(lParam));
		return 0;
	}
	case WM_SETFOCUS:
	{                           // 创建并显示插入符
		CreateCaret(hWnd, NULL, 1, m_CyChar);
		m_Caret.SetPos(xCaretPix, yCaretPix);
		ShowCaret(hWnd);
		return 0;
	}
	case WM_KILLFOCUS:
	{                           // 隐藏并销毁插入符
		HideCaret(hWnd);
		DestroyCaret();
		return 0;
	}
	case WM_KEYDOWN:
	{
		OnKeyDown(wParam, lParam);
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		//       int Pos_X = LOWORD(lParam);     // 水平坐标
		//       int Pos_Y = HIWORD(lParam);     // 竖直坐标
		//       printf("Pos_X = %d, Pos_Y = %d\n", Pos_X/m_CxChar, Pos_Y/m_CyChar);
		::SetCapture(hWnd);     // 设置窗口鼠标捕获
		OnLButtonDown(wParam, lParam);
		return 0;
	}
	case WM_LBUTTONUP:
	{
		OnLButtonUp(wParam, lParam);
		::ReleaseCapture();     // 释放窗口鼠标捕获
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		OnMouseMove(wParam, lParam);
		return 0;
	}
	case WM_COMMAND:
	{
		OnCommand(wParam, lParam);
		return 0;
	}
	case WM_CHAR:
	{
		OnChar(wParam, lParam);
		return 0;
	}
	case WM_FIND:
	{
		OnFind(wParam, lParam);
		return 0;
	}
	case WM_SETTEXT:
	{
		return OnSetText(wParam, lParam);
	}
	case WM_VSCROLL:
	{
		OnVScroll(LOWORD(wParam));
		return 0;
	}
	case WM_HSCROLL:
	{
		OnHScroll(LOWORD(wParam));
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		OnMouseWheel(wParam, lParam);
		return 0;
	}
	case WM_PAINT:
	{
		OnPaint();
		return 0;
	}
	case WM_DESTROY:
	{
		OnDestroy();
		break;
	}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void XEdit::CARET::GetPos(int &cx, int &cy)           // 获取绝对坐标
{
	cx = this->x;
	cy = this->y;
}
// void XEdit::CARET::GetRelativePos(int &cx, int &cy)   // 获取相对坐标(相对窗体)
// {
//   cx = (this->x - xOffset);
//   cy = (this->y - yOffset);
// }
void XEdit::CARET::SetPos(int cx, int cy)           // 设置绝对坐标
{
	this->x = cx;
	this->y = cy;
	SetCaretPos(x - xOffset + EDIT_MARGIN_LEFT, y - yOffset);
}
// void XEdit::CARET::SetRelativePos(int cx, int cy)   // 设置相对坐标(相对窗体)
// {
//   this->x = (cx + xOffset);
//   this->y = (cy + yOffset);
//   SetCaretPos(x - xOffset, y - yOffset);
// }

void XEdit::ScrollMove(int nBar, int Pos)
{
	RECT rect;

	m_Scroll.si.cbSize = sizeof(m_Scroll.si);
	m_Scroll.si.fMask = SIF_ALL;
	GetScrollInfo(m_hWnd, nBar, &m_Scroll.si);
	m_Scroll.m_Pos = m_Scroll.si.nPos;
	m_Scroll.si.nPos = Pos;
	m_Scroll.si.fMask = SIF_POS;
	SetScrollInfo(m_hWnd, nBar, &m_Scroll.si, TRUE);
	// 如果位置变化了，则移动窗口并重绘
	GetScrollInfo(m_hWnd, nBar, &m_Scroll.si);
	if (m_Scroll.si.nPos != m_Scroll.m_Pos)
	{
		if (nBar == SB_HORZ)
			m_Caret.xOffset = m_Scroll.si.nPos * m_CxChar;
		else if (nBar == SB_VERT)
			m_Caret.yOffset = m_Scroll.si.nPos * m_CyChar;

		m_Caret.SetPos(xCaretPix, yCaretPix);
		GetClientRect(m_hWnd, &rect);
		InvalidateRect(m_hWnd, &rect, FALSE);
		UpdateWindow();
	}
}

void XEdit::ScrollMove(int nBar, SCROLL_FLAG Flag)
{
	RECT rect;

	m_Scroll.si.cbSize = sizeof(m_Scroll.si);
	m_Scroll.si.fMask = SIF_ALL;
	GetScrollInfo(m_hWnd, nBar, &m_Scroll.si);
	m_Scroll.m_Pos = m_Scroll.si.nPos;
	if (Flag == SCROLL_MAX)
		m_Scroll.si.nPos = m_Scroll.si.nMax;
	else if (Flag == SCROLL_MIN)
		m_Scroll.si.nPos = m_Scroll.si.nMin;
	else if (Flag == SCROLL_INC)
		m_Scroll.si.nPos += 1;
	else if (Flag == SCROLL_DEC)
		m_Scroll.si.nPos -= 1;
	m_Scroll.si.fMask = SIF_POS;
	SetScrollInfo(m_hWnd, nBar, &m_Scroll.si, TRUE);
	// 如果位置变化了，则移动窗口并重绘
	GetScrollInfo(m_hWnd, nBar, &m_Scroll.si);
	if (m_Scroll.si.nPos != m_Scroll.m_Pos)
	{
		if (nBar == SB_HORZ)
			m_Caret.xOffset = m_Scroll.si.nPos * m_CxChar;
		else if (nBar == SB_VERT)
			m_Caret.yOffset = m_Scroll.si.nPos * m_CyChar;

		m_Caret.SetPos(xCaretPix, yCaretPix);
		GetClientRect(m_hWnd, &rect);
		InvalidateRect(m_hWnd, &rect, FALSE);
		UpdateWindow();
	}
}

void XEdit::ScrollResize(int nBar, int nMax)
{
	//   if (nBar = SB_HORZ)
	//     nMax += EDIT_MARGIN_LEFT/8;
	m_Scroll.si.cbSize = sizeof(m_Scroll.si);
	m_Scroll.si.fMask = SIF_ALL;
	GetScrollInfo(m_hWnd, nBar, &m_Scroll.si);
	m_Scroll.si.nMin = 0;
	m_Scroll.si.nMax = (nMax > 0) ? (nMax) : 0;
	SetScrollInfo(m_hWnd, nBar, &m_Scroll.si, FALSE);
	//   SetScrollPos(m_hWnd, nBar, 0, TRUE);             // 竖直滚动条复位
}

void XEdit::SELECTOR::Reset()
{
	start = TRUE;
	selected = FALSE;
	rows = 0;
	Point0.x = 0; Point0.y = 0;
	Point1.x = 0; Point1.y = 0;
}

int XEdit::GetRealCxChar(int line, int col)
{
	TCHAR ch = article.GetData(line, col);
	if (article.GetDataType(ch))
	{
		return (m_CxChar * 2);
	}
	else
	{
		return m_CxChar;
	}
}

bool XEdit::Redo(void)               // 恢复
{
	if (RedoArticleTagList.Empty())
	{
		return FALSE;
	}
	else
	{
		LogElement log = { LogElement::DEFAULT, 0, 0, NULL };
		AddLog(log);

		int colTemp = 0, lineTemp = 0;
		m_Caret.GetPos(colTemp, lineTemp);
		undoColPixList.Push(colTemp);
		undoLinePixList.Push(lineTemp);

		CArticleTag *redoArticleTemp = RedoArticleTagList.Pop();  // redoList出栈，执行redo操作

		CArticleTag *undoArticleTemp = new CArticleTag;           // 新增到undoList
		undoArticleTemp->Initialize(TEXT(""));
		undoArticleTemp->LineCountPre = redoArticleTemp->LineCount;
		undoArticleTemp->LineCount = redoArticleTemp->LineCountPre;
		undoArticleTemp->LineBegin = redoArticleTemp->LineBeginPre;
		undoArticleTemp->LineBeginPre = redoArticleTemp->LineBegin;

		for (int i = 0; i<redoArticleTemp->LineCount; i++)
		{
			undoArticleTemp->InsertRow(i, article.GetContent(redoArticleTemp->LineBegin));
			article.DeleteRow(redoArticleTemp->LineBegin);
		}
		for (int i = 0; i<redoArticleTemp->LineCountPre; i++)
		{
			article.InsertRow(redoArticleTemp->LineBeginPre + i, redoArticleTemp->GetContent(i));
		}
		delete redoArticleTemp;

		UndoArticleTagList.Push(undoArticleTemp);

		m_Caret.SetPos(redoColPixList.Pop(), redoLinePixList.Pop());
	}
	//   article.Initialize(redoArticle.Format());
	//   article.Initialize(RedoArticleList.Pop()->Format());

	//   m_Caret.SetPos(redoColPix, redoLinePix);

	// 新增行后，若超出客户区范围，则刷新滚动条
	m_Caret.GetPos(xCaretPix, yCaretPix);
	if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
	{
		//           article.GetColMax() = article.GetColMax();
		ScrollResize(SB_HORZ, article.GetColMax());
		ScrollMove(SB_HORZ, SCROLL_MIN);
	}
	if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
	{
		//           article.GetRows() = article.GetRows();
		ScrollResize(SB_VERT, article.GetRows());
		::SendMessage(m_hWnd, WM_KEYDOWN, VK_UP, NULL);   // 插入换行后插入符向上移动(这两步只是为了滚动页面)
		::SendMessage(m_hWnd, WM_KEYDOWN, VK_DOWN, NULL); // 插入换行后插入符向下移动
	}

	RECT rect;
	GetClientRect(m_hWnd, &rect);
	InvalidateRect(m_hWnd, &rect, FALSE);   // 窗口重绘
	UpdateWindow();

	//   LogElement log;
	//   if (RedoStack.Empty())
	//     return false;
	// 
	//   while (!RedoStack.Empty())
	//   {
	//     log = RedoStack.Pop();
	//     UndoStack.Push(log);
	//     DoCMD(REDO, log);
	//   }

	return true;
}

bool XEdit::Undo(void)               // 撤销
{
	//   article.Initialize(undoArticle.Format());
	if (UndoArticleTagList.Empty())
	{
		return FALSE;
		//     article.Destroy();
		//     article.Initialize(TEXT(""));
		// 
		//     m_Caret.SetPos(0, 0);
	}
	else
	{
		LogElement log = { LogElement::DEFAULT, 0, 0, NULL };
		AddLog(log);

		int colTemp = 0, lineTemp = 0;
		m_Caret.GetPos(colTemp, lineTemp);
		redoColPixList.Push(colTemp);
		redoLinePixList.Push(lineTemp);

		CArticleTag *undoArticleTemp = UndoArticleTagList.Pop();  // undoList出栈，执行undo操作

		CArticleTag *redoArticleTemp = new CArticleTag;           // 新增到到redoList
		redoArticleTemp->Initialize(TEXT(""));
		redoArticleTemp->LineCountPre = undoArticleTemp->LineCount;
		redoArticleTemp->LineCount = undoArticleTemp->LineCountPre;
		redoArticleTemp->LineBegin = undoArticleTemp->LineBeginPre;
		redoArticleTemp->LineBeginPre = undoArticleTemp->LineBegin;

		//     article.Initialize(undoArticleTemp->Format());
		for (int i = 0; i<undoArticleTemp->LineCount; i++)
		{
			redoArticleTemp->InsertRow(i, article.GetContent(undoArticleTemp->LineBegin));
			article.DeleteRow(undoArticleTemp->LineBegin);
		}
		for (int i = 0; i<undoArticleTemp->LineCountPre; i++)
		{
			article.InsertRow(undoArticleTemp->LineBeginPre + i, undoArticleTemp->GetContent(i));
		}
		delete undoArticleTemp;

		RedoArticleTagList.Push(redoArticleTemp);

		m_Caret.SetPos(undoColPixList.Pop(), undoLinePixList.Pop());
	}

	//   m_Caret.SetPos(undoColPix, undoLinePix);
	//   cout << "undoLinePix: " << undoLinePix << ", undoColPix：" << undoColPix << endl;
	// 新增行后，若超出客户区范围，则刷新滚动条
	m_Caret.GetPos(xCaretPix, yCaretPix);
	cout << "yCaretPix: " << yCaretPix << ", xCaretPix：" << xCaretPix << endl;
	if (xCaretPix < m_Caret.xOffset || xCaretPix >(m_Caret.xOffset + m_CxClient - m_CxChar - EDIT_MARGIN_LEFT))   // 插入符不在客户区时
	{
		//           article.GetColMax() = article.GetColMax();
		ScrollResize(SB_HORZ, article.GetColMax());
		ScrollMove(SB_HORZ, SCROLL_MIN);
	}
	if (yCaretPix < m_Caret.yOffset || yCaretPix >(m_Caret.yOffset + m_CyClient - m_CyChar))
	{
		//           article.GetRows() = article.GetRows();
		ScrollResize(SB_VERT, article.GetRows());
		::SendMessage(m_hWnd, WM_KEYDOWN, VK_UP, NULL);   // 插入换行后插入符向上移动(这两步只是为了滚动页面)
		::SendMessage(m_hWnd, WM_KEYDOWN, VK_DOWN, NULL); // 插入换行后插入符向下移动
	}

	RECT rect;
	GetClientRect(m_hWnd, &rect);
	InvalidateRect(m_hWnd, &rect, FALSE);   // 窗口重绘
	UpdateWindow();

	//   LogElement log;
	//   if (UndoStack.Empty())
	//     return false;
	// 
	//   while (!UndoStack.Empty())
	//   {
	//     log = UndoStack.Pop();
	//     RedoStack.Push(log);
	//     DoCMD(UNDO, log);
	//   }

	return true;
}

void XEdit::DoCMD(RedoOrUndo select, LogElement log)    // 执行一项纪录
{
	//   LogElement logTemp;
	//   logTemp = log;
	// 
	//   // 屏蔽Log纪录,后面恢复
	//   redoUndoFlag = TRUE;
	// 
	//   if (select == UNDO)       // 如果是撤销，则执行回滚操作
	//   {
	//     if (log.cmd > LogElement::OFFSET)
	//     {
	//       logTemp.cmd = (LogElement::CMD)(log.cmd - LogElement::OFFSET);
	//     }
	//     else if (log.cmd < LogElement::OFFSET)
	//     {
	//       logTemp.cmd = (LogElement::CMD)(log.cmd + LogElement::OFFSET);
	//     }
	//   }
	// 
	//   switch (logTemp.cmd)
	//   {
	//   case LogElement::ADD_CHAR:
	//     {
	//       std::wcout  << "Add_Char -- " << "Line:" << logTemp.line 
	//         << ",Index:" << logTemp.index << ",Char:" << logTemp.ch << endl;
	//       ::SendMessage(m_hWnd, WM_CHAR, logTemp.ch, 1);    // 发送消息
	//       break;
	//     }
	//   case LogElement::DEL_CHAR:
	//     {
	//       std::wcout  << "Del_Char -- " << "Line:" << logTemp.line 
	//         << ",Index:" << logTemp.index << ",Char:" << logTemp.ch << endl;
	//       break;
	//     }
	//   case LogElement::ENTER:
	//     {
	//       std::wcout  << "Enter    -- " << "Line:" << logTemp.line 
	//         << ",Index:" << logTemp.index << ",Char:" << logTemp.ch << endl;
	//       break;
	//     }
	//   case LogElement::BACKSPACE:
	//     {
	//       std::wcout  << "Backspace-- " << "Line:" << logTemp.line 
	//         << ",Index:" << logTemp.index << ",Char:" << logTemp.ch << endl;
	//       break;
	//     }
	//   case LogElement::UN_ADD_CHAR:
	//     {
	//       std::wcout  << "Un_Add_Char -- " << "Line:" << logTemp.line 
	//         << ",Index:" << logTemp.index << ",Char:" << logTemp.ch << endl;
	//       ::SendMessage(m_hWnd, WM_KEYDOWN, VK_BACK, NULL);
	//       break;
	//     }
	//   case LogElement::UN_DEL_CHAR:
	//     {
	//       std::wcout  << "Un_Del_Char -- " << "Line:" << logTemp.line 
	//         << ",Index:" << logTemp.index << ",Char:" << logTemp.ch << endl;
	//       break;
	//     }
	//   case LogElement::UN_ENTER:
	//     {
	//       std::wcout  << "Un_Enter    -- " << "Line:" << logTemp.line 
	//         << ",Index:" << logTemp.index << ",Char:" << logTemp.ch << endl;
	//       break;
	//     }
	//   case LogElement::UN_BACKSPACE:
	//     {
	//       std::wcout  << "Un_Backspace-- " << "Line:" << logTemp.line 
	//         << ",Index:" << logTemp.index << ",Char:" << logTemp.ch << endl;
	//       break;
	//     }
	//   default:
	//     break;
	//   }
	// 
	//   // 恢复Log纪录
	//   redoUndoFlag = FALSE;
}

void XEdit::AddLog(LogElement log)   // 添加一项纪录
{
	static LogElement logPre;
	static LogElement logNow;

	int  undoLinePix;
	int  undoColPix;

	// 如果undoList超过10，则清掉最初1个
	//   if (UndoArticleTagList.GetSize() >= 10)
	//   {
	//     CArticleTag *articleTemp = UndoArticleTagList.PopFront();
	//     delete articleTemp;
	//     undoColPixList.PopFront();
	//     undoLinePixList.PopFront();
	//   }

	// 当进行过Undo操作时，清空RedoList
	if (log.cmd != LogElement::DEFAULT)
	{
		redoColPixList.Clear();
		redoLinePixList.Clear();
		while (!RedoArticleTagList.Empty())
		{
			CArticleTag* temp = RedoArticleTagList.Pop();
			temp->Destroy();
			delete temp;
		}
	}

	logPre = logNow;
	logNow = log;

	if (logNow.cmd == LogElement::UN_ADD_CHAR)
	{
		if (logPre.cmd != LogElement::ADD_CHAR || logNow.line != logPre.line || logNow.index != logPre.index)
		{
			CArticleTag* undoArticle = new CArticleTag;
			//     undoArticle->Initialize(article.Format());
			undoArticle->Initialize(TEXT(""));
			undoArticle->InsertRow(0, article.GetContent(logNow.line / m_CyChar));
			undoArticle->LineBegin = logNow.line / m_CyChar;
			undoArticle->LineCount = 1;
			undoArticle->LineBeginPre = logNow.line / m_CyChar;
			undoArticle->LineCountPre = 1;

			undoLinePix = logNow.line;
			undoColPix = logNow.index;
			UndoArticleTagList.Push(undoArticle);
			undoLinePixList.Push(undoLinePix);
			undoColPixList.Push(undoColPix);
			return;
		}
		return;
	}

	if (logNow.cmd == LogElement::UN_BACKSPACE)
	{
		if (logPre.cmd != LogElement::BACKSPACE || logNow.line != logPre.line || logNow.index != logPre.index)
		{
			CArticleTag* undoArticle = new CArticleTag;
			//     undoArticle->Initialize(article.Format());
			undoArticle->Initialize(TEXT(""));
			if (logNow.lineCount == NULL)
			{
				undoArticle->LineBegin = logNow.line / m_CyChar;
				undoArticle->LineCount = 1;
				undoArticle->LineBeginPre = logNow.line / m_CyChar;
				undoArticle->LineCountPre = 1;
				undoArticle->InsertRow(0, article.GetContent(logNow.line / m_CyChar));
			}
			else
			{
				undoArticle->LineBegin = logNow.line / m_CyChar;
				undoArticle->LineCount = 1;
				undoArticle->LineBeginPre = logNow.line / m_CyChar;
				undoArticle->LineCountPre = logNow.lineCount;
				for (int i = 0; i<logNow.lineCount; i++)
				{
					undoArticle->InsertRow(i, article.GetContent(logNow.line / m_CyChar + i));
				}
			}

			undoLinePix = logNow.line;
			undoColPix = logNow.index;
			UndoArticleTagList.Push(undoArticle);
			undoLinePixList.Push(undoLinePix);
			undoColPixList.Push(undoColPix);
			return;
		}
		return;
	}
	if (logNow.cmd == LogElement::BACKSPACE)
	{
		//     redoArticle->Initialize(article.Format());
		//     redoLinePix = logNow.line;
		//     redoColPix = logNow.index;
		//     RedoArticleList.Push(redoArticle);
		//     redoLinePixList.Push(redoLinePix);
		//     redoColPixList.Push(redoColPix);
		return;
	}

	if (logNow.cmd == LogElement::UN_DEL_CHAR)
	{
		if (logPre.cmd != LogElement::DEL_CHAR || logNow.line != logPre.line || logNow.index != logPre.index)
		{
			CArticleTag* undoArticle = new CArticleTag;
			//     undoArticle->Initialize(article.Format());
			undoArticle->Initialize(TEXT(""));
			if (logNow.lineCount == NULL)
			{
				undoArticle->LineBegin = logNow.line / m_CyChar;
				undoArticle->LineCount = 1;
				undoArticle->LineBeginPre = logNow.line / m_CyChar;
				undoArticle->LineCountPre = 1;
				undoArticle->InsertRow(0, article.GetContent(logNow.line / m_CyChar));
			}
			else
			{
				undoArticle->LineBegin = logNow.line / m_CyChar;
				undoArticle->LineCount = 1;
				undoArticle->LineBeginPre = logNow.line / m_CyChar;
				undoArticle->LineCountPre = logNow.lineCount;
				for (int i = 0; i<logNow.lineCount; i++)
				{
					undoArticle->InsertRow(i, article.GetContent(logNow.line / m_CyChar + i));
				}
			}
			undoLinePix = logNow.line;
			undoColPix = logNow.index;
			UndoArticleTagList.Push(undoArticle);
			undoLinePixList.Push(undoLinePix);
			undoColPixList.Push(undoColPix);
			return;
		}
		return;
	}
	if (logNow.cmd == LogElement::DEL_CHAR)
	{
		//     redoArticle->Initialize(article.Format());
		//     redoLinePix = logNow.line;
		//     redoColPix = logNow.index;
		//     RedoArticleList.Push(redoArticle);
		//     redoLinePixList.Push(redoLinePix);
		//     redoColPixList.Push(redoColPix);
		return;
	}

	if (logNow.cmd == LogElement::UN_ENTER)
	{
		CArticleTag* undoArticle = new CArticleTag;
		//       undoArticle->Initialize(article.Format());
		undoArticle->Initialize(TEXT(""));
		undoArticle->InsertRow(0, article.GetContent(logNow.line / m_CyChar));
		undoArticle->LineBegin = logNow.line / m_CyChar;
		undoArticle->LineCount = 2;
		undoArticle->LineBeginPre = logNow.line / m_CyChar;
		undoArticle->LineCountPre = 1;

		undoLinePix = logNow.line;
		undoColPix = logNow.index;
		UndoArticleTagList.Push(undoArticle);
		undoLinePixList.Push(undoLinePix);
		undoColPixList.Push(undoColPix);
		return;
	}
	if (logNow.cmd == LogElement::ENTER)
	{
		//     redoArticle->Initialize(article.Format());
		//     redoLinePix = logNow.line;
		//     redoColPix = logNow.index;
		//     RedoArticleList.Push(redoArticle);
		//     redoLinePixList.Push(redoLinePix);
		//     redoColPixList.Push(redoColPix);
		return;
	}

	if (logNow.cmd == LogElement::UN_CUT)
	{
		CArticleTag* undoArticle = new CArticleTag;
		//     undoArticle->Initialize(article.Format());
		undoArticle->Initialize(TEXT(""));
		undoArticle->LineBegin = logNow.line / m_CyChar;
		undoArticle->LineCount = 1;
		undoArticle->LineBeginPre = logNow.line / m_CyChar;
		undoArticle->LineCountPre = logNow.lineCount;
		for (int i = 0; i<logNow.lineCount; i++)
		{
			undoArticle->InsertRow(i, article.GetContent(logNow.line / m_CyChar + i));
		}

		undoLinePix = logNow.line;
		undoColPix = logNow.index;
		UndoArticleTagList.Push(undoArticle);
		undoLinePixList.Push(undoLinePix);
		undoColPixList.Push(undoColPix);
		return;
	}
	if (logNow.cmd == LogElement::CUT)
	{
		//     redoArticle->Initialize(article.Format());
		//     redoLinePix = logNow.line;
		//     redoColPix = logNow.index;
		//     RedoArticleList.Push(redoArticle);
		//     redoLinePixList.Push(redoLinePix);
		//     redoColPixList.Push(redoColPix);
		return;
	}

	if (logNow.cmd == LogElement::UN_PASTE)
	{
		CArticleTag* undoArticle = new CArticleTag;
		//     undoArticle->Initialize(article.Format());
		undoArticle->Initialize(TEXT(""));
		undoArticle->LineBegin = logNow.line / m_CyChar;
		undoArticle->LineCount = logNow.lineCount;
		undoArticle->LineBeginPre = logNow.line / m_CyChar;
		undoArticle->LineCountPre = 1;
		undoArticle->InsertRow(0, article.GetContent(logNow.line / m_CyChar));

		undoLinePix = logNow.line;
		undoColPix = logNow.index;
		UndoArticleTagList.Push(undoArticle);
		undoLinePixList.Push(undoLinePix);
		undoColPixList.Push(undoColPix);
		return;
	}
	if (logNow.cmd == LogElement::PASTE)
	{
		//     redoArticle->Initialize(article.Format());
		//     redoLinePix = logNow.line;
		//     redoColPix = logNow.index;
		//     RedoArticleList.Push(redoArticle);
		//     redoLinePixList.Push(redoLinePix);
		//     redoColPixList.Push(redoColPix);
		return;
	}
}

// void XEdit::ClearLogAll(void)                // 清空纪录
// {
//   if (redoUndoFlag == FALSE)  // 只有正常文本处理时才纪录
//   {
//     UndoStack.Clear();
//     RedoStack.Clear();
//   }
// }

