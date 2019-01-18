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
#include <time.h>

#include "const.h"


extern int handleCmdArgu(int argc, char **argv, char*root){
	//返回端口号默认21 
	int port = 21;
	for(int i = 1; i < argc; i++){
		if(!strcmp(argv[i], "-port")){
			port = atoi(argv[++i]);
		}
		else if(!strcmp(argv[i], "-root")){
			strcpy(root, argv[i]);
		}
	}
	return port;
}


extern int normalizeRecv(char *sentence){
	for(int i = 0; i < strlen(sentence); i++){
		if(sentence[i] == '\r' || sentence[i] == '\n'){
			sentence[i] = '\0';
		}
	}
	return 1;
}

extern int judgeCmdType(char* cmdStr){
	normalizeRecv(cmdStr);
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
	else if(strncmp(cmdStr, "ABOR", 4) == 0){
		return ABOR;
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
		else if(ptr->d_type == 8)    //file
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
	puts("进入createFile函数");
	FILE *fp = fopen(filename, "w");
	int nFileLen = strlen(content);
	if(fwrite(content, sizeof(char), nFileLen, fp) < 0)
	{
		printf("写入文件失败\n");
	}
	fclose(fp);	//关闭文件才会写入
	return 1;
}

extern int dealPort(char* sentence, char*newip){
	puts("进入port函数");
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

/*list处理*/
extern int handleLIST(int connfd, int pasvlistenfd, int portconnfd, int MODE)
{
	char listContent[CONTENT_SIZE] = "\0";
	system("ls -l > list.txt");//导出在list.txt文件
	int nFileLen = 0;
	FILE* fp = fopen("list.txt", "r");
	fseek(fp,0,SEEK_END); //定位到文件末 
	nFileLen = ftell(fp); //文件长度
	fseek(fp,0,SEEK_SET);		//fp指向文件头
	fread(listContent,sizeof(char),nFileLen+1,fp);
	fclose(fp);

	if(MODE == PORTMODE){
		int n = send(portconnfd, listContent, strlen(listContent), 0);	//传输文件需要用到portconnfd
		if(n < 0){
			printf("文件传输有误\n");
			return -1;
		}
		close(portconnfd);
		printf("PORT模式下list函数中send成功\n");
	}
	else if(MODE == PASVMODE){

		int pasvconnfd = accept(pasvlistenfd, NULL, NULL);		//pasv用于传输
		if (pasvconnfd == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			return -1;
		}
		//printf("PASV模式下list函数中accept成功\n");
		int n = send(pasvconnfd, listContent, CONTENT_SIZE, 0);
		if(n < 0){
			printf("文件传输有误\n");
			return -1;
		}
		else{
			//printf("文件传输得到的内容是%s\n", fileContent);
			close(pasvconnfd);		//传输结束
		}
	}
	else{
		return -2;
	}
	
	if(unlink("list.txt") == 0){//删除文件
		printf("删除list.txt成功");
	}
	else{
		printf("删除list.txt失败");
	}
	return 1;
}


/*retr指令处理*/
extern int handleRetr(char*relative_path, char* sentence, int portconnfd,
	int pasvlistenfd, int MODE, int connfd,char filename[])	//sentence, portconnfd, pasvlistenfd, MODE
{
	printf("in server retr The file name is %s\n", filename);
	char path[64] = "\0";	//存储当前绝对目录
	char fileContent[CONTENT_SIZE] = "\0";
	FILE *fp = fopen(filename,"rb");  		//rb,read binary file
	if (fp == NULL){  
		strcpy(sentence, "550 Open file failed\r\n");
		perror("Open file failed\n");  
		printf("异常退出retr函数\n");
		return -2;
	}  
	else{
		//统计txt文件的字节数,只统计字节数是不够的，应该先发送完之后回复字节数
		int nFileLen = 0;
		fseek(fp,0,SEEK_END); //定位到文件末 
		nFileLen = ftell(fp); //文件长度
		/*send the first reply*/
		 memset(sentence, '\0', strlen(sentence));		//清空
		 strcpy(sentence, "50 Opening BINARY mode data connection for ");
		 strcat(sentence, filename);
		 strcat(sentence, " (");
		 char strnum[20] = "\0";
		 snprintf(strnum, 19, "%d", nFileLen);
		 strcat(sentence, strnum);
		 strcat(sentence, " bytes).\r\n");
		 send(connfd, sentence, strlen(sentence), 0);
		 /*发送第一条指令结束*/

		memset(sentence, '\0', strlen(sentence));		//清空
		fseek(fp,0,SEEK_SET);		//fp指向文件头
		fread(sentence,sizeof(char),nFileLen+1,fp);
		printf("服务端读取到的文件内容是%s\n", sentence);
		strcpy(fileContent, sentence);
		fclose(fp);		                                //关闭文件
	}
	//	char transFinish[] = "226 Transfer complete.\r\n";
	//    int n = send(connfd, transFinish, strlen(transFinish), 0); 	//发送指令还是用之前的connfd
	//	if(n < 0){
	//	    puts("第二条指令发送失败");
	//	}
	//	else    puts("第二条指令发送成功");

	//以上为获取RETR的文件内容，接下来是发送
	if(MODE == PORTMODE){
		int n = send(portconnfd, fileContent, strlen(fileContent), 0);		//传输文件需要用到portconnfd
		if(n < 0)
		{
			printf("服务端send文件出错\n");	
			printf("Error send(): %s(%d)\n", strerror(errno), errno);
			return -1;
		}
		close(portconnfd);					//关掉port传输套接字
		memset(sentence, '\0', strlen(sentence));		//清空
		puts("在PORT模式下RETR完成");
		// char transFinish[] = "226 Transfer complete.\r\n";
		// n = send(connfd, transFinish, strlen(transFinish), 0); 
		// if(n < 0)	puts("发送transFInish");
		// else puts("没有发送transFinish2");
		// n = send(connfd, "226 Transfer complete.\r\n", CONTENT_SIZE, 0);
		return 1;
	}
	else if(MODE == PASVMODE){
		int pasvconnfd = accept(pasvlistenfd, NULL, NULL);	//testfd用于传输
		int n = send(pasvconnfd, fileContent, CONTENT_SIZE, 0);
		if(n < 0){
			printf("文件传输有误\n");
//			char transError[] = "transError\r\n";
//			n = send(connfd, transError, strlen(transError), 0);
			return -1;
		}
		else{
			puts("PASV模式下传输文件成功");
			close(pasvconnfd);		//传输结束
			memset(sentence, '\0', strlen(sentence));		//清空
			// char transFinish[] = "226 Transfer complete.\r\n";
			// n = send(connfd, transFinish, strlen(transFinish), 0); 
			// if(n < 0)	puts("发送transFInish");
			// else puts("没有发送transFinish2");	
		}
		return 1;
	}
}


/*STOR指令处理(接收文件)*/
extern int stor(char*sentence, int portconnfd, int pasvlistenfd, int connfd, int MODE, char*filename)	
{
	char fileContent[CONTENT_SIZE] = "\0";				//用来存储接收的文件内容
	int n;
	if(MODE == PORTMODE){
		n = recv(portconnfd, fileContent, CONTENT_SIZE, 0);
		printf("server接收到的内容是：%s", fileContent);
		if(n < 0){
			printf("PORT模式下STOR出错\n");
			return -1;
		}
		else{
			createFile(fileContent,filename);
			close(portconnfd);
			
			// memset(sentence, '\0', strlen(sentence));		//清空
			// strcpy(sentence, "226 Transfer complete.\r\n");
			// n = send(connfd, sentence, strlen(sentence), 0);	//发送第2条指令

			// memset(sentence, '\0', strlen(sentence));		//清空
			// strcpy(sentence, "226 Server has successfully store the file.");
			// n = send(connfd, sentence, strlen(sentence), 0);	//发送第1条指令
			return 1;
		}
	}
	else if(MODE == PASVMODE){
		int pasvconnfd = accept(pasvlistenfd, NULL, NULL);		//pasv用于传输
		if (pasvconnfd == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			return -1;
		}
		printf("PASV模式下stor函数中accept成功\n");
		printf("1server接收到的内容是：%s", fileContent);
		n = recv(pasvconnfd, fileContent, CONTENT_SIZE, 0);
		printf("2server接收到的内容是：%s", fileContent);
		if(n < 0){
			printf("文件传输有误\n");
			return -1;
		}
		else{
			//printf("文件传输得到的内容是%s\n", fileContent);
			createFile(fileContent,filename);
			close(pasvconnfd);		//传输结束
		}

		// memset(sentence, '\0', strlen(sentence));		//清空
		// strcpy(sentence, "Server has successfully store the file.\r\n");
		// n = send(connfd, sentence, strlen(sentence), 0);	//发送第1条指令
		// memset(sentence, '\0', strlen(sentence));		//清空
		// strcpy(sentence, "226 Transfer complete.\r\n");
		// n = send(connfd, sentence, strlen(sentence), 0);	//发送第2条指令
		return 1;
	}
}

//生成随机端口号
extern int random_port()
{
    srand((unsigned)time(0));
    return (rand() % 45536 + 20000);
}

//根据端口号生成字符串h1,h2,h3,h4,p1,p2
extern void getIpPort(char *ip, int port, struct sockaddr_in pasv_addr)
{
    strcpy(ip, "(");
    char temp[20];
    inet_ntop(AF_INET, &pasv_addr.sin_addr, temp, sizeof(temp));
    int i;
    size_t len = strlen(temp);
    for(i = 0; i < len; i++)
        if(temp[i] == '.')
            temp[i] = ',';
    strcat(ip, temp);

    char num[3] = "\0";
    sprintf(num, ",%d,", port / 256);
    strcat(ip, num);
    sprintf(num, "%d", port % 256);
    strcat(ip, num);

    strcat(ip, ")\r\n");
}

extern int dealPasv(char*sentence)
{
	int pasvlistenfd;
	int pasvPort = random_port();
	//strcpy(sentence, "227 Entering Passive Mode (127,0,0,1,102,109)");
	
	/*建立监听*/
	if ((pasvlistenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	struct sockaddr_in pasv_addr;
	memset(&pasv_addr, 0, sizeof(pasv_addr));
	pasv_addr.sin_family = AF_INET;
	//pasv_addr.sin_port = htons(26221);			//按照助教给的端口102*256+109=26221
	pasv_addr.sin_port = htons(pasvPort);
	pasv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	memset(sentence, '\0', strlen(sentence));		//清空
	strcpy(sentence, "227 Entering Passive Mode ");
	char ipPort[100] = "\0";
	getIpPort(ipPort, pasvPort, pasv_addr);
	strcat(sentence, ipPort);

	if (bind(pasvlistenfd, (struct sockaddr*)&pasv_addr, sizeof(pasv_addr)) == -1) {		//bind
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

/*CWD指令处理*/

extern int handleCWD(char*sentence)
{
	char suffix[100] = "\0";
	
	char path[64] = "\0";	//存储当前绝对目录
	if (NULL == realpath("./", path))
	{
		printf("***Error***\n");
		return -1;
	}
	printf("current absolute path:%s\n", path);
	strncpy(suffix, sentence+4, strlen(sentence)-4);//获取文件夹名
	//!!!应该判断当前文件夹是否存在
	if(strcmp(sentence, "..") == 0){
		printf("用户想要回退操作");
		int len = strlen(sentence);
		int maxPos = 0;
		for(int pos = 0;  pos < len; pos++){
			//查找最后一个反斜线所在的位置
			if(sentence[pos] == '\\'){
				maxPos = pos;
			}
		}
		strncpy(path, sentence, (maxPos+1));
	}
	else{
		strcat(path,"/");
		strcat(path,suffix);
	}
	

	//printf("suffix=%s\n", suffix);
	printf("修改后的目录是：%s",path);
	chdir(path);	//修改当前工作目录
	memset(sentence, '\0',strlen(sentence));//清空
	return 1;		

	
}


extern int handleRMD(char *sentence)
{
	
	char delfile[100] = "\0";
	strncpy(delfile, sentence+4, strlen(sentence)-4);
	if(rmdir(delfile) == 0){
		return 1;
	}
	else	return -1;
}



extern int handleRNFR(char*sentence){
	char filename[20] = "\0";
	strncpy(filename, sentence+5, strlen(sentence)-5);
	char rootPath[100] = "\0";
	realpath("./", rootPath);

	DIR *dir;
	struct dirent *ptr;
	if((dir = opendir(rootPath)) == NULL){
		perror("Open dir error.");
		return -1;
	}
	printf("filename is %s", filename);
	while((ptr = readdir(dir)) != NULL){
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
			continue;
		else if(ptr->d_type == 8 && strcmp(filename, ptr->d_name) == 0) { //file
			printf("file_name:%s/%s\n",rootPath,ptr->d_name);
			//return 1;
		} 
		else if(ptr->d_type == 4 && strcmp(filename, ptr->d_name) == 0){	//dir
		
			//针对文件夹可有其他的操作，这里只获取文件名
			printf("dir_name:%s/%s\n",rootPath,ptr->d_name);
			//return 1;
		}
	}
	closedir(dir);
	return 1;
}

#endif 
