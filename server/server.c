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
char transFinish[] = "226 Transfer complete.\r\n";
char pasvReply[] = "227 Entering Passive Mode (127,0,0,1,102,109)\r\n";

char listReply[] = "150 The directory listing is going to transmit.\r\n";
char listTransFinish[] = "226 the entire directory was successfully transmitted.\r\n";
char listNoTCP[] = "425 no TCP connection was established\r\n";


char noPort[] = "503 You have not enter the PORTMODE\r\n";
char noPasv[] = "503 You have not enter the PASVMODE\r\n";

char cwdOK[] = "250 Okay.\r\n";
char cwdError[] = "550 : No such file or directory.\r\n";
char mkdOK[] = "257 Make directory successfully.\r\n";
char mkdError[] = "550 Failed to make directory.\r\n";
char rmdOK[] = "250 Remove directory successfully.\r\n";
char rmdError[] = "550 Failed to remove directory.\r\n";
char RNFROK[] = "350 The file to be named exists.\r\n";
char RNFRError[] = "450 The file to be named does not exist.\r\n";
char noRNFR[] = "550 You have not input RNFR command.\r\n";
char RNTOOK[] = "250 Rename the file successfully.\r\n";

char ABORReply[] = "221 Thank you for using the FTP service. Goodbye.\r\n";

char wrongCmd[] = "503 Wrong commamd.\r\n";
char RETROK[] = "50 Opening BINARY mode data connection\r\n";
char STOROK[] = "226 Server has successfully store the file.\r\n";

int MODE = NOUSER;					//记录server和client状态的全局变量，这里有bug，因为每个用户操作的状态不一样，所以应该单独为每个用户建立一个用户表
char sentence[CONTENT_SIZE] = "\0";			//发送数据初始化,全局变量
int hasRNFR = 0;
char oldName[100] = "\0";

