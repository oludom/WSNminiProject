#include "contiki.h"

#include "os/net/netstack.h"
#include "jshared.h"

#include "net/nullnet/nullnet.h"

#include "sys/log.h"
#define LOG_MODULE "constjam"
#define LOG_LEVEL LOG_LEVEL_DBG

#define CHANNEL_NUMBER ((radio_value_t) 11)

PROCESS(constant_jammer, "jammer");
AUTOSTART_PROCESSES(&constant_jammer); 

PROCESS_THREAD(constant_jammer, ev, data)
{

    PROCESS_BEGIN();
    NETSTACK_RADIO.init();

    NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, 0);

    // set channel
    NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, CHANNEL_NUMBER);
    
    // start jammer loop after 1 second
    start_jam_loop(1);

    PROCESS_END();
}
