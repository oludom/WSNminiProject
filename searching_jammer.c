#include "contiki.h"

#include "os/net/netstack.h"
#include "dev/leds.h"

#include <stdio.h>

PROCESS(sweeping_jammer, "jammer");
AUTOSTART_PROCESSES(&sweeping_jammer);

#define NUMBER_CHANNELS 8

int avg(int values[], int length){
  int i, retVal = 0; 
  for (i = 0; i<length; i++){
    retVal += values[i];
  }
  return (int)(retVal/length);
}


PROCESS_THREAD(sweeping_jammer, ev, data)
{

    static struct etimer timer;
    static int channelNumber = 11;
    static int value = 0;
    static int rssivalues[NUMBER_CHANNELS][4] = {{0}};
    static int i = 0;
    static int j = 0;
    static int minChannelValue = -100, minChannelNumber = 11;

    PROCESS_BEGIN();
    NETSTACK_RADIO.init();
    etimer_set(&timer, (32)); // 1/4s time interval


    // set channel
    NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, (radio_value_t) channelNumber);

    while(1) {

        // wait for timer
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        etimer_reset(&timer);

        // NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &value);

        leds_toggle(LEDS_GREEN);

        if(i<32){
            // sense
            NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &value);

            printf("channel: %d; rssi param: %d\n", channelNumber, (int) value);
            rssivalues[(int)((channelNumber-11)/2)][i%4] = (int)value;

            j++;
            if(j>3){
                
                // calc average
                int a = avg(rssivalues[(int)((channelNumber-11)/2)], 4);
                if (a > minChannelValue){
                    minChannelValue = a;
                    minChannelNumber = channelNumber;
                    printf("average: %d; channel: %d\n", a, minChannelNumber);
                }

                channelNumber +=2;
                j = 0;
                // set channel
                NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, (radio_value_t) channelNumber);
            }

        }
        else if (i==32){
            // set channel
            printf("set channel to %d\n", minChannelNumber);
            NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, (radio_value_t) minChannelNumber);
            minChannelNumber = 11;
            minChannelValue = -100;

        } else {

            // jam 

            NETSTACK_RADIO.set_value(RADIO_PARAM_POWER_MODE, RADIO_POWER_MODE_CARRIER_ON);
            printf("carrier on.\n");
        }
        i++;
        if(i>=272){
            i = 0;
            channelNumber = 11;
        }

    }

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
