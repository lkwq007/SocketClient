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

DWORD id_main;
HWND hwnd_main;
HANDLE handle_send, handle_recv, handle_socket[2];
DWORD id_send, id_recv;
int server_port = 8100;
char server_ip[51] = "127.0.0.1";
SOCKET s_client;//连接套接字
struct sockaddr_in sa_server;//地址信息
int send_x100 = 0, connect_state = 0;

Ihandle *dlg, *vbox, *hbox;
Ihandle *text_log, *text_ip, *text_port, *text_msg;
Ihandle *toggle;
Ihandle *menu, *item_connect, *item_time, *item_name, *item_list, *item_msg, *item_disconnect, *item_exit;

int log_writeln(char *str)
{
	IupSetAttribute(text_log, "APPENDNEWLINE", "YES");
	IupSetAttribute(text_log, "APPEND", str);
	return 0;
}

int log_write(char *str)
{
	IupSetAttribute(text_log, "APPENDNEWLINE", "NO");
	IupSetAttribute(text_log, "APPEND", str);
}

DWORD WINAPI socket_send(LPVOID pm)
{
	MSG msg;
	BOOL msg_ret;
	int ret, type;
	pkg_header header;
	char *header_ptr = (char *)&header;
	char buf[1001], *buf_ptr;
	int i,total;
	while (1)
	{
		msg_ret = GetMessage(&msg, NULL, TMSG_REQ_TIME, TMSG_REQ_TIME100);
		if (msg_ret)
		{
			switch (msg.message)
			{
			case TMSG_REQ_TIME:
				header.type = TYPE_REQ_TIME;
				header.length = 0;
				break;
			case TMSG_REQ_NAME:
				header.type = TYPE_REQ_NAME;
				header.length = 0;
				break;
			case TMSG_REQ_LIST:
				header.type = TYPE_REQ_CLIENTLIST;
				header.length = 0;
				break;
			case TMSG_REQ_SEND_MSG:
				header.type = TYPE_SEND_MESSAGE;
				header.length = strlen((char*)msg.wParam) + 1;
				break;
			case TMSG_DISCONNECT:
				shutdown(s_client, SD_SEND);
				//closesocket(s_client);
				return 0;
			}
			if (send_x100)
			{
				total = 100;
			}
			else
			{
				total = 1;
			}
				for (i = 0; i < total; i++)
				{
					ret = send(s_client, (char *)&header, sizeof(header), 0);
					if (ret == SOCKET_ERROR)
					{
						log_writeln("Send header fail");
						sprintf(buf, "WSA Error: %d", WSAGetLastError());
						log_writeln(buf);
						//return 0;
						//printf("send() failed!\n");
					}
					else
					{
						printf("send!\n");
					}
					if (header.length > 0)
					{
						ret = send(s_client, (char*)msg.wParam, (header.length) * sizeof(char), 0);
						if (ret == SOCKET_ERROR)
						{
							log_writeln("Send body fail");
							sprintf(buf, "WSA Error: %d", WSAGetLastError());
							log_writeln(buf);
							//return 0;
							//printf("send() failed!\n");
						}
						else
						{
							printf("send!\n");
						}
					}
				}
			
			free((char*)msg.wParam);
		}
	}
	return 0;
}

DWORD WINAPI socket_recv(LPVOID pm)
{
	int ret;
	pkg_header header;
	char *header_ptr = (char *)&header;
	char *buf,*buf_ptr;
	char info[101];
	//char buf[1001], *buf_ptr;
	int stream_left;
	while (1)
	{
		if(!connect_state)
		{
			return 0;
		}
		stream_left = sizeof(pkg_header);
		header_ptr = (char *)&header;
		while (stream_left > 0)
		{
			ret = recv(s_client, header_ptr, stream_left, 0);
			if (ret == SOCKET_ERROR)
			{
				log_writeln("Recv header fail");
				sprintf(info, "WSA Error: %d", WSAGetLastError());
				log_writeln(info);
				return 0;
			}
			if (ret == 0)
			{
				log_writeln("Connection closed!");
				return 0;
			}
			stream_left -= ret;
			header_ptr += ret;
		}
		if (stream_left == 0)
		{
			printf(info,"type:%d len:%d ", header.type, header.length);
		}
		if (header.length > 0)
		{
			buf = (char *)malloc(header.length * sizeof(char));
			buf_ptr = buf;
			stream_left = header.length;
			while (stream_left > 0)
			{
				ret = recv(s_client, buf_ptr, stream_left, 0);
				if (ret == SOCKET_ERROR)
				{
					log_writeln("Recv body fail");
					sprintf(info,"WSA Error: %d", WSAGetLastError());
					log_writeln(info);
					return 0;
				}
				if (ret == 0)
				{
					log_writeln("Connection closed!");
					return 0;
				}
				stream_left -= ret;
				buf_ptr += ret;
			}
			if (stream_left == 0)
			{
				switch (header.type)
				{
				case TYPE_RES_TIME:
					log_writeln("Recv time: ");
					log_write(buf);
					//PostThreadMessage(id_main, TMSG_RECV_TIME, (WPARAM)buf, 0);
					break;
				case TYPE_RES_NAME:
					log_writeln("Recv name: ");
					log_write(buf);
					//PostThreadMessage(id_main, TMSG_RECV_NAME, (WPARAM)buf, 0);
					break;
				case TYPE_RES_CLIENTLIST:
					log_writeln("Recv list: ");
					log_writeln(buf);
					//PostThreadMessage(id_main, TMSG_RECV_LIST, (WPARAM)buf, 0);
					break;
				case TYPE_RECV_MESSAGE:
					log_writeln("Recv msg: ");
					log_writeln(buf);
					//PostThreadMessage(id_main, TMSG_RECV_MSG, (WPARAM)buf, 0);
					break;
				default:
					log_writeln("Recv msg: ");
					log_writeln(buf);
					//PostMessage(hwnd_main, TMSG_RECV_MSG, (WPARAM)buf, 0);
					//printf("%d",PostThreadMessage(id_main, TMSG_RECV_MSG, (WPARAM)buf, 0));
					break;
				}
				//printf("data: %s ", buf);
			}
		}
	}
	return 0;
}

