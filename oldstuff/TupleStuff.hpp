///@file TupleStuff.hpp Convenience functions for dealing with tuples (some are hacks for C++14 functionality)
#include <tuple>


using namespace std;

// TODO: See if these are available in Boost?
/// Has a typedef same_type<T,T>::type Result only if types are the same (removes references)
template<typename T,typename U,typename Result>struct same_type {};
template<typename T,typename Result>struct same_type<T,T,Result> { typedef Result type; };

/// Has a typedef different_type<T,U>::type Result only if types T,U differ (removes references)
template<typename T,typename U,typename Result>struct different_type { typedef Result type; };
template<typename T,typename Result>struct different_type<T,T,Result> {  };



/** The functions below get a value from a tuple by type. Apparently this is going into the C++14 standard under get<T,TupleTypes>.
 * Returns either a T* to the first element of type T after I, or NULL if not found.
 *
 * @param 	t 			The tuple
 * @tparam 	T			The type to search for
 * @tparam 	I			Current index to search (default 0)
 * @tparam 	TupleTypes	Types inside the tuple
 * @returns Pointer to the item found, or NULL if not present.
 */

template<typename T,typename... TupleTypes>constexpr T* get_by_type(std::tuple<TupleTypes...>&t,int,int);


// Case where the types match
template<typename T,unsigned I=0,typename... TupleTypes>constexpr T* get_by_type(std::tuple<TupleTypes...>& t,
		typename same_type<T,typename std::remove_reference<decltype(std::get<I,TupleTypes...>(t))>::type,int>::type =0,
		typename std::enable_if<(I<sizeof...(TupleTypes)), int>::type=0)
{
	return &std::get<I>(t);
}

// Case where the types differ and we're at the last; must avoid instantiating <I+1> because get<I+1> will be invalid
template<typename T,unsigned I=0,typename... TupleTypes>constexpr T* get_by_type(std::tuple<TupleTypes...>& t,
		typename different_type<T,typename std::remove_reference<decltype(std::get<I,TupleTypes...>(t))>::type,int>::type =0,
		typename std::enable_if<(I==sizeof...(TupleTypes)-1),int>::type=0)
{
	return NULL;
}

// Lastly, the case where the types differ
template<typename T,unsigned I=0,typename... TupleTypes>constexpr T* get_by_type(std::tuple<TupleTypes...>& t,
		typename std::enable_if<(I<sizeof...(TupleTypes)-1), int>::type=0,
		typename different_type<T,typename std::remove_reference<decltype(std::get<I,TupleTypes...>(t))>::type,int>::type =0)
{
	return get_by_type<T,I+1U,TupleTypes...>(t);
}




struct bad_tuple_element;


/** Variant of get_by_type which returns a reference or throws an exception if not found
 * @throws bad_tuple_element Requested type could not be found
 */

template<typename T,unsigned I=0,typename... TupleTypes>T& get_by_type_ref(std::tuple<TupleTypes...>& t)
{
	if (T* p=get_by_type<T,0,TupleTypes...>(t))
		return *p;
	else
		throw bad_tuple_element();
}

struct bad_tuple_element {
	const char* what() const { return "Incorrect tuple element"; };
};



/** Creates a new tuple by dropping elements from a source tuple
 * TODO: See if can create a proxy class holding T...& or const Ts...& to eliminate need to copy elements
 */

// I is the number of elements to drop, J is the element currently being considered

template<unsigned I,unsigned J,typename TupleType,typename... Ts>struct tuple_drop{};

template<unsigned I,unsigned J,typename TupleType,typename T,typename... Ts>struct tuple_drop<I,J,TupleType,T,Ts...> {
	typedef typename tuple_drop<I-1,J+1,TupleType,Ts...>::type type;

	static type apply(const TupleType& t){ return tuple_drop<I-1,J+1,TupleType,Ts...>::apply(t); }
};

