#pragma once

#include <vector>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <string>
using namespace std;

namespace std{
	/**
	* @brief ��������С�ڻ����t��key��ע��multimap�ҵ���key������ж��value�������Ƿ������һ��value
	* @param m		map
	* @param t		ֵ
	* @return		���ض�Ӧmap�ĵ�����
	*/
	/**<pre>
		map<UINT,UINT> m_joblist;
		map<UINT,UINT>::iterator it=map_lower_key< map<UINT,UINT> >(m_joblist,(UINT)m_PriorityFlag);
		//ע������
		//map_lower_key�Ƿ�������С�ڻ����t��key
		//map::lower_bound�Ƿ�����С�Ĵ��ڻ����t��key
		//map::upper_bound�Ƿ�����С�Ĵ���t��key
		map<int,int> test;
		test[1]=1;
		test[2]=2;
		test[4]=4;
		ASSERT(test.lower_bound(1)->second==1);
		ASSERT(test.upper_bound(1)->second==2);
		ASSERT((map_lower_key< map<int,int> >(test,1)->second==1));
		ASSERT(test.lower_bound(3)->second==4);
		ASSERT(test.upper_bound(3)->second==4);
		ASSERT((map_lower_key< map<int,int> >(test,3)->second==2));//3�����ڣ�����С������2
		multimap<int,int> mtest;
		mtest.insert(multimap<int,int>::value_type(1,10));
		mtest.insert(multimap<int,int>::value_type(1,11));
		mtest.insert(multimap<int,int>::value_type(2,20));
		mtest.insert(multimap<int,int>::value_type(2,21));
		mtest.insert(multimap<int,int>::value_type(4,40));
		ASSERT(mtest.lower_bound(1)->second==10);
		ASSERT(mtest.upper_bound(1)->second==20);
		ASSERT((map_lower_key< multimap<int,int> >(mtest,1)->second==11));//ע�⣬multimap���ص������һ��value
		ASSERT(mtest.lower_bound(3)->second==40);
		ASSERT(mtest.upper_bound(3)->second==40);
		ASSERT((map_lower_key< multimap<int,int> >(mtest,3)->second==21));//ע�⣬multimap���ص������һ��value
	</pre>*/
	template<class tmap>
	typename tmap::iterator map_lower_key(tmap &m,typename const tmap::key_type &t)
	{
		typename tmap::iterator it;
		it=m.upper_bound(t);
		if (it!=m.begin())
			it--;
		else
			it=m.end();
		return it;
	}

	/**
	* @brief ��������С�ڻ����t��key��ע��multimap�ҵ���key������ж��value�������Ƿ������һ��value
	* @param m		map
	* @param t		ֵ
	* @return		���ض�Ӧmap��const������
	*/
	template<class tmap>
	typename tmap::const_iterator map_lower_key_c(const tmap &m,typename const tmap::key_type &t)
	{
		typename tmap::const_iterator it;
		it=m.upper_bound(t);
		if (it!=m.begin())
			it--;
		else
			it=m.end();
		return it;
	}

	/**
	* @brief �ַ����滻
	* @param str		Ҫ�滻���ַ��������ֱ�ӷ���
	* @param old_value	Ҫ�滻��ֵ
	* @param new_value	�滻�ɵ�ֵ
	* @return �ַ�������
	*/
	static string& str_replace(string& str,const string& old_value,const string& new_value)
	{
		while(1)
		{
			string::size_type pos(0);
			if((pos=str.find(old_value))!=string::npos)
				str.replace(pos,old_value.length(),new_value);
			else
				break;
		}
		return str;
	}

	/**
	* @brief �ϸ��ַ����滻��һֱ�滻�������滻Ϊֹ��
	* @param str		Ҫ�滻���ַ��������ֱ�ӷ���
	* @param old_value	Ҫ�滻��ֵ
	* @param new_value	�滻�ɵ�ֵ
	* @return �ַ�������
	*/
	static string& str_replace_distinct(string& str,const string& old_value,const string& new_value)
	{
		for(string::size_type pos(0);pos!=string::npos;pos+=new_value.length())
		{
			if((pos=str.find(old_value))!=string::npos)
				str.replace(pos,old_value.length(),new_value);
			else
				break;
		}
		return str;
	}
}