int exit_cb(void)
{
	if (connect_state)
	{
		shutdown(s_client, SD_BOTH);
		closesocket(s_client);
	}
	return IUP_CLOSE;
}

int change_x100(void)
{
	send_x100 = IupGetInt(toggle, "VALUE");
	//printf("%d", send_x100);
	return IUP_DEFAULT;
}

static int idle(void)
{
	MSG msg;
	if (PeekMessage(&msg, NULL, TMSG_RECV_TIME, TMSG_REQ_TIME100, PM_REMOVE))
	{
		switch (msg.message)
		{
		case TMSG_RECV_TIME:
			log_writeln("Recv time: ");
			log_write((char *)msg.wParam);
			break;
		case TMSG_RECV_NAME:
			log_writeln("Recv name: ");
			log_write((char *)msg.wParam);
			break;
		case TMSG_RECV_LIST:
			log_writeln("Recv list: ");
			log_writeln((char *)msg.wParam);
			break;
		case TMSG_RECV_MSG:
			log_writeln("Recv msg: ");
			log_writeln((char *)msg.wParam);
			break;
		case TMSG_RECV_MSG_FEEDBACK:
			log_writeln("Msg sent feedback: ");
			log_write((char *)msg.wParam);
			break;
		case TMSG_SEND_FAIL:
			log_writeln("Send fail!");
			break;
		}
		free((char *)msg.wParam);
	}
	//printf("idle");
	return IUP_DEFAULT;
}

int iup_connect(void)
{
	char buf[101],*str;
	int ret;
	if (connect_state)
	{
		log_writeln("Disconnect!");
		//closesocket(s_client);
		PostThreadMessage(id_send, TMSG_DISCONNECT, 0, 0);
		IupSetAttribute(item_connect, "TITLE", "Connect");
		IupSetFunction("IDLE_ACTION", NULL);
		connect_state = 0;
	}
	else
	{
		str = IupGetAttribute(text_ip, "VALUE");
		strcpy(server_ip, str);
		server_port = IupGetInt(text_port, "VALUE");
		sprintf(buf, "Connecting %s:%d ...", server_ip, server_port);
		log_writeln(buf);
		//构建服务器地址信息
		sa_server.sin_family = AF_INET;//地址家族
		sa_server.sin_port = htons(server_port);//注意转化为网络字节序
		sa_server.sin_addr.S_un.S_addr = inet_addr(server_ip);

		s_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s_client == INVALID_SOCKET)
		{
			printf("socket() failed!\n");
			return 0;
		}
		//连接服务器
		ret = connect(s_client, (struct sockaddr *)&sa_server, sizeof(sa_server));
		if (ret == SOCKET_ERROR)
		{
			log_writeln("Connection failed!");
			printf("%d", WSAGetLastError());
			closesocket(s_client);//关闭套接字
			IupSetFunction("IDLE_ACTION", NULL);
			connect_state = 0;
		}
		else
		{
			connect_state = 1;
			handle_socket[0] = (HANDLE)_beginthreadex(NULL, 0, (LPTHREAD_START_ROUTINE)socket_send, NULL, 0, &id_send);
			handle_socket[1] = (HANDLE)_beginthreadex(NULL, 0, (LPTHREAD_START_ROUTINE)socket_recv, NULL, 0, &id_recv);
			if (!handle_socket[0] || !handle_socket[1])
			{
				log_writeln("Error creating threads!");
				connect_state = 0;
			}
			else
			{
				IupSetAttribute(item_connect, "TITLE", "Disconnect");
				//connect_state = 1;
				log_writeln("Connection established!");
				//IupSetFunction("IDLE_ACTION", (Icallback)idle);
			}
		}
	}
	return IUP_DEFAULT;
}

