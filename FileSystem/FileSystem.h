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

// ʹ��std����
using std::ios;
using std::fstream;
using std::ifstream;
using std::ofstream;

using std::cin;
using std::cout;
using std::endl;
using std::string;

/******************************************�����ļ�ϵͳ�ӿ�****************************/

//************************************�ļ�ϵͳ**********************************************/

/**********1. ����һ��100M���ļ����ߴ���һ��100M�Ĺ����ڴ�
***********2. �����������һ��C����С����ʹ�ò���1�����100M�ռ䣨�����ڴ��mmap����Ȼ�������100M�ռ�Ϊһ���հ״��̣����һ���򵥵��ļ�ϵͳ��������հ״��̣�
**************�����ļ���Ŀ¼����Ļ������ݽṹ���������ļ�ϵͳ�����ṹͼ���Լ����������ӿڡ���40�֣�
***********3. �ڲ���1�Ļ�����ʵ�ֲ����ļ������ӿڲ���������Ŀ¼mkdir��ɾ��Ŀ¼rmdir���޸����ƣ������ļ�open���޸��ļ���ɾ���ļ�rm���鿴�ļ�ϵͳĿ¼�ṹls����60�֣�ע����ȫ�����ڴ���ʵ��*
***********4. �ο�����ͬ��������½ڣ�ͨ���ź�������ʵ�ֶ���ն˶������ļ�ϵͳ�Ļ�����ʣ�ϵͳ�����������̶���������д����������ֻ����һ��д�������������/

//****************************************************************************************/

/*******ʵ��Ҫ��******/
// �ļ��߼��ṹ�� ����˳���ļ��ļ��ṹ��˳���ļ��еļ�¼�ȷ�Ϊ�飬Ϊ˳���ļ�����һ������������������Ϊÿ���еĵ�һ����¼�����������¼���ļ��е�λ�����������˳����������(��������������)
// �ļ�����ṹ�� ˳��洢
// ʹ������������Bitmap, �����ڴ���Ҫ���룬����ʹ��boolҲ����Ҫռ��8���ֽ�
// ���ڴ��ļ�ʵ��
/***********************/

/*******ʵ������******/
/*
1. ls �� ls -l	// ��ʾĿ¼�ļ����ļ���
2. mkdir        // ����Ŀ¼
3. open         // �����ļ�
4. rmdir        // ɾ��Ŀ¼
5. rm           // ɾ���ļ�
6. write        // д�ļ�
7. cat          // ���ļ�
8. mv           // �������ļ�
9. cd           // �л�Ŀ¼
10. help        // ����ָ��
11. chmod       // �����ļ�Ȩ��
12. account     // �����˺���Ϣ
13. logout      // �ǳ�
14. exit        // �˳�
*/

#define KB 1024
#define MB 1024 * 1024

// ϵͳ��С
const long SystemSize = 100 * MB;
// �̿��С
const long BlockSize = 1 * KB;
// ϵͳ�̿���Ŀ
const long BlockNum = SystemSize / BlockSize;
// ϵͳ���� - 20B
const char SystemName[20] = "VictorSystem";

// ����ʵ������---1B
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


/***********************************���Ͷ���************************************************/
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

typedef unsigned int I_INDEX;   //����I�ڵ�����
typedef unsigned int FILE_SIZE; //�����ļ���С����
typedef unsigned int BLOCK_ID;  //���ݿ��id
typedef char FILE_NAME[30];     //�����ļ���,����30B


/***********************************����ṹ************************************************/


////////////////enum
// �����ļ�����---1B
enum class FileType : unsigned char
{
	IsDir,
	IsFile
};

// �����ļ�Ȩ��---1B
enum class PriorityType : unsigned char
{
	Read_Only,
	Read_Write
};

///////////////��ؽṹ


//�ļ�ϵͳ��ṹ�Ǽ�¼���� filesystem �����Ϣ�ĵط�,���ڹ����ļ�
typedef struct
{
	unsigned int blockSize; //4B,�ļ����С
	unsigned int blockNum;  //4B,�ļ�������
	unsigned int inodeNum;  //4B,i�ڵ�����,���ļ�����
	unsigned int blockFree; //4B,���п�����
} SuperBlock;


// �����ļ����ź���
struct Fsig
{
	int  rw : 2;	    // 0Ϊδ��Ҳδд��1��ʾ����2��ʾд
	int curNum : 3;    // ��������ʾ��ǰ���ļ�������
	int  maxNum : 3;	// ������ʾ�����������
	Fsig() :rw(0), curNum(0), maxNum(3) {}//��������Ϊ3

