#include "FileSystem.h"

int main()
{
	FileSystem fs;	            // �����ļ�ϵͳ�����캯�����ʼ������ڲ�����
	int result;
	result = fs.createMemory();	// ����100M���ļ�
	fs.init(result);	        // ��������ʼ����������
	return 0;
}