#include "FileSystem.h"

///////////////////////////////////////�ļ�������ģ��
// �����ļ�
int FileSystem::createFile(string filename, FileType ftype) {

	if (filename.empty() || filename.length() == 0 || findInodeOfDir(curLink, filename.c_str()))
	{	// �����Ѵ����ļ������ļ������Ϸ�
		cout << "invalid file name:the name is empty,or the file has existed" << endl;
		return -1;
	}
	unsigned int index;
	I_INDEX dirBlockId = getAvailableFileItem(curInode, index);
	if (dirBlockId > 0 || curInode.id == 0) {
		I_INDEX blockId = getAvailableBlockId();
		if (blockId > 0) {
			superBlock.blockFree--;
			setSuperBlock(superBlock);
			blockBitmap[blockId] = 1;
			setBlockBitmap(blockId, 1);
			I_INDEX inodeId = getAvailableInodeId();
			//����i�ڵ�
			Inode newInode;
			newInode.id = inodeId;
			strcpy(newInode.filename, filename.c_str());
			newInode.filetype = ftype;
			newInode.parent = curInode.id;
			newInode.length = 0;
			newInode.prioritytype = PriorityType::Read_Write;
			time(&(newInode.time));
			unsigned int i;

			newInode.addr[0] = blockId;
			for (i = 1; i < 12; ++i)
			{
				newInode.addr[i] = 0;
			}

			newInode.blockId = dirBlockId;
			setInode(newInode);
			inodeBitmap[inodeId] = 1;
			setInodeBitmap(inodeId, 1);

			setItem(dirBlockId, index, inodeId);	// �洢Inode�ڵ�Id

			// ����i�ڵ�
			curInode.length++;
			time(&(curInode.time));
			setInode(curInode);

			// ����curLink
			appendFcbLinkNode(curLink, newInode);
			return 0;

		}
		else
		{
			cout << "storage space is not enough, " << blockId << endl;
		}
	}
	else
	{
		cout << "the directory can't append file item, " << dirBlockId << endl;
		return -1;
	}


}


// ��ȡ�ļ�����
int FileSystem::readFile(string filename, I_INDEX &sid)
{

	I_INDEX inodeId = findInodeOfDir(curLink, filename.c_str());
	if (inodeId > 0)
	{
		Inode inode;
		getInode(inode, inodeId);
		{
			int r = inode.sig.wait(1);	// ������Դ
			if (r == -1)
				return -1;
			sid = inodeId;
			setInode(inode);	//����i�ڵ��ź���
			Inode ainode;
			getInode(ainode, inode.id);
			cout << "�����ļ�" << " ��ǰ������" << ainode.sig.curNum << endl;
			
		}
		time(&(inode.time));
		if (inode.filetype == FileType::IsFile)
		{// �ļ�
			unsigned int len = inode.length;
			if (len <= 0)
			{
				cout << endl;
				return 1;
			}
			char *buffer = new char[blockSize + 1];
			for (unsigned int j = 0; j < blockSize + 1; ++j)
				buffer[j] = '\0';
			unsigned int i;
			I_INDEX blockId;
			// ����ֱ��������ȡ����
			for (i = 0; i < 11; ++i)
			{
				blockId = inode.addr[i];
				if (blockId > 0)
				{
					if (len > blockSize)
					{
						len -= getBlockData(blockId, buffer, blockSize, 0);
						cout << buffer;	// ���
					}
					else
					{ // ���
						len -= getBlockData(blockId, buffer, len, 0);
						cout << buffer;
						delete buffer;
						return 0;
					}
				}
				else
				{// ��ȡ���
					cout << endl;
					delete buffer;
					return 0;
				}
			}
			if (len <= 0)
			{
				cout << endl;
				delete buffer;
				return 0;
			}

			I_INDEX addrBlockId = inode.addr[10];	// ����һ������
			unsigned int itemTotal = blockSize / itemSize;
			// �������������ȡ����
			if (addrBlockId > 0)
			{
				for (i = 0; i < itemTotal; ++i)
				{
					blockId = getItem(addrBlockId, i);
					if (blockId > 0)
					{
						if (len > blockSize)
						{
							len -= getBlockData(blockId, buffer, blockSize, 0);
							cout << buffer;
						}
						else
						{
							len -= getBlockData(blockId, buffer, len, 0);
							cout << buffer;
							delete buffer;
							return 0;
						}
					}
					else
					{
						cout << endl;
						delete buffer;
						return 0;
					}
				}
			}
			else
			{// ��ȡ���
				cout << endl;
				delete buffer;
				return 0;
			}
			if (len <= 0)
			{
				cout << endl;
				delete buffer;
				return 0;
			}

			// ����һ����������
			I_INDEX addrBlockId2 = inode.addr[11];
			if (addrBlockId2 > 0)
			{
				for (i = 0; i < itemTotal; ++i)
				{
					blockId = getItem(addrBlockId2, i);
					if (blockId > 0)
					{
						if (len > blockSize)
						{
							len -= getBlockData(blockId, buffer, blockSize, 0);
							cout << buffer;
						}
						else
						{
							len -= getBlockData(blockId, buffer, len, 0);
							cout << endl;
							delete buffer;
							return 0;
						}
					}
					else
					{
						cout << endl;
						delete buffer;
						return 0;
					}
				}
			}
			else
			{
				cout << endl;
				delete buffer;
				return 0;
			}
			return 0;

		}
		else
		{
			cout << filename << " is a directory" << endl;
			return -1;
		}


	}
	else
	{
		cout << "no such file or directory" << endl;
		return -1;
	}


	return 0;
}


