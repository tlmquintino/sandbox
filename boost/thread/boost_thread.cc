#include <iostream>

#include <boost/thread.hpp>

#define HERE  std::cout << "--" << __LINE__ << std::endl;

void do_stuff_1()
{
    for( int i = 0; i < 5; ++i )
    {    
        std::cout << "[1] doing stuff ...\n";
        ::sleep(2);
    }
}

void do_stuff_2()
{
    for( int i = 0; i < 10; ++i )
    {    
        std::cout << "[2] doing stuff ...\n";
        ::sleep(1);
    }
}

int main()
{
    std::cout << "> starting main" << std::endl;

    try 
    {    
        boost::thread t1(do_stuff_1);
        boost::thread t2(do_stuff_2);
        t1.join();
        t2.join();
    }
    catch(std::exception& e)
    {
        std::cerr << "caught exception: " << e.what() << std::endl;
    }

    std::cout << "> ending main" << std::endl;
}

