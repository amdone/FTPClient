#ifndef __FTP_H_
#define __FTP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

/* -------------------------------------------全局变量/常量-------------------------------------------*/
// FTP服务器默认的命令端口/传输控制信息的端口
#define DEFAULT_FTP_PORT 21

// ftp usr
char user[64];
// ftp passwd
char passwd[64];
// 记录从服务器接收的消息
static char rcv_buf[512];
// 记录ftp_get_reply_fromServer执行一次从服务器接收的长度大于0的消息的条数
static int rcv_count = 0;
// 用于printf("[@ftp_cli]> ");
char cmdHead[100];
// 记录客户端路径(用于切换)
char localPath[100];
// 控制cd使用"cd ../"的上限
int limit_gotoUp = 0;

// sockaddr_in结构用于指明FTP服务器的地址信息
struct sockaddr_in ftp_server_sockaddr;
// host entry的缩写，该结构记录主机的信息，包括主机名、别名、地址类型、地址长度和地址列表。
struct hostent *ftp_server_hostent;
// 与服务器端的命令套接口(对应命令端口)相连通的客户端套接口的整型标识符
int sockfd;

// FTP工作模式, 0 is PORT, 1 is PASV，这里仅考虑PASV，即mode = 1;
const int mode = 1;

// FTP服务器端的响应码及其对应的信息(这些变量并未用到，这里只是用于了解相应的响应码)
const char *info220 = "220 myFTP Server ready...\r\n";
const char *info230 = "230 User logged in, proceed.\r\n";
const char *info331 = "331 User name okay, need password.\r\n";
const char *info221 = "221 Goodbye!\r\n";
const char *info150 = "150 File status okay; about to open data connection.\r\n";
const char *info226 = "226 Closing data connection.\r\n";
const char *info200 = "200 Command okay.\r\n";
const char *info215 = "215 Unix Type FC5.\r\n";
const char *info213 = "213 File status.\r\n";
const char *info211 = "211 System status, or system help reply.\r\n";
const char *info350 = "350 Requested file action pending further infromation.\r\n";
const char *info530 = "530 Not logged in.\r\n";
const char *info531 = "531 Not root client. Anonymous client.\r\n";
const char *info202 = "202 Command not implemented, superfluous at this site.\r\n";

/* -------------------------------------------函数声明-------------------------------------------*/
/**
 * @description 帮助函数，用于程序执行出错时
 *				https://blog.csdn.net/mao834099514/article/details/52303074
 */
void show_help();

/**
 * @description 当输入的命令在执行过程中出错时(但并没有异常退出)，执行该函数
 */
void commandError();

/**
 * @description 执行命令出错而导致程序异常退出时，执行该函数
 */
void cmd_run_err_exit(char *err_msg, int err_code);

/**
 * @description 填充(服务器)主机的地址信息，即struct sockaddr_in ftp_server_sockaddr
 */
int fill_host_sockaddr(char *ip, struct sockaddr_in *sockaddr, int port);


/**
 * @description 连接FTP服务器，返回连接的服务器端的套接口标识符/描述字s
 *				(s在connect()后就用来标识连通服务器端的某个(命令/数据)套接口的客户端的套接口标识)
 */
int ftp_connect_server(struct sockaddr_in *s_addr, int type);

/**
 * @description 通过与FTP服务器的命令端口的套接字相连通的客户端套接口的描述字，向FTP服务器发送命令(字符串)
 * @s1 一个命令字符串的一部分命令段(首)
 * @s2 一个命令字符串的一部分命令段
 * @sockfd 套接口标识符/描述字
 */
int ftp_send_cmd_toServer(const char *s1, const char *s2, int sockfd);

/**
 * @description 从客户端相应的套接口获取从服务器端(命令/数据)套接口发送过来的消息
 *				服务器端的命令套接口发送回来的是响应码
 *				服务器端的数据套接口发送回来的是数据
 */
int ftp_get_reply_fromServer(int sockfd);

/**
 * @description 获取FTP服务器端的数据端口，区别于命令端口
 */
int ftp_get_dataPort_fromServer();

/**
 * 根据FTP服务器的数据端口port，获取与此数据端口对应的套接口标识符/描述字连接的客户端套接口描述字
 * 用来创建数据端口对应的套接字
 */
int ftp_get_dataPort_sockid();

/**
 * @description get user and password for login(注意这里设置的是不允许匿名登录！)
 */
void get_user();
void get_pass();

/**
 * @description login to the server
 */
int ftp_login();

/**
 * @description get filename(s) from user's command
 *				从用户输入的命令字符串中获取相应的文件名，与ftp_size()、ftp_list()、ftp_get()和ftp_put()函数搭配
 * ftp_cmd_getFileOrDir_path()最好返回一个err，来判断是否出错，给调用他的函数一个响应信息，这里忘了添加！！！
 */
void ftp_cmd_getFileOrDir_path(char *cmdStr, char *path);

/**
 * 获取命令头前缀，如[@ftp_cli home]>
 */
void ftp_cmdHead();

/**
 * @description 显示客户端当前目录或指定目录的一级子目录和文件
 */
void ftp_llist(char *cmdStr);

/**
 * @description deal with the "list" command
 *				显示服务器端当前目录或指定目录的一级子目录和文件
 */
void ftp_list(char *cmdStr);

