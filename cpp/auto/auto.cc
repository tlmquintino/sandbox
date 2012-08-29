#include <cassert>
#include <iostream>
#include <string>
#include <vector>

struct Component
{
  Component( const std::string& name ) : name_(name) {}
  std::string name_;
};

int main(int argc, char *argv[])
{
  std::vector< Component* > v;

  v.push_back( new Component("lolo") );
  v.push_back( new Component("popo") );

  for( auto itr = v.begin(); itr != v.end(); ++itr )
    std::cout << (*itr)->name_ << std::endl;
}

