#pragma once

//Mfc Ctrls Lib include
/*_MFC_STATIC 定义不再使用
使用_AFXDLL直接判断
*/
#ifndef _AFXDLL
 #ifdef _UNICODE
  #ifdef _DEBUG
   #pragma comment(lib,"MfcCtrlsSUD.lib")
   #pragma message("link Mfc Ctrls with setting static unicode debug")
  #else
   #pragma comment(lib,"MfcCtrlsSUR.lib")
   #pragma message("link Mfc Ctrls with setting static unicode release")
  #endif
 #else
  #ifdef _DEBUG
   #pragma comment(lib,"MfcCtrlsSD.lib")
   #pragma message("link Mfc Ctrls with setting static debug")
  #else
   #pragma comment(lib,"MfcCtrlsSR.lib")
   #pragma message("link Mfc Ctrls with setting static release")
  #endif
 #endif
#else
 #ifdef _UNICODE
  #ifdef _DEBUG
   #pragma comment(lib,"MfcCtrlsDUD.lib")
   #pragma message("link Mfc Ctrls with setting dyn unicode debug")
  #else
   #pragma comment(lib,"MfcCtrlsDUR.lib")
   #pragma message("link Mfc Ctrls with setting dyn unicode release")
  #endif
 #else
  #ifdef _DEBUG
   #pragma comment(lib,"MfcCtrlsDD.lib")
   #pragma message("link Mfc Ctrls with setting dyn debug")
  #else
   #pragma comment(lib,"MfcCtrlsDR.lib")
   #pragma message("link Mfc Ctrls with setting dyn release")
  #endif
 #endif
#endif