/**
 *  @description get fileSize of a file in the server
 *				获取服务器端指定文件的大小(以字节为单位)
 */
void ftp_size(char *cmdStr);

/**
 *  @description get fileSize of a file in the client
 *				获取客户端指定文件的大小(以字节为单位)
 */
void ftp_lsize(char *cmdStr);

/**
 *  @description change the directory in the server
 *				切换服务器端的目录
 */
void ftp_cd(char *cmdStr);

/**
 *  @description 显示当前的目录
 */
void ftp_pwd();

/**
 * @description 显示客户端当前的目录
 */
void ftp_lpwd();

/**
 *  @description 创建目录
 */
void ftp_mkdir(char *cmdStr);

/**
 *  @description 删除目录
 */
void ftp_rmdir(char *cmdStr);

/**
 *  @description 删除文件
 */
void ftp_delete_file(char * cmdStr);

/**
 *  @description 切换本地目录
 */
void ftp_lcd(char *cmdStr);

/**
 * @description deal with the "get" command
 *				下载
 */
void ftp_get(char *cmdStr);

/**
 * @description deal with "put" command
 *				上传
 */
void ftp_put(char *cmdStr);

/**
 *  @description close the connection with the server
 *				断开与服务器端的连接
 */
void ftp_close();

/**
 *  @description call this function to quit
 *				退出FTP客户端
 */
void ftp_quit();

/**
 * @description 根据命令字符串参数返回相应的命令ID
 * @param cmdStr 用户输入的命令字符串(注意在使用前就已经将'\r\n' => '\0')
 * @return 命令字符串对应的整型ID，如果不属于内置命令字符串，则返回-1
 */
int ftp_cmdStrToId(char *cmdStr);

/**
 * @description 尝试连接FTP服务器，连接后，进行相应的命令操作
 */
int build_connection(char *ip, int port);

/**
 * @description 连接FTP服务器
 */
int ftp_openStart();

/* -------------------------------------------函数定义-------------------------------------------*/
/**
 * 统计一个字符串中某个字符出现的次数
 */
int countChar(char *str, char a) {
    int n = 0;
    int i = 0;
    while(*(str+i) != '\0'){
        if(a == *(str+i))
            n++;
        i++;
    }
    return n;
}

/**
 * 显示FTP客户端版本号
 */
void show_version() {
	printf("\033[31mFTP Client version: 1.0\033[0m\t\n");
}

void show_help() {
	printf("\033[33m-------------------------FTP Client-----------------------------\033[0m\n");
	printf("\033[32mversion\033[0m\t\t\t\t--show this ftp client's version\n");
	printf("\033[32mopen [ip]\033[0m\t\t\t--open the server\n");
	printf("\033[32mclose\033[0m\t\t\t\t--close the connection with the server\n");
	printf("\033[32mclear\033[0m\t\t\t\t--make your terminal like a new born baby\n");
	printf("\033[32mpwd\033[0m\t\t\t\t--show server's working path\n");
	printf("\033[32mlpwd\033[0m\t\t\t\t--show local working path\n");
	printf("\033[32mls [directory]\033[0m\t\t\t--list server files and directoris in current directory\n");
	printf("\033[32mlls [directory]\033[0m\t\t\t--list local files and directoris in current directory\n");
	printf("\033[32mcd [directory]\033[0m\t\t\t--switch directory on ftp server\n");
	printf("\033[32mlcd [directory]\033[0m\t\t\t--switch local directory\n");
	printf("\033[32mmkdir [directory]\033[0m\t\t--make a directory on ftp server\n");
	printf("\033[32mrmdir [directory]\033[0m\t\t--remove  directory on ftp server\n");
	printf("\033[32msize [remote_file]\033[0m\t\t--return the size of server's file\n");
	printf("\033[32mlsize [local_file]\033[0m\t\t--return the size of local file\n");
	printf("\033[32mput [local_file]\033[0m\t\t--send [local_file] to server\n");
	printf("\033[32mget [remote_file]\033[0m\t\t--get [remote_file] to local host\n");
	printf("\033[32mdel [remote_file]\033[0m\t\t--drop your selected file like a garbage\n");
	printf("\033[32mbye or quit\033[0m\t\t\t--quit this ftp client program\n");
	printf("\033[33m----------------------------------------------------------------\033[0m\n");
}

void commandError() {
	printf("Command error!\n");
	show_help();
}

void cmd_run_err_exit(char *err_msg, int err_code) {
	printf("%s\n", err_msg);
	exit(err_code);
}

int fill_host_sockaddr(char *ip, struct sockaddr_in *sockaddr, int port) {
	// Invalid port
	if(port <= 0 || port > 65535)
		return 254;
	bzero(sockaddr, sizeof(struct sockaddr_in));
	// TCP
	sockaddr->sin_family = AF_INET;
	// inet_addr(ip)将一个点分十进制的char*类型的ip转换成一个长整数型数
    if(inet_addr(ip) != -1) {
		sockaddr->sin_addr.s_addr = inet_addr(ip);
	} else {
		//  Invalid ftp server address
		if((ftp_server_hostent = gethostbyname(ip)) != 0)
			memcpy(&sockaddr->sin_addr, ftp_server_hostent->h_addr, sizeof(sockaddr->sin_addr));
		else return 253;
	}
    sockaddr->sin_port = htons(port);
	return 1;
}

