
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
static unsigned int current_channel = 20;
static unsigned int updated_channel = 11;
static unsigned int msg_timeout_timer1 = 10;
static unsigned int msg_timeout_timer2 = 10;
static unsigned int channel_map[] = {11, 13, 16, 12, 17, 20, 26};
static unsigned int channel_count = 0;
// Search channels and on first join to find the right one channels are
// currently communicating on.
static bool search_channels = true;
static unsigned int update_channel_count = 0;


static linkaddr_t mote1_src = {{0x51, 0xf6, 0x6e, 0x14, 0x00, 0x74, 0x12, 0x00}}; // "51f6.6e14.0074.1200";
// static linkaddr_t mote2_src = {{0xb9, 0xce, 0x6e, 0x14, 0x00, 0x74, 0x12, 0x00}};
static linkaddr_t mote2_src = {{0x46, 0x95, 0x08, 0x15, 0x00, 0x74, 0x12, 0x00}};
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

  msg = current_channel;
  while(1) {
    // Search on all channels to find active motes
    if(search_channels || msg_timeout_timer1 <= 0 || msg_timeout_timer2 <= 0) {
      LOG_INFO("Messages until timeout/channel switch %u \n", msg_timeout_timer1);
      unsigned int tmp_channel = current_channel;
      if(msg_timeout_timer1 <= 0 || msg_timeout_timer2 <= 0){
        msg_timeout_timer1 = 10;
        msg_timeout_timer2 = 10;
        if(channel_count < 6) {
          channel_count += 1;
        } else {
          channel_count = 0;
        }
       current_channel = channel_map[channel_count];
      }
      if(tmp_channel != current_channel){
        LOG_INFO("Advertising new channel(%u) since no new messages where received \n", current_channel);
        // cc2420_set_channel(current_channel);
        // search_channels = true;
        // msg = current_channel;
        update_channel_count = 3;
      }
    }

    if(update_channel_count <= 3) {
      msg = current_channel;
      update_channel_count -= 1;
    }

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    LOG_INFO("Sending %u to ", msg);
    LOG_INFO_LLADDR(NULL);
    LOG_INFO_("\n");


    memcpy(nullnet_buf, &msg, sizeof(msg));
    nullnet_len = sizeof(msg);

    NETSTACK_NETWORK.output(NULL);

    msg_timeout_timer1 -= 1;
    msg_timeout_timer2 -= 1;
    etimer_reset(&periodic_timer);

    if(update_channel_count <= 0){
      cc2420_set_channel(current_channel);
    }
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len, const linkaddr_t *src, const linkaddr_t *dest) {
  if(len == sizeof(unsigned)) {
    unsigned int recv_channel;
    memcpy(&recv_channel, data, sizeof(recv_channel));
    search_channels = false;
    LOG_INFO("Current incoming MSG (Channel Nr:) %u from ", recv_channel);
    // if(recv_channel != current_channel){
    //   updated_channel = recv_channel;
    //   LOG_INFO("Incoming channel differs from current channel!\n");
    //   LOG_INFO("Current channel: %u", current_channel);
    //   LOG_INFO("Incoming channel: %u", recv_channel);
    // }
    LOG_INFO_LLADDR(src);


    LOG_INFO_("\n");
    if(linkaddr_cmp(src, &mote1_src)){
      msg_timeout_timer1 = 10;
    }
    if(linkaddr_cmp(src, &mote2_src)) {
      msg_timeout_timer2 = 10;
    }
  }
}
/*---------------------------------------------------------------------------*/
