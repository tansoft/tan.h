//包括正则表达式计算
#pragma once

#include <THSystem.h>
#include <THStruct.h>
#include <THString.h>

/**
* @brief 四则混合运算类
* @author Barry(barrytan@21cn.com,QQ:20962493)
* @2007-06-07 新建类
* @2007-06-16 改正支持(-1)和(+1)计算，改正表示式空时计算出错问题
* @2007-09-20 修正一内存泄漏问题
*/
/**<pre>
用法：
	//支持+ - * /
	//支持^次方 ~余数 %余数
	//支持()
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
		//不使用自释放模式，pop的时候会有内存问题
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

	//chExpression:运算公式,dwResult执行结果
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
				//分析数字
				TempBuffer[0]=*chExpression;
				_tcscat(NumberString,TempBuffer);
			}
			else
			{
				if (_tcslen(NumberString)>0)
				{
					//数字压栈
					double *temp=new double;
					if (!temp) {m_sErrorMsg=_T("内存处理错误");return FALSE;}
					*temp=_tstof(NumberString);
					NumberString[0]='\0';
					if (!m_Num.Push(temp)) {m_sErrorMsg=_T("内存处理错误");delete temp;return FALSE;}
				}
				if (*chExpression=='(')
				{
					//如果是'('开始，无条件压栈
					if (!m_Oper.Push((TCHAR *)*chExpression)) {m_sErrorMsg=_T("内存处理错误");return FALSE;}
				}
				else if (*chExpression=='+' || *chExpression=='-' ||
					*chExpression=='*' || *chExpression=='/' ||
					*chExpression=='~' || *chExpression=='%' ||
					*chExpression=='^' || *chExpression==')')
				{
					//如果还没有数字数据并且以+或-开头，说明该符号是加减号，理应保存到数字处理中，并跳出循环，但由于)压出栈时需要两个操作数，不能处理例如(-1)这种情况，因此可直接按成加减号操作，压多一个0在前面如0-1这样
					if ((m_Num.IsEmpty() || lastExp=='(') && (*chExpression=='-' || *chExpression=='+'))
					{
						double *tmp=new double;
						if (!tmp) {m_sErrorMsg=_T("内存处理错误");return FALSE;}
						*tmp=0.0;
						if (!m_Num.Push(tmp)) {m_sErrorMsg=_T("内存处理错误");delete tmp;return FALSE;}
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
						//如果当前表达式优先级大于栈中的数进行压栈
						if (GetPriority(*chExpression)>GetPriority(oper))
						{
							if (!m_Oper.Push((TCHAR *)*chExpression)) {m_sErrorMsg=_T("内存处理错误");return FALSE;}
						}
						else
						{
							//栈中的数据优先级较高，计算栈中的数据
							while(m_Oper.IsEmpty()==FALSE && m_Num.IsEmpty()==FALSE)
							{
								double *a=NULL,*b=NULL,c;
								TCHAR *o=NULL;
								//取出栈中表达式值
								if (!m_Num.Pop(b)) {m_sErrorMsg=_T("内存处理错误");return FALSE;}
								if (!m_Num.Pop(a)) {m_sErrorMsg=_T("内存处理错误");delete b;return FALSE;}
								if (!m_Oper.Pop(o)) {m_sErrorMsg=_T("内存处理错误");delete a;delete b;return FALSE;}
								//已完成()中的计算
								if((TCHAR)o=='(')
								{
									//操作数没有用到，重新压回栈中
									if (!m_Num.Push(a)) {m_sErrorMsg=_T("内存处理错误");delete a;delete b;return FALSE;}
									if (!m_Num.Push(b)) {m_sErrorMsg=_T("内存处理错误");delete b;return FALSE;}
									//如果当前字符不是')'，需要继续处理该符号，减一以实现不加当前的字符
									if (*chExpression!=')')
									{
										chExpression--;
										break;
									}
								}
								//计算栈中的表达式
								if (!WorkOut(*a,*b,&c,(TCHAR)o))
								{
									delete a;
									delete b;
									return FALSE;
								}
								delete a;
								delete b;
								//把计算结果压回栈
								double *tmp=new double;
								if (!tmp) {m_sErrorMsg=_T("内存处理错误");return FALSE;}
								*tmp=c;
								if (!m_Num.Push(tmp)) {m_sErrorMsg=_T("内存处理错误");delete tmp;return FALSE;}
								//如果当前不是')'括号,只需要计算一次结果
								if (*chExpression!=')')
								{
									//减一以实现不加当前的字符
									chExpression--;
									break;
								}
								//如果栈顶没有数据了,可以退出
								if (!m_Oper.GetTop(o))
								{
									//认为')'是多出来的，不处理
									break;
								}
								else
								{
									//栈顶已经是'('，完成()内的所有操作
									if ((TCHAR)o=='(')
									{
										if (!m_Oper.Pop(o)) {m_sErrorMsg=_T("内存处理错误");return FALSE;}
										break;
									}
								}
							}
						}
					}
					else
					{
						//证明没有操作数在栈中，直接压栈
						if (!m_Oper.Push((TCHAR *)*chExpression)) {m_sErrorMsg=_T("内存处理错误");return FALSE;}
					}
				}
			}
			lastExp=*chExpression;
			chExpression++;
		}
		sExpression.ReleaseBuffer();

		//最后的数字压栈
		if (_tcslen(NumberString)>0)
		{
			//数字压栈
			double *temp=new double;
			if (!temp) {m_sErrorMsg=_T("内存处理错误");return FALSE;}
			*temp=_tstof(NumberString);
			NumberString[0]='\0';
			if (!m_Num.Push(temp)) {m_sErrorMsg=_T("内存处理错误");delete temp;return FALSE;}
		}

		//表达式已处理完毕，计算栈中剩下的数据
		while(m_Oper.IsEmpty()==FALSE)
		{
			//还有操作符，但是已经没有操作数了，报错
			if(m_Num.IsEmpty()) {m_sErrorMsg=_T("发现多余的运算符");return FALSE;}
			double *a=NULL,*b=NULL,c;
			TCHAR *o=NULL;
			//取出栈中表达式值
			if (!m_Num.Pop(b)) {m_sErrorMsg=_T("内存处理错误");return FALSE;}
			if (!m_Num.Pop(a)) {m_sErrorMsg=_T("内存处理错误");delete a;return FALSE;}
			//丢弃()的处理
			do{
				if (!m_Oper.Pop(o)) {m_sErrorMsg=_T("内存处理错误");delete a;delete b;return FALSE;}
			}
			while((TCHAR)o=='(' || (TCHAR)o==')');
			//计算栈中的表达式
			if (!WorkOut(*a,*b,&c,(TCHAR)o))
			{
				delete a;
				delete b;
				return FALSE;
			}
			delete a;
			delete b;
			//把计算结果压回栈
			double *tmp=new double;
			if (!tmp) {m_sErrorMsg=_T("内存处理错误");return FALSE;}
			*tmp=c;
			if (!m_Num.Push(tmp)) {m_sErrorMsg=_T("内存处理错误");delete tmp;return FALSE;}
		}
		//返回最后结果
		if (m_Num.IsEmpty())
		{
			//如果表达式不需要任何计算，返回0
			*dResult=0.0;
		}
		else
		{
			double *tmpret=NULL;
			if (!m_Num.Pop(tmpret)) {m_sErrorMsg=_T("内存处理错误");return FALSE;}
			*dResult=*tmpret;
			delete tmpret;
		}
		//如果栈中还有剩下的数据，返回出错
		if (!m_Num.IsEmpty()) {m_sErrorMsg=_T("发现多余的数");return FALSE;}
		m_sErrorMsg=_T("计算成功");
		return TRUE;
	}

	THString GetLastError(){return m_sErrorMsg;}
private:
	THStack<TCHAR *> m_Oper;									///<操作符栈
	THStack<double *> m_Num;									///<操作数栈
	BOOL WorkOut(double a,double b,double *dResult,TCHAR opr)	///<计算对a,b进行操作opr的值
	{
		switch(opr)
		{
			case '+':*dResult=a+b;return TRUE;
			case '-':*dResult=a-b;return TRUE;
			case '*':*dResult=a*b;return TRUE;
			case '/':if (b==0)
					{
						m_sErrorMsg=_T("除零错误");
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
						m_sErrorMsg=_T("求余数除零错误");
						return FALSE;
					}
					else
					{
						*dResult=(double)((int)a%(int)b);
						return TRUE;
					}
			case '^':*dResult=pow(a,b);return TRUE;
		}
		m_sErrorMsg=_T("未知符号");
		return FALSE;
	}
	int GetPriority(TCHAR oper)									///<获取运算符的运算级别
	{
		if (oper=='(' || oper==')') return 0;
		else if (oper=='+' || oper=='-') return 10;
		else if (oper=='~' || oper=='%') return 20;
		else if (oper=='*' || oper=='/') return 30;
		else if (oper=='^') return 40;
		return 0;
	}
	THString m_sErrorMsg;										///<错误代码
};
