#ifndef _X_FIND_DLG_H 
#define _X_FIND_DLG_H

#define  WM_FIND      (WM_USER + 10)
#define  WM_REPLACE   (WM_USER + 11)

enum FIND_PARAM
{
	FP_CASE_SENSITIVE = 0x0001,                 // ���ִ�Сд
	FP_WHOLE_WORD = 0x0002,                     // ȫ��ƥ��
	FP_BEGIN_OF_FILE = 0x0004,                  // ���ļ�ͷ��ʼ
	FP_REPLACE_NOTIFY = 0x0008,                 // �滻ʱ��ʾ
	FP_FIND_NEXT = 0x0010,                      // ������һ��(������)
	FP_FIND_PREV = 0x0020,                      // ������һ��(��ǰ����)
	FP_REPLACE_ALL = 0x0040,                    // ȫ���滻
	FP_REPLACE = 0x0080                         // �滻ѡ����
};

class XFindDlg : public XDialog
{
	virtual BOOL DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
