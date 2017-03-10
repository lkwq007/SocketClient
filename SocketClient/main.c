#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <iup.h>
#include "package.h"
#include "thread_msg.h"

#pragma comment(lib,"ws2_32.lib")

//子线程 handle
HANDLE hEvent;

int server_port=8100;
char server_ip[51]="127.0.0.1";

int request_time();
int request_hostname();
int request_clientlist();
int send_message();
/*
DWORD WINAPI socket_thread(LPVOID pm)
{
	SOCKET s_client;//连接套接字
	struct sockaddr_in sa_server;//地址信息
	int ret,msg_ret,super_req=0;
	MSG msg;
	pkg_header header;
	char *header_ptr = (char *)&header;
	const int buffer_size = 10241;
	char storage[10241], *buffer;
	//创建socket，使用TCP协议
	s_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s_client == INVALID_SOCKET)
	{
		printf("socket() failed!\n");
		return 0;
	}

	//构建服务器地址信息
	sa_server.sin_family = AF_INET;//地址家族
	sa_server.sin_port = htons(server_port);//注意转化为网络字节序
	sa_server.sin_addr.S_un.S_addr = inet_addr(server_ip);
	
	//连接服务器
	ret = connect(s_client, (struct sockaddr *)&sa_server, sizeof(sa_server));
	if (ret == SOCKET_ERROR)
	{
		printf("connect() failed!\n");
		closesocket(s_client);//关闭套接字
		return 0;
	}

	while (1)
	{
		header.length = 0;
		super_req = 0;
		msg_ret = GetMessage(&msg, NULL, TMSG_REQ_TIME, TMSG_REQ_TIME100);
		if (msg_ret)
		{
			switch (msg.message)
			{
			case TMSG_REQ_TIME:
				header.type = REQ_HOSTTIME;
			case TMSG_REQ_TIME100:
				super_req = 1;
				break;
			case TMSG_REQ_NAME:
				header.type = REQ_HOSTNAME;
				break;
			case TMSG_REQ_LIST:
				header.type = REQ_CLIENTLIST;
				break;
			case TMSG_SEND_MSG:
				header.type = SEND_MESSAGE;
				header.length = strlen(message_wait_send);
				break;
			case TMSG_DISCONNECT:
				closesocket(s_client);
				return 0;
			default:
				;
			}
		}
	}
	
	closesocket(s_client);//关闭套接字
	return 0;
}

void test()
{
	


	
	

	


	while (1)
	{
		;
	}

	//按照预定协议，客户端将发送信息
	message.type = SEND_MSG;
	message.length = strlen(server_ip);
	ret = send(s_client, server_ip, sizeof(server_ip), 0);
	//ret = send(s_client, (char *)&message, sizeof(message), 0);
	if (ret == SOCKET_ERROR)
	{
		printf("send() failed!\n");
	}
	else
		printf("student info has been sent!\n");
	ret = send(s_client, server_ip, sizeof(server_ip), 0);
	
	return;
}
*/
int main(int argc, char *argv[])
{
	WORD w_version_requested;
	WSADATA wsa_data;
	int ret;
	HANDLE handle_socket;
	DWORD id_socket;

	//WinSock初始化
	w_version_requested = MAKEWORD(2, 2);//希望使用的WinSock DLL的版本
	ret = WSAStartup(w_version_requested, &wsa_data);
	if (ret != 0)
	{
		printf("WSAStartup() failed!\n");
		return -1;
	}

	//确认WinSock DLL支持版本2.2
	if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
	{
		WSACleanup();
		printf("Invalid Winsock version!\n");
		return -1;
	}
	/*
	if (1)
	{
		handle_socket = (HANDLE)_beginthreadex(NULL, 0, (LPTHREAD_START_ROUTINE)ThrdFunc, NULL, 0, &id_socket);
		if (!handle_socket)
		{
			printf("Error creating threads!\n");
			return -1;
		}

	}
	*/
	WSACleanup();
	printf("common: thread %d",GetCurrentThreadId());
	getchar();
	IupOpen(&argc, &argv);
	IupMessage("test", "test");
	IupClose();
	return 0;
}