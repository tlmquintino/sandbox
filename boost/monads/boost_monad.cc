#include <iostream>
#include <string>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

//-----------------------------------------------------------------------------

template < typename T >
struct Unit {
    Unit( const T& v ) : v_(v) {}
    T operator()()
    {
        return v_;
    }

private:
    T v_;
};

template < typename V >
Unit<V> U( const V& v ) { return Unit<V>(v); }

//-----------------------------------------------------------------------------

template < typename F, typename V >
struct Monad {

    typedef typename F::result_type result_type;
    typedef typename F::argument_type argument_type;

    Monad( const F& f, const V& v ) : f_(f), v_(v) {}
    typename F::result_type operator()()
    {
        return f_( v_() );
    }

private:
    F f_;
    V v_;
};

template < typename F, typename V >
Monad<F,V> M( const F& f, const V& v ) { return Monad<F,V>(f,v); }

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

int times  ( int r, int l ) { return r*l; }
int times2 ( int i ) { return 2*i; }
int inc    ( int i ) { return ++i; }

int stoint ( std::string s ) { return boost::lexical_cast<int>(s); }

//-----------------------------------------------------------------------------

int main()
{
    // example of using bind

    boost::function< int ( int ) > t2 = boost::bind( &times, _1 , 2 );

    std::cout << "#0 " << t2(10) << std::endl;

    // usage 1 - use directly

    boost::function< int ( int ) > f = &times2;
    boost::function< int ( int ) > g = &inc;

    std::cout << "#1 " << M( f, M( g, U(2) ))() << std::endl;

    // usage 2 - store it in a function object

    boost::function< int ( ) > fog = M( f, M( g, U(2)) );

    std::cout << "#2 " << fog() << " " << fog() << std::endl;

    // usage 3 - different types

    boost::function< int ( std::string ) > s2i = &stoint;

    boost::function< int ( ) > fos2i = M( f, M( s2i, U(std::string("21")) ) );

    std::cout << "#3 " << fos2i() << std::endl;

}
