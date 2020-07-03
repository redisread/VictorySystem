#include "FileSystem.h"

///////////////////////////////////系统内核模块

// 构造函数，初始化文件系统，设置相关数据的大小和偏移值
FileSystem::FileSystem() : systemSize(SystemSize), 
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


// 系统初始化函数，初始化后直接接受命令
void FileSystem::init(int result) {
	if (result == 1) {	// 已存在系统
		if ((fp = fopen(this->systemName, "rb+")) == NULL)
		{
			cout << "open file " << systemName << " error..." << endl;
			::exit(1);
		}
		cout << "Load the " << systemName << " Success!" << endl;
		rewind(fp);
		login();           // 登录判断
		load_afterLogin(); // 登录之后加载
		cout << "Load ok!" << endl;
	}
	else if(result == 0){  //不存在该系统，创建系统
		fp = fopen(this->systemName, "wb+");
		assert(fp);
		rewind(fp);
		cout << "Create File System Ok!" << endl;
		cout << "Enter the VictorSystem File System..." << endl;
		cout << "First enter,Please create your account" << endl;
		createUser();      // 创建用户
		login();           // 登录判断
		init_afterLogin(); // 登录之后的初始化
	}
	else {
		cout << "error!" << endl;
		return;
	}
	cout << "Enter the " << systemName << " FileSystem bash" << endl;
	// 执行命令
	this->doCommand();
}

void FileSystem::init_afterLogin() {

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
	fseek(fp, offset.inode, SEEK_SET);
	for (i = 0; i < len; ++i)
		putc(0, fp);
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
	setInode(this->curInode);
	// 刷新缓冲区
	fflush(fp);
	// 构建目录的信息链
	buildFcbLink(curLink, curInode);
	curPath = "/";	// 设置当前路径
}

// 登录之后加载文件系统
void FileSystem::load_afterLogin() {
	rewind(fp);
	getSuperBlock(this->superBlock);
	getBlockBitmap(this->blockBitmap);
	getInodeBitmap(this->inodeBitmap);
	// 获取当前的Inode节点
	getInode(this->curInode, 0);
	// 构建目录的信息链
	buildFcbLink(curLink, curInode);
	// 设置当前目录
	curPath = "/";
}



// 建立目录的信息链
void FileSystem::buildFcbLink(FcbLink &fcbLink, Inode inode) {
	if (fcbLink != NULL)
		releaseFcbLink(fcbLink);
	//printf("start read dir self inode\n");
	fcbLink = new FcbLinkNode();
	getFcbLink_ByInode(fcbLink, inode);
	//printf("end read dir self inode\n");
	if (inode.length <= 0)
		return;

	// 遍历建立子文件或者子目录节点
	FcbLink link = curLink;
	unsigned int len = inode.length; //子文件或子目录数
	const int itemTotal = blockSize / itemSize;

	Inode fileInode;
	FcbLink pNode;
	I_INDEX blockId;
	unsigned int fileItem;

	//遍历11个直接索引
	for (unsigned int i = 0; i < 11; i++)
	{
		blockId = inode.addr[i];
		if (blockId > 0 || curInode.id == 0)
		{
			//遍历直接索引下itemTotal个fileItem项(文件i节点索引)
			for (unsigned int index = 0; index < itemTotal; index++)
			{
				fileItem = getItem(blockId, index);
				//读取i子文件或子目录i节点
				if (fileItem > 0)
				{
					getInode(fileInode, fileItem);
					pNode = new FcbLinkNode();
					getFcbLink_ByInode(pNode, fileInode);
					link->next = pNode;
					link = pNode;
					len--;
					//printf("read dir item inode: id=%d, name=%s, index=%d\n", fileItem, fileInode.filename, index);
					if (len <= 0)
					{
						return;
					}
				}
			}
		}
	}

	//遍历一级索引1个
	unsigned int addrBlockId = inode.addr[11];
	if (addrBlockId > 0)
	{
		//遍历一级索引下itemTotal个直接索引
		for (int i = 0; i < itemTotal; ++i)
		{
			{
				blockId = getItem(addrBlockId, itemTotal);
				if (blockId > 0)
				{
					for (int index = 0; index < itemTotal; ++index)
					{
						fileItem = getItem(blockId, index);
						if (fileItem > 0)
						{
							getInode(fileInode, fileItem);
							pNode = new FcbLinkNode();
							getFcbLink_ByInode(pNode, fileInode);
							link->next = pNode;
							link = pNode;
							len--;
							if (len <= 0)	return;

						}
					}
				}
			}
		}

	}
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

	if (curLink != NULL)
		releaseFcbLink(curLink);
	cout << "Exit File System" << endl;
}



