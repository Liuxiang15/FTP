#ifndef SERVER_H
#define SERVER_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>   
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "const.h"



extern int judgeCmdType(const char* cmdStr){
    if(strncmp(cmdStr, "USER", 4) == 0){
        return USER;
    }
    else if(strncmp(cmdStr, "PASS", 4) == 0){
        return PASS;
    }
    else if(strncmp(cmdStr, "RETR", 4) == 0){
        return RETR;
    }
    else if(strncmp(cmdStr, "STOR", 4) == 0){
        return STOR;
    }
    else if(strncmp(cmdStr, "QUIT", 4) == 0){
        return QUIT;
    }
    else if(strncmp(cmdStr, "SYST", 4) == 0){
        return SYST;
    }
    else if(strncmp(cmdStr, "TYPE", 4) == 0){
        return TYPE;
    }
    else if(strncmp(cmdStr, "PORT", 4) == 0){
        return PORT;
    }
    else if(strncmp(cmdStr, "PASV", 4) == 0){
        return PASV;
    }
    else if(strncmp(cmdStr, "MKD", 3) == 0){
        return MKD;
    }
    else if(strncmp(cmdStr, "CWD", 3) == 0){
        return CWD;
    }
    else if(strncmp(cmdStr, "PWD", 3) == 0){
        return PWD;
    }
    else if(strncmp(cmdStr, "LIST", 4) == 0){
        return LIST;
    }
    else if(strncmp(cmdStr, "RMD", 3) == 0){
        return RMD;
    }
    else if(strncmp(cmdStr, "RNFR", 4) == 0){
        return RNFR;
    }
    else if(strncmp(cmdStr, "RNTO", 4) == 0){
        return RNTO;
    }
    else{
        return NOCMD;
    }
}



