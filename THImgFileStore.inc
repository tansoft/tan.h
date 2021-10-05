
#define IMGIDX_ROOT		1			///<<<当img文件没有任何数据时，返回该索引值
#define RECNAMEFBLEN	35			///<<<Record块首块名字长度
#define RECNAMESBLEN	60			///<<<Record块其他块名字长度

#pragma pack(push,1)
/*
映像文件结构：
THIMGFILE_HEADER

THIMGFILE_BLOCK{
	nextblock
	0
	blocklen
}
blockdata

THIMGFILE_BLOCK{
	0
	0
	fullblocklen
}
recorddata{
	THIMGFILE_RECORD
	string
	THIMGFILE_RECORD
	string
}

文件组织形式：
recordoff -> 根目录第一个文件 -> suboff -> 文件数据
             | nextoff
			 v
             根目录第一个目录 -> suboff -> 子目录 -> suboff -> 子目录的子目录
             | nextoff                     |
			 v                             v
             跟目录下一个文件              下一个子目录

block 默认是32KB，是磁盘中的最小分块，一个文件由多个块组成，由THIMGFILE_BLOCK->nextblock 链向下一个块
目录结构由THIMGFILE_HEADER->recordoff开始，作为根目录指向，目录结构储存直接开辟整个block来使用，在block里以record为单位保存目录结构数据，一个record长64字节。
block一旦分配作为record后，整个block只作record使用，空余的record全部进THIMGFILE_HEADER->recordfree
*/
typedef struct _THIMGFILE_HEADER{
	U8 magic[5];			//<<<映像文件标识THIMG
	U8 ver;					//<<<映像文件版本,低4位为版本号，高4位为属性，0x80为gzip版本，0x40为加密版本
	U8 reserved[2];			//<<<保存字
	U16 blocksize;			//<<<每个区块的大小，单位KB，默认32KB
	U16 blockcount;			//<<<区块数，整个文件的大小=sizeof(THIMGFILE_HEADER)+(blocksize+sizeof(THIMGFILE_BLOCK))*blockcount
	U32 freeblockoff;		//<<<空闲区块链表
	U32 recordoff;			//<<<根记录块链表
	U32 freerecordoff;		//<<<空闲记录块链表
}THIMGFILE_HEADER;

typedef struct _THIMGFILE_BLOCK{
	U32 nextblockoff;		//<<<下一区块的偏移
	U32 prevblockoff;		//<<<上一区块的偏移
	U32 blocklen;			//<<<该block实际数据使用长度
}THIMGFILE_BLOCK;

typedef struct _THIMGFILE_RECORD{
	U32 createdate;			//<<<创建时间
	U32 modifydate;			//<<<修改时间
	U32 accessdate;			//<<<访问时间
	U32 datasize;			//<<<大小
	U32 suboff;				//<<<如果是目录，是子目录THIMGFILE_RECORD；如果是文件，是文件保存的偏移THIMGFILE_BLOCK，注意偏移可能是跨区块的
	U32 nextoff;			//<<<同目录下一个THIMGFILE_RECORD，注意偏移可能是跨区块的
	U32 nameoff;			//<<<如果默认的RECNAMEFBLEN字节空间不能保存名字，则使用下一空间来保存名字，下一空间开头的U32说明是否还有再下一级空间
	U8 attrib;				//<<<0x1，是否目录；0x2，是否只读；
	U8 name[RECNAMEFBLEN];	//<<<默认RECNAMEFBLEN长度的名字
}THIMGFILE_RECORD;
#pragma pack(pop)

/**
* @brief 数据文件储存器描述类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-09-07 新建类
*/
/*
	SetBaseDirectory(_T("res://c:\\a.exe/img/web.img"));
	SetBaseDirectory(_T("c:\\img\\web.img"));
*/
class THImgFileStore : public ITHFileStore
{
public:
	THImgFileStore():ITHFileStore()
	{
		m_store=NULL;
		m_imgbuf=NULL;
		m_imglen=0;
		m_bDirty=FALSE;
		m_bZip=FALSE;
	}
	virtual ~THImgFileStore()
	{
		_FreeIntStore();
	}
	virtual ITHFileStore *CreateObject()
	{
		return new THImgFileStore();
	}

	virtual void SetBaseDirectory(THString path)
	{
		ITHFileStore::SetBaseDirectory(path);
		_FreeIntStore();
		if (path.Find(_T("res://"),0)==0)
		{
			int pos=path.Find(_T("/"),7);
			if (pos!=-1)
			{
				m_store=new THResFileStore();
				m_store->SetBaseDirectory(path.Left(pos).Mid(6));
				m_ImgFileName=path.Mid(pos+1);
			}	
		}
		else
		{
			m_store=new THFileStore();
			m_ImgFileName=path;
		}
		if (m_store)
		{
			BOOL bOk=FALSE;
			UINT imglen;
			void *imgbuf=m_store->ReadWholeFile(m_ImgFileName,&imglen);
			if (imgbuf && imglen>sizeof(THIMGFILE_HEADER))
			{
				THIMGFILE_HEADER *h=(THIMGFILE_HEADER *)imgbuf;
				U8 *curbuf=(U8 *)imgbuf;
				U32 curlen=imglen;
				if (memcmp(h->magic,"THIMG",5)==0)
				{
					if ((h->ver&0x0F)==1)
					{
						if (h->ver&0x40)
						{
							//encrpyted
							int retlen;
							THAes aes;
							void *buf=aes.DecryptBuffer(curbuf+sizeof(THIMGFILE_HEADER),curlen-sizeof(THIMGFILE_HEADER),&retlen,m_sAuth);
							if (buf)
							{
								retlen+=sizeof(THIMGFILE_HEADER);
								U8 *tmpbuf=new U8[retlen];
								if (tmpbuf)
								{
									memcpy(tmpbuf,curbuf,sizeof(THIMGFILE_HEADER));
									memcpy(tmpbuf+sizeof(THIMGFILE_HEADER),buf,retlen-sizeof(THIMGFILE_HEADER));
									curlen=retlen;
									curbuf=tmpbuf;
								}
								else
									bOk=FALSE;
								aes.Free(buf);
							}
							else
								curlen=0;
						}
						if (curlen>0 && h->ver&0x80)
						{
							//ziped ,unzip
							m_bZip=TRUE;
							THMemBuf buf;
							if (THGZip::ZipDecode2(curbuf+sizeof(THIMGFILE_HEADER),curlen-sizeof(THIMGFILE_HEADER),&buf))
							{
								imglen=buf.GetBufferedSize()+sizeof(THIMGFILE_HEADER);
								U8 *tmpbuf=new U8[imglen];
								if (tmpbuf)
								{
									bOk=TRUE;
									memcpy(tmpbuf,curbuf,sizeof(THIMGFILE_HEADER));
									memcpy(tmpbuf+sizeof(THIMGFILE_HEADER),buf.GetBuf(NULL,FALSE),imglen-sizeof(THIMGFILE_HEADER));
									curlen=imglen;
									if (curbuf!=imgbuf)
										delete [] curbuf;
									curbuf=tmpbuf;
								}
								else
									curlen=0;
							}
							else
								curlen=0;
						}
						if (curlen>0)
						{
							if (curbuf==imgbuf)
							{
								m_imgbuf=new U8[imglen];
								if (m_imgbuf)
								{
									bOk=TRUE;
									memcpy(m_imgbuf,imgbuf,imglen);
									m_imglen=imglen;
								}
							}
							else
							{
								bOk=TRUE;
								m_imgbuf=curbuf;
								m_imglen=curlen;
							}
						}
					}
				}
				m_store->FreeBuf(imgbuf);
			}
			if (!bOk)
			{
				NewImg();
			}
		}
	}

