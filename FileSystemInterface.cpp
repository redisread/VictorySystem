#include "FileSystemInterface.h"

// 初始化文件系统
FileSystem::FileSystem() : systemSize(SystemSize), //系统大小
                           blockSize(BlockSize),
                           blockNum(BlockNum),
                           blockBitmap(new unsigned char[BlockNum + 1]),
                           inodeBitmap(new unsigned char[BlockNum + 1]),
                           isAlive(0),
                           fp(NULL),
                           curLink(NULL)
{

    strcpy(systemName, SystemName); // 初始化文件名

    // 初始化相关数据
    userSize = sizeof(User);
    superBlockSize = sizeof(SuperBlock);
    blockBitmapSize = blockNum;
    inodeBitmapSize = blockNum;
    inodeSize = sizeof(Inode);
    fcbSize = sizeof(Fcb);
    itemSize = sizeof(unsigned int);

    // 设置偏移值
    sOffset = userSize;
    bbOffset = sOffset + sizeof(SuperBlock);
    ibOffset = bbOffset + blockBitmapSize;
    iOffset = ibOffset + inodeBitmapSize;
    bOffset = iOffset + sizeof(Inode) * blockNum;
}

void FileSystem::createSystem()
{

    cout << "Create File System........" << endl;

    // 初始化文件指针
    if ((fp = fopen(systemName, "wb+")) == NULL)
    {
        cout << "Failed to create File System!" << endl;
    }
    cout << "Create File System Ok!" << endl;
    cout << "Enter the VictorSystem File System..." << endl;

    // 创建用户
    cout << "First enter,Please create your account" << endl;
    cout << "UserName:";
    cin >> user.username;
    cout << "PassWord:";
    cin >> user.password;
    cout << "Create your account......" << endl;
    cout << "Create ok! UserName：" << user.username << "  PassWord: " << user.password << endl;

    // 初始化 superBlock
    superBlock.blockSize = blockSize;
    superBlock.blockNum = blockNum;

    
}
FileSystem::~FileSystem()
{
    fclose(fp);
    cout << "Exit File System" << endl;
}

void FileSystem::help()
{
    printf("command: \n\
    help    ---  show help menu \n\
    sysinfo ---  show system base information \n\
    logout  ---  exit user \n\
    account ---  modify username and password \n\
    clear   ---  clear the screen \n\
    ls      ---  list the digest of the directory's children \n\
    ls -l   ---  list the detail of the directory's children \n\
    cd      ---  change directory \n\
    mkdir   ---  make directory   \n\
    touch   ---  create a new file \n\
    cat     ---  read a file \n\
    write   ---  write something to a file \n\
    rm      ---  delete a directory or a file \n\
    cp      ---  cp a directory file to another directory or file (not finish)\n\
    mv      ---  rename a file or directory \n\
    chmod   ---  change the authorizatino of a directory or a file \n\
    exit    ---  exit this system\n");
}