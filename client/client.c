#include "client.h"
#include "common.h"

int MODE = NOUSER;
int main(int argc, char **argv) {
	char serverIP[] = "127.0.0.1";
	int serverListenPort = handleCmdArgu(argc, argv, serverIP);
	int sockfd =  createconnectfd(serverIP, serverListenPort);//本机测试
	char sentence[CMD_SIZE] = "\0";		//存储用户输入的指令
	int n;
	if(recv(sockfd, sentence, CMD_SIZE, 0) < 0)
	{
		printf("First recv error!%s(%d)\n", strerror(errno), errno);  
 		return 1; 
	}
	else
	{
		printf("%s", sentence);
	}		  

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

	while(fgets(sentence, CMD_SIZE, stdin) != NULL)
	{
		normalizeInput(sentence);
		int cmd_type = judgeCmdType(sentence);

		switch(cmd_type){
			case USER:
			case PASS:
			case SYST:
			case TYPE:
				send(sockfd, sentence, strlen(sentence), 0);
				if(recv(sockfd, sentence, CMD_SIZE, 0) < 0)
				{
					printf("recv error!%s(%d)\n", strerror(errno), errno);  
					continue; 
				}
				normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
				printf("%s\n", sentence);
				memset(sentence, '\0', CMD_SIZE);		//空串
				break;
			case PORT:
				MODE = PORTMODE;
				portport = port(sentence, newip);
				if(send(sockfd, sentence, CMD_SIZE, 0) < 0){//发送要上传的文件名
					printf("PORT error!%s(%d)\n", strerror(errno), errno); 
				}		
				portlistenfd = createclientlistenfd(portport);	//在port模式下进行监听
				if(recv(sockfd, sentence, CMD_SIZE, 0) < 0){
					printf("PORT Listen error!%s(%d)\n", strerror(errno), errno); 
				}
				normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
				printf("%s\n", sentence);
				memset(sentence, '\0', CMD_SIZE);		//空串
				break;
			case PASV:
				MODE = PASVMODE;
				n = send(sockfd, sentence, CMD_SIZE, 0);  
				memset(sentence, '\0', CMD_SIZE);	//空串
				n = recv(sockfd, sentence, CMD_SIZE, 0);
				normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
				printf("%s\n", sentence);
				memset(newip, '\0', strlen(newip));		//空串
				pasvport = dealpasv(sentence, newip);		//pasv返回串的端口
				if((pasvconnfd = createconnectfd(newip,pasvport)) != -1){//pasv模式下连接,如果连接出错的话再函数内部就会报错了
					memset(sentence, '\0', CMD_SIZE);	//空串
				}
			case RETR: 
				if(send(sockfd, sentence, strlen(sentence), 0) < 0)	printf("RETR send失败\n");
				testRETR(sentence, portlistenfd, pasvconnfd, sockfd, MODE);
			
		}

		// if(strstr(sentence, "QUIT") != NULL || strstr(sentence, "ABOR") != NULL)
		// {
		// 	n = send(sockfd, sentence, CMD_SIZE, 0);		//发送要上传的文件名
		// 	n = recv(sockfd, sentence, 1024, 0);		//收到第二条指令
		// 	printf("%s", sentence);
		// 	close(sockfd);	//传输结束
		// 	return 0;
		// }
		// else if(strstr(sentence, "RETR") != NULL)
		// {
		// 	testRETR(sentence, portlistenfd, pasvconnfd, sockfd, MODE);
		// 	n = recv(sockfd, sentence, 1024, 0);
		// 	normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
		// 	printf("%s\n", sentence);
		// 	memset(sentence, '\0', strlen(sentence));	//空串
		// 	n = recv(sockfd, sentence, 1024, 0);
		// 	continue;
		// }
		// else if(strstr(sentence, "STOR") != NULL)
		// {
		// 	char temp[20] = "\0";
		// 	strcpy(temp, sentence);
			
		// 	n = send(sockfd, sentence, 1024, 0);
		// 	memset(sentence, '\0', strlen(sentence));	//空串
		// 	n = recv(sockfd, sentence, 1024, 0);
		// 	normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
		// 	printf("%s\n", sentence);
		// 	memset(sentence, '\0', strlen(sentence));	//空串
		// 	if(n < 0)	printf("接收150指令出错\n");
		// 	//这里用到了MODE
		// 	//!!!testSTOR(temp, sockfd, pasvconnfd, MODE);
		// 	printf("STOR传输成功\n");
		// 	n = recv(sockfd, sentence, 1024, 0);
		// 	normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
		// 	printf("%s\n", sentence);
		// 	continue;
		// }
		// else if(strstr(sentence, "LIST") != NULL)
		// {
		// 	n = send(sockfd, sentence, 65535, 0);
		// 	/*********以下为和助教测试**************/
		// 	memset(sentence, '\0', strlen(sentence));	//空串
		// 	n = recv(sockfd, sentence, 65535, 0);
		// 	printf("%s", sentence);				//!!!此行代码只为本机测试
		// 	continue;					//!!!此行代码只为本机测试
		// 	normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
		// 	printf("%s\n", sentence);
		// 	//!!!testLIST(sentence, portlistenfd, pasvconnfd, sockfd, MODE);	//接收LIST传来的信息
		// 	memset(sentence, '\0', strlen(sentence));	//空串
		// 	n = recv(sockfd, sentence, 1024, 0);
		// 	normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
		// 	printf("%s\n", sentence);	
		// 	continue;
		// }
		// send(sockfd, sentence, strlen(sentence), 0);
		// n = recv(sockfd, sentence, 1024, 0);
		// normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
		// if(n < 0)
		// {
		// 	printf("recv error!%s(%d)\n", strerror(errno), errno);  
	 	// 	continue; 
		// }
		// printf("%s\n", sentence);
		// memset(sentence, '\0', strlen(sentence));		//空串
	}	
	
}
