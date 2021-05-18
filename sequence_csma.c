
#include "contiki.h"
#include "shared.h"

#include "sys/log.h"
#define LOG_MODULE "seq_csma"
#define LOG_LEVEL LOG_LEVEL_DBG

#define CHANNEL_SEARCH_COUNT 3
#define CHANNEL_SEARCH_STOP 10
#define CHANNEL_SEQUENCE_COUNT 7
#define CURRENT_COUNT msg_counter
#define MESSAGE_SEPARATOR ((uint16_t)0xBEEF)

static unsigned int msg_counter = CHANNEL_SEQUENCE_COUNT;
static unsigned int search_channels = CHANNEL_SEARCH_COUNT;

typedef struct message_format_struct{
    uint16_t channel;
    uint16_t separator;
    uint16_t count;
} message_format_t;


PROCESS(sequence_process, "sequence_csma");
AUTOSTART_PROCESSES(&sequence_process);

void nullnet_sendcurrentcounter(void);

PROCESS_THREAD(sequence_process, ev, data)
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
    if(search_channels <= 0) {

        search_channels = CHANNEL_SEARCH_COUNT;
        increase_channel_index();

        LOG_INFO("Switching channel to %u since no new messages where received \n", CURRENT_CHANNEL);
        cc2420_set_channel(CURRENT_CHANNEL);
        
    }else {
        if(msg_counter <= 0) {
            increase_channel_index();

            LOG_INFO("Switching channel to %u \n", CURRENT_CHANNEL);
            cc2420_set_channel(CURRENT_CHANNEL);
            msg_counter = CHANNEL_SEQUENCE_COUNT;
        }
    }

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    // LOG_INFO("Sending current channel '%u'", CURRENT_CHANNEL);
    // LOG_INFO_(" on channel %u\n", cc2420_get_channel());

    nullnet_sendcurrentcounter();

    // decrement message counter
    msg_counter -= 1;

    // decrement search counter
    if(search_channels < CHANNEL_SEARCH_STOP){
        search_channels--;
    }

    // reset timer
    etimer_reset(&periodic_timer);

  }
  PROCESS_END();

}

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len, const linkaddr_t *src, const linkaddr_t *dest) {
    // LOG_INFO("input callback len: %d\n", len);
  if(len == sizeof(message_format_t)) {
    unsigned int recv_counter;
    unsigned int recv_channel; 
    message_format_t * msg_ptr = (message_format_t*)data;
    // LOG_INFO("input callback correct length\n");
    if(msg_ptr->separator == MESSAGE_SEPARATOR){
        recv_channel = msg_ptr->channel;
        recv_counter = msg_ptr->count;
        LOG_INFO("Current incoming MSG (counter) %u on channel %d from", recv_counter, recv_channel);
        LOG_INFO_LLADDR(src);
        LOG_INFO_("\n");

        // stop searching channels
        if(search_channels < CHANNEL_SEARCH_STOP)
            search_channels = CHANNEL_SEARCH_STOP;

        if(recv_counter < CURRENT_COUNT && recv_channel == CURRENT_CHANNEL){
            LOG_INFO("Incoming counter (different): %u\n", recv_counter);
            CURRENT_COUNT = recv_counter;
        }
    }

  }
}
/*---------------------------------------------------------------------------*/


void nullnet_sendcurrentcounter(void){
    message_format_t * temp_message = (message_format_t*) nullnet_buf;
    LOG_INFO("sending counter %d on channel %d \n", CURRENT_COUNT, cc2420_get_channel());
    temp_message->channel = CURRENT_CHANNEL;
    temp_message->separator = MESSAGE_SEPARATOR;
    temp_message->count = CURRENT_COUNT;
    nullnet_len = sizeof(message_format_t);
    // LOG_INFO("msg len: %d\n", nullnet_len);
    NETSTACK_NETWORK.output(NULL);
}
