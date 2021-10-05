// wrappit.cpp : Defines the entry point for the console application.
// 1.1


#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>
using namespace std;

struct EXPORT_ITEM
	{
	bool IsOnlyOrdinal;
	char in[300];
	char en[300];
	int o;
	int h;
	};

vector<EXPORT_ITEM> v;

//#pragma warning(disable : 4996)

int main(int argc, char* argv[])
{
	/*unsigned long regs[12];
	__asm mov regs+0,eax;
	__asm mov regs+4,ebx;
	__asm mov regs+8,ecx;
	__asm mov regs+12,edx;
	__asm mov regs+16,esi;
	__asm mov regs+20,edi;
	__asm mov regs+24,ebp;
	__asm mov regs+28,esp;
	regs[0]+=1;
	regs[1]+=1;
	regs[2]+=1;
	argc=regs[5];*/

	printf("ProxyDll Maker v1.0. Copyright (C) Tansoft\r\n");

	if (argc < 2)
	{
		printf("Usage: ProxyDll dll.dll <convention>\r\n");
		printf("Make sure the vc environment vcvars32.bat in path setting\r\n");
		printf("the Program default behavior:\r\n");
		printf("  copy dll.dll to dll_.dll\r\n");
		printf("  dump dll_.dll's export table to dll_.txt with dumpbin\r\n");
		printf("  generate dll.cpp and dll.def and project files use __stdcall convention by default\r\n");
		printf("  make dll.cpp to generate dll.dll\r\n");
		system("pause");
		return 0;
	}
	char conversion[64];
	char target[1024];
	char prefix[1024];
	char realdll[1024];
	char dumptxt[1024];
	char cpp[1024];
	char def[1024];
	char bat[1024];
	char del[1024];
	char cmd[1024];
	strcpy(prefix,argv[1]);
	if (strrchr(prefix,'\\')!=NULL)
	{
		char *end=strrchr(prefix,'\\');
		*end='\0';
		SetCurrentDirectory(prefix);
		strcpy(prefix,end+1);
	}
	strcpy(target,prefix);
	prefix[strlen(prefix)-4]='\0';
	strcpy(realdll,prefix);strcat(realdll,"_.dll");
	strcpy(dumptxt,prefix);strcat(dumptxt,".txt");
	strcpy(cpp,prefix);strcat(cpp,".cpp");
	strcpy(def,prefix);strcat(def,".def");
	strcpy(bat,prefix);strcat(bat,"_make.bat");
	strcpy(del,prefix);strcat(del,"_del.bat");
	if (argc>=3)
		strcpy(conversion,argv[2]);
	else
		strcpy(conversion,"__stdcall");

	system("vcvars32.bat");
	sprintf(cmd,"copy /y /b %s %s",argv[1],realdll);
	system(cmd);
	sprintf(cmd,"dumpbin /exports %s > %s",realdll,dumptxt);
	system(cmd);

	_tprintf(_T("Parsing %s...\r\n"),dumptxt);
	FILE* fp = _tfopen(dumptxt,_T("rb"));
	if (!fp)
		{
		printf("Error: Txt file not found.\r\n");
		return 0;
		}
	int MSStart = 0;
	for(int i = 0 ;; )
		{
		char x[1000] = {0};
		EXPORT_ITEM e = {0};
		if (!fgets(x,1000,fp))
			break;

		if (strnicmp(x,"EXPORT ord",10) == 0) // tdump
			{
			//EXPORT ord:1141='AcceptEx' => 'MSWSOCK.AcceptEx'
			e.o = atoi(x + 11);
			sprintf(e.in,"__E__%u__",i);
			char* y = strchr(x,'\'');
			if (y)
				{
				y++;
				char* y2 = strchr(y,'\'');
				if (y2)
					*y2 = 0;
				strcpy(e.en,y);
				e.IsOnlyOrdinal = false;
				}
			else
				{
				e.IsOnlyOrdinal = true;
				sprintf(e.en,"___XXX___%u",i);
				}
			v.insert(v.end(),e);
			}
		else
			{
			if (strstr(x,"ordinal") != 0 && strstr(x,"hint") != 0 && strstr(x,"RVA") != 0)
				{
				MSStart = 1;
				continue;
				}
			if (!MSStart)
				continue;
			char* a1 = x;
			while(*a1 == ' ')
				a1++;
			if (*a1 == '\r' || *a1 == '\n')
				{
				if (MSStart == 1)
					{
					MSStart = 2;
					continue;
					}
				break;
				}
			e.o = atoi(a1);
			while(*a1 != ' ')
				a1++;
			while(*a1 == ' ')
				a1++;
			if (*a1 == '\r' || *a1 == '\n')
				{
				if (MSStart == 1)
					{
					MSStart = 2;
					continue;
					}
				break;
				}
			e.h = atoi(a1);
			while(*a1 != ' ')
				a1++;
			while(*a1 == ' ')
				a1++;
			if (*a1 == '\r' || *a1 == '\n')
				{
				if (MSStart == 1)
					{
					MSStart = 2;
					continue;
					}
				break;
				}
			if (*a1 >= 0x30 && *a1 <= 0x39) // RVA exists
				{
				while(*a1 != ' ')
					a1++;
				while(*a1 == ' ')
					a1++;
				if (*a1 == '\r' || *a1 == '\n')
					break;
				}

			sprintf(e.in,"__E__%u__",i++);
			e.IsOnlyOrdinal = false;
			if (strnicmp(a1,"[NONAME]",8) == 0)
				{
				e.IsOnlyOrdinal = true;
				sprintf(e.en,"___XXX___%u",i);
				}
			else
				{
				for(int y = 0 ; ; y++)
					{
					if (*a1 == ' ' || *a1 == '\r' || *a1 == '\n' || *a1 == '\t')
						break;
					e.en[y] = *a1++;
					}
				}
			v.insert(v.end(),e);
			}
		}
	fclose(fp);
	_tprintf(_T("%u exported functions parsed.\r\n"),v.size());
	_tprintf(_T("Generating .DEF file %s...\r\n"),def);
	FILE* fdef = _tfopen(def,_T("wb"));
	if (!fdef)
		{
		printf("Error: DEF file cannot be created.\r\n");
		return 0;
		}
	fprintf(fdef,"EXPORTS\r\n");
	for(unsigned int i = 0 ; i < v.size () ; i++)
		{
		if (v[i].IsOnlyOrdinal == false)
			fprintf(fdef,"%s=%s @%u\r\n",v[i].en,v[i].in,v[i].o);
		else
			fprintf(fdef,"%s=%s @%u NONAME\r\n",v[i].en,v[i].in,v[i].o);
		}
	fclose(fdef);
	_tprintf(_T("%u exported functions written to DEF.\r\n"),v.size());
	_tprintf(_T("Generating .CPP file %s...\r\n"),cpp);
	FILE* fcpp = _tfopen(cpp,_T("wb"));
	if (!fcpp)
		{
		printf("Error: CPP file cannot be created.\r\n");
		return 0;
		}

	// Write headers
	fprintf(fcpp,"#include <windows.h>\r\n");
	fprintf(fcpp,"#include <stdio.h>\r\n");
	fprintf(fcpp,"#pragma pack(1)\r\n\r\n");

/*	fprintf(fcpp,"#define PUSHARGS	{\\\r\n");
	fprintf(fcpp,"\t__asm push eax\\\r\n");
	fprintf(fcpp,"\t__asm push ebx\\\r\n");
	fprintf(fcpp,"\t__asm push ecx\\\r\n");
	fprintf(fcpp,"\t__asm push edx\\\r\n");
	fprintf(fcpp,"\t__asm push esi\\\r\n");
	fprintf(fcpp,"\t__asm push edi}\r\n\r\n");

	fprintf(fcpp,"#define POPARGS	{\\\r\n");
	fprintf(fcpp,"\t__asm pop edi\\\r\n");
	fprintf(fcpp,"\t__asm pop esi\\\r\n");
	fprintf(fcpp,"\t__asm pop edx\\\r\n");
	fprintf(fcpp,"\t__asm pop ecx\\\r\n");
	fprintf(fcpp,"\t__asm pop ebx\\\r\n");
	fprintf(fcpp,"\t__asm pop eax}\r\n\r\n");
*/
	fprintf(fcpp,"#define PUSHARGS	{__asm pushad}\r\n\r\n");
	fprintf(fcpp,"#define POPARGS	{__asm popad}\r\n\r\n");

	fprintf(fcpp,"#define PRINTARGS(x)	\\\r\n");
	fprintf(fcpp,"\tchar outputbuf[1024];\\\r\n");
	//fprintf(fcpp,"\t__asm {\\\r\nmov regs+0,eax\\\r\n");
	//fprintf(fcpp,"\tmov regs+4,ebx\\\r\n");
	//fprintf(fcpp,"\tmov regs+8,ecx\\\r\n");
	//fprintf(fcpp,"\tmov regs+12,edx\\\r\n}\\\r\n");
	//fprintf(fcpp,"\t__asm mov regs+16,esi;\\\r\n");
	//fprintf(fcpp,"\t__asm mov regs+20,edi;\\\r\n");
	//fprintf(fcpp,"\t__asm mov regs+24,ebp;\\\r\n");
	//fprintf(fcpp,"\t__asm mov regs+28,esp;\r\n");
	/*fprintf(fcpp,"\t__asm mov regs[8],dword ptr [esp+4]\\\r\n");
	fprintf(fcpp,"\t__asm mov regs[9],dword ptr [esp+8]\\\r\n");
	fprintf(fcpp,"\t__asm mov regs[10],dword ptr [esp+12]\\\r\n");
	fprintf(fcpp,"\t__asm mov regs[11],dword ptr [esp+16]\r\n");*/
	fprintf(fcpp,"\tsprintf(outputbuf,\"fn:%%s eax:%%08x ebx:%%08x ecx:%%08x edx:%%08x \"\\\r\n"
				"\t\t\"esi:%%08x edi:%%08x ebp:%%08x \"\\\r\n"
				"\t\t\"esp:%%08x esp+4:%%08x esp+8:%%08x esp+12:%%08x esp+16:%%08x\"\\\r\n"
				"\t\t,x,regs[0],regs[1],regs[2],regs[3],regs[4],regs[5],regs[6],regs[7]\\\r\n"
				"\t\t,regs[8],regs[9],regs[10],regs[11]);\\\r\n"
				"\tOutputDebugString(outputbuf);\r\n\r\n");

	// Write variables
	fprintf(fcpp,"HINSTANCE hLThis = 0;\r\n");
	fprintf(fcpp,"HINSTANCE hL = 0;\r\n");
	fprintf(fcpp,"FARPROC p[%u] = {0};\r\n\r\n",v.size());

	//Write Functions
	fprintf(fcpp,"FARPROC MyGetProcAddress(HMODULE hModule,LPCSTR lpProcName)\r\n");
	fprintf(fcpp,"{\r\n");
	fprintf(fcpp,"\tFARPROC ret=GetProcAddress(hModule,lpProcName);\r\n");
	fprintf(fcpp,"\tFARPROC ret1=GetProcAddress(hLThis,lpProcName);\r\n");
	fprintf(fcpp,"\tchar outputbuf[1024];\r\n");
	fprintf(fcpp,"\tsprintf(outputbuf,\"Function '%%s' Loaded in Address:0x%%x -> 0x%%x\\r\\n\",lpProcName,ret1,ret);\r\n");
	fprintf(fcpp,"\tOutputDebugString(outputbuf);\r\n");
	fprintf(fcpp,"\treturn ret;\r\n");
	fprintf(fcpp,"}\r\n\r\n");

	// Write DllMain
	fprintf(fcpp,"BOOL WINAPI DllMain(HINSTANCE hInst,DWORD reason,LPVOID)\r\n");
	fprintf(fcpp,"{\r\n");
	fprintf(fcpp,"\tif (reason == DLL_PROCESS_ATTACH)\r\n");
	fprintf(fcpp,"\t{\r\n");

	fprintf(fcpp,_T("\t\thLThis = hInst;\r\n"),realdll);
	fprintf(fcpp,_T("\t\thL = LoadLibrary(\"%s\");\r\n"),realdll);
	fprintf(fcpp,"\t\tif (!hL) return false;\r\n");

	fprintf(fcpp,"\r\n");
	for(unsigned int i = 0 ; i < v.size () ; i++)
		{
		if (v[i].IsOnlyOrdinal == true)
			fprintf(fcpp,"\t\tp[%u] = MyGetProcAddress(hL,(LPCSTR)\"%u\");\r\n",i,v[i].o);
		else
			fprintf(fcpp,"\t\tp[%u] = MyGetProcAddress(hL,\"%s\");\r\n",i,v[i].en);
		}

	fprintf(fcpp,"\t}\r\n");

	fprintf(fcpp,"\tif (reason == DLL_PROCESS_DETACH)\r\n");
	fprintf(fcpp,"\t{\r\n");
	fprintf(fcpp,_T("\t\tFreeLibrary(hL);\r\n"));
	fprintf(fcpp,"\t}\r\n");


	fprintf(fcpp,"\treturn 1;\r\n");
	fprintf(fcpp,"}\r\n\r\n");


	// Write function to be exported
	for(unsigned int i = 0 ; i < v.size() ; i++)
	{
		fprintf(fcpp,"// %s\r\nextern \"C\" __declspec(naked) void %s %s()\r\n",v[i].en,conversion,v[i].in);
		fprintf(fcpp,"{\r\n");
		fprintf(fcpp,"\tunsigned long regs;\r\n");
		fprintf(fcpp,"\t__asm {\r\n");
		fprintf(fcpp,"\t\tpushad\r\n");
		fprintf(fcpp,"\t\tmov regs,eax\r\n");
		//fprintf(fcpp,"\t\tmov regs+4,ebx\r\n");	
		//fprintf(fcpp,"\t\tmov regs+8,ecx\r\n");	
		//fprintf(fcpp,"\t\tmov regs+12,edx\r\n");	
		fprintf(fcpp,"\t}\r\n");
		//fprintf(fcpp,"\tPUSHARGS;\r\n");
		//fprintf(fcpp,"\tPRINTARGS(\"%s\");\r\n",v[i].en);
		//fprintf(fcpp,"\tPRINTARGS(\"%s\");\r\n",v[i].en);
		//fprintf(fcpp,"\tPOPARGS;\r\n");
		fprintf(fcpp,"\t__asm\r\n");
		fprintf(fcpp,"\t{\r\n");
		fprintf(fcpp,"\t\tpopad\r\n");
		fprintf(fcpp,"\t\tjmp p[%u*%u]\r\n",i,sizeof(void*));
		fprintf(fcpp,"\t}\r\n");
		fprintf(fcpp,"}\r\n\r\n");
	}
	fprintf(fcpp,"/*\r\n\t//If you know the function how to work,use this type proxy:");
	fprintf(fcpp,"\r\n\t//extern \"C\" __declspec(naked) void %s __E__69__()",conversion);
	fprintf(fcpp,"\r\n\textern \"C\" int %s __E__69__(SOCKET x,char* b,int l,int pr)",conversion);
	fprintf(fcpp,"\r\n\t{");
	fprintf(fcpp,"\r\n\t\ttypedef int (%s *pS)(SOCKET,char*,int,int);",conversion);
	fprintf(fcpp,"\r\n\t\tpS pps = (pS)p[63*4];");
	fprintf(fcpp,"\r\n\t\tint rv = pps(x,b,l,pr);");
	fprintf(fcpp,"\r\n\t\treturn rv;");
	fprintf(fcpp,"\r\n\t}");
	fprintf(fcpp,"\r\n*/");
	fclose(fcpp);
	FILE* fbat = _tfopen(bat,_T("wb"));
	if (!fbat)
		{
		printf("Error: bat file cannot be created.\r\n");
		return 0;
		}

//	_stprintf(ay,_T("BCC32 -o%s.obj -c %s\r\n"),argv[5],argv[5]);
	fprintf(fbat,"@call vcvars32.bat\r\n");
	fprintf(fbat,"@CL.EXE /O2 /I \".\" /D \"WIN32\" /D \"NDEBUG\" /D \"_WINDOWS\" /D \"_WINDLL\" /FD /EHsc /MT /Fo\".\\%s.obj\" /Fd\".\\vc80.pdb\" /W3 /nologo /c /TP %s\r\n",prefix,cpp);
//	_stprintf(ay,_T("ILINK32 -c -Tpd %s.obj,%s,,,%s\r\n"),argv[5],argv[1],argv[6]);
	fprintf(fbat,"@LINK.EXE /OUT:\"%s\" /INCREMENTAL:NO /NOLOGO /DLL /DEF:\"%s\" /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /MACHINE:X86 %s.obj kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib\r\n",target,def,prefix);
	fprintf(fbat,"@pause\r\n");
	fclose(fbat);
	system(bat);
//
	FILE* fdel = _tfopen(del,_T("wb"));
	if (!fdel)
		{
		printf("Error: del_bat file cannot be created.\r\n");
		return 0;
		}
	fprintf(fdel,"@del %s\r\n",target);
	fprintf(fdel,"@ren %s %s\r\n",realdll,target);
	fprintf(fdel,"@del %s\r\n",dumptxt);
	fprintf(fdel,"@del %s\r\n",cpp);
	fprintf(fdel,"@del %s\r\n",def);
	fprintf(fdel,"@del %s\r\n",bat);
	fprintf(fdel,"@del %s.obj\r\n",prefix);
	fprintf(fdel,"@del %s.lib\r\n",prefix);
	fprintf(fdel,"@del %s.exp\r\n",prefix);
	fprintf(fdel,"@del vc80.idb\r\n",prefix);
	fprintf(fdel,"@del %s\r\n",del);
	
	fclose(fdel);
	return 0;
}

