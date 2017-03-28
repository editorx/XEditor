#ifndef _X_FIND_DLG_H 
#define _X_FIND_DLG_H

#define  WM_FIND      (WM_USER + 10)
#define  WM_REPLACE   (WM_USER + 11)

enum FIND_PARAM
{
	FP_CASE_SENSITIVE = 0x0001,                 // 区分大小写
	FP_WHOLE_WORD = 0x0002,                     // 全词匹配
	FP_BEGIN_OF_FILE = 0x0004,                  // 从文件头开始
	FP_REPLACE_NOTIFY = 0x0008,                 // 替换时提示
	FP_FIND_NEXT = 0x0010,                      // 查找下一个(向后查找)
	FP_FIND_PREV = 0x0020,                      // 查找上一个(向前查找)
	FP_REPLACE_ALL = 0x0040,                    // 全部替换
	FP_REPLACE = 0x0080                         // 替换选中项
};

class XFindDlg : public XDialog
{
	virtual BOOL DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
