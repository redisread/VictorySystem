#include "FileSystem.h"

///////////////////////////////////ϵͳ�ں�ģ��

// ���캯������ʼ���ļ�ϵͳ������������ݵĴ�С��ƫ��ֵ
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
	strcpy(systemName, SystemName); // ��ʼ���ļ���
	// �������ƫ��ֵ��User��ƫ��ֵΪ0
	offset.user = 0;
	offset.superBlock = userSize;
	offset.blockBitmap = offset.superBlock + sizeof(SuperBlock);
	offset.inodeBitmap = offset.blockBitmap + blockBitmapSize;
	offset.inode = offset.inodeBitmap + inodeBitmapSize; // i�ڵ�ƫ�ƣ�ռ���ڴ�block * sizeof(Inode)
	offset.block = offset.inode + sizeof(Inode) * blockNum;
}


// ϵͳ��ʼ����������ʼ����ֱ�ӽ�������
void FileSystem::init(int result) {
	if (result == 1) {	// �Ѵ���ϵͳ
		if ((fp = fopen(this->systemName, "rb+")) == NULL)
		{
			cout << "open file " << systemName << " error..." << endl;
			::exit(1);
		}
		cout << "Load the " << systemName << " Success!" << endl;
		rewind(fp);
		login();           // ��¼�ж�
		load_afterLogin(); // ��¼֮�����
		cout << "Load ok!" << endl;
	}
	else if(result == 0){  //�����ڸ�ϵͳ������ϵͳ
		fp = fopen(this->systemName, "wb+");
		assert(fp);
		rewind(fp);
		cout << "Create File System Ok!" << endl;
		cout << "Enter the VictorSystem File System..." << endl;
		cout << "First enter,Please create your account" << endl;
		createUser();      // �����û�
		login();           // ��¼�ж�
		init_afterLogin(); // ��¼֮��ĳ�ʼ��
	}
	else {
		cout << "error!" << endl;
		return;
	}
	cout << "Enter the " << systemName << " FileSystem bash" << endl;
	// ִ������
	this->doCommand();
}

void FileSystem::init_afterLogin() {

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
	fseek(fp, offset.inode, SEEK_SET);
	for (i = 0; i < len; ++i)
		putc(0, fp);
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
	setInode(this->curInode);
	// ˢ�»�����
	fflush(fp);
	// ����Ŀ¼����Ϣ��
	buildFcbLink(curLink, curInode);
	curPath = "/";	// ���õ�ǰ·��
}

// ��¼֮������ļ�ϵͳ
void FileSystem::load_afterLogin() {
	rewind(fp);
	getSuperBlock(this->superBlock);
	getBlockBitmap(this->blockBitmap);
	getInodeBitmap(this->inodeBitmap);
	// ��ȡ��ǰ��Inode�ڵ�
	getInode(this->curInode, 0);
	// ����Ŀ¼����Ϣ��
	buildFcbLink(curLink, curInode);
	// ���õ�ǰĿ¼
	curPath = "/";
}



