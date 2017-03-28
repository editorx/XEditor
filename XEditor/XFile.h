#ifndef _X_FILE_H 
#define _X_FILE_H

class XFile
{
private:
	HWND hwnd;
	OPENFILENAME ofn;
public:
	XFile() {}
	XFile(HWND hwnd);
	~XFile();

	void Initialize(HWND hwnd);

	BOOL FileOpenDlg(PTSTR pstrFileName, PTSTR pstrTitleName);
	BOOL FileRead(HWND hwndEdit, PTSTR pstrFileName);
	BOOL FileSaveDlg(PTSTR pstrFileName, PTSTR pstrTitleName);
	BOOL FileSave(TCHAR *pBuf, int len, PTSTR pstrFileName);

};
#endif
