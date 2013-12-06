#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <iostream>

typedef boost::function<int(int)> int_2_int_f;

// Y-combinator compatible factorial
// which avoids recursion
int fact( int_2_int_f f, int v )
{
  if(v == 0)
    return 1;
  else
    return v * f(v -1);
}

// Y-combinator for the int type
int_2_int_f ycomb ( boost::function< int ( int_2_int_f, int ) > f )
{
  return boost::bind( f, boost::bind( &ycomb, f ), _1);
}


int main(int argc,char** argv)
{
  int_2_int_f factorial = ycomb( fact );
  std::cout << factorial(5) << std::endl;
  return 0;
}