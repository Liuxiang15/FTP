#include "client.h"
int MODE;
#define NOGINMODE -1
#define LoginMode 1
#define PASVMODE 2
#define PORTMODE 3
int main(int argc, char **argv) {
	//int sockfd =  createconnectfd("166.111.80.127", 5050);//助教测试
	int sockfd =  createconnectfd("127.0.0.1", 21);//本机测试
	char sentence[65535] = "\0";		//存储用户输入的指令
	int n = recv(sockfd, sentence, 1024, 0);
	if(n < 0)
	{
		printf("recv error!%s(%d)\n", strerror(errno), errno);  
 		return 1; 
	}
	else
	{
		printf("%s", sentence);
	}
	int len = 1024;  			//存储指令长度		  
	char*temp = sentence;
	int clienttranfd;		//用于RETR传输文件
	char newip[20] = "\0";		//存储传输文件端口的ip地址
	int portport;			//port模式传输文件端口
	char fileContent[1024] = "\0";	//用来存储传输的文件内容
	int pasvconnfd;			//client用于连接服务端传输文件的套接字
	int portlistenfd;
	int pasvport;		//用来连接pasv的端口	
	int listport;		//接收list数据端口
	int listconnfd;		//连接list套接字 
	while(fgets(sentence, len, stdin) != NULL)
	{
		normalizeget(sentence);
		if(strstr(sentence, "QUIT") != NULL || strstr(sentence, "ABOR") != NULL)
		{
			n = send(sockfd, sentence, 1024, 0);		//发送要上传的文件名
			n = recv(sockfd, sentence, 1024, 0);		//收到第二条指令
			printf("%s", sentence);
			close(sockfd);	//传输结束
			return 0;
		}
		else if(strstr(sentence, "PASV") != NULL) 
		{
			MODE = PASVMODE;
			n = send(sockfd, sentence, strlen(sentence), 0);  
			memset(sentence, '\0', strlen(sentence));	//空串
			n = recv(sockfd, sentence, 1024, 0);
			normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
			printf("%s\n", sentence);
			memset(newip, '\0', strlen(newip));		//空串
			pasvport = dealpasv(sentence, newip);		//pasv返回串的端口
			pasvconnfd = createconnectfd(newip,pasvport);	//pasv模式下连接
			memset(sentence, '\0', strlen(sentence));	//空串
			continue;
			
		}
		else if(strstr(sentence, "PORT") != NULL)		//PORT处理完成
		{
			MODE = PORTMODE;
			portport = port(sentence, newip);
			n = send(sockfd, sentence, 1024, 0);		//发送要上传的文件名
			portlistenfd = createclientlistenfd(portport);	//在port模式下进行监听
			n = recv(sockfd, sentence, 1024, 0);
			normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
			printf("%s\n", sentence);
			continue;
		}
		else if(strstr(sentence, "RETR") != NULL)
		{
			n = send(sockfd, sentence, strlen(sentence), 0);  
			if(n < 0)	printf("RETR send失败\n");
			testRETR(sentence, portlistenfd, pasvconnfd, sockfd, MODE);
			n = recv(sockfd, sentence, 1024, 0);
			normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
			printf("%s\n", sentence);
			memset(sentence, '\0', strlen(sentence));	//空串
			n = recv(sockfd, sentence, 1024, 0);
			continue;
		}
		else if(strstr(sentence, "STOR") != NULL)
		{
			char temp[20] = "\0";
			strcpy(temp, sentence);
			
			n = send(sockfd, sentence, 1024, 0);
			memset(sentence, '\0', strlen(sentence));	//空串
			n = recv(sockfd, sentence, 1024, 0);
			normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
			printf("%s\n", sentence);
			memset(sentence, '\0', strlen(sentence));	//空串
			if(n < 0)	printf("接收150指令出错\n");
			testSTOR(temp, sockfd, pasvconnfd, MODE);
			printf("STOR传输成功\n");
			n = recv(sockfd, sentence, 1024, 0);
			normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
			printf("%s\n", sentence);
			continue;
		}
		else if(strstr(sentence, "LIST") != NULL)
		{

			
			n = send(sockfd, sentence, 65535, 0);
			/*********以下为和助教测试**************/
			memset(sentence, '\0', strlen(sentence));	//空串
			n = recv(sockfd, sentence, 65535, 0);
			printf("%s", sentence);				//!!!此行代码只为本机测试
			continue;					//!!!此行代码只为本机测试
			normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
			printf("%s\n", sentence);
			testLIST(sentence, portlistenfd, pasvconnfd, sockfd, MODE);	//接收LIST传来的信息
			memset(sentence, '\0', strlen(sentence));	//空串
			n = recv(sockfd, sentence, 1024, 0);
			normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
			printf("%s\n", sentence);	
			continue;
		}
		send(sockfd, sentence, strlen(sentence), 0);
		n = recv(sockfd, sentence, 1024, 0);
		normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
		if(n < 0)
		{
			printf("recv error!%s(%d)\n", strerror(errno), errno);  
	 		continue; 
		}
		printf("%s\n", sentence);
		memset(sentence, '\0', strlen(sentence));		//空串
	}	
	
}
