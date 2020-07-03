#include "FileSystem.h"

/*******************ʵ�ֳ��õ��ļ���������ģ��*********************/

// ��ʾ��ǰ·��
void FileSystem::showCurPath() {

	cout << user.username << "@localhost " << this->curPath.data() << "> ";
}

// ִ�н�������
void FileSystem::doCommand() {

	string cmd_line,cmdName,param1, param2, param3;
	CMD type;
	int r, i = 0;;
	I_INDEX sid;
	Inode sInode;
	while (1)
	{
		cmd_line.clear();
		param1.clear();
		param2.clear();
		param3.clear();
		showCurPath();	// ��ʾ��ǰ·��
		getline(cin, cmd_line); // ��������
		// ��������
		type = parseCommand(cmd_line, cmdName, param1, param2, param3);
		switch (type)
		{
		case CMD::ACCOUNT:
			this->updateUser();
			break;
		case CMD::HELP:
			help();
			break;
		case CMD::LS:
			if (param1.length() > 0)
			{
				if (param1 == "-l" || param1 == "l") ls_l();
				else cout << "please use -l or l" << endl;
			}
			else ls();
			break;
		case CMD::CD:
			cd(param1);
			break;
		case CMD::WRITE:
			r = writeFile(param1, sid);
			if (r != -1)
			{
				getInode(sInode, sid);
				r = sInode.sig.signal(2);
				setInode(sInode);	// ����i�ڵ�
				if (r == 0)   cout << "�ͷųɹ�" << " ��ǰ����������" << sInode.sig.curNum << endl;
				else cout << "�ͷ�ʧ��" << endl;
			}
			else
				cout << "read file failure,someone write the file or exceed the max read num" << endl;
			break;
		case CMD::MKDIR:
			createFile(param1, FileType::IsDir);
			fflush(fp);
			break;
		case CMD::TOUCH:
			createFile(param1, FileType::IsFile);
			fflush(fp);
			break;
		case CMD::CAT:	// ��ȡ�ļ�
			r = readFile(param1, sid);
			if (r != -1)
			{
				getchar();
				getInode(sInode, sid);
				 r = sInode.sig.signal(1);
				setInode(sInode);	// ����i�ڵ�
				if (r == 0)  cout << "�ͷųɹ�" <<" ��ǰ����������" <<sInode.sig.curNum << endl;
				else cout << "�ͷ�ʧ��" << endl;
			}
			else
				cout << "read file failure,someone write the file or exceed the max read num" << endl;
			break;
		case CMD::WHITCH:
			showCurPath();
			cout << curPath << endl;
			break;
		case CMD::RM:
			deleteFile(param1);
			break;
		case CMD::MV:
			this->mv(param1, param2);
			break;
		case CMD::COPY:
			this->cp(param1, param2);
			break;
		case CMD::CHMOD:
			if (param2 == "r" || param2 == "-r")
				chmod(param1, PriorityType::Read_Only);
			else if (param2 == "rw" || param2 == "wr" || param2 == "-rw" || param2 == "-wr")
				chmod(param1, PriorityType::Read_Write);
			else
				cout << "please use -r or -rw" << endl;
			break;
		case CMD::LOGOUT:
			logout();
			break;
		case CMD::EXIT:
			exit();
			break;
		case CMD::CLEAR:
			clear();
			break;
		case CMD::SYSINFO:
			systemInfo();
			break;
		case CMD::SUDO:
			cout << "No support command" << endl;
			break;
		case CMD::EMPTY:
			cout << endl;
			break;
		case CMD::NONE:
			cout << "No this Command!" << endl;
			break;
		default:
			cout << "No this Command!" << endl;
			break;
			}
		};
	}

	// �������������
	CMD FileSystem::parseCommand(string cmd_line, string &cmdName, string &param1, string &param2, string &param3) {
		CMD type;
		unsigned int poffset[3];
		for (unsigned int i = 0; i < 3; ++i)
			poffset[i] = 0;
		unsigned int pi = 0;
		for (unsigned int i = 0; i < cmd_line.length(); ++i){
			if (cmd_line[i] == ' ')
				poffset[pi++] = i;
		}
		if (poffset[0]){
			cmdName = cmd_line.substr(0, poffset[0]);
			if (poffset[1]){
				param1 = cmd_line.substr(poffset[0] + 1, poffset[1] - (poffset[0] + 1));
				if (poffset[2]){
					param2 = cmd_line.substr(poffset[1] + 1, poffset[2] - (poffset[1] + 1));
					param3 = cmd_line.substr(poffset[2] + 1, cmd_line.length() - (poffset[2] + 1));
				}
				else{
					param2 = cmd_line.substr(poffset[1] + 1, cmd_line.length() - (poffset[1] + 1));
				}
			}
			else{
				param1 = cmd_line.substr(poffset[0] + 1, cmd_line.length() - (poffset[0] + 1));
			}
		}
		else cmdName = cmd_line;
		type = getCmdType(cmdName);
		return type;
	}


	// Helpָ��
	void FileSystem::help()
	{
		printf("command: \n\
    help    ---  show help menu \n\
    sysinfo ---  show system base information \n\
    logout  ---  exit user \n\
    account ---  modify username and password \n\
    clear   ---  clear the screen \n\
    ls      ---  list the digest of the directory's children \n\
    ls -l   ---  list the detail of the directory's children \n\
    cd      ---  change directory \n\
    mkdir   ---  make directory   \n\
    rmdir   ---  delete a directory   \n\
    open   ---  create a new file \n\
    cat     ---  read a file \n\
    write   ---  write something to a file \n\
    rm      ---  delete a file \n\
    cp      ---  cp a directory file to another directory or file (not finish)\n\
    mv      ---  rename a file or directory \n\
    chmod   ---  change the authorizatino of a directory or a file \n\
    exit    ---  exit this system\n");
	}

	void FileSystem::clear() {
		system("cls");
	}

	void FileSystem::systemInfo() {
		printf("Sum of block number:%d\n", superBlock.blockNum);
		printf("Each block size:%d\n", superBlock.blockSize);
		printf("Free of block number:%d\n", superBlock.blockFree);
		printf("Sum of inode number:%d\n", superBlock.inodeNum);
	}

	int FileSystem::cd(string dirName) {
		I_INDEX id;
		if (strcmp(dirName.c_str(), "..") == 0) {	// �ص���Ŀ¼
			id = curInode.parent;
			if (curInode.id > 0)
			{
				getInode(curInode, id);
				buildFcbLink(curLink, curInode);
				unsigned int pos = curPath.rfind('/', curPath.length() - 2);
				curPath.erase(pos + 1, curPath.length() - 1 - pos);
			}
			return 0;
		}
		// ����Ŀ¼��Ŀ¼
		id = findInodeOfDir(curLink, dirName.c_str());
		if (id > 0) {
			getInode(curInode, id);
			getFcbLink_ByInode(curLink, curInode);
			curPath.append(dirName);
			curPath.append(1, '/');
			return 0;
		}
		cout << "no such file or directory:" << dirName << endl;
		return -1;
	}


	// �����ļ���
	int FileSystem::mv(string fileName, string newName) {
		// �ҵ�Inode�ڵ�
		I_INDEX inodeId = findInodeOfDir(curLink, fileName.c_str());
		if (inodeId > 0) {
			Inode inode;
			getInode(inode, inodeId);
			if (inode.prioritytype == PriorityType::Read_Only) {
				cout << "file " << fileName << " is Read-Only" << endl;
				return -1;
			}
			strcpy(inode.filename, fileName.c_str());	// ����������
			time(&(inode.time));
			setInode(inode);	//����Inode�ڵ�
			FcbLink link = curLink->next;
			while (link != NULL)
			{	// ����fcbLinkĿ¼��Ϣ��
				if (strcmp(link->fcb.filename, fileName.c_str()) == 0) {
					strcpy(link->fcb.filename, newName.c_str());
					return 0;
				}
				link = link->next;
			}
		}
		else
		{
			cout << "no such file or directory:" << fileName << endl;
			return -1;
		}

		return -1;
	}



	int FileSystem::cp(string src, string dest) {

		cout << "unimplement command" << endl;
		return -1;
	}


	int FileSystem::chmod(string fileName, PriorityType pType) {

		I_INDEX inodeId = findInodeOfDir(curLink, fileName.c_str());
		if (inodeId > 0) {// �ҵ����ļ�
			Inode inode;
			getInode(inode, inodeId);

			// ������Ŀ¼
			if (inode.filetype == FileType::IsDir)
			{
				cd(inode.filename);
				FcbLink link = curLink->next;
				while (link != NULL)
				{
					chmod(link->fcb.filename, pType);
					link = link->next;
				}
				cd("..");
			}
			inode.prioritytype = pType;
			time(&(inode.time));
			setInode(inode);
			return 0;
		}
		else
		{
			cout << "no such file or directory:" << fileName << endl;
		}
		return 1;
	}


	// ��ʾ�ļ���
	void FileSystem::showFileName(FcbLink fcbLink) {
		if (fcbLink == NULL)
			return;
		cout << fcbLink->fcb.filename;
		if (fcbLink->fcb.filetype == FileType::IsDir) {
			cout << "/";
		}
		cout << endl;
	}

	// ��ʾ�ļ�������Ϣ
	void FileSystem::showFileInfo(FcbLink fcbLink) {
		if (fcbLink == NULL) return;
		Inode inode;
		getInode(inode, fcbLink->fcb.id);
		if (inode.filetype == FileType::IsDir) cout << 'd';
		else cout << '-';
		cout << 'r';
		if (inode.prioritytype == PriorityType::Read_Write) cout << 'w';
		else cout << '-';
		printf(" %10d", inode.length);
		printf(" %.12s", 4 + ctime(&(inode.time)));
		cout << " " << inode.filename << endl;
	}

	// ��ʾĿ¼�µ��ļ���Ŀ¼��
	void FileSystem::ls() {
		FcbLink link = curLink->next;
		while (link != NULL)
		{
			showFileName(link);
			link = link->next;
		}
	}
	

	void FileSystem::ls_l() {
		FcbLink link = curLink->next;
		Inode inode;
		while (link != NULL)
		{
			inode.id = link->fcb.id;
			getInode(inode, inode.id);
			FcbLink tlink = new FcbLinkNode();
			getFcbLink_ByInode(tlink, inode);
			showFileInfo(tlink);
			link = link->next;
		}
	}

	void FileSystem::stop(int sig) {
		updateResource();
		::exit(sig);
	}
	void FileSystem::exit() {
		cout << "Bye!" << endl;
		stop(0);
	}