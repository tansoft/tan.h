<%
rem 文章标题:在ASP中通过vbs类实现rsa加密与解密
rem 收集整理:yanek
rem 联系:ASPboy@263.net

%>
<%Option Explicit%>
<!--#INCLUDE FILE="tinirsa.ASP"-->
<%

Dim LngKeyE
Dim LngKeyD
Dim LngKeyN
Dim StrMessage
Dim ObjRSA
If Not Request.Form = "" Then

LngKeyE = Request.Form("KeyE")
LngKeyD = Request.Form("KeyD")
LngKeyN = Request.Form("KeyN")
StrMessage = Request.Form("Message")

Set ObjRSA = New clsRSA

Select Case Request.Form("Action")
Case "Generate Keys"
Call ObjRSA.GenKey()
LngKeyE = ObjRSA.PublicKey
LngKeyD = ObjRSA.PrivateKey
LngKeyN = ObjRSA.Modulus
Case "Encrypt"
ObjRSA.PublicKey = LngKeyE
ObjRSA.Modulus = LngKeyN
StrMessage = ObjRSA.Encode(StrMessage)
Case "Decrypt"
ObjRSA.PrivateKey = LngKeyD
ObjRSA.Modulus = LngKeyN
StrMessage = ObjRSA.Decode(StrMessage)
End Select

Set ObjRSA = Nothing

End If
%>
<HTML>
<HEAD>
<TITLE>RSA Cipher Demonstration</TITLE>
</HEAD>
<BODY>
<H1>RSA Cipher Demonstration</H1>
<P>
You will first need to generate your public/privage key-pair
before you can encrypt/decrypt messages.
</P>
<FORM method="post">
<TABLE>
<TR>
<TD>Public Key</TD>
<TD><INPUT name="KeyE" value="<%=Server.HTMLEncode(LngKeyE)%>"></TD>
<TD rowspan="3">
<INPUT type="Submit" name="Action" value="Generate Keys">
</TD>
</TR>
<TR>
<TD>Private Key</TD>
<TD><INPUT name="KeyD" value="<%=Server.HTMLEncode(LngKeyD)%>"></TD>
</TR>
<TR>
<TD>Modulus</TD>
<TD><INPUT name="KeyN" value="<%=Server.HTMLEncode(LngKeyN)%>"></TD>
</TR>
<TR>
<TD colspan="3">
Test Message:<BR>
<TEXTAREA name="Message" cols="50" rows="7"><%=Server.HTMLEncode(StrMessage)%></TEXTAREA>
</TD>
</TR>
<TR>
<TD align="right" colspan="3">
<INPUT type="Submit" name="Action" value="Encrypt">
<INPUT type="Submit" name="Action" value="Decrypt">
</TD>
</TR>
</TABLE>
</FORM>
</BODY>
</HTML>