int ftp_connect_server(struct sockaddr_in *s_addr, int type) {
	struct timeval outtime;
	// socket()函数用于根据指定的地址族、数据类型和协议来分配一个套接口的描述字(返回的就是该描述字/标识符)及其所用的资源。
	int s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0)
		cmd_run_err_exit("create socket error!", 249);

	//set outtime for the control socket
	if(1 == type) {
		outtime.tv_sec = 0;
		outtime.tv_usec = 300000;
	} else {
		outtime.tv_sec = 5;
		outtime.tv_usec = 0;
	}
	// 设置等待时间/延时，正常返回值为0
	// 在client连接服务器过程中，如果处于非阻塞模式下的socket在connect()的过程中可以设置connect()延时，直到accpet()被调用
	if(0 != setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &outtime, sizeof(outtime))) {
		printf("set socket %s errno:%d\n", strerror(errno), errno);
		cmd_run_err_exit("set socket", 1);
	}

	// connect to the server，注意执行完connect()后，s依然保存客户端的套接口标识，但对应的客户端套接口已经绑定连通了服务器端的某一套接口
	if (connect(s, (struct sockaddr *)s_addr, sizeof(struct sockaddr_in)) < 0) {
		printf("Can't connect to server %s, port %d\n", inet_ntoa(s_addr->sin_addr), ntohs(ftp_server_sockaddr.sin_port));
		// 没有成功连接到ftp服务器
		exit(252);
	}
	return s;
}

int ftp_send_cmd_toServer(const char *s1, const char *s2, int sockfd) {
	char send_buf[256];
	int send_err, len;
	if(s1) {
		strcpy(send_buf, s1);
		if(s2) {
			strcat(send_buf, s2);
			// 一定要在最后拼接上"\r\n"，否则FTP服务器不响应整条命令
			strcat(send_buf, "\r\n");
			len = strlen(send_buf);
			// 向服务器端发送操作命令
			send_err = send(sockfd, send_buf, len, 0);
		} else {
			// 整条命令字符串中的第二个命令字段为NULL
			strcat(send_buf, "\r\n");
			len = strlen(send_buf);
			send_err = send(sockfd, send_buf, len, 0);
		}
    }
	if(send_err < 0)
		printf("send() error!\n");
	return send_err;
}

int ftp_get_reply_fromServer(int sockfd) {
	// 循环次数清零
	rcv_count = 0;

	// reply_code是FTP服务器端的响应码
	int reply_code = 0, count=0;
	char reply_code_str[4];
	memset(rcv_buf, '\0', sizeof(rcv_buf));
	memset(reply_code_str, '\0', sizeof(reply_code_str));
	/* 
	 * atoi (表示 ascii to integer)是把字符串转换成整型数的一个函数，
	 * 如果实参不能转换成 int 或者 实参为空字符串，那么将返回 0。
	 */
	do {
		count = read(sockfd, rcv_buf, 512);
		strncpy(reply_code_str, rcv_buf, 3);
		reply_code = atoi(reply_code_str);
		if(count <= 0) {
			// 这里返回的是最后一次响应码！！！
			return reply_code;
		}
		rcv_count++;
		//printf("\033[32mFTP Server reply:");
		rcv_buf[count] = '\0';
		// rcv_buf中包含换行符
		//printf("\nBytes: %d | Replycode: %d\n", count, reply_code);
		/*
		if (count > 55) {
			printf("Content:\n%s\033[0m", rcv_buf);
		} else
			printf("Content: %s\033[0m", rcv_buf);
		*/
	} while (count > 0);
}

int ftp_get_dataPort_fromServer() {
	char port_respond[512], *buf_ptr;
	int count, port_num = 2080;
	memset(port_respond, '\0', sizeof(port_respond));
	// 全局变量sockfd是客户端的套接口描述字，其代表的套接口连通的是服务器端的命令端口对应的命令套接口
	ftp_send_cmd_toServer("PASV", NULL, sockfd);
	count = read(sockfd, port_respond, 512);
	if(count <= 0)
		return 0;
	// port_respond[count] = '\0';
	printf("dataPort: %s", port_respond);
	port_respond[3] = '\0';
	// FTP服务器端的响应码是227
	if(227 == atoi(port_respond)) {
		port_respond[3] = ' ';
		*strrchr(port_respond, ')') = '\0';
		/*
		 * strrchr()查找一个字符c在另一个字符串str中末次出现的位置（也就是从str的右侧开始查找字符c首次出现的位置），
		 * 并返回这个位置的地址。如果未能找到指定字符，那么函数将返回NULL。
		 * 使用这个地址返回从最后一个字符c到str末尾的字符串。
		 *
		 * strchr()在一个串中查找给定字符的第一个匹配之处。函数原型为：char *strchr(const char *str, int c)，
		 * 即在参数 str 所指向的字符串中搜索第一次出现字符 c（一个无符号字符）的位置。strchr函数包含在C 标准库 <string.h>中。
		 */
		// get low byte of the port  低8位
		buf_ptr = strrchr(port_respond, ',');
		port_num = atoi(buf_ptr + 1);
		*buf_ptr = '\0';
		// get high byte of the port  高8位
		buf_ptr = strrchr(port_respond, ',');
		port_num += atoi(buf_ptr + 1) * 256;
		printf("get dataPort: %d\n", port_num);
		return port_num;
	}
	return 0;
}

