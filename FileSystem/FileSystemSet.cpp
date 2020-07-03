#include "FileSystem.h"

//////////////////////////////////////////////数据设置模块

// 各种核心数据的设置写入和更新
void FileSystem::updateResource() {
	if (!fp)
		return;
	fseek(fp, offset.user, SEEK_SET);
	fwrite(&this->user, userSize, 1, fp);
	fseek(fp, offset.superBlock, SEEK_SET);
	fwrite(&this->superBlock, superBlockSize, 1, fp);
	fseek(fp, offset.blockBitmap, SEEK_SET);
	fwrite(&this->blockBitmap, blockBitmapSize, 1, fp);
	fseek(fp, offset.inodeBitmap, SEEK_SET);
	fwrite(&this->inodeBitmap, inodeBitmapSize, 1, fp);
	fclose(fp);
}


/***************User数据****************/
void FileSystem::setUser(User user)
{
	if (!fp)
		return;
	// 跳转文件流指针到User位置
	fseek(fp, offset.user, SEEK_SET);
	// 写入用户数据
	fwrite(&user, userSize, 1, fp);
}

// 从文件读取用户数据
void FileSystem::getUser(User &user)
{
	if (!fp)
		return;
	// 跳转文件流指针到User位置
	fseek(fp, offset.user, SEEK_SET);
	// 读取用户数据
	fread(&user, userSize, 1, fp);
}
/*********************************************/


/***************SuperBlock数据****************/
void FileSystem::setSuperBlock(SuperBlock superBlock)
{
	if (!fp)
		return;
	fseek(fp, offset.superBlock, SEEK_SET);
	fwrite(&superBlock, superBlockSize, 1, fp);
}

void FileSystem::getSuperBlock(SuperBlock &superBlock)
{
	if (!fp)
		return;
	fseek(fp, offset.superBlock, SEEK_SET);
	fread(&superBlock, superBlockSize, 1, fp);
}
/*********************************************/


void FileSystem::setBlockBitmap(unsigned int start, unsigned int count) {
	if (!fp)
		return;
	fseek(fp, offset.blockBitmap, SEEK_SET);
	fwrite(blockBitmap + start, count, 1, fp);
}

void FileSystem::getBlockBitmap(char *blockBitmap) {
	if (!fp)
		return;
	fseek(fp, offset.blockBitmap, SEEK_SET);
	fread(blockBitmap, blockBitmapSize, 1, fp);
}

void FileSystem::setInodeBitmap(unsigned int start, unsigned int count) {
	if (!fp)
		return;
	fseek(fp, offset.inodeBitmap, SEEK_SET);
	fwrite(inodeBitmap + start, count, 1, fp);
}

void FileSystem::getInodeBitmap(char *inodeBitmap) {
	if (!fp)
		return;
	fseek(fp, offset.inodeBitmap, SEEK_SET);
	fread(inodeBitmap, inodeBitmapSize, 1, fp);
}

void FileSystem::setInode(Inode inode) {
	if (!fp)
		return;
	fseek(fp, offset.inode +inode.id * inodeSize, SEEK_SET);
	fwrite(&inode, inodeSize, 1, fp);
	fflush(fp);
}

void FileSystem::getInode(Inode &inode, I_INDEX id) {
	if (!fp)
		return;
	fseek(fp, offset.inode + id*inodeSize, SEEK_SET);
	fread(&inode, inodeSize, 1, fp);
	fflush(fp);
}

// 释放Inode节点
void FileSystem::releaseInode(I_INDEX id) {
	if (!fp)
		return;
	fseek(fp, offset.inode + id * inodeSize, SEEK_SET);
	int i;
	for (i = 0; i < inodeSize; i++)
	{
		fputc(0, fp);
	}
}


// item 地址或者文件项
unsigned int FileSystem::getItem(I_INDEX blockId, unsigned int index) {
	unsigned int value;
	if (!fp)
		return 0;
	fseek(fp, offset.block + blockId * blockSize + index * itemSize, SEEK_SET);
	fread(&value, itemSize, 1, fp);

	return value;
}

void FileSystem::releaseItem(I_INDEX blockId, I_INDEX id) {
	if (!fp)
		return;
	unsigned int itemTotal = blockSize / itemSize;
	unsigned int itemId;
	fseek(fp, offset.block + blockId * blockSize, SEEK_SET);
	for (unsigned int i = 0; i < itemTotal; ++i)
	{
		fread(&itemId, itemSize, 1, fp);
		if (itemId == id) {
			fseek(fp, -(int)(itemSize), SEEK_CUR);
			itemId = 0;
			fwrite(&itemId, itemSize, 1, fp);
			return;
		}
	}
}

void FileSystem::setItem(I_INDEX blockId, unsigned int index, unsigned int value) {
	if (!fp)
		return;
	fseek(fp, offset.block + blockId * blockSize + index * itemSize, SEEK_SET);
	fwrite(&value, itemSize, 1, fp);
}

//


//////////////FcbLink
void FileSystem::releaseFcbLink(FcbLink &fcbLink) {
	FcbLink tlink = fcbLink;
	FcbLink temp;
	while (tlink != NULL)
	{
		temp = tlink->next;
		delete tlink;
		tlink = temp;
	}
	fcbLink = NULL;
}

// 使用Inode构造FcbLink节点
void FileSystem::getFcbLink_ByInode(FcbLink fcbLink, Inode inode) {
	if (fcbLink == NULL)
		return;
	fcbLink->fcb.id = inode.id;
	strcpy(fcbLink->fcb.filename, inode.filename);
	fcbLink->fcb.filetype = inode.filetype;
	fcbLink->fcb.blockId = inode.blockId;
	fcbLink->next = NULL;
}




// Block
int FileSystem::getBlockData(I_INDEX blockId, char *buffer, unsigned int size, unsigned int offSet) {
	unsigned int len = 0;
	if (!fp || buffer == NULL || offSet > blockSize)
		return len;
	fseek(fp, offset.block + blockSize * blockId + offSet, SEEK_SET);

	if (size > blockSize - offSet)
		size = blockSize - offSet;

	len = fread(buffer, size, 1, fp);
	return len;
}

int FileSystem::writeBlockData(I_INDEX blockId, char *buffer, unsigned int size, unsigned int offSet) {
	unsigned int len = 0;
	if (!fp || buffer == NULL || offSet > blockSize)
		return len;
	fseek(fp, offset.block + blockSize * blockId + offSet, SEEK_SET);
	if (size > blockSize - offSet)
		size = blockSize - offSet;
	len = fwrite(buffer, size, 1, fp);
	return len;

}

// 释放
void FileSystem::releaseBlock(I_INDEX blockId) {
	if (!fp)
		return;
	fseek(fp, offset.block + blockId * blockSize, SEEK_SET);

	for (int i = 0; i < blockSize; ++i)
		putc(0, fp);
}

