#include "contiki.h"

#include "shared.h"

#include "jshared.h"

#include "net/nullnet/nullnet.h"
#include <stdio.h>
#include <string.h>

#include "sys/log.h"
#define LOG_MODULE "sweepjam"
#define LOG_LEVEL LOG_LEVEL_DBG


PROCESS(sweeping_jammer, "sweepjam");
AUTOSTART_PROCESSES(&sweeping_jammer);


PROCESS_THREAD(sweeping_jammer, ev, data)
{
    static struct etimer timer;
    
    PROCESS_BEGIN();
    NETSTACK_RADIO.init();

    // start jammer loop after 1 second
    start_jam_loop(1);

    etimer_set(&timer, CLOCK_SECOND*10);
    while(1) {
        NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, CURRENT_CHANNEL);
        leds_toggle(LEDS_YELLOW);

        LOG_INFO("Jamming channel %d\n",CURRENT_CHANNEL);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        
        increase_channel_index();
        NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, CURRENT_CHANNEL);

        etimer_reset(&timer);
    }
    
    PROCESS_END();
}