int ftp_get_dataPort_sockid() {
	if(mode)
	{
		int data_port = ftp_get_dataPort_fromServer();
		if(data_port != 0)
			ftp_server_sockaddr.sin_port=htons(data_port);
		return ftp_connect_server(&ftp_server_sockaddr, 0);
	}
}

void get_user() {
	char read_buf[64];
	do {
		printf("User: ");
		fgets(read_buf, sizeof(read_buf), stdin);
	} while ('\n' == read_buf[0]);
	strncpy(user, read_buf, strlen(read_buf) - 1);
}

void get_pass() {
	char read_buf[64];
	do {
		printf("Password: ");
		fgets(read_buf, sizeof(read_buf), stdin);
	} while ('\n' == read_buf[0]);
	strncpy(passwd, read_buf, strlen(read_buf) - 1);
	printf("\n");
}

int ftp_login() {
	int err;
	get_user();
	if(ftp_send_cmd_toServer("USER ", user, sockfd) < 0)
		cmd_run_err_exit("Can not send message", 1);
	err = ftp_get_reply_fromServer(sockfd);
	// 成功获取到用户名(服务器端要记录用户名，因为要给客户端发送匹配的密码)
	if(331 == err)
	{
		get_pass();
		if(ftp_send_cmd_toServer("PASS ", passwd, sockfd) < 0)
			cmd_run_err_exit("Can not send message", 1);
		else
			err = ftp_get_reply_fromServer(sockfd);
		// 成功获取密码，并成功登录FTP服务器
		if(230 == err)
			return 1;
		else if(531 == err)
			return 1;
		else {
			printf("Password error! err_code: %d\n", err);
			return 0;
		}
	} else {
		printf("User error!\n");
		return 0;
	}
}

void ftp_cmd_getFileOrDir_path(char *cmdStr, char *path) {
	int length = 0, i = 0;
	char *cmdSegment = strchr(cmdStr, ' ');
	// 清除一个完整用户命令中的某个命令段的段首空格
	while(NULL != cmdSegment && 0 != strlen(cmdSegment) && ' ' == *cmdSegment)
		cmdSegment++;
	if(NULL == cmdSegment || '\0' == *cmdSegment) {
		commandError();
		return;
	}
	length = strlen(cmdSegment);
	// 这里是去除命令之后的空格，上面是去除命令之前的空格！注意路径中一定不能有空格，否则会出现问题！！！
	for(; i < length && ' ' != *cmdSegment; ++i) {
		path[i] = cmdSegment[i];
	}
	path[i] = '\0';
}

void ftp_cmdHead() {
	ftp_pwd();
    char *head = strrchr(rcv_buf, '/') + 1;
    head[strlen(head) - 3] = '\0';
    strncpy(cmdHead, "[@ftp_cli ", 10);
    cmdHead[10] = '\0';
    strncat(cmdHead, head, strlen(head));
    cmdHead[11 + strlen(head)] = '\0';
    strncat(cmdHead, "]> \0", 4);
}

void ftp_list(char *cmdStr) {

	/*
	 * 注意当NULL==cmdStr时，实现"ls"，即列出当前目录的一级子目录和文件
	 *	   当NULL!=cmdStr时，实现 "ls 指定目录"，即列出指定目录的一级子目录和文件
	 */
	if (2 != strlen(cmdStr) && ' ' != cmdStr[2]) {
		// 防止出现 lsxxx，而非ls xxx或ls
		commandError();
		return;
	}

	char path[512];
	memset(path, '\0', sizeof(path));
	char *cmdSegment = strchr(cmdStr, ' ');
	int list_data_sock = ftp_get_dataPort_sockid();
	if(list_data_sock < 0) {
		ftp_get_reply_fromServer(sockfd);
		printf("create data sock error!\n");
		return;
	}
	while(NULL != cmdSegment && 0 != strlen(cmdSegment) && ' ' == *cmdSegment)
		cmdSegment++;

	if(NULL == cmdSegment || '\0' == *cmdSegment) {
		ftp_cmd_getFileOrDir_path("ls ./", path);
	} else {
		ftp_cmd_getFileOrDir_path(cmdStr, path);
	}
	ftp_send_cmd_toServer("LIST ", path, sockfd);
	// 执行下面的ftp_get_reply_fromServer()，服务器返回2条消息
	ftp_get_reply_fromServer(sockfd);
	// 执行下面的ftp_get_reply_fromServer()，服务器返回1条消息
	ftp_get_reply_fromServer(list_data_sock);
	if(mode && 1 != rcv_count){
		printf("LIST error or this is an empty directory!\n");
	}
	printf("%s", rcv_buf);
	close(list_data_sock);
}

void ftp_llist(char *cmdStr) {
	if (3 != strlen(cmdStr) && ' ' != cmdStr[3]) {
		// 防止出现 lsxxx，而非ls xxx或ls
		commandError();
		return;
	}

	char path[512];
	char *cmdSegment = strchr(cmdStr, ' ');
	while(NULL != cmdSegment && 0 != strlen(cmdSegment) && ' ' == *cmdSegment)
		cmdSegment++;

	memset(path, '\0', sizeof(path));
	strncpy(path, "ls -l ", 6);
	if(NULL == cmdSegment || '\0' == *cmdSegment) {
		ftp_cmd_getFileOrDir_path(localPath, path + 6);
	} else {
		ftp_cmd_getFileOrDir_path(cmdStr, path + 6);
	}
	system(path);
}

