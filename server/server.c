#include "server.h"
#define COMMONMODE 0
#define LOGINMODE 1					//登录模式
#define PORTMODE 2
#define PASVMODE 3
#define QUITMODE -1
int MODE = COMMONMODE;					//全局变量
char sentence[8192] = "\0";			//发送数据初始化,全局变量
/****************************************************************************************/
int main(int argc, char **argv) {
	int listenfd, connfd;		//服务端最初始的两个套接字
	char newip[20] = "\0";		//用于传输文件的ip地址
	int newport;			//用于传输文件的port,可通过对字符串的截取计算得出
	int portconnfd;			//port模式下server端用来连接的套接字
	int pasvlistenfd;		//pasv模式下server端用来监听的套接字
	listenfd = createlistenfd(6789);//监听并绑定端口
	while (1) {
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
		}
		printf("进入了server的函数\n");
		strcpy(sentence, "220 ftp.ssast.org FTP server ready.\r\n");
		int n = send(connfd, sentence, strlen(sentence), 0);   
		if(n < 0)
		{
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;			//连接多个客户端，不能return
		}   

		while(1)
		{	
			memset(sentence, '\0', sizeof(sentence));
			n = recv(connfd, sentence, 8192, 0);		
			//printf("服务端接受到的字符串为%s\n", sentence);		
			if(n < 0)
			{
				printf("recv error!%s(%d)\n", strerror(errno), errno);  
		 		return 1; 
			}
			else
			{
				printf("%s\n",sentence);
				if(logIn(sentence) == 1){
					MODE = LOGINMODE;		//登录状态
					n = send(connfd, sentence, strlen(sentence), 0); 
					memset(sentence, '\0', strlen(sentence));		//清空
				}	
				else if(passEmail(sentence) == 1){
					n = send(connfd, sentence, strlen(sentence), 0); 
					memset(sentence, '\0', strlen(sentence));		//清空
				}
				else if(syst(sentence) == 1){
					n = send(connfd, sentence, strlen(sentence), 0); 
					memset(sentence, '\0', strlen(sentence));		//清空
				}	
				else if(type(sentence) == 1){
					n = send(connfd, sentence, strlen(sentence), 0); 
					memset(sentence, '\0', strlen(sentence));		//清空
				}	
				else if(strstr(sentence, "PORT") != NULL){			//port指令
					//新建socket用于文件传输，连接client
					newport = port(sentence, newip);
					printf("服务端port函数发送的内容是%s", sentence);
					MODE = PORTMODE;					//PORTMODE模式
					portconnfd = createconnectfd(newport);			//由助教实例计算得出：port = 128*256+79=32847
					n = send(connfd, sentence, strlen(sentence), 0); 	//发送指令还是用之前的connfd
					memset(sentence, '\0', strlen(sentence));		//清空
					
				}	
				else if(retr(sentence, portconnfd, pasvlistenfd, MODE) == 1){			//RETR指令
					if(MODE != PORTMODE || MODE != PASVMODE)
					{
						memset(sentence, '\0', strlen(sentence));		//清空
						strcpy(sentence, "Please login in or set correct mode.");
						n = send(connfd, sentence, strlen(sentence), 0); 	//发送指令还是用之前的connfd
						memset(sentence, '\0', strlen(sentence));		//清空
						continue;
					}
					printf("第一次retr的内容是%s", sentence);
					n = send(connfd, sentence, strlen(sentence), 0); 	//发送指令还是用之前的connfd
					memset(sentence, '\0', strlen(sentence));		//清空
					strcpy(sentence,"226 Transfer complete.");
					n = send(connfd, sentence, strlen(sentence), 0); 	//发送另一条指令
					memset(sentence, '\0', strlen(sentence));		//清空
					MODE = COMMONMODE;
				}
				else if(pasv(sentence) == 1)
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
					MODE = COMMONMODE;
					
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
				}
				memset(sentence, '\0', strlen(sentence));		//清空
			}
		}
	}
	close(listenfd);
}

