// Valarray.h

/* Put your solution in this file, we expect to be able to use
 * your epl::valarray class by simply saying #include "Valarray.h"
 *
 * We will #include "Vector.h" to get the epl::vector<T> class 
 * before we #include "Valarray.h". You are encouraged to test
 * and develop your class using std::vector<T> as the base class
 * for your epl::valarray<T>
 * you are required to submit your project with epl::vector<T>
 * as the base class for your epl::valarray<T>
 */

#ifndef _Valarray_h
#define _Valarray_h
#include "Vector.h"
#include <complex>
//using std::vector; // during development and testing
using epl::vector; // after submission
using std::complex;

template <typename>
struct SRank;
template <typename T> struct SRank { static constexpr int value = 0; static constexpr bool is_complex = 0;};
template <> struct SRank<int> { static constexpr int value = 1; static constexpr bool is_complex = 0; };
template <> struct SRank<float> { static constexpr int value = 2; static constexpr bool is_complex = 0; };
template <> struct SRank<double> { static constexpr int value = 3; static constexpr bool is_complex = 0; };
template <typename T> struct SRank<complex<T>> {
	static constexpr int value = SRank<T>::value;//????
	static constexpr bool is_complex = 1;
};

template <bool p, typename T>
using EnableIf = typename std::enable_if<p, T>::type;

template <int>
struct SType;

template <> struct SType<1> { using type = int; };
template <> struct SType<2> { using type = float; };
template <> struct SType<3> { using type = double; };

template <typename T, bool c>struct CType;
template<typename T> struct CType<T, true> { using type = std::complex<T>; };
template<typename T> struct CType<T, false> { using type = T; };

template <typename T1, typename T2>
struct choose_type {
	static constexpr int t1_rank = SRank<T1>::value;
	static constexpr int t2_rank = SRank<T2>::value;
	static constexpr int max_rank = (t1_rank > t2_rank) ? t1_rank : t2_rank;
	using type1 = typename SType<max_rank>::type;
	static constexpr bool complex = SRank<T1>::is_complex || SRank<T2>::is_complex;
	using type = typename CType<type1, complex>::type;
};

template <typename T1, typename T2>
using ChooseType = typename choose_type<T1, T2>::type;

template<typename V> struct Wrap;
template<typename V1, typename V2> struct striptype;
template<typename V1, typename V2> struct striptype<Wrap<V1>,Wrap<V2>> {
	using type = ChooseType<typename V1::value_type, typename V2::value_type>;
};
template<typename V, typename S> struct striptype<Wrap<V>, S> {
	using type = ChooseType<typename V::value_type, S>;
};
template<typename S, typename V> struct striptype<S, Wrap<V>> {
	using type = ChooseType<S, typename V::value_type>;
};
template <typename V1, typename V2>
using StripType = typename striptype<V1, V2>::type;


template<typename T>
class valarray1 :public vector<T> {
public:
	using Same = valarray1<T>;

public:
	using value_type = T;
	using vector<T>::vector;
	valarray1<T>():vector<T>(){}
    valarray1<T>(const vector<T>& v):vector<T>(v){}
	valarray1<T>(std::initializer_list<T> l) : vector<T>(l) {}
	explicit valarray1<T>(uint64_t size) : vector<T>(size) {}

	Same& operator=(Same const& rhs) {
		uint64_t size = this->size() < rhs.size() ? this->size() : rhs.size(); 
		Same& lhs{ *this };
		for (uint64_t k = 0; k < size; k += 1) {
			lhs[k] = rhs[k];
		}
		return *this;
	}
};
template<typename V>
class const_iterator {
	const V vec;
	uint64_t index;
public:
	const_iterator(const V t1, const uint64_t t2) :vec(t1), index(t2) {}
	typename V::value_type operator*()const { return vec[index]; }
	bool operator==(const_iterator const& rhs)const {
		return this->index == rhs.index;
	}
	bool operator!=(const_iterator const& rhs)const {
		return !(*this == rhs);
	}
	bool operator>(const_iterator const& rhs)const {
		return(rhs.index  < this->index);
	}
	bool operator<(const_iterator const& rhs)const {
		return(rhs.index > this->index);
	}
	bool operator<=(const_iterator const& rhs)const {
		return !(rhs.index  < this->index);
	}
	bool operator>=(const_iterator const&rhs)const {
		return !(this->index  < rhs.index);
	}
	const_iterator& operator++(void) {
		index += 1;
		return *this;
	}
	const_iterator& operator--(void) {
		index -= 1;
		return *this;
	}
	const_iterator operator++(int) {
		const_iterator t{ *this };
		operator++();
		return t;
	}
	const_iterator operator--(int) {
		const_iterator t{ *this };
		operator--();
		return t;
	}
};
template <typename T>
struct choose_ref {
	using type = T;
};
template<typename T>struct choose_ref<valarray1<T>> {
	using type = valarray1<T> const&;
};
template <typename T> using ChooseRef = typename choose_ref<T>::type;

