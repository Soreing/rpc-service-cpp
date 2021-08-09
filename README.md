# rpc-service-cpp
# Description
rpc-service-cpp is a Remote Procedure Call library for C++. You can create a server that hosts functions with their implementation, and/or make a client that contacts the server to call a function and wait for a return value. Arguments for the functions get serialized to bytes and sent with the request.

# Installation
Add the folder `rpc-service` from `/include` in your include path. If you want to compile the library from source, include `XSocket.cpp` and `RPCService.cpp` from the `/src` folder. Alternatively, you can compile the source code to a static library and include it that way.

# Usage
The basic usage of the library is included in `DEMO_Server.cpp` to create an RPC Server and `DEMO_Client.cpp` to create a Client.

## Creating a Server
You will need the following headers to create an RPC Server
```c++
#include <rpc-service/RPCService.h>
#include <rpc-service/RPCFunction.h>
```

To create a server that hosts specific functions, first you will need to create a list of functions with a list of parameters types using `MakeFunction()`.
```c++
auto RPCs = std::make_tuple(
    MakeFunction("Function", Type<void>(), NoFunction, std::tuple<>()),
    MakeFunction("Divide", Type<float>(), Divide, std::tuple<Type<int>, Type<int> >()),
    MakeFunction("TestFunction", Type<int>(), TestFunction, std::tuple<Type<int>, Type<int>, Type<ADT> >())
);
``` 
The `1st` parameter is the name ID of the function, the `2nd` parameter is the return type, `3rd` is the function pointer for the implementation, then the `4th` parameter is a tuple of types for parameters to the function call.  
  
After the list of functions is created, you need to create an interface to and RPC Service, then start hosting the service online on a port number.
```c++
auto service = MakeIRPCService(RPCs);

// Service running on port 7971 for example
if(service.Start(7971))
{    while (true) {}
}
```

## Creating a Client
You will need the following headers to create an RPC Client
```c++
#include <rpc-service/RPCService.h>
```

To create a client that contacts the RPC Server, you simply need to call a function with the correct parameters
```c++
float fresult;
if(RPC("127.0.0.1", 7971, fresult, "Divide", 3, 6))
{    cout << fresult;
}
```
The `RCP()` function returns `true` if the call was successful, `false` if it was not. Parameters are:

1. IP address of the RPC Server
2. Port number of the RPC Server
3. Return value (Skip if the function has no return)
4. Name ID of the requested function
5. Any additional parameters will be converted to bytes and sent with the request as data for calling the function with.  
  
More Examples:
```c++
// Function with no return value
if(RPC("127.0.0.1", 7971, "Function"))
{    cout << "Call Succesful!\n\n";
}

// Function with an abstract data type as a parameter
int iresult;
if (RPC("127.0.0.1", 7971, iresult, "TestFunction", 1, 2, ADT(123, 456.789f)))
{   cout << iresult;
}
```

## Working with Abstract Data Types
You can use classes and structures as arguemnts in the RPC calls only if both the server and the client defines them, and implement the required functions. Both functions have been defined for common C++ types.
  
The `Marshall()` function converts the data type to an array of bytes as a string, while the `Unmarshall()` function converts a string of bytes to an object.

```c++
str Marshall(ADT raw)
{   return str((char*)&raw, sizeof(raw));
}
ADT Unmarshall(cstr data, int* size, Type<ADT>)
{    if (size != NULL) { *size = sizeof(ADT); } return *(ADT*)data;
}
```