#include "FileSystem.h"
//#include <bitset>


struct
{
	unsigned char widthValidated : 1;
	unsigned char heightValidated : 1;
} status;

void StructInfo()
{
	cout << "Hello" << endl;
	cout << "Size of CMD:  " << sizeof(int) << endl;
	cout << "Size of SuperBlock:  " << sizeof(SuperBlock) << endl;
	cout << "Size of FileType:  " << sizeof(FileType) << endl;
	cout << "Size of PriorityType:  " << sizeof(PriorityType) << endl;
	cout << "Size of Inode:  " << sizeof(Inode) << endl;
	cout << "Size of Fcb:  " << sizeof(Fcb) << endl;
	cout << "Size of FcbLinkNode:  " << sizeof(FcbLinkNode) << endl;
	cout << "Size of User:  " << sizeof(User) << endl;
}
int main()
{
	//bool *abc = new bool[100];
	//for (int i = 0; i < 100; i++)
	//	abc[i] = i % 2 == 0 ? true : false;
	//cout << sizeof(bool) << endl;
	////char b[10] = "Assad";
	//ofstream ofin("test.txt");
	//if (ofin.is_open()) {
	//	for (int i = 0; i < 100; ++i)
	//		ofin << abc[i];
	//	cout << ofin.tellp() << endl;;
	//	ofin.close();
	//	delete abc;
	//}
	////int x;
	////int y;
	////string s;

	//ifstream ifin("test.txt");
	//if (ifin.is_open())
	//{
	//	bool x;
	//	for (int i = 0; i < 100; ++i)
	//	{
	//		ifin >> x;
	//		cout << x << " ";
	//	}
	//	cout << endl;
	//	ifin.close();
	//}
	//fstream myfs("testa.tsh", ios::out | ios::binary);
	//if (myfs.is_open()) {
	//	cout << "ok" << endl;
	//	myfs.close();
	//}

	//fstream location_out;
	//string ss;
	//ss = "(1, 2)";
	//location_out.open("locations_out.txt", std::ios::out);  //以写入和在文件末尾添加的方式打开.txt文件，没有的话就创建该文件。
	//if (!location_out.is_open())
	//{
	//	cout << "error" << endl;
	//	return 0;
	//}

	////location_out << ss << endl;
	//location_out << "(" << 5 << ", " << 10 << ") \n";
	//location_out.close();
	StructInfo();

	// FileSystem myFS;
	// auto name = myFS.getSystemName();
	// auto fp = myFS.getSystemAddress();
	//FILE_NAME a;
	// char a[10];
	// bool a;
	// // cout << name << " " << fp << endl;
	// cin >> a;
	// cout << a << endl;
	//std::bitset<32>bit("00000001000000010000000100000001");
	//cout << sizeof(bit) << endl;
	return 0;
}