	int wait(int prw) {	// �������Ϊ������д����
		if (rw == 0) {	// û�ж�д
			if (prw == 1)	// ������
				curNum++;
			//д����
			rw = prw;
			return 0;
		}
		else {
			if (rw == 1) { // ��ǰ���˶���������˶�������д
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
			else if (rw == 2) {	// ��ǰ����д���������
				return -1;
			}
		}
		return -1;
	}
	int signal(int vrw) {// �������Ϊ������д����
		if (vrw == 1)	// �ͷŶ�
		{
			curNum--;
			if (curNum<= 0) {
				rw = 0;
			}
			return 0;
		}
		else if(vrw == 2)
		{// �ͷ�д
			rw = 0;
			curNum = 0;
			return 0;
		}
		return -1;
	}

};



/**�ļ�i�ڵ�ṹ - 104��*/
typedef struct
{
	I_INDEX id;                     //4B,i�ڵ�����
	I_INDEX parent;                 //4B,��Ŀ¼��i�ڵ�����
	FILE_SIZE length;               //4B,�ļ�����
	BLOCK_ID blockId;               //4B,�ļ������ڵ�Ŀ¼���ݿ��id
	unsigned int addr[12];          //12*4B,�ļ���������,ǰ11��Ϊֱ������
	time_t time;                    //8B,�ļ�����޸�ʱ��
	char filename[24];             //30B,�ļ�������󳤶�29
	enum FileType filetype;         //1B,�ļ����� enum IsDir,IsFile
	enum PriorityType prioritytype; //1B,�ļ�Ȩ��,enum Read_Only��Read_Write
	Fsig sig;
} Inode, *PInode;

/**�ļ�������Ϣ�ڵ㣬�ɼ�Ҫ��ʾ�ļ���Ϣ:�ļ��� �ļ���Ŀ¼���ļ����ȡ��ļ�Ȩ�ޡ��޸�ʱ��
*/
typedef struct
{
	I_INDEX id;             //4B
	FILE_NAME filename;     //30B
	enum FileType filetype; //1B
	BLOCK_ID blockId;       //4B
} Fcb, *PFcb;

// �����ļ��������ݽṹ
typedef struct FCB_LINK_NODE
{
	Fcb fcb;
	struct FCB_LINK_NODE *next;
} FcbLinkNode, *FcbLink;


// �û��˺����ݽṹ
typedef struct
{
	char username[10]; //�û�������󳤶�9
	char password[10]; //���룬��󳤶�9
} User;



// ����ƫ�Ʒ�װ
typedef struct
{
	unsigned int user;			 //userƫ��
	unsigned int superBlock;	 //superBlockƫ��
	unsigned int blockBitmap;	 //blockBitmapƫ��
	unsigned int inodeBitmap;	 //inodeBitmapƫ��
	unsigned int inode;			 //indoe�ڵ���ƫ��
	unsigned int block;			 //block������ƫ��
} Offset;

/***********************************�����ṹ����************************************************/


//////////////////////�ļ�ϵͳ��ӿ�

// ʵ���ļ�ϵͳ
class FileSystem
{
public:
	//////////////////ϵͳ�ں�
	FileSystem();			                                    // ���캯�������ݳ�ʼ��
	virtual ~FileSystem();	                                    // ��������
	int createMemory();                                         // �������ļ������ڴ�
	void computeData();	                                        // ������ص�block�������͵ȵ�
	void init(int result);			                            // �ļ�ϵͳ��ز�����ʼ��
	void init_afterLogin();	                                    // ��һ�ε�¼֮��ĳ�ʼ��
	void load_afterLogin();	                                    // �ٴμ����ļ�ϵͳ
	void stop(int sig);									        // ֹͣϵͳ
	void loadSystem();                                          // �����ļ�ϵͳ���ڣ�ֱ�Ӽ���
	void createUser();                                          // �����û�
	void login();								                // ��¼����
	void logout();								                // �ǳ�����
	int updateUser();                                           // �޸��û���Ϣ

	////////////////////////////������������¡���ȡ���ͷ�
	void updateResource();                                                 // ������������
	void setUser(User user);                                               // �����û�����	
	void getUser(User &user);									           // ��ȡ�û�����
	void setSuperBlock(SuperBlock superBlock);                             // ���ÿ��ƿ�����
	void getSuperBlock(SuperBlock &superBlock);                            // ��ȡ���ƿ�����

	void setBlockBitmap(unsigned int start, unsigned int count);           // ����Blockλͼ
	void getBlockBitmap(char *blockBitmap);                                // ��ȡBlockλͼ
	void setInodeBitmap(unsigned int start, unsigned int count);           // ����Inodeλͼ
	void getInodeBitmap(char *inodeBitmap);                                // ��ȡInodeλͼ

	void setInode(Inode inode);                                            // ����Indoe�ڵ�
	void getInode(Inode &inode, I_INDEX id);                               // ��ȡInode�ڵ�
	void releaseInode(I_INDEX id);                                         // �ͷ�Inode�ڵ�

