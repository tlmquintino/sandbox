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

template< typename R, typename A >
struct Continuator {
    virtual ~Continuator() {}
    virtual R andThen( function< R (A) > k ) {}
};

//-----------------------------------------------------------------------------

void asyncApi( function< void ( string )> handler )
{
    thread th([handler]()
    {
        cout << "Started async\n";
        std::this_thread::sleep_for(chrono::seconds(3));
        handler("Done async");
    });
    th.detach();
}

struct AsyncApi : Continuator<void, string> {
    void andThen(function<void(string)> k)
    {
       asyncApi(k);
    }
};

//-----------------------------------------------------------------------------

template< typename R, typename A >
struct Return : Continuator<R,A> {
    Return(A x) : _x(x) {}
    R andThen( function<R(A)> k) {
        return k(_x);
    }
    A _x;
};

//-----------------------------------------------------------------------------

template< typename R, typename A, typename C>
struct Bind : Continuator<R,A>
{
    Bind( C ktor, function< unique_ptr< Continuator<R,A> > ( A ) > rest ) :
        ktor_(ktor),
        rest_(rest)
    {}

    void andThen( function<R(A)> k )
    {
        function< unique_ptr< Continuator<R,A> > ( A ) > rest = rest_;
        function<R(A)> lambda = [k, rest](A a)
        {
            return rest(a)->andThen(k);
        };
        ktor_.andThen(lambda);
    }

    C ktor_;
    function< unique_ptr< Continuator<R,A> > ( A ) > rest_;
};

//-----------------------------------------------------------------------------

struct Loop : Continuator<void,string>
{
    Loop( string s ) : s_(s) {}

    void andThen( function< void(string)> k )
    {
        cout << "Loop::andThen: " << s_ << endl;
        Bind< void, string, AsyncApi >( AsyncApi(), [](string t)
        {
            return unique_ptr< Continuator<void,string> >( new Loop(t) );
        }).andThen(k);
    }
    string s_;
};

//-----------------------------------------------------------------------------

struct LoopN : Continuator<void, string>
{
    LoopN(string s, int n) : s_(s), n_(n) {}

    void andThen(function<void(string)> k)
    {
        cout << "[LoopN::andThen] " <<s_ << " " << n_ << endl;
        int n = n_;
        Bind<void, string, AsyncApi>(AsyncApi(),
        [n](string s) -> unique_ptr<Continuator>
        {
            if (n > 0)
                return unique_ptr<Continuator>(
                    new LoopN(s, n - 1));
            else
                return unique_ptr<Continuator> (
                    new Return<void, string>("Done!"));
        }).andThen(k);
    }
    string s_;
    int    n_;
};

//-----------------------------------------------------------------------------

    struct And : Continuator<void, string>
    {
        And(unique_ptr<Continuator<void,string> > & ktor1,
            unique_ptr<Continuator<void,string> > & ktor2)
            : _ktor1(move(ktor1)),
              _ktor2(move(ktor2))
        {}

        void andThen(function<void(pair<string, string>)> k)
        {
            _ktor1->andThen([this, k](string s1)
            {
                            lock_guard<mutex> l(_mtx);
                    if (_s2.empty())
                    _s1 = s1;
            else
            k(make_pair(s1, _s2));
        });
        _ktor2->andThen([this, k](string s2)
        {
                        lock_guard<mutex> l(_mtx);
        if (_s1.empty())
        _s2 = s2;
        else
        k(make_pair(_s1, s2));
    });
    }
    mutex _mtx;
    string _s1;
    string _s2;
    unique_ptr< Continuator<void,string> > _ktor1;
    unique_ptr< Continuator<void,string> > _ktor2;
    };

//-----------------------------------------------------------------------------

//    void main()
//    {
//        LoopN("Begin ", 3).andThen([](string s)
//        {
//            cout << "Finally " << s << endl;
//        });
//        for(int i = 0; i < 15; ++i)
//        {
//            cout << i << endl;
//            this_thread::sleep_for(chrono::seconds(1));
//        }
//    }

int main()
{
    And both (
                (new LoopN("Begin 1 ", 3))->andThen([](string s) { cout << "Finally 1 " << s << endl; }),
                (new LoopN("Begin 2 ", 4))->andThen([](string s) { cout << "Finally 2 " << s << endl; }) );

    for(int i = 0; i < 15; ++i)
    {
        cout << i << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
}