unsigned int FileSystem::waitForInput(char *buff, unsigned int limit)
{
	unsigned int len = 0;
	char ch[3];
	ch[0] = 0;
	ch[1] = 0;
	while (len < limit)
	{
		ch[2] = getchar();
		if (ch[0] == '<' && ch[1] == '/' && ch[2] == '>')
		{
			len -= 2;
			buff[len] = '\0';
			return len;
		}
		else
		{
			ch[0] = ch[1];
			ch[1] = ch[2];
			buff[len] = ch[2];
			len++;
		}
	}
	buff[len] = '\0';
	return len;
}

// д���ļ�
int FileSystem::writeFile(string filename, I_INDEX &sid)
{
	unsigned int id = findInodeOfDir(curLink, filename.c_str());
	unsigned long len = 0;
	unsigned int num;
	char ch;
	if (id > 0)
	{
		//��ȡi�ڵ�
		PInode pInode = new Inode();
		getInode(*pInode, id);
		{
			int r = pInode->sig.wait(2);	// ����д����
			if (r == -1)
				return -1;
			cout << "����д�ļ�"<< ",��ǰ��������:" << pInode->sig.curNum << endl;
			setInode(*pInode);	// ����I�ڵ�
			sid = id;
		}
		if (pInode->filetype == FileType::IsFile)
		{
			if (pInode->prioritytype == PriorityType::Read_Only)
			{
				printf("file %s is Read-Only file\n", filename.c_str());
				pInode->sig.signal(2);
				setInode(*pInode);
				return -1;
			}
			printf("write %s: use flag \"</>\" to end\n", filename.c_str());
			//remember to delete it
			char *buff = new char[blockSize + 1];
			//д��10��ֱ��������Ӧ���ݿ�
			int i;
			unsigned int blockId;
			for (i = 0; i < 10; i++)
			{
				blockId = pInode->addr[i];
				if (blockId > 0)
				{
					num = waitForInput(buff, blockSize);
					writeBlockData(blockId, buff, num, 0);
					len += num;
					if (num < blockSize)
					{
						pInode->length = len;
						time(&(pInode->time));
						setInode(*pInode);
						delete buff;
						return 0;
					}
					else
					{
						printf("You have input %ld Byte dat, continue?[Y/N]", len);
						ch = getchar();
						if (ch != 'Y' && ch != 'y')
						{
							pInode->length = len;
							time(&(pInode->time));
							setInode(*pInode);
							delete buff;
							return 0;
						}
					}
				}
				else
				{
					blockId = getAvailableBlockId();
					if (blockId > 0)
					{
						superBlock.blockFree--;
						setSuperBlock(superBlock);
						blockBitmap[blockId] = 1;
						setBlockBitmap(blockId, 1);
						pInode->addr[i] = blockId;
						time(&(pInode->time));
						setInode(*pInode);
						num = waitForInput(buff, blockSize);
						writeBlockData(blockId, buff, num, 0);
						len += num;
						if (num < blockSize)
						{
							pInode->length = len;
							time(&(pInode->time));
							setInode(*pInode);
							delete buff;
							return 0;
						}
						else
						{
							printf("You have input %ld Byte dat, continue?[Y/N]", len);
							ch = getchar();
							if (ch != 'Y' && ch != 'y')
							{
								pInode->length = len;
								time(&(pInode->time));
								setInode(*pInode);
								delete buff;
								return 0;
							}
						}
					}
					else
					{
						pInode->length = len;
						time(&(pInode->time));
						setInode(*pInode);
						delete buff;
						return 0;
					}
				}
			}
			//д��1��һ������
			unsigned addrBlockId = pInode->addr[10];
			int itemTotal = blockSize / itemSize;
			//д��itemTotal��ֱ��������Ӧ�����ݿ�
			if (addrBlockId > 0)
			{
				for (i = 0; i < itemTotal; i++)
				{
					blockId = getItem(addrBlockId, i);
					if (blockId > 0)
					{
						num = waitForInput(buff, blockSize);
						writeBlockData(blockId, buff, num, 0);
						len += num;
						if (num < blockSize)
						{
							pInode->length = len;
							time(&(pInode->time));
							setInode(*pInode);
							delete buff;
							return 0;
						}
						else
						{
							printf("You have input %ld Byte dat, continue?[Y/N]", len);
							ch = getchar();
							if (ch != 'Y' && ch != 'y')
							{
								pInode->length = len;
								time(&(pInode->time));
								setInode(*pInode);
								delete buff;
								return 0;
							}
						}
					}
					else
					{
						blockId = getAvailableBlockId();
						if (blockId > 0)
						{
							superBlock.blockFree--;
							setSuperBlock(superBlock);
							blockBitmap[blockId] = 1;
							setBlockBitmap(blockId, 1);
							setItem(addrBlockId, i, blockId);
							time(&(pInode->time));
							setInode(*pInode);
							num = waitForInput(buff, blockSize);
							writeBlockData(blockId, buff, num, 0);
							len += num;
							if (num < blockSize)
							{
								pInode->length = len;
								time(&(pInode->time));
								setInode(*pInode);
								delete buff;
								return 0;
							}
							else
							{
								printf("You have input %ld Byte dat, continue?[Y/N]", len);
								ch = getchar();
								if (ch != 'Y' && ch != 'y')
								{
									pInode->length = len;
									time(&(pInode->time));
									setInode(*pInode);
									delete buff;
									return 0;
								}
							}
						}
						else
						{
							pInode->length = len;
							time(&(pInode->time));
							setInode(*pInode);
							delete buff;
							return 0;
						}
					}
				}
			}
			else
			{
				addrBlockId = getAvailableBlockId();
				if (addrBlockId > 0)
				{
					superBlock.blockFree--;
					setSuperBlock(superBlock);
					blockBitmap[addrBlockId] = 1;
					setBlockBitmap(addrBlockId, 1);
					pInode->addr[10] = addrBlockId;
					time(&(pInode->time));
					setInode(*pInode);
					//
					for (i = 0; i < itemTotal; i++)
					{
						blockId = getItem(addrBlockId, i);
						if (blockId > 0)
						{
							num = waitForInput(buff, blockSize);
							writeBlockData(blockId, buff, num, 0);
							len += num;
							if (num < blockSize)
							{
								pInode->length = len;
								time(&(pInode->time));
								setInode(*pInode);
								delete buff;
								return 0;
							}
							else
							{
								printf("You have input %ld Byte dat, continue?[Y/N]", len);
								ch = getchar();
								if (ch != 'Y' && ch != 'y')
								{
									pInode->length = len;
									time(&(pInode->time));
									setInode(*pInode);
									delete buff;
									return 0;
								}
							}
						}
						else
						{
							blockId = getAvailableBlockId();
							if (blockId > 0)
							{
								superBlock.blockFree--;
								setSuperBlock(superBlock);
								blockBitmap[blockId] = 1;
								setBlockBitmap(blockId, 1);
								setItem(addrBlockId, i, blockId);
								time(&(pInode->time));
								setInode(*pInode);
								num = waitForInput(buff, blockSize);
								writeBlockData(blockId, buff, num, 0);
								len += num;
								if (num < blockSize)
								{
									pInode->length = len;
									time(&(pInode->time));
									setInode(*pInode);
									delete buff;
									return 0;
								}
								else
								{
									printf("You have input %ld Byte dat, continue?[Y/N]", len);
									ch = getchar();
									if (ch != 'Y' && ch != 'y')
									{
										pInode->length = len;
										time(&(pInode->time));
										setInode(*pInode);
										delete buff;
										return 0;
									}
								}
							}
							else
							{
								pInode->length = len;
								time(&(pInode->time));
								setInode(*pInode);
								delete buff;
								return 0;
							}
						}
					}
				}
				else
				{
					pInode->length = len;
					time(&(pInode->time));
					setInode(*pInode);
					delete buff;
					return 0;
				}
			}
			//д��1����������
			unsigned int addrBlockId2 = pInode->addr[11];
			int j;
			if (addrBlockId2 > 0)
			{
				//����itemTotal��һ������
				for (j = 0; j < itemTotal; j++)
				{
					addrBlockId = getItem(addrBlockId2, j);

					if (addrBlockId > 0)
					{
						//����itemTotal��ֱ������
						for (i = 0; i < itemTotal; i++)
						{
							blockId = getItem(addrBlockId, i);
							if (blockId > 0)
							{
								num = waitForInput(buff, blockSize);
								writeBlockData(blockId, buff, num, 0);
								len += num;
								if (num < blockSize)
								{
									pInode->length = len;
									time(&(pInode->time));
									setInode(*pInode);
									delete buff;
									return 0;
								}
								else
								{
									printf("You have input %ld Byte dat, continue?[Y/N]", len);
									ch = getchar();
									if (ch != 'Y' && ch != 'y')
									{
										pInode->length = len;
										time(&(pInode->time));
										setInode(*pInode);
										delete buff;
										return 0;
									}
								}
							}
							else
							{
								blockId = getAvailableBlockId();
								if (blockId > 0)
								{
									superBlock.blockFree--;
									setSuperBlock(superBlock);
									blockBitmap[blockId] = 1;
									setBlockBitmap(blockId, 1);
									setItem(addrBlockId, i, blockId);
									time(&(pInode->time));
									setInode(*pInode);
									num = waitForInput(buff, blockSize);
									writeBlockData(blockId, buff, num, 0);
									len += num;
									if (num < blockSize)
									{
										pInode->length = len;
										time(&(pInode->time));
										setInode(*pInode);
										delete buff;
										return 0;
									}
									else
									{
										printf("You have input %ld Byte dat, continue?[Y/N]", len);
										ch = getchar();
										if (ch != 'Y' && ch != 'y')
										{
											pInode->length = len;
											time(&(pInode->time));
											setInode(*pInode);
											delete buff;
											return 0;
										}
									}
								}
								else
								{
									pInode->length = len;
									time(&(pInode->time));
									setInode(*pInode);
									delete buff;
									return 0;
								}
							}
						}
					}
					else
					{
						addrBlockId = getAvailableBlockId();
						if (addrBlockId > 0)
						{
							superBlock.blockFree--;
							setSuperBlock(superBlock);
							blockBitmap[addrBlockId] = 1;
							setBlockBitmap(addrBlockId, 1);
							setItem(addrBlockId2, j, addrBlockId);
							time(&(pInode->time));
							setInode(*pInode);
							//
							for (i = 0; i < itemTotal; i++)
							{
								blockId = getItem(addrBlockId, i);
								if (blockId > 0)
								{
									num = waitForInput(buff, blockSize);
									writeBlockData(blockId, buff, num, 0);
									len += num;
									if (num < blockSize)
									{
										pInode->length = len;
										time(&(pInode->time));
										setInode(*pInode);
										delete buff;
										return 0;
									}
									else
									{
										printf("You have input %ld Byte dat, continue?[Y/N]", len);
										ch = getchar();
										if (ch != 'Y' && ch != 'y')
										{
											pInode->length = len;
											time(&(pInode->time));
											setInode(*pInode);
											delete buff;
											return 0;
										}
									}
								}
								else
								{
									blockId = getAvailableBlockId();
									if (blockId > 0)
									{
										superBlock.blockFree--;
										setSuperBlock(superBlock);
										blockBitmap[blockId] = 1;
										setBlockBitmap(blockId, 1);
										setItem(addrBlockId, i, blockId);
										time(&(pInode->time));
										setInode(*pInode);
										num = waitForInput(buff, blockSize);
										writeBlockData(blockId, buff, num, 0);
										len += num;
										if (num < blockSize)
										{
											pInode->length = len;
											time(&(pInode->time));
											setInode(*pInode);
											delete buff;
											return 0;
										}
										else
										{
											printf("You have input %ld Byte dat, continue?[Y/N]", len);
											ch = getchar();
											if (ch != 'Y' && ch != 'y')
											{
												pInode->length = len;
												time(&(pInode->time));
												setInode(*pInode);
												delete buff;
												return 0;
											}
										}
									}
									else
									{
										pInode->length = len;
										time(&(pInode->time));
										setInode(*pInode);
										delete buff;
										return 0;
									}
								}
							}
						}
						else
						{
							pInode->length = len;
							time(&(pInode->time));
							setInode(*pInode);
							delete buff;
							return 0;
						}
					}
				}
			}
			else
			{
				addrBlockId2 = getAvailableBlockId();
				if (addrBlockId2 > 0)
				{
					superBlock.blockFree--;
					setSuperBlock(superBlock);
					blockBitmap[addrBlockId2] = 1;
					setBlockBitmap(addrBlockId2, 1);
					pInode->addr[11] = addrBlockId2;
					time(&(pInode->time));
					setInode(*pInode);
					//����itemTotalһ������
					for (j = 0; j < itemTotal; j++)
					{
						addrBlockId = getItem(addrBlockId2, j);

						if (addrBlockId > 0)
						{
							//����itemTotal��ֱ������
							for (i = 0; i < itemTotal; i++)
							{
								blockId = getItem(addrBlockId, i);
								if (blockId > 0)
								{
									num = waitForInput(buff, blockSize);
									writeBlockData(blockId, buff, num, 0);
									len += num;
									if (num < blockSize)
									{
										pInode->length = len;
										time(&(pInode->time));
										setInode(*pInode);
										delete buff;
										return 0;
									}
									else
									{
										printf("You have input %ld Byte dat, continue?[Y/N]", len);
										ch = getchar();
										if (ch != 'Y' && ch != 'y')
										{
											pInode->length = len;
											time(&(pInode->time));
											setInode(*pInode);
											delete buff;
											return 0;
										}
									}
								}
								else
								{
									blockId = getAvailableBlockId();
									if (blockId > 0)
									{
										superBlock.blockFree--;
										setSuperBlock(superBlock);
										blockBitmap[blockId] = 1;
										setBlockBitmap(blockId, 1);
										setItem(addrBlockId, i, blockId);
										time(&(pInode->time));
										setInode(*pInode);
										num = waitForInput(buff, blockSize);
										writeBlockData(blockId, buff, num, 0);
										len += num;
										if (num < blockSize)
										{
											pInode->length = len;
											time(&(pInode->time));
											setInode(*pInode);
											delete buff;
											return 0;
										}
										else
										{
											printf("You have input %ld Byte dat, continue?[Y/N]", len);
											ch = getchar();
											if (ch != 'Y' && ch != 'y')
											{
												pInode->length = len;
												time(&(pInode->time));
												setInode(*pInode);
												delete buff;
												return 0;
											}
										}
									}
									else
									{
										pInode->length = len;
										time(&(pInode->time));
										setInode(*pInode);
										delete buff;
										return 0;
									}
								}
							}
						}
						else
						{
							addrBlockId = getAvailableBlockId();
							if (addrBlockId > 0)
							{
								superBlock.blockFree--;
								setSuperBlock(superBlock);
								blockBitmap[addrBlockId] = 1;
								setBlockBitmap(addrBlockId, 1);
								setItem(addrBlockId2, j, addrBlockId);
								time(&(pInode->time));
								setInode(*pInode);
								//
								for (i = 0; i < itemTotal; i++)
								{
									blockId = getItem(addrBlockId, i);
									if (blockId > 0)
									{
										num = waitForInput(buff, blockSize);
										writeBlockData(blockId, buff, num, 0);
										len += num;
										if (num < blockSize)
										{
											pInode->length = len;
											time(&(pInode->time));
											setInode(*pInode);
											delete buff;
											return 0;
										}
										else
										{
											printf("You have input %ld Byte dat, continue?[Y/N]", len);
											ch = getchar();
											if (ch != 'Y' && ch != 'y')
											{
												pInode->length = len;
												time(&(pInode->time));
												setInode(*pInode);
												delete buff;
												return 0;
											}
										}
									}
									else
									{
										blockId = getAvailableBlockId();
										if (blockId > 0)
										{
											superBlock.blockFree--;
											setSuperBlock(superBlock);
											blockBitmap[blockId] = 1;
											setBlockBitmap(blockId, 1);
											setItem(addrBlockId, i, blockId);
											time(&(pInode->time));
											setInode(*pInode);
											num = waitForInput(buff, blockSize);
											writeBlockData(blockId, buff, num, 0);
											len += num;
											if (num < blockSize)
											{
												pInode->length = len;
												time(&(pInode->time));
												setInode(*pInode);
												delete buff;
												return 0;
											}
											else
											{
												printf("You have input %ld Byte dat, continue?[Y/N]", len);
												ch = getchar();
												if (ch != 'Y' && ch != 'y')
												{
													pInode->length = len;
													time(&(pInode->time));
													setInode(*pInode);
													delete buff;
													return 0;
												}
											}
										}
										else
										{
											pInode->length = len;
											time(&(pInode->time));
											setInode(*pInode);
											delete buff;
											return 0;
										}
									}
								}
							}
							else
							{
								pInode->length = len;
								time(&(pInode->time));
								setInode(*pInode);
								delete buff;
								return 0;
							}
						}
					}
				}
				else
				{
					pInode->length = len;
					time(&(pInode->time));
					setInode(*pInode);
					delete buff;
					return 0;
				}
			}
			pInode->length = len;
			time(&(pInode->time));
			setInode(*pInode);
			delete buff;
			return 0;
		}
		else
		{
			printf("%s is a directory\n", filename.c_str());
			return -1;
		}
	}
	else
	{
		printf("no such file or directory\n");
		return -1;
	}
}

