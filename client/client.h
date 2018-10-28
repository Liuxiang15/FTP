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
extern int MODE;		//作为客户端的状态变量

extern int normalizeInput(char * sentence)		//把所有输入的字符串后加上\r\n
{
	int len = strlen(sentence);
	sentence[len-1] = '\r';
	strcat(sentence,"\n");//这里默认加上了'\0'字符
	return 0;
}

extern int normalizerecv(char * sentence)		//把所有读入的字符串后加上\r\n
{
	int len = strlen(sentence);
	for(int i = 0; i < len; i++)
	{
		if(sentence[i] == '\r')
		{
			sentence[i] = '\0';
		}
		else if(sentence[i] == '\n')
		{
			sentence[i] = '\0';
		}
	}
	return 0;
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
	addr.sin_port = htons(port);	//默认6789
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

extern int createclientlistenfd(int port)
{
	int clientlistenfd;	//客户端监听
	if ((clientlistenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	struct sockaddr_in newaddr;
	memset(&newaddr, 0, sizeof(newaddr));
	newaddr.sin_family = AF_INET;
	newaddr.sin_port = htons(port);			//按照助教给的端口128*256+79=32847
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
	//FILE *fp = fopen(filename, "w");					/*w 打开只写文件，若文件存在则文件长度清为0，即该文件内容会消失。若文件不存在则建立该文件。*/
	FILE *fp = fopen(filename, "w");
	int nFileLen = strlen(content);
	if(fwrite(content, sizeof(char), nFileLen, fp) < 0)	/*fwrite返回值表示成功写入的数目。*/
	{
		printf("写入文件失败\n");
		return -1;
	}
	fclose(fp);	//关闭文件才会写入
	return 1;
}
/*PORT指令处理,返回端口*/
extern int port(char* sentence, char*newip){
	int j = 0;	//ip下标
	int num = 0;	//标识，的数量
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
/*处理RETR指令*/
extern int retr(char*sentence)
{
	if(strstr(sentence, "RETR") != NULL)
	{
		return 1;
	}
	return -1;
}
/*测试retr函数*/
extern int testRETR(char*sentence, int clientlistenfd, int pasvconnfd, int sockfd, int MODE)
{
	normalizerecv(sentence);
	///!!!在写入文件的时候还应该注意写入字数是否等于fwrite返回字数
	char filename[20] = "\0";
	char fileContent[CONTENT_SIZE] = "\0";		//默认传输文件不超过1024位
	
	strncpy(filename, sentence+5, strlen(sentence)-5);//获取文件名
	FILE *fp = fopen(filename, "w");
	int n;
	//现在只支持PASV 模式下的RETR
	if(MODE ==PASVMODE){
		int readnum = recv(pasvconnfd, fileContent, CONTENT_SIZE, 0);
		if(readnum > 0)
		{
			//size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
			fwrite(fileContent, 1, readnum, fp);		//写入新建文件
		}
		else
		{
			fclose(fp);
			close(pasvconnfd);	//!!!此时不应该关闭文件传输和监听，毕竟不一定值传输一次
		}
	}
	else if(MODE == PORTMODE){
		int testfd  = accept(clientlistenfd, NULL, NULL);	//testfd用于传输
		if (testfd == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
		}
		n = recv(testfd, fileContent, CONTENT_SIZE, 0);				//接收数据
		if(n < 0){
			printf("PORT模式下RETR文件传输有误\n");
			return -1;
		}
		else{
			//printf("进入了testRETR的创建文件函数\n"); 
			if(createFile(filename, fileContent) == 1){
				//puts("createfile OK");
			}
			else{
				puts("createfile Error");
			}
			close(testfd);			//传输结束,关闭
			//close(clientlistenfd);	//监听的话继续，只是之前的传输连接关掉
		}
	}
	else{

	}
	//接下来等待接收server的完结指令
	memset(sentence, '\0', CMD_SIZE);		//空串
	n = recv(sockfd, sentence, CMD_SIZE, 0);		//收到回复的指令
	if(n < 0)	printf("RETR文件传输完成有误\n");
	else{
		normalizerecv(sentence);
		printf("%s\n", sentence);
	}		
	memset(sentence, '\0', strlen(sentence));		//空串
	n = recv(sockfd, sentence, 1024, 0);		//收到回复的指令
	if(n < 0)	printf("RETRError\n");
	else{
		printf("%s\n", sentence);
		normalizerecv(sentence);
	}
	return 1;
}

/*测试STOR指令*/
extern int testSTOR(char *sentence, int sockfd, int pasvconnfd, int MODE)
{
	if(MODE == 2)	//PASVMODE
	{
		printf("进入testSTOR函数\n");
		char storfileContent[30] = "\0";		//默认传输文件不超过19KB 
		char filename[20] = "\0";
		printf("传入的sentence是%s\n", sentence);
		strncpy(filename, sentence+5, strlen(sentence)-5);//获取文件名
		for(int i = 0; i < strlen(filename); i++)
		{
			if(filename[i] == '\r' || filename[i] == '\n')
				filename[i] = '\0';
		}
		printf("文件名是%s\n", filename);
		FILE *fp = fopen(filename, "rb");
		if(fp == NULL)
		{
			printf("文件打开出错\n");
			return -1;
		}
		else
			printf("文件打开正常\n");
		while(1)
		{
			int readnum = fread (storfileContent, sizeof(char), 30, fp);	//隐藏危险
			if(readnum > 0)
			{
				printf("进入STOR的传输过程\n");
				int n = send(pasvconnfd, storfileContent, 30, 0);
				if(n < 0)
				{
					printf("STORsend失败\n");
					return -1;
				}
			}
			else
			{
				fclose(fp);
				close(pasvconnfd);
				printf("STOR传输文件出错\n");
				return 1;
			}
		}
	}
	
}

/**/
/*测list函数*/
extern int testLIST(char*sentence, int clientlistenfd, int pasvconnfd, int sockfd, int MODE)
{
	printf("进入testRETR函数\n");
	FILE *fp;
	if(MODE == 2)			//PASVMODE
	{
		char pasvfileContent[128] = "\0";		//默认传输文件不超过19KB 
		fp = fopen("list.txt", "w");
		if(fp == NULL)
		{
			printf("打开list.txt文件失败\n");
			return 0;
		}
		while(1)
		{
			int readnum = recv(pasvconnfd, pasvfileContent, 128, 0);
			if(readnum > 0)
			{
				fwrite(pasvfileContent, 1, readnum, fp);		//写入新建文件
			}
			else
			{
				fclose(fp);
				close(pasvconnfd);	//关闭文件传输和监听
				break;
			}
		}
		
	}
	printf("LIST已经全部导入list.txt文件里\n");
	char line[1024] = "\0";
	fp = fopen("list.txt", "r");
	if(fp == NULL)
	{
		printf("打开list.txt文件失败\n");
		return 0;
	}
	while(fgets(line, 1024, fp))
	{
		printf("%s\n", line);
	}
	return 0;
}
#endif
