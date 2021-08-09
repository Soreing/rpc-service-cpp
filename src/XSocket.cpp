#include <rpc-service/XSocket.h>
#include <time.h>


// Attempts to connect the socket to the specified address
// Failing to connect sets error flags and quits the thread
// Connection is re-tried if times out or refused untill time threshold reached
static DWORD WINAPI connectFn(LPVOID lpParameter)
{
	XSocket* const XS = (XSocket*)lpParameter;
	long long startTime = time(NULL);

	while ((XS->flag & 0x0F) == 12 && time(NULL) - startTime < XS->ctime)
	{
		if (0 == connect(XS->socketObj, (struct sockaddr *) &XS->addrInfo, sizeof(XS->addrInfo)))
		{	XS->flag &= 0xFB;
			break;
		}

		int error = WSAGetLastError();
		if (error != WSAETIMEDOUT && error != WSAECONNREFUSED)
		{	XS->flag |= 0x02;
		}
	}

	XS->connectThr = NULL;
	return 0;
}


// Craetes an empty socket with placeholder values
XSocket::XSocket()
{
	this->connectThr = NULL;
	this->socketObj  = INVALID_SOCKET;
	
	this->flag  = 0xFF;
	this->addr  = new char[20];
	this->host = false;
	this->port  = 0;
	this->type  = 0;
	this->ctime = 0;
}


// Creates a socket from a connected socket and an address
// Extracts IP and Port from the address structure
// Starts receiving from the socket in a different thread
XSocket::XSocket(const SOCKET socket, const sockaddr_in addrinf)
{
	this->connectThr = NULL;
	this->socketObj  = socket;
	this->addrInfo   = addrinf;
	
	this->flag  = 0xF8;
	this->addr  = new char[20];
	this->host = false;
	this->type  = TCP;
	this->ctime = 0;

	auto ip = addrInfo.sin_addr.S_un.S_un_b;

	char* cstring = new char[4];

	this->addr  = _itoa(ip.s_b1, cstring, 10);
	this->addr += ".";
	this->addr +=_itoa(ip.s_b2, cstring, 10);
	this->addr += ".";
	this->addr += _itoa(ip.s_b3, cstring, 10);
	this->addr += ".";
	this->addr += _itoa(ip.s_b4, cstring, 10);
	this->port  = addrInfo.sin_port;

	delete[] cstring;
}


// Creates a new interface managing a new socket
IXSocket::IXSocket() : xsocket(new XSocket()) { }

// Creates a new interface managing a specific socket
IXSocket::IXSocket(XSocket* socket) : xsocket(socket) {  }

// Creates a copy of the interface specified
IXSocket::IXSocket(const IXSocket& obj) : xsocket(obj.xsocket) { }

#pragma endregion


#pragma region Methods

//Constructs the socket with the given parameters and opens the connection.
//If the process encounters an error, error flags are set, and the process halts.
//Successful connection leaves all 3 flags at 0, meaning flag|0x07 is 0.
void XSocket::Open(const int _type, const str _addr, const int _port, const int _ctime = 1)
{
	Close();

	this->addr = _addr;
	this->port = _port;
	this->type = _type;
	this->ctime = _ctime;
	this->host = false;
	this->flag |= 0x0E;

	if ((flag & 0x01) == 1 && 0 == WSAStartup(MAKEWORD(2, 2), &wsaData))
	{	flag &= 0xFE;
	}

	auto inf_socket   = type == TCP ? SOCK_STREAM : SOCK_DGRAM;
	auto inf_protocol = type == TCP ? IPPROTO_TCP : IPPROTO_UDP;
	ZeroMemory(&addrInfo, sizeof(addrInfo));

	if ((flag & 0x03) == 2 && INVALID_SOCKET != (socketObj = socket(AF_INET, inf_socket, inf_protocol)))
	{
		flag &= 0xFD;
		addrInfo.sin_family = AF_INET;
		addrInfo.sin_port = htons(port);
		addrInfo.sin_addr.S_un.S_addr = inet_addr(addr.data());
	}

	if ((flag & 0x0F) == 12 && type == TCP)
	{	connectThr = CreateThread(0, 0, connectFn, (LPDWORD)this, 0, NULL);
		for (long long startTime = time(NULL); (flag & 0x0F) == 12 && time(NULL) - startTime < ctime;) {}
	}
}



