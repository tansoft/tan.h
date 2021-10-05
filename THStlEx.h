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
	* @brief 查找最大的小于或等于t的key，注意multimap找到的key中如果有多个value，会总是返回最后一个value
	* @param m		map
	* @param t		值
	* @return		返回对应map的迭代器
	*/
	/**<pre>
		map<UINT,UINT> m_joblist;
		map<UINT,UINT>::iterator it=map_lower_key< map<UINT,UINT> >(m_joblist,(UINT)m_PriorityFlag);
		//注意区别：
		//map_lower_key是返回最大的小于或等于t的key
		//map::lower_bound是返回最小的大于或等于t的key
		//map::upper_bound是返回最小的大于t的key
		map<int,int> test;
		test[1]=1;
		test[2]=2;
		test[4]=4;
		ASSERT(test.lower_bound(1)->second==1);
		ASSERT(test.upper_bound(1)->second==2);
		ASSERT((map_lower_key< map<int,int> >(test,1)->second==1));
		ASSERT(test.lower_bound(3)->second==4);
		ASSERT(test.upper_bound(3)->second==4);
		ASSERT((map_lower_key< map<int,int> >(test,3)->second==2));//3不存在，返回小于它的2
		multimap<int,int> mtest;
		mtest.insert(multimap<int,int>::value_type(1,10));
		mtest.insert(multimap<int,int>::value_type(1,11));
		mtest.insert(multimap<int,int>::value_type(2,20));
		mtest.insert(multimap<int,int>::value_type(2,21));
		mtest.insert(multimap<int,int>::value_type(4,40));
		ASSERT(mtest.lower_bound(1)->second==10);
		ASSERT(mtest.upper_bound(1)->second==20);
		ASSERT((map_lower_key< multimap<int,int> >(mtest,1)->second==11));//注意，multimap返回的是最后一个value
		ASSERT(mtest.lower_bound(3)->second==40);
		ASSERT(mtest.upper_bound(3)->second==40);
		ASSERT((map_lower_key< multimap<int,int> >(mtest,3)->second==21));//注意，multimap返回的是最后一个value
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
	* @brief 查找最大的小于或等于t的key，注意multimap找到的key中如果有多个value，会总是返回最后一个value
	* @param m		map
	* @param t		值
	* @return		返回对应map的const迭代器
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
	* @brief 字符串替换
	* @param str		要替换的字符串，结果直接返回
	* @param old_value	要替换的值
	* @param new_value	替换成的值
	* @return 字符串本身
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
	* @brief 严格字符串替换（一直替换到不能替换为止）
	* @param str		要替换的字符串，结果直接返回
	* @param old_value	要替换的值
	* @param new_value	替换成的值
	* @return 字符串本身
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
