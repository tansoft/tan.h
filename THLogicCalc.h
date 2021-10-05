//����������ʽ����
#pragma once

#include <THSystem.h>
#include <THStruct.h>
#include <THString.h>

/**
* @brief ������������
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-07 �½���
* @2007-06-16 ����֧��(-1)��(+1)���㣬������ʾʽ��ʱ�����������
* @2007-09-20 ����һ�ڴ�й©����
*/
/**<pre>
�÷���
	//֧��+ - * /
	//֧��^�η� ~���� %����
	//֧��()
	double ret;
	THString str=_T("((1.0+2.3)*12.2-6.5*23-2^5+8.2)/2.3+5~3");//-55.843478
	THLogicCalc::LogicCalc(str,&ret);
	THDebug(_T("LogicCalc %s=%f"),str,ret);
</pre>*/
class THLogicCalc
{
private:
	static void FreeCallBack(void *key,void *value,void *adddata)
	{
		//((THLogicCalc *)adddata)
		delete (double *)value;
	}
public:
	THLogicCalc()
	{
		//��ʹ�����ͷ�ģʽ��pop��ʱ������ڴ�����
		//m_Num.SetFreeProc(FreeCallBack,this);
	}
	virtual ~THLogicCalc(){}

	static BOOL LogicCalc(THString sExpression,double *dResult)
	{
		THLogicCalc calc;
		return calc.Run(sExpression,dResult);
	}

	void FreeNum()
	{
		THPosition pos=m_Num.GetStartPosition();
		double *tmp;
		while(!pos.IsEmpty())
		{
			if (m_Num.GetNextPosition(pos,tmp))
				delete tmp;
		}
		m_Num.RemoveAll();
	}

