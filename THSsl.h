#pragma once

#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#pragma message("linking with Open ssl lib,release place openssl\\bin\\libeay32.dll and ssleay32.dll to the target path")
/*#pragma comment(lib,"DelayImp.lib")
#pragma comment(linker,"/DelayLoad:ssleay32.dll")
#pragma comment(linker,"/DelayLoad:libeay32.dll")
#pragma comment(linker,"/Delay:nobind")
#pragma comment(linker,"/Delay:unload")*/
#pragma comment(lib,"ssleay32.lib")
#pragma comment(lib,"libeay32.lib")
#include <THRawSocketModel.h>

/*ssl形式映射
SSLv2_method(void)			=	THSSL_TYPE_SSLV2	or		THSSL_TYPE_SSLV2|THSSL_TYPE_CLIENT|THSSL_TYPE_SERVER
SSLv2_server_method(void)	=	THSSL_TYPE_SSLV2|THSSL_TYPE_SERVER
SSLv2_client_method(void)	=	THSSL_TYPE_SSLV2|THSSL_TYPE_CLIENT
A TLS/SSL connection established with these methods will only understand the SSLv2 protocol. A client will send out SSLv2 client hello messages and will also indicate that it only understand SSLv2. A server will only understand SSLv2 client hello messages.

SSLv3_method(void)			=	THSSL_TYPE_SSLV3	or		THSSL_TYPE_SSLV3|THSSL_TYPE_CLIENT|THSSL_TYPE_SERVER
SSLv3_server_method(void)	=	THSSL_TYPE_SSLV3|THSSL_TYPE_SERVER
SSLv3_client_method(void)	=	THSSL_TYPE_SSLV3|THSSL_TYPE_CLIENT
A TLS/SSL connection established with these methods will only understand the SSLv3 protocol. A client will send out SSLv3 client hello messages and will indicate that it only understands SSLv3. A server will only understand SSLv3 client hello messages. This especially means, that it will not understand SSLv2 client hello messages which are widely used for compatibility reasons, see SSLv23_*_method().

TLSv1_method(void)			=	THSSL_TYPE_TLSV1	or		THSSL_TYPE_TLSV1|THSSL_TYPE_CLIENT|THSSL_TYPE_SERVER
TLSv1_server_method(void)	=	THSSL_TYPE_TLSV1|THSSL_TYPE_SERVER
TLSv1_client_method(void)	=	THSSL_TYPE_TLSV1|THSSL_TYPE_CLIENT
A TLS/SSL connection established with these methods will only understand the TLSv1 protocol. A client will send out TLSv1 client hello messages and will indicate that it only understands TLSv1. A server will only understand TLSv1 client hello messages. This especially means, that it will not understand SSLv2 client hello messages which are widely used for compatibility reasons, see SSLv23_*_method(). It will also not understand SSLv3 client hello messages.

SSLv23_method(void)			=	THSSL_TYPE_SSLV2|THSSL_TYPE_SSLV3|THSSL_TYPE_TLSV1	or	THSSL_TYPE_SSLV2|THSSL_TYPE_SSLV3|THSSL_TYPE_TLSV1|THSSL_TYPE_CLIENT|THSSL_TYPE_SERVER
SSLv23_server_method(void)	=	THSSL_TYPE_SSLV2|THSSL_TYPE_SSLV3|THSSL_TYPE_TLSV1|THSSL_TYPE_SERVER
SSLv23_client_method(void)	=	THSSL_TYPE_SSLV2|THSSL_TYPE_SSLV3|THSSL_TYPE_TLSV1|THSSL_TYPE_CLIENT
A TLS/SSL connection established with these methods will understand the SSLv2, SSLv3, and TLSv1 protocol. A client will send out SSLv2 client hello messages and will indicate that it also understands SSLv3 and TLSv1. A server will understand SSLv2, SSLv3, and TLSv1 client hello messages. This is the best choice when compatibility is a concern.
*/
#define THSSL_TYPE_SSLV2		0x1
#define THSSL_TYPE_SSLV3		0x2
#define THSSL_TYPE_TLSV1		0x4
#define THSSL_TYPE_CLIENT		0x10
#define THSSL_TYPE_SERVER		0x20
#define THSSL_TYPE_NORMAL		THSSL_TYPE_SSLV2|THSSL_TYPE_SSLV3|THSSL_TYPE_TLSV1
#define THSSL_TYPE_NORMALCLIENT	THSSL_TYPE_NORMAL|THSSL_TYPE_CLIENT
#define THSSL_TYPE_NORMALSERVER	THSSL_TYPE_NORMAL|THSSL_TYPE_SERVER

