#ifndef META_TYPES
#define META_TYPES

#include <tuple>

template<class T>
class Type 
{public: 
	typedef T type; 
};

template<class Var>
Type<Var> GetType(Var) { return Type<Var>(); }

class Null
{public:
	typedef Null type;
	typedef Null next;
};

template <class Type, class Next>
class TypeList
{public:
	typedef Type type;
	typedef Next next;
};

//Push Tuple
template<class Tuple, class Data>
static auto PushTuple(Tuple tuple, Data data)
{	auto size = std::make_index_sequence< std::tuple_size<Tuple>::value>{};
	return PushTuple(tuple, data, size);
}

template<class Tuple, class Data, size_t... Is>
static auto PushTuple(Tuple tuple, Data data, std::index_sequence<Is...>)
{	return std::make_tuple(std::get<Is>(tuple)..., data);
}


//Pop Tuple
template<class Tuple>
static auto PopTuple(Tuple tuple)
{	auto size = std::make_index_sequence< std::tuple_size<Tuple>::value>{};
	return PopTuple(tuple, size);
}

template<class Tuple, size_t... Is>
static auto PopTuple(Tuple tuple, std::index_sequence<Is...>)
{	return PopTuple(std::get<Is>(tuple)...);
}

template<class Pop, class... Args>
static auto PopTuple(Pop val, Args... tuple)
{	return std::make_tuple(tuple...);
}

static auto PopTuple()
{	return std::tuple<>();
}

#endif