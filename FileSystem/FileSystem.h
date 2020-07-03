#pragma once
#define _CRT_SECURE_NO_WARNINGS
#ifndef FILESYSTEM_H
#define FILESYSTEM_H


#include <iostream>
#include <windows.h>
#include <cstddef>
#include <malloc.h>
#include <signal.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <cassert>
#include <ctime>
#include <map>
#include <vector>

// 使用std声明
using std::ios;
using std::fstream;
using std::ifstream;
using std::ofstream;

using std::cin;
using std::cout;
using std::endl;
using std::string;

/******************************************定义文件系统接口****************************/

//************************************文件系统**********************************************/

/**********1. 创建一个100M的文件或者创建一个100M的共享内存
***********2. 尝试自行设计一个C语言小程序，使用步骤1分配的100M空间（共享内存或mmap），然后假设这100M空间为一个空白磁盘，设计一个简单的文件系统管理这个空白磁盘，
**************给出文件和目录管理的基本数据结构，并画出文件系统基本结构图，以及基本操作接口。（40分）
***********3. 在步骤1的基础上实现部分文件操作接口操作，创建目录mkdir，删除目录rmdir，修改名称，创建文件open，修改文件，删除文件rm，查看文件系统目录结构ls。（60分）注明：全部在内存中实现*
***********4. 参考进程同步的相关章节，通过信号量机制实现多个终端对上述文件系统的互斥访问，系统中允许多个进程读，不允许写操作；或者只允许一个写操作，不允许读/

//****************************************************************************************/

/*******实现要点******/
// 文件逻辑结构： 索引顺序文件文件结构，顺序文件中的记录先分为组，为顺序文件建立一张索引表，在索引表里为每组中的第一个记录建立索引项，记录在文件中的位置由索引表和顺序来决定。(提升：二级索引)
// 文件物理结构： 顺序存储
// 使用数组来管理Bitmap, 由于内存需要对齐，就算使用bool也是需要占用8个字节
// 基于大文件实现
/***********************/

/*******实现命令******/
/*
1. ls 与 ls -l	// 显示目录文件与文件夹
2. mkdir        // 创建目录
3. open         // 创建文件
4. rmdir        // 删除目录
5. rm           // 删除文件
6. write        // 写文件
7. cat          // 读文件
8. mv           // 重命名文件
9. cd           // 切换目录
10. help        // 帮助指令
11. chmod       // 更改文件权限
12. account     // 更改账号信息
13. logout      // 登出
14. exit        // 退出
*/

#define KB 1024
#define MB 1024 * 1024

// 系统大小
const long SystemSize = 100 * MB;
// 盘块大小
const long BlockSize = 1 * KB;
// 系统盘块数目
const long BlockNum = SystemSize / BlockSize;
// 系统名字 - 20B
const char SystemName[20] = "VictorSystem";

// 定义实现命令---1B
enum class CMD : unsigned int
{
	SUDO = 1,
	HELP,
	LS,
	CD,
	MKDIR,
	TOUCH,
	CAT,
	WRITE,
	WHITCH,
	RM,
	MV,
	COPY,
	CHMOD,
	LOGOUT,
	EXIT,
	CLEAR,
	SYSINFO,
	ACCOUNT,
	EMPTY,
	NONE
};

inline const char* getCommandName(CMD cmd) {
	switch (cmd)
	{
	case CMD::SUDO:
		return "SUDO";
	case CMD::HELP:
		return "HELP";
	case CMD::LS:
		return "LS";
	case CMD::CD:
		return "CD";
	case CMD::MKDIR:
		return "MKDIR";
	case CMD::TOUCH:
		return "TOUCH";
	case CMD::CAT:
		return "CAT";
	case CMD::WHITCH:
		break;
	case CMD::RM:
		break;
	case CMD::MV:
		break;
	case CMD::COPY:
		break;
	case CMD::CHMOD:
		break;
	case CMD::LOGOUT:
		break;
	case CMD::EXIT:
		break;
	case CMD::CLEAR:
		break;
	case CMD::SYSINFO:
		break;
	case CMD::ACCOUNT:
		break;
	default:
		return "";
		break;
	}
}


