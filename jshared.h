#ifndef _JSHARED_H
# define  _JSHARED_H

#include "dev/radio/cc2420/cc2420.h"
#include "dev/radio/cc2420/cc2420_const.h"
#include "sys/rtimer.h"
#include "dev/leds.h"
#include "dev/spi-legacy.h"

void start_jam_loop(unsigned int sec);
void stop_jam_loop(void);

#endif