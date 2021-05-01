
#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

//include the radio (CC2420 driver)
#include "cc2420.h"
#include "cc2420_const.h"

#include <math.h>
#include <string.h>

#include "sys/log.h"
#define LOG_MODULE "broadcasting_node"
#define LOG_LEVEL LOG_LEVEL_DBG

/*---------------------------------------------------------------------------*/
#define SEND_INTERVAL (4 * CLOCK_SECOND)

static unsigned int current_channel = 11;
static unsigned int msg_timeout_timer = 10;
static unsigned int channel_map[] = {13, 15, 12, 17, 20, 26};
static unsigned int channel_count = 0;
static bool was_ever_connected = false;

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
#endif /* MAC_CONF_WITH_TSCH */

/*---------------------------------------------------------------------------*/
void printFloat(char* string,float F);
void input_callback(const void *data, uint16_t len, const linkaddr_t *src, const linkaddr_t *dest);

/*---------------------------------------------------------------------------*/
PROCESS(broadcasting_node_process, "broadcasting process");
AUTOSTART_PROCESSES(&broadcasting_node_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(broadcasting_node_process, ev, data)
{
  static struct etimer periodic_timer;
  // static unsigned int count = 0;

  PROCESS_BEGIN();

  cc2420_set_channel(current_channel);

  #if MAC_CONF_WITH_TSCH
  tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
  #endif /* MAC_CONF_WITH_TSCH */

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&current_channel;
  nullnet_len = sizeof(current_channel);
  nullnet_set_input_callback(input_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1)
  {
    if((msg_timeout_timer <= 0) && (was_ever_connected)){
      if(channel_count <= 6){
        channel_count += 1;
      } else {
        channel_count = 0;
      }
      LOG_INFO("Switching channel to %u since no new messages received \n", channel_map[channel_count]);
      current_channel = channel_map[channel_count];
      cc2420_set_channel(current_channel);
      msg_timeout_timer = 10;
    }
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    LOG_INFO("Sending %u to ", current_channel);
    LOG_INFO_LLADDR(NULL);
    LOG_INFO_("\n");

    memcpy(nullnet_buf, &current_channel, sizeof(current_channel));
    nullnet_len = sizeof(current_channel);

    NETSTACK_NETWORK.output(NULL);

    msg_timeout_timer -= 1;
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
void printFloat(char* string,float F)
{
  unsigned long A;
  float frac;

  A=F;
  frac=(F-A)*pow(10,6);
  LOG_INFO("%s%lu.%06lu\n",string,A,(unsigned long)frac);
}

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len, const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(unsigned))
  {
    unsigned recv_channel;
    memcpy(&recv_channel, data, sizeof(recv_channel));
    // THIS IS FOR AUTOMATICALLY SWITCHING TOWARDS A NEWLY SEND CHANNEL MESSAGE
    if(recv_channel != current_channel){
      LOG_INFO("Channel not matching anymore: %u ", recv_channel);
      // if(channel_count <= 6){
      //   channel_count += 1;
      // } else {
      //   channel_count = 0;
      // }
      // current_channel = channel_map[channel_count];
      LOG_INFO_LLADDR(src);
      LOG_INFO_("\n");
    } else {
      LOG_INFO("Current Channel %u from ", recv_channel);
      LOG_INFO_LLADDR(src);
      LOG_INFO_("\n");
      msg_timeout_timer = 10;
      was_ever_connected = true;
    }
  }
}