template<typename V1Type, typename V2Type, typename Op>
class proxy {
public:
	using LeftType = ChooseRef<V1Type>;
	using RightType = ChooseRef<V2Type>;
	LeftType val1;
	RightType val2;
	Op op;
public:
	using value_type = ChooseType<typename V1Type::value_type, typename V2Type::value_type>;
	using result_type = value_type;
	proxy(V1Type const& left, V2Type const& right, Op op1) :
		val1(left), val2(right), op(op1) {}
	proxy(const proxy& that) :val1(that.val1), val2(that.val2), op(that.op) {}
	//proxy() = default;
	//~proxy() = default;
	uint64_t size(void)const {
		uint64_t size = val1.size() < val2.size() ? val1.size() : val2.size();
		return size;
	}
	value_type operator[](uint64_t k)const {
		return op(val1[k], val2[k]);
	}
	const_iterator<proxy>begin(void)const {
		return const_iterator<proxy>(*this, 0);
	}
	const_iterator<proxy>end(void)const {
		return const_iterator<proxy>(*this, this->size());
	}
};
template<typename T, typename Op>
class Uproxy {
public:
	using UType = ChooseRef<T>;
	UType val;
	Op op;
	
	using result_type = ChooseType<typename T::value_type,typename Op::result_type>;
	using value_type = result_type;
	Uproxy(UType const& val1, Op op1) :
		val(val1), op(op1) {}
	Uproxy(const Uproxy& that) :val(that.val), op(that.op) {}

	uint64_t size(void)const {
		uint64_t size = val.size();
		return size;
	}

	result_type operator[](uint64_t k)const {
		return op(val[k]);
	}
	const_iterator<Uproxy>begin(void)const {
		return const_iterator<Uproxy>(*this, 0);
	}
	const_iterator<Uproxy>end(void)const {
		return const_iterator<Uproxy>(*this, this->size());
	}

};
template <typename V>
struct sf {
	using result_type = ChooseType<V,double>;
	result_type operator()(V v) const {
		return std::sqrt(v);
	}
};
template<typename V>
struct Wrap :public V {
	using V::V;
	using value_type = typename V::value_type;
	Wrap():V(){}
	Wrap(const V& v) :V(v) {}
	explicit Wrap(uint64_t size) :V(size) {}
	template<typename T>
	Wrap(std::initializer_list<T> l) : V(l) {}
	Wrap<V>& operator=(const Wrap<V>& rhs) {
		uint64_t size = this->size() < rhs.size() ? this->size() : rhs.size(); 
		Wrap<V>& lhs{ *this };
		for (uint64_t k = 0; k < size; k += 1) {
			lhs[k] = rhs[k];
		}
		return *this;
	}
	template<typename T1>
	Wrap<V>(const Wrap<T1>& that) {
		uint64_t size = that.size();
		for (uint64_t k = 0; k < size; k += 1) {
			this->push_back(static_cast<typename V::value_type>(that[k]));
		}
	}
	template<typename T1>
	Wrap<V>& operator=(const Wrap<T1>& rhs) {
		uint64_t size = this->size() < rhs.size() ? this->size() : rhs.size();
		Wrap<V>& lhs{ *this };
		for (uint64_t k = 0; k < size; k += 1) {
			lhs[k] =static_cast<typename V::value_type>(rhs[k]);
		}
		return *this;
	}

	template<typename Op>
	typename Op::result_type accumulate(Op op) {
		uint64_t size = this->size();
		if (size == 0) { return 0; }
		else{
		typename Op::result_type i(this->operator[](0));
		
		for (uint64_t k = 1; k < size; k += 1) {
			i = op(i, this->operator[](k));
		}
		return i;
		}
	}

