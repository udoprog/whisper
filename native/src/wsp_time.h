#ifndef _WSP_TIME_H_
#define _WSP_TIME_H_

#include <stdint.h>

typedef uint32_t wsp_time_t;

wsp_time_t wsp_time(void);
wsp_time_t wsp_time_floor(wsp_time_t base, wsp_time_t interval);
wsp_time_t wsp_time_from_timestamp(uint32_t timestamp);

#endif /* _WSP_TIME_H_ */