/**
* @brief Ssl类工厂
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-01 新建类
*/
/**<pre>
使用Sample：
</pre>*/
class THSsl
{
public:
	static SSL* AttachSocket(RAWSOCKET sock,SSL_CTX *ctx)
	{
		SSL *ssl=SSL_new(ctx);
		if (!ssl) return NULL;
		if (SSL_set_fd(ssl,(int)sock)==0) return NULL;
		return ssl;
	}
	static BOOL DetachSocket(SSL *ssl)
	{
		BOOL bRet=TRUE;
		int waitsec=1000;
		//SSL_CTX_set_quiet_shutdown
		do{
			int ret=SSL_shutdown(ssl);
			if (ret==1) break;
			if (ret==-1)
			{
				bRet=FALSE;
				break;
			}
			if (waitsec<=0) break;
			waitsec-=100;
			Sleep(100);
			//ret==0 for continue
		}while(1);
		SSL_clear(ssl);
		return bRet;
	}
	static void FreeSslCtx(SSL_CTX *ctx){SSL_CTX_free(ctx);}
	//THSSL_TYPE_XXX
	static SSL_CTX *CreateSslCtx(int type=THSSL_TYPE_NORMAL)
	{
		InitFactory();
		SSL_METHOD *m=NULL;
		int mode;
		if ((type&THSSL_TYPE_SERVER) && (type&THSSL_TYPE_CLIENT)) mode=0;
		else if ((type&THSSL_TYPE_SERVER)) mode=1;
		else if ((type&THSSL_TYPE_CLIENT)) mode=2;
		else mode=0;
		if ((type&THSSL_TYPE_SSLV2) && (type&THSSL_TYPE_SSLV3) && (type&THSSL_TYPE_TLSV1))
		{
			if (mode==0)
				m=SSLv23_method();
			else if (mode==1)
				m=SSLv23_server_method();
			else if (mode==2)
				m=SSLv23_client_method();
		}
		else if ((type&THSSL_TYPE_SSLV2))
		{
			if (mode==0)
				m=SSLv2_method();
			else if (mode==1)
				m=SSLv2_server_method();
			else if (mode==2)
				m=SSLv2_client_method();
		}
		else if ((type&THSSL_TYPE_SSLV3))
		{
			if (mode==0)
				m=SSLv3_method();
			else if (mode==1)
				m=SSLv3_server_method();
			else if (mode==2)
				m=SSLv3_client_method();
		}
		else if ((type&THSSL_TYPE_TLSV1))
		{
			if (mode==0)
				m=TLSv1_method();
			else if (mode==1)
				m=TLSv1_server_method();
			else if (mode==2)
				m=TLSv1_client_method();
		}
		if (m)
			return SSL_CTX_new(m);
		return NULL;
	}
private:
	static void InitFactory()
	{
		static BOOL g_bInitedSsl=FALSE;
		if (!g_bInitedSsl)
		{
			g_bInitedSsl=TRUE;
			/* readable error messages */
			SSL_load_error_strings();
			/* initialize library */
			SSL_library_init();
			//actions_to_seed_PRNG();
			RAND_poll();
			while (RAND_status() == 0) 
			{
				unsigned short rand_ret = rand() % 65536;
				RAND_seed(&rand_ret, sizeof(rand_ret));
			}
		}
	}
};

