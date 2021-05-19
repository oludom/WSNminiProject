
#include "contiki.h"
#include "shared.h"

#include "sys/log.h"
#define LOG_MODULE "bcast_slave"
#define LOG_LEVEL LOG_LEVEL_DBG


static unsigned int msg_error_counter = MAX_MESSAGE_ERROR_COUNT;
static bool search_channels = true;

static unsigned int update_channel_count = 0;
static unsigned int message_count_master = 0;
static unsigned int message_count_slave = 0;


PROCESS(broadcasting_node_process, "broadcasting process");
AUTOSTART_PROCESSES(&broadcasting_node_process);

PROCESS_THREAD(broadcasting_node_process, ev, data)
{
  static struct etimer periodic_timer;

  // start process
  PROCESS_BEGIN();
  // set timer to defined interval
  etimer_set(&periodic_timer, SEND_INTERVAL);
  // set channel to initial value
  cc2420_set_channel(CURRENT_CHANNEL);
  // initialize nullnet
  nullnet_init();


  while(1) {
    // Search on all channels to find active motes
    if(search_channels || msg_error_counter <= 0) {
      // LOG_INFO("Messages until timeout/channel switch %u \n", msg_error_counter);
      if(msg_error_counter <= 0) {
        msg_error_counter = MAX_MESSAGE_ERROR_COUNT;
        increase_channel_index();

        LOG_INFO("Switching channel to %u since no new messages where received \n", CURRENT_CHANNEL);
        cc2420_set_channel(CURRENT_CHANNEL);
      }
      msg_error_counter -= 1;
    }

    if(update_channel_count > 0 && update_channel_count <= 3) {
      update_channel_count -= 1;
    }else 
    // if countdown to channel switch is over
    if (update_channel_count <= 0){
      LOG_INFO("Switching channel to advertised number: %u.\n", CURRENT_CHANNEL);
      cc2420_set_channel(CURRENT_CHANNEL);
      // stop channel change
      update_channel_count = 5;
    }

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    // LOG_INFO("Sending current channel '%u'", CURRENT_CHANNEL);
    // LOG_INFO_(" on channel %u\n", cc2420_get_channel());

    nullnet_sendcurrentchannel();

    // decrement error counter
    msg_error_counter -= 1;

    // reset timer
    etimer_reset(&periodic_timer);

  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len, const linkaddr_t *src, const linkaddr_t *dest) {
  if(len == sizeof(unsigned)) {
    unsigned int recv_channel;
    memcpy(&recv_channel, data, sizeof(recv_channel));
    // LOG_INFO("Current incoming MSG (Channel Nr:) %u from", recv_channel);
    // LOG_INFO_LLADDR(src);
    // LOG_INFO_("\n");

    if(recv_channel != CURRENT_CHANNEL) {
      // LOG_INFO("Incoming channel differs from current channel!\n");
      LOG_INFO("Incoming channel (different): %u\n", recv_channel);
      set_current_channel(recv_channel);
      update_channel_count = 3;
    }
    if(linkaddr_cmp(src, &master_src)){
      if(message_count_master >= 10) {
        message_count_master = 0;
        LOG_INFO("10 messages received from the Master {");
        LOG_INFO_LLADDR(src);
        LOG_INFO_("}\n");
      }
      message_count_master++;
    }
    else {
      if(message_count_slave >= 10) {
        message_count_slave = 0;
        LOG_INFO("10 messages received from slave: {");
        LOG_INFO_LLADDR(src);
        LOG_INFO_("}\n");
      }
      message_count_slave++;
    }
    msg_error_counter = 10;

  }
}
/*---------------------------------------------------------------------------*/