void ftp_size(char *cmdStr) {
	char path[512];
	ftp_cmd_getFileOrDir_path(cmdStr, path);
	ftp_send_cmd_toServer("SIZE ", path, sockfd);
	if(ftp_get_reply_fromServer(sockfd) != 213) {
		printf("SIZE error!\n");
		return;
	}
	printf("file %s size(bytes): %s", path, strrchr(rcv_buf, ' ') + 1);
}

void ftp_lsize(char *cmdStr) {
	char path[512];
	memset(path, '\0', sizeof(path));
	ftp_cmd_getFileOrDir_path(cmdStr, path);
    struct stat statbuff;
    if(stat(path, &statbuff) < 0){
		printf("SIZE error!\n");
        return;
    }
	printf("file %s size(bytes): %ld\n", path, statbuff.st_size);
}

void ftp_cd(char *cmdStr) {
	char *cmdSegment = strchr(cmdStr, ' ');
	char path[1024];
	// cd命令后面不能为空！cmdSegment定位到所有空格后面第一个字符
	while(NULL != cmdSegment && 0 != strlen(cmdSegment) && ' ' == *cmdSegment)
		cmdSegment++;
	if(NULL == cmdSegment || '\0' == *cmdSegment) {
		commandError();
		return;
	}

	strncpy(path, cmdSegment, strlen(cmdSegment));
	path[strlen(cmdSegment)] = '\0';
	int len = strlen(path);

	// 处理带"./"的字符串(去除"./")
	if (0 == strncmp(path, "./", 2)) {
		char *temp = strchr(path, '/') + 1;
		strncpy(path, temp, strlen(temp));
		len -=2;
		path[len] = '\0';
	}

	// printf("\033[31m%d\n\033[0m", limit_gotoUp);
	if (0 == strncmp(path, "../", 3)) {
		if (-1 == --limit_gotoUp) {
			limit_gotoUp = 0;
			printf("Unautorized access!\n");
			ftp_list("ls ./");
			return;
		}
	}

	// 前面几个字符既不是"./"也不是"../"
	// 下面去掉"a/b/"中的最后一个'/'
	if ('/' == path[len - 1]) {
		path[len - 1] = '\0';
		len--;
	}
	limit_gotoUp += 1 + countChar(path, '/');

	ftp_send_cmd_toServer("CWD ", path, sockfd);
	if (250 != ftp_get_reply_fromServer(sockfd)){
		limit_gotoUp--;
		printf("CWD error!\n");
		return;
	};
	ftp_list("ls");
}


void ftp_lcd(char *cmdStr) {
	char *cmdSegment = strchr(cmdStr, ' ');
	char path[1024];
	strncpy(path, "cd ", 3);

	// cd命令后面不能为空！cmdSegment定位到所有空格后面第一个字符
	while(NULL != cmdSegment && 0 != strlen(cmdSegment) && ' ' == *cmdSegment)
		cmdSegment++;
	// printf("%s\n", cmdSegment);
	if(NULL == cmdSegment || '\0' == *cmdSegment) {
		commandError();
		return;
	}

	strncpy(path + 3, cmdSegment, strlen(cmdSegment));
	path[strlen(cmdSegment) + 3] = '\0';

	if (system(path) < 0)
		printf("cd local: %s error!\n", path);
	else {
		char curr_dir[512];
		getcwd(curr_dir, sizeof(curr_dir));
		// 清除多层../嵌套
		while (0 == strncmp(cmdSegment, "../", 3)) {
			if (3 == strlen(cmdSegment)) {
				cmdSegment = "\0";
			} else {
				cmdSegment = strchr(cmdSegment, '/') + 1;
			}
			//*strrchr(curr_dir, '/') = '\0';
		}

		int len = 0;
		while('\0' != *(curr_dir + len)) {
			len++;
		}
		if (0 != strlen(cmdSegment)) {
			strncpy(curr_dir + len, "/\0", 2);
			len += 1;
		}

		strncpy(curr_dir + len, cmdSegment, strlen(cmdSegment));
		curr_dir[len + strlen(cmdSegment)] = '\0';
 		strncpy(localPath + 4, curr_dir, strlen(curr_dir));
		localPath[strlen(curr_dir) + 4] = '\0';
		printf("now is %s\n", strchr(localPath, ' '));
	}
	ftp_llist(localPath);
}

void ftp_pwd() {
	ftp_send_cmd_toServer("PWD", NULL, sockfd);
	if (ftp_get_reply_fromServer(sockfd) != 257) {
		printf("something error!\n");
		return;
	}
}

void ftp_lpwd() {
	char curr_dir[512];
	if (NULL == getcwd(curr_dir, sizeof(curr_dir)))
		printf("something wrong\n");
	else
		printf("Current directory(Local): %s\n", curr_dir);
}

