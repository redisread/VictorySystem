#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstddef>
#include <malloc.h>
#include <signal.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <cassert>
#include <ctime>

// ʹ������
using std::ios;
using std::fstream;
using std::ifstream;
using std::ofstream;

using std::cin;
using std::cout;
using std::endl;
using std::string;

/******************************************�����ļ�ϵͳ�ӿ�*******************/

//************************************�ļ�ϵͳ**********************************************/

/**********1. ����һ��100M���ļ����ߴ���һ��100M�Ĺ����ڴ�
***********2. �����������һ��C����С����ʹ�ò���1�����100M�ռ䣨�����ڴ��mmap����Ȼ�������100M�ռ�Ϊһ���հ״��̣����һ���򵥵��ļ�ϵͳ��������հ״��̣�
**************�����ļ���Ŀ¼����Ļ������ݽṹ���������ļ�ϵͳ�����ṹͼ���Լ����������ӿڡ���40�֣�
***********3. �ڲ���1�Ļ�����ʵ�ֲ����ļ������ӿڲ���������Ŀ¼mkdir��ɾ��Ŀ¼rmdir���޸����ƣ������ļ�open���޸��ļ���ɾ���ļ�rm���鿴�ļ�ϵͳĿ¼�ṹls����60�֣�ע����ȫ�����ڴ���ʵ��*
***********4. �ο�����ͬ��������½ڣ�ͨ���ź�������ʵ�ֶ���ն˶������ļ�ϵͳ�Ļ�����ʣ�ϵͳ�����������̶���������д����������ֻ����һ��д�������������/

//****************************************************************************************/

/*******ʵ��Ҫ��******/
// �ļ��߼��ṹ�� ����˳���ļ��ļ��ṹ��˳���ļ��еļ�¼�ȷ�Ϊ�飬Ϊ˳���ļ�����һ������������������Ϊÿ���еĵ�һ����¼�����������¼���ļ��е�λ�����������˳����������(��������������)
// �ļ�����ṹ�� ����˳��洢
//


// - ʹ������������Bitmap,�����ڴ���Ҫ���룬����ʹ��boolҲ����Ҫռ��8���ֽ�


// ���ڴ��ļ�ʵ��

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
	WHITCH,
	RM,
	MV,
	COPY,
	CHMOD,
	LOGOUT,
	EXIT,
	CLEAR,
	SYSINFO
};


/***********************************����ṹ************************************************/

/**�ļ�ϵͳ��ṹ*/
//�Ǽ�¼���� filesystem �����Ϣ�ĵط�
typedef struct
{
	unsigned int blockSize; //4B,�ļ����С
	unsigned int blockNum;  //4B,�ļ�������
	unsigned int inodeNum;  //4B,i�ڵ�����,���ļ�����
	unsigned int blockFree; //4B,���п�����
} SuperBlock;


typedef unsigned int I_INDEX;   //����I�ڵ�����
typedef unsigned int FILE_SIZE; //�����ļ���С����
typedef unsigned int BLOCK_ID;  //���ݿ��id
typedef char FILE_NAME[30];     //�����ļ���,����30B

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

/**�ļ�i�ڵ�ṹ104��*/
typedef struct
{
	I_INDEX id;                     //4B,i�ڵ�����
	I_INDEX parent;                 //4B,��Ŀ¼i�ڵ�����
	FILE_SIZE length;               //4B,�ļ�����,unsigned int���2^32-1(4GB-1B),Ŀ¼�ļ���Ϊ���ļ�������
	BLOCK_ID blockId;               //4B,�ļ������ڵ�Ŀ¼���ݿ��id������ɾ��ʱ��λ
	unsigned int addr[12];          //12*4B,�ļ���������,�ļ�ǰ10��Ϊֱ��������Ŀ¼ǰ11��Ϊֱ������
	time_t time;                    //8B,�ļ�����޸�ʱ�䣬��1970��1��1��00ʱ00��00������������������
	FILE_NAME filename;             //30B,�ļ�������󳤶�29
	enum FileType filetype;         //1B,�ļ����� IsDir,IsFile
	enum PriorityType prioritytype; //1B,�ļ�Ȩ��,0-read-only,1-read-write
									// �ļ�������
} Inode, *PInode;

/**�ļ�������Ϣ�ڵ㣬���ڼ�Ҫ��ʾ�ļ���Ϣ:�ļ��� �ļ���Ŀ¼���ļ����ȡ��ļ�Ȩ�ޡ��޸�ʱ��
*/
typedef struct
{
	I_INDEX id;             //4B
	FILE_NAME filename;     //30B
	enum FileType filetype; //1B
	BLOCK_ID blockId;       //4B,��Ŀ¼���ݿ��е�λ��0-255
} Fcb, *PFcb;

