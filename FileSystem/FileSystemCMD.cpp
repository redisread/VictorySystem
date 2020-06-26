#include "FileSystem.h"

/*******************实现常用的文件操作命令*********************/



// Help指令
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
    touch   ---  create a new file \n\
    cat     ---  read a file \n\
    write   ---  write something to a file \n\
    rm      ---  delete a directory or a file \n\
    cp      ---  cp a directory file to another directory or file (not finish)\n\
    mv      ---  rename a file or directory \n\
    chmod   ---  change the authorizatino of a directory or a file \n\
    exit    ---  exit this system\n");
}


void FileSystem::cd() {
	;
}