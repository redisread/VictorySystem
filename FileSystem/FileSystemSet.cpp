#include "FileSystem.h"

//////////////////////////////////////////////��������ģ��

// ���ֺ������ݵ�����д��͸���
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


/***************User����****************/
void FileSystem::setUser(User user)
{
	if (!fp)
		return;
	// ��ת�ļ���ָ�뵽Userλ��
	fseek(fp, offset.user, SEEK_SET);
	// д���û�����
	fwrite(&user, userSize, 1, fp);
}

// ���ļ���ȡ�û�����
void FileSystem::getUser(User &user)
{
	if (!fp)
		return;
	// ��ת�ļ���ָ�뵽Userλ��
	fseek(fp, offset.user, SEEK_SET);
	// ��ȡ�û�����
	fread(&user, userSize, 1, fp);
}
/*********************************************/


/***************SuperBlock����****************/
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

// �ͷ�Inode�ڵ�
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


// item ��ַ�����ļ���
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

// �ͷ�
void FileSystem::releaseBlock(I_INDEX blockId) {
	if (!fp)
		return;
	fseek(fp, offset.block + blockId * blockSize, SEEK_SET);

	for (int i = 0; i < blockSize; ++i)
		putc(0, fp);
}

