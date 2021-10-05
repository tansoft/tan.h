# Tan.H
多年整理的，非常实用的Windows工具类库

# 使用方法

* 增加目录到系统include目录中
* 打开 prj/Tan.h.prj, 进行几个版本的编译

# 文档
建议使用doc目录下的doxgen，进行生成使用

# 常用用法
* 动画组件可以使用：THSkinButton，THSkinStatic，THSkinButtonStatic。
* 数据库数据显示可以使用：THDBQueryCtrls 里的 THDBQueryXXX 系列组件。
* Flash控件交互对话框可以使用：THFlashDlg。

# 目录结构
* BaseCtrls: 一些界面组件
* Dialogs: 一些对话框组件
* DBBrowser: 一个SqlServer查询的例子
* DataMaker: 二进制打印数据的例子
* FlashReport: FusionCharts报表
* HardWare: 硬件操作类

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
