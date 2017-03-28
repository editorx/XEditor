#ifndef _DEBUG_H_
#define _DEBUG_H_

/*
  class DEBUG       // 用于在Win32窗体编程时，创建控制台窗口辅助调试(Release模式下不启用)
*/

#ifndef _USE_OLD_IOSTREAMS
using namespace std;
#endif

#ifdef _DEBUG
// class DEBUG Begin
class DEBUG
{
public:
  static void RedirectIOToConsole();    // 创建控制台窗口并执行IO重定向
//   static void test();                   // 测试输入输出(查看重定向是否成功)
private:
  // maximum mumber of lines the output console should have? 
  static const WORD MAX_CONSOLE_LINES = 500;
};
// class DEBUG End
#endif // _DEBUG

#endif
