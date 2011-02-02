#ifndef HSV_H
#define HSV_H

// How to tell if a value is in a range which wraps
// Cases:
//    Ordering           In?   
//    val   min---max    0    min<=max:1  min<=val:0  val<=max:1
//    min---val---max    1    min<=max:1  min<=val:1  val<=max:1
//    min---max   val    0    min<=max:1  min<=val:1  val<=max:0
// ---val---max   min--- 1    min<=max:0  min<=val:0  val<=max:1
// ---max   val   min--- 0    min<=max:0  min<=val:0  val<=max:0
// ---max   min---val--- 1    min<=max:0  min<=val:1  val<=max:0
// So you can use (min<=max) ^ (min<=val) ^ (val<=max)

inline bool in_hsv_cube(int h, unsigned char s, unsigned char v,
                        int min_h, int max_h,
                        int min_s, int max_s,
                        int min_v, int max_v)
{
  // Hue is from 0 to 360, but wraps.  See above
  return ((min_h <= max_h) ^ (min_h <= h) ^ (h <= max_h)) &&
    min_s <= s && s <= max_s &&
    min_v <= v && v <= max_v;
}

// h from 0 to 360
// s from 0 to 255
// v from 0 to 255
void hsv_to_rgb(unsigned short h, unsigned char s,  unsigned char v,
                unsigned char &r, unsigned char &g, unsigned char &b);

// Hue from 0 to 360
// 0:   red
// 60:  yellow
// 120: green
// 180: cyan
// 240: blue
// 300: magenta

void rgb_to_hsv(unsigned char r, unsigned char g, unsigned char b,
                unsigned short &h, unsigned char &s, unsigned char &v);


#endif
