#ifndef XSOCKET_H
#define XSOCKET_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#define TCP 1
#define UDP 2

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>

#pragma comment(lib,"ws2_32.lib")

typedef std::string   str;
typedef const char*   cstr;
typedef unsigned char byte;
typedef unsigned int  uint;


struct Message
{
	const str	data;			// Content of the message
	sockaddr_in address;		// Address of the recipient (only in datagrams)
};


class XSocket
{
public:
	uint  socketObj;			// Unsigned Integer memory address to the socket object
	void* connectThr;			// Handle to the thread responsible for establishing connection

	int  port;					// Port of the connection
	int  type;					// Protocol of the connection
	int  ctime;					// Maximum time limit to establish connection
	bool host;					// Flag of wether the socket is a server or not
	byte flag;					// Error flags of the connection keeping the socket safe
	str  addr;					// IPv4 Address of the connection

	WSAData	    wsaData;		// Windows networking information structure
	sockaddr_in addrInfo;		// Address information structure

	// Elements access to the class's services
	static friend DWORD WINAPI connectFn(LPVOID lpParameter);
	friend class IXSocket;

	// Private constructors
	XSocket();
	XSocket(const SOCKET socket, const sockaddr_in addrinf);

	// Private methods
	void Close();
	void Host(const int _type, const int _port);
	void Open(const int _type, const str _addr, const int _port, const int _ctime);
	void Send(const str data,  const int size,  const sockaddr_in address);
	void Send(const str data,  const int size);
	str  Recv(sockaddr_in* address, const int maxSize);
	XSocket* Accept();

};


class IXSocket
{
public:
	XSocket* xsocket;			// Constant pointer to the socket managed by the Interface

	// Privare constructors
	IXSocket(XSocket* socket);

public:
	// Public constructors
	IXSocket();
	IXSocket(const IXSocket& obj);

	// Public methods
	IXSocket Accept();
	void Host(const int _type, const int _port);
	void Open(const int _type, const str _addr, const int _port, const int _ctime);
	void Send(const str data,  const int size,  const sockaddr_in address);
	void Send(const str data,  const int size);
	str  Recv(sockaddr_in* address, const int maxSize);
	void Close();
	void Delete();
	bool good();

};

#endif