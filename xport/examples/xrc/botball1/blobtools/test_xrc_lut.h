#ifndef TEST_XRC_LUT_H
#define TEST_XRC_LUT_H

void image_threshold_hsv_cube_xrc(const Image &src, Image &dest,
                                  int min_h, int max_h,
                                  int min_s, int max_s,
                                  int min_v, int max_v);
#endif