class THRawSslSocket : public THRawSocket
{
public:
	THRawSslSocket()
	{
		m_ctx=NULL;
	}
	virtual ~THRawSslSocket()
	{
		if (m_ctx) THSsl::FreeSslCtx(m_ctx);
	}
	virtual CString GetSocketLayerName(){return _T("THRawSslSocket");}
	void InitSsl(int type=THSSL_TYPE_NORMAL,CString pem=_T(""),CString key=_T(""))
	{
		if (m_ctx) THSsl::FreeSslCtx(m_ctx);
		m_ctx=THSsl::CreateSslCtx(type);
		if (!pem.IsEmpty() && !key.IsEmpty())
		{
			SSL_CTX_use_certificate_file(m_ctx,pem,SSL_FILETYPE_PEM);
			SSL_CTX_use_PrivateKey_file(m_ctx,key,SSL_FILETYPE_PEM);
			SSL_CTX_check_private_key(m_ctx);
		}
	}
	BOOL UseClientCertificate(THString sCertFile,THString sPrivateKeyFile)
	{
		if (!m_ctx) return FALSE;
		BOOL bRet=FALSE;
		char *cert=THCharset::t2a(sCertFile);
		if (!cert) return FALSE;
		char *pkey=THCharset::t2a(sPrivateKeyFile);
		m_sLastError.Empty();
		if (pkey)
		{
			if (SSL_CTX_use_certificate_chain_file(m_ctx,cert)==1)
			{
				if (SSL_CTX_use_PrivateKey_file(m_ctx,pkey,SSL_FILETYPE_PEM)==1 ||
					SSL_CTX_use_PrivateKey_file(m_ctx,pkey,SSL_FILETYPE_ASN1)==1)
					if (SSL_CTX_check_private_key(m_ctx)==1) bRet=TRUE;
			}
			if (!bRet) m_sLastError=_T("certificate file Error:")+THCharset::a2t(ERR_error_string(ERR_get_error(), NULL));
			THCharset::free(pkey);
		}
		THCharset::free(cert);
		return bRet;
	}
	SSL_CTX *GetCtx(){return m_ctx;}
	THString GetLastError(){return m_sLastError;}
protected:
	virtual int connect_route(RAWSOCKET s,const LPSOCKADDR addr,int addrsize)
	{
		int ret=connect(s,addr,addrsize);
		if (ret!=SOCKET_ERROR)
		{
			THSingleLock l(&m_mtx);
			ASSERT(m_ctx);
			SSL *ssl=THSsl::AttachSocket(s,m_ctx);
			if (!ssl) return SOCKET_ERROR;
			ret=SSL_connect(ssl);
			if (ret!=1)
			{
				THSsl::DetachSocket(ssl);
				return SOCKET_ERROR;
			}
			m_sslmap[s]=ssl;
		}
		return ret;
	}
	virtual RAWSOCKET accept_route(RAWSOCKET s,LPSOCKADDR addr,int *addrlen)
	{
		RAWSOCKET ns=accept(s,addr,addrlen);
		if (ns!=INVALID_SOCKET)
		{
			THSingleLock l(&m_mtx);
			ASSERT(m_ctx);
			SSL *ssl=THSsl::AttachSocket(ns,m_ctx);
			if (!ssl) return SOCKET_ERROR;
			int ret=SSL_accept(ssl);
			if (ret!=1)
			{
				THSsl::DetachSocket(ssl);
				return SOCKET_ERROR;
			}
			m_sslmap[ns]=ssl;
		}
		return ns;
	}
	virtual int recvfrom_route(RAWSOCKET s,char *buf,int len,int flags,LPSOCKADDR addr,int *addrlen)
	{
		return recv_route(s,buf,len,flags);
	}
	virtual int recv_route(RAWSOCKET s,char *buf,int len,int flags)
	{
		m_mtx.Lock();
		SSL *ssl=m_sslmap[s];
		m_mtx.Unlock();
		if (!ssl) return -1;
		if (flags&MSG_PEEK)
			return SSL_peek(ssl,buf,len);
		return SSL_read(ssl,buf,len);
	}
	virtual int sendto_route(RAWSOCKET s,const char *buf,int len,int flags,const LPSOCKADDR to,int tolen)
	{
		return send_route(s,buf,len,flags);
	}
	virtual int send_route(RAWSOCKET s,const char *buf,int len,int flags)
	{
		m_mtx.Lock();
		SSL *ssl=m_sslmap[s];
		m_mtx.Unlock();
		if (!ssl) return -1;
		return SSL_write(ssl,buf,len);
	}
	virtual int closesocket_route(RAWSOCKET s)
	{
		int ret=closesocket(s);
		m_mtx.Lock();
		SSL *ssl=m_sslmap[s];
		m_sslmap.erase(s);
		m_mtx.Unlock();
		if (ssl)
			THSsl::DetachSocket(ssl);
		return ret;
	}
	THMutex m_mtx;
	map<RAWSOCKET,SSL *> m_sslmap;
	SSL_CTX *m_ctx;
	THString m_sLastError;
};