	//zipmode压缩模式:no不压缩，gzip进行gzip压缩
	//authmode授权模式:no无授权，pass+使用密码
	virtual BOOL SetStoreAttrib(THString key,THString value)
	{
		if (key==_T("zipmode"))
		{
			if (value==_T("no"))
				m_bZip=FALSE;
			else if (value==_T("gzip"))
				m_bZip=TRUE;
			else
				return FALSE;
		}
		else if (key==_T("authmode"))
		{
			if (value==_T("no"))
				m_sAuth.Empty();
			else if (value.Left(4)==_T("pass"))
				m_sAuth=value.Mid(4);
			else
				return FALSE;
		}
		else
			return FALSE;
		m_bDirty=TRUE;
		return TRUE;
	}

	virtual THString GetStoreAttrib(THString key)
	{
		if (key==_T("zipmode"))
		{
			return m_bZip?_T("gzip"):_T("no");
		}
		else if (key==_T("authmode"))
		{
			return m_sAuth.IsEmpty()?_T("no"):(_T("pass")+m_sAuth);
		}
		else if (key==_T("storestate"))
		{
			U32 state[20];
			THString ret;
			GetImgState(state,_T("\\"),ret);
			return ret;
		}
		return _T("");
	}

	//InitSize*BlockSize=总容量，64*32=32k一block的2MB的磁盘文件
	virtual BOOL NewImg(U16 InitCount=64,U16 BlockSize=32)
	{
		_FreeImg();
		m_imglen=sizeof(THIMGFILE_HEADER)+(BlockSize*1024+sizeof(THIMGFILE_BLOCK))*InitCount;
		m_imgbuf=new U8[m_imglen];
		if (!m_imgbuf) return FALSE;
		THIMGFILE_HEADER *h=(THIMGFILE_HEADER *)m_imgbuf;
		memcpy(h->magic,"THIMG",5);
		h->ver=1;
		memset(h->reserved,0,2);
		h->blocksize=ntohs(BlockSize);
		h->blockcount=ntohs(InitCount);
		h->recordoff=0;
		h->freerecordoff=0;
		int bl=BlockSize*1024+sizeof(THIMGFILE_BLOCK);
		U32 off=sizeof(THIMGFILE_HEADER);
		h->freeblockoff=htonl(off);
		U8 *write=m_imgbuf+off;
		for(int i=0;i<InitCount;i++)
		{
			off+=bl;
			if (i==InitCount-1) off=0;
			*(U32 *)write=htonl(off);//THIMGFILE_BLOCK
			*(U32 *)(write+sizeof(U32))=0;
			*(U32 *)(write+sizeof(U32)*2)=0;
			write+=bl;
		}
		m_bDirty=TRUE;
		return TRUE;
	}

	//增加的是Block数，ReAllocCount=0为默认增加当前容量的一倍，如果已超过最高容量则到最高容量
	virtual BOOL ReAllocImg(U16 ReAllocCount=0)
	{
		if (!m_imgbuf) return NewImg();
		THIMGFILE_HEADER *h=(THIMGFILE_HEADER *)m_imgbuf;
		U16 bc=ntohs(h->blockcount);
		U16 bs=ntohs(h->blocksize);
		if (ReAllocCount==0)
		{
			ReAllocCount=0xFFFF-bc;
			if (ReAllocCount>bc) ReAllocCount=bc;
		}
		if (0xFFFF-bc<ReAllocCount) return FALSE;
		bc+=ReAllocCount;
		UINT imglen=sizeof(THIMGFILE_HEADER)+(bs*1024+sizeof(THIMGFILE_BLOCK))*bc;
		U8 *tmpbuf=new U8[imglen];
		if (!tmpbuf) return FALSE;
		memcpy(tmpbuf,m_imgbuf,m_imglen);
		_FreeImg();
		m_imgbuf=tmpbuf;
		m_imglen=imglen;
		h=(THIMGFILE_HEADER *)m_imgbuf;
		h->blockcount=htons(bc);
		int bl=bs*1024+sizeof(THIMGFILE_BLOCK);
		U32 lastfree=ntohl(h->freeblockoff);
		U32 off=sizeof(THIMGFILE_HEADER)+bl*(bc-ReAllocCount);
		h->freeblockoff=htonl(off);
		U8 *write=m_imgbuf+off;
		for(int i=0;i<ReAllocCount;i++)
		{
			off+=bl;
			if (i==ReAllocCount-1) off=lastfree;
			*(U32 *)write=htonl(off);//THIMGFILE_BLOCK
			*(U32 *)(write+sizeof(U32))=0;
			*(U32 *)(write+sizeof(U32)*2)=0;
			write+=bl;
		}
		m_bDirty=TRUE;
		return TRUE;
	}