// �����ļ��������ݽṹ
typedef struct FCB_LINK_NODE
{
	Fcb fcb;
	struct FCB_LINK_NODE *next;
} FcbLinkNode, *FcbLink;

// �����û����ݽṹ

typedef struct
{
	char username[10]; //�û�������󳤶�9
	char password[10]; //���룬��󳤶�9
} User;

/***********************************�����ṹ����************************************************/

// ����ƫ��
typedef struct
{
	unsigned int user;
	unsigned int superBlock;	 //superBlockƫ��
	unsigned int blockBitmap;	 //blockBitmapƫ��
	unsigned int inodeBitmap;	 //inodeBitmapƫ��
	unsigned int inode;			//i�ڵ���ƫ��
	unsigned int block;			// ������ƫ��
} Offset;

// ʵ���ļ�ϵͳ
class FileSystem
{
public:
	FileSystem();	//���캯�������ݳ�ʼ��
	void init();	//�ļ�ϵͳ��ز�����ʼ��
	void init_afterLogin();	//��һ�ε�¼֮��ĳ�ʼ��
	void load_afterLogin();	//�ٴμ����ļ�ϵͳ
	virtual ~FileSystem();	//��������

	// ϵͳ��ʼ����غ���
	void createSystem(); // �����ļ�ϵͳ�����粻����
	void loadSystem();   // �����ļ�ϵͳ���ڣ�ֱ�Ӽ���
	void doCommand();	// �����û������ִ��

	// �û�����
	void createUser();                          // �����û�
	void login();								// ��¼����
	void logout();								// �ǳ�����

	// ���á���ȡ����
	void setUser(User user); // �û�����	
	void getUser(User &user);
	void setSuperBlock(SuperBlock superBlock); //���ƿ�����
	void getSuperBlock(SuperBlock &superBlock);


	void setBlockBitmap(unsigned int start, unsigned int count);
	void getBlockBitmap(char *blockBitmap);
	void setInodeBitmap(unsigned int start, unsigned int count);
	void getInodeBitmap(char *inodeBitmap);

	void setInode(Inode inode);
	void getInode(Inode &inode, I_INDEX id);
	void releaseInode(I_INDEX id);

	// ��ַ��
	unsigned int getItem(I_INDEX blockId, unsigned int index);
	void releaseItem(I_INDEX blockId, unsigned int index);
	void setItem(I_INDEX blockId, unsigned int index, unsigned int value);
	

	// FcbLink
	void buildFcbLink();
	void releaseFcbLink(FcbLink &fcbLink);	// ����ָ������
	void getFcbLink_ByInode(FcbLink fcbLink, Inode inode);

	

	// �ļ���������
	void createFile();
	void readFile();
	void writeFile();

	// ��������
	void ls();
	void mkdir();
	void touch();
	void df();
	void cd();
	void cp();
	void help();
	void systemInfo();
	void clear();

	// ��ȡ�ڲ����ݽӿ�
	string getSystemName() { return SystemName; }
	//FILE *getSystemAddress() { return fp; }
	const int getBlockSize() { return blockSize; }

protected:
private:
	ifstream fin;	//�ļ�������
	ofstream fout;	//�ļ������

	char systemName[20]; //�ļ�ϵͳ���֣�Ҳ���Ǵ������ļ���
	User user;           //�û�

	const int blockSize;        //�ļ����С
	const int blockNum;         //�ļ�������
	char *blockBitmap;			//�ļ���ʹ��ͼ (������ձ�)
	char *inodeBitmap;			//i�ڵ�ʹ��ͼ (inode ���ձ�)

	//��С
	unsigned int systemSize;        // ϵͳ��С
	unsigned short userSize;        // �û��ṹ��С
	unsigned short superBlockSize;  // ���ƿ��С
	unsigned short blockBitmapSize; // ��λͼ��С
	unsigned short inodeBitmapSize; // i�ڵ�λͼ��С
	unsigned short inodeSize;       // i�ڵ��С
	unsigned short fcbSize;         // fcb��С
	unsigned short itemSize;		// 4B

	//����ƫ��
	Offset offset;

	unsigned char isAlive; //ϵͳ�Ƿ�������
	SuperBlock superBlock; //�ļ�ϵͳ����Ϣ

	// ��ǰ�ڵ���Ϣ
	string curPath;  //��ǰĿ¼�ַ���
	Inode curInode;  //��ǰĿ¼i�ڵ�
	FcbLink curLink; //��ǰĿ¼���ļ���ָ��
};

