#pragma once

#include <THString.h>
#include <THArith.h>
#include <THCharset.h>
#include <THFile.h>
#include <THRawSocketModel.h>

class THSmtpEncodeContent
{
public:
	THSmtpEncodeContent(){
		m_szSender="Tansoft Mailer V1.0 [ch]";
		m_nInPro=1;
		m_szCharset="GB2312";
		m_bIsUseHtml=FALSE;
		m_bIsUseBase64=TRUE;
	}
	THStringA GetEncodedContent(){
		THStringA szContent;
		THStringA szDateTime;
		THStringA szSubject;
		THStringA szBodyHead;
		THStringA szBody;
		THTime m_tDateTime;
		THStringA szAttachment;
		THStringA szTemp;
		//���½��� Body �Ĵ���
		szDateTime = "Date:";
		szDateTime += THCharset::t2astr(m_tDateTime.Format(_T("%a, %d %b %Y %H:%M:%S %Z")));
		szDateTime += "\r\n";
		szSubject.Format("Subject:%s\r\n", m_szMailTitle);
		szBodyHead = "X-Mailer: "+m_szSender+"\r\n";
		szBodyHead += "MIME_Version:1.0\r\n";
		if (m_nInPro==0)
			szBodyHead += "Importance: Low\r\n";
		else if (m_nInPro==2)
			szBodyHead += "Importance: High\r\n";
		else
			szBodyHead += "Importance: Normal\r\n";
		szBodyHead += "Content-type:multipart/mixed;Boundary=tansoft\r\n\r\n";
		szBodyHead += "--tansoft\r\n";
		if (m_bIsUseHtml==TRUE)
			szBodyHead += "Content-type:text/html;Charset="+m_szCharset+"\r\n";
		else
			szBodyHead += "Content-type:text/plain;Charset="+m_szCharset+"\r\n";
		if (m_bIsUseBase64==TRUE){
			szBodyHead += "Content-Transfer-Encoding:Base64\r\n\r\n";
			szBody.Format("%s\r\n\r\n",THCharset::t2astr(THBase64::EncodeStrA(m_szMailContent)));
		} else {
			szBodyHead += "Content-Transfer-Encoding:8bit\r\n\r\n";
			szBody.Format("%s\r\n\r\n",m_szMailContent);
		}
		//���½��и����Ĵ���
		if(m_saFilenames.GetSize() != 0)
		{
			szTemp = "--tansoft\r\n";
			szTemp += "Content-Type:application/octet-stream;Name=%s\r\n";
			szTemp += "Content-Disposition:attachment;FileName=%s\r\n";
			szTemp += "Content-Transfer-Encoding:Base64\r\n\r\n";
			szTemp += "%s\r\n\r\n";

			szContent = szDateTime + szSubject + szBodyHead + szBody;
			for(int i = 0; i < m_saFilenames.GetSize(); i++)
			{
				THFile hFile;
				if(hFile.Open(m_saFilenames.GetAt(i), CFile::modeRead))
				{
					DWORD dwFileSize = (DWORD)hFile.GetLength();
					THStringA szFileBuffer;
					hFile.Read(szFileBuffer.GetBuffer(dwFileSize), dwFileSize);
					THStringA filenamea=THCharset::t2astr(hFile.GetFileName());
					szAttachment.Format(szTemp, filenamea, filenamea, THCharset::t2astr(THBase64::Encode(szFileBuffer, dwFileSize)));
					szFileBuffer.ReleaseBuffer();
					hFile.Close();
					szContent += szAttachment;
				}
			}

			//���ӳ� Content ��
			szContent += "--tansoft--\r\n.\r\n";
		}
		else
		{
			//���ӳ� Content ��
			szContent = szDateTime + szSubject + szBodyHead + szBody + "--tansoft" + "\r\n.\r\n";
		}
		return szContent;
	}
	void BaseConentA(THStringA title,THStringA body,THStringA charset="GB2312",BOOL bUseHtml=FALSE,BOOL bUseBase64=TRUE){
		m_szMailTitle=title;
		m_szMailContent=body;
		m_szCharset=charset;
		m_bIsUseHtml=bUseHtml;
		m_bIsUseBase64=bUseBase64;
	}
	void BaseConent(THString title,THString body,THString charset=_T("GB2312"),BOOL bUseHtml=FALSE,BOOL bUseBase64=TRUE){
		m_szMailTitle=THCharset::t2astr(title);
		m_szMailContent=THCharset::t2astr(body);
		m_szCharset=THCharset::t2astr(charset);
		m_bIsUseHtml=bUseHtml;
		m_bIsUseBase64=bUseBase64;
	}
	void SetPriority(int pri){m_nInPro=pri;}
	void SetMailer(THString mailer){m_szSender=THCharset::t2astr(mailer);}
	void AddAttachment(THString file){m_saFilenames.Add(file);}
	void CleanAttachment(){m_saFilenames.RemoveAll();}
protected:
	THStringA m_szMailTitle;
	THStringA m_szSender;
	THStringA m_szCharset;
	int m_nInPro;
	BOOL m_bIsUseHtml;
	BOOL m_bIsUseBase64;
	THStringA m_szMailContent;
	THStringArray m_saFilenames;
};