	virtual BOOL FindFiles(THString sPath,THStringArray *arDir,THStringArray *arFile)
	{
		if (!CheckImg()) return FALSE;
		U32 nextidx=_GetPathIndex(sPath);
		if (nextidx==0) return FALSE;
		nextidx=_GetSubIndex(nextidx);
		if (nextidx==0) return TRUE;
		while(nextidx!=0)
		{
			THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+nextidx);
			THStringArray *ar;
			if (r->attrib&0x1) ar=arDir;
			else ar=arFile;
			if (ar) ar->Add(_GetFileName(r));
			nextidx=ntohl(r->nextoff);
		}
		return TRUE;
	}

	virtual BOOL GetFileAttrib(THString sFile,UINT *size,THTime *ct,THTime *mt)
	{
		if (!CheckImg()) return FALSE;
		U32 nextidx=_GetPathIndex(sFile);
		if (nextidx==IMGIDX_ROOT || nextidx==0) return FALSE;
		THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+nextidx);
		//目录也允许返回
		//if (r->attrib&0x1) return FALSE;
		if (size) *size=ntohl(r->datasize);
		if (ct) ct->SetTime((time_t)ntohl(r->createdate),FALSE);
		if (mt) mt->SetTime((time_t)ntohl(r->modifydate),FALSE);
		return TRUE;
	}

	virtual UINT GetFileSize(int idx)
	{
		if (idx==0 || !CheckImg()) return 0;
		THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+idx);
		return ntohl(r->datasize);
	}

	virtual THString GetStoreName()
	{
		return _T("映像文件");
	}

	virtual THString FileDialogOpenExistOne(HWND hOwner=NULL,THString sFileName=_T(""),THString sFilter=_T("所有文件(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{//fixme
	return THSysDialog::FileDialogOpenExistOne(hOwner,sFileName,sFilter,sExt,sInitDir,sTitle);}

	virtual THString FileDialogSaveOne(HWND hOwner=NULL,THString sFileName=_T("未命名"),THString sFilter=_T("所有文件(*.*)|*.*||"),THString sExt=_T(""),THString sInitDir=_T(""),THString sTitle=_T(""))
	{//fixme
	return THSysDialog::FileDialogSaveOne(hOwner,sFileName,sFilter,sExt,sInitDir,sTitle);}

	virtual THString DirDialog(HWND hWnd=NULL,THString sTitle=_T("请选择目录"))
	{//fixme
	return THSysDialog::DirDialog(hWnd,sTitle);}

	virtual THString BaseDirDialog(HWND hWnd=NULL,THString sTitle=_T("请选择映像文件"))
	{return THSysDialog::FileDialogOpenExistOne(hWnd,_T(""),_T("映像文件(*.*)|*.*||"),_T(""),_T(""),sTitle);}

#define TESTASSERT(x)	if (!(x)) {ASSERT(FALSE);return FALSE;}
#define CHECKSTATE(x)	{((THImgFileStore *)s)->GetImgState(state,_T("\\"),ret);TESTASSERT(ret==x);}
	static BOOL TestCase()
	{
		THImgFileStore file;
		ITHFileStore *s=file.CreateObject();
		::DeleteFile(_T("c:\\test.img"));
		s->SetBaseDirectory(_T("c:\\test.img"));
		U32 state[20];
		THString ret;
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:64,ImgFileSize:2097944 B,RecordCount:1,Dirs:0,Files:0,FreeBlock:64,FreeRecord:0,FileSize:0 B,UsedSize:0 KB,FreeSize:2048 KB"));
		//test create dir findfiles
		THStringArray d,f;
		TESTASSERT(s->FindFiles(_T("\\"),&d,&f));
		TESTASSERT(d.GetSize()==0 && f.GetSize()==0);
		TESTASSERT(s->CreateDirectory(_T("abc")));
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:64,ImgFileSize:2097944 B,RecordCount:2,Dirs:1,Files:0,FreeBlock:63,FreeRecord:511,FileSize:0 B,UsedSize:0 KB,FreeSize:2016 KB"));
		TESTASSERT(s->CreateDirectory(_T("test\\test2")));
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:64,ImgFileSize:2097944 B,RecordCount:4,Dirs:3,Files:0,FreeBlock:63,FreeRecord:509,FileSize:0 B,UsedSize:0 KB,FreeSize:2016 KB"));
		TESTASSERT(s->FindFiles(_T("\\"),&d,&f));
		TESTASSERT(d.GetSize()==2 && f.GetSize()==0);
		TESTASSERT(d[0]==_T("abc") && d[1]==_T("test"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("\\test"),&d,&f));
		TESTASSERT(d.GetSize()==1 && f.GetSize()==0);
		TESTASSERT(d[0]==_T("test2"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("test"),&d,&f));
		TESTASSERT(d.GetSize()==1 && f.GetSize()==0);
		TESTASSERT(d[0]==_T("test2"));
		TESTASSERT(s->CreateDirectory(_T("test3\\test4")));
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:64,ImgFileSize:2097944 B,RecordCount:6,Dirs:5,Files:0,FreeBlock:63,FreeRecord:507,FileSize:0 B,UsedSize:0 KB,FreeSize:2016 KB"));
		s->DeleteDirectory(_T("test"));
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:64,ImgFileSize:2097944 B,RecordCount:4,Dirs:3,Files:0,FreeBlock:63,FreeRecord:509,FileSize:0 B,UsedSize:0 KB,FreeSize:2016 KB"));
		//test file oper
		TESTASSERT(s->OpenFile(_T("test3\\test"))==-1);
		int idx=s->OpenFile(_T("test3\\test4.file"),FILEMODE_CREATEOREXIST);
		TESTASSERT(idx!=-1);
		char buf[4096];
		TESTASSERT(s->ReadFile(idx,buf,5)==0);
		TESTASSERT(s->WriteFile(idx,"testdata",8)==8);
		s->CloseFile(idx);
		idx=s->OpenFile(_T("test3\\test4.file"),FILEMODE_CREATEOREXIST);
		TESTASSERT(idx!=-1);
		TESTASSERT(s->SeekFile(idx,SEEK_SET,3)==3);
		TESTASSERT(s->SeekFile(idx,SEEK_END,-6)==2);
		TESTASSERT(s->GetPosition(idx)==2);
		TESTASSERT(s->ReadFile(idx,buf,5)==5);
		TESTASSERT(s->GetPosition(idx)==7);
		TESTASSERT(memcmp(buf,"stdat",5)==0);
		TESTASSERT(s->WriteFile(idx,"testtest",8)==8);
		TESTASSERT(s->GetPosition(idx)==15);
		TESTASSERT(s->GetFileSize(idx)==15);
		TESTASSERT(s->SeekFile(idx,SEEK_CUR,-1)==14);
		TESTASSERT(s->ReadFile(idx,buf,5)==1);
		TESTASSERT(memcmp(buf,"t",1)==0);
		s->CloseFile(idx);
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:64,ImgFileSize:2097944 B,RecordCount:5,Dirs:3,Files:1,FreeBlock:62,FreeRecord:508,FileSize:15 B,UsedSize:32 KB,FreeSize:1984 KB"));
		//test move file
		TESTASSERT(s->MoveFile(_T("test3\\test4.file"),_T("abc\\test4.file")));
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:64,ImgFileSize:2097944 B,RecordCount:5,Dirs:3,Files:1,FreeBlock:62,FreeRecord:508,FileSize:15 B,UsedSize:32 KB,FreeSize:1984 KB"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("\\test3"),&d,&f));
		TESTASSERT(d.GetSize()==1 && f.GetSize()==0);
		TESTASSERT(d[0]==_T("test4"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("\\abc"),&d,&f));
		TESTASSERT(d.GetSize()==0 && f.GetSize()==1);
		TESTASSERT(f[0]==_T("test4.file"));
		//test delete file
		s->DeleteFile(_T("abc\\test4.file"));
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:64,ImgFileSize:2097944 B,RecordCount:4,Dirs:3,Files:0,FreeBlock:63,FreeRecord:509,FileSize:0 B,UsedSize:0 KB,FreeSize:2016 KB"));
		//test realloc
		TESTASSERT(((THImgFileStore *)s)->ReAllocImg());
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:128,ImgFileSize:4195864 B,RecordCount:4,Dirs:3,Files:0,FreeBlock:127,FreeRecord:509,FileSize:0 B,UsedSize:0 KB,FreeSize:4064 KB"));
		//test copyfile
		TESTASSERT(s->WriteWholeFile(_T("abc\\test3.file"),"test",4));
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:128,ImgFileSize:4195864 B,RecordCount:5,Dirs:3,Files:1,FreeBlock:126,FreeRecord:508,FileSize:4 B,UsedSize:32 KB,FreeSize:4032 KB"));
		TESTASSERT(s->CopyFile(_T("abc\\test3.file"),_T("\\test3.file")));
		TESTASSERT(s->CopyFile(_T("abc\\test3.file"),_T("test3\\test3.file")));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("\\"),&d,&f));
		TESTASSERT(d.GetSize()==2);
		TESTASSERT(d[0]==_T("abc") && d[1]==_T("test3"));
		TESTASSERT(f.GetSize()==1);
		TESTASSERT(f[0]==_T("test3.file"));		
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:128,ImgFileSize:4195864 B,RecordCount:7,Dirs:3,Files:3,FreeBlock:124,FreeRecord:506,FileSize:12 B,UsedSize:96 KB,FreeSize:3968 KB"));
		//test move dir
		TESTASSERT(s->MoveFile(_T("test3"),_T("abc\\test3")));
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:128,ImgFileSize:4195864 B,RecordCount:7,Dirs:3,Files:3,FreeBlock:124,FreeRecord:506,FileSize:12 B,UsedSize:96 KB,FreeSize:3968 KB"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("\\"),&d,&f));
		TESTASSERT(d.GetSize()==1 && f.GetSize()==1);
		TESTASSERT(d[0]==_T("abc") && f[0]==_T("test3.file"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("abc"),&d,&f));
		TESTASSERT(d.GetSize()==1 && f.GetSize()==1);
		TESTASSERT(d[0]==_T("test3"));
		TESTASSERT(f[0]==_T("test3.file"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("abc\\test3"),&d,&f));
		TESTASSERT(d.GetSize()==1 && f.GetSize()==1);
		TESTASSERT(d[0]==_T("test4"));
		TESTASSERT(f[0]==_T("test3.file"));
		//test copy dir
		TESTASSERT(s->CopyFile(_T("abc\\test3"),_T("abc\\test4")));
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:128,ImgFileSize:4195864 B,RecordCount:10,Dirs:5,Files:4,FreeBlock:123,FreeRecord:503,FileSize:16 B,UsedSize:128 KB,FreeSize:3936 KB"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("abc\\test3"),&d,&f));
		TESTASSERT(d.GetSize()==1 && f.GetSize()==1);
		TESTASSERT(d[0]==_T("test4"));
		TESTASSERT(f[0]==_T("test3.file"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("abc\\test4"),&d,&f));
		TESTASSERT(d.GetSize()==1 && f.GetSize()==1);
		TESTASSERT(d[0]==_T("test4"));
		TESTASSERT(f[0]==_T("test3.file"));
		//test delete root
		TESTASSERT(s->DeleteDirectory(_T("\\")));
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:128,ImgFileSize:4195864 B,RecordCount:1,Dirs:0,Files:0,FreeBlock:127,FreeRecord:512,FileSize:0 B,UsedSize:0 KB,FreeSize:4064 KB"));
		//test long name
		THString longname=_T("abcdefghijklnopqrstuvwxyz0123456789abcdefghijklnopqrstuvwxyz0123456789abcdefghijklnopqrstuvwxyz0123456789abcdefghijklnopqrstuvwxyz0123456789abcdefghijklnopqrstuvwxyz");
		TESTASSERT(s->CreateDirectory(longname));
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:128,ImgFileSize:4195864 B,RecordCount:5,Dirs:1,Files:0,FreeBlock:127,FreeRecord:508,FileSize:0 B,UsedSize:0 KB,FreeSize:4064 KB"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T(""),&d,&f));
		TESTASSERT(d.GetSize()==1 && f.GetSize()==0);
		TESTASSERT(d[0]==longname);
		//test long file
		idx=s->OpenFile(longname+_T(".txt"),FILEMODE_CREATEOREXIST);
		TESTASSERT(idx!=0);
		THStringA lna(longname);
		char *longbuf=new char[66001];
		char *tmp=longbuf;
		UINT rdlen=0;
		for(int i=0;i<400;i++)
		{
			memcpy(tmp,(LPCSTR)lna,lna.GetLength());
			tmp+=lna.GetLength();
			rdlen+=lna.GetLength();
			s->WriteFile(idx,(LPCSTR)lna,lna.GetLength());
		}
		s->CloseFile(idx);
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:128,ImgFileSize:4195864 B,RecordCount:9,Dirs:1,Files:1,FreeBlock:124,FreeRecord:504,FileSize:66000 B,UsedSize:96 KB,FreeSize:3968 KB"));
		UINT len;
		void *retbuf=s->ReadWholeFile(longname+_T(".txt"),&len);
		TESTASSERT(retbuf && len==rdlen);
		TESTASSERT(memcmp(retbuf,longbuf,rdlen)==0);
		s->FreeBuf(retbuf);
		delete [] longbuf;
		TESTASSERT(s->DeleteDirectory(_T("\\")));
		CHECKSTATE(_T("ImgVer:1,BlockSize:32 KB,BlockCount:128,ImgFileSize:4195864 B,RecordCount:1,Dirs:0,Files:0,FreeBlock:127,FreeRecord:512,FileSize:0 B,UsedSize:0 KB,FreeSize:4064 KB"));
		s->SetFreeor(s);
		s->DeleteObjectByFreeor(s);
		//test zip and aes
		::DeleteFile(_T("c:\\test.img"));
		void *rbuf[4];
		UINT rlen[4];
		//save file to zipmode
		s=file.CreateObject();
		s->SetBaseDirectory(_T("c:\\test.img"));
		TESTASSERT(s->CreateDirectory(_T("\\test")));
		TESTASSERT(s->WriteWholeFile(_T("\\test\\abc.txt"),"testdatatestdata",16));
		TESTASSERT(s->SetStoreAttrib(_T("zipmode"),_T("gzip")));
		s->SetFreeor(s);
		s->DeleteObjectByFreeor(s);
		//save file to zip & authmode
		s=file.CreateObject();
		s->SetBaseDirectory(_T("c:\\test.img"));
		TESTASSERT(s->GetStoreAttrib(_T("zipmode"))==_T("gzip") && s->GetStoreAttrib(_T("authmode"))==_T("no"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("\\test"),&d,&f));
		TESTASSERT(d.GetSize()==0 && f.GetSize()==1);
		TESTASSERT(f[0]==_T("abc.txt"));
		rbuf[0]=s->ReadWholeFile(_T("\\test\\abc.txt"),&rlen[0]);
		TESTASSERT(s->SetStoreAttrib(_T("zipmode"),_T("gzip")));
		TESTASSERT(s->SetStoreAttrib(_T("authmode"),_T("pass1234")));
		s->SetFreeor(s);
		s->DeleteObjectByFreeor(s);
		//save file to authmode
		s=file.CreateObject();
		TESTASSERT(s->SetStoreAttrib(_T("authmode"),_T("pass1234")));
		s->SetBaseDirectory(_T("c:\\test.img"));
		TESTASSERT(s->GetStoreAttrib(_T("zipmode"))==_T("gzip") && s->GetStoreAttrib(_T("authmode"))==_T("pass1234"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("\\test"),&d,&f));
		TESTASSERT(d.GetSize()==0 && f.GetSize()==1);
		TESTASSERT(f[0]==_T("abc.txt"));
		rbuf[1]=s->ReadWholeFile(_T("\\test\\abc.txt"),&rlen[1]);
		TESTASSERT(s->SetStoreAttrib(_T("zipmode"),_T("no")));
		s->SetFreeor(s);
		s->DeleteObjectByFreeor(s);
		//save file to normalmode
		s=file.CreateObject();
		TESTASSERT(s->SetStoreAttrib(_T("authmode"),_T("pass12344")));
		s->SetBaseDirectory(_T("c:\\test.img"));
		TESTASSERT(s->GetStoreAttrib(_T("zipmode"))==_T("no") && s->GetStoreAttrib(_T("authmode"))==_T("pass1234"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("\\test"),&d,&f));
		TESTASSERT(d.GetSize()==0 && f.GetSize()==1);
		TESTASSERT(f[0]==_T("abc.txt"));
		rbuf[2]=s->ReadWholeFile(_T("\\test\\abc.txt"),&rlen[2]);
		TESTASSERT(s->SetStoreAttrib(_T("authmode"),_T("no")));
		s->SetFreeor(s);
		s->DeleteObjectByFreeor(s);
		//read normalmode file
		s=file.CreateObject();
		s->SetBaseDirectory(_T("c:\\test.img"));
		TESTASSERT(s->GetStoreAttrib(_T("zipmode"))==_T("no") && s->GetStoreAttrib(_T("authmode"))==_T("no"));
		d.RemoveAll();f.RemoveAll();
		TESTASSERT(s->FindFiles(_T("\\test"),&d,&f));
		TESTASSERT(d.GetSize()==0 && f.GetSize()==1);
		TESTASSERT(f[0]==_T("abc.txt"));
		rbuf[3]=s->ReadWholeFile(_T("\\test\\abc.txt"),&rlen[3]);
		TESTASSERT(rlen[0]==rlen[1] && rlen[1]==rlen[2] && rlen[2]==rlen[3] && rlen[3]==16);
		TESTASSERT(memcmp(rbuf[0],rbuf[1],16)==0 && memcmp(rbuf[1],rbuf[2],16)==0 && memcmp(rbuf[2],rbuf[3],16)==0 && memcmp(rbuf[3],"testdatatestdata",16)==0);
		s->FreeBuf(rbuf[0]);s->FreeBuf(rbuf[1]);s->FreeBuf(rbuf[2]);s->FreeBuf(rbuf[3]);
		s->SetFreeor(s);
		s->DeleteObjectByFreeor(s);
		return TRUE;
	}