//Constructs the socket with the given parameters and opens the connection.
//If the process encounters an error, error flags are set, and the process halts.
//Successful connection leaves all 3 flags at 0, meaning flag|0x07 is 0.
void XSocket::Host(int _type, int _port)
{
	Close();

	this->addr = "0.0.0.0";
	this->port = _port;
	this->type = _type;
	this->ctime = 0;
	this->host = true;
	this->flag |= 0x0E;

	if ((flag & 0x01) == 1 && 0 == WSAStartup(MAKEWORD(2, 2), &wsaData))
		flag &= 0xFE;

	int addrlen = sizeof(addrInfo);
	auto inf_socket   = type == TCP ? SOCK_STREAM : SOCK_DGRAM;
	auto inf_protocol = type == TCP ? IPPROTO_TCP : IPPROTO_UDP;
	ZeroMemory(&addrInfo, sizeof(addrInfo));

	if ((flag & 0x03) == 2 && INVALID_SOCKET != (socketObj = socket(AF_INET, inf_socket, inf_protocol)))
	{
		flag &= 0xFD;
		addrInfo.sin_family = AF_INET;
		addrInfo.sin_port = htons(port);
		addrInfo.sin_addr.S_un.S_addr = inet_addr(addr.data());
	}

	if ((flag & 0x0F) == 12 && type == TCP)
	{	if (0 == bind(socketObj, (struct sockaddr *) &addrInfo, addrlen))
			if (0 == listen(socketObj, 10))
				flag &= 0xF7;
	}		
	else if ((flag & 0x0F) == 12 && type == UDP)
	{	if (0 == bind(socketObj, (struct sockaddr *) &addrInfo, addrlen))
			flag &= 0xF3;
	}
}


//If the connection is successfully established, sends a C-String with a size to the receiver.
//If an error occurs, it raises the connection and the socket error flags.
void XSocket::Send(const str data, const int size)
{
	int addrlen = sizeof(addrInfo);
	int sent = 0;

	if (type == UDP && (flag & 0x03) == 0 && data != "")
	{
		sent = sendto(socketObj, data.data(), size, 0, (struct sockaddr *) &addrInfo, addrlen);
		if (sent > 0)
			flag &= 0xFB;
	}

	if (type == TCP && (flag & 0x07) == 0 && data != "")
		sent = send(socketObj, data.data(), size, 0);

	if (sent < 1)
		flag |= 0x06;
}


//If the connection is successfully established, sends a C-String with a size to the receiver.
//If an error occurs, it raises the connection and the socket error flags.
void XSocket::Send(const str data, const int size, const sockaddr_in address)
{
	int addrlen = sizeof(address);
	int sent = 0;

	if (type == UDP && (flag & 0x03) == 0 && data != "")
	{
		sent = sendto(socketObj, data.data(), size, 0, (struct sockaddr *) &address, addrlen);
		if (sent > 0)
			flag &= 0xFB;
	}

	if (sent < 1)
		flag |= 0x06;
}


