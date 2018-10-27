/*
声明了server和client共同的函数
函数1.命令行参数处理
对于server有两个可选参数，分别为port和dir
对于client只有监听server的port，格式也按照-port n来决定(n为端口号)

函数2.对用户输入指令(上传到server指令)判断

*/
#ifndef COMMON_H
#define COMMON_H

extern int handleCmdArgu(int argc, char **argv, char*ip){
	//返回监听端口号默认21,并设置server的ip地址，默认127.0.0.1
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
    else{
        return NOCMD;
    }
}




#endif