	//chExpression:���㹫ʽ,dwResultִ�н��
	BOOL Run(THString sExpression,double *dResult)
	{
		sExpression.Trim();
		m_Oper.Empty();
		FreeNum();
		TCHAR NumberString[100],TempBuffer[2];
		NumberString[0]='\0';
		TempBuffer[1]='\0';
		TCHAR *chExpression=sExpression.GetBuffer();
		TCHAR lastExp=0;
		while(*chExpression!='\0')
		{
			if ((*chExpression>='0' && *chExpression<='9') || *chExpression=='.')
			{
				//��������
				TempBuffer[0]=*chExpression;
				_tcscat(NumberString,TempBuffer);
			}
			else
			{
				if (_tcslen(NumberString)>0)
				{
					//����ѹջ
					double *temp=new double;
					if (!temp) {m_sErrorMsg=_T("�ڴ洦�����");return FALSE;}
					*temp=_tstof(NumberString);
					NumberString[0]='\0';
					if (!m_Num.Push(temp)) {m_sErrorMsg=_T("�ڴ洦�����");delete temp;return FALSE;}
				}
				if (*chExpression=='(')
				{
					//�����'('��ʼ��������ѹջ
					if (!m_Oper.Push((TCHAR *)*chExpression)) {m_sErrorMsg=_T("�ڴ洦�����");return FALSE;}
				}
				else if (*chExpression=='+' || *chExpression=='-' ||
					*chExpression=='*' || *chExpression=='/' ||
					*chExpression=='~' || *chExpression=='%' ||
					*chExpression=='^' || *chExpression==')')
				{
					//�����û���������ݲ�����+��-��ͷ��˵���÷����ǼӼ��ţ���Ӧ���浽���ִ����У�������ѭ����������)ѹ��ջʱ��Ҫ���������������ܴ�������(-1)�����������˿�ֱ�Ӱ��ɼӼ��Ų�����ѹ��һ��0��ǰ����0-1����
					if ((m_Num.IsEmpty() || lastExp=='(') && (*chExpression=='-' || *chExpression=='+'))
					{
						double *tmp=new double;
						if (!tmp) {m_sErrorMsg=_T("�ڴ洦�����");return FALSE;}
						*tmp=0.0;
						if (!m_Num.Push(tmp)) {m_sErrorMsg=_T("�ڴ洦�����");delete tmp;return FALSE;}
/*						TempBuffer[0]=*chExpression;
						_tcscat(NumberString,TempBuffer);
						chExpression++;
						continue;*/
					}
					TCHAR *tmpoper=NULL;
					TCHAR oper;
					BOOL bSucc=m_Oper.GetTop(tmpoper);
					oper=(TCHAR)tmpoper;
					if (bSucc)
					{
						//�����ǰ���ʽ���ȼ�����ջ�е�������ѹջ
						if (GetPriority(*chExpression)>GetPriority(oper))
						{
							if (!m_Oper.Push((TCHAR *)*chExpression)) {m_sErrorMsg=_T("�ڴ洦�����");return FALSE;}
						}
						else
						{
							//ջ�е��������ȼ��ϸߣ�����ջ�е�����
							while(m_Oper.IsEmpty()==FALSE && m_Num.IsEmpty()==FALSE)
							{
								double *a=NULL,*b=NULL,c;
								TCHAR *o=NULL;
								//ȡ��ջ�б��ʽֵ
								if (!m_Num.Pop(b)) {m_sErrorMsg=_T("�ڴ洦�����");return FALSE;}
								if (!m_Num.Pop(a)) {m_sErrorMsg=_T("�ڴ洦�����");delete b;return FALSE;}
								if (!m_Oper.Pop(o)) {m_sErrorMsg=_T("�ڴ洦�����");delete a;delete b;return FALSE;}
								//�����()�еļ���
								if((TCHAR)o=='(')
								{
									//������û���õ�������ѹ��ջ��
									if (!m_Num.Push(a)) {m_sErrorMsg=_T("�ڴ洦�����");delete a;delete b;return FALSE;}
									if (!m_Num.Push(b)) {m_sErrorMsg=_T("�ڴ洦�����");delete b;return FALSE;}
									//�����ǰ�ַ�����')'����Ҫ��������÷��ţ���һ��ʵ�ֲ��ӵ�ǰ���ַ�
									if (*chExpression!=')')
									{
										chExpression--;
										break;
									}
								}
								//����ջ�еı��ʽ
								if (!WorkOut(*a,*b,&c,(TCHAR)o))
								{
									delete a;
									delete b;
									return FALSE;
								}
								delete a;
								delete b;
								//�Ѽ�����ѹ��ջ
								double *tmp=new double;
								if (!tmp) {m_sErrorMsg=_T("�ڴ洦�����");return FALSE;}
								*tmp=c;
								if (!m_Num.Push(tmp)) {m_sErrorMsg=_T("�ڴ洦�����");delete tmp;return FALSE;}
								//�����ǰ����')'����,ֻ��Ҫ����һ�ν��
								if (*chExpression!=')')
								{
									//��һ��ʵ�ֲ��ӵ�ǰ���ַ�
									chExpression--;
									break;
								}
								//���ջ��û��������,�����˳�
								if (!m_Oper.GetTop(o))
								{
									//��Ϊ')'�Ƕ�����ģ�������
									break;
								}
								else
								{
									//ջ���Ѿ���'('�����()�ڵ����в���
									if ((TCHAR)o=='(')
									{
										if (!m_Oper.Pop(o)) {m_sErrorMsg=_T("�ڴ洦�����");return FALSE;}
										break;
									}
								}
							}
						}
					}
					else
					{
						//֤��û�в�������ջ�У�ֱ��ѹջ
						if (!m_Oper.Push((TCHAR *)*chExpression)) {m_sErrorMsg=_T("�ڴ洦�����");return FALSE;}
					}
				}
			}
			lastExp=*chExpression;
			chExpression++;
		}
		sExpression.ReleaseBuffer();

		//��������ѹջ
		if (_tcslen(NumberString)>0)
		{
			//����ѹջ
			double *temp=new double;
			if (!temp) {m_sErrorMsg=_T("�ڴ洦�����");return FALSE;}
			*temp=_tstof(NumberString);
			NumberString[0]='\0';
			if (!m_Num.Push(temp)) {m_sErrorMsg=_T("�ڴ洦�����");delete temp;return FALSE;}
		}

		//���ʽ�Ѵ�����ϣ�����ջ��ʣ�µ�����
		while(m_Oper.IsEmpty()==FALSE)
		{
			//���в������������Ѿ�û�в������ˣ�����
			if(m_Num.IsEmpty()) {m_sErrorMsg=_T("���ֶ���������");return FALSE;}
			double *a=NULL,*b=NULL,c;
			TCHAR *o=NULL;
			//ȡ��ջ�б��ʽֵ
			if (!m_Num.Pop(b)) {m_sErrorMsg=_T("�ڴ洦�����");return FALSE;}
			if (!m_Num.Pop(a)) {m_sErrorMsg=_T("�ڴ洦�����");delete a;return FALSE;}
			//����()�Ĵ���
			do{
				if (!m_Oper.Pop(o)) {m_sErrorMsg=_T("�ڴ洦�����");delete a;delete b;return FALSE;}
			}
			while((TCHAR)o=='(' || (TCHAR)o==')');
			//����ջ�еı��ʽ
			if (!WorkOut(*a,*b,&c,(TCHAR)o))
			{
				delete a;
				delete b;
				return FALSE;
			}
			delete a;
			delete b;
			//�Ѽ�����ѹ��ջ
			double *tmp=new double;
			if (!tmp) {m_sErrorMsg=_T("�ڴ洦�����");return FALSE;}
			*tmp=c;
			if (!m_Num.Push(tmp)) {m_sErrorMsg=_T("�ڴ洦�����");delete tmp;return FALSE;}
		}
		//���������
		if (m_Num.IsEmpty())
		{
			//������ʽ����Ҫ�κμ��㣬����0
			*dResult=0.0;
		}
		else
		{
			double *tmpret=NULL;
			if (!m_Num.Pop(tmpret)) {m_sErrorMsg=_T("�ڴ洦�����");return FALSE;}
			*dResult=*tmpret;
			delete tmpret;
		}
		//���ջ�л���ʣ�µ����ݣ����س���
		if (!m_Num.IsEmpty()) {m_sErrorMsg=_T("���ֶ������");return FALSE;}
		m_sErrorMsg=_T("����ɹ�");
		return TRUE;
	}

