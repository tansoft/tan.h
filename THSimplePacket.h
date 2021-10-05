#pragma once

#include <THNetWork.h>
#include <THArith.h>

#pragma pack(push,1)
	typedef struct _THSimplePacketHeader
	{
		U16 magic;
		U16 sum;
		U16 len;
		U8 cmd;
		U8 flag;
		U16 seq;
	}THSimplePacketHeader;
#pragma pack(pop)

#define THSIMPLEPACKET_MAXPACKETLEN			0xffff
#define THSIMPLEPACKET_MAXCONTENTLEN		(THSIMPLEPACKET_MAXPACKETLEN-sizeof(THSimplePacketHeader)-1-32)//1�������д����32����aes���ܺ�д��
#define THSIMPLEPACKET_MAGICSEED	(U16)0x3f9d

//flag mask
#define THSIMPLEPACKET_FLAG_DATATYPE_MASK			0x7
#define THSIMPLEPACKET_FLAG_DATATYPE_RAWDATA		0x1
#define THSIMPLEPACKET_FLAG_DATATYPE_GZIP			0x2
#define THSIMPLEPACKET_FLAG_DATATYPE_AES			0x3
#define THSIMPLEPACKET_FLAG_DATATYPE_GZIP_AES		0x4

/**
* @brief �򵥰���װ��
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2008-03-05 �½���
*/
/**<pre>
�÷���
</pre>*/
class THSimplePacket
{
public:
	THSimplePacket()
	{
	}
	virtual ~THSimplePacket()
	{
	}