	void releaseBlock(I_INDEX blockId);                                    // �ͷ�ָ��Block
	int getBlockData(I_INDEX blockId, char *buffer, unsigned int size, unsigned int offSet);
	int writeBlockData(I_INDEX blockId, char *buffer, unsigned int size, unsigned int offSet);

	unsigned int getItem(I_INDEX blockId, unsigned int index);             // ��ȡItem��ַ����ļ���
	void releaseItem(I_INDEX blockId, I_INDEX id);                         // �ͷ�Item
	void setItem(I_INDEX blockId, unsigned int index, unsigned int value); // ����Item


	///////////////////////////////// ���ݹ��������                                                    
	void buildFcbLink(FcbLink &fcbLink, Inode inode);	                          // ����Ŀ¼����Ϣ��
	void releaseFcbLink(FcbLink &fcbLink);				                          // �ͷ�Ŀ¼����Ϣ��
	void getFcbLink_ByInode(FcbLink fcbLink, Inode inode);	                      // ����Inode��ȡFcbLink
	void removeFcbLinkInode(FcbLink fcbLink, Inode inode);	                      // �Ƴ�Inode��Ӧ��FcbLink
	void appendFcbLinkNode(FcbLink curLink, Inode inode);	                      // ����FcbLink
	unsigned int getAvailableFileItem(Inode &inode, unsigned int &availableIndex);// ��ȡ���õ��ļ���
	unsigned int findInodeOfDir(FcbLink curLink, const char *name);               // �ҵ�ָ���ļ�����InodeId
	I_INDEX getAvailableBlockId();	                                              // ��ȡ���õ�Block��Id
	I_INDEX getAvailableInodeId();                                                // ��ȡ���õ�Inode��Id


	///////////////////////////////// ʵ��ָ��ӿ�
	void doCommand();	                                // �����û������ִ��
	CMD parseCommand(string cmd_line, string &cmdName, string &param1, string &param2, string &param3);
	void showCurPath();                                 // ��ʾ��ǰ·��
	void showFileName(FcbLink fcbLink);                 // ��ʾ�ļ���
	void showFileInfo(FcbLink fcbLink);                 // ��ʾ�ļ�����

	// ָ��ӿ�
	void ls();	                                        // ��ʾĿ¼���ļ���
	void ls_l();                                        // ��ʾĿ¼��ϸ��Ϣ
	void mkdir();	                                    // ����Ŀ¼
	void touch();	                                    // �����ļ�
	int cd(string dirName);                             // ����Ŀ¼
	int mv(string fileName, string newName);			// �������ļ�
	int cp(string src, string dest);	                // �����ļ�
	int chmod(string fileName, PriorityType pType);	    // �����ļ�Ȩ��
	void help();										// ����ָ��
	void systemInfo();									// �鿴�ļ�ϵͳ��Ϣ
	void clear();                                       // ����
	void exit();	                                    // �˳�

	// �ļ���������
	int createFile(string filename, FileType ftype);
	int deleteFile(string filename);
	int readFile(string filename,I_INDEX &sid);
	int writeFile(string filename, I_INDEX &sid);

	// �ȴ����뺯��
	int waitInput(string &buffer);
	unsigned int waitForInput(char *buff, unsigned int limit);

private:
	FILE *fp;	                                        // �ļ���ָ��
	char systemName[20];                                // �ļ�ϵͳ���֣�Ҳ���Ǵ������ļ���
	User user;                                          // ϵͳ�û�
	char *blockBitmap;			                        // �ļ���λͼ (Block���ձ�)
	char *inodeBitmap;			                        // i�ڵ�λͼ  (Inode���ձ�)
	const int blockSize;                                // �ļ����С
	const int blockNum;                                 // �ļ�������
	unsigned int systemSize;                            // ϵͳ��С
	unsigned int userSize;                              // �û��ṹ��С
	unsigned int superBlockSize;                        // ���ƿ��С
	unsigned int blockBitmapSize;                       // ��λͼ��С
	unsigned int inodeBitmapSize;                       // i�ڵ�λͼ��С
	unsigned int inodeSize;                             // i�ڵ��С
	unsigned int fcbSize;                               // fcb��С
	unsigned int itemSize;		                        // 4B
	//////////////////////ƫ���Լ����ƿ�                                           
	Offset offset;                                      //����ƫ��
	SuperBlock superBlock;                              //�ļ�ϵͳ����Ϣ
	////////////////////////��ǰ�ڵ���Ϣ
	string curPath;                                     //��ǰĿ¼�ַ���
	Inode curInode;                                     //��ǰĿ¼i�ڵ�
	FcbLink curLink;                                    //��ǰĿ¼���ļ���ָ��
};

#endif // !FILESYSTEM_H