#include <stdio.h>
#include "common.h"

XEditor::XEditor(HINSTANCE hInstance)
{
	m_hInst = hInstance;
	InitFilePath[0] = TEXT('\0');
}

XEditor::~XEditor() {}

void XEditor::SetInitFilePath(TCHAR *path)
{
	int length = lstrlen(path);
	for (int i = 0; i < length; i++)
	{
		InitFilePath[i] = path[i];
	}
	InitFilePath[length] = TEXT('\0');
}

void XEditor::SetInitFilePath(CHAR *path)
{
	int iFileLength = strlen(path);


	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)path, -1, (PTSTR)InitFilePath,
		iFileLength + 1);

	InitFilePath[iFileLength] = TEXT('\0');
}

bool XEditor::CreateEX(DWORD dwExStyle, LPCTSTR lpszClass, LPCTSTR lpszName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hParent, HMENU hMenu, HINSTANCE hInst)
{
	m_hInst = hInst;
	bool ret = XWnd::CreateEx(dwExStyle, lpszClass, lpszName, dwStyle,
		x, y, nWidth, nHeight, hParent, hMenu, hInst);
	m_hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	return ret;
}

WPARAM XEditor::MessageLoop()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (m_findDialog.m_hWnd == 0 || !(::IsDialogMessage(m_findDialog.m_hWnd, &msg)))
		{
			if (!TranslateAccelerator(m_hWnd, m_hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	return msg.wParam;
}


LRESULT XEditor::OnCreate()
{
	RECT  rect;
	XFile myFile;
	GetClientRect(m_hWnd, &rect);                     // ��ȡ�ͻ�����С
	m_edit.CreateEx(0, TEXT("EDIT_CLASS"), NULL,      // ���� Edit�ؼ�
		WS_VISIBLE | WS_CHILD, 0, 0,
		rect.right, rect.bottom, m_hWnd, NULL, m_hInst);


	/*::SetClassLong(m_edit.m_hWnd, GCL_HCURSOR, (LONG)LoadCursor(m_hInst, MAKEINTRESOURCE(IDC_CURSOR1)));*/

	if (InitFilePath[0] != TEXT('\0'))
	{
		if (!myFile.FileRead(m_edit.m_hWnd, InitFilePath))
		{
			InitFilePath[0] = TEXT('\0');
			MessageBox(m_hWnd, TEXT("�򿪳����ļ������ڣ�"), TEXT("��"), NULL);
			::SendMessage(m_hWnd, WM_DESTROY, NULL, NULL);
		}
	}

	// ��������
	if (InitFilePath[0] != TEXT('\0'))      // ���ļ�ʱ��ʾ�ļ���
	{
		TCHAR appName[255];
		::LoadString(m_hInst, IDS_APP_NAME, appName, 255);
		TCHAR title[MAX_PATH + 10];
		wsprintf(title, TEXT("%s - %s"), InitFilePath, appName);
		::SetWindowText(m_hWnd, title);
	}
	else                                    // û���ļ�ʱʹ��default����
	{
		TCHAR filePath[MAX_PATH];
		::LoadString(m_hInst, IDS_FILE_NAME_DEFAULT, filePath, MAX_PATH);
		TCHAR appName[255];
		::LoadString(m_hInst, IDS_APP_NAME, appName, 255);
		TCHAR title[MAX_PATH + 10];
		wsprintf(title, TEXT("%s - %s"), filePath, appName);
		::SetWindowText(m_hWnd, title);
	}

	//m_findDialog.Create(m_hInst, IDD_DIALOG_SEARCH, m_hWnd);  // ��������/�滻����

	return 0;
}


void XEditor::OnSize(int width, int height)
{
	MoveWindow(m_edit.m_hWnd, 0, 0, width, height, FALSE);
}

void XEditor::OnDraw(HDC hDC)
{

}

void XEditor::OnDragFiles(WPARAM wParam, LPARAM lParam)
{
	TCHAR szFileName[MAX_PATH];

	//   POINT pt;  
	//   ::DragQueryPoint((HDROP) wParam, &pt); 
	int   fileCount = ::DragQueryFile((HDROP)wParam, 0xFFFFFFFF, (LPTSTR)NULL, 0);
	if (fileCount > 1)
	{
		::MessageBox(m_hWnd, TEXT("��֧��һ�δ�һ���ļ�"), TEXT("��ʾ"), NULL);
		return;
	}
	else
	{
		::DragQueryFile((HDROP)wParam, 0, &szFileName[0], 512);
	}

	XFile myFile;
	if (!myFile.FileRead(m_edit.m_hWnd, szFileName))
	{
		szFileName[0] = TEXT('\0');
		MessageBox(m_hWnd, TEXT("�򿪳����ļ������ڣ�"), TEXT("��"), NULL);
		return;
	}
	wsprintf(InitFilePath, TEXT("%s"), szFileName);

	// ��������
	if (InitFilePath[0] != TEXT('\0'))      // ���ļ�ʱ��ʾ�ļ���
	{
		TCHAR appName[255];
		::LoadString(m_hInst, IDS_APP_NAME, appName, 255);
		TCHAR title[MAX_PATH + 10];
		wsprintf(title, TEXT("%s - %s"), InitFilePath, appName);
		::SetWindowText(m_hWnd, title);
	}
}

void XEditor::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case IDM_FILE_NEW:
	{
		//       ::SendMessage(m_edit.m_hWnd, WM_SETTEXT, NULL, (LPARAM)(TEXT("\0")));
		InitFilePath[0] = TEXT('\0');
		::SetWindowText(m_edit.m_hWnd, (PTSTR)(TEXT("\0")));;
		// ��������
		if (InitFilePath[0] != TEXT('\0'))      // ���ļ�ʱ��ʾ�ļ���
		{
			TCHAR appName[255];
			::LoadString(m_hInst, IDS_APP_NAME, appName, 255);
			TCHAR title[MAX_PATH + 10];
			wsprintf(title, TEXT("%s - %s"), InitFilePath, appName);
			::SetWindowText(m_hWnd, title);
		}
		else                                    // û���ļ�ʱʹ��default����
		{
			TCHAR filePath[MAX_PATH];
			::LoadString(m_hInst, IDS_FILE_NAME_DEFAULT, filePath, MAX_PATH);
			TCHAR appName[255];
			::LoadString(m_hInst, IDS_APP_NAME, appName, 255);
			TCHAR title[MAX_PATH + 10];
			wsprintf(title, TEXT("%s - %s"), filePath, appName);
			::SetWindowText(m_hWnd, title);
		}
		return;
	}
	case IDM_FILE_OPEN:
	{
		static XFile  myFile;
		static TCHAR  szFileName[MAX_PATH], szTitleName[MAX_PATH];
		myFile.Initialize(m_hWnd);
		if (myFile.FileOpenDlg(szFileName, szTitleName))
		{
			if (!myFile.FileRead(m_edit.m_hWnd, szFileName))
			{
				szFileName[0] = TEXT('\0');
				szTitleName[0] = TEXT('\0');
				MessageBox(m_hWnd, TEXT("�򿪳����ļ������ڣ�"), TEXT("��"), NULL);
			}
			wsprintf(InitFilePath, TEXT("%s"), szFileName);
		}
		else
		{
			//         MessageBox(m_hWnd, L"ȡ����", L"��ʾ", NULL);
			return;
		}
		// ��������
		if (InitFilePath[0] != TEXT('\0'))      // ���ļ�ʱ��ʾ�ļ���
		{
			TCHAR appName[255];
			::LoadString(m_hInst, IDS_APP_NAME, appName, 255);
			TCHAR title[MAX_PATH + 10];
			wsprintf(title, TEXT("%s - %s"), InitFilePath, appName);
			::SetWindowText(m_hWnd, title);
		}
		else                                    // û���ļ�ʱʹ��default����
		{
			TCHAR filePath[MAX_PATH];
			::LoadString(m_hInst, IDS_FILE_NAME_DEFAULT, filePath, MAX_PATH);
			TCHAR appName[255];
			::LoadString(m_hInst, IDS_APP_NAME, appName, 255);
			TCHAR title[MAX_PATH + 10];
			wsprintf(title, TEXT("%s - %s"), filePath, appName);
			::SetWindowText(m_hWnd, title);
		}
		return;
	}
	case IDM_FILE_SAVE:
	{
		static XFile  myFile;
		static TCHAR  szFileName[MAX_PATH], szTitleName[MAX_PATH];
		myFile.Initialize(m_hWnd);
		if (InitFilePath[0] != TEXT('\0'))
		{
			m_edit.article.Format();   // ����������Ӧ�������У��ȴ�д�����
			myFile.FileSave(m_edit.article.pBufSave, m_edit.article.GetSize(), InitFilePath);
		}
		else
		{
			if (myFile.FileSaveDlg(szFileName, szTitleName))
			{
				m_edit.article.Format();   // ����������Ӧ�������У��ȴ�д�����
				if (!myFile.FileSave(m_edit.article.pBufSave, m_edit.article.GetSize(), szFileName))
				{
					szFileName[0] = TEXT('\0');
					szTitleName[0] = TEXT('\0');
					MessageBox(m_hWnd, TEXT("�������"), TEXT("����"), NULL);
				}
				wsprintf(InitFilePath, TEXT("%s\0"), szFileName);
			}
			else
			{
				return;
			}
		}
		// ��������
		if (InitFilePath[0] != TEXT('\0'))      // ���ļ�ʱ��ʾ�ļ���
		{
			TCHAR appName[255];
			::LoadString(m_hInst, IDS_APP_NAME, appName, 255);
			TCHAR title[MAX_PATH + 10];
			wsprintf(title, TEXT("%s - %s"), InitFilePath, appName);
			::SetWindowText(m_hWnd, title);
		}
		else                                    // û���ļ�ʱʹ��default����
		{
			TCHAR filePath[MAX_PATH];
			::LoadString(m_hInst, IDS_FILE_NAME_DEFAULT, filePath, MAX_PATH);
			TCHAR appName[255];
			::LoadString(m_hInst, IDS_APP_NAME, appName, 255);
			TCHAR title[MAX_PATH + 10];
			wsprintf(title, TEXT("%s - %s"), filePath, appName);
			::SetWindowText(m_hWnd, title);
		}
		return;
	}
	case IDM_FILE_SAVE_ANOTHER:
	{
		//       MessageBox(m_hWnd, TEXT("���Ϊ"), TEXT("��ʾ"), NULL);
		static XFile  myFile;
		static TCHAR  szFileName[MAX_PATH], szTitleName[MAX_PATH];
		myFile.Initialize(m_hWnd);
		if (myFile.FileSaveDlg(szFileName, szTitleName))
		{
			m_edit.article.Format();   // ����������Ӧ�������У��ȴ�д�����
			if (!myFile.FileSave(m_edit.article.pBufSave, m_edit.article.GetSize(), szFileName))
			{
				szFileName[0] = TEXT('\0');
				szTitleName[0] = TEXT('\0');
				MessageBox(m_hWnd, TEXT("�������"), TEXT("����"), NULL);
			}
			wsprintf(InitFilePath, TEXT("%s\0"), szFileName);
		}
		else
		{
			return;
		}
		// ��������
		if (InitFilePath[0] != TEXT('\0'))      // ���ļ�ʱ��ʾ�ļ���
		{
			TCHAR appName[255];
			::LoadString(m_hInst, IDS_APP_NAME, appName, 255);
			TCHAR title[MAX_PATH + 10];
			wsprintf(title, TEXT("%s - %s"), InitFilePath, appName);
			::SetWindowText(m_hWnd, title);
		}
		else                                    // û���ļ�ʱʹ��default����
		{
			TCHAR filePath[MAX_PATH];
			::LoadString(m_hInst, IDS_FILE_NAME_DEFAULT, filePath, MAX_PATH);
			TCHAR appName[255];
			::LoadString(m_hInst, IDS_APP_NAME, appName, 255);
			TCHAR title[MAX_PATH + 10];
			wsprintf(title, TEXT("%s - %s"), filePath, appName);
			::SetWindowText(m_hWnd, title);
		}
		return;
	}
	case IDM_QUIT:
	{
		::SendMessage(m_hWnd, WM_CLOSE, NULL, NULL);
		return;
	}
	case IDM_EDIT_UNDO:
	{
		//       MessageBox(m_hWnd, TEXT("������δʵ�֣�"), TEXT("��ʾ"), NULL);
		::SendMessage(m_edit.m_hWnd, WM_COMMAND, IDM_EDIT_UNDO, NULL);
		return;
	}
	case IDM_EDIT_REDO:
	{
		//       MessageBox(m_hWnd, TEXT("�ָ���δʵ�֣�"), TEXT("��ʾ"), NULL);
		::SendMessage(m_edit.m_hWnd, WM_COMMAND, IDM_EDIT_REDO, NULL);
		return;
	}
	case IDM_EDIT_ALL:
	{
		//       MessageBox(m_hWnd, TEXT("ȫѡ"), TEXT("��ʾ"), NULL);
		::SendMessage(m_edit.m_hWnd, WM_COMMAND, IDM_EDIT_ALL, NULL);
		return;
	}
	case IDM_EDIT_COPY:
	{
		::SendMessage(m_edit.m_hWnd, WM_COMMAND, IDM_EDIT_COPY, NULL);
		return;
	}
	case IDM_EDIT_CUT:
	{
		::SendMessage(m_edit.m_hWnd, WM_COMMAND, IDM_EDIT_CUT, NULL);
		return;
	}
	case IDM_EDIT_PASTE:
	{
		::SendMessage(m_edit.m_hWnd, WM_COMMAND, IDM_EDIT_PASTE, NULL);
		return;
	}
	case IDM_SEARCH:
	{
		//       CFindDlg findDialog;
		////       dialog.Create(m_hInst, IDD_DIALOG_SEARCH, m_hWnd);
		//      dialog.DoModal(m_hInst, MAKEINTRESOURCE(IDD_DIALOG_SEARCH), m_hWnd);
		////       DialogBox(m_hInst, MAKEINTRESOURCE(IDD_DIALOG_SEARCH), m_hWnd, StrSearch);
		m_findDialog.ShowWindow();
		return;
	}
	case IDM_REPLACE:
	{
		//      CFindDlg dialog;
		////       dialog.Create(m_hInst, IDD_DIALOG_SEARCH, m_hWnd);
		//      dialog.DoModal(m_hInst, MAKEINTRESOURCE(IDD_DIALOG_SEARCH), m_hWnd);
		////       DialogBox(m_hInst, MAKEINTRESOURCE(IDD_DIALOG_SEARCH), m_hWnd, StrSearch);
		m_findDialog.ShowWindow();
		return;
	}
	case IDM_APP_ABOUT:
		//       CAboutDlg aboutDlg;
		//       aboutDlg.Create(m_hInst, IDD_ABOUT, m_hWnd);
		m_aboutDlg.DoModal(m_hInst, MAKEINTRESOURCE(IDD_ABOUT), m_hWnd);
		//       DialogBox(m_hInst, MAKEINTRESOURCE(IDD_ABOUT), m_hWnd, this->AboutDlgProc);
		return;
	}
}

