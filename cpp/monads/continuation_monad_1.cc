#define _GLIBCXX_USE_NANOSLEEP 1

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

using namespace std;


//-----------------------------------------------------------------------------

void asyncApi( function< void ( string )> handler )
{
    thread th([handler]()
    {
        cout << "Started async\n";
//        std::this_thread::sleep_for(chrono::seconds(3));
        handler("Done async");
    });
    th.detach();
}

//-----------------------------------------------------------------------------

template<class R, class A>
struct Continuator {
    virtual ~Continuator() {}
    virtual R andThen( function< R (A) > k ) {}
};

//-----------------------------------------------------------------------------

struct AsyncApi : Continuator<void, string >
{
    void andThen( function< void( string ) > k )
    {
        asyncApi(k);
    }
};

void print( string s )
{
    cout << s << endl;
}

int main()
{
    AsyncApi callApi;
    callApi.andThen( print );
}