int iup_time(void)
{
	if (connect_state)
	{
		log_writeln("Requesting time...");
		PostThreadMessage(id_send, TMSG_REQ_TIME, 0, 0);
	}
	else
	{
		log_writeln("Error not connected!");
	}
	return IUP_DEFAULT;
}

int iup_name(void)
{
	if (connect_state)
	{
		log_writeln("Requesting name...");
		PostThreadMessage(id_send, TMSG_REQ_NAME, 0, 0);
	}
	else
	{
		log_writeln("Error not connected!");
	}
	return IUP_DEFAULT;
}

int iup_list(void)
{
	if (connect_state)
	{
		log_writeln("Requesting list...");
		PostThreadMessage(id_send, TMSG_REQ_LIST, 0, 0);
	}
	else
	{
		log_writeln("Error not connected!");
	}
	return IUP_DEFAULT;
}

int iup_msg(void)
{
	char *new_msg;
	int count = IupGetInt(text_msg, "COUNT");
	char *str = IupGetAttribute(text_msg, "VALUE");
	new_msg = (char *)malloc((count + 1) * sizeof(char));
	strcpy(new_msg, str);
	if (connect_state)
	{
		log_writeln("Sending msg...");
		PostThreadMessage(id_send, TMSG_REQ_SEND_MSG, (WPARAM)new_msg, 0);
	}
	else
	{
		log_writeln("Error not connected!");
	}
	return IUP_DEFAULT;
}

int main(int argc, char *argv[])
{
	WORD w_version_requested;
	WSADATA wsa_data;
	int ret;
	int type;
	char buf[1001];

	id_main = GetCurrentThreadId();

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

	// scanf("%s %d", server_ip, &server_port);

	// using IUP
	IupOpen(&argc, &argv);
	// x100 toggle
	toggle = IupToggle("Send x100", NULL);
	//IupSetAttribute(toggle, "RIGHTBUTTON", "YES");
	IupSetCallback(toggle, "ACTION", change_x100);
	// log
	text_log = IupText(NULL);
	IupSetAttribute(text_log, "MULTILINE", "YES");
	IupSetAttribute(text_log, "EXPAND", "YES");
	IupSetAttribute(text_log, "READONLY", "YES");
	// port
	text_ip = IupText(NULL);
	IupSetAttribute(text_ip, "VISIBLECOLUMNS", "18");
	IupSetAttribute(text_ip, "VALUE", "127.0.0.1");
	// port
	text_port = IupText(NULL);
	IupSetAttribute(text_port, "VISIBLECOLUMNS", "8");
	IupSetAttribute(text_port, "VALUE", "8100");
	// msg
	text_msg = IupText(NULL);
	IupSetAttribute(text_msg, "EXPAND", "HORIZONTAL");
	IupSetAttribute(text_msg, "CUEBANNER", "Write your message here");
	// menu
	item_connect = IupItem("Connect", NULL);
	IupSetAttribute(item_connect, "AUTOTOGGLE", "YES");
	IupSetCallback(item_connect, "ACTION", iup_connect);

	item_time = IupItem("GetTime", NULL);
	IupSetCallback(item_time, "ACTION", iup_time);

	item_name = IupItem("GetName", NULL);
	IupSetCallback(item_name, "ACTION", iup_name);

	item_list = IupItem("GetList", NULL);
	IupSetCallback(item_list, "ACTION", iup_list);

	item_msg = IupItem("SendMsg", NULL);
	IupSetCallback(item_msg, "ACTION", iup_msg);
	item_disconnect = IupItem("Disconnect", NULL);

	item_exit = IupItem("Exit", NULL);
	IupSetCallback(item_exit, "ACTION", (Icallback)exit_cb);

	menu = IupMenu(item_connect, item_time, item_name, item_list, item_msg, item_exit, NULL);

	vbox = IupVbox(
		IupHbox(IupLabel("IP: "), text_ip, IupLabel("Port: "), text_port, toggle, NULL),
		IupHbox(IupLabel("Message: "), text_msg, NULL),
		IupVbox(IupLabel("Log: "), text_log, NULL),
		NULL);
	IupSetAttribute(vbox, "GAP", "3");
	IupSetAttribute(vbox, "MARGIN", "5");

	dlg = IupDialog(vbox);

	IupSetAttributeHandle(dlg, "MENU", menu);
	IupSetAttribute(dlg, "TITLE", "Socket Client");
	IupSetAttribute(dlg, "SIZE", "HALFxHALF");
	hwnd_main = IupGetAttribute(dlg, "HWND");

	IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
	IupSetAttribute(dlg, "USERSIZE", NULL);

	IupMainLoop();
	IupClose();

	WaitForMultipleObjects(2, handle_socket, TRUE, INFINITE);
	WSACleanup();
	printf("common: thread %d", GetCurrentThreadId());
	getchar();
}