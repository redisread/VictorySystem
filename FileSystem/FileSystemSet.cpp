#include "FileSystem.h"

// ���ֺ������ݵ�����д��͸���



/***************User����****************/
void FileSystem::setUser(User user)
{
	if (!fout.is_open())
	{
		cout << "Can`t write file,set user error" << endl;
		return;
	}
	// ��ת�ļ���ָ�뵽Userλ��
	fout.seekp(offset.user, ios::beg);
	// д���û�����
	fout.write((char*)(&user), sizeof(User));
}

// ���ļ���ȡ�û�����
void FileSystem::getUser(User &user)
{
	if (!fin.is_open()) {
		cout << "Can`t read the file,get user error" << endl;
		return;
	}
	// ��ת�ļ���ָ�뵽Userλ��
	fin.seekg(offset.user, ios::beg);
	// ��ȡ�û�����
	fin.read((char*)(&user), sizeof(User));
}
/*********************************************/


/***************SuperBlock����****************/
void FileSystem::setSuperBlock(SuperBlock superBlock)
{
	if (!fout.is_open())
	{
		cout << "Can`t write file,set superBlock error" << endl;
		return;
	}
	fout.seekp(offset.superBlock, ios::beg);
	fout.write((char*)(&superBlock), this->superBlockSize);
}

void FileSystem::getSuperBlock(SuperBlock &superBlock)
{
	if (!fin.is_open()) {
		cout << "Can`t read the file,get superBlock error" << endl;
		return;
	}
	fin.seekg(offset.superBlock, ios::beg);
	fin.read((char*)(&superBlock), this->superBlockSize);
}
/*********************************************/


void FileSystem::setBlockBitmap(unsigned int start, unsigned int count) {
	if (!fout.is_open())
	{
		cout << "Can`t write file,set BlockBitmap error" << endl;
		return;
	}
	fout.seekp(offset.blockBitmap, ios::beg);
	fout.write(blockBitmap + start, blockNum);
}

void FileSystem::getBlockBitmap(char *blockBitmap) {
	if (!fin.is_open())
		return;
	fin.seekg(offset.blockBitmap, ios::beg);
	fin.read(blockBitmap, blockBitmapSize);
}

void FileSystem::setInodeBitmap(unsigned int start, unsigned int count) {
	if (!fout.is_open())
	{
		cout << "Can`t write file,set InodeBitmap error" << endl;
		return;
	}
	fout.seekp(offset.inodeBitmap, ios::beg);
	fout.write(inodeBitmap + start, blockNum);
}

void FileSystem::getInodeBitmap(char *inodeBitmap) {
	if (!fin.is_open())
		return;
	fin.seekg(offset.inode, ios::beg);
	fin.read(inodeBitmap, inodeBitmapSize);
}

void FileSystem::setInode(Inode inode) {
	if (!fout.is_open())
		return;
	fout.seekp(offset.inode + inode.id * inodeSize, ios::beg);
	fout.write((char*)&inode, inodeSize);
}

void FileSystem::getInode(Inode &inode, I_INDEX id) {
	if (!fin.is_open())
		return;
	fin.seekg(offset.inode + id * inodeSize, ios::beg);
	fin.read((char*)&inode, inodeSize);
}

// �ͷ�Inode�ڵ�
void FileSystem::releaseInode(I_INDEX id) {
	if (!fout.is_open())
		return;
	fout.seekp(offset.inode + id * inodeSize, ios::beg);
	fout.write(0, inodeSize);
}


// item ��ַ�����ļ���
unsigned int FileSystem::getItem(I_INDEX blockId, unsigned int index) {
	unsigned int value;
	if (!fin.is_open())
		return;
	fin.seekg(offset.block + blockId * blockSize + index * itemSize, ios::beg);
	fin.read((char*)&value, itemSize);
	return value;
}

void FileSystem::releaseItem(I_INDEX blockId, unsigned int index) {
	if (!fout.is_open())
		return;
	fout.seekp(offset.block + blockId * blockSize + index * itemSize, ios::beg);
	fout.write(0, itemSize);
}

void FileSystem::setItem(I_INDEX blockId, unsigned int index, unsigned int value) {
	if (!fout.is_open())
		return;
	fout.seekp(offset.block + blockId * blockSize + index * itemSize, ios::beg);
	fout.write((char*)&value, itemSize);
}

//


//////////////FcbLink
void FileSystem::releaseFcbLink(FcbLink &fcbLink) {
	FcbLink tlink = fcbLink;
	FcbLink temp;
	while (tlink != NULL)
	{
		temp =  tlink->next;
		delete tlink;
		tlink = temp;
	}
	fcbLink = NULL;
}

// ʹ��Inode����FcbLink�ڵ�
void FileSystem::getFcbLink_ByInode(FcbLink fcbLink, Inode inode) {
	if (fcbLink == NULL)
		return;
	fcbLink->fcb.id = inode.id;
	strcpy(fcbLink->fcb.filename, inode.filename);
	fcbLink->fcb.filetype = inode.filetype;
	fcbLink->fcb.blockId = inode.blockId;
	fcbLink->next = NULL;
}