void ftp_mkdir(char *cmdStr) {
	char *cmd = strchr(cmdStr, ' ');
	char path[1024];
	if (cmd == NULL) {
		printf("command error!\n");
		return;
	} else {
		while (*cmd == ' ')
			cmd++;
	}
	if (cmd == NULL || cmd == '\0') {
		printf("command error!\n");
		return;
	} else {
		char *head = " ./";
		strncpy(path, head, strlen(head));
		strcat(path, cmd);
		// strncpy(path,cmd,strlen(cmd));
		path[strlen(path)] = '\0';
		ftp_send_cmd_toServer("MKD", path, sockfd);
		ftp_get_reply_fromServer(sockfd);
		memset(path, 0, sizeof(path) / sizeof(char));
	}
}

void ftp_rmdir(char *cmdStr) {
	char *cmd = strchr(cmdStr, ' ');
	char path[1024];
	if (cmd == NULL) {
		printf("command error!\n");
		return;
	} else {
		while (*cmd == ' ')
			cmd++;
	}
	if (cmd == NULL || cmd == '\0') {
		printf("command error!\n");
		return;
	} else {
		char *head = " ./";
		strncpy(path, head, strlen(head));
		strcat(path, cmd);
		//strncpy(path,cmd,strlen(cmd));
		path[strlen(path)] = '\0';
		ftp_send_cmd_toServer("RMD", path, sockfd);
		ftp_get_reply_fromServer(sockfd);
		memset(path, 0, sizeof(path) / sizeof(char));
	}
}

void ftp_delete_file(char * cmdStr){
	char *cmd = strchr(cmdStr, ' ');
	char filename[1024];
	if (cmd == NULL) {
		printf("command error!\n");
		return;
	} else {
		while (*cmd == ' ')
			cmd++;
	}
	if (cmd == NULL || cmd == '\0') {
		printf("command error!\n");
		return;
	} else {
		strncpy(filename, cmd, strlen(cmd));
		//efilename[strlen(cmd)]='\0';
		ftp_send_cmd_toServer("DELE ", filename, sockfd);
		ftp_get_reply_fromServer(sockfd);
	}
}

void ftp_get(char *cmdStr) {
	// 与服务器数据端口对应的数据套接口连通的客户端套接口
	int get_sock;
	// path转存文件路径，src_file->服务器文件路径、local_file->本地文件路径，二者皆包含文件名
	char path[512];
	char src_file[512];
	char local_file[512];
	// 本地文件覆盖标志
	char cover_flag[3];
	// 保存本地文件属性的结构体
	struct stat file_info;
	// 本地新创建的文件的文件描述符
	int new_file;
	// 记录读取数据的个数
	int count = 0;
	// 从cmdStr得到文件名
	ftp_cmd_getFileOrDir_path(cmdStr, path);
	strcpy(src_file, path);
	strcpy(local_file, path);
	// 得到待下载文件的大小
	ftp_send_cmd_toServer("SIZE ", src_file, sockfd);
	if (ftp_get_reply_fromServer(sockfd) != 213) {
		printf("SIZE error!\n");
		return;
	}
	// stat函数得到local_file文件的属性写入到file_info中
	if (!stat(local_file, &file_info)) {
		// 如果本地存在相同命名的文件，询问是否覆盖，不覆盖则取消下载并退出
		printf("already exists %s (%d bytes) in local\n", local_file, (int)file_info.st_size);
		printf("Do you want to cover it? [y/n]");
		fgets(cover_flag, sizeof(cover_flag), stdin);
		fflush(stdin);
		if (cover_flag[0] != 'y') {
			printf("get file %s aborted.\n", src_file);
			return;
		}
	}
	// 创建并打开本地文件，O_TRUNC将文件大小置为0，属性不变
	new_file = open(local_file, O_CREAT | O_TRUNC | O_WRONLY);
	if (new_file < 0) {
		printf("creat local file %s error!\n", local_file);
		return;
	}
	// 从服务器得到数据传输端口
	get_sock = ftp_get_dataPort_sockid();
	if (get_sock < 0) {
		printf("socket error!\n");
		return;
	}

	// 告诉服务器数据传输类型为Binary
	ftp_send_cmd_toServer("TYPE I", NULL, sockfd);
	ftp_get_reply_fromServer(sockfd);
	// 从服务器取得(复制)文件
	ftp_send_cmd_toServer("RETR ", src_file, sockfd);
	ftp_get_reply_fromServer(sockfd);
	// 循环地从数据端口读取数据并写入到本地文件
	while (1) {
		count = read(get_sock, rcv_buf, sizeof(rcv_buf));
		if (count <= 0)
			break;
		else {
			write(new_file, rcv_buf, count);
		}
	}
	close(new_file);
	close(get_sock);
	ftp_get_reply_fromServer(sockfd);

	ftp_get_reply_fromServer(sockfd);
	// 更改文件权限(前提是Linux主机作为FTP客户端)
	if (!chmod(src_file, 0644)) {
		printf("chmod %s to 0644\n", local_file);
		return;
	} else
		printf("chmod %s to 0644 error!\n", local_file);
	ftp_get_reply_fromServer(sockfd);
	printf("%s download over,success!\n", path);
}

