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
//				puts("进入QUIT");
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
				portport = handlePort(sentence, newip);
				if(send(sockfd, sentence, CMD_SIZE, 0) < 0){//发送要上传的文件名
					printf("PORT error!%s(%d)\n", strerror(errno), errno); 
				}		
				portlistenfd = createClientListenfd(portport);	//在port模式下进行监听
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
				printf("%s\n", sentence);
				
				//pthread_t pid;
				//int returnValue = pthread_create(&pid,NULL,(void *) thread,sentence); // 成功返回0，错误返回错误编号 

                //printf("进入getFileLength函数的参数是%s", sentence);
                //int fileLen = getFileLength(sentence);              //获取文件的总字节数
                //printf("传输文件的字节数是%d", fileLen);
				if(testRETR(retrCopy, portlistenfd, pasvconnfd, sockfd, MODE, 0) == 1){
//                    puts("RETR结束");
				}
				MODE = LOGGED;
				break;
			case LIST:
			{
			    send(sockfd, sentence, strlen(sentence), 0);    //发送指令
                char listCopy[CONTENT_SIZE]= "\0";
				strcpy(listCopy, sentence);
				n = recv(sockfd, sentence, CMD_SIZE, 0);
				normalizerecv(sentence);
				printf("%s\n", sentence);

			    if(MODE == PASVMODE || MODE == PORTMODE){
                    if(testRETR(listCopy, portlistenfd, pasvconnfd, sockfd, MODE, 1) == 1){
                        //puts("LIST结束");
                    }
                    n = recv(sockfd, sentence, CMD_SIZE, 0);
                    normalizerecv(sentence);
                    printf("%s\n", sentence);
			    }
				MODE = LOGGED;
				break;
			}
			case STOR:
			{
			     //注意STOR只接收一次reply
				if(send(sockfd, sentence, strlen(sentence), 0) < 0) printf("STOR失败");
				strncpy(filename, sentence+5, strlen(sentence)-7);
				testSTOR(sentence, filename, portlistenfd, pasvconnfd, MODE);
				n = recv(sockfd, sentence, CMD_SIZE, 0);
				normalizerecv(sentence);			//将收到的回复末尾的\r\n全部改为\0
				printf("%s\n", sentence);
				MODE = LOGGED;
				break;
			}
			case MKD:
			case CWD:
			case PWD:
			case RMD:
			case RNFR:
			case RNTO:
			case NOCMD:
			{
				n = send(sockfd, sentence, CMD_SIZE, 0);
				n = recv(sockfd, sentence, CONTENT_SIZE, 0);
				normalizerecv(sentence);
				printf("%s\n", sentence);
				break;
			}
		}
	}	
	
}
