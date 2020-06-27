#include "FileSystem.h"

/*************************实现用户的相关操作命令***********************/

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
	char username[10];
	char password[10];

	cout << "login......" << endl;
	cout << "username: ";
	cin >> username;
	cout << "password: ";
	cin >> password;

	if (strcmp(username, this->user.username) == 0 && strcmp(password, this->user.password) == 0);
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

	// 重新进入文件系统
	login();
	init_afterLogin();

}