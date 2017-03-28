#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include "XWnd.h"
#include "XDialog.h"
#include "XAboutDlg.h"

BOOL XAboutDlg::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INITDIALOG:
    return TRUE;
  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
    case IDCANCEL:
      EndDialog(hDlg, 0);
      return TRUE;
    }
    break;
  }
  return FALSE;
}