	typename std::plus<value_type>::result_type sum(void) {
		return accumulate(std::plus<value_type>{});
	}
	template <typename Op>
	Wrap<Uproxy<V, Op>>apply(Op op) {
		Uproxy<V,Op>result{ *this,op };
		return wrap(result);
	}
	Wrap<Uproxy<V, sf<value_type>>>sqrt(void) {
		return apply(sf<value_type>{});
	}
};
template<typename T>
Wrap<T>wrap(T const& x) {
	return Wrap<T>(x);
}
template<typename T>
class Wrapscalar {
	T n;

public:
	using value_type = T;
	using result_type = value_type;
	Wrapscalar(const T t) { this->n = t; }
	T operator[](uint64_t k) const { return n; }
	uint64_t size(void) const { return -1; }
	Wrapscalar(const Wrapscalar& that) { n = that.n; }
	Wrapscalar() { n = 0; }
	const_iterator<Wrapscalar>begin(void)const {
		return const_iterator<Wrapscalar>(*this, 0);
	}
	const_iterator<Wrapscalar>end(void)const {
		return const_iterator<Wrapscalar>(*this, this->size());
	}
};


template<typename T>
using valarray = Wrap<valarray1<T>>;
/* plus */

template<typename V1, typename V2>
Wrap<proxy<V1, V2, std::plus<StripType<Wrap<V1>, Wrap<V2>>>>>
operator+(Wrap<V1> const& lhs, Wrap<V2> const& rhs) {
	V1 const& left{ lhs };
	V2 const& right{ rhs };
	proxy<V1, V2, std::plus<StripType<Wrap<V1>,Wrap<V2>>>>result{ left, right,std::plus<StripType<Wrap<V1>,Wrap<V2>>>{} };
	return wrap(result);
}
template <typename V, typename S>
EnableIf<SRank<StripType<Wrap<V>, S>>::value!=0,Wrap<proxy<V, Wrapscalar<S>, std::plus<StripType<Wrap<V>, S>>>>>
operator+(Wrap<V> const& lhs, S const rhs) {
	V const& left{ lhs };
	Wrapscalar<S>scalarwrap{ rhs };
	proxy<V, Wrapscalar<S>, std::plus<StripType<Wrap<V>, S>>> result{ left,scalarwrap,std::plus<StripType<Wrap<V>,S>>{} };
	return wrap(result);
}
template <typename S, typename V>
EnableIf<SRank<StripType<S, Wrap<V>>>::value!=0,Wrap<proxy<Wrapscalar<S>, V, std::plus<StripType<S, Wrap<V>>>>>>
operator+(S const lhs, Wrap<V> const& rhs) {
	Wrapscalar<S>scalarwrap{ lhs };
	V const& right{ rhs };
	proxy<Wrapscalar<S>, V, std::plus<StripType<S,Wrap<V>>>> result{ scalarwrap,right,std::plus<StripType<S,Wrap<V>>>{} };
	return wrap(result);
}
/* minus */
template<typename V1, typename V2>
Wrap<proxy<V1, V2, std::minus<StripType<Wrap<V1>, Wrap<V2>>>>>
operator-(Wrap<V1> const& lhs, Wrap<V2> const& rhs) {
	V1 const& left{ lhs };
	V2 const& right{ rhs };
	proxy<V1, V2, std::minus<StripType<Wrap<V1>, Wrap<V2>>>>result{ left, right,std::minus<StripType<Wrap<V1>,Wrap<V2>>>{} };
	return wrap(result);
}
template <typename V, typename S>
EnableIf<SRank<StripType<Wrap<V>, S>>::value!=0,Wrap<proxy<V, Wrapscalar<S>, std::minus<StripType<Wrap<V>, S>>>>>
operator-(Wrap<V> const& lhs, S const rhs) {
	V const& left{ lhs };
	Wrapscalar<S>scalarwrap{ rhs };
	proxy<V, Wrapscalar<S>, std::minus<StripType<Wrap<V>, S>>>result{ left,scalarwrap,std::minus<StripType<Wrap<V>,S>>{} };
	return wrap(result);
}
template <typename S, typename V>
EnableIf<SRank<StripType<S, Wrap<V>>>::value!=0,Wrap<proxy<Wrapscalar<S>, V, std::minus<StripType<S, Wrap<V>>>>>>
operator-(S const lhs, Wrap<V> const& rhs) {
	Wrapscalar<S>scalarwrap{ lhs };
	V const& right{ rhs };
	proxy<Wrapscalar<S>, V, std::minus<StripType<S, Wrap<V>>>> result{ scalarwrap,right,std::minus<StripType<S,Wrap<V>>>{} };
	return wrap(result);
}
/* multiplies */
template<typename V1, typename V2>
Wrap<proxy<V1, V2, std::multiplies<StripType<Wrap<V1>, Wrap<V2>>>>>
operator*(Wrap<V1> const& lhs, Wrap<V2> const& rhs) {
	V1 const& left{ lhs };
	V2 const& right{ rhs };
	proxy<V1, V2, std::multiplies<StripType<Wrap<V1>, Wrap<V2>>>>result{ left, right,std::multiplies<StripType<Wrap<V1>,Wrap<V2>>>{} };
	return wrap(result);
}
template <typename V, typename S>
EnableIf<SRank<StripType<Wrap<V>, S>>::value!=0,Wrap<proxy<V, Wrapscalar<S>, std::multiplies<StripType<Wrap<V>, S>>>>>
operator*(Wrap<V> const& lhs, S const rhs) {
	V const& left{ lhs };
	Wrapscalar<S>scalarwrap{ rhs };
	proxy<V, Wrapscalar<S>, std::multiplies<StripType<Wrap<V>, S>>> result{ left,scalarwrap,std::multiplies<StripType<Wrap<V>,S>>{} };
	return wrap(result);
}
template <typename S, typename V>
EnableIf<SRank<StripType<S, Wrap<V>>>::value!=0,Wrap<proxy<Wrapscalar<S>, V, std::multiplies<StripType<S, Wrap<V>>>>>>
operator*(S const lhs, Wrap<V> const& rhs) {
	Wrapscalar<S>scalarwrap{ lhs };
	V const& right{ rhs };
	proxy<Wrapscalar<S>, V, std::multiplies<StripType<S, Wrap<V>>>> result{ scalarwrap,right,std::multiplies<StripType<S,Wrap<V>>>{} };
	return wrap(result);
}
/* divides */
template<typename V1, typename V2>
Wrap<proxy<V1, V2, std::divides<StripType<Wrap<V1>, Wrap<V2>>>>>
operator/(Wrap<V1> const& lhs, Wrap<V2> const& rhs) {
	V1 const& left{ lhs };
	V2 const& right{ rhs };
	proxy<V1, V2, std::divides<StripType<Wrap<V1>, Wrap<V2>>>>result{ left, right,std::divides<StripType<Wrap<V1>,Wrap<V2>>>{} };
	return wrap(result);
}
template <typename V, typename S>
EnableIf<SRank<StripType<Wrap<V>, S>>::value!=0,Wrap<proxy<V, Wrapscalar<S>, std::divides<StripType<Wrap<V>, S>>>>>
operator/(Wrap<V> const& lhs, S const rhs) {
	V const& left{ lhs };
	Wrapscalar<S>scalarwrap{ rhs };
	proxy<V, Wrapscalar<S>, std::divides<StripType<Wrap<V>, S>>> result{ left,scalarwrap,std::divides<StripType<Wrap<V>,S>>{} };
	return wrap(result);
}
template <typename S, typename V>
EnableIf<SRank<StripType<S, Wrap<V>>>::value!=0,Wrap<proxy<Wrapscalar<S>, V, std::divides<StripType<S, Wrap<V>>>>>>
operator/(S const lhs, Wrap<V> const& rhs) {
	Wrapscalar<S>scalarwrap{ lhs };
	V const& right{ rhs };
	proxy<Wrapscalar<S>, V, std::divides<StripType<S, Wrap<V>>>> result{ scalarwrap,right,std::divides<StripType<S,Wrap<V>>>{} };
	return wrap(result);
}
template<typename V>
EnableIf<SRank<typename V::value_type>::value!=0,Wrap<Uproxy<V, std::negate<typename V::value_type>>>>
operator -(Wrap<V> const&rhs) {
	V const& right{ rhs };
	Uproxy<V, std::negate<typename V::value_type>>result{ right,std::negate<typename V::value_type>{} };
	return wrap(result);
}


template<typename T>
std::ostream& operator<<(std::ostream& out,  Wrap<T> const& vala) {
	const char* pref = "";
	for (uint64_t k = 0; k < vala.size(); k += 1) {
		out << pref << vala[k];
		pref = ",";
	}
	return out;
}


#endif /* _Valarray_h */

