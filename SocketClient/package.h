#pragma once
#ifndef PACKAGE_H_
#define PACKAGE_H_

// instructions
#define TYPE_TIME 0x01
#define TYPE_NAME 0x02
#define TYPE_CLIENTLIST 0x03
#define TYPE_MESSAGE 0x04
#define TYPE_FEEDBACK 0x05

// ʵ�ʷ���Ϊ pkg_header + ��Ӧ���ַ���
typedef struct pkg_header_
{
	char type;
	__int64 length;
} pkg_header;

typedef struct msg_header_
{
	__int32 source;
	__int32 target;
} msg_header;

#endif // !PACKAGE_H_