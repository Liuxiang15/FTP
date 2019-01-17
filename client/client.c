#include "client.h"

int MODE = NOUSER;
int main(int argc, char **argv) {
	char serverIP[] = "127.0.0.1";
	int serverListenPort = handleCmdArgu(argc, argv, serverIP);
	//printf("ip地址和端口分别是%s， %d", serverIP, serverListenPort);
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


	int clienttranfd;		//用于RETR传输文件
	char newip[20] = "\0";		//存储传输文件端口的ip地址
	int portport;			//port模式传输文件端口
	
	int pasvconnfd;			//client用于连接服务端传输文件的套接字
	int portlistenfd;
	int pasvport;		//用来连接pasv的端口	
	int listport;		//接收list数据端口
	int listconnfd;		//连接list套接字 

	char filename[CMD_SIZE] = "\0";			//存储RETR和STOR指令的文件名
	char fileContent[CONTENT_SIZE] = "\0";	//用来存储传输的文件内容

	while(fgets(sentence, CMD_SIZE, stdin) != NULL)
	{
		normalizeInput(sentence);	//把所有输入的字符串后加上\r\n

		int cmd_type = judgeCmdType(sentence);

		switch(cmd_type){
			case QUIT:
			case ABOR:
			{
				puts("进入QUIT");
				n = send(sockfd, sentence, CMD_SIZE, 0);		//发送要上传的文件名
				n = recv(sockfd, sentence, CMD_SIZE, 0);
				normalizerecv(sentence);
				printf("%s\n", sentence);
				close(sockfd);	//传输结束
				return 0;
			}
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
				//puts("进入PORT");
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
				
				break;
			case RETR: 
				//puts("进入RETR");
				if(send(sockfd, sentence, strlen(sentence), 0) < 0)	printf("RETR send失败\n");
				// char filename[20] = "\0";
				// strncpy(filename, sentence+5, strlen(sentence)-5);//获取文件名

				char retrCopy[CMD_SIZE]="\0";
				strcpy(retrCopy, sentence);
				n = recv(sockfd, sentence, CMD_SIZE, 0);
				normalizerecv(sentence);
				printf("RETR收到的第一次回复是：%s\n", sentence);

//				n = recv(sockfd, sentence, CMD_SIZE, 0);
//				normalizerecv(sentence);
//				printf("RETR收到的第二次回复是：%s\n", sentence);

				if(testRETR(retrCopy, portlistenfd, pasvconnfd, sockfd, MODE, 0) == 1){
					// //正常返回才需要第二次接收
					// memset(sentence, '\0', CMD_SIZE);		//空串
//					if(recv(sockfd, sentence, CMD_SIZE, 0) < 0)	printf("RETR文件传输完成有误\n");
//					else{
//					    normalizerecv(sentence);
//						puts("接收第2条指令");
//						printf("%s\n", sentence);
//					}
                    puts("RETR结束");

				}
				
				// memset(sentence, '\0', strlen(sentence));		//空串

				MODE = LOGGED;
				break;
			case LIST:
			{
				if(send(sockfd, sentence, strlen(sentence), 0) < 0)	printf("RETR send失败\n");
				char listCopy[CONTENT_SIZE]="\0";
				strcpy(listCopy, sentence);
				n = recv(sockfd, sentence, CMD_SIZE, 0);
				normalizerecv(sentence);
				printf("%s\n", sentence);
				if(testRETR(listCopy, portlistenfd, pasvconnfd, sockfd, MODE, 1) == 1){
					//puts("LIST结束");
				}
				MODE = LOGGED;
				 n = recv(sockfd, sentence, CMD_SIZE, 0);
				 normalizerecv(sentence);
				 printf("%s\n", sentence);

				break;
			}
			case STOR:
			//注意STOR只接收一次回复
				if(send(sockfd, sentence, strlen(sentence), 0) < 0) printf("STOR失败");
				strncpy(filename, sentence+5, strlen(sentence)-7);
				//puts("进入PORTMODE下的STOR指令");
				testSTOR(sentence, filename, portlistenfd, pasvconnfd, MODE);
				n = recv(sockfd, sentence, CMD_SIZE, 0);
				normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
				printf("%s\n", sentence);
				MODE = LOGGED;
				break;
			

			case MKD:		
			//A MKD request asks the server to create a new directory.
			//The MKD parameter is an encoded pathname specifying the directory.
			//If the server accepts MKD (required code 257), 
			//its response 
			//includes the pathname of the directory, 
			//in the same format used for responses to PWD.
			//A typical server accepts MKD with code 250 if the directory was successfully created, 
			//or rejects MKD with code 550 if the creation failed. 
			case CWD:		//It asks the server to set the name prefix to this pathname
			case PWD:		//print the name prefix
			//257 "/home/joe"
			case RMD:
			//An RMD request asks the server to remove a directory. 
			//The RMD parameter is an encoded pathname specifying the directory. 
			case RNFR:
			//A RNFR request asks the server to begin renaming a file. 
			//The RNFR parameter is an encoded pathname specifying the file.
			//A typical server accepts RNFR with code 350 if the file exists, 
			//or rejects RNFR with code 450 or 550 otherwise. 
			case RNTO:
			//A RNTO request asks the server to finish renaming a file.
			// The RNTO parameter is an encoded pathname specifying the new location of the file. 
			//!!!RNTO must come immediately after RNFR; otherwise the server may reject RNTO with code 503.
			//A typical server accepts RNTO with code 250 if the file was renamed successfully, 
			//or rejects RNTO with code 550 or 553 otherwise.
			case NOCMD:
			{
				
				//An RMD request asks the server to remove a directory. 
				//The RMD parameter is an encoded pathname specifying the directory.
				//A typical server accepts RMD with code 250 if the directory was successfully removed, 
				//or rejects RMD with code 550 if the removal failed. 
				n = send(sockfd, sentence, CMD_SIZE, 0);
				n = recv(sockfd, sentence, CONTENT_SIZE, 0);
				normalizerecv(sentence);
				printf("%s\n", sentence);
				break;
			}

			
			
		}
	}	
	
}