void ftp_put(char *cmdStr) {
	// char src_file[512];
	char send_buf[512];
	// path是要上传的文件路径(包含文件名)
	char path[512];
	// file_info保存文件属性
	struct stat file_info;
	int local_file;
	int file_put_sock, count = 0;
	// 根据输入的命令提取文件路径至path
	ftp_cmd_getFileOrDir_path(cmdStr, path);
	// ftp_cmd_filename(usr_cmd, src_file, dst_file);
	if (stat(path, &file_info) < 0) {
		printf("local file %s doesn't exist!\n", path);
		return;
	}
	// 以只读方式打开要上传的文件
	local_file = open(path, O_RDONLY);
	if (local_file < 0) {
		printf("open local file %s error!\n", path);
		return;
	}
	// 得到数据传输的socket
	file_put_sock = ftp_get_dataPort_sockid();
	if (file_put_sock < 0) {
		ftp_get_reply_fromServer(sockfd);
		printf("creat data sock errro!\n");
		return;
	}
	// 注意p2file决定了从客户端上传的文件保存到FTP服务器端的那个位置(目前不提供，只能传到服务器的当前目录下)以及文件名
	char *p2file;
	p2file = strrchr(path, '/');
	if (p2file) {
		p2file++;
	} else {
		p2file = path;
	}
	// 判断是否已经存在
	ftp_send_cmd_toServer("SIZE ", path, sockfd);
	int reply_code = 0;
	char reply_code_str[4];
	memset(reply_code_str, '\0', sizeof(reply_code_str));
	read(sockfd, reply_code_str, 3);
	reply_code = atoi(reply_code_str);
	if (213 == reply_code) {
		printf("A file with the same name already exists!\n");
		printf("Do you want cover it?[y/n]\n");
		char cover_flag[2];
		fgets(cover_flag, sizeof(cover_flag), stdin);
		fflush(stdin);
		if (cover_flag[0] != 'y') {
			printf("put file %s aborted.\n", path);
			// ftp_send_cmd_toServer("ABOR", NULL, sockfd);
			close(local_file);
			close(file_put_sock);
			ftp_get_reply_fromServer(sockfd);
			return;
		}
	}
	// 向服务器发送存储命令
	ftp_send_cmd_toServer("STOR ", p2file, sockfd);
	ftp_get_reply_fromServer(sockfd);
	// 告诉服务器以Binary类型进行数据传输
	ftp_send_cmd_toServer("TYPE I", NULL, sockfd);
	ftp_get_reply_fromServer(sockfd);

	// 循环读取本地文件，上传到服务器，每次最多512
	while (1) {
		count = read(local_file, send_buf, sizeof(send_buf));
		if (count <= 0)
			break;
		else {
			write(file_put_sock, send_buf, count);
		}
	}
	// 关闭
	close(local_file);
	close(file_put_sock);
	ftp_get_reply_fromServer(sockfd);
}

void ftp_close() {
	ftp_send_cmd_toServer("QUIT", NULL, sockfd);
	ftp_get_reply_fromServer(sockfd);
}

void ftp_quit() {
	ftp_send_cmd_toServer("QUIT", NULL, sockfd);
	ftp_get_reply_fromServer(sockfd);
	close(sockfd);
}

int ftp_cmdStrToId(char *cmdStr) {
	// 查看FTP客户端指定文件的大小命令：lsize xxx(文件)
	if(!strncmp(cmdStr, "lsize ", 6))
		return 18;
	// 显示FTP客户端的当前或指定目录的一级子目录和子文件命令：lls 或 lls xxx(目录)
	if (!strncmp(cmdStr, "lls", 3))
		return 17;
	// 查看客户端当前工作目录命令：lpwd
	if (!strncmp(cmdStr, "lpwd", 4))
		return 16;
	// 登录/连接FTP服务器命令：open xxx(ip)
	if(!strncmp(cmdStr, "open ", 5))
		return 15;
	// 注销/断开与FTP服务器的连接命令：close
	if(!strncmp(cmdStr, "close", 5))
		return 14;
	// 退出FTP客户端命令：quit/bye
	if(!strncmp(cmdStr, "quit", 4) || !strncmp(cmdStr, "bye", 3))
		return 13;
	// 显示FTP服务器端的当前或指定目录的一级子目录和子文件命令：ls 或 ls xxx(目录)
	if(!strncmp(cmdStr, "ls", 2))
		return 12;
	// 切换FTP服务器端的当前目录命令：cd xxx(目录)
	if(!strncmp(cmdStr, "cd ", 3))
		return 11;
	// 查看FTP服务器端指定文件的大小命令：sizeof xxx(文件)
	if(!strncmp(cmdStr, "size ", 5))
		return 10;
	// 上传FTP客户端文件命令：put xxx(文件/目录)
	if(!strncmp(cmdStr, "put ", 4))
		return 9;
	// 下载FTP服务器端文件命令：get xxx(文件/目录)
	if(!strncmp(cmdStr, "get ", 4))
		return 8;
	// 查看当前工作目录命令：pwd
	if (!strncmp(cmdStr, "pwd", 3))
		return 7;
	// 创建文件夹命令：mkdir xxx
	if (!strncmp(cmdStr, "mkdir ", 6))
		return 6;
	// 删除文件夹命令：rmdir xxx
	if (!strncmp(cmdStr, "rmdir ", 6))
		return 5;
	// 删除文件命令：del xxx
	if (!strncmp(cmdStr, "del ", 4))
		return 4;
	// 切换FTP客户端本地目录命令：lcd xxx
	if (!strncmp(cmdStr, "lcd ", 4))
		return 3;
	//显示帮助命令
	if (!strncmp(cmdStr, "help", 4))
		return 2;
	//显示版本信息命令
	if (!strncmp(cmdStr, "version", 7))
		return 1;
	if (!strncmp(cmdStr, "clear", 5)) {
		return 0;
	}
	return -1;
}

