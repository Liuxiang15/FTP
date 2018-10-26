#include "cmdtype.h"
#include "string.h"

int judgeCmdType(const char* cmdStr){
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


