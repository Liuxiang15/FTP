//created in 2018.10.25 by Liuxiang

#ifndef CMDTYPE_H
#define CMDTYPE_H

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

/*
int strncmp(const char *string1, const char *string2, size_t count); 
比较字符串string1和string2大小，只比较前面count个字符. 
比较过程中, 任何一个字符串的长度小于count, 则count将被较短的字符串的长度取代. 
此时如果两串前面的字符都相等, 则较短的串要小. 
返回值< 0, 表示string1的子串小于string2的子串; 
返回值为0, 表示string1的子串等于string2的子串; 
返回值> 0, 表示string1的子串大于string2的子串.
*/

int judgeCmdType(const char* cmdStr);

#endif