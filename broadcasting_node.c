
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

// hacky workaround for c pointer stuff. Should be same as current_channel.
static unsigned int msg_buffer = 11;

static unsigned int ping = 99;
static unsigned int ack_ping = 98;
static unsigned int current_channel = 11;
static unsigned int msg_timeout_timer = 10;
static unsigned int channel_map[] = {11, 13, 16, 12, 17, 20, 26};
static unsigned int channel_count = 0;
// Search channels and on first join to find the right one channels are
// currently communicating on.
static bool search_channels = true;
static bool recv_ack_ping = false;

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
#endif /* MAC_CONF_WITH_TSCH */

/*---------------------------------------------------------------------------*/
// void printFloat(char* string,float F);
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
  unsigned int msg = current_channel;
  nullnet_buf = (uint8_t *)&msg_buffer;
  nullnet_len = sizeof(msg_buffer);
  nullnet_set_input_callback(input_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    // Search on all channels to find active motes
    if(search_channels || msg_timeout_timer == 0) {
      LOG_INFO("Messages until timeout/channel switch %u \n", msg_timeout_timer);
      unsigned int tmp_channel = current_channel;
      if(msg_timeout_timer <= 0) {
        msg_timeout_timer = 10;
        if(channel_count < 6) {
          channel_count += 1;
        } else {
          channel_count = 0;
        }
        current_channel = channel_map[channel_count];
      }
      if(tmp_channel != current_channel){
        LOG_INFO("Switching channel to %u since no new messages where received \n", current_channel);
        cc2420_set_channel(current_channel);
        search_channels = true;
      }
    }

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    if(recv_ack_ping) {
      msg = ack_ping;
      recv_ack_ping = false;
    } else if(search_channels) {
      msg = ping;
    } else {
      msg = current_channel;
    }
    LOG_INFO("Sending %u to ", msg);
    LOG_INFO_LLADDR(NULL);
    LOG_INFO_("\n");

    memcpy(nullnet_buf, &msg, sizeof(msg));
    nullnet_len = sizeof(msg);

    NETSTACK_NETWORK.output(NULL);

    msg_timeout_timer -= 1;
    etimer_reset(&periodic_timer);
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len, const linkaddr_t *src, const linkaddr_t *dest) {
  if(len == sizeof(unsigned)) {
    unsigned int recv_channel;
    memcpy(&recv_channel, data, sizeof(recv_channel));
    if (recv_channel == ping) {
      LOG_INFO("Ping received, ack sending ping back\n");
      recv_ack_ping = true;
      search_channels = false;
    } else if(recv_channel == ack_ping) {
      LOG_INFO("Ack ping received!\n");
      search_channels = false;
    } else {
      LOG_INFO("Current incoming MSG (Channel Nr) %u from ", recv_channel);
      LOG_INFO_LLADDR(src);
      LOG_INFO_("\n");
      msg_timeout_timer = 10;
    }
  }
}
/*---------------------------------------------------------------------------*/
