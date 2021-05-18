#include "contiki.h"

#include "os/net/netstack.h"
#include "dev/radio/cc2420/cc2420.h"
#include "dev/radio/cc2420/cc2420_const.h"
#include "sys/rtimer.h"
#include "dev/leds.h"

#include "net/nullnet/nullnet.h"
#include <stdio.h>
#include <string.h>

#include "sys/log.h"
#define LOG_MODULE "broadcasting_node"
#define LOG_LEVEL LOG_LEVEL_DBG

static struct rtimer rtimer; // Create the rtimer variable

PROCESS(constant_jammer, "jammer");
AUTOSTART_PROCESSES(&constant_jammer);

#define NUMBER_CHANNELS 16
#define CONSTANT_MICROS 300 // 300 us was defined by the paper [1]
#define TIME_TICK 31        // The time of 1 tick is 30,51 us in rtimer for sky motes in Contiki v3.0. TIME_TICK = 1 / RTIMER_ARCH_SECOND = 1 / 32768 s


static void timer_callback(struct rtimer *timer, void *ptr)
{
    uint32_t num_ticks; // Number of ticks of the time_next_period

    leds_toggle(LEDS_GREEN);

    // carrier frequency strobe
    NETSTACK_RADIO.set_value(RADIO_PARAM_POWER_MODE, RADIO_POWER_MODE_CARRIER_ON);
    printf("carrier on.\n");

    // Set the rtimer to the time_next_period (num_ticks) by compute the next time period according to the paper [1]
    num_ticks = 10 * CONSTANT_MICROS / TIME_TICK;

    rtimer_set(&rtimer, RTIMER_NOW() + num_ticks, 1, timer_callback, NULL); // Set the rtimer again to the time_next_period (num_ticks)
}

PROCESS_THREAD(constant_jammer, ev, data)
{

    static int channelNumber = 20;
    PROCESS_BEGIN();
    NETSTACK_RADIO.init();

    NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, -25);

    // set channel
    NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, (radio_value_t) channelNumber);
    rtimer_set(&rtimer, RTIMER_NOW() + RTIMER_ARCH_SECOND, 1, timer_callback, NULL); //Initiates the rtimer 1 second after boot
    PROCESS_END();
}
