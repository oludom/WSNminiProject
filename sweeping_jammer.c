#include "contiki.h"

#include "os/net/netstack.h"
#include "dev/leds.h"

#include <stdio.h>

PROCESS(sweeping_jammer, "jammer");
AUTOSTART_PROCESSES(&sweeping_jammer);

#define NUMBER_CHANNELS 16

PROCESS_THREAD(sweeping_jammer, ev, data)
{

    static struct etimer timer;
    static int channel, txpower = -25;

    PROCESS_BEGIN();
    NETSTACK_RADIO.init();
    NETSTACK_RADIO.on();

    // set channel power
    NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, txpower);

    etimer_set(&timer, CLOCK_SECOND * 15);

    while(1) {

        for(channel = 11; channel <= 26; channel++)
        {
            leds_toggle(LEDS_GREEN);
            NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel);

            while(!etimer_expired(&timer))
            {
                // carrier frequency strobe
                NETSTACK_RADIO.set_value(RADIO_PARAM_POWER_MODE, RADIO_POWER_MODE_CARRIER_ON);
                printf("carrier on.\n");
            }

            etimer_reset(&timer);
        }

    }

    PROCESS_END();
}
