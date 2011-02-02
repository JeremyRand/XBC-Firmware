#ifndef _MATHUTILS_
#define _MATHUTILS_

// These are math utilities that are generally useful.  Unfortunately
// this sort of thing gets reimplemented so many times that there are
// problems with collisions, so this is in it's own namespace.  

namespace gba {
  template<class T>
  inline T abs(const T& x)  {return (x < 0 ? -(x) : (x));}

  template<class T>
  inline int sgn(const T& x)  {return (x < 0 ? -1 : (x > 0 ? 1 : 0));}

  template<class T>
  inline T sqr(const T& x)  {return (x*x);}

  template<class T> 
  inline const T& max(const T& a, const T& b) 
  {
    return (a > b) ? a : b;
  }

  template<class T> 
  inline const T& min(const T& a, const T& b) 
  {
    return (a < b) ? a : b;
  }

  // Returns true x is between min and max, and false otherwise
  template <class T>
  inline bool in_range(const T& x, const T& min, const T& max) { 
    return ((x)>(min) && (x)<(max))?true:false;
  }
  
};

#endif
