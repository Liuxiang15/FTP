/*
声明了1.命令的标志2.用户状态的标志
*/
#ifndef CONST_H
#define CONST_H


#define USER 0
#define PASS 1
#define RETR 2
#define STOR 3
#define QUIT 4
#define SYST 5
#define TYPE 6
#define PORT 7
#define PASV 8
#define MKD  9
#define CWD  10
#define PWD  11
#define LIST 12
#define RMD  13
#define RNFR 14
#define RNTO 15
#define NOCMD 16
#define ABOR 17

#define NOUSER	0		//还没输入USER指令
#define NOPASS 4		//输入USER指令后未输入密码
#define LOGGED 1		//登录模式
#define PORTMODE 2		//输入合法的PORT指令
#define PASVMODE 3		//输入合法的PASV指令
#define QUITMODE -1

#define CMD_SIZE 128
#define CONTENT_SIZE 2048


#endif