#include "server.h"

char initMsg[] = "220 Anonymous FTP server ready.\r\n";
char notLogged[] = "530 You have not logged in. Please login with USER and PASS first.\r\n";
char userOK[] = "331 Guest login ok, send your complete e-mail address as password.\r\n";
char passOK[] = "230 Guest login ok, access restrictions apply.\r\n";
char passError[] = "503 Wrong password.\r\n";
char systReply[] = "215 UNIX Type: L8\r\n";
char typeOK[] = "200 Type set to I.\r\n";
char typeError[] = "503 This ftp only supports TYPE I\r\n";
char portOK[] = "200 PORT command successful.\r\n";
char retrFinish[] = "226 Transfer complete.\r\n";


int MODE = NOUSER;					//记录server和client状态的全局变量，这里有bug，因为每个用户操作的状态不一样，所以应该单独为每个用户建立一个用户表
char sentence[8192] = "\0";			//发送数据初始化,全局变量

int handleCmdArgu(int argc, char **argv, char*root){
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


int main(int argc, char **argv) {
	char rootPath[100] = "/tmp";
	int listenPort = handleCmdArgu(argc, argv, rootPath);
	chdir(rootPath);//change file content
	
	int listenfd, connfd;		//服务端最初始的两个套接字
	char newip[20] = "\0";		//用于传输文件的ip地址
	int newport;			//用于传输文件的port,可通过对字符串的截取计算得出
	int portconnfd;			//port模式下server端用来连接的套接字
	int pasvlistenfd;		//pasv模式下server端用来监听的套接字
	listenfd = createlistenfd(listenPort);//监听并绑定端口
	int n;
	//持续监听连接请求
	while (1) {
		//struct sockaddr_in cli_addr;
		//memset(&cli_addr, 0, sizeof(cli_addr));
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			//printf("出错的客户端口是%d\n", ntohl(cli_addr.sin_port));
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
			//break;
			//break;
		}
		printf("进入了server的函数\n"); 
		if(send(connfd, initMsg, strlen(initMsg), 0) < 0)
		{
			printf("Error send(): %s(%d)\n", strerror(errno), errno);
			continue;	//连接多个客户端，不能return
		}   

		while(1)
		{	
			memset(sentence, '\0', sizeof(sentence));
			n = recv(connfd, sentence, 8192, 0);			
			if(n < 0)
			{
				printf("recv error!%s(%d)\n", strerror(errno), errno);  
		 		continue;
			}
			else
			{
				printf("服务端接受到的字符串为%s", sentence);	
				int cmd_type = judgeCmdType(sentence);//判断命令类型

				//根据用户的不同状态进行操作
				if(MODE == NOUSER){
					switch(cmd_type){
						case USER:
							MODE = NOPASS;
							n = send(connfd, userOK, strlen(userOK), 0); 
							memset(sentence, '\0', strlen(sentence));		//清空
							break;
						default:
							n = send(connfd, notLogged, strlen(notLogged), 0); 
							memset(sentence, '\0', strlen(sentence));		//清空
					}
				}
				else if(MODE == NOPASS){
					switch (cmd_type){
						case PASS:
							MODE = LOGGED;
							n = send(connfd, passOK, strlen(passOK), 0); 
							memset(sentence, '\0', strlen(sentence));		//清空
							break;
						default:
							n = send(connfd, passError, strlen(passError), 0); 
							memset(sentence, '\0', strlen(sentence));		//清空
					}
				}
				else if(MODE == LOGGED){
					switch (cmd_type){
						//其实并不是只有在登录状态下才可以发SYST指令
						case SYST:
							n = send(connfd, systReply, strlen(systReply), 0); 
							memset(sentence, '\0', strlen(sentence));		//清空
							break;
						case TYPE:
							{
								if(strncmp(sentence, "TYPE I", 6) == 0){
									n = send(connfd, typeOK, strlen(typeOK), 0); 	
								}
								else{
									n = send(connfd, typeError, strlen(typeError), 0); 
								}
								memset(sentence, '\0', strlen(sentence));		//清空
							}
							break;
						case PORT:
							{
								//新建socket用于文件传输，连接client
								MODE = PORTMODE;
								newport = port(sentence, newip);
								//printf("客户端传过来的ip是%s", newip);
								portconnfd = createconnectfd(newport, newip);			//由助教实例计算得出：port = 128*256+79=32847
								n = send(connfd, portOK, strlen(portOK), 0); 	//发送指令还是用之前的connfd
								memset(sentence, '\0', strlen(sentence));		//清空
							}
							break;
						case PASV:
							{
								MODE = PASVMODE;
								pasvlistenfd = dealpasv(sentence);	//返回port
								printf("进入PASV\n");
							}
						default:
							memset(sentence, '\0', strlen(sentence));		//清空
					}
				}
				else if(MODE == PORTMODE){
					switch(cmd_type){
						case RETR:
							//puts("enter RETR");
							//readFileList(rootPath);
							normalizeRecv(sentence);
							retr(rootPath,sentence, portconnfd, pasvlistenfd, MODE,connfd);
							n = send(connfd, retrFinish, strlen(retrFinish), 0); 	//发送另一条指令
							memset(sentence, '\0', strlen(sentence));		//清空
							break;
						case STOR:
							puts("enter STOR\n");
							memset(sentence, '\0', strlen(sentence));		//清空
							break;
						
					}

				}
				else if(MODE == PASVMODE){
					
				}
				
				/*
				if(pasv(sentence) == 1)
				{	
				
					pasvlistenfd = dealpasv(sentence);	//返回套接字
					printf("执行完pasv函数的listenfd为%d\n", pasvlistenfd);
					MODE = PASVMODE;			//pasv模式
					printf("进入PASV\n");
					n = send(connfd, sentence, strlen(sentence), 0); 	//发送指令还是用之前的connfd
					memset(sentence, '\0', strlen(sentence));		//清空
				}	
				else if(stor(sentence, portconnfd, pasvlistenfd,connfd, MODE) == 1)		//根据pasvlistenfd新建connfd用于传输或者传入portconnfd传输
				{
					if(MODE != PORTMODE || MODE != PASVMODE)
					{
						memset(sentence, '\0', strlen(sentence));		//清空
						strcpy(sentence, "Please login in or set correct mode.");
						n = send(connfd, sentence, strlen(sentence), 0); 	//发送指令还是用之前的connfd
						memset(sentence, '\0', strlen(sentence));		//清空
						continue;
					}
					printf("传入stor函数的pasclistenfd是%d\n",pasvlistenfd);
					printf("STOR文件成功\n");
					memset(sentence, '\0', strlen(sentence));		//清空
					MODE = NOUSER;
					
				}		
				else if(quit(sentence) == 1)
				{
					n = send(connfd, sentence, strlen(sentence), 0);
					memset(sentence, '\0', strlen(sentence));		//清空
					close(portconnfd);					//quit和abor指令终止除了服务端最初监听的listenfd之外的其他连接
					close(connfd);
					break;							//继续监听其他客户端的连接请求
				}
				else if(cwd(sentence) == 1)
				{
					n = send(connfd, sentence, strlen(sentence), 0);
					memset(sentence, '\0', strlen(sentence));		//清空
				}
				else if(mkd(sentence) == 1)
				{
					n = send(connfd, sentence, strlen(sentence), 0);
					memset(sentence, '\0', strlen(sentence));		//清空
				}
				else if(rmd(sentence) == 1)
				{

					n = send(connfd, sentence, strlen(sentence), 0);
					memset(sentence, '\0', strlen(sentence));		//清空
				}
				else if(strstr(sentence, "LIST") != NULL)
				{
					list(connfd);
				}
				else
				{
					//printf("出错的指令是%s\n", sentence);
					strcpy(sentence,"指令出错\r\n");
					printf("此时的指令为%s\n", sentence);
					n = send(connfd, sentence, strlen(sentence), 0);
				}*/
				memset(sentence, '\0', strlen(sentence));		//清空
			}
		}
	}
	close(listenfd);
}