// ����Ŀ¼����Ϣ��
void FileSystem::buildFcbLink(FcbLink &fcbLink, Inode inode) {
	if (fcbLink != NULL)
		releaseFcbLink(fcbLink);
	//printf("start read dir self inode\n");
	fcbLink = new FcbLinkNode();
	getFcbLink_ByInode(fcbLink, inode);
	//printf("end read dir self inode\n");
	if (inode.length <= 0)
		return;

	// �����������ļ�������Ŀ¼�ڵ�
	FcbLink link = curLink;
	unsigned int len = inode.length; //���ļ�����Ŀ¼��
	const int itemTotal = blockSize / itemSize;

	Inode fileInode;
	FcbLink pNode;
	I_INDEX blockId;
	unsigned int fileItem;

	//����11��ֱ������
	for (unsigned int i = 0; i < 11; i++)
	{
		blockId = inode.addr[i];
		if (blockId > 0 || curInode.id == 0)
		{
			//����ֱ��������itemTotal��fileItem��(�ļ�i�ڵ�����)
			for (unsigned int index = 0; index < itemTotal; index++)
			{
				fileItem = getItem(blockId, index);
				//��ȡi���ļ�����Ŀ¼i�ڵ�
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

	//����һ������1��
	unsigned int addrBlockId = inode.addr[11];
	if (addrBlockId > 0)
	{
		//����һ��������itemTotal��ֱ������
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

	if (curLink != NULL)
		releaseFcbLink(curLink);
	cout << "Exit File System" << endl;
}



// �����ͷ���غ���


// ����Ŀ¼���Ƿ�������Ϊname���ļ�����Ŀ¼�������з���fcb��id�����򷵻�0
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


// Ѱ��һ�����е�i�ڵ��,�����ѱ��������Ŀ¼������ֵ����0��ʾ�ҵ�
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



// Ѱ��һ����ǰĿ¼�ļ��п��õ��ļ���������ݿ��ַ���ļ�������,����0��ʾ�ҵ�
unsigned int FileSystem::getAvailableFileItem(Inode &inode, unsigned int &availableIndex) {
	//�����������ļ�����Ŀ¼�ڵ�
	unsigned int i;
	unsigned int index;
	I_INDEX blockId;
	unsigned int fileItem;

	Inode fileInode;
	const unsigned int itemTotal = blockSize / itemSize;

	// ����11��ֱ������
	for (i = 0; i < 11; ++i)
	{
		blockId = inode.addr[i];
		if (blockId > 0 || inode.id == 0)
		{//���ݿ��Ѿ����䣬�ڷ�������ݿ��в��ң����������Ǹ��ڵ�
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
		else {//���ݿ�δ���䣬����֮�󷵻�
			blockId = getAvailableBlockId();
			if (blockId > 0) {//����ɹ�
				//����
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
			{// û�пռ���
				return 0;
			}
		}
	}

	// һ����������block�洢һ��������ַ
	I_INDEX addrBlockId = inode.addr[11];

	if (addrBlockId > 0)
	{ // һ�������Ѿ�����
		//����һ��������itemTotal��ֱ������
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
					inode.addr[i] = blockId;	//����i�ڵ�
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
	{//һ������δ����
		addrBlockId = getAvailableBlockId();
		if (addrBlockId > 0) {//����һ���������ݿ�
			superBlock.blockFree--;
			setSuperBlock(superBlock);
			blockBitmap[addrBlockId] = 1;
			setBlockBitmap(addrBlockId, 1);
			inode.addr[11] = addrBlockId;
			setInode(inode);

			// ����
			blockId = getAvailableBlockId();
			if (blockId > 0) {	//����ɹ�
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



// Ŀ¼��Ϣ������һ���ڵ�,�������Ӹ��ڵ�
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


// �����û�
void FileSystem::createUser()
{
	cout << "Create a new Account....." << endl;
	cout << "UserName:";
	cin >> user.username;
	cout << "PassWord:";
	cin >> user.password;
	cout << "Create ok! UserName��" << user.username << "  PassWord: " << user.password << endl;

	// д���ļ�
	setUser(this->user);
}


// ��¼����
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
	// ����ȷ�Ļ�����ݹ�
	cout << "The Username or Password is Incorrect,try again!" << endl;
	this->login();
}

void FileSystem::logout() {
	// ��������
	fseek(fp, offset.user, SEEK_SET);
	fwrite(&this->user, userSize, 1, fp);
	fseek(fp, offset.superBlock, SEEK_SET);
	fwrite(&this->superBlock, superBlockSize, 1, fp);
	fseek(fp, offset.blockBitmap, SEEK_SET);
	fwrite(&this->blockBitmap, blockBitmapSize, 1, fp);
	fseek(fp, offset.inodeBitmap, SEEK_SET);
	fwrite(&this->inodeBitmap, inodeBitmapSize, 1, fp);
	//fflush(fp);
	// ���½����ļ�ϵͳ
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
	if (fp)// �ļ��Ѵ���
		return 1;
	fp = fopen(systemName, "wb+");
	if (fp != NULL)
	{
		// ָ���ƶ���100MB��ָ�봦��������100M�Ĵ��ļ�
		fseek(fp, 100 * MB, SEEK_SET);
		char end = 'F';
		fwrite(&end, sizeof(char), 1, fp);	// д�������־
		fclose(fp);
		return 0;
	}
	return -1;
}


void FileSystem::computeData() {




}