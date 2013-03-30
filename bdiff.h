#ifndef __BDIFF_H
#define __BDIFF_H

#define LINE_LENGTH     16

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
	_num / _median * _median; })
	

#endif