extern int readFileList(char *rootPath){
	//由根目录读取包含的文件名和文件夹名
	/*
	struct dirent
	{
		long d_ino; 							inode number 索引节点号 
		off_t d_off; 							offset to this dirent 		在目录文件中的偏移 
		unsigned short d_reclen; 				 length of this d_name 文件名长 
		unsigned char d_type; 					 the type of d_name 文件类型 
		char d_name [NAME_MAX+1]; 				file name (null-terminated) 文件名，最长255字符 
	}
	*/
	DIR *dir;
	struct dirent *ptr;
	char base[1000];
	if((dir = opendir(rootPath)) == NULL){
		perror("Open dir error.");
		return -1;
	}
	while((ptr = readdir(dir)) != NULL){
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
			continue;
		else if(ptr->d_type == 8)    ///file
			printf("file_name:%s/%s\n",rootPath,ptr->d_name);
		else if(ptr->d_type == 4){	//dir
			//针对文件夹可有其他的操作，这里只获取文件名
			printf("dir_name:%s/%s\n",rootPath,ptr->d_name);
		}
	}
	closedir(dir);
	return 1;
}
/***********************************************************************************************/
/*创建并返回监听套接字*/
extern int createlistenfd(int port)
{
	int listenfd;
	struct sockaddr_in addr;
	
	//创建socket
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	//设置本机的ip和port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);//监听"0.0.0.0"
	
	//将本机的ip和port与socket绑定
	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		//puts("bangding error");
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	//开始监听socket
	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return listenfd;
}
/*创建并返回连接套接字*/
extern int createconnectfd(int port, char ip[])
{
	int connfd;
	if ((connfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) 
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	struct sockaddr_in newaddr;		
	memset(&newaddr, 0, sizeof(newaddr));
	newaddr.sin_family = AF_INET;
	newaddr.sin_port = htons(port);		//计算得出：port = 128*256+79=32847
	
	//本来应该是连接目标主机的IP地址的，但是因为在自己测试的时候使用的本地的IP地址，所以需要获取本地IP
	if (inet_pton(AF_INET, "127.0.0.1", &newaddr.sin_addr) <= 0) {//转换ip地址:点分十进制-->二进制
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	//连接上目标主机（将socket和目标主机连接）-- 阻塞函数
	if (connect(connfd, (struct sockaddr*)&newaddr, sizeof(newaddr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return connfd;
}


/*创建文件并写入内容*/
extern int createFile(char*content, char* filename)
{
	FILE *fp = fopen(filename, "w");
	int nFileLen = strlen(content);
	if(fwrite(content, sizeof(char), nFileLen, fp) < 0)
	{
		printf("写入文件失败\n");
	}
	fclose(fp);	//关闭文件才会写入
	return 1;
}


extern int port(char* sentence, char*newip){
	puts("enter port f");
	int j = 0;	//ip下标
	int num = 0;	//标识，的数量
	char strp1[4] = "\0";
	char strp2[4] = "\0";
	//获取ip地址:h1,h2,h3,h4,p1,p2
	for(int i = 5; i < strlen(sentence); i++)//ip地址从第五位开始
	{
		if(sentence[i] != ' ' && sentence[i] != ','){
			newip[j++] = sentence[i];
		}
		else if(sentence[i] == ',')
		{
			num++;
			if(num == 4)	//ip地址结束
				break;	//跳出循环
			newip[j++] = '.';
		}
	}
	num = 0;
	int k = 0;
	j = 0;
	for(int i = 5; i < strlen(sentence); i++)
	{
		if(num == 4)
			strp1[j++] = sentence[i];
		else if(num == 5)	
			strp2[k++] = sentence[i];
		if(sentence[i] == ',')
			num++;
		
	}
	int nump1 = atoi(strp1);
	int nump2 = atoi(strp2);
	int port = 256 * nump1 + nump2;		//计算求得port
	printf("port函数中的p1和p2分别是%d %d\n", nump1, nump2);
	printf("ip地址为：%s\n", newip);
	//memset(sentence, '\0', strlen(sentence));		//空串
	//strcpy(sentence, "200 PORT command successful.\r\n");
	return port;

}
/*retr指令处理*/
int retr(char*relative_path, char* sentence, int portconnfd,int pasvlistenfd, int MODE)	//sentence, portconnfd, pasvlistenfd, MODE
{

	char filename[100] = "\0";
	strncpy(filename, sentence+5, strlen(sentence)-5);//截取文件名
	//消除空白符对文件名字符串的影响
	for(int i = 0; i < strlen(sentence)-5; i++)
	{
		if(filename[i] == '\n' || filename[i] == '\t' ||filename[i] == '\r')
			filename[i] = '\0';
	}
	printf("The file name is %s\n", filename);

	char path[64] = "\0";	//存储当前绝对目录
	//if (NULL == realpath(relative_path, path))
	//{
	//    printf("***Error***\n");
	//    return -1;
	//}
	//printf("current absolute path:%s\n", path);

	FILE *fp = fopen(filename,"rb");  		//rb,read binary file
	if (fp == NULL)
	{  
		strcpy(sentence, "Open file failed\n");
		perror("Open file failed\n");  
		printf("异常退出retr函数\n");
		return -1;
	}  
	else{
		//统计txt文件的字节数,只统计字节数是不够的，应该先发送完之后回复字节数
		int nFileLen = 0;
		fseek(fp,0,SEEK_END); //定位到文件末 
		nFileLen = ftell(fp); //文件长度
		memset(sentence, '\0', strlen(sentence));		//清空
		fseek(fp,0,SEEK_SET);		//fp指向文件头
		fread(sentence,1,nFileLen+1,fp);
		printf("服务端读取到的文件是%s\n", sentence);
		char sendContent[1000] = "\0";
		strcpy(sendContent, sentence);
		int n = send(portconnfd, sendContent, strlen(sendContent), 0);		//传输文件需要用到portconnfd
		if(n < 0)
		{
			printf("服务端send文件出错\n");	
			printf("Error send(): %s(%d)\n", strerror(errno), errno);
		}
		close(portconnfd);		//关掉套接字
		memset(sentence, '\0', strlen(sentence));		//清空
		/*以下是连接发送的字符串*/
		strcpy(sentence, "50 Opening BINARY mode data connection for ");	
		strcat(sentence, filename);
		strcat(sentence, " (");
		char strnum[20] = "\0"; 
		snprintf(strnum, 19, "%d", nFileLen);
		strcat(sentence, strnum);
		strcat(sentence, " bytes).");
		printf("返回文件打开结果%s\n",sentence);		//看返回什么东西 
		fclose(fp);		//关闭文件
		printf("%s",sentence);
		return 1;
	}
}

/*pasv指令处理*/
/*返回监听的套接字*/
extern int pasv(char*sentence)
{
	char *msg = "PASV";
	if(strstr(sentence, msg) != NULL)
	{	
		return 1;
	}
	else
		return -1;
}

extern int dealpasv(char*sentence)
{
	int pasvlistenfd;
	memset(sentence, '\0', strlen(sentence));		//清空
	strcpy(sentence, "227 Entering Passive Mode (127,0,0,1,102,109)");
	/*建立监听*/
	if ((pasvlistenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	struct sockaddr_in newaddr;
	memset(&newaddr, 0, sizeof(newaddr));
	newaddr.sin_family = AF_INET;
	newaddr.sin_port = htons(26221);			//按照助教给的端口102*256+109=26221
	newaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(pasvlistenfd, (struct sockaddr*)&newaddr, sizeof(newaddr)) == -1) {		//bind
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	if (listen(pasvlistenfd, 10) == -1) {			//listen
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	printf("dealpasv函数中监听成功\n");
	return pasvlistenfd;
}
/*QUIT和ABOR指令一起处理*/
extern int quit(char*sentence)
{
	char *msg = "QUIT";
	if(strstr(sentence, msg) != NULL ||strstr(sentence, "ABOR"))
	{
		memset(sentence, '\0', strlen(sentence));		//清空
		strcpy(sentence, "221 Goodbye.\r\n");
		return 1;		
	}
	return -1;
	
}
/*CWD指令处理*/
extern int cwd(char*sentence)
{
	char *msg = "CWD";	
	char suffix[20] = "\0";
	if(strstr(sentence, msg) != NULL && strchr(sentence, '/'))
	{
		printf("进入cwd函数\n");
		char path[64] = "\0";	//存储当前绝对目录
		if (NULL == realpath("./", path))
		{
		    printf("***Error***\n");
		    return -1;
		}
		printf("current absolute path:%s\n", path);
		strncpy(suffix, sentence+4, strlen(sentence)-4);
		strcat(path,suffix);
		printf("suffix=%s\n", suffix);
		printf("path=%s\n", path);
		//printf("修改后的目录是：%s",path);
		chdir(path);	//修改当前工作目录
		memset(sentence, '\0',strlen(sentence));//清空
		strcpy(sentence, "250:the directory was successfully reached.\r\n");
		return 1;		
	}
	return -1;
	
}

extern int mkd(char*sentence)
{
	//没有考虑创建失败的情况
	char *msg = "MKD";
	char filename[20] = "\0";
	if(strstr(sentence, msg) != NULL)
	{
		
		strncpy(filename, sentence+4, strlen(sentence)-5);
		printf("filename=%s\n", filename);
		mkdir(filename,0777);		
		/*
		//权限为0777，即拥有者权限为读、写、执行//拥有者所在组的权限为读、写、执行//其它用户的权限为读、写、执行
		*/
		strcpy(sentence, "250:the directory was successfully created\r\n");
		return 1;
	}
	return -1;
}


extern int rmd(char *sentence)
{
	char *msg = "RMD";
	printf("传入的指令是%s\n", sentence);
	if(strstr(sentence, msg) != NULL)
	{
		char delfile[20] = "\0";
		strncpy(delfile, sentence+4, strlen(sentence)-5);
		char order[64] = "rm -r ";
		strcat(order, delfile);
		for(int i = 0; i < strlen(order); i++)
		{
			if(order[i] == '$' || order[i] == '\n'||order[i] == '\r')
			{
				order[i] = '\0';
			}
		}
		system("pwd");
		system(order);
		strcpy(sentence, "250:the directory was successfully removed\r\n");
		return 1;
	}
	return -1;
}

/*STOR指令处理(接收文件)*/
extern int stor(char*sentence, int portconnfd, int pasvlistenfd, int connfd, int MODE)	
{
	printf("传入store函数的pasvlistenfd=%d\n", pasvlistenfd);
	printf("传入store函数的sentence=%s\n", sentence);
	/*if(MODE <= 1)		//用户未登录或者未设定模式
	{
		strcpy(sentence, "Please Set Mode");
		return -1;
	}
	//此时server端用于accept
	if(MODE == 2)	//PORT模式下还没写
		return -1;*/
	char *msg = "STOR";
	//接下来就是MODE=3的情况，也就是PASV模式下
	if(strstr(sentence,msg) != NULL)
	{
		char filename[20] = "\0";
		strncpy(filename, sentence+5, strlen(sentence) - 5);	//获取上传的文件名
		memset(sentence, '\0', sizeof(sentence));		//空串
		char fileContent[8192] = "\0";				//用来存储传输的文件内容
		int pasvconnfd  = accept(pasvlistenfd, NULL, NULL);		//pasv用于传输
		if (pasvconnfd == -1) {
				printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			}
		printf("store函数中accept成功\n");
		int n = recv(pasvconnfd, fileContent, 8192, 0);
		if(n < 0)
		{
			printf("文件传输有误\n");
		}
		else
		{
			printf("文件传输得到的内容是%s\n", fileContent);
			createFile(fileContent,filename);
		}
		close(pasvconnfd);		//传输结束
		close(pasvlistenfd);
		/*******************************************默认执行STOR指令时服务端只发送成功接收*****************************/
		memset(sentence, '\0', strlen(sentence));		//清空
		strcpy(sentence, "Server has successfully store the file");
		n = send(connfd, sentence, strlen(sentence), 0);	//发送第1条指令
		memset(sentence, '\0', strlen(sentence));		//清空
		strcpy(sentence, "226 Transfer complete.");
		n = send(connfd, sentence, strlen(sentence), 0);	//发送第2条指令
		return 1;
	}
	return -1;
}
/*list处理*/
extern int list(int connfd)
{
	/*DIR * dir;
	struct dirent *ptr;
	char path[64] = "\0";	//存储当前绝对目录
	if (NULL == realpath("./", path))
	{
	    printf("***Error***\n");
	    return -1;
	}
	printf("current absolute path:%s\n", path);
	dir = opendir(path);
	while((ptr = readdir(dir)) != NULL)
	{
		printf("%s\n", ptr->d_name);
	}
	//close(dir);*/
	char listcontent[1024] = "\0";
	system("ls -l > mylist.txt");//导出在list.txt文件
	int nFileLen = 0;
	FILE* fp = fopen("mylist.txt", "r");
	fseek(fp,0,SEEK_END); //定位到文件末 
	nFileLen = ftell(fp); //文件长度
	fseek(fp,0,SEEK_SET);		//fp指向文件头
	fread(listcontent,1,nFileLen+1,fp);
	int n = send(connfd, listcontent, strlen(listcontent), 0);		//传输文件需要用到portconnfd
	fclose(fp);
	return 0;
}
#endif 