/***********************************类型定义************************************************/
inline CMD getCmdType(const string cmdName) {
	if (cmdName == "sudo")
		return CMD::SUDO;
	else if (cmdName == "cd")
		return CMD::CD;
	else if (cmdName == "exit")
		return CMD::EXIT;
	else if (cmdName == "mv")
		return CMD::MV;
	else if (cmdName == "write")
		return CMD::WRITE;
	else if (cmdName == "rm" || cmdName == "rmdir")
		return CMD::RM;
	else if (cmdName == "account")
		return CMD::ACCOUNT;
	else if (cmdName == "help")
		return CMD::HELP;
	else if (cmdName == "ls")
		return CMD::LS;
	else if (cmdName == "mkdir")
		return CMD::MKDIR;
	else if (cmdName == "open")
		return CMD::TOUCH;
	else if (cmdName == "cat")
		return CMD::CAT;
	else if (cmdName == "cp")
		return CMD::COPY;
	else if (cmdName == "chmod")
		return CMD::CHMOD;
	else if (cmdName == "logout")
		return CMD::LOGOUT;
	else if (cmdName == "clear")
		return CMD::CLEAR;
	else if (cmdName == "sysinfo")
		return CMD::SYSINFO;
	else if (cmdName == "")
		return CMD::EMPTY;
	return CMD::NONE;
}

typedef unsigned int I_INDEX;   //定义I节点类型
typedef unsigned int FILE_SIZE; //定义文件大小类型
typedef unsigned int BLOCK_ID;  //数据块的id
typedef char FILE_NAME[30];     //定义文件名,长度30B


/***********************************定义结构************************************************/


////////////////enum
// 定义文件类型---1B
enum class FileType : unsigned char
{
	IsDir,
	IsFile
};

// 定义文件权限---1B
enum class PriorityType : unsigned char
{
	Read_Only,
	Read_Write
};

///////////////相关结构


//文件系统块结构是记录整个 filesystem 相关信息的地方,用于管理文件
typedef struct
{
	unsigned int blockSize; //4B,文件块大小
	unsigned int blockNum;  //4B,文件块数量
	unsigned int inodeNum;  //4B,i节点数量,即文件数量
	unsigned int blockFree; //4B,空闲块数量
} SuperBlock;


// 控制文件的信号量
struct Fsig
{
	int  rw : 2;	    // 0为未读也未写，1表示读，2表示写
	int curNum : 3;    // 若读，表示当前读文件的数量
	int  maxNum : 3;	// 若读表示读的最大数量
	Fsig() :rw(0), curNum(0), maxNum(3) {}//最大读者数为3

	int wait(int prw) {	// 传入参数为读或者写请求
		if (rw == 0) {	// 没有读写
			if (prw == 1)	// 读操作
				curNum++;
			//写操作
			rw = prw;
			return 0;
		}
		else {
			if (rw == 1) { // 当前有人读，允许多人读不允许写
				if (prw == 2)
					return -1;
				if (curNum + 1 <= maxNum)
				{
					curNum++;
					return 0;
				}
				else {
					return -1;
				}
			}
			else if (rw == 2) {	// 当前有人写，不允许读
				return -1;
			}
		}
		return -1;
	}
	int signal(int vrw) {// 传入参数为读或者写请求
		if (vrw == 1)	// 释放读
		{
			curNum--;
			if (curNum<= 0) {
				rw = 0;
			}
			return 0;
		}
		else if(vrw == 2)
		{// 释放写
			rw = 0;
			curNum = 0;
			return 0;
		}
		return -1;
	}

};



