#ifndef RPCSERVICE_H
#define RPCSERVICE_H

#include "mp_types.h"
#include "XSocket.h"

#include <functional>
#include <vector>
#include <string>
#include <tuple>

typedef std::string   str;
typedef const char*   cstr;
typedef unsigned char byte;
typedef unsigned int  uint;

// Request structure consisting of an XSocket interface to a client
// And a void* memory address to a thread fullfilling the request
struct Request
{
	IXSocket client;		// Interface to the XSocket of the client being served
	void*	 thread;		// Pointer to the thread handling the client's request
};


// A service with a list of functions that can be requested by the client
// Listens to incoming requests in a separate thread, and creates a new
// Threads for completing the requests. Active requests are stored in a vector.
// RPCService is managed by IRPCService, an interface for dealing with the service.
template<class List>
class RPCService
{
public:
	List     RPCList;				// List of functions available in the service
	IXSocket server;				// Interface of the socket listening for requests
	void*    serverThr;				// Handle of the thread listening for requests

	std::vector<Request> requests;	// List of requests running on the service

	RPCService(List functions) 
	: RPCList(functions), serverThr(NULL) 
	{ 
	}

	// Stops the service if it's currently running
	// Starts the service on the port specified
	// Creates a thread listening to new requests
	bool Start(int port)
	{
		Stop();
		server.Host(TCP, port);

		if (server.good())
		{	serverThr = CreateThread(NULL, NULL, serverFn<List>, this, NULL, NULL);
		}

		return serverThr != NULL;
	}
	 
	// Closes the server socket, and deallocates all client sockets 
	// Terminates all running threads and clears requests
	void Stop()
	{
		server.Close();
		if (serverThr != NULL)
		{	TerminateThread(serverThr, 0);
			serverThr = NULL;
		}

		for (int i = 0; i < requests.size(); i++)
		{	if (requests[i].thread != NULL)
			{	TerminateThread(requests[i].thread, 0);
				requests[i].client.Delete();
			}
		}

		requests.clear();
	}


	// Starts the process of executing the requested service
	// Creates an index sequence to start cycling through services
	bool Parse(IXSocket client, str name, str data)
	{	auto iSeq = std::make_index_sequence< std::tuple_size< List >::value>{};
		return Cycle(client, name, data, iSeq);
	}

	// Starts cycling through the services with the indexes
	template<size_t... Is>
	bool Cycle(IXSocket client, str name, str data, std::index_sequence<Is...>)
	{	return Evaluate(client, name, data, std::get<Is>(RPCList)...);
	}

	// Evaluates the request by comparing it with the list of available services
	template<class Proc, class... Args>
	bool Evaluate(IXSocket client, str name, str data, Proc function, Args... args)
	{
		if (function.name == name)
		{	return Prepare(client, function.result, function.funct, function.params, data);
		}
		else 
		{	return Evaluate(client, name, data, args...); 
		}
	}

	// Finishes evaluating the request with the last service available
	template<class Proc>
	bool Evaluate(IXSocket client, str name, str data, Proc function)
	{
		if (function.name == name)
		{	return Prepare(client, function.result, function.funct, function.params, data);
		}
		else 
		{	return false;
			console("<Server> ", name, " is not an RPC function!\n"); 
			client.Delete();
		}
	}

	// Prepares the execution of the function with parameters
	// Creates an index sequence to start cycling through parameters
	template<class Return, class Proc, class Types>
	bool Prepare(IXSocket client, Return result, Proc funct, Types types, str data)
	{	auto iSeq = std::make_index_sequence< std::tuple_size< Types >::value>{};
		return Sequence(client, result, funct, types, data, iSeq);
	}

	// Starts going through the parameters supplied
	template<class Return, class Proc, class Types, size_t... Is>
	bool Sequence(IXSocket client, Return result, Proc funct, Types types, str data, std::index_sequence<Is...>)
	{	int index = 0;
		return Unpack(client, result, funct, data, index, std::tuple<>(), std::get<Is>(types)...);
	}

	// Unmarshalls the parameters from the bytes in the request
	// Moves the pointer forward to the next parameter
	template<class Return, class Proc, class Param, class Type, class... Args>
	bool Unpack(IXSocket client, Return result, Proc funct, str data, int &index, Param param, Type type, Args... args)
	{	int size = 0;
		auto var = Unmarshall(data.data() + index, &size, type);
		auto par = PushTuple(param, var);
		index += size;

		return Unpack(client, result, funct, data, index, par, args...);
	}

	// Unmarshalls the last parameter from the bytes in the request
	template<class Return, class Proc, class Param, class Type>
	bool Unpack(IXSocket client, Return result, Proc funct, str data, int &index, Param param, Type type)
	{	auto var = Unmarshall(data.data() + index, NULL, type);
		auto par = PushTuple(param, var);

		return Unpack(client, result, funct, data, index, par);
	}

	// Finishes unpacking parameters and starts executing the function
	template<class Return, class Proc, class Param>
	bool Unpack(IXSocket client, Return result, Proc funct, str data, int &index, Param param)
	{	auto iSeq = std::make_index_sequence< std::tuple_size<Param>::value>{};
		return Execute(client, result, funct, param, iSeq);
	}