int main(int argc, char **argv) {
	char rootPath[100] = "/tmp";
	int listenPort = handleCmdArgu(argc, argv, rootPath);
	chdir(rootPath);		    //修改工作目录
	
	int listenfd, connfd;		//服务端最初始的两个套接字
	char newip[20] = "\0";		//用于传输文件的ip地址
	int trans_port;			    //用于传输文件的port,可通过对字符串的截取计算得出
	int portconnfd;			    //port模式下server端用来连接的套接字
	int pasvlistenfd;		    //pasv模式下server端用来监听的套接字
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
			//n = recv(connfd, sentence, CMD_SIZE, 0);
			if(recv(connfd, sentence, CMD_SIZE, 0) < 0)
			{
				printf("recv error!%s(%d)\n", strerror(errno), errno);  
		 		break;
			}
			else
			{
				printf("服务端接受到的字符串为%s", sentence);	
				int cmd_type = judgeCmdType(sentence);//判断命令类型
                printf("cmd_type是%d",cmd_type);
				//ABOR和QUIT命令可以随时终止
				if(cmd_type == QUIT || cmd_type == ABOR){
					n = send(connfd, ABORReply, strlen(ABORReply), 0); 
					if(MODE == PORTMODE){
						close(portconnfd);
					}
					else if(MODE == PASVMODE){
						close(pasvlistenfd);
					}
					continue;
				}

				char filename[CMD_SIZE] = "\0";			//存储RETR和STOR指令的文件名
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
								trans_port = dealPort(sentence, newip);
								//printf("客户端传过来的ip是%s", newip);
								portconnfd = createconnectfd(trans_port, newip);//由助教实例计算得出：port = 128*256+79=32847
								n = send(connfd, portOK, strlen(portOK), 0); 	//发送指令还是用之前的connfd
								memset(sentence, '\0', strlen(sentence));		//清空
							}
							break;
						case PASV:
							{
								MODE = PASVMODE;
								pasvlistenfd = dealPasv(sentence);	//返回port
								printf("进入PASV\n");
								n = send(connfd, sentence, strlen(sentence), 0); 	//发送指令还是用之前的connfd
								memset(sentence, '\0', strlen(sentence));		//清空

							}
							break;
						case MKD:
						{
							//默认MKD后面的参数是在当前目录下建立命名pathname的文件夹
							char pathname[100] = "\0";
							strncpy(pathname, sentence+4, strlen(sentence)-4);
							printf("MKD的路径名是%s", pathname);
//							if(mkdir(pathname, S_IRWXU | S_IRWXG) == 0){
								//S_IRWXU  00700权限，代表该文件所有者拥有读，写和执行操作的权限
								//char mkdReply[] = "250 \"";
								//strcat(mkdReply, pathname); 	
								//strcat(mkdReply, "\"");
							if(mkdir(pathname, S_IRWXU | S_IRWXG | S_IRWXO) == 0){
								puts("server make files OK");
								n = send(connfd, mkdOK, strlen(mkdOK), 0);
							}
							else{
							    puts("server make files failed");
								n = send(connfd, mkdError, strlen(mkdError), 0);
							}
							break;

						}
						case CWD:
						{
							if(handleCWD(sentence) == 1){
								n = send(connfd, cwdOK, strlen(cwdOK), 0);
							}
							else{
								n = send(connfd, cwdError, strlen(cwdError), 0);
							}	
							break;
						}
						case PWD:
						{
							//puts("进入PWD处理");
							char namePrefix[100] = "\0";
							// if(realpath("./", namePrefix) == NULL){//将相对路径转变为绝对路径
							// 	printf("***Error***\n");
							// 	char pathError[] = "***Error***\n";
							// 	n = send(connfd, pathError, strlen(pathError), 0);
								
							// } 
							if(getcwd(namePrefix, 100) == NULL){
									char pathError[] = "***Error***\n";
									n = send(connfd, pathError, strlen(pathError), 0);
							}
							else{
								printf("当前工作目录是：%s",namePrefix);
								char pwdReply[] = "250 \"";
								strcat(pwdReply, namePrefix); 
								strcat(pwdReply, "\"");
								strcat(pwdReply,"\r\n");
								n = send(connfd, pwdReply, strlen(pwdReply), 0);
							}
							puts("PWD执行结束");
							//handlePWD(sentence, namePrefix);
							break;
				
						}
						case RMD:
						{
							if(handleRMD(sentence) == 1)	
								n = send(connfd, rmdOK, strlen(rmdOK), 0);
							else
								n = send(connfd, rmdError, strlen(rmdError), 0);
							break;
						}

						case RNFR:
						{
							if(handleRNFR(sentence) == 1){
								hasRNFR = 1;
								strncpy(oldName, sentence+5, strlen(sentence)-5);
								n = send(connfd, RNFROK, strlen(RNFROK), 0);
							}
							else{
								n = send(connfd, RNFRError, strlen(RNFRError), 0);
							}
							break;
						}

						case RNTO:
						{
							char newName[100] = "\0";
							strncpy(newName, sentence+5, strlen(sentence)-5);
							printf("oldname %s newname %s", oldName, newName);
							if(hasRNFR == 1){
								if(rename(oldName, newName) == 0){
									puts("重命名成功");
									n = send(connfd, RNTOOK, strlen(RNTOOK), 0);
								}
								else{
									puts("重命名失败");
									n = send(connfd, noRNFR, strlen(RNTOOK), 0);
								}
							}
							else {
								n = send(connfd, noRNFR, strlen(RNTOOK), 0);
							}
										
							hasRNFR = 0;	
						}

					
						default:
							memset(sentence, '\0', strlen(sentence));		//清空
							strcpy(sentence, "In LOGGED MOde wrong grammer");
							n = send(connfd, portOK, strlen(portOK), 0); 	//发送指令还是用之前的connfd
							memset(sentence, '\0', strlen(sentence));		//清空
					}
				}
				else if(MODE == PORTMODE){
					
					switch(cmd_type){
						case RETR:
							n = send(connfd, RETROK, strlen(RETROK), 0); 	//发送指令还是用之前的connfd
							memset(filename, '\0', strlen(filename));		//清空
							strncpy(filename, sentence+5, strlen(sentence)-5);//截取文件名
							if(handleRetr(rootPath,sentence, portconnfd, pasvlistenfd, MODE, connfd, filename) == -2){
								n = send(connfd, sentence, strlen(sentence), 0); 	
								printf("retr出错服务端发送的指令是%s", sentence);
							}
							// else{
							// 	puts("进入第二次指令的发送");
							// 	n = send(connfd, transFinish, strlen(transFinish), 0);
							// 	if(n > 0){
							// 		puts("第二次指令的发送成功");
							// 	} 	
							// 	memset(sentence, '\0', strlen(sentence));		//清空
								
							// }
							puts("retr完全结束");
							memset(sentence, '\0', strlen(sentence));		//清空
							MODE = LOGGED;	//传输数据进行模式切换
							close(portconnfd);		
							//puts("PORTMODE下RETR结束");
							break;
						case STOR:
							strncpy(filename, sentence+5, strlen(sentence)-5);//截取文件名
							puts("enter STOR\n");
							if(stor(sentence, portconnfd, pasvlistenfd,connfd, MODE, filename) == 1){
								n = send(connfd, STOROK, strlen(STOROK), 0); 	
							}
							memset(sentence, '\0', strlen(sentence));		//清空
							MODE = LOGGED;	//传输数据进行模式切换
							close(portconnfd);		
							break;
						case LIST:
							n = send(connfd, listReply, strlen(listReply), 0); 	//发送指令还是用之前的connfd
							if(handleLIST(connfd, pasvlistenfd, portconnfd, MODE) == -2){
								n = send(connfd, listNoTCP, strlen(listNoTCP), 0); 	
								memset(sentence, '\0', strlen(sentence));		//清空
							}
							else{
								n = send(connfd, listTransFinish, strlen(listTransFinish), 0); 	
								memset(sentence, '\0', strlen(sentence));		//清空
							}
							MODE = LOGGED;	//传输数据进行模式切换
							close(portconnfd);		
							break;
						default:
							n = send(connfd, noPort, strlen(noPort), 0); 	
							memset(sentence, '\0', strlen(sentence));		//清空
					}
				}
				else if(MODE == PASVMODE){
					switch(cmd_type){
						case RETR:
							n = send(connfd, RETROK, strlen(RETROK), 0); 	//发送指令还是用之前的connfd
							memset(filename, '\0', strlen(filename));		//清空
							strncpy(filename, sentence+5, strlen(sentence)-5);//截取文件名

//							n = send(connfd, transFinish, strlen(transFinish), 0); 	//发送指令还是用之前的connfd
//                            if(n < 0){
//                                puts("第二条指令发送失败");
//                            }
//                            else    puts("第二条指令发送成功");

							if(handleRetr(rootPath,sentence, portconnfd, pasvlistenfd, MODE, connfd, filename) == -2){
							    puts("handleRetr的返回值为-2");
								n = send(connfd, sentence, strlen(sentence), 0);
							}
							else{
							    puts("handleRetr的返回值不为-2");
//								n = send(connfd, transFinish, strlen(transFinish), 0);
								memset(sentence, '\0', strlen(sentence));		//清空
							}
							puts("server.c PASV RETR 完成");
							MODE = LOGGED;	//传输数据进行模式切换
							close(pasvlistenfd);
							break;
						case LIST:
							n = send(connfd, listReply, strlen(listReply), 0); 	//发送指令还是用之前的connfd
							if(handleLIST(connfd, pasvlistenfd, portconnfd, MODE) == -2){
								n = send(connfd, listNoTCP, strlen(listNoTCP), 0); 	
								memset(sentence, '\0', strlen(sentence));		//清空
							}
							else{
								n = send(connfd, listTransFinish, strlen(listTransFinish), 0); 	
								memset(sentence, '\0', strlen(sentence));		//清空
							}
							MODE = LOGGED;	//传输数据进行模式切换
							close(pasvlistenfd);
							
							break;
						case STOR:
							strncpy(filename, sentence+5, strlen(sentence)-5);//截取文件名
							puts("enter PASVMODE 下STOR\n");
							strncpy(filename, sentence+5, strlen(sentence)-5);//截取文件名
							puts("enter STOR\n");
							if(stor(sentence, portconnfd, pasvlistenfd,connfd, MODE, filename) == 1){
								n = send(connfd, STOROK, strlen(STOROK), 0); 	
							}
							memset(sentence, '\0', strlen(sentence));		//清空
							MODE = LOGGED;	//传输数据进行模式切换
							close(pasvlistenfd);
							//puts("PASV模式下STOR结束");
							break;
						
						default:
							n = send(connfd, wrongCmd, strlen(wrongCmd), 0); 	
							memset(sentence, '\0', strlen(sentence));		//清空
					}
					// if(cmd_type == RETR || cmd_type == STOR || cmd_type == LIST){
					// 	break;
					// }
				}
				
				
			}
		}

	}
	close(listenfd);
}



