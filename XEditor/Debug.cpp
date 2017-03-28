#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <conio.h>
#include "Debug.h"

#ifdef _DEBUG
void DEBUG::RedirectIOToConsole()
{
  int hConHandle;
  long lStdHandle;
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE *fp;

  // 分配一个控制台程序? 
  AllocConsole();

  // 设置足够大的屏幕缓存可以让我们滑动文本
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

  // 设置控制台屏幕的高度
  coninfo.dwSize.Y = MAX_CONSOLE_LINES;

  // 设置控制台屏幕缓存大小
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
    coninfo.dwSize);

  // 获取标准输出句柄
  lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);

  //打开标准输出句柄，类似打开文件的方式如fopen,返回一个文件描述符
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

  // 以可写的方式打开
  fp = _fdopen(hConHandle, "w");

  *stdout = *fp;

  setvbuf(stdout, NULL, _IONBF, 0);

  // redirect unbuffered STDIN to the console? 
  lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

  fp = _fdopen(hConHandle, "r");
  *stdin = *fp;
  setvbuf(stdin, NULL, _IONBF, 0);

  // redirect unbuffered STDERR to the console? 
  lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

  fp = _fdopen(hConHandle, "w");
  *stderr = *fp;
  setvbuf(stderr, NULL, _IONBF, 0);

  // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog? 
  // point to console as well? 
  ios::sync_with_stdio();
}

#endif // _DEBUG
