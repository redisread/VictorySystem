#include "FileSystem.h"

// ���캯������ʼ���ļ�ϵͳ
FileSystem::FileSystem() : systemSize(SystemSize), //ϵͳ��С
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
	strcpy(systemName, SystemName); // ��ʼ���ļ���

	// �������ƫ��ֵ��User��ƫ��ֵΪ0
	offset.user = 0;
	offset.superBlock = userSize;
	offset.blockBitmap = offset.superBlock + sizeof(SuperBlock);
	offset.inodeBitmap = offset.blockBitmap + blockBitmapSize;
	offset.inode = offset.inodeBitmap + inodeBitmapSize; // i�ڵ�ƫ�ƣ�ռ���ڴ�block * sizeof(Inode)
	offset.block = offset.inode + sizeof(Inode) * blockNum;
}

void FileSystem::init() {
	//��ʼ���ļ��������ļ�
	fin.open(systemName, ios::in | ios::binary);
	if (fin.is_open()) {	// �Ѵ���ϵͳ���ļ��򿪳ɹ�
		fout.open(systemName, ios::out | ios::binary);	//��ʼ�������
		cout << "Load the " << systemName << " Success!" << endl;
		// ��¼�ж�
		login();
	}
	else {//�����ڸ�ϵͳ������ϵͳ
		createSystem();
		cout << "First enter,Please create your account" << endl;
		// �����û�
		createUser();
		// ��¼�ж�
		login();
	}

	// ��ʼ�� superBlock
	this->superBlock.blockSize = blockSize;
	this->superBlock.blockNum = blockNum;
	// �����root
	this->superBlock.inodeNum = 1;
	this->superBlock.blockFree = blockNum - 1;
	setSuperBlock(this->superBlock);

	int i;
	// ��ʼ������λͼ
	this->blockBitmap[0] = 1;
	this->inodeBitmap[0] = 1;
	for (i = 1; i < blockNum; ++i)
	{
		blockBitmap[i] = inodeBitmap[i] = 0;
	}
	setBlockBitmap(0, blockBitmapSize);
	setInodeBitmap(0, inodeBitmapSize);

	// ��ʼ��inode�ڵ��block
	int len = (inodeSize + blockSize) * blockNum;	//�ܹ��ֽڳ���
	if (fout.tellp() != offset.inode)
		fout.seekp(offset.inode, ios::beg);
	fout.write(0, len);

	// ��ʼ����Ŀ¼
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

// �������ļ�ϵͳʱ�����ļ�ϵͳ
void FileSystem::createSystem()
{
	cout << "Create File System........" << endl;

	// ��ʼ���ļ���
	fout.open(systemName, ios::out | ios::binary);
	fout.seekp(0, ios::beg);
	fin.open(systemName, ios::in | ios::binary);
	fin.seekg(0, ios::beg);

	assert(fout.is_open() && fin.is_open());	//�жϳ�ʼ���ļ���

	cout << "Create File System Ok!" << endl;
	cout << "Enter the VictorSystem File System..." << endl;
}



// �����������ͷ���ص��ڴ�
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

