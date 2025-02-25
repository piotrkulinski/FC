#pragma once
#include <string>
#include "stdafx.h"
#define SOCKET_READ_TIMEOUT_SEC       1

using std::string;

struct IPv4
{
    unsigned char b1, b2, b3, b4;
};

class TSocket
{
private:
	SOCKET sock;
	SOCKET broadcastsock;
	bool useWSAStartup;
	unsigned short port;
	unsigned int ipv4addr;
	bool server;
	struct sockaddr_in addr, srcaddr;
	CRITICAL_SECTION readsect;
	CRITICAL_SECTION writesect;

	TSocket(const TSocket&);
public:
	bool test;
	TSocket(void);
	int connect_it(const string host, unsigned short port);
	int connect_it(const int ipv4addr, unsigned short port);
	int connect_it_udp(const string host, unsigned short port);
	int listen(unsigned short port);
	TSocket* accept();
	int readbytes(char* dest, int len);
	int readbytes_timeout2(char* dest, int len);
	int readbytes_any(char* dest, int maxlen);
	int readbytes_timeout(char* dest, int len, int timeout);
	int writebytes(char* src, int len);
	int sendtosock(char* src, int len);
	int readfromsocket(char* dest, int len);
	bool is_connected(void);
	void disconnect(void);
	int try_reconnect();
	int discoverterminal(const int ports, char* src, int len, char* dst, int dlen);
	bool getMyIP(IPv4 & myIP);
	unsigned int get_ipv4addr(void) {
		return ipv4addr;
	}

	void assign_handle(SOCKET newhandle);

	~TSocket(void);
};

