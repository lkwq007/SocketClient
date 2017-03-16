#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <iup.h>
#include "package.h"
#include "thread_msg.h"

#pragma comment(lib,"ws2_32.lib")

//子线程 handle
HANDLE hEvent;

int server_port = 8100;
char server_ip[51] = "127.0.0.1";
SOCKET s_client;//连接套接字

DWORD WINAPI socket_send(LPVOID pm)
{
	MSG msg;
	BOOL msg_ret;
	int ret, type;
	pkg_header header;
	char *header_ptr = (char *)&header;
	char buf[1001], *buf_ptr;
	while (1)
	{
		msg_ret = GetMessage(&msg, NULL, TMSG_REQ_TIME, TMSG_REQ_TIME100);
		if (msg_ret)
		{
			switch (msg.message)
			{
			case TMSG_REQ_TIME:
				header.type = TYPE_REQ_TIME + 'a';
				header.length = 0;
				break;
			case TMSG_REQ_NAME:
				header.type = TYPE_REQ_NAME + 'a';
				header.length = 0;
				break;
			case TMSG_REQ_LIST:
				header.type = TYPE_REQ_CLIENTLIST + 'a';
				header.length = 0;
				break;
			case TMSG_REQ_SEND_MSG:
				header.type = TYPE_SEND_MESSAGE + 'a';
				header.length = strlen((char*)msg.wParam) + 1;
				break;
			case TMSG_DISCONNECT:
				closesocket(s_client);
				return 0;
			}
			ret = send(s_client, (char *)&header, sizeof(header), 0);
			if (ret == SOCKET_ERROR)
			{
				printf("send() failed!\n");
			}
			else
				printf("send!");
			if (header.length > 0)
			{
				ret = send(s_client, (char*)msg.wParam, (header.length) * sizeof(char), 0);
				if (ret == SOCKET_ERROR)
				{
					printf("send() failed!\n");
				}
				else
					printf("msg send!");
			}
		}
	}
	return 0;
}

DWORD WINAPI socket_recv(LPVOID pm)
{
	int ret;
	pkg_header header;
	char *header_ptr = (char *)&header;
	char buf[1001], *buf_ptr;
	int stream_left;
	while (1)
	{
		stream_left = sizeof(pkg_header);
		header_ptr = (char *)&header;
		while (stream_left > 0)
		{
			ret = recv(s_client, header_ptr, stream_left, 0);
			if (ret == SOCKET_ERROR)
			{
				printf("recv() fail");
				return 0;
				break;
			}
			if (ret == 0)
			{
				printf("connection closed!");
			}
			stream_left -= ret;
			header_ptr += ret;
		}
		if (stream_left == 0)
		{
			printf("%c l:%d", header.type, header.length);
		}
		if (header.length > 0)
		{
			buf_ptr = buf;
			stream_left = header.length;
			while (stream_left > 0)
			{
				ret = recv(s_client, buf_ptr, stream_left, 0);
				if (ret == SOCKET_ERROR)
				{
					printf("recv() fail");
					printf("%d", WSAGetLastError());
					return 0;
					break;
				}
				if (ret == 0)
				{
					printf("connection closed!");
				}
				stream_left -= ret;
				buf_ptr += ret;
			}
			if (stream_left == 0)
			{
				printf(" %s ", buf);
			}
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	WORD w_version_requested;
	WSADATA wsa_data;
	int ret;
	HANDLE handle_send, handle_recv, handle_socket[2];
	DWORD id_send,id_recv;
	int type;
	char buf[1001];

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


	struct sockaddr_in sa_server;//地址信息
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

	handle_socket[0] = (HANDLE)_beginthreadex(NULL, 0, (LPTHREAD_START_ROUTINE)socket_send, NULL, 0, &id_send);
	handle_socket[1] = (HANDLE)_beginthreadex(NULL, 0, (LPTHREAD_START_ROUTINE)socket_recv, NULL, 0, &id_recv);
	if (!handle_socket[0] || !handle_socket[1])
	{
		printf("Error creating threads!\n");
		return -1;
	}
	
	while (1)
	{
		scanf("%d", &type);
		switch (type)
		{
		case TYPE_REQ_TIME:
			PostThreadMessage(id_send, TMSG_REQ_TIME, 0, 0);
			break;
		case TYPE_REQ_NAME:
			PostThreadMessage(id_send, TMSG_REQ_NAME, 0, 0);
			break;
		case TYPE_REQ_CLIENTLIST:
			PostThreadMessage(id_send, TMSG_REQ_LIST, 0, 0);
			break;
		case TYPE_SEND_MESSAGE:
			scanf("%s", buf);
			PostThreadMessage(id_send, TMSG_REQ_SEND_MSG, (WPARAM)buf, 0);
			break;
		default:
			PostThreadMessage(id_send, TMSG_DISCONNECT, 0, 0);
			goto END_PROG;
		}
	}
END_PROG:
	WaitForMultipleObjects(2, handle_socket, TRUE, INFINITE);
	WSACleanup();
	printf("common: thread %d", GetCurrentThreadId());
	getchar();
}