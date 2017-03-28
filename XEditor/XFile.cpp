#include <Windows.h>
#include <stdio.h>
#include "XFile.h"

XFile::XFile(HWND hwnd) : hwnd(hwnd)
{
	this->Initialize(hwnd);
}

XFile::~XFile() {}

void XFile::Initialize(HWND hwnd)
{
	static TCHAR szFilter[] = 
		TEXT("C/C++ Files (*.c;*.cpp;*.cc;*.cxx;*.h;*.hpp;*hxx)\0*.c;*.cpp;*.cc;*.cxx;*.h;*.hpp;*hxx\0")  \
		TEXT("Text Files (*.txt)\0*.txt\0") \
		TEXT("All Files (*.*)\0*.*\0\0");

	this->hwnd = hwnd;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = NULL;          // Set in Open and Close functions
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;          // Set in Open and Close functions
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = 0;             // Set in Open and Close functions
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = TEXT("txt");
	ofn.lCustData = 0L;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
}

BOOL XFile::FileOpenDlg(PTSTR pstrFileName, PTSTR pstrTitleName)
{
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = pstrTitleName;
	ofn.lpstrFileTitle = pstrTitleName;
	ofn.Flags = OFN_HIDEREADONLY;

	return GetOpenFileName(&ofn);
}

BOOL XFile::FileSaveDlg(PTSTR pstrFileName, PTSTR pstrTitleName)
{
	static TCHAR szFilter[] =
		TEXT("C/C++ Files (*.cpp;*.c;*.cc;*.cxx;*.h;*.hpp;*hxx)\0*.cpp;*.c;*.cc;*.cxx;*.h;*.hpp;*hxx\0")  \
		TEXT("Text Files (*.txt)\0*.txt\0") \
		TEXT("All Files (*.*)\0*.*\0\0");

	ZeroMemory(&ofn, sizeof(ofn));

	TCHAR filename[MAX_PATH] = { 0 };
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrDefExt = TEXT("txt");
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
	ofn.FlagsEx = OFN_EX_NOPLACESBAR;
	ofn.lStructSize = sizeof(OPENFILENAME);  // No OFN_ENABLEHOOK
	ofn.hwndOwner = hwnd;
	return GetSaveFileName(&ofn);
}

BOOL XFile::FileRead(HWND hwndEdit, PTSTR pstrFileName)
{
	BYTE bySwap;
	DWORD dwBytesRead;
	HANDLE hFile;
	int i, iFileLength, iUniTest;
	PBYTE pBuffer, pText, pConv;

	if (INVALID_HANDLE_VALUE ==
		(hFile = CreateFile(pstrFileName, GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, 0, NULL)))
	{
		return FALSE;
	}

	// 获取文件长度，并分配内存(多分配2字节用于字符串结束符)
	iFileLength = GetFileSize(hFile, NULL);
	pBuffer = (BYTE *)malloc(iFileLength + 2);

	ReadFile(hFile, pBuffer, iFileLength, &dwBytesRead, NULL);
	CloseHandle(hFile);
	pBuffer[iFileLength] = '\0';
	pBuffer[iFileLength + 1] = '\0';

	iUniTest = IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE;

	if (IsTextUnicode(pBuffer, iFileLength, &iUniTest))
	{
		pText = pBuffer + 2;
		iFileLength -= 2;
		if (iUniTest & IS_TEXT_UNICODE_REVERSE_SIGNATURE)
		{
			for (i = 0; i < iFileLength / 2; ++i)
			{
				bySwap = ((BYTE *)pText)[2 * i];
				((BYTE *)pText)[2 * i] = ((BYTE *)pText)[2 * i + 1];
				((BYTE *)pText)[2 * i + 1] = bySwap;
			}
		}

		pConv = (BYTE *)malloc(iFileLength + 2);
#ifndef UNICODE
		WideCharToMultiByte(CP_ACP, 0, (PWSTR)pText, -1, pConv,
			iFileLength + 2, NULL, NULL);
#else 
		lstrcpy((PTSTR)pConv, (PTSTR)pText);
#endif 
	}
	else
	{
		pText = pBuffer;
		pConv = (BYTE *)malloc(2 * iFileLength + 2);
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pText, -1, (PTSTR)pConv,
			iFileLength + 1);
#else
		lstrcpy((PTSTR)pConv, (PTSTR)pText);
#endif
	}
	SetWindowText(hwndEdit, (PTSTR)pConv);
	free(pBuffer);
	free(pConv);

	return TRUE;
}

BOOL XFile::FileSave(TCHAR *pBuf, int len, PTSTR pstrFileName)
{
	HANDLE hFile;

	if (INVALID_HANDLE_VALUE ==
		(hFile = CreateFile(pstrFileName, GENERIC_WRITE, 0,
			NULL, OPEN_ALWAYS, NULL, NULL)))
	{
		return FALSE;
	}

	DWORD num;
	WORD unicodeFlag = 0xFEFF;
	WriteFile(hFile, &unicodeFlag, sizeof(WORD), &num, NULL);
	WriteFile(hFile, pBuf, len * sizeof(TCHAR), &num, NULL);
	SetEndOfFile(hFile);
	CloseHandle(hFile);
	return TRUE;
}