// Blocks the thread untill there is a message in the queue and returns it
// Sets the address when using datagrams
// Returns empty string on failure
str XSocket::Recv(sockaddr_in* address, const int maxSize = 256)
{
	sockaddr_in addr = sockaddr_in{};
	int addrlen = sizeof(addr);
	str result  = "";

	int received = 0;
	char* buffer = new char[maxSize];

	if (type == UDP || (type == UDP && host))
	{
		received = recvfrom(socketObj, buffer, maxSize, NULL, (struct sockaddr*) (address != NULL ? address : NULL), (address != NULL ? &addrlen : NULL));
		if (received > 0)
			result = std::string(buffer, received);
	}
	else if (type == TCP)
	{
		received = recv(socketObj, buffer, maxSize, 0);
		if (received > 0)
			result = std::string(buffer, received);
	}

	if (received <= 0)
	{	flag |= 0x06;
	}

	delete[] buffer;
	return result;
}


// Accepts a new connection using the hosting socket
// If the connection fails, returns an empty socket
XSocket* XSocket::Accept()
{
	SOCKET s = INVALID_SOCKET;
	sockaddr_in clInfo;
	int addrlen = sizeof(clInfo);

	if (type != TCP || (flag & 0x0F) != 4)
	{	return new XSocket();
	}

	s = accept(socketObj, (struct sockaddr *)&clInfo, &addrlen);

	if (s != INVALID_SOCKET)
	{	return new XSocket(s, clInfo);
	}

	return new XSocket();
}


// Opens a new connection with the address specified
// Only applicable to interfaces that manage a socket
void IXSocket::Open(int _type, const str _addr, int _port, int _ctime)
{
	if (xsocket != NULL)
	{	xsocket->Open(_type, _addr, _port, _ctime);
	}
}


// Hosts a new server using the currently managed socket
// Only applicable to interfaces that manage a socket
void IXSocket::Host(int _type, int _port)
{
	if (xsocket != NULL)
	{	xsocket->Host(_type, _port);
	}
}


// Listens to a new connection and returns a new interface managing a socket
// On failure, the managed socket points to NULL
// Only applicable to TCP sockets
IXSocket IXSocket::Accept()
{
	if (xsocket != NULL)
	{	XSocket* newSocket = xsocket->Accept();
		return IXSocket(newSocket);
	}

	return IXSocket(new XSocket());
}


void IXSocket::Send(const str data, const int size, const sockaddr_in address)
{
	if (xsocket != NULL)
		xsocket->Send(data, size, address);
}

void IXSocket::Send(const str data, const int size)
{
	if (xsocket != NULL)
		xsocket->Send(data, size);
}

str IXSocket::Recv(sockaddr_in* address, const int size = 256)
{
	if (xsocket != NULL)
		return xsocket->Recv(address, size);
	return "";
}


// Returns true if the managed socket is working as intended
// Interfaces returning false must be discarded
bool IXSocket::good()
{
	if (xsocket != NULL)
	{
		if      (xsocket->type == TCP &&  xsocket->host && (xsocket->flag & 0x0F) == 4) return true;
		else if (xsocket->type == TCP && !xsocket->host && (xsocket->flag & 0x0F) == 8) return true;
		else if (xsocket->type == UDP &&  xsocket->host && (xsocket->flag & 0x0F) == 0) return true;
		else if (xsocket->type == UDP && !xsocket->host && (xsocket->flag & 0x0F) == 8) return true;
	}

	return false;
}

#pragma endregion


#pragma region Cleaning

// Closes the socket, clears messages and stops every running process
void XSocket::Close()
{
	if (connectThr != NULL)
	{	TerminateThread(connectThr, -1);
	}

	if (socketObj != INVALID_SOCKET)
	{	closesocket(socketObj);
	}

	this->connectThr = NULL;
	this->socketObj  = INVALID_SOCKET;

	this->addr  = "";
	this->port  = 0;
	this->type  = 0;
	this->ctime = 0;
	this->flag |= 0x0E;
}

// Closes the XSocket
void IXSocket::Close()
{
	if (xsocket != NULL)
	{	xsocket->Close();
	}
}

// Closes the XSocket and frees memory allocation
void IXSocket::Delete()
{
	if (xsocket != NULL)
	{	xsocket->Close();
		delete xsocket;
	}
}

#pragma endregion