protected:
	//获取储存器内部状态
	#define STS_BLKSIZE		0	///<区块大小，单位KB
	#define STS_BLKCNT		1	///<区块总数
	#define STS_FREEBLKCNT	2	///<可用区块总数
	#define STS_FREERCDCNT	3	///<可用记录区块总数
	#define STS_FREESIZE	4	///<可用空间，通过可用区块得出，单位KB
	#define STS_IMGSIZE		5	///<Img文件大小，单位B
	//和目录相关
	#define STS_RCDCNT		10	///<已使用记录区块总数
	#define STS_DIRS		11	///<目录数
	#define STS_FILES		12	///<文件数
	#define STS_FILESIZE	13	///<总文件大小，通过文件大小的项统计，单位B
	#define STS_USEDSIZE	14	///<通过文件所占用的区块数计算占用大小
	void GetImgState(U32 state[20],THString path,THString &ret)
	{
		memset(state,0,sizeof(U32)*20);
		THIMGFILE_HEADER *h=(THIMGFILE_HEADER *)m_imgbuf;
		state[STS_BLKSIZE]=ntohs(h->blocksize);
		state[STS_BLKCNT]=ntohs(h->blockcount);
		U32 tmp=ntohl(h->freeblockoff);
		while(tmp!=0)
		{
			state[STS_FREEBLKCNT]++;
			tmp=ntohl(((THIMGFILE_BLOCK *)(m_imgbuf+tmp))->nextblockoff);
		}
		tmp=ntohl(h->freerecordoff);
		while(tmp!=0)
		{
			state[STS_FREERCDCNT]++;
			tmp=ntohl(((THIMGFILE_RECORD *)(m_imgbuf+tmp))->nextoff);
		}
		state[STS_FREESIZE]=state[STS_FREEBLKCNT]*state[STS_BLKSIZE];
		state[STS_IMGSIZE]=sizeof(THIMGFILE_HEADER)+(state[STS_BLKSIZE]*1024+sizeof(THIMGFILE_BLOCK))*state[STS_BLKCNT];
		_GetPathState(state,path);
		state[STS_USEDSIZE]*=state[STS_BLKSIZE];
		ret.Format(_T("ImgVer:%x,BlockSize:%u KB,BlockCount:%u,ImgFileSize:%u B,"),h->ver,state[STS_BLKSIZE],state[STS_BLKCNT],state[STS_IMGSIZE]);
		ret.AppendFormat(_T("RecordCount:%u,Dirs:%u,Files:%u,FreeBlock:%u,FreeRecord:%u,"),state[STS_RCDCNT],state[STS_DIRS],state[STS_FILES],state[STS_FREEBLKCNT],state[STS_FREERCDCNT]);
		ret.AppendFormat(_T("FileSize:%u B,UsedSize:%u KB,FreeSize:%u KB"),state[STS_FILESIZE],state[STS_USEDSIZE],state[STS_FREESIZE]);
	}
	void _GetPathState(U32 state[20],THString path)
	{
		U32 nextidx=_GetPathIndex(path);
		if (nextidx==0) return;
		state[STS_RCDCNT]++;
		nextidx=_GetSubIndex(nextidx);
		if (nextidx==0) return;
		_GetPathStateByIdx(state,nextidx);
	}
	void _GetPathStateByIdx(U32 state[20],U32 nextidx)
	{
		while(nextidx!=0)
		{
			THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+nextidx);
			state[STS_FILESIZE]+=ntohl(r->datasize);
			if (r->nameoff)
			{
				U32 tmp1=ntohl(r->nameoff);
				while(tmp1!=0)
				{
					tmp1=ntohl(*(U32 *)(m_imgbuf+tmp1));
					state[STS_RCDCNT]++;
				}
			}
			if (r->attrib&0x1)
			{
				state[STS_DIRS]++;
				if (r->suboff)
					_GetPathStateByIdx(state,ntohl(r->suboff));
			}
			else
			{
				state[STS_FILES]++;
				U32 tmp1=ntohl(r->suboff);
				while(tmp1)
				{
					THIMGFILE_BLOCK *bl=(THIMGFILE_BLOCK *)(m_imgbuf+tmp1);
					tmp1=ntohl(bl->nextblockoff);
					state[STS_USEDSIZE]++;
				}
			}
			state[STS_RCDCNT]++;
			nextidx=ntohl(r->nextoff);
		}
	}
	//返回目录的子目录索引
	U32 _GetSubIndex(U32 idx)
	{
		if (idx==IMGIDX_ROOT) return ntohl(((THIMGFILE_HEADER *)m_imgbuf)->recordoff);
		THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+idx);
		return ntohl(r->suboff);
	}
	//获取文件或者目录的idx，指向THIMGFILE_RECORD结构，如果是根目录，总是返回IMGIDX_ROOT
	U32 _GetPathIndex(THString sPath)
	{
		sPath=GetRelPath(GetFullPath(sPath));
		if (sPath==_T("\\") || sPath.IsEmpty()) return IMGIDX_ROOT;//根目录偏移
		U32 idx=ntohl(((THIMGFILE_HEADER *)m_imgbuf)->recordoff);
		if (idx==0) return 0;
		THStringToken t;
		t.Init(sPath,_T("\\"));
		//try to into the directory
		U32 nextidx=idx;
		while(t.IsMoreTokens())
		{
			THString sSubDir=t.GetNextToken();
			if (nextidx==0) return 0;
			while(nextidx!=0)
			{
				THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+nextidx);
				if (sSubDir.CompareNoCase(_GetFileName(r))==0)
				{
					if (r->attrib&0x1)
					{
						if (!t.IsMoreTokens()) return nextidx;
						//subdir
						nextidx=ntohl(r->suboff);
						break;
					}
					//fileidx
					if (t.IsMoreTokens()) return 0;//no more idx found
					return nextidx;
				}
				nextidx=ntohl(r->nextoff);
			}
		}
		return nextidx;
	}
	THString _GetFileName(THIMGFILE_RECORD *r)
	{
		THStringA stra,strb;
		stra.SetString((char *)r->name,RECNAMEFBLEN);
		U32 nextoff=ntohl(r->nameoff);
		while(nextoff!=0)
		{
			U32 *buf=(U32 *)(m_imgbuf+nextoff);
			strb.SetString((char *)(buf+1),RECNAMESBLEN);
			stra+=strb;
			nextoff=ntohl(*buf);
		}
		//这里直接返回THString(stra)会导致有时数据丢失，很奇怪
		return THCharset::a2t(stra);
	}
	BOOL CheckImg()
	{
		if (!m_imgbuf) return NewImg();
		return TRUE;
	}
	void _SaveImg()
	{
		if (m_bDirty)
		{
			if (m_imgbuf && m_imglen && m_store && !m_ImgFileName.IsEmpty())
			{
				THIMGFILE_HEADER *h=(THIMGFILE_HEADER *)m_imgbuf;
				if (m_bZip) h->ver|=0x80;
				else h->ver&=~0x80;
				if (!m_sAuth.IsEmpty()) h->ver|=0x40;
				else h->ver&=~0x40;
				U8 *curbuf=m_imgbuf;
				U32 curlen=m_imglen;
				if (m_bZip)
				{
					void *buf;
					unsigned long retlen;
					if (THGZip::ZipEncode(curbuf+sizeof(THIMGFILE_HEADER),curlen-sizeof(THIMGFILE_HEADER),&buf,&retlen))
					{
						retlen+=sizeof(THIMGFILE_HEADER);
						U8 *tmpbuf=new U8[retlen];
						if (tmpbuf)
						{
							memcpy(tmpbuf,curbuf,sizeof(THIMGFILE_HEADER));
							memcpy(tmpbuf+sizeof(THIMGFILE_HEADER),buf,retlen-sizeof(THIMGFILE_HEADER));
							curbuf=tmpbuf;
							curlen=retlen;
						}
						else
							curlen=0;
						THGZip::Free(buf);
					}
					else
						curlen=0;
				}
				if (!m_sAuth.IsEmpty())
				{
					void *buf;
					int outlen;
					THAes aes;
					buf=aes.EncryptBuffer(curbuf+sizeof(THIMGFILE_HEADER),curlen-sizeof(THIMGFILE_HEADER),&outlen,m_sAuth);
					if (buf)
					{
						outlen+=sizeof(THIMGFILE_HEADER);
						U8 *tmpbuf=new U8[outlen];
						if (tmpbuf)
						{
							memcpy(tmpbuf,curbuf,sizeof(THIMGFILE_HEADER));
							memcpy(tmpbuf+sizeof(THIMGFILE_HEADER),buf,outlen-sizeof(THIMGFILE_HEADER));
							if (curbuf!=m_imgbuf)
								delete [] curbuf;
							curbuf=tmpbuf;
							curlen=outlen;
						}
						else
							curlen=0;
						aes.Free(buf);
					}
					else
						curlen=0;
				}
				if (curlen!=0)
				{
					m_store->WriteWholeFile(m_ImgFileName,curbuf,curlen);
					m_bDirty=FALSE;
				}
				if (curbuf!=m_imgbuf) delete [] curbuf;
			}
		}
	}
	//需要作内存空间重分配，所有业务逻辑不受影响
	void _FreeImg()
	{
		if (m_imgbuf) delete [] m_imgbuf;
		m_imgbuf=NULL;
		m_imglen=0;
	}
	//释放磁盘，影响业务逻辑
	void _FreeIntStore()
	{
		CloseAllFile();
		if (m_store)
		{
			_SaveImg();
			_FreeImg();
			m_ImgFileName.Empty();
			delete m_store;
			m_store=NULL;
			m_bDirty=FALSE;
			m_bZip=FALSE;
		}
	}
	virtual int _OpenFile(THString sFile,int nMode=FILEMODE_READ)
	{
		if (sFile.IsEmpty()) return -1;
		sFile=GetFullPath(sFile);
		BOOL bMustExist=FALSE;
		int nOpenMode=(nMode&FILEMODE_MASK);
		UINT mode=0;
		if (nOpenMode==FILEMODE_READ)
			bMustExist=TRUE;
		else if (nOpenMode==FILEMODE_READWRITE)
			bMustExist=TRUE;
		else if (nOpenMode==FILEMODE_CREATEOREXIST)
			bMustExist=FALSE;
		else if (nOpenMode==FILEMODE_ALWAYSCREATE)
			bMustExist=FALSE;
		else
			return -1;
		m_mtx.Lock();
		U32 idx=_GetPathIndex(sFile);
		if (idx==0 && bMustExist) return -1;
		//文件打开只支持独占模式
		if (idx!=0 && idx!=IMGIDX_ROOT)
			//文件已打开 || 文件属于目录
			if (m_openfile.find(idx)!=m_openfile.end() || 
				(((((THIMGFILE_RECORD *)(m_imgbuf+idx)))->attrib)&0x01) )
			{
				m_mtx.Unlock();
				return -1;
			}
		if (nOpenMode==FILEMODE_ALWAYSCREATE)
		{
			DeleteFile(sFile);
			idx=0;
		}
		if (idx==0 || idx==IMGIDX_ROOT)
		{
			if (nOpenMode==FILEMODE_CREATEOREXIST || nOpenMode==FILEMODE_ALWAYSCREATE)
			{
				//make file record
				idx=_CreateFileRecord(sFile,TRUE);
				if (idx==0) return -1;
			}
			else return -1;
		}
		if (idx && (nOpenMode==FILEMODE_READWRITE))
		{
			//read only file
			if (((THIMGFILE_RECORD *)(m_imgbuf+idx))->attrib&0x2) return -1;
		}
		OpeningInfo info;
		info.fileoff=idx;
		info.curblockoff=ntohl(((THIMGFILE_RECORD *)(m_imgbuf+idx))->suboff);
		info.inblockoff=0;
		info.relpos=0;
		if (nOpenMode==FILEMODE_READ)
			info.mode=1;
		else
			info.mode=2;
		m_openfile.insert(map<U32,OpeningInfo>::value_type(idx,info));
		m_mtx.Unlock();
		return idx;
	}

	virtual void _CloseFile(int idx)
	{
		THSingleLock s(&m_mtx);
		map<U32,OpeningInfo>::iterator it=m_openfile.find(idx);
		if (it!=m_openfile.end())
		{
			if (it->second.mode==3) m_bDirty=TRUE;
			m_openfile.erase(it);
		}
	}

	virtual void _CloseAllFile()
	{
		THSingleLock s(&m_mtx);
		map<U32,OpeningInfo>::iterator it=m_openfile.begin();
		while(it!=m_openfile.end())
		{
			if (it->second.mode==3)
			{
				m_bDirty=TRUE;
				break;
			}
			it++;
		}
		m_openfile.clear();
	}
	
	virtual UINT _GetPosition(int idx)
	{
		THSingleLock s(&m_mtx);
		map<U32,OpeningInfo>::iterator it=m_openfile.find(idx);
		if (it!=m_openfile.end()) return it->second.relpos;
		return 0;
	}

	virtual UINT _SeekFile(int idx,int nStart,int nPos)
	{
		THSingleLock s(&m_mtx);
		map<U32,OpeningInfo>::iterator it=m_openfile.find(idx);
		if (it!=m_openfile.end())
		{
			OpeningInfo *info=&(it->second);
			UINT relpos=_GetRealSeekPos(idx,nStart,nPos);
			U32 curoff=info->curblockoff;
			U32 inoff=info->inblockoff;
			if (relpos<info->relpos && relpos*2>info->relpos)
			{
				//从当前位置减进行定位比较快
				U32 needoff=info->relpos-relpos;
				while(needoff>0)
				{
					if (inoff>0)
					{
						if (inoff>needoff)
						{
							inoff-=needoff;
							needoff=0;
						}
						else
						{
							needoff-=inoff;
							inoff=0;
						}
					}
					if (needoff!=0 && inoff==0)
					{
						curoff=ntohl(((THIMGFILE_BLOCK *)(m_imgbuf+curoff))->prevblockoff);
						U32 len=ntohl(((THIMGFILE_BLOCK *)(m_imgbuf+curoff))->blocklen);
						inoff=len-1;
					}
				}
			}
			else
			{
				U32 needoff;
				if (relpos<info->relpos)
				{
					//从开始位置定位比较快
					curoff=ntohl(((THIMGFILE_RECORD *)(m_imgbuf+info->fileoff))->suboff);
					inoff=0;
					needoff=relpos;
				}
				else//从当前位置定位
					needoff=relpos-info->relpos;
				while(needoff>0)
				{
					U32 len=ntohl(((THIMGFILE_BLOCK *)(m_imgbuf+curoff))->blocklen);
					len-=inoff;
					if (needoff<len)
					{
						inoff+=needoff;
						needoff=0;
					}
					else
					{
						curoff=ntohl(((THIMGFILE_BLOCK *)(m_imgbuf+curoff))->nextblockoff);
						inoff=0;
						needoff-=len;
						if (curoff==0)
						{
							//不能跳了
							relpos-=needoff;
							break;
						}
					}
				}
			}
			info->curblockoff=curoff;
			info->inblockoff=inoff;
			info->relpos=relpos;
			return relpos;
		}
		return 0;
	}

	virtual UINT _ReadFile(int idx,void *buf,UINT len,int pos=-1)
	{
		THSingleLock s(&m_mtx);
		if (pos!=-1) _SeekFile(idx,SEEK_SET,pos);
		map<U32,OpeningInfo>::iterator it=m_openfile.find(idx);
		if (it==m_openfile.end()) return 0;
		OpeningInfo *info=&(it->second);
		UINT readed=0;
		U32 rlen;
		while(len>0)
		{
			if (info->curblockoff==0) return readed;
			U32 blen=ntohl(((THIMGFILE_BLOCK *)(m_imgbuf+info->curblockoff))->blocklen);
			blen-=info->inblockoff;
			rlen=min(len,blen);
			memcpy((char *)buf+readed,m_imgbuf+info->curblockoff+info->inblockoff+sizeof(THIMGFILE_BLOCK),rlen);
			readed+=rlen;
			len-=rlen;
			info->inblockoff+=rlen;
			info->relpos+=rlen;
			if (len>0)
			{
				rlen=ntohl(((THIMGFILE_BLOCK *)(m_imgbuf+info->curblockoff))->nextblockoff);
				if (rlen==0) break;
				info->curblockoff=rlen;
				info->inblockoff=0;
			}
		}
		return readed;
	}

	virtual UINT _WriteFile(int idx,const void *buf,UINT len,int pos=-1)
	{
		THSingleLock s(&m_mtx);
		if (pos!=-1) _SeekFile(idx,SEEK_SET,pos);
		map<U32,OpeningInfo>::iterator it=m_openfile.find(idx);
		if (it==m_openfile.end()) return 0;
		OpeningInfo *info=&(it->second);
		if (info->mode==1 || len==0) return 0;
		info->mode=3;
		UINT writed=0;
		U32 wlen;
		U32 bcklen=(U32)ntohs(((THIMGFILE_HEADER *)m_imgbuf)->blocksize);
		bcklen*=1024;
		while(len>0)
		{
			U32 blen=bcklen;
			blen-=info->inblockoff;
			wlen=min(len,blen);
			if (info->curblockoff)
			{
				memcpy(m_imgbuf+info->curblockoff+sizeof(THIMGFILE_BLOCK)+info->inblockoff,(const char *)buf+writed,wlen);
				blen=ntohl(((THIMGFILE_BLOCK *)(m_imgbuf+info->curblockoff))->blocklen);
				if (info->inblockoff+wlen>blen)
				{
					((THIMGFILE_BLOCK *)(m_imgbuf+info->curblockoff))->blocklen=htonl(U32(info->inblockoff+wlen));
					THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+info->fileoff);
					r->datasize=htonl((U32)(ntohl(r->datasize)+info->inblockoff+wlen-blen));
				}
				writed+=wlen;
				len-=wlen;
				info->inblockoff+=wlen;
				info->relpos+=wlen;
				if (info->relpos>ntohl(((THIMGFILE_RECORD *)(m_imgbuf+info->fileoff))->datasize))
					((THIMGFILE_RECORD *)(m_imgbuf+info->fileoff))->datasize=htonl(info->relpos);
			}
			if (len>0)
			{
				if (info->curblockoff)
					wlen=ntohl(((THIMGFILE_BLOCK *)(m_imgbuf+info->curblockoff))->nextblockoff);
				else
					wlen=0;
				if (wlen==0)
				{
					//alloc space
					THIMGFILE_HEADER *h=(THIMGFILE_HEADER *)m_imgbuf;
					U32 freeoff=ntohl(h->freeblockoff);
					if (freeoff==0)
					{
						if (!ReAllocImg()) break;
						h=(THIMGFILE_HEADER *)m_imgbuf;
						freeoff=ntohl(h->freeblockoff);
					}
					THIMGFILE_BLOCK *nb=(THIMGFILE_BLOCK *)(m_imgbuf+freeoff);
					h->freeblockoff=nb->nextblockoff;
					nb->nextblockoff=0;
					nb->prevblockoff=htonl(info->curblockoff);
					nb->blocklen=0;
					wlen=freeoff;
					//save the start block map
					if (((THIMGFILE_RECORD *)(m_imgbuf+info->fileoff))->suboff==0)
						((THIMGFILE_RECORD *)(m_imgbuf+info->fileoff))->suboff=htonl(wlen);
					else //continue block
						((THIMGFILE_BLOCK *)(m_imgbuf+info->curblockoff))->nextblockoff=htonl(wlen);
				}
				info->curblockoff=wlen;
				info->inblockoff=0;
			}
		}
		return writed;
	}

	virtual BOOL _CreateDirectory(THString sPath) {return _CreateFileRecord(sPath,FALSE)!=0;}
	virtual BOOL _DeleteDirectory(THString sPath) {return _DeleteFileRecord(sPath);}
	//直接使用目录挂接
	virtual BOOL MoveFile(THString sPath,THString sNewPath)
	{
		sPath=GetFullPath(sPath);
		sNewPath=GetFullPath(sNewPath);
		return _MoveFile(sPath,sNewPath);
	}
	virtual BOOL _MoveFile(THString sPath,THString sNewPath)
	{
		//把结构挂接到新的目录里即可
		BOOL bFile=IsPathFile(sPath);
		//因为名字可能改变了，这里新建一个记录
		U32 newidx=_CreateFileRecord(sNewPath,bFile);
		if (newidx==0) return FALSE;
		U32 idx=_PopFileRecord(sPath);
		if (idx==0)
		{
			_DeleteFileRecord(sNewPath);
			return FALSE;
		}
		//挂接原数据
		THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+newidx);
		THIMGFILE_RECORD *or=(THIMGFILE_RECORD *)(m_imgbuf+idx);
		r->suboff=or->suboff;
		or->suboff=0;
		r->accessdate=or->accessdate;
		r->createdate=or->createdate;
		r->modifydate=or->modifydate;
		r->datasize=or->datasize;
		r->nextoff=or->nextoff;
		r->attrib=or->attrib;
		r->nextoff=0;
		_ReleaseFileRecord(idx);
		return TRUE;
	}
	virtual BOOL _DeleteFile(THString sFile){return _DeleteFileRecord(sFile);}
	U32 _GetFreeRecord()
	{
		THIMGFILE_HEADER *h=(THIMGFILE_HEADER *)m_imgbuf;
		if (h->freerecordoff==0)
		{
			//alloc record block
			if (h->freeblockoff==0)
				if (!ReAllocImg()) return 0;
			U32 freeoff=ntohl(h->freeblockoff);
			THIMGFILE_BLOCK *nb=(THIMGFILE_BLOCK *)(m_imgbuf+freeoff);
			h->freeblockoff=nb->nextblockoff;
			nb->nextblockoff=0;
			nb->prevblockoff=0;
			U32 bs=((U32)ntohs(h->blocksize))*1024;
			nb->blocklen=htonl(bs);
			U32 bsaddr=freeoff+sizeof(THIMGFILE_BLOCK);
			//insert into free record block list
			for(U32 i=0;i<bs;i+=sizeof(THIMGFILE_RECORD))
				((THIMGFILE_RECORD *)(m_imgbuf+bsaddr+i))->nextoff=(i+sizeof(THIMGFILE_RECORD)<bs)?htonl(bsaddr+i+sizeof(THIMGFILE_RECORD)):0;
			h->freerecordoff=htonl(freeoff+sizeof(THIMGFILE_BLOCK));
		}
		//pop free name record
		U32 freeidx=ntohl(h->freerecordoff);
		THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+freeidx);
		h->freerecordoff=r->nextoff;
		return freeidx;
	}
	//返回record索引，返回0表示失败
	U32 _CreateFileRecord(THString sPath,BOOL bFile)
	{
		THStringA sName(GetFileBaseName(sPath));
		if (sName.IsEmpty()) return 0;
		U32 idx=_GetPathIndex(sPath);
		//always exist
		if (idx!=0 && idx!=IMGIDX_ROOT) return 0;
		idx=_GetPathIndex(GetParentPath(sPath));
		//can not found parent directory
		if (idx==0) return 0;
		U32 freeidx=_GetFreeRecord();
		if (idx==IMGIDX_ROOT && ((THIMGFILE_HEADER *)m_imgbuf)->recordoff==0)
			((THIMGFILE_HEADER *)m_imgbuf)->recordoff=htonl(freeidx);
		else
		{
			if (idx==IMGIDX_ROOT)
				idx=ntohl(((THIMGFILE_HEADER *)m_imgbuf)->recordoff);
			else
			{
				//it is not root,into the directory
				THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+idx);
				idx=ntohl(r->suboff);
				if (idx==0) r->suboff=htonl(freeidx);
			}
			while(idx!=0)
			{
				THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+idx);
				idx=ntohl(r->nextoff);
				if (idx==0) r->nextoff=htonl(freeidx);
			}
		}
		THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+freeidx);
		r->accessdate=htonl((U32)time(NULL));
		r->createdate=htonl((U32)time(NULL));
		r->modifydate=htonl((U32)time(NULL));
		r->datasize=0;
		r->suboff=0;
		r->nextoff=0;
		r->attrib=bFile?0:0x1;
		if (sName.GetLength()>RECNAMEFBLEN)
		{
			memcpy(r->name,(LPCSTR)sName,RECNAMEFBLEN);
			sName=sName.Mid(RECNAMEFBLEN);
			U32 *lastaddr=&r->nameoff;
			while(sName.GetLength()>0)
			{
				U32 ns=_GetFreeRecord();
				if (ns==0)
				{
					*lastaddr=0;
					break;
				}
				*lastaddr=htonl(ns);
				U32 len=min(RECNAMESBLEN,sName.GetLength());
				memcpy(m_imgbuf+ns+sizeof(U32),(LPCSTR)sName,len);
				if (len!=RECNAMESBLEN) *(m_imgbuf+ns+sizeof(U32)+len)='\0';
				sName=sName.Mid(len);
				lastaddr=(U32 *)(m_imgbuf+ns);
			}
			*lastaddr=0;
		}
		else
		{
			strcpy((char *)r->name,sName);
			r->nameoff=0;
		}
		m_bDirty=TRUE;
		return freeidx;
	}
	//弹出FileRecord记录，把记录从上级目录中移除
	U32 _PopFileRecord(THString sPath)
	{
		THIMGFILE_HEADER *h=(THIMGFILE_HEADER *)m_imgbuf;
		U32 idx=_GetPathIndex(sPath);
		if (idx==0) return 0;
		if (idx==IMGIDX_ROOT)
		{
			idx=ntohl(h->recordoff);
			h->recordoff=0;
			m_bDirty=TRUE;
			return idx;
		}
		U32 pidx=_GetPathIndex(GetParentPath(sPath));
		//can not found parent directory
		if (pidx==0) return 0;
		if (pidx==IMGIDX_ROOT)
		{
			pidx=ntohl(((THIMGFILE_HEADER *)m_imgbuf)->recordoff);
			if (pidx==idx)
			{
				((THIMGFILE_HEADER *)m_imgbuf)->recordoff=((THIMGFILE_RECORD *)(m_imgbuf+idx))->nextoff;
				((THIMGFILE_RECORD *)(m_imgbuf+idx))->nextoff=0;
				m_bDirty=TRUE;
				return idx;
			}
		}
		else
		{
			THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+pidx);
			pidx=ntohl(r->suboff);
			if (pidx==idx)
			{
				r->suboff=((THIMGFILE_RECORD *)(m_imgbuf+idx))->nextoff;
				((THIMGFILE_RECORD *)(m_imgbuf+idx))->nextoff=0;
				m_bDirty=TRUE;
				return idx;
			}
		}
		while(pidx)
		{
			THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+pidx);
			if (ntohl(r->nextoff)==idx)
			{
				r->nextoff=((THIMGFILE_RECORD *)(m_imgbuf+idx))->nextoff;
				((THIMGFILE_RECORD *)(m_imgbuf+idx))->nextoff=0;
				m_bDirty=TRUE;
				return idx;
			}
			pidx=ntohl(r->nextoff);
		}
		return 0;
	}
	BOOL _DeleteFileRecord(THString sPath)
	{
		U32 idx=_PopFileRecord(sPath);
		if (idx)
		{
			_ReleaseFileRecord(idx);
			return TRUE;
		}
		idx=_GetPathIndex(sPath);
		//如果删除的是根目录，而且根目录为空的话，直接返回
		if (idx==IMGIDX_ROOT) return TRUE;
		return FALSE;
	}
	//回收文件名记录块，处理记录块中的逻辑，回收所有关联记录
	void _ReleaseFileRecord(U32 rpos)
	{
		THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+rpos);
		//free sub dir info
		U32 sub=ntohl(r->suboff);
		if (sub)
		{
			if (r->attrib&0x01)
				_ReleaseFileRecord(sub);
			else
				_ReleaseBlock(sub);
		}
		//free name info
		sub=ntohl(r->nameoff);
		while(sub!=0)
		{
			U32 next=ntohl(*((U32 *)(m_imgbuf+sub)));
			__ReleaseFileRecord(sub);
			sub=next;
		}
		//free next dir info
		sub=ntohl(r->nextoff);
		__ReleaseFileRecord(rpos);
		if (sub) _ReleaseFileRecord(sub);
	}
	//回收文件名记录块，只是当前块
	void __ReleaseFileRecord(U32 rpos)
	{
		THIMGFILE_HEADER *h=(THIMGFILE_HEADER *)m_imgbuf;
		THIMGFILE_RECORD *r=(THIMGFILE_RECORD *)(m_imgbuf+rpos);
		r->nextoff=h->freerecordoff;
		h->freerecordoff=htonl(rpos);
	}
	//回收block，处理所有关联block
	void _ReleaseBlock(U32 block)
	{
		THIMGFILE_HEADER *h=(THIMGFILE_HEADER *)m_imgbuf;
		U32 lastfree=h->freeblockoff;
		h->freeblockoff=htonl(block);
		if (lastfree!=0)
		{
			while(block!=0)
			{
				THIMGFILE_BLOCK *nb=(THIMGFILE_BLOCK *)(m_imgbuf+block);
				if (nb->nextblockoff==0)
				{
					nb->nextblockoff=lastfree;
					break;
				}
				block=ntohl(nb->nextblockoff);
			}
		}
	}

	ITHFileStore *m_store;
	THString m_ImgFileName;
	U8 *m_imgbuf;
	UINT m_imglen;
	BOOL m_bDirty;
	typedef struct _OpeningInfo{
		U32 fileoff;			///<<<文件名称记录偏移
		U32 curblockoff;		///<<<当前游标所在块偏移
		U32 inblockoff;			///<<<当前游标所在块内偏移
		int mode;				///<<<打开方式 1 读，2 读写，未改动；3 读写，已改动
		U32 relpos;				///<<<逻辑文件的相对偏移，从0开始
	}OpeningInfo;
	map<U32,OpeningInfo> m_openfile;
	THMutex m_mtx;
	BOOL m_bZip;
	THString m_sAuth;
};
