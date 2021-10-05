#pragma once

/*
生成工程对应的Map文件方法：
C/C++ -> General -> Debug Information Format -> Line Numbers Only (/Zd)
Linker -> Debugging:
 Generate Map File -> Yes (/MAP)
 Map File Name -> PrjName.Map
 Map Lines -> Yes (/MAPINFO:LINES)
如果是dll工程：
 Map Exports -> Yes (/MAPINFO:EXPORTS)
*/
