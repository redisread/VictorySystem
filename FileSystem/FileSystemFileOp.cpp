#include "FileSystem.h"

///////////////////////////////////////文件按操作模块
// 创建文件
int FileSystem::createFile(string filename, FileType ftype) {

	if (filename.empty() || filename.length() == 0 || findInodeOfDir(curLink, filename.c_str()))
	{	// 假如已存在文件或者文件名不合法
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
			//创建i节点
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

			setItem(dirBlockId, index, inodeId);	// 存储Inode节点Id

			// 更新i节点
			curInode.length++;
			time(&(curInode.time));
			setInode(curInode);

			// 更新curLink
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


// 读取文件内容
int FileSystem::readFile(string filename, I_INDEX &sid)
{

	I_INDEX inodeId = findInodeOfDir(curLink, filename.c_str());
	if (inodeId > 0)
	{
		Inode inode;
		getInode(inode, inodeId);
		{
			int r = inode.sig.wait(1);	// 申请资源
			if (r == -1)
				return -1;
			sid = inodeId;
			setInode(inode);	//更新i节点信号量
			Inode ainode;
			getInode(ainode, inode.id);
			cout << "访问文件" << " 当前人数：" << ainode.sig.curNum << endl;
			
		}
		time(&(inode.time));
		if (inode.filetype == FileType::IsFile)
		{// 文件
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
			// 遍历直接索引读取数据
			for (i = 0; i < 11; ++i)
			{
				blockId = inode.addr[i];
				if (blockId > 0)
				{
					if (len > blockSize)
					{
						len -= getBlockData(blockId, buffer, blockSize, 0);
						cout << buffer;	// 输出
					}
					else
					{ // 完成
						len -= getBlockData(blockId, buffer, len, 0);
						cout << buffer;
						delete buffer;
						return 0;
					}
				}
				else
				{// 读取完成
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

			I_INDEX addrBlockId = inode.addr[10];	// 遍历一级索引
			unsigned int itemTotal = blockSize / itemSize;
			// 遍历间接索引读取数据
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
			{// 读取完成
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

			// 遍历一个二级索引
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

// 写入文件
int FileSystem::writeFile(string filename, I_INDEX &sid)
{
	unsigned int id = findInodeOfDir(curLink, filename.c_str());
	unsigned long len = 0;
	unsigned int num;
	char ch;
	if (id > 0)
	{
		//读取i节点
		PInode pInode = new Inode();
		getInode(*pInode, id);
		{
			int r = pInode->sig.wait(2);	// 申请写操作
			if (r == -1)
				return -1;
			cout << "申请写文件"<< ",当前访问人数:" << pInode->sig.curNum << endl;
			setInode(*pInode);	// 更新I节点
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
			//写入10个直接索引对应数据块
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
			//写入1个一级索引
			unsigned addrBlockId = pInode->addr[10];
			int itemTotal = blockSize / itemSize;
			//写入itemTotal个直接索引对应的数据块
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
			//写入1个二级索引
			unsigned int addrBlockId2 = pInode->addr[11];
			int j;
			if (addrBlockId2 > 0)
			{
				//遍历itemTotal个一级索引
				for (j = 0; j < itemTotal; j++)
				{
					addrBlockId = getItem(addrBlockId2, j);

					if (addrBlockId > 0)
					{
						//遍历itemTotal个直接索引
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
					//遍历itemTotal一级索引
					for (j = 0; j < itemTotal; j++)
					{
						addrBlockId = getItem(addrBlockId2, j);

						if (addrBlockId > 0)
						{
							//遍历itemTotal个直接索引
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



// 删除文件或者目录
int FileSystem::deleteFile(string filename) {
	// 找到对应Inode节点Id
	I_INDEX inodeId = findInodeOfDir(curLink, filename.c_str());
	if (inodeId > 0)
	{//目录下找到该文件节点
		Inode loadInode;	//读取该节点
		getInode(loadInode, inodeId);
		if (loadInode.filetype == FileType::IsFile) {	//删除文件
			if (loadInode.prioritytype == PriorityType::Read_Only) {
				cout << "You have no priority to delete the file" << filename << endl;
				return -1;
			}
			// 释放内容数据块
			unsigned int i;
			I_INDEX blockId;
			//遍历10个直接索引，释放数据块
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
			// 遍历间接索引
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
				//释放一级数据块
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
				// 释放二级数据块
				superBlock.blockFree++;
				setSuperBlock(superBlock);
				releaseBlock(addrBlockId2);
				blockBitmap[addrBlockId2] = 0;
				setBlockBitmap(addrBlockId2, 1);
			}

			// 释放i节点
			releaseInode(loadInode.id);
			superBlock.inodeNum--;
			setSuperBlock(superBlock);
			inodeBitmap[loadInode.id] = 0;
			setInodeBitmap(loadInode.id, 1);
			releaseItem(loadInode.blockId, loadInode.id);

		}
		else
		{ // 删除目录
			if (loadInode.prioritytype == PriorityType::Read_Only) {
				cout << "dir" << filename << " is Read-Only" << endl;
				return -1;
			}

			// 先进入目录
			cd(filename);
			FcbLink link = curLink->next;
			while (link != NULL)
			{
				deleteFile(link->fcb.filename);
				link = link->next;
			}
			cd("..");	//	回到原来的目录
			// 删除目录本身
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

			// 遍历一个一级索引
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

				// 释放一级索引
				superBlock.blockFree++;
				setSuperBlock(superBlock);
				releaseBlock(addrBlockId);
				blockBitmap[addrBlockId] = 0;
				setBlockBitmap(addrBlockId, 1);

			}
		}

		// 更新目录i节点
		curInode.length--;
		time(&(curInode.time));
		setInode(curInode);
		// 更新FcbLink
		removeFcbLinkInode(curLink, loadInode);
		return 0;
	}
	else
	{
		cout << "no such file or directory" << endl;
		return -1;
	}
}

