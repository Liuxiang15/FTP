#include <stdio.h>
#include <string.h>
int getFileLength(char* sentence){
   char*pStart = strchr(sentence, '(');
   char*pEnd = strstr(sentence, " bytes");
   char numStr[20] = "\0";
   int i = 0;
   while(pStart++ != pEnd-1){
       numStr[i++] = *pStart;
   }
   printf("%s", numStr);
   return 0;
}

int main(){
	
	char str[] = "50 Opening BINARY mode data connection for retr.json (91503979 bytes).";
	getFileLength(str);
	return 0;
}