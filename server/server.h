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



//int tranlistenfd, tranconnfd;			//用于STOR文件传输
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
extern int createconnectfd(int port)
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

extern checkport(int port){
	
}
/*创建文件并写入内容*/
extern int createFile(char*content, char* filename)
{
	/*定义函数 FILE * fopen(const char * path,const char * mode);
	r 打开只读文件，该文件必须存在。r+ 打开可读写的文件，该文件必须存在。
	w 打开只写文件，若文件存在则文件长度清为0，即该文件内容会消失。若文件不存在则建立该文件。*/
	/*
	fwrite和fread相对应，负责将准备好的数据写入到文件流中。
	通常情况下，这个函数执行完的时候，只是将数据写入了缓存，磁盘的文件中并不会立即出现刚刚写入的数据,在调用fclose之后，计算机才将缓存中的数据写入磁盘。
	函数原型：size_t fwrite(void *p, size_t size, size_t num, FILE *fp);
	fwrite和fread的参数要表达的意思是一样的，不同的是将*p中的数据写入到文件流中，以及返回值表示成功写入的数目。
	*/
	FILE *fp = fopen(filename, "w");
	int nFileLen = strlen(content);
	if(fwrite(content, sizeof(char), nFileLen, fp) < 0)
	{
		printf("写入文件失败\n");
	}
	fclose(fp);	//关闭文件才会写入
	return 1;
}

extern int logIn(char* sentence)
{
	char *msg = "USER anonymous";
	 if(strstr(sentence, msg) == NULL)
	 {
		 return -1;//失败返回
	 }
	 strcpy(sentence,"331 Guest login ok, send your complete e-mail address as password.\r\n");
	 return 1;	   //成功返回
}

extern int passEmail(char* sentence)
{
	char * msg1 = "PASS";
	char msg2 = '@';
	if(strstr(sentence, msg1) != NULL && strchr(sentence, msg2)!= NULL)
	{
		strcpy(sentence, "230 Guest login ok, access restrictions apply.\r\n");
		return 1;
	}
	else
	{
		return -1;
	}
}


extern int syst(char* sentence)
{
	//
	char * msg = "SYST";
	if(strstr(sentence, msg) != NULL)
	{
		strcpy(sentence, "215 UNIX Type: L8\r\n");
		return 1;
	}
	else
	{
		//printf("ErrorWhenSYST\n");
		return -1;
	}
}
extern int type(char* sentence)
{
	char * msg = "TYPE";
	//printf("%s\n",sentence);
	if(strstr(sentence, msg) != NULL)
	{
		if(strchr(sentence, 'I') != NULL){
			strcpy(sentence, "200 Type set to I.\r\n");
		}
		else{
			strcpy(sentence, "503 the wrong command sequence\r\n");
		}
		return 1;
	}
	else
	{

		//printf("ErrorWhenTYPE\n");
		return -1;
	}
}


extern int port(char* sentence, char*newip){
	char * msg = "PORT";
	printf("port函数中的sentence是%s\n", sentence);
	if(strstr(sentence, msg) != NULL)
	{	
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
		//printf("port函数中的p1和p2分别是%d %d\n", nump1, nump2);
		//printf("ip地址为：%s\n", newip);
		memset(sentence, '\0', strlen(sentence));		//空串
		strcpy(sentence, "200 PORT command successful.\r\n");
		return port;
	}
	else
		return -1;
}
/*retr指令处理*/
int retr(char* sentence, int portconnfd,int pasvlistenfd, int MODE)	//sentence, portconnfd, pasvlistenfd, MODE
{
	//if(MODE <= 1)		//用户未登录或者未设定模式
	//{
	//	strcpy(sentence, "Please Set Mode");
	//	return -1;
	//}
	//这个函数应该还有提取文件名并保存在外部可访问变量的功能
	//if(MODE == 2)		//PORT模式
	//{
		char* msg = "RETR";
		if(strstr(sentence, msg) != NULL)
		{
			char filename[20] = "\0";
			strncpy(filename, sentence+5, strlen(sentence)-5);//截取文件名
			//消除空白符对文件名字符串的影响
			for(int i = 0; i < strlen(sentence)-5; i++)
			{
				if(filename[i] == '\n' || filename[i] == '\t' ||filename[i] == '\r')
					filename[i] = '\0';
			}
			FILE *fp = fopen(filename,"rb");  		//用来二进制打开文件的
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
		else
			return  -1;
	//}
	//else if(MODE == 3)
	//{
	//	return -1;
		/*此处可以补充pasv模式下文件的RETR指令*/
	//}
	
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
