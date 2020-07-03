#include "FileSystem.h"

int main()
{
	FileSystem fs;	            // 创建文件系统，构造函数会初始化相关内部数据
	int result;
	result = fs.createMemory();	// 创建100M大文件
	fs.init(result);	        // 传入结果初始化并且运行
	return 0;
}