	THString GetLastError(){return m_sErrorMsg;}
private:
	THStack<TCHAR *> m_Oper;									///<������ջ
	THStack<double *> m_Num;									///<������ջ
	BOOL WorkOut(double a,double b,double *dResult,TCHAR opr)	///<�����a,b���в���opr��ֵ
	{
		switch(opr)
		{
			case '+':*dResult=a+b;return TRUE;
			case '-':*dResult=a-b;return TRUE;
			case '*':*dResult=a*b;return TRUE;
			case '/':if (b==0)
					{
						m_sErrorMsg=_T("�������");
						return FALSE;
					}
					else
					{
						*dResult=a/b;
						return TRUE;
					}
			case '~':
			case '%':
					if (b==0)
					{
						m_sErrorMsg=_T("�������������");
						return FALSE;
					}
					else
					{
						*dResult=(double)((int)a%(int)b);
						return TRUE;
					}
			case '^':*dResult=pow(a,b);return TRUE;
		}
		m_sErrorMsg=_T("δ֪����");
		return FALSE;
	}
	int GetPriority(TCHAR oper)									///<��ȡ����������㼶��
	{
		if (oper=='(' || oper==')') return 0;
		else if (oper=='+' || oper=='-') return 10;
		else if (oper=='~' || oper=='%') return 20;
		else if (oper=='*' || oper=='/') return 30;
		else if (oper=='^') return 40;
		return 0;
	}
	THString m_sErrorMsg;										///<�������
};
