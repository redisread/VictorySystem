#include "FileSystem.h"

/*************************ʵ���û�����ز�������***********************/

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
	// ����ȷ�Ļ�����ݹ�
	cout << "The Username or Password is Incorrect,try again!" << endl;
	this->login();
}


void FileSystem::logout() {
	// ��������

	// ���½����ļ�ϵͳ
	login();
	init_afterLogin();

}