	// Binds the function pointer with the parameters and executes the function
	// Sends the client the result of the function
	// Returns true if the data was sent successfully
	template<class Return, class Proc, class Params, size_t... Is>
	bool Execute(IXSocket client, Return result, Proc function, Params parameters, std::index_sequence<Is...>)
	{	auto funct = std::bind(function, std::get<Is>(parameters)...);
		auto data  = funct();
		
		str res = Marshall(data);
		client.Send(res, (int)res.length());
		return client.good();
	}

	// Binds the function pointer with the parameters and executes the function
	// Always returns true, and the request has no return
	template< class Proc, class Params, size_t... Is>
	bool Execute(IXSocket client, Type<void>, Proc function, Params parameters, std::index_sequence<Is...>)
	{	auto funct = std::bind(function, std::get<Is>(parameters)...);
		funct();

		client.Send("1", 1);
		return client.good();
	}
};


template<class List>
class IRPCService
{
private:
	// Pointer to the RPCService object managed by the interface
	RPCService<List> *remote;

public:
	// Public constructors
	IRPCService(List RPCList) : remote(new RPCService<List>(RPCList)) {}

	// Starts the service on a specific port
	bool Start(int port)
	{	return remote->Start(port);
	}

	// Stops the service and ends all active requests
	void Stop()
	{	return remote->Stop();
	}

	// Deallocates the memory and ends the service
	void Delete()
	{	remote->Stop();
		remote->server.Delete();
		delete remote;
	}
};


// Resources for a thread fulfilling the request
template<class List>
struct Resource
{
	RPCService<List> *service;	// RPCService sevice fulfilling the request
	XSocket *socket;			// XSocket with the connection to the client
};

//Listens to new connections
template <class Type>
DWORD WINAPI serverFn(LPVOID lparameter)
{
	RPCService<Type> *remote = (RPCService<Type>*)lparameter;
	Resource<Type> resource;
	resource.service = remote;

	IXSocket client;

	while (remote->server.good())
	{
		client = remote->server.Accept();

		if (client.good())
		{
			resource.socket = client.xsocket;
			void* address = CreateThread(NULL, NULL, processFn<Type>, &resource, NULL, NULL);

			if (address != NULL)
			{	remote->requests.push_back(Request{ client, address });
			}
			else
			{	client.Delete();
			}
		}
	}

	remote->server.Close();
	remote->serverThr = NULL;
	return 0;
}

//Fulfills requests
template <class Type>
DWORD WINAPI processFn(LPVOID lparameter)
{
	Resource<Type> res = *(Resource<Type>*)lparameter;
	IXSocket client(res.socket);

	str request  = "";
	str function = "";
	str params   = "";

	if (client.good())
		request = client.Recv(NULL, 256);

	if (client.good())
	{	function = request.substr(0, request.find('\n'));
		params   = request.substr(1 + request.find('\n'));
		res.service->Parse(client, function, params);
	}
	else
	{	client.Delete();
	}

	return 0;
}


// Opens a connection to the remote computer serving requests
// Deconstructs parameters into a Byte array
// Sends the request to the remote computer and waits for the result
template<class Return, class... Args>
bool RPC(cstr address, int port, Return &data, str function, Args... args)
{
	IXSocket conn;
	str params = Package(args...);
	str request = function + '\n' + params;
	str result = "";

	conn.Open(TCP, address, port, 1);

	if (conn.good())
	{	conn.Send(request, (int)request.length());
	}

	if (conn.good())
	{	result = conn.Recv(NULL, 256);
	}

	if (result != "")
	{	data = Unmarshall(result.data(), NULL, Type<Return>());
	}

	conn.Delete();
	return result != "";
}


// Opens a connection to the remote computer serving requests
// Deconstructs parameters into a Byte array
// Sends the request to the remote computer
template<class... Args>
auto RPC(cstr address, int port, str function, Args... args)
{
	IXSocket conn;
	str params = Package(args...);
	str request = function + '\n' + params;
	str result = "";
	
	conn.Open(TCP, address, port, 1);

	if (conn.good())
	{	conn.Send(request, (int)request.length());
	}

	if (conn.good())
	{	result = conn.Recv(NULL, 256);
	}

	bool sent = conn.good();

	conn.Delete();
	return result != "";
}


//Creates an RPCService Interface
template <class List>
auto MakeIRPCService(List RPCList)
{	return IRPCService<List>(RPCList);
}


// Packages information from parameters into a Byte array.
// Packing is done recursively untill there's only one parameter left.
static str Package()
{	return "";
}

template <class Type>
static str Package(Type data)
{	return Marshall(data);
}

template<class Type, class... Args>
static str Package(Type data, Args... variadic)
{	return Marshall(data) + Package(variadic...);
}

// Breaks down data types into Byte arrays
// Byte arrays can be sent through the network
str Marshall(char      raw);
str Marshall(short     raw);
str Marshall(int       raw);
str Marshall(long      raw);
str Marshall(long long raw);
str Marshall(float     raw);
str Marshall(double    raw);
str Marshall(cstr      raw);
str Marshall(str       raw);

// Casts data from Byte arrays into the specified data type
// Assigns the number of bytes consumed in size if pointer is not null
char Unmarshall(cstr data, int* size, Type<char>);
short Unmarshall(cstr data, int* size, Type<short>);
int Unmarshall(cstr data, int* size, Type<int>);
long Unmarshall(cstr data, int* size, Type<long>);
long long Unmarshall(cstr data, int* size, Type<long long>);
float Unmarshall(cstr data, int* size, Type<float>);
double Unmarshall(cstr data, int* size, Type<double>);

#endif