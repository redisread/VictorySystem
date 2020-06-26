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

void FileSystem::setInodeBitmap(unsigned int start, unsigned int count) {
	if (!fout.is_open())
	{
		cout << "Can`t write file,set InodeBitmap error" << endl;
		return;
	}
	fout.seekp(offset.inodeBitmap, ios::beg);
	fout.write(inodeBitmap + start, blockNum);
}

