#ifndef RPCFUNCTION_H
#define RPCFUNCTION_H

#include <string>

typedef std::string   str;
typedef const char*   cstr;
typedef unsigned char byte;
typedef unsigned int  uint;

template <class Return, class Funct, class Params>
class Function
{
public:
	str    name;		// Name of the function
	Return result;		// Retrun type of the function
	Funct  funct;		// Function pointer to the function
	Params params;		// Parameter type list of the function

	// Public constructors
	Function(str n, Return r, Funct f, Params p) : name(n), result(r), funct(f), params(p) {}
};

//Creates a Function Object
template <class Return, class Funct, class Params>
static auto MakeFunction(str n, Return r, Funct f, Params p)
{
	return Function<Return, Funct, Params>(n, r, f, p);
}

#endif