#include <rpc-service/RPCService.h>

// Breaks down data types into Byte arrays
// Byte arrays can be sent through the network
str Marshall(char      raw) { return str((char*)&raw, sizeof(raw)); }
str Marshall(short     raw) { return str((char*)&raw, sizeof(raw)); }
str Marshall(int       raw) { return str((char*)&raw, sizeof(raw)); }
str Marshall(long      raw) { return str((char*)&raw, sizeof(raw)); }
str Marshall(long long raw) { return str((char*)&raw, sizeof(raw)); }
str Marshall(float     raw) { return str((char*)&raw, sizeof(raw)); }
str Marshall(double    raw) { return str((char*)&raw, sizeof(raw)); }
str Marshall(cstr      raw) { return str(raw, sizeof(raw)); }
str Marshall(str       raw) { return raw; }


// Casts data from Byte arrays into the specified data type
// Assigns the number of bytes consumed in size if pointer is not null
char Unmarshall(cstr data, int* size, Type<char>)
{ 
	if (size != NULL)
	{	*size = 1;
	}
	return *(char*)data; 
}

short Unmarshall(cstr data, int* size, Type<short>)
{ 
	if (size != NULL)
	{	*size = 2; 
	}
	return *(short*)data; 
}

int Unmarshall(cstr data, int* size, Type<int>) 
{ 
	if (size != NULL) 
	{	*size = 4; 
	}
	return *(int*)data; 
}

long Unmarshall(cstr data, int* size, Type<long>)
{ 
	if (size != NULL) 
	{	*size = 8; 
	}
	return *(long*)data; 
}

long long Unmarshall(cstr data, int* size, Type<long long>)
{
	if (size != NULL)
	{	*size = 8;
	}
	return *(long long*)data;
}

float Unmarshall(cstr data, int* size, Type<float>)
{ 
	if (size != NULL) 
	{	*size = 8; 
	}
	return *(float*)data; 
}

double Unmarshall(cstr data, int* size, Type<double>)
{ 
	if (size != NULL) 
	{	*size = 8; 
	}
	return *(double*)data; 
}