	/**
	* @brief ����������Ƿ��а��壬�����ȷ�������
	* @param buf			����
	* @param len			���ݳ���
	* @param cmd			���ذ�����
	* @param seq			���ذ�seq
	* @param outdata		���ص����ݣ��ɵ��ú����ṩ��buffer��������ڵ���THSIMPLEPACKET_MAXPACKETLEN
	* @param outlen			���ص����ݳ��ȣ�-1Ϊ����
	* @param sDecryptKey	��������
	* @return �������Ѵ������ݳ��ȣ�����0����������δ����У���ʧ�ܣ����ʧ�ܵ������Ȼ�������ݳ��ȣ�������ն�Ӧ���壬��ֱ�ӷ��ظ�ITHRawSocketBufferedEventHandler::IsBufferReady
	*/
	static UINT CheckPacket(const void *buf,unsigned int len,U8 *cmd,U16 *seq,void *outdata,int *outlen,THString sDecryptKey=_T("DefaultPass"))
	{
		static THAes aes;
		ASSERT(buf && outdata && outlen);
		*outlen=-1;
		if (len<sizeof(THSimplePacketHeader)) return 0;
		THSimplePacketHeader *tmp=(THSimplePacketHeader *)buf;
		if (ntohs(tmp->magic)!=THSIMPLEPACKET_MAGICSEED) return len;//���ݴ���ȫ������
		if (ntohs(tmp->len)>len) return 0;
		len=ntohs(tmp->len);
		if (ischecksumright(tmp->sum,(const U8*)buf+sizeof(U16)*2,len-sizeof(U16)*2))
		{
			if (cmd) *cmd=tmp->cmd;
			if (seq) *seq=ntohs(tmp->seq);
			buf=tmp+1;
			int inlen=len-sizeof(THSimplePacketHeader);
			U8 datatype=(tmp->flag&THSIMPLEPACKET_FLAG_DATATYPE_MASK);
			if (datatype==THSIMPLEPACKET_FLAG_DATATYPE_AES || datatype==THSIMPLEPACKET_FLAG_DATATYPE_GZIP_AES)
				buf=aes.DecryptBuffer(buf,inlen,&inlen,sDecryptKey);
			if (buf)
			{
				if (datatype==THSIMPLEPACKET_FLAG_DATATYPE_GZIP || datatype==THSIMPLEPACKET_FLAG_DATATYPE_GZIP_AES)
				{
					void *obuf;
					unsigned long olen;
					if (THGZip::ZipDecode(buf,inlen,&obuf,&olen,THGZIP_ZLIB,0xffff))//the max packet len
					{
						if (olen<=THSIMPLEPACKET_MAXCONTENTLEN)
						{
							memcpy(outdata,obuf,olen);
							*outlen=olen;
						}
						THGZip::Free(obuf);
					}
				}
				else if (datatype==THSIMPLEPACKET_FLAG_DATATYPE_RAWDATA || datatype==THSIMPLEPACKET_FLAG_DATATYPE_AES)
				{
					memcpy(outdata,buf,inlen);
					*outlen=inlen;
				}
				if (datatype==THSIMPLEPACKET_FLAG_DATATYPE_AES || datatype==THSIMPLEPACKET_FLAG_DATATYPE_GZIP_AES)
					aes.Free((void *)buf);
			}
		}
		return len;
	}
	/**
	* @brief ����������Ƿ��а��壬�����ȷ�������
	* @param buf			����
	* @param len			���ݳ���
	* @param outdata		���ص����ݣ��ɵ��ú����ṩ��buffer��������ڵ���THSIMPLEPACKET_MAXPACKETLEN
	* @param outlen			���ص����ݳ���
	* @param cmd			������
	* @param seq			��seq
	* @param bAutoZip		�Ƿ��Զ�����ѹ��
	* @param sEncryptKey	�������룬���Ϊ��Ϊ������
	* @return �Ƿ�ɹ�
	*/
	static BOOL GeneratePacket(const void *buf,unsigned int len,void *outdata,unsigned int *outlen,U8 cmd,U16 seq,BOOL bAutoZip=TRUE,THString sEncrptKey=_T("DefaultPass"))
	{
		static THAes aes;
		ASSERT(buf && outdata && outlen);
		ASSERT(len<=THSIMPLEPACKET_MAXCONTENTLEN);
		THSimplePacketHeader *tmp=(THSimplePacketHeader *)outdata;
		tmp->cmd=cmd;
		tmp->seq=htons(seq);
		tmp->magic=htons(THSIMPLEPACKET_MAGICSEED);
		U8 flag=THSIMPLEPACKET_FLAG_DATATYPE_RAWDATA;
		if (bAutoZip && len>0)
		{
			void *obuf;
			unsigned long olen;
			if (THGZip::ZipEncode(buf,len,&obuf,&olen,THGZIP_ZLIB))//the max packet len
			{
				if (olen<len)
				{
					buf=obuf;
					len=olen;
					flag=THSIMPLEPACKET_FLAG_DATATYPE_GZIP;
				}
				else
					THGZip::Free(obuf);
			}
		}
		void *obuf;
		if (!sEncrptKey.IsEmpty() && len>0)
		{
			obuf=aes.EncryptBuffer(buf,len,(int *)&len,sEncrptKey);
			if (!obuf) return FALSE;
			if (flag==THSIMPLEPACKET_FLAG_DATATYPE_GZIP)
				flag=THSIMPLEPACKET_FLAG_DATATYPE_GZIP_AES;
			else
				flag=THSIMPLEPACKET_FLAG_DATATYPE_AES;
		}
		else
			obuf=(void *)buf;
		//obuf,len
		tmp->flag=flag;
		if (len>0) memcpy(tmp+1,obuf,len);
		len+=sizeof(THSimplePacketHeader);
		tmp->len=htons((U16)len);
		*outlen=len;
		tmp->sum=checksum((const U8*)tmp+sizeof(U16)*2,len-sizeof(U16)*2);
		if (flag==THSIMPLEPACKET_FLAG_DATATYPE_GZIP || flag==THSIMPLEPACKET_FLAG_DATATYPE_GZIP_AES)
			THGZip::Free((void *)buf);
		if (flag==THSIMPLEPACKET_FLAG_DATATYPE_AES || flag==THSIMPLEPACKET_FLAG_DATATYPE_GZIP_AES)
			aes.Free(obuf);
		return TRUE;
	}
private:
	static inline U16 checksum(const void *buffer,int size)
	{
		const U16 *pbuf=(const U16 *)buffer;
		unsigned int cksum=0;
		while (size > 1)
		{
			cksum += *pbuf++;
			size -= sizeof(U16);
		}
		if (size)
		{
			cksum += *(const U8*)pbuf;
		}
		cksum = (cksum >> 16) + (cksum & 0xffff);
		cksum += (cksum >>16);
		return htons((U16)(~cksum));
	}

	static inline bool ischecksumright(U16 sum,const void *buffer,int size)
	{
		return (sum==checksum(buffer,size));
	}
};
