
#include "contiki.h"
#include "shared.h"

#include "sys/log.h"
#define LOG_MODULE "bcast_master"
#define LOG_LEVEL LOG_LEVEL_DBG


static unsigned int msg_error_counter1 = MAX_MESSAGE_ERROR_COUNT;
static unsigned int msg_error_counter2 = MAX_MESSAGE_ERROR_COUNT;

static unsigned int update_channel_count = 0;


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
    // if any of the two message error counters have expired
    if(msg_error_counter1 <= 0 || msg_error_counter2 <= 0) {
      // LOG_INFO("Messages until timeout/channel switch %u \n", msg_error_counter1);

      // reset timers
      msg_error_counter1 = MAX_MESSAGE_ERROR_COUNT;
      msg_error_counter2 = MAX_MESSAGE_ERROR_COUNT;
      
      // change channel
      increase_channel_index();

      LOG_INFO("Advertising new channel(%u) since no new messages where received \n", CURRENT_CHANNEL);
      update_channel_count = 3;
    }

    if(update_channel_count > 0 && update_channel_count <= 3) {
      update_channel_count -= 1;
    }else 
    // if countdown to channel switch is over
    if (update_channel_count <= 0){
      LOG_INFO("Switching channel to advertised number.\n");
      cc2420_set_channel(CURRENT_CHANNEL);
      // stop channel change
      update_channel_count = 5;
    }

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    // LOG_INFO("Sending current channel '%u'", CURRENT_CHANNEL);
    // LOG_INFO_(" on channel %u\n", cc2420_get_channel());

    nullnet_sendcurrentchannel();

    // decrement error counter
    msg_error_counter1 -= 1;
    msg_error_counter2 -= 1;

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
    // LOG_INFO("Current incoming MSG (Channel Nr:) %u from ", recv_channel);
    // LOG_INFO_LLADDR(src);
    // LOG_INFO_("\n");

    if(linkaddr_cmp(src, &mote1_src)){
      // LOG_INFO("Message from Jonas\n");
      msg_error_counter1 = 10;
    }
    if(linkaddr_cmp(src, &mote2_src)) {
      // LOG_INFO("Message from Marta\n");
      msg_error_counter2 = 10;
    }
  }
}
/*---------------------------------------------------------------------------*/