template<unsigned J,typename TupleType>struct tuple_drop<0,J,TupleType>{
	typedef std::tuple<> type;
	static type apply(const TupleType&){ return make_tuple(); }
};

template<unsigned J,typename TupleType,typename T,typename... Ts>struct tuple_drop<0,J,TupleType,T,Ts...> {
	typedef std::tuple<T,Ts...> type;

	static type apply(const TupleType& t){ return tuple_cat(make_tuple(get<J>(t)),
			tuple_drop<0,J+1,TupleType,Ts...>::apply(t)); }
};

template<unsigned I=1,typename T,typename... Ts>typename tuple_drop<I,0,std::tuple<T,Ts...>,T,Ts...>::type drop(const std::tuple<T,Ts...>& t){
	return tuple_drop<I,0,std::tuple<T,Ts...>,T,Ts...>::apply(t);
}



template<unsigned I,typename TupleType,typename... Ts>struct LoggerResult {};

template<class L>std::tuple<typename L::ResultType> get_one_result_tuple(const L& l,typename L::single_result_tag::type =true_type());
template<class L>typename L::ResultType get_one_result_tuple(const L& l,typename L::tuple_result_tag::type =true_type());


// recursive case
template<unsigned I,typename TupleType,typename T,typename... Ts>struct LoggerResult<I,TupleType,T,Ts...> {
	typedef TupleType logger_type;
	typedef decltype(tuple_cat(
			get_one_result_tuple(get<I>(std::declval<TupleType>())),
			LoggerResult<I+1,TupleType,Ts...>::apply(std::declval<TupleType>()))) type;

	static type apply(const TupleType& t){ return tuple_cat(
			get_one_result_tuple(get<I>(t)),
			LoggerResult<I+1,TupleType,Ts...>::apply(t)); }
};

// terminal case
template<unsigned I,typename TupleType>struct LoggerResult<I,TupleType> {
	typedef const std::tuple<> logger_type;
	typedef const std::tuple<> type;

	static type apply(const TupleType&){ return make_tuple(); }
};


template<class L>std::tuple<typename L::ResultType> get_one_result_tuple(const L& l,typename L::single_result_tag::type =true_type())
{
	return make_tuple(l.getResults());
}

template<class L>typename L::ResultType get_one_result_tuple(const L& l,typename L::tuple_result_tag::type =true_type())
{
	return l.getResults();
}

template<typename... Ts>typename LoggerResult<0,std::tuple<Ts...>,Ts...>::type get_results_tuple(const std::tuple<Ts...>& l)
{
	return LoggerResult<0,std::tuple<Ts...>,Ts...>::apply(l);
}





/** Provides an overload of operator<< for tuples, using the operator<< for each member type separated by spaces.
 *
 */

template<unsigned I=0,class OS,typename... Ts>typename std::enable_if<(I <sizeof...(Ts)),OS&>::type operator<<(OS& os,const std::tuple<Ts...>& t);
template<unsigned I=0,class OS,typename... Ts>typename std::enable_if<(I==sizeof...(Ts)),OS&>::type operator<<(OS& os,const std::tuple<Ts...>& t);


template<unsigned I=0,class OS,typename... Ts>typename std::enable_if<(I <sizeof...(Ts)),OS&>::type operator<<(OS& os,const std::tuple<Ts...>& t)
{
	return operator<<<I+1>(os << get<I>(t) << ' ',t);
}

template<unsigned I=0,class OS,typename... Ts>typename std::enable_if<(I==sizeof...(Ts)),OS&>::type operator<<(OS& os,const std::tuple<Ts...>& t)
{
	return os;
}


/// Converts a T into std::tuple<T>
template<typename T>std::tuple<T> make_tuple_if_not(const T& t){ return make_tuple(t); }

/// Leaves a std::tuple<Ts...> unchanged
template<typename... Ts>std::tuple<Ts...> make_tuple_if_not(const std::tuple<Ts...>& t){ return t; }
