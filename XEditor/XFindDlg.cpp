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
	static HWND hCaseSensitive;             // ���ִ�Сд
	static HWND hWholeWord;                 // ȫ��ƥ��
	static HWND hBeginOfFile;               // ���ļ�ͷ��ʼ
	static HWND hReplaceNotify;             // �滻ʱ��ʾ
	static HWND hFindNext;                  // ��һ��
	static HWND hFindPrev;                  // ��һ��

	static BOOL beginOfFile = FALSE;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_SHOWWINDOW:
		{
			if (SendMessage(hBeginOfFile, BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				beginOfFile = TRUE;
				wcout << "���ļ�ͷ��ʼ�� " << "\tON" << endl;
			}
			else
			{
				beginOfFile = FALSE;
				wcout << "���ļ�ͷ��ʼ��" << "\tOFF" << endl;
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

			::SendMessage(hFindPrev, BM_SETCHECK, 0, 0);        // Ĭ��������
			::SendMessage(hFindNext, BM_SETCHECK, 1, 0);
			::SendMessage(hCaseSensitive, BM_SETCHECK, 1, 0);   // Ĭ�����ִ�Сд
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

		case IDC_BUTTON_SEARCH_QUIT:        // "ȡ��"����
		case IDCANCEL:                      // �������ϽǵĲ�
			printf("IDCANCEL\n");
			EndDialog(hDlg, FALSE);
			return TRUE;


		case IDC_CHECK_BEGIN_OF_FILE:
		{
			if ((SendMessage(hBeginOfFile, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				beginOfFile = TRUE;
				wcout << "���ļ�ͷ��ʼ��" << "\tON" << endl;
			}
			else
			{
				beginOfFile = FALSE;
				wcout << "���ļ�ͷ��ʼ��" << "\tOFF" << endl;
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
				wcout << "���ִ�Сд��" << "\tON" << endl;
			}
			else
			{
				wcout << "���ִ�Сд��" << "\tOFF" << endl;
			}
			if ((SendMessage(hWholeWord, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_WHOLE_WORD;
				wcout << "ȫ��ƥ�䣺" << "\tON" << endl;
			}
			else
			{
				wcout << "ȫ��ƥ�䣺" << "\tOFF" << endl;
			}
			//           if ((SendMessage(hBeginOfFile, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			if (beginOfFile)
			{
				beginOfFile = FALSE;
				findParam |= FP_BEGIN_OF_FILE;
				wcout << "���ļ�ͷ��ʼ��" << "\tON" << endl;
			}
			else
			{
				wcout << "���ļ�ͷ��ʼ��" << "\tOFF" << endl;
			}
			if ((SendMessage(hReplaceNotify, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_REPLACE_NOTIFY;
				wcout << "�滻ʱ��ʾ��" << "\tON" << endl;
			}
			else
			{
				wcout << "�滻ʱ��ʾ��" << "\tOFF" << endl;
			}
			if ((SendMessage(hFindNext, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_FIND_NEXT;
				wcout << "�����ң�" << "\tON" << endl;
			}
			else
			{
				wcout << "�����ң�" << "\tOFF" << endl;
			}
			if ((SendMessage(hFindPrev, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_FIND_PREV;
				wcout << "��ǰ���ң�" << "\tON" << endl;
			}
			else
			{
				wcout << "��ǰ���ң�" << "\tOFF" << endl;
			}
			wcout << "FindParam: " << findParam << endl;

			//           ::SendMessage(GetParent(hDlg), WM_CHAR, (WPARAM)(TEXT('\r')), (LPARAM)(1));
			wcout << "������һ��\n";
			TCHAR strTemp[100];
			::GetWindowText(GetDlgItem(m_hWnd, IDC_EDIT1), strTemp, 100);
			wcout << "Ŀ�꣺" << (TCHAR *)strTemp << endl;
			::SendMessage(GetParent(hDlg), WM_FIND, findParam, (LPARAM)strTemp);

			return TRUE;
		}
		case IDC_BUTTON_REPLACE:
		{
			printf("�滻\n");
			WPARAM findParam = 0;

			wcout << endl;
			if ((SendMessage(hCaseSensitive, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_CASE_SENSITIVE;
				wcout << "���ִ�Сд��" << "\tON" << endl;
			}
			else
			{
				wcout << "���ִ�Сд��" << "\tOFF" << endl;
			}
			if ((SendMessage(hWholeWord, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_WHOLE_WORD;
				wcout << "ȫ��ƥ�䣺" << "\tON" << endl;
			}
			else
			{
				wcout << "ȫ��ƥ�䣺" << "\tOFF" << endl;
			}
			//           if ((SendMessage(hBeginOfFile, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			if (beginOfFile)
			{
				beginOfFile = FALSE;
				findParam |= FP_BEGIN_OF_FILE;
				wcout << "���ļ�ͷ��ʼ��" << "\tON" << endl;
			}
			else
			{
				wcout << "���ļ�ͷ��ʼ��" << "\tOFF" << endl;
			}
			if ((SendMessage(hReplaceNotify, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_REPLACE_NOTIFY;
				wcout << "�滻ʱ��ʾ��" << "\tON" << endl;
			}
			else
			{
				wcout << "�滻ʱ��ʾ��" << "\tOFF" << endl;
			}
			if ((SendMessage(hFindNext, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_FIND_NEXT;
				wcout << "�����ң�" << "\tON" << endl;
			}
			else
			{
				wcout << "�����ң�" << "\tOFF" << endl;
			}
			if ((SendMessage(hFindPrev, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_FIND_PREV;
				wcout << "��ǰ���ң�" << "\tON" << endl;
			}
			else
			{
				wcout << "��ǰ���ң�" << "\tOFF" << endl;
			}
			wcout << "FindParam: " << findParam << endl;

			//           ::SendMessage(GetParent(hDlg), WM_CHAR, (WPARAM)(TEXT('\r')), (LPARAM)(1));
			wcout << "������һ��\n";
			TCHAR strTemp[200];
			::GetWindowText(GetDlgItem(m_hWnd, IDC_EDIT1), strTemp, 100);

			findParam |= FP_REPLACE;
			::GetWindowText(GetDlgItem(m_hWnd, IDC_EDIT2), &strTemp[100], 100);
			wcout << "Ŀ�꣺" << (TCHAR *)strTemp << endl;
			::SendMessage(GetParent(hDlg), WM_FIND, findParam, (LPARAM)strTemp);

			return TRUE;
		}
		case IDC_BUTTON_REPLACE_ALL:
		{
			printf("ȫ���滻\n");
			WPARAM findParam = 0;

			wcout << endl;
			if ((SendMessage(hCaseSensitive, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_CASE_SENSITIVE;
				wcout << "���ִ�Сд��" << "\tON" << endl;
			}
			else
			{
				wcout << "���ִ�Сд��" << "\tOFF" << endl;
			}
			if ((SendMessage(hWholeWord, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_WHOLE_WORD;
				wcout << "ȫ��ƥ�䣺" << "\tON" << endl;
			}
			else
			{
				wcout << "ȫ��ƥ�䣺" << "\tOFF" << endl;
			}
			//           if ((SendMessage(hBeginOfFile, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			if (beginOfFile)
			{
				beginOfFile = FALSE;
				findParam |= FP_BEGIN_OF_FILE;
				wcout << "���ļ�ͷ��ʼ��" << "\tON" << endl;
			}
			else
			{
				wcout << "���ļ�ͷ��ʼ��" << "\tOFF" << endl;
			}
			if ((SendMessage(hReplaceNotify, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_REPLACE_NOTIFY;
				wcout << "�滻ʱ��ʾ��" << "\tON" << endl;
			}
			else
			{
				wcout << "�滻ʱ��ʾ��" << "\tOFF" << endl;
			}
			if ((SendMessage(hFindNext, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_FIND_NEXT;
				wcout << "�����ң�" << "\tON" << endl;
			}
			else
			{
				wcout << "�����ң�" << "\tOFF" << endl;
			}
			if ((SendMessage(hFindPrev, BM_GETCHECK, 0, 0)) == BST_CHECKED)
			{
				findParam |= FP_FIND_PREV;
				wcout << "��ǰ���ң�" << "\tON" << endl;
			}
			else
			{
				wcout << "��ǰ���ң�" << "\tOFF" << endl;
			}
			wcout << "FindParam: " << findParam << endl;

			//           ::SendMessage(GetParent(hDlg), WM_CHAR, (WPARAM)(TEXT('\r')), (LPARAM)(1));
			wcout << "������һ��\n";
			TCHAR strTemp[200];
			::GetWindowText(GetDlgItem(m_hWnd, IDC_EDIT1), strTemp, 100);

			findParam |= FP_REPLACE_ALL;
			::GetWindowText(GetDlgItem(m_hWnd, IDC_EDIT2), &strTemp[100], 100);
			wcout << "Ŀ�꣺" << (TCHAR *)strTemp << endl;
			::SendMessage(GetParent(hDlg), WM_FIND, findParam, (LPARAM)strTemp);

			return TRUE;
		}
		}
		break;
	}
	}
	return FALSE;
}
