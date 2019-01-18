#ifndef CLIENT_H
#define CLIENT_H
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>

#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>
//用于获取当前目录
#include <sys/types.h>   
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "const.h"

//返回监听端口号默认21,并设置server的ip地址，默认127.0.0.1
extern int handleCmdArgu(int argc, char **argv, char*ip){
	int port = 21;
	for(int i = 1; i < argc; i++){
		if(!strcmp(argv[i], "-port")){
			port = atoi(argv[++i]);
		}
		else if(!strcmp(argv[i], "-ip")){
			strcpy(ip, argv[i]);
		}
	}
	return port;
}

//把所有输入的字符串后加上\r\n
extern int normalizeInput(char * sentence)
{
	int len = strlen(sentence);
	sentence[len-1] = '\r';
	strcat(sentence,"\n");                  //这里默认加上了'\0'字符
	return 0;
}

extern int normalizerecv(char * sentence)		//把所有读入的字符串后加上\r\n
{
	int len = strlen(sentence);
	for(int i = 0; i < len; i++)
	{
		if(sentence[i] == '\r' || sentence[i] == '\n')
		{
			sentence[i] = '\0';
		}
	}
	return 0;
}

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
	else if(strncmp(cmdStr, "ABOR", 4) == 0){
		return ABOR;
	}
    else{
        return NOCMD;
    }
}

/*创建并返回客户端用于连接的套接字*/
extern int createconnectfd(char * ip, int port)//ip为要连接的ip地址
{
	//printf("进入createconnectfd函数\n");
	int sockfd;
	struct sockaddr_in addr;	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);	
	if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return sockfd;
}