/**
* @brief SMTPЭ���װ��
* @author Barry
* @2013-03-07 �½���
*/
/**<pre>
ʹ��Sample��
	THSmtpEncodeContent content;
	content.BaseConent(_T("title"),_T("<body><a>test</a></body>"),_T("GB2312"),TRUE);
	THSmtpClient::DirectSend(&content,_T("barrytan@21cn.com"),_T("Barry Tan"),_T("tlj@yixia.com;t@yixia.com;T@yixia.com"),
		_T("barrytan@21cn.com"),_T("123456"),_T("smtp.21cn.com"));
</pre>*/
class THSmtpClient
{
public:
	THSmtpClient(){
		m_nPort=25;
		m_sError = _T("OK");
		m_s=INVALID_SOCKET;
		m_content=NULL;
	}
	virtual ~THSmtpClient(){
		Disconnect();
	}
	static BOOL DirectSend(THSmtpEncodeContent *content,THString from,THString fromname,THString tos,
		THString account,THString password,THString server,unsigned short nPort=25){
			THStringToken token(tos,_T(";"));
			THString to;
			THStringArray toar;
			while(token.IsMoreTokens()){
				to=token.GetNextToken().Trim();
				if (!to.IsEmpty()) {
					toar.Add(to);
				}
			}
			if (toar.GetSize()==0) return FALSE;
			return DirectSend(content,from,fromname,toar,account,password,server,nPort);
	}
	static BOOL DirectSend(THSmtpEncodeContent *content,THString from,THString fromname,const THStringArray& to,
		THString account,THString password,THString server,unsigned short nPort=25){
		THSmtpClient client;
		client.SetServer(server,nPort);
		client.m_strAccount=account;
		client.m_strPassword=password;
		client.m_strFrom=from;
		client.m_strTo.Copy(to);
		client.m_strUsername=fromname;
		client.m_content=content;
		if (client.Connect()){
			if (client.Auth()) {
				if (client.SendMessage())
					client.Disconnect();
					return TRUE;
			}
		}
		client.Disconnect();
		return FALSE;
	}
	void SetContent(THSmtpEncodeContent *content){m_content=content;}
	void SetServer(THString server,unsigned short nPort=25){
		m_server = server;
		m_nPort = nPort;
	}
	THString GetLastError(){return m_sError;}
	U16 GetPort(){return m_nPort;}
	BOOL Disconnect(){
		BOOL ret;
		if(m_s==INVALID_SOCKET) return TRUE;
		if (!m_rs.Send(m_s,"QUIT\r\n",6)) {
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		ret = get_response(QUIT_SUCCESS);
		m_rs.FreeSocket(m_s);
		m_s=INVALID_SOCKET;
		return ret;
	}
	BOOL Connect()
	{
		if(m_s!=INVALID_SOCKET) return TRUE;
		m_s=m_rs.CreateSocket(THRawSocket::GetIpFromHostName(m_server,NULL),m_nPort,0,RawProtocol_Tcp,FALSE);
		if(m_s==INVALID_SOCKET)
		{
			m_sError = _T("�޷����ӵ���������");
			return FALSE;
		}
		if(!get_response(CONNECT_SUCCESS))
		{
			m_sError = _T("������û����Ӧ��");
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		//���͡�EHLO %SMTP-Server Name%����
		CStringA szHello;
		char *servera=THCharset::t2a(m_server);
		szHello.Format("EHLO %s\r\n", servera);
		if (!m_rs.Send(m_s,(LPCSTR)szHello,szHello.GetLength()))
		{
			m_sError = _T("�޷���������");
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			THCharset::free(servera);
			return FALSE;
		}
		if(!get_response(GENERIC_SUCCESS))
		{
			//���Է��͡�EHLO %SMTP-Server Name%����
			szHello.Format("HELO %s\r\n", servera);
			m_rs.Send(m_s,(LPCSTR)szHello,szHello.GetLength());
			if(!get_response(GENERIC_SUCCESS)) {
				m_sError = _T("�޷����ӵ�������");
				m_rs.FreeSocket(m_s);
				m_s=INVALID_SOCKET;
				THCharset::free(servera);
				return FALSE;
			}
		}
		THCharset::free(servera);
		return TRUE;
	}
	BOOL Auth(){
		if(m_s==INVALID_SOCKET) {
			m_sError = _T("�������ȴ������ӣ�");
			return FALSE;
		}
		//���͡�AUTH LOGIN����
		if(!m_rs.Send(m_s,"AUTH LOGIN\r\n",12)){
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		if(!get_response(AUTHLOGIN_SUCCESS)) {
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		//���;���Base64������û��ʺţ�
		THStringA szBuf;
		szBuf.Format("%s\r\n", THCharset::t2astr(THBase64::EncodeStr(m_strAccount)));
		if(!m_rs.Send(m_s,szBuf,szBuf.GetLength())){
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		if(!get_response(AUTHLOGIN_SUCCESS)) {
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		//���;���Base64������û����룺
		szBuf.Format("%s\r\n", THCharset::t2astr(THBase64::EncodeStr(m_strPassword)));
		if(!m_rs.Send(m_s,szBuf,szBuf.GetLength())){
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		if(!get_response(AUTH_SUCCESS)) {
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		return TRUE;
	}
	BOOL SendMessage(){
		if(m_s==INVALID_SOCKET) {
			m_sError = _T("�������ȴ������ӣ�");
			return FALSE;
		}
		return transmit_message();
	}
	THString GetServer(){return m_server;}

	THString m_strAccount;
	THString m_strPassword;
	THString m_strFrom;
	THStringArray m_strTo;
	THSmtpEncodeContent *m_content;
	THString m_strUsername;
protected:
	BOOL get_response(UINT response_expected){
		CStringA sResponse;
		UINT response;
		int ret=m_rs.ReceiveFor(m_s,response_buf,1024,0,0,5000);
		if (ret<0) {
			m_sError = _T("δ֪�������ݣ�");
			return FALSE;
		}
		response_buf[ret]='\0';
		sResponse = response_buf;
		response = atoi(sResponse);
		if(response != response_expected) {
			m_sError.Format(_T("%d:%s"), response, (LPCTSTR)GetResponseCode(response_expected));
			return FALSE;
		}
		return TRUE;
	}

	THString m_sError;
	UINT m_nPort;
	THString m_server;
	THRawSocket m_rs;
	RAWSOCKET m_s;
	virtual BOOL transmit_message(){
		THStringA szBuf;
		if(m_s==INVALID_SOCKET) {
			m_sError = _T("�������ȴ������ӣ�");
			return FALSE;
		}
		//����Header�еġ�MAIL FROM����
		szBuf.Format("MAIL FROM:<%s>\r\n", THCharset::t2astr(m_strFrom));
		if(!m_rs.Send(m_s,szBuf,szBuf.GetLength())){
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		if(!get_response(GENERIC_SUCCESS)) {
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		BOOL bOK=FALSE;
		for(int i=0;i<m_strTo.GetSize();i++){
			//����Header�еġ�RCPT TO����
			szBuf.Format("RCPT TO:<%s>\r\n", THCharset::t2astr(m_strTo.GetAt(i)));
			if(!m_rs.Send(m_s,szBuf,szBuf.GetLength())){
				m_rs.FreeSocket(m_s);
				m_s=INVALID_SOCKET;
				return FALSE;
			}
			if(get_response(GENERIC_SUCCESS)) bOK=TRUE;
		}
		if (bOK==FALSE) {
			//���е�ַ�����ʹ���
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		//���͡�DATA\r\n����
		if(!m_rs.Send(m_s,"DATA\r\n",6)){
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		if(!get_response(DATA_SUCCESS)){
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		szBuf.Format("FROM:%s<%s>\r\n", THCharset::t2astr(m_strUsername), THCharset::t2astr(m_strFrom));
		CString tmpSendTo=m_strTo.GetAt(0);
		for (i=1;i<m_strTo.GetSize();i++){
			tmpSendTo+=";";
			tmpSendTo+=m_strTo.GetAt(i);
		}
		szBuf.AppendFormat("TO:<%s>\r\n", THCharset::t2astr(tmpSendTo));
		if (m_content)
			szBuf+=m_content->GetEncodedContent();
		//���� Content ��
		if(!m_rs.Send(m_s,szBuf,szBuf.GetLength())){
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		if(!get_response(GENERIC_SUCCESS)){
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		//���͡�Quit����
		if(!m_rs.Send(m_s,"QUIT\r\n",6)){
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		if(!get_response(QUIT_SUCCESS)){
			m_rs.FreeSocket(m_s);
			m_s=INVALID_SOCKET;
			return FALSE;
		}
		return TRUE;
	}
	enum eResponse
	{
		GENERIC_SUCCESS = 250,
		AUTHLOGIN_SUCCESS = 334,
		AUTH_SUCCESS = 235,
		CONNECT_SUCCESS = 220,
		DATA_SUCCESS = 354,
		QUIT_SUCCESS =221,
		LAST_RESPONSE
	};
	THString GetResponseCode(unsigned int code){
		THString ret;
		switch(code){
			case 250:ret=_T("SMTP ����������");break;
			case 334:ret=_T("SMTP ��֤���Ӵ���");break;
			case 235:ret=_T("SMTP �û���/������֤����");break;
			case 220:ret=_T("SMTP ������������");break;
			case 354:ret=_T("SMTP ���������ܽ�������");break;
			case 221:ret=_T("SMTP û����ֹ�Ự");break;
			default:;
		}
		return ret;
	}
	char response_buf[1024];
};