/**文件i节点结构 - 104Ｂ*/
typedef struct
{
	I_INDEX id;                     //4B,i节点索引
	I_INDEX parent;                 //4B,父目录的i节点索引
	FILE_SIZE length;               //4B,文件长度
	BLOCK_ID blockId;               //4B,文件项所在的目录数据块的id
	unsigned int addr[12];          //12*4B,文件内容索引,前11项为直接索引
	time_t time;                    //8B,文件最后修改时间
	char filename[24];             //30B,文件名，最大长度29
	enum FileType filetype;         //1B,文件类型 enum IsDir,IsFile
	enum PriorityType prioritytype; //1B,文件权限,enum Read_Only，Read_Write
	Fsig sig;
} Inode, *PInode;

/**文件部分信息节点，可简要显示文件信息:文件名 文件／目录　文件长度　文件权限　修改时间
*/
typedef struct
{
	I_INDEX id;             //4B
	FILE_NAME filename;     //30B
	enum FileType filetype; //1B
	BLOCK_ID blockId;       //4B
} Fcb, *PFcb;

// 定义文件链接数据结构
typedef struct FCB_LINK_NODE
{
	Fcb fcb;
	struct FCB_LINK_NODE *next;
} FcbLinkNode, *FcbLink;


// 用户账号数据结构
typedef struct
{
	char username[10]; //用户名，最大长度9
	char password[10]; //密码，最大长度9
} User;



// 各种偏移封装
typedef struct
{
	unsigned int user;			 //user偏移
	unsigned int superBlock;	 //superBlock偏移
	unsigned int blockBitmap;	 //blockBitmap偏移
	unsigned int inodeBitmap;	 //inodeBitmap偏移
	unsigned int inode;			 //indoe节点区偏移
	unsigned int block;			 //block数据区偏移
} Offset;

/***********************************结束结构定义************************************************/


//////////////////////文件系统类接口

// 实现文件系统
class FileSystem
{
public:
	//////////////////系统内核
	FileSystem();			                                    // 构造函数，数据初始化
	virtual ~FileSystem();	                                    // 析构函数
	int createMemory();                                         // 创建大文件共享内存
	void computeData();	                                        // 计算相关的block的数量和等等
	void init(int result);			                            // 文件系统相关操作初始化
	void init_afterLogin();	                                    // 第一次登录之后的初始化
	void load_afterLogin();	                                    // 再次加载文件系统
	void stop(int sig);									        // 停止系统
	void loadSystem();                                          // 假如文件系统存在，直接加载
	void createUser();                                          // 创建用户
	void login();								                // 登录操作
	void logout();								                // 登出操作
	int updateUser();                                           // 修改用户信息

	////////////////////////////数据设置与更新、获取与释放
	void updateResource();                                                 // 更新所有数据
	void setUser(User user);                                               // 设置用户数据	
	void getUser(User &user);									           // 获取用户数据
	void setSuperBlock(SuperBlock superBlock);                             // 设置控制块数据
	void getSuperBlock(SuperBlock &superBlock);                            // 获取控制块数据

	void setBlockBitmap(unsigned int start, unsigned int count);           // 设置Block位图
	void getBlockBitmap(char *blockBitmap);                                // 获取Block位图
	void setInodeBitmap(unsigned int start, unsigned int count);           // 设置Inode位图
	void getInodeBitmap(char *inodeBitmap);                                // 获取Inode位图

	void setInode(Inode inode);                                            // 设置Indoe节点
	void getInode(Inode &inode, I_INDEX id);                               // 获取Inode节点
	void releaseInode(I_INDEX id);                                         // 释放Inode节点

	void releaseBlock(I_INDEX blockId);                                    // 释放指定Block
	int getBlockData(I_INDEX blockId, char *buffer, unsigned int size, unsigned int offSet);
	int writeBlockData(I_INDEX blockId, char *buffer, unsigned int size, unsigned int offSet);

