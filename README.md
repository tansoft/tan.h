# Tan.H
多年整理的，非常实用的Windows工具类库

# 使用方法

* 增加目录到系统include目录中
* 解压lib下的库文件，增加lib目录到系统lib目录中
* 打开 prj/tan.sln, 进行需要版本的编译，尤其是Arith是各种算法库的预编译（Debug，Release等）

# 文档
建议使用doc目录下的doxgen，进行生成使用

# 常用用法

## 界面
* 动画组件可以使用：THSkinButton，THSkinStatic，THSkinButtonStatic。
* 对话框可以使用：THBaseDialog，THBasePassDialog，THBaseUserPassDialog，THBaseInputDialog，THBaseWaitingDialog，THBaseMulSelectDialog，THBaseTreeDialog。
* 对话框组件：THDlgItemXXX
* 对话框位置控制：THResizeDialogRect
* 自适应对话框：THResizeDlg
* Html控件：THWebCtrl
* 数据库数据显示可以使用：THDBQueryCtrls 里的 THDBQueryXXX 系列组件。
* 图片处理：THBitmap，THMemDC
* 自动更新：THLiveUpdate，THLiveUpdateDll

## Flash相关
* THFlashDlg：控件交互对话框；
* THAmfParser：Flash命令解释。
* THFlashBase：Flash处理基类。

## 算法
* THArith：各种算法封装（THMd5，THBase64，THGzip，THCrc，。。。）
* THBitParser：位处理
* THAes：Aes
* THSha：Sha
* THNettleLib：nettlelib的算法。
* THMeasure：度量单位转换。
* THSpeedCounting：计数器

## 文字处理、内存处理
* 常用类型定义：THBase
* 常用数据结构：THStruct
* 编码处理：THCharset
* 字符串：THString
* 调试：THDebug
* Guid：THGuid
* Json处理：THJsonParser
* 四则运算：THLogicCalc
* 内存缓冲管理类：THMemBuf，内存管理：THMenManager，内存检查：THMemCheck，对象池管理：THObjectBuffer
 * THMemBuf 缓冲由块组成,读出大块时需要另起缓冲
 * THTiniBuf 缓冲由固定大小块组成,new时使用基本大小倍数,读出时可直接引用
* 正则表达式：THRegExpDll
* 脚本引擎：THScript
* stl库扩展：THStlEx

## 文件处理
* 常规文件：THFile
* 文件块：THBlockFile，内存文件缓存：THMemCache
* 文件设备封装：THFileStore，可把资源文件，镜像文件，ftp文件等进行文件系统映射
* 自动关闭文件：THTimeoutFile
* 文件变更监听：THFileWatcher
* 可执行文件版本信息：THFileVersionInfo
* 可执行文件图标管理：THSystemIconList
* Icon文件处理：THIconParser
* Ini文件处理：THIni
* 快捷方式文件处理：THLinkFile
* Flv文件处理：THFlvParser
* Mp4文件处理：THMp4Parser
* Swf文件处理：THSwfParser
* 内存映射：THMapFile
* 压缩文件： THGZip，THLzma
* Xml：THTiniXml

## 进程处理
* 进程管理：THCmdProcess
* 线程管理：THThread，线程池：THThreadPool
* 系统类库：THSystem，THSysMisc，THSysCallRapper
* 锁，事件等：THSyn
* 类库加载：THDllLoader
* 崩溃信息：THExceptionHandler
* IE管理：THIeHandler，THRemoteIeAccess，THIeHtmlElementParser
* 插件管理：THPlugin
* 注册表管理：THReg
* 单一进程：THSingleProcess
* WinApp基类：THWinApp
* Skin++基类：THSkinWinApp
* 系统栏图标：THTrayIcon
* 系统时间：THTime
* Vista DWM：THVistaDwm

## 网络处理
* 网络处理：THNetWork
* Socket封装：THRawSocket
* 广播：THRawBoardcastSocket
* 客户端连接管理：THRawSocketClients
* C/S模型： THServer，THClient，THPacketBuffer
* 封包管理：THSimplePacket
* 授权管理：THSoftwareAuth
* ssl：THSsl
* url处理：THUrlExplain
* http头处理：THHttpHeaderParser，THHttpHeaderResponseMaker，THHttpHeaderRequestMaker
* cookie处理：THHttpHeaderCookieParser

## 协议实现
* http代理：THHttpProxyServer
* http服务器：THHttpServer
* 简单文件共享：THSimpleShare
* rtmp协议处理：THRtmpParser
* inet库封装：THINetXXX
* smtp：THSmtpClient
* socket代理：THSocketProxyServer
* 对时协议：THTimeClient
* Telnet：THTelnetServer
* Whois：THWhoisClient

## 数据库管理
* 数据库：THDataBase
* 查询：THDBQuery
* ODBC：THDBOdbc
* 数据显示可以使用：THDBQueryXXX 系列组件
* Sqlite：THSqliteDll

## 硬件
* 硬件相关操作，包括读卡器，显示屏，打印机等使用Hardware里的类。
* Modem处理：THModem
* 摄像头：THOpencvCamera，opencv常用操作：THOpencvXXX
* Bios：THPhysicalBiosAccess
* 硬盘：THPhysicalHardDiskAccess
* CPU：THPhysicalCpuAccess
* 串口：THSerialPort
* 系统信息：THSysInfo

# 实用例子
* HttpServer: 提供强大的HttpServer例子
* prj/tan.sln: 提供几个项目的例子
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
* sniffercap: 进行网络抓包分析例子

# 目录结构
* BaseCtrls: 一些界面组件
* Dialogs: 一些对话框组件
* FlashReport: FusionCharts报表
* HardWare: 硬件操作类，包括读卡器，显示屏，打印机等
* MsgHookDll: 鼠标，键盘钩子处理
* ResizableLib: Resizable控件封装
* SkinPlusPlus: Skin++控件封装
* SqliteDll: Sqlite封装
* aeslib,gzlib,md5lib,nettlelib,opencvlib,openssl,zlib: 常用算法实现的封装，统一编译到THArch
* doc: doxgen相关配置，可生成说明文档
* flash: flash相关配置，提供flash控件的皮肤类skinloader，以及所见即所得编辑器wysiwygeditor
* lib: 提供预编译好的环境。
* misc: 一些资料
