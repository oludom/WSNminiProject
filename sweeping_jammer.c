
#include "sys/log.h"
#define LOG_MODULE "sweeping jammer"
#define LOG_LEVEL LOG_LEVEL_DBG

#include "contiki.h"
#include "shared.h"

#include "os/net/netstack.h"
#include "dev/leds.h"

#include <stdio.h>

PROCESS(sweeping_jammer, "jammer");
AUTOSTART_PROCESSES(&sweeping_jammer);

#define NUMBER_CHANNELS 16

PROCESS_THREAD(sweeping_jammer, ev, data)
{

    static struct etimer timer;
    static int txpower = -25;

    PROCESS_BEGIN();
    NETSTACK_RADIO.init();
    NETSTACK_RADIO.on();

    // set channel power
    NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, txpower);

    etimer_set(&timer, CLOCK_SECOND * 1);

    while(1) {
        NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, CURRENT_CHANNEL);
        leds_toggle(LEDS_GREEN);

        LOG_INFO("Jamming channel %d\n",CURRENT_CHANNEL);
        while(!etimer_expired(&timer))
        {
            // carrier frequency strobe
            NETSTACK_RADIO.set_value(RADIO_PARAM_POWER_MODE, RADIO_POWER_MODE_CARRIER_ON);
            PROCESS_PAUSE();
        }

        increase_channel_index();
        etimer_reset(&timer);
    }

    PROCESS_END();
}