LRESULT XEditor::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		m_hWnd = hWnd;
		::DragAcceptFiles(hWnd, TRUE);
		return OnCreate();

	case WM_DROPFILES:
		OnDragFiles(wParam, lParam);
		return 0;

	case WM_SIZE:
		OnSize(LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_SETFOCUS:
		SetFocus(m_edit.m_hWnd);
		return 0;

	case WM_FIND:
		//       MessageBox(hWnd, (TCHAR *)lParam, TEXT("Find"), NULL);
		::SendMessage(m_edit.m_hWnd, uMsg, wParam, lParam);
		//       SetFocus(m_findDialog.m_hWnd);
		return 0;

	case WM_COMMAND:
		OnCommand(wParam, lParam);
		return 0;

	case WM_KEYDOWN:
		::SendMessage(m_edit.m_hWnd, uMsg, wParam, lParam);
		return 0;

	case WM_CHAR:
		::SendMessage(m_edit.m_hWnd, uMsg, wParam, lParam);
		return 0;

	case WM_CLOSE:
	{
		int quitFlag = MessageBox(hWnd, TEXT("�Ƿ񱣴��ļ�"), TEXT("��ʾ"), MB_YESNOCANCEL | MB_ICONQUESTION);
		if (quitFlag == IDYES)
		{
			::SendMessage(hWnd, WM_COMMAND, IDM_FILE_SAVE, NULL);
		}
		else if (quitFlag == IDNO)
		{
			// ����Ĭ�ϴ��ں��������������ٴ��ڲ��˳�����
		}
		else if (quitFlag == IDCANCEL)
		{
			return 0;   // ȡ���˳�
		}
	}
	case WM_DESTROY:
		::DragAcceptFiles(hWnd, FALSE);
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}