#pragma once

//Mfc Ctrls Lib include
/*_MFC_STATIC 定义不再使用
使用_AFXDLL直接判断
*/
#ifndef _AFXDLL
 #ifdef _DEBUG
  #pragma comment(lib,"arithsd.lib")
  #pragma message("link arith lib with setting static debug")
 #else
  #pragma comment(lib,"arithsr.lib")
  #pragma message("link arith lib with setting static release")
 #endif
#else
 #ifdef _DEBUG
  #pragma comment(lib,"arithdd.lib")
  #pragma message("link arith lib with setting dyn debug")
 #else
  #pragma comment(lib,"arithdr.lib")
  #pragma message("link arith lib with setting dyn release")
 #endif
#endif