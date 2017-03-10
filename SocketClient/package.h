#pragma once
#ifndef PACKAGE_H_
#define PACKAGE_H_

// instructions
#define TYPE_REQ_TIME 0x01
#define TYPE_RES_TIME 0x02
#define TYPE_REQ_NAME 0x03
#define TYPE_RES_NAME 0x04
#define TYPE_REQ_CLIENTLIST 0x05
#define TYPE_RES_CLIENTLIST 0x06
#define TYPE_SEND_MESSAGE 0x07
#define TYPE_RECV_MESSAGE 0x08

// 实际发包为 pkg_header + 对应的字符串
typedef struct pkg_header_
{
	char type;
	long long length;
} pkg_header;

#endif // !PACKAGE_H_