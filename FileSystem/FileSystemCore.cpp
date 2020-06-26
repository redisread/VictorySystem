#include "FileSystem.h"

// 构造函数，初始化文件系统
FileSystem::FileSystem() : systemSize(SystemSize), //系统大小
userSize(sizeof(User)),
superBlockSize(sizeof(SuperBlock)),
blockSize(BlockSize),
blockNum(BlockNum),
blockBitmapSize(BlockNum),
inodeBitmapSize(BlockNum),
blockBitmap(new char[BlockNum + 1]),
inodeBitmap(new char[BlockNum + 1]),
inodeSize(sizeof(Inode)),
fcbSize(sizeof(Fcb)),
itemSize(sizeof(unsigned int)),
isAlive(0),
curLink(NULL)
{
	strcpy(systemName, SystemName); // 初始化文件名

	// 设置相关偏移值，User的偏移值为0
	offset.user = 0;
	offset.superBlock = userSize;
	offset.blockBitmap = offset.superBlock + sizeof(SuperBlock);
	offset.inodeBitmap = offset.blockBitmap + blockBitmapSize;
	offset.inode = offset.inodeBitmap + inodeBitmapSize; // i节点偏移，占用内存block * sizeof(Inode)
	offset.block = offset.inode + sizeof(Inode) * blockNum;
}

void FileSystem::init() {
	//初始化文件流，打开文件
	fin.open(systemName, ios::in | ios::binary);
	if (fin.is_open()) {	// 已存在系统，文件打开成功
		fout.open(systemName, ios::out | ios::binary);	//初始化输出流
		cout << "Load the " << systemName << " Success!" << endl;
		// 登录判断
		login();
	}
	else {//不存在该系统，创建系统
		createSystem();
		cout << "First enter,Please create your account" << endl;
		// 创建用户
		createUser();
		// 登录判断
		login();
	}

	// 初始化 superBlock
	this->superBlock.blockSize = blockSize;
	this->superBlock.blockNum = blockNum;
	// 分配给root
	this->superBlock.inodeNum = 1;
	this->superBlock.blockFree = blockNum - 1;
	setSuperBlock(this->superBlock);

	int i;
	// 初始化两个位图
	this->blockBitmap[0] = 1;
	this->inodeBitmap[0] = 1;
	for (i = 1; i < blockNum; ++i)
	{
		blockBitmap[i] = inodeBitmap[i] = 0;
	}
	setBlockBitmap(0, blockBitmapSize);
	setInodeBitmap(0, inodeBitmapSize);

	// 初始化inode节点和block
	int len = (inodeSize + blockSize) * blockNum;	//总共字节长度
	if (fout.tellp() != offset.inode)
		fout.seekp(offset.inode, ios::beg);
	fout.write(0, len);

	// 初始化根目录
	this->curInode.id = 0;
	strcpy(this->curInode.filename, "/");
	this->curInode.filetype = FileType::IsDir;
	this->curInode.parent = inodeSize;
	this->curInode.length = 0;
	this->curInode.prioritytype = PriorityType::Read_Only;
	time(&(this->curInode.time));
	for (i = 0; i < 12; ++i)
		this->curInode.addr[i] = 0;
	this->curInode.blockId = 0;




}

// 不存在文件系统时创建文件系统
void FileSystem::createSystem()
{
	cout << "Create File System........" << endl;

	// 初始化文件流
	fout.open(systemName, ios::out | ios::binary);
	fout.seekp(0, ios::beg);
	fin.open(systemName, ios::in | ios::binary);
	fin.seekg(0, ios::beg);

	assert(fout.is_open() && fin.is_open());	//判断初始化文件流

	cout << "Create File System Ok!" << endl;
	cout << "Enter the VictorSystem File System..." << endl;
}



// 析构函数，释放相关的内存
FileSystem::~FileSystem()
{
	if (blockBitmap != NULL)
	{
		delete blockBitmap;
		blockBitmap = NULL;
	}
	if (inodeBitmap != NULL)
	{
		delete inodeBitmap;
		inodeBitmap = NULL;
	}
	fclose(fp);
	cout << "Exit File System" << endl;
}

