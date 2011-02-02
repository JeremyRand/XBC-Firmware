#ifndef CSINGLETON_H
#define CSINGLETON_H

#include <cassert>

// NOTE:  This is not thread-safe.  
template <typename T> 
class CSingleton
{
protected:
  static T* ms_Singleton;

public:
  CSingleton( void ) {}
  ~CSingleton( void )
  {  assert( ms_Singleton );  ms_Singleton = 0;  }
  static T& Instance( void ) {
    return (*InstancePtr());
  }
  static T* InstancePtr( void ) {  
    if(!ms_Singleton) {
      ms_Singleton = new T();
    }
    assert( ms_Singleton );  
    return ( ms_Singleton );  
  }
};

template <typename T> T* CSingleton <T>::ms_Singleton = 0;

#endif
