#include <rpc-service/RPCService.h>
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

void main()
{
	float fresult = 0;
	int   iresult = 0;

	std::cout<< "FUNCTION 1 !\n";
	if(RPC("127.0.0.1", 7971, fresult, "Divide", 3, 6))
	{	console("<Result> ", fresult, "\n\n");
	}

	std::cout<< "FUNCTION 2 !\n";
	if(RPC("127.0.0.1", 7971, "NoFunction"))
	{	console("Call Succesful!\n\n");
	}

	std::cout<< "FUNCTION 3 !\n";
	if (RPC("127.0.0.1", 7971, iresult, "TestFunction", 1, 2, ADT(123, 456.789f)))
	{	console("<Result> ", iresult, "\n\n");
	}
}