int FileSystem::waitInput(string &buffer)
{


	return 0;
}



// ɾ���ļ�����Ŀ¼
int FileSystem::deleteFile(string filename) {
	// �ҵ���ӦInode�ڵ�Id
	I_INDEX inodeId = findInodeOfDir(curLink, filename.c_str());
	if (inodeId > 0)
	{//Ŀ¼���ҵ����ļ��ڵ�
		Inode loadInode;	//��ȡ�ýڵ�
		getInode(loadInode, inodeId);
		if (loadInode.filetype == FileType::IsFile) {	//ɾ���ļ�
			if (loadInode.prioritytype == PriorityType::Read_Only) {
				cout << "You have no priority to delete the file" << filename << endl;
				return -1;
			}
			// �ͷ��������ݿ�
			unsigned int i;
			I_INDEX blockId;
			//����10��ֱ���������ͷ����ݿ�
			for (i = 0; i < 10; ++i)
			{
				blockId = loadInode.addr[i];
				if (blockId > 0) {
					superBlock.blockFree++;
					setSuperBlock(superBlock);
					releaseBlock(blockId);
					blockBitmap[blockId] = 0;
					setBlockBitmap(blockId, 1);
				}
			}
			// �����������
			I_INDEX addrBlockId = loadInode.addr[10];
			const unsigned int totalItem = blockSize / itemSize;
			if (addrBlockId > 0)
			{
				for (i = 0; i < totalItem; ++i)
				{
					blockId = getItem(addrBlockId, i);
					if (blockId > 0) {
						superBlock.blockFree++;
						setSuperBlock(superBlock);
						blockBitmap[blockId] = 0;
						releaseBlock(blockId);
						setBlockBitmap(blockId, 1);

					}
				}
				//�ͷ�һ�����ݿ�
				superBlock.blockFree++;
				setSuperBlock(superBlock);
				releaseBlock(addrBlockId);
				blockBitmap[addrBlockId] = 0;
				setBlockBitmap(addrBlockId, 1);

			}

			I_INDEX addrBlockId2 = loadInode.addr[11];
			if (addrBlockId2 > 0) {
				for (i = 0; i < totalItem; ++i)
				{
					blockId = getItem(addrBlockId2, i);
					if (blockId > 0) {
						superBlock.blockFree++;
						setSuperBlock(superBlock);
						releaseBlock(blockId);
						blockBitmap[blockId] = 0;
						setBlockBitmap(blockId, 1);
					}
				}
				// �ͷŶ������ݿ�
				superBlock.blockFree++;
				setSuperBlock(superBlock);
				releaseBlock(addrBlockId2);
				blockBitmap[addrBlockId2] = 0;
				setBlockBitmap(addrBlockId2, 1);
			}

			// �ͷ�i�ڵ�
			releaseInode(loadInode.id);
			superBlock.inodeNum--;
			setSuperBlock(superBlock);
			inodeBitmap[loadInode.id] = 0;
			setInodeBitmap(loadInode.id, 1);
			releaseItem(loadInode.blockId, loadInode.id);

		}
		else
		{ // ɾ��Ŀ¼
			if (loadInode.prioritytype == PriorityType::Read_Only) {
				cout << "dir" << filename << " is Read-Only" << endl;
				return -1;
			}

			// �Ƚ���Ŀ¼
			cd(filename);
			FcbLink link = curLink->next;
			while (link != NULL)
			{
				deleteFile(link->fcb.filename);
				link = link->next;
			}
			cd("..");	//	�ص�ԭ����Ŀ¼
			// ɾ��Ŀ¼����
			unsigned int i;
			I_INDEX blockId;

			for (i = 0; i < 11; ++i)
			{
				blockId = loadInode.addr[i];
				if (blockId > 0) {
					superBlock.blockFree++;
					setSuperBlock(superBlock);
					releaseBlock(blockId);
					blockBitmap[blockId] = 0;
					setBlockBitmap(blockId, 1);

				}
			}

			// ����һ��һ������
			I_INDEX addrBlockId = loadInode.addr[11];
			unsigned int itemTotal = blockSize / itemSize;
			if (addrBlockId > 0) {
				for (i = 0; i < itemTotal; ++i)
				{
					blockId = getItem(addrBlockId, i);
					if (blockId > 0) {
						superBlock.blockFree++;
						setSuperBlock(superBlock);
						releaseBlock(blockId);
						blockBitmap[blockId] = 0;
						setBlockBitmap(blockId, 1);
					}
				}

				// �ͷ�һ������
				superBlock.blockFree++;
				setSuperBlock(superBlock);
				releaseBlock(addrBlockId);
				blockBitmap[addrBlockId] = 0;
				setBlockBitmap(addrBlockId, 1);

			}
		}

		// ����Ŀ¼i�ڵ�
		curInode.length--;
		time(&(curInode.time));
		setInode(curInode);
		// ����FcbLink
		removeFcbLinkInode(curLink, loadInode);
		return 0;
	}
	else
	{
		cout << "no such file or directory" << endl;
		return -1;
	}
}