// 查找释放相关函数


// 查找目录下是否有名字为name的文件或者目录，假如有返回fcb的id，否则返回0
unsigned int FileSystem::findInodeOfDir(FcbLink curLink, const char *name) {
	if (!curLink || !name)
		return 0;
	FcbLink link = curLink->next;
	while (link != NULL)
	{
		if (strcmp(link->fcb.filename, name) == 0)
			return link->fcb.id;
		link = link->next;
	}
	return 0;
}


// 寻找一个空闲的i节点块,０号已被分配给根目录，返回值大于0表示找到
I_INDEX FileSystem::getAvailableInodeId() {
	I_INDEX i;
	for (i = 0; i < blockNum; ++i)
	{
		if (inodeBitmap[i] == 0)
		{
			return i;
		}
	}
	return 0;
}



// 寻找一个当前目录文件中可用的文件项，返回数据块地址和文件项索引,大于0表示找到
unsigned int FileSystem::getAvailableFileItem(Inode &inode, unsigned int &availableIndex) {
	//遍历建立子文件或子目录节点
	unsigned int i;
	unsigned int index;
	I_INDEX blockId;
	unsigned int fileItem;

	Inode fileInode;
	const unsigned int itemTotal = blockSize / itemSize;

	// 遍历11个直接索引
	for (i = 0; i < 11; ++i)
	{
		blockId = inode.addr[i];
		if (blockId > 0 || inode.id == 0)
		{//数据块已经分配，在分配的数据块中查找；或者现在是根节点
			for (index = 0; index < itemTotal; ++index)
			{
				fileItem = getItem(blockId, index);
				if (fileItem == 0)
				{
					availableIndex = index;
					return blockId;
				}
			}
		}
		else {//数据块未分配，分配之后返回
			blockId = getAvailableBlockId();
			if (blockId > 0) {//分配成功
				//更新
				superBlock.blockFree--;
				setSuperBlock(superBlock);
				blockBitmap[blockId] = 1;
				setBlockBitmap(blockId, 1);
				inode.addr[i] = blockId;
				setInode(inode);
				availableIndex = 0;
				return blockId;
			}
			else
			{// 没有空间了
				return 0;
			}
		}
	}

	// 一级索引，该block存储一级索引地址
	I_INDEX addrBlockId = inode.addr[11];

	if (addrBlockId > 0)
	{ // 一级索引已经分配
		//遍历一级索引下itemTotal个直接索引
		for (i = 0; i < itemTotal; ++i)
		{
			blockId = getItem(addrBlockId, i);
			if (blockId > 0)
			{
				for (index = 0; index < itemTotal; ++index)
				{
					fileItem = getItem(blockId, index);
					if (fileItem == 0) {
						availableIndex = index;
						return blockId;
					}
				}
			}
			else
			{
				blockId = getAvailableBlockId();
				if (blockId > 0) {
					superBlock.blockFree--;
					setSuperBlock(superBlock);
					blockBitmap[blockId] = 1;
					setBlockBitmap(blockId, 1);
					inode.addr[i] = blockId;	//更新i节点
					setInode(inode);
					availableIndex = 0;
					return blockId;
				}
				else
				{
					return 0;
				}
			}
		}

	}
	else
	{//一级索引未分配
		addrBlockId = getAvailableBlockId();
		if (addrBlockId > 0) {//分配一级索引数据块
			superBlock.blockFree--;
			setSuperBlock(superBlock);
			blockBitmap[addrBlockId] = 1;
			setBlockBitmap(addrBlockId, 1);
			inode.addr[11] = addrBlockId;
			setInode(inode);

			// 分配
			blockId = getAvailableBlockId();
			if (blockId > 0) {	//分配成功
				superBlock.blockFree--;
				setSuperBlock(superBlock);
				blockBitmap[blockId] = 1;
				setBlockBitmap(blockId, 1);
				setItem(addrBlockId, 0, blockId);
				availableIndex = 0;
				return blockId;
			}

		}
		else
		{
			return 0;
		}
	}
}

