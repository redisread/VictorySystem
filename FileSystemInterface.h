#ifndef FILESYSTEM_FILESYSTEMINTERFACE
#define FILESYSTEM_FILESYSTEMINTERFACE
#include <iostream>
#include <stddef.h>
#include <sys/shm.h>
#include <unistd.h>
#include <malloc.h>
#include <signal.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
using std::cin;
using std::cout;
using std::endl;
using std::string;

/******************************************定义文件系统接口*******************/

//************************************文件系统**********************************************/

/**********1. 创建一个100M的文件或者创建一个100M的共享内存
***********2. 尝试自行设计一个C语言小程序，使用步骤1分配的100M空间（共享内存或mmap），然后假设这100M空间为一个空白磁盘，设计一个简单的文件系统管理这个空白磁盘，
**************给出文件和目录管理的基本数据结构，并画出文件系统基本结构图，以及基本操作接口。（40分）
***********3. 在步骤1的基础上实现部分文件操作接口操作，创建目录mkdir，删除目录rmdir，修改名称，创建文件open，修改文件，删除文件rm，查看文件系统目录结构ls。（60分）注明：全部在内存中实现*
***********4. 参考进程同步的相关章节，通过信号量机制实现多个终端对上述文件系统的互斥访问，系统中允许多个进程读，不允许写操作；或者只允许一个写操作，不允许读/

//****************************************************************************************/

/*******实现要点******/
// 文件逻辑结构： 索引顺序文件文件结构，顺序文件中的记录先分为组，为顺序文件建立一张索引表，在索引表里为每组中的第一个记录建立索引项，记录在文件中的位置由索引表和顺序来决定。(提升：二级索引)
// 文件物理结构： 初步顺序存储
//

// 基于大文件实现

#define KB 1024
#define MB 1024 * 1024

// 系统大小
const long SystemSize = 100 * MB;
// 盘块大小
const long BlockSize = 1 * KB;
// 系统盘块数目
const long BlockNum = SystemSize / BlockSize;
// 系统名字
const char SystemName[20] = "VictorSystem";

// 定义实现命令---1B
enum class CMD : unsigned char
{
    SUDO,
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

// 全局Key
key_t GlobalKey = 12345;

/***********************************定义结构************************************************/

/**文件系统块结构*/
//是记录整个 filesystem 相关信息的地方
typedef struct
{
    unsigned short blockSize; //2B,文件块大小
    unsigned int blockNum;    //4B,文件块数量
    unsigned int inodeNum;    //4B,i节点数量,即文件数量
    unsigned int blockFree;   //4B,空闲块数量
} SuperBlock;

//#define unsigned inode_index;  //inode节点索引

typedef unsigned int I_INDEX;   //定义I节点类型
typedef unsigned int FILE_SIZE; //定义文件大小类型
typedef unsigned int BLOCK_ID;  //数据块的id
typedef char FILE_NAME[30];     //定义文件名,长度30B

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

/**文件i节点结构100Ｂ*/
typedef struct
{
    I_INDEX id;                     //4B,i节点索引
    I_INDEX parent;                 //4B,父目录i节点索引
    FILE_NAME filename;             //30B,文件名，最大长度29
    enum FileType filetype;         //4B,文件类型 IsDir,IsFile
    FILE_SIZE length;               //4B,文件长度,unsigned int最大2^32-1(4GB-1B),目录文件则为子文件项数量
    enum PriorityType prioritytype; //1B,文件权限,0-read-only,1-read-write
    time_t time;                    //8B,文件最后修改时间，从1970年1月1日00时00分00秒至今所经过的秒数
    unsigned int addr[12];          //12*4B,文件内容索引,文件前10项为直接索引，目录前11项为直接索引
    BLOCK_ID blockId;               //文件项所在的目录数据块的id，便于删除时定位
    // 文件所有者
} Inode, *PInode;

/**文件部分信息节点，用于简要显示文件信息:文件名 文件／目录　文件长度　文件权限　修改时间
*/
typedef struct
{
    I_INDEX id;             //4B
    FILE_NAME filename;     //30B
    enum FileType filetype; //1B
    BLOCK_ID blockId;       //4B,在目录数据块中的位置0-255
} Fcb, *PFcb;

// 定义文件链接数据结构
typedef struct FCB_LINK_NODE
{
    Fcb fcb;
    struct FCB_LINK_NODE *next;
} FcbLinkNode, *FcbLink;

// 定义用户数据结构

typedef struct
{
    char username[10]; //用户名，最大长度9
    char password[10]; //密码，最大长度9
} User;

/***********************************结束定义************************************************/

// 实现内存分配操作集合
namespace Memoryhelper
{
    int createShareMemory(key_t key, long size)
    {
        int shmid = shmget(key, size * 8, 0666 | IPC_CREAT);
        return shmid;
    }
    void *matShareMemory(key_t key)
    {
        return shmat(key, 0, 0);
    }
    void releasememory(key_t key)
    {
    }

} // namespace Memoryhelper

// 实现文件系统
class FileSystem
{
public:
    FileSystem();
    virtual ~FileSystem();

    // 系统初始化相关函数
    void createSystem(); // 创建文件系统，假如不存在
    void loadSystem();   // 假如文件系统存在，直接加载

    // 文件操作命令
    void createFile(); // 创建文件
    void readFile();   //读取文件
    void writeFile();  //写文件

    // 其他命令
    void ls();
    void mkdir();
    void touch();
    void df();
    void cd();
    void cp();
    void help();

    // 获取内部数据接口
    string getSystemName() { return SystemName; }
    FILE *getSystemAddress() { return fp; }

protected:
private:
    FILE *fp;            //文件系统文件指针
    char systemName[30]; //文件系统名字，也就是创建的文件名
    User user;           //用户

    const int blockSize;        //文件块大小
    const int blockNum;         //文件块数量
    unsigned char *blockBitmap; //文件块使用图 (区块对照表)
    unsigned char *inodeBitmap; //i节点使用图 (inode 对照表)

    //大小
    unsigned int systemSize; // 系统大小
    unsigned short userSize;
    unsigned short superBlockSize;
    unsigned short blockBitmapSize;
    unsigned short inodeBitmapSize;
    unsigned short inodeSize;
    unsigned short fcbSize;
    unsigned short itemSize;
    //偏移
    unsigned long sOffset;  //superBlock偏移
    unsigned long bbOffset; //blockBitmap偏移
    unsigned long ibOffset; //inodeBitmap偏移
    unsigned long iOffset;  //i节点区偏移
    unsigned long bOffset;  //数据区偏移
    unsigned char isAlive;  //系统是否在运行

    SuperBlock superBlock; //文件系统块信息

    // 当前节点信息
    string curPath;  //当前目录字符串
    Inode curInode;  //当前目录i节点
    FcbLink curLink; //当前目录子文件链指针
};

#endif