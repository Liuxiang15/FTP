1.linux 复制文件或文件夹
用cp或者mv命令就可以拷贝或者移动文件
cp 文件名 /tmp 如果是目录 cp -r 目录名 /tmp
有时需要root权限

2.添加了显示某一目录下所有文件和文件夹的函数

3.规范了命令行指令
client端的命令行指令格式是：
./client -port n -ip xxx.xxx.xxx.xxx

4.client输入处理
13回车键，即\r，10是换行键，即\n 
OK
字符0是OASCII79
字符1是KASCII75
字符2是
ASCII10
字符3是ASCII0
------------------------------------------
the length of new sentence is 4
字符0是OASCII79
字符1是KASCII75
ASCII13
字符3是
ASCII10
字符4是ASCII0
5.
在执行switch-case结构的时候遇到了break，就会结束这个switch-case
break是可以省略的，如果省略了break，就会往下一个case项穿透，直到遇到break或者这个switch-case结束为止。
default是可以省略的，不会有语法错误。  
如果switch后面的表达式有可能出现的值都在case项里面被罗列出来了，
那么就永远不可能执行default,此时就可以省略default。(反正我不建议省略)

6.
int a, b;
	if((a = -1) > 0)puts("Yes 1");	No
	printf("%d", a); a=-1
	if(b = -2 > 0)  puts("Yes 2");	No
	printf("%d", b); b=0
