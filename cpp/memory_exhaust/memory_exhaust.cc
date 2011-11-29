#include <iostream>
#include <new>

int main()
{
    size_t size = 0;

    std::cout << "which size?" << std::endl;
    std::cin >> size;
    std::cout << (double)(size*sizeof(int)) / (1024*1024) << " Mb" << std::endl;

    int* ptr;

    try
    {
        ptr = new int[size];
    }
    catch ( std::bad_alloc& e )
    {
        std::cerr << "bad alloc caught: " << e.what() << std::endl;
    }

    std::cout << "looping" << std::endl;

    for( size_t i; i < size; ++i )
        ptr[i] = i*1000;

    delete ptr;

    std::cout << "done" << std::endl;

    return 0;
}