int build_connection(char *ip, int port) {
	int err;
	int cmdId;
	char cmdStr[1024];
	err = fill_host_sockaddr(ip, &ftp_server_sockaddr, port);
	if(254 == err)
		cmd_run_err_exit("Invalid port!", 254);
	if(253 == err)
		cmd_run_err_exit("Invalid server address!", 253);

	// 注意sockfd是与FTP服务器端的命令套接口相连通的客户端的相应套接口的描述字/标识符
	sockfd = ftp_connect_server(&ftp_server_sockaddr, 1);
	if((err =  ftp_get_reply_fromServer(sockfd)) != 220)
		cmd_run_err_exit("Connect error!", 220);
	do {
		err = ftp_login();
	} while(err != 1);

	printf("%s:%d ", ip, port);
    printf("Connect Successed!\n");
	ftp_list("ls");
	while(1) {
		ftp_cmdHead();
		printf(cmdHead);
		fgets(cmdStr, 512, stdin);
		fflush(stdin);
		if('\n' == cmdStr[0])
			continue;
		cmdStr[strlen(cmdStr) - 1] = '\0';
		cmdId = ftp_cmdStrToId(cmdStr);
		switch(cmdId) {
			case 18: {
				ftp_lsize(cmdStr);
			}
				break;
			case 17: {
				ftp_llist(cmdStr);
			}
				break;
			case 16: {
				ftp_lpwd();
			}
				break;
			case 15: {
				printf("已成功连接上FTP服务器，再次连接需要先断开！\n");
				continue;
			} break;
			case 14: {
				ftp_close();
				return ftp_openStart();
			} break;
			case 13: {
				ftp_quit();
				printf("Bye Bye!\n");
				exit(0);
			} break;
			case 12: {
				ftp_list(cmdStr);
			} break;
			case 11: {
				ftp_cd(cmdStr);
			} break;
			case 10: {
				ftp_size(cmdStr);
			} break;
			case 9: {
				ftp_put(cmdStr);
			} break;
			case 8: {
				ftp_get(cmdStr);
			} break;
			case 7: {
				ftp_pwd();
				printf("Current path: %s", strrchr(rcv_buf, ' ') + 1);
			} break;
			case 6: {
				ftp_mkdir(cmdStr);
			} break;
			case 5: {
				ftp_rmdir(cmdStr);
			} break;
			case 4: {
				ftp_delete_file(cmdStr);
			} break;
			case 3: {
				ftp_lcd(cmdStr);
			} break;
			case 2: {
				show_help();
			} break;
			case 1: {
				show_version();
			} break;
			case 0: {
				printf("%s","\033[1H\033[2J");
			} break;
			default:
				show_help();
				break;
			}
		memset(cmdStr, '\0',sizeof(cmdStr));
	}
	return 1;
}

int ftp_openStart() {
	// 初始化客户端路径
	memset(localPath, '\0', sizeof(localPath));
	strncpy(localPath, "lls ./", 6);

	// 存储用户输入的命令字符串
	char cmdStr[1024];
	memset(cmdStr, '\0', sizeof(cmdStr));
	// 用户输入的命令字符串对应的ID号
    int cmdId;
    while(1) {
		printf("[@ftp_cli]> ");
		fgets(cmdStr, 512, stdin);
        fflush(stdin);
        if('\n' == cmdStr[0])
			continue;
		// Linux中'\n' => '\0'
		cmdStr[strlen(cmdStr) - 1] = '\0';
        cmdId = ftp_cmdStrToId(cmdStr);
		switch(cmdId) {
			case 18: {
				ftp_lsize(cmdStr);
			}
				break;
			case 17: {
				ftp_llist(cmdStr);
			}
				break;
			case 16: {
				ftp_lpwd();
			}
				break;
			case 15: {
				// 在参数cmdStr所指向的字符串中搜索第一次出现空字符' '（一个无符号字符）的位置(/地址)，匹配不到则返回NULL
				char *cmdSegment = strchr(cmdStr,' ');
				// 注意未匹配到会返回NULL, 清除一条命令字符串中的相邻命令段之间的多余空格
				while(NULL != cmdSegment && 0 != strlen(cmdSegment) && ' ' == *cmdSegment)
					cmdSegment++;
				// 检测剩下的命令段是否全是空格
				if (NULL == cmdSegment || '\0' == *cmdSegment) {
					commandError();
					continue;
				}
				build_connection(cmdSegment, DEFAULT_FTP_PORT);
			} break;
			case 13: {
				printf("Bye Bye!\n");
				exit(0);
			} break;
			case 3: {
				ftp_lcd(cmdStr);
			} break;
			case 2: {
				show_help();
			} break;
			case 1: {
				show_version();
			} break;
			case 0: {
				printf("%s","\033[1H\033[2J");
			} break;
			default: {
				printf("\033[31mCommand error or Not logined in!\033[0m\n");
				show_help();
				continue;
			}
		}
    }
	return 0;
}

#endif //__FTP_H_
