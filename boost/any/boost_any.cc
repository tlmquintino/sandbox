#include <cassert>
#include <iostream>
#include <string>

#include <boost/any.hpp>

#define HERE  std::cout << "--" << __LINE__ << std::endl;

struct SomeType
{
    int i_;
    std::string s_;
    friend std::ostream& operator<<( std::ostream& os, const SomeType& v)
    {
        os << v.i_ << "/" << v.s_;
        return os;
    }

};

int main()
{
    // 1. empty

    boost::any a1 = boost::any();
    assert( a1.empty() );

    // 2. int

    boost::any a2 = int(10);
    assert( ! a2.empty() );
    std::cout << "a2 : " << boost::any_cast<int>(a2) << std::endl;

    // 3. string

    boost::any a3 = std::string("lolo+koko");
    assert( ! a3.empty() );
    std::cout << "a3 : " << boost::any_cast< std::string >(a3) << std::endl;

    // 4. SomeType

    SomeType st1; st1.i_ = 20; st1.s_ = "popo";
    boost::any a4 = st1;
    assert( ! a4.empty() );
    std::cout << "a4 : " << boost::any_cast< SomeType >(a4) << std::endl;

}

