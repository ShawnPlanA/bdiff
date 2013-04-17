#ifndef __BDIFF_H
#define __BDIFF_H

#define LINE_LENGTH     16

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long int64;

/*
 *  * min()/max()/clamp() macros that also do
 *   * strict type-checking.. See the
 *    * "unnecessary" pointer comparison.
 *     */
#define min(x, y) ({                            \
        typeof(x) _min1 = (x);                  \
        typeof(y) _min2 = (y);                  \
        (void) (&_min1 == &_min2);              \
        _min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({                            \
        typeof(x) _max1 = (x);                  \
        typeof(y) _max2 = (y);                  \
        (void) (&_max1 == &_max2);              \
        _max1 > _max2 ? _max1 : _max2; })

#define alignment(num, median) ({		\
	typeof(num) _num = (num);		\
	typeof(median) _median = (median);	\
	(void) (&_num == &_median);		\
	(_num + _median - 1) / _median * _median; })

#endif
