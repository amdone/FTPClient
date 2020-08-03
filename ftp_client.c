#include "ftp.h"

/**
 * FTP客户端功能：
 *	※必要功能：
 *		1）登录/连接FTP服务器
 *			即在FTP客户端输入账号+口令，连接FTP服务器进行验证，
 *			用以获取可以连接FTP服务器的用户相应的命令操作权限(/授权)。
 *		2）注销/断开与FTP服务器的连接
 *			仅断开与FTP服务器的连接，不退出FTP客户端
 *		3）退出FTP客户端
 *			如果不登录服务器，此时只能执行FTP客户端命令
 *		4）显示FTP客户端的当前或指定目录的一级子目录和子文件
 *		5）切换FTP服务器端的当前目录
 *		6）查看FTP服务器端指定文件的大小
 *		7）文件传输(并进行完整度检验)
 *			上传FTP客户端文件、下载FTP服务器端文件
 *  ※扩展功能(先实现并成功运行必要功能后，再添加！！！)：
 *		1）显示FTP客户端的当前或指定目录的一级子目录和子文件
 *		2）切换FTP客户端的当前目录
 *		3）查看FTP客户端指定文件的大小
 *		4）对FTP客户端和服务器端的目录和文件的增、删、重命名
 *		5）匿名登录服务器(此时只能执行客户端命令和下载文件命令)
 */
int main(int argc, char **argv) {
	// argc[0] = ./ftp_client
	switch(argc) {
		case 1: {
			printf("Hello, Welcom to use the FTP SoftWare!\n");
			ftp_openStart();
		} break;
		case 2: {
			// 可以直接跟着地址  ./ftp_client 127.0.0.1
			build_connection(argv[1], DEFAULT_FTP_PORT);
		} break;
		default: {
			printf("The input is invalid!\n");
			exit(-1);
		}
	}
	return 0;
}