extern int createClientListenfd(int port)
{
	int clientlistenfd;	//客户端监听
	if ((clientlistenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	struct sockaddr_in newaddr;
	memset(&newaddr, 0, sizeof(newaddr));
	newaddr.sin_family = AF_INET;
	newaddr.sin_port = htons(port);			
	newaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(clientlistenfd, (struct sockaddr*)&newaddr, sizeof(newaddr)) == -1) {		//bind
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	if (listen(clientlistenfd, 10) == -1) {							//listen
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return clientlistenfd;
}
extern int createFile(char*filename, char*content)
{
	//printf("the file name is %s", filename);
	FILE *fp = fopen(filename, "w");					    /*w 打开只写文件，若文件存在则文件长度清为0，即该文件内容会消失。若文件不存在则建立该文件。*/
	int file_len = strlen(content);
	if(fwrite(content, sizeof(char), file_len, fp) < 0)	/*fwrite返回值表示成功写入的数目。*/
	{
		printf("写入文件失败\n");
		return -1;
	}
	fclose(fp);	//关闭文件才会写入
	return 1;
}
/*PORT指令处理,返回端口*/
extern int handlePort(char* sentence, char*newip){
	int j = 0;	             //ip下标
	int num = 0;	        //标识，的数量
	char strp1[4] = "\0";
	char strp2[4] = "\0";
	
	//获取ip地址:h1,h2,h3,h4,p1,p2
	for(int i = 5; i < strlen(sentence); i++)//ip地址从第五位开始
	{
		if(sentence[i] != ' ' && sentence[i] != ',')
		{
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
	//memset(sentence, '\0', strlen(sentence));		//清空
	//strcpy(sentence, "200 PORT command successful.\r\n");
	//printf("port函数中的port=%d\n",port);
	return port;
	
}
/*最新的处理服务器pasv回复，返回端口*/
extern  int dealpasv(char* sentence, char *newip)
{
	int j = 0;
	int num = 0;//逗号的数量
	char strp1[10] = "\0";
	char strp2[10] = "\0";
	for(int i = 27; i < strlen(sentence); i++)//ip地址从第27位开始
	{
		if(sentence[i] != ' ' && sentence[i] != ',')
		{
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
	for(int i = 27; i < strlen(sentence); i++)
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
	//printf("dealpasv函数中的p1和p2分别是%d %d\n", nump1, nump2);
	//printf("dealpasv中deal的ip地址为：%s\n", newip);
	//printf("dealpasv函数中的port=%d\n",port);	
	return port;
}
/*stor指令处理*/
extern int stor(char* sentence, int newconnfd)
{
	//这个函数应该还有提取文件名并保存在外部可访问变量的功能要补充,目前默认上传的文件名为temp.txt,提取文件名已经实现
	char filename[20] = "\0";
	strncpy(filename, sentence+5, strlen(sentence)-5);//截取文件名
	int n;
	FILE *fp;  		//用来二进制打开文件的 
	if ((fp = fopen(filename,"rb")) == NULL) {  
		strcpy(sentence, "Open file failed\n");
		perror("Open file failed\n");  
		return -1;
	    }  
	else{
		//只统计字节数是不够的，应该先发送完之后回复字节数
		int nFileLen = 0;
		fseek(fp,0,SEEK_END); //定位到文件末 
		nFileLen = ftell(fp); //文件长度
		memset(sentence, '\0', strlen(sentence));		//清空
		fseek(fp,0,SEEK_SET);//定位到文件头
		/*read the data and display it*/
		fread(sentence,1,nFileLen+1,fp);
		char sendContent[1024] = "\0";		//！！！！！！！！！！！！！！！！！！！！！！！！！目前默认不超过1KB
		//printf("stor文件传输中的newconnfd是%d",newconnfd);
		strcpy(sendContent, sentence);
		n = send(newconnfd, sendContent, strlen(sendContent), 0);		//传输文件需要用到newconnfd，???担心在发送sentence的时候执行了以下的赋值操作
		if(n < 0)
		{
			printf("客户端send文件出错\n");	
		}
		close(newconnfd);			//传输文件完成之后关掉套接字
		memset(sentence, '\0', strlen(sentence));		//清空
		strcpy(sentence, "150 Opening BINARY mode data connection for ");
		strcat(sentence, filename);
		strcat(sentence, "(");
		char strnum[20] = "\0"; 
		snprintf(strnum, 19, "%d", nFileLen);
		strcat(sentence, strnum);
		strcat(sentence, " bytes).");
		printf("%s\n",sentence);		//看返回什么东西 
		fclose(fp);		//关闭文件
	}
}

/*测试retr函数*/
extern int testRETR(char*sentence, int clientlistenfd, int pasvconnfd, int sockfd, int MODE, int listFlag)
{
//    puts("进入testRETR函数");
	char filename[20] = "\0";
	char fileContent[CONTENT_SIZE] = "\0";		//默认传输文件不超过8KB
	if(listFlag == 1){
		strcpy(filename, "list.txt");
	}
	else{
		normalizerecv(sentence);
		strncpy(filename, sentence+5, strlen(sentence)-5);//获取文件名
	}
//    puts("testRETR中recv之前");
//	if(recv(sockfd, sentence, CMD_SIZE, 0) <= 0){
//		printf("RETRError\n");
//		return -1;
//	}
//	else{
//		puts("接收第2条指令");
//		normalizerecv(sentence);
//		printf("收到的指令是：%s\n", sentence);
//	}
//	int n;
	//现在只支持PASV 模式下的RETR
	if(MODE == PASVMODE){
//		puts("进入testRETR函数的PASVMODE判断中");
		int readnum = recv(pasvconnfd, fileContent, CONTENT_SIZE, 0);
//		printf("文件内容%s", fileContent);
		if(readnum > 0)
		{
			if(listFlag == 1){
				printf("%s", fileContent);
			}
			else{
//				printf("RETR函数的文件名是：%s\n", filename);
//				printf("RETR函数的内容是：%s\n", fileContent);
				if(createFile(filename, fileContent) == 1){
//					puts("createfile OK");
					char transFinish[] = "226 Transfer complete.\n";
					printf("%s", transFinish);
				}
				else{
					puts("createfile Error");
				}
			}
		}
		else
		    close(pasvconnfd);	//!!!此时不应该关闭文件传输和监听，毕竟不一定值传输一次
	}
	else if(MODE == PORTMODE){
		int trans_connfd  = accept(clientlistenfd, NULL, NULL);	//testfd用于传输
		if (trans_connfd == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
		}
		int n = recv(trans_connfd, fileContent, CONTENT_SIZE, 0);				//接收数据
		
		if(n < 0){
			printf("PORT模式下RETR文件传输有误\n");
			return -1;
		}
		else{
			
			if(listFlag == 1){
				printf("%s", fileContent);
			}
			else{
				if(createFile(filename, fileContent) == 1){	
//					puts("createfile OK");
					char transFinish[] = "226 Transfer complete.\n";
					printf("%s", transFinish);
				}
				else{
					puts("createfile Error");
				}
			}
			close(trans_connfd);			//传输结束,关闭
			close(clientlistenfd);	        //监听的话继续，只是之前的传输连接关掉
		}
	}
	
	return 1;
}

/*测试STOR指令*/
extern int testSTOR(char *sentence, char *filename, int portlistenfd, int pasvconnfd, int MODE)
{
	//printf("进入testSTOR函数\n");
	char fileContent[CONTENT_SIZE] = "\0";		//默认传输文件不超过8KB
	//printf("文件名是%s\n", filename);
	FILE *fp = fopen(filename, "r");
	if(fp == NULL){
		printf("文件打开出错\n");
		return -1;
	}

	fseek(fp,0,SEEK_END);       //定位到文件末
	int fileSize = ftell(fp);   //文件长度
	fseek(fp,0,SEEK_SET);		//fp指向文件头

	if(MODE == PORTMODE){
		int portconnfd  = accept(portlistenfd, NULL, NULL);	//portconnfd用于传输
		if (portconnfd == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
		}
		else{
			int readnum = fread (fileContent, sizeof(char), fileSize, fp);	//隐藏危险//每次读一个，共读size次 
			fclose(fp); 
			if(readnum > 0)
			{
				//printf("读取到的文件内容是%s", fileContent);
				//printf("进入STOR的传输过程\n");
				int n = send(portconnfd, fileContent, fileSize, 0);
				if(n < 0)
				{
					printf("STORsend失败\n");
					return -1;
				}
				close(portlistenfd);
				close(portconnfd);
				//printf("STOR传输文件完成\n");
			}
		}
	}
	else if(MODE == PASVMODE)	//PASVMODE
	{
		//先获取文件大小（待提取）
		int readnum = fread (fileContent, sizeof(char), fileSize, fp);	//隐藏危险//每次读一个，共读size次  
		if(readnum > 0)
		{
			//printf("进入STOR的传输过程\n");
			int n = send(pasvconnfd, fileContent, fileSize, 0);
			if(n < 0)
			{
				printf("STORsend失败\n");
				return -1;
			}
			fclose(fp);
			close(pasvconnfd);
			// printf("STOR传输文件完成\n");
		}
	}
}

#endif
