#include "contiki.h"
#include "dev/leds.h"


PROCESS(empty, "empty");
AUTOSTART_PROCESSES(&empty);

#define NUMBER_CHANNELS 16

PROCESS_THREAD(empty, ev, data)
{
    static struct etimer timer;

    PROCESS_BEGIN();
    etimer_set(&timer, (CLOCK_SECOND));

    while(1) {

        // wait for timer
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        etimer_reset(&timer);

        leds_toggle(LEDS_GREEN);

    }

    PROCESS_END();
}
