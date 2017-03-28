#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "XWnd.h"
#include "XDialog.h"
#include "XFindDlg.h"
#include "Resource.h"

using namespace std;

BOOL XFindDlg::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hCaseSensitive;             // 区分大小写
	static HWND hWholeWord;                 // 全词匹配
	static HWND hBeginOfFile;               // 从文件头开始
	static HWND hReplaceNotify;             // 替换时提示
	static HWND hFindNext;                  // 下一个
	static HWND hFindPrev;                  // 上一个

	static BOOL beginOfFile = FALSE;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_SHOWWINDOW:
		{
			if (SendMessage(hBeginOfFile, BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				beginOfFile = TRUE;
				wcout << "从文件头开始： " << "\tON" << endl;
			}
			else
			{
				beginOfFile = FALSE;
				wcout << "从文件头开始：" << "\tOFF" << endl;
			}
			return TRUE;
		}
	case WM_INITDIALOG:
		{
			m_hWnd = hDlg;
			hCaseSensitive = GetDlgItem(hDlg, IDC_CHECK_WHOLE_WORD);
			hWholeWord = GetDlgItem(hDlg, IDC_CHECK_WHOLE_WORD);
			hBeginOfFile = GetDlgItem(hDlg, IDC_CHECK_BEGIN_OF_FILE);
			hReplaceNotify = GetDlgItem(hDlg, IDC_CHECK_REPLACE_NOTIFY);
			hFindNext = GetDlgItem(hDlg, IDC_RADIO_FIND_NEXT);
			hFindPrev = GetDlgItem(hDlg, IDC_RADIO_FIND_PREV);

			::SendMessage(hFindPrev, BM_SETCHECK, 0, 0);        // 默认向后查找
			::SendMessage(hFindNext, BM_SETCHECK, 1, 0);
			::SendMessage(hCaseSensitive, BM_SETCHECK, 1, 0);   // 默认区分大小写
			return TRUE;
		}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
			printf("IDOK\n");
			//           EndDialog(hDlg, TRUE);
			::SendMessage(hDlg, WM_COMMAND, IDC_BUTTON_SEARCH_NEXT, NULL);
			return TRUE;

		case IDC_BUTTON_SEARCH_QUIT:        // "取消"按键
		case IDCANCEL:                      // 窗口右上角的叉
			printf("IDCANCEL\n");
			EndDialog(hDlg, FALSE);
			return TRUE;


		case IDC_CHECK_BEGIN_OF_FILE:
		{
			if ((SendMessage(hBeginOfFile, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				beginOfFile = TRUE;
				wcout << "从文件头开始：" << "\tON" << endl;
			}
			else
			{
				beginOfFile = FALSE;
				wcout << "从文件头开始：" << "\tOFF" << endl;
			}
			return TRUE;
		}

		case IDC_BUTTON_SEARCH_NEXT:
		{
			WPARAM findParam = 0;

			wcout << endl;
			if ((SendMessage(hCaseSensitive, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_CASE_SENSITIVE;
				wcout << "区分大小写：" << "\tON" << endl;
			}
			else
			{
				wcout << "区分大小写：" << "\tOFF" << endl;
			}
			if ((SendMessage(hWholeWord, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_WHOLE_WORD;
				wcout << "全词匹配：" << "\tON" << endl;
			}
			else
			{
				wcout << "全词匹配：" << "\tOFF" << endl;
			}
			//           if ((SendMessage(hBeginOfFile, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			if (beginOfFile)
			{
				beginOfFile = FALSE;
				findParam |= FP_BEGIN_OF_FILE;
				wcout << "从文件头开始：" << "\tON" << endl;
			}
			else
			{
				wcout << "从文件头开始：" << "\tOFF" << endl;
			}
			if ((SendMessage(hReplaceNotify, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_REPLACE_NOTIFY;
				wcout << "替换时提示：" << "\tON" << endl;
			}
			else
			{
				wcout << "替换时提示：" << "\tOFF" << endl;
			}
			if ((SendMessage(hFindNext, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_FIND_NEXT;
				wcout << "向后查找：" << "\tON" << endl;
			}
			else
			{
				wcout << "向后查找：" << "\tOFF" << endl;
			}
			if ((SendMessage(hFindPrev, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_FIND_PREV;
				wcout << "向前查找：" << "\tON" << endl;
			}
			else
			{
				wcout << "向前查找：" << "\tOFF" << endl;
			}
			wcout << "FindParam: " << findParam << endl;

			//           ::SendMessage(GetParent(hDlg), WM_CHAR, (WPARAM)(TEXT('\r')), (LPARAM)(1));
			wcout << "查找下一个\n";
			TCHAR strTemp[100];
			::GetWindowText(GetDlgItem(m_hWnd, IDC_EDIT1), strTemp, 100);
			wcout << "目标：" << (TCHAR *)strTemp << endl;
			::SendMessage(GetParent(hDlg), WM_FIND, findParam, (LPARAM)strTemp);

			return TRUE;
		}
		case IDC_BUTTON_REPLACE:
		{
			printf("替换\n");
			WPARAM findParam = 0;

			wcout << endl;
			if ((SendMessage(hCaseSensitive, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_CASE_SENSITIVE;
				wcout << "区分大小写：" << "\tON" << endl;
			}
			else
			{
				wcout << "区分大小写：" << "\tOFF" << endl;
			}
			if ((SendMessage(hWholeWord, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_WHOLE_WORD;
				wcout << "全词匹配：" << "\tON" << endl;
			}
			else
			{
				wcout << "全词匹配：" << "\tOFF" << endl;
			}
			//           if ((SendMessage(hBeginOfFile, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			if (beginOfFile)
			{
				beginOfFile = FALSE;
				findParam |= FP_BEGIN_OF_FILE;
				wcout << "从文件头开始：" << "\tON" << endl;
			}
			else
			{
				wcout << "从文件头开始：" << "\tOFF" << endl;
			}
			if ((SendMessage(hReplaceNotify, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_REPLACE_NOTIFY;
				wcout << "替换时提示：" << "\tON" << endl;
			}
			else
			{
				wcout << "替换时提示：" << "\tOFF" << endl;
			}
			if ((SendMessage(hFindNext, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_FIND_NEXT;
				wcout << "向后查找：" << "\tON" << endl;
			}
			else
			{
				wcout << "向后查找：" << "\tOFF" << endl;
			}
			if ((SendMessage(hFindPrev, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_FIND_PREV;
				wcout << "向前查找：" << "\tON" << endl;
			}
			else
			{
				wcout << "向前查找：" << "\tOFF" << endl;
			}
			wcout << "FindParam: " << findParam << endl;

			//           ::SendMessage(GetParent(hDlg), WM_CHAR, (WPARAM)(TEXT('\r')), (LPARAM)(1));
			wcout << "查找下一个\n";
			TCHAR strTemp[200];
			::GetWindowText(GetDlgItem(m_hWnd, IDC_EDIT1), strTemp, 100);

			findParam |= FP_REPLACE;
			::GetWindowText(GetDlgItem(m_hWnd, IDC_EDIT2), &strTemp[100], 100);
			wcout << "目标：" << (TCHAR *)strTemp << endl;
			::SendMessage(GetParent(hDlg), WM_FIND, findParam, (LPARAM)strTemp);

			return TRUE;
		}
		case IDC_BUTTON_REPLACE_ALL:
		{
			printf("全部替换\n");
			WPARAM findParam = 0;

			wcout << endl;
			if ((SendMessage(hCaseSensitive, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_CASE_SENSITIVE;
				wcout << "区分大小写：" << "\tON" << endl;
			}
			else
			{
				wcout << "区分大小写：" << "\tOFF" << endl;
			}
			if ((SendMessage(hWholeWord, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_WHOLE_WORD;
				wcout << "全词匹配：" << "\tON" << endl;
			}
			else
			{
				wcout << "全词匹配：" << "\tOFF" << endl;
			}
			//           if ((SendMessage(hBeginOfFile, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			if (beginOfFile)
			{
				beginOfFile = FALSE;
				findParam |= FP_BEGIN_OF_FILE;
				wcout << "从文件头开始：" << "\tON" << endl;
			}
			else
			{
				wcout << "从文件头开始：" << "\tOFF" << endl;
			}
			if ((SendMessage(hReplaceNotify, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_REPLACE_NOTIFY;
				wcout << "替换时提示：" << "\tON" << endl;
			}
			else
			{
				wcout << "替换时提示：" << "\tOFF" << endl;
			}
			if ((SendMessage(hFindNext, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_FIND_NEXT;
				wcout << "向后查找：" << "\tON" << endl;
			}
			else
			{
				wcout << "向后查找：" << "\tOFF" << endl;
			}
			if ((SendMessage(hFindPrev, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_FIND_PREV;
				wcout << "向前查找：" << "\tON" << endl;
			}
			else
			{
				wcout << "向前查找：" << "\tOFF" << endl;
			}
			wcout << "FindParam: " << findParam << endl;

			//           ::SendMessage(GetParent(hDlg), WM_CHAR, (WPARAM)(TEXT('\r')), (LPARAM)(1));
			wcout << "查找下一个\n";
			TCHAR strTemp[200];
			::GetWindowText(GetDlgItem(m_hWnd, IDC_EDIT1), strTemp, 100);

			findParam |= FP_REPLACE_ALL;
			::GetWindowText(GetDlgItem(m_hWnd, IDC_EDIT2), &strTemp[100], 100);
			wcout << "目标：" << (TCHAR *)strTemp << endl;
			::SendMessage(GetParent(hDlg), WM_FIND, findParam, (LPARAM)strTemp);

			return TRUE;
		}
		}
		break;
	}
	}
	return FALSE;
}
