# Tan.H
多年整理的，非常实用的Windows工具类库

# 使用方法

* 增加目录到系统include目录中
* 解压lib下的库文件，增加lib目录到系统lib目录中
* 打开 prj/Tan.h.prj, 进行需要版本的编译（Debug，Release等）

# 文档
建议使用doc目录下的doxgen，进行生成使用

# 常用用法
* 动画组件可以使用：THSkinButton，THSkinStatic，THSkinButtonStatic。
* 数据库数据显示可以使用：THDBQueryCtrls 里的 THDBQueryXXX 系列组件。
* Flash控件交互对话框可以使用：THFlashDlg。
* 硬件相关操作，包括读卡器，显示屏，打印机等使用Hardware里的类。

# 实用例子
* HttpServer: 提供强大的HttpServer例子
* MiscTools: 各种杂项操作演示
* LiveUpdateDll: 通用在线更新例子
* PopupDlgDll: 通用弹窗例子
* DBBrowser: 一个SqlServer查询的例子
* DataMaker: 二进制打印数据的例子
* ProxyDll: 进程插入例子
* RegExpDll: 正则表达式例子
* RemoteHookDll: 远程钩子例子
* SimpleShare: 局域网共享例子
* THScriptDebugger: THScript调试器

# 目录结构
* BaseCtrls: 一些界面组件
* Dialogs: 一些对话框组件
* FlashReport: FusionCharts报表
* HardWare: 硬件操作类，包括读卡器，显示屏，打印机等
* MsgHookDll: 鼠标，键盘钩子处理
* ResizableLib: Resizable控件封装
* SkinPlusPlus: Skin++控件封装
* SqliteDll: Sqlite封装
* aeslib,gzlib,md5lib,nettlelib,opencvlib,openssl,vldlib,zlib: 常用算法实现的封装，统一编译到THArch
* doc: doxgen相关配置，可生成说明文档
* flash: flash相关配置，提供flash控件的皮肤类skinloader，以及所见即所得编辑器wysiwygeditor
* lib: 提供预编译好的环境。

# 常用类

THBase.h
 各种常规类型定义

THStruct.h
 数据结构类，Map，List，Pool

THSyn.h
 锁，事件

THMath.h
 THArithmetic
 THRegExp

THEncode.h
 THTiniEncode
 THDes
 THAes

THHash.h
 THMd5
 THSha

THZip.h
 THGZip
 THLzma

THString.h:
 字符串处理类

THThread.h:
 多线程类
 THThreadPool

THIni.h
 THIni
 THTiniIni
 THTiniXml

THDataFile.h
 文件数据库类

THDataBase.h
 数据库封装

THHttp.h
 http协议封装

THNetWork.h
 THRawNetWorkLayer

THCsModel
 THServer
 THClient
 THPacketBuffer
 C/S模型

THBuf.h
 THMemBuf 缓冲由块组成,读出大块时需要另起缓冲
 THTiniBuf 缓冲由固定大小块组成,new时使用基本大小倍数,读出时可直接引用

THSystem.h
 系统封装
 THFile
 THTime
 THDebug
 所有系统函数声明
 SysXX

ui:
 THButton
 THStatic
 THStaticButton
 THAniStatic
 THSkinPPW
 THBitmap
 THWebCtrl

win:
 THWinApp

app:
 THLiveupdate