I_INDEX FileSystem::getAvailableBlockId() {
	if (superBlock.blockFree == 0)
	{
		cout << "No free block" << endl;
		return 0;
	}
	unsigned int i;
	for (i = 0; i < blockNum; ++i)
	{
		if (blockBitmap[i] == 0) {
			releaseBlock(i);
			return i;
		}
	}
	cout << "No free block" << endl;
	return 0;
}



// 目录信息链增加一个节点,不能增加根节点
void FileSystem::appendFcbLinkNode(FcbLink curLink, Inode inode) {
	if (curLink == NULL || inode.id < 0)
		return;
	FcbLink link = curLink;

	while (link->next != NULL)
	{
		link = link->next;
	}
	FcbLink pNode = new FcbLinkNode();
	getFcbLink_ByInode(pNode, inode);
	link->next = pNode;
}


void FileSystem::removeFcbLinkInode(FcbLink fcbLink, Inode inode) {
	if (fcbLink == NULL || inode.id <= 0)
		return;
	FcbLink link = fcbLink->next;
	FcbLink last = fcbLink;
	while (link != NULL)
	{
		if (link->fcb.id == inode.id) {
			last->next = link->next;
			delete link;
			return;
		}
		last = link;
		link = link->next;

	}
}


// 创建用户
void FileSystem::createUser()
{
	cout << "Create a new Account....." << endl;
	cout << "UserName:";
	cin >> user.username;
	cout << "PassWord:";
	cin >> user.password;
	cout << "Create ok! UserName：" << user.username << "  PassWord: " << user.password << endl;

	// 写入文件
	setUser(this->user);
}


// 登录操作
void FileSystem::login()
{
	getUser(this->user);

	char username[10];
	char password[10];

	cout << "login......" << endl;
	cout << "username: ";
	cin >> username;
	cout << "password: ";
	cin >> password;

	if (strcmp(username, this->user.username) == 0 && strcmp(password, this->user.password) == 0)
	{
		cout << "login successful!" << endl;
		return;
	}
	// 不正确的话进入递归
	cout << "The Username or Password is Incorrect,try again!" << endl;
	this->login();
}

void FileSystem::logout() {
	// 保存数据
	fseek(fp, offset.user, SEEK_SET);
	fwrite(&this->user, userSize, 1, fp);
	fseek(fp, offset.superBlock, SEEK_SET);
	fwrite(&this->superBlock, superBlockSize, 1, fp);
	fseek(fp, offset.blockBitmap, SEEK_SET);
	fwrite(&this->blockBitmap, blockBitmapSize, 1, fp);
	fseek(fp, offset.inodeBitmap, SEEK_SET);
	fwrite(&this->inodeBitmap, inodeBitmapSize, 1, fp);
	//fflush(fp);
	// 重新进入文件系统
	login();
	load_afterLogin();
	doCommand();
}

int FileSystem::updateUser() {

	char username[10];
	char password[10];

	cout << "please input the old password: ";
	cin >> password;

	if (strcmp(password, user.password) == 0) {
		string newUsername, newPassword;

		while (1)
		{
			cout << "new username: ";
			cin >> newUsername;
			cout << "new password: ";
			cin >> newPassword;
			if (newUsername.length() > 0 && newUsername.length() < 10\
				&& newPassword.length() > 0 && newPassword.length() < 10)
			{
				strcpy(user.username, newUsername.c_str());
				strcpy(user.password, newPassword.c_str());
				setUser(user);
				cout << "modify account info success." << endl;
				return 0;
			}
			cout << "your username or password is too long or too short,please try again" << endl;
		}
	}
	else
	{
		cout << "password is not correct, you are not permitted to modify account info." << endl;
		return -1;
	}

	return -1;
}

int FileSystem::createMemory() {
	FILE *fp;
	fp = fopen(systemName, "rb");
	if (fp)// 文件已存在
		return 1;
	fp = fopen(systemName, "wb+");
	if (fp != NULL)
	{
		// 指针移动到100MB的指针处，即创建100M的大文件
		fseek(fp, 100 * MB, SEEK_SET);
		char end = 'F';
		fwrite(&end, sizeof(char), 1, fp);	// 写入结束标志
		fclose(fp);
		return 0;
	}
	return -1;
}


void FileSystem::computeData() {




}