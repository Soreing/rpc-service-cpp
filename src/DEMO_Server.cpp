#include <rpc-service/RPCService.h>
#include <rpc-service/RPCFunction.h>
#include <iostream>

// Writes all parameters to the console
// Unpacks parameters recursively
template<class T, class... Args>
void console(T data, Args... args)
{	std::cout << data;
	console(args...);
}

template<class T>
void console(T data)
{	std::cout << data;
}

//Abstract Data Type of an int and a boolean
class ADT
{
public:
	int integer;
	float real;

public:
	ADT(int i, float f) : integer(i), real(f) {}
	void print() { console("(", integer, ", ", real, ")"); }

	friend std::ostream& operator<<(std::ostream &o, ADT data)
	{	std::cout << "(" << data.integer << ", " << data.real << ")";
		return o;
	}
};

// Overloading the Marshall and Unmarshall functions (RPC operators)
// Overloading allows the RPC send and receive ADT objects.
str Marshall(ADT raw)
{	return str((char*)&raw, sizeof(raw));
}
ADT Unmarshall(cstr data, int* size, Type<ADT>)
{	if (size != NULL) { *size = sizeof(ADT); } return *(ADT*)data;
}


// Test function that prints an abstract datatype and returns the sum of 2 integers.
// Should be called by the Execute function.
int TestFunction(int a, int b, ADT c)
{	c.print();
	console('\n');
	return a + b;
}

// Test function that prints an abstract datatype and returns the sum of 2 integers.
// Should be called by the Execute function.
float Divide(int a, int b)
{	return (float)a / b;
}

// Empty function
void NoFunction() 
{
}

void main()
{	//Create RPC Handler with available functions
	auto RPCs = std::make_tuple(
		MakeFunction("NoFunction", Type<void>(), NoFunction, std::tuple<>()),
		MakeFunction("Divide", Type<float>(), Divide, std::tuple<Type<int>, Type<int> >()),
		MakeFunction("TestFunction", Type<int>(), TestFunction, std::tuple<Type<int>, Type<int>, Type<ADT> >())
	);

	auto service = MakeIRPCService(RPCs);
	
	if(service.Start(7971))
	{	while (true) {}
	}
}