	unsigned int getItem(I_INDEX blockId, unsigned int index);             // 获取Item地址项或文件项
	void releaseItem(I_INDEX blockId, I_INDEX id);                         // 释放Item
	void setItem(I_INDEX blockId, unsigned int index, unsigned int value); // 设置Item


	///////////////////////////////// 数据构建与查找                                                    
	void buildFcbLink(FcbLink &fcbLink, Inode inode);	                          // 构建目录的信息链
	void releaseFcbLink(FcbLink &fcbLink);				                          // 释放目录的信息链
	void getFcbLink_ByInode(FcbLink fcbLink, Inode inode);	                      // 更具Inode获取FcbLink
	void removeFcbLinkInode(FcbLink fcbLink, Inode inode);	                      // 移除Inode对应的FcbLink
	void appendFcbLinkNode(FcbLink curLink, Inode inode);	                      // 插入FcbLink
	unsigned int getAvailableFileItem(Inode &inode, unsigned int &availableIndex);// 获取可用的文件项
	unsigned int findInodeOfDir(FcbLink curLink, const char *name);               // 找到指定文件名的InodeId
	I_INDEX getAvailableBlockId();	                                              // 获取可用的Block的Id
	I_INDEX getAvailableInodeId();                                                // 获取可用的Inode的Id


	///////////////////////////////// 实现指令接口
	void doCommand();	                                // 接受用户命令并且执行
	CMD parseCommand(string cmd_line, string &cmdName, string &param1, string &param2, string &param3);
	void showCurPath();                                 // 显示当前路径
	void showFileName(FcbLink fcbLink);                 // 显示文件名
	void showFileInfo(FcbLink fcbLink);                 // 显示文件详情

	// 指令接口
	void ls();	                                        // 显示目录下文件名
	void ls_l();                                        // 显示目录详细信息
	void mkdir();	                                    // 创建目录
	void touch();	                                    // 创建文件
	int cd(string dirName);                             // 进入目录
	int mv(string fileName, string newName);			// 重命名文件
	int cp(string src, string dest);	                // 复制文件
	int chmod(string fileName, PriorityType pType);	    // 更改文件权限
	void help();										// 帮助指令
	void systemInfo();									// 查看文件系统信息
	void clear();                                       // 清屏
	void exit();	                                    // 退出

	// 文件操作命令
	int createFile(string filename, FileType ftype);
	int deleteFile(string filename);
	int readFile(string filename,I_INDEX &sid);
	int writeFile(string filename, I_INDEX &sid);

	// 等待输入函数
	int waitInput(string &buffer);
	unsigned int waitForInput(char *buff, unsigned int limit);

private:
	FILE *fp;	                                        // 文件流指针
	char systemName[20];                                // 文件系统名字，也就是创建的文件名
	User user;                                          // 系统用户
	char *blockBitmap;			                        // 文件块位图 (Block对照表)
	char *inodeBitmap;			                        // i节点位图  (Inode对照表)
	const int blockSize;                                // 文件块大小
	const int blockNum;                                 // 文件块数量
	unsigned int systemSize;                            // 系统大小
	unsigned int userSize;                              // 用户结构大小
	unsigned int superBlockSize;                        // 控制块大小
	unsigned int blockBitmapSize;                       // 块位图大小
	unsigned int inodeBitmapSize;                       // i节点位图大小
	unsigned int inodeSize;                             // i节点大小
	unsigned int fcbSize;                               // fcb大小
	unsigned int itemSize;		                        // 4B
	//////////////////////偏移以及控制块                                           
	Offset offset;                                      //各种偏移
	SuperBlock superBlock;                              //文件系统块信息
	////////////////////////当前节点信息
	string curPath;                                     //当前目录字符串
	Inode curInode;                                     //当前目录i节点
	FcbLink curLink;                                    //当前目录子文件链指针
};

#endif // !FILESYSTEM_H