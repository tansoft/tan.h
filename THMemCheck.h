#pragma once

void * operator new (size_t size)
{
	//������ҳ�����
	int page_num = (int) ( size / 4096 + 1 );
	//ƫ����
	size_t offset = page_num * 4096 - size;
	//���ڴ��ĺ��洴��һ������ı���ҳ��,���ҽ�ҳ�����������Ϊ���ɶ�д
	void *p = VirtualAlloc(NULL,page_num*4096+1,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	//��λ���һ������ҳ��ĵ�ַ
	void *pchecker = (char *)p + page_num*4096;
	//�������һҳΪ���ɶ�д
	DWORD old_value;
	VirtualProtect(pchecker,4096,PAGE_NOACCESS,&old_value);
	return (char *)p + offset;
}
void operator delete (void *p)
{
	//�ҵ�VirtualAllocʱ�����ʼ��ַ
	p = (char *)p - (size_t)p % 4096;
	VirtualFree(p,0,MEM_RELEASE);
}