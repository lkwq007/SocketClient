#pragma once
#ifndef PACKAGE_H
#define PACKAGE_H

// instructions
#define REQ_HOSTTIME 0x01
#define RES_HOSTTIME 0x02
#define REQ_HOSTNAME 0x03
#define RES_HOSTNAME 0x04
#define REQ_CLIENTLIST 0x05
#define RES_CLIENTLIST 0x06
#define SEND_MESSAGE 0x07
#define RECV_MESSAGE 0x08

// 实际发包为 pkg_header + 对应的字符串
typedef struct pkg_header_
{
	char type;
	long long length;
} pkg_header;

#endif // !PACKAGE_H