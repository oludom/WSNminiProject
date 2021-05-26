
#include "contiki.h"
#include "shared.h"
#include <sys/node-id.h>
#include "random.h"

#include "sys/log.h"
#define LOG_MODULE "seq_csma"
#define LOG_LEVEL LOG_LEVEL_DBG

#define CHANNEL_SEARCH_COUNT 3
#define CHANNEL_SEARCH_STOP 15
#define CHANNEL_SEQUENCE_COUNT 7
#define CURRENT_COUNT msg_counter
#define MESSAGE_SEPARATOR ((uint16_t) 0xBEEF)

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

  // init random number generator for channel search
  random_init(node_id);

  while(1) {
    // Search on all channels to find active motes
    if(search_channels <= 0) {

        search_channels = random_rand() % 10 + 2;
        increase_channel_index();

        LOG_INFO("Switching channel to %u since no new messages where received \n", CURRENT_CHANNEL);
        cc2420_set_channel(CURRENT_CHANNEL);
        msg_counter = CHANNEL_SEQUENCE_COUNT;
        
    }else {
        // if message counter is low switch to next channel in channel map and reset counter
        if(msg_counter <= 0) {
            increase_channel_index();

            LOG_INFO("Switching channel to %u \n", CURRENT_CHANNEL);
            cc2420_set_channel(CURRENT_CHANNEL);
            msg_counter = CHANNEL_SEQUENCE_COUNT;
        }
    }

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    // send current counter and channel as broadcast message
    nullnet_sendcurrentcounter();

    // decrement message counter
    msg_counter -= 1;

    // decrement search counter if channel search is active
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
    // check message length
    // LOG_INFO("received\n");
    if(len == sizeof(message_format_t)) {
        unsigned int recv_counter;
        unsigned int recv_channel; 
        message_format_t msg_ptr;
        memcpy(&msg_ptr, data, len);
        // LOG_INFO("right size\n");
        // check if message contains separator
        if(msg_ptr.separator == MESSAGE_SEPARATOR){
            recv_channel = msg_ptr.channel;
            recv_counter = msg_ptr.count;
            LOG_INFO("Current incoming MSG (counter) %u on channel %d from", recv_counter, recv_channel);
            LOG_INFO_LLADDR(src);
            LOG_INFO_("\n");

            // stop searching channels
            if(search_channels < CHANNEL_SEARCH_STOP)
                search_channels = CHANNEL_SEARCH_STOP;

            // if incoming message on current channel contains a lower counter, change own counter to that value
            if(recv_counter < CURRENT_COUNT && recv_channel == CURRENT_CHANNEL){
                LOG_INFO("Incoming counter (different): %u\n", recv_counter);
                CURRENT_COUNT = recv_counter;
            }

            CURRENT_COUNT = CHANNEL_SEQUENCE_COUNT;
        }
    }
}
/*---------------------------------------------------------------------------*/

/*
send current counter value and current channel as broadcast message over nullnet
*/
void nullnet_sendcurrentcounter(void){
    message_format_t temp_message;

    LOG_INFO("sending counter %d on channel %d \n", CURRENT_COUNT, cc2420_get_channel());
    
    // write to nullnet buffer
    temp_message.channel = CURRENT_CHANNEL;
    temp_message.separator = MESSAGE_SEPARATOR;
    temp_message.count = CURRENT_COUNT;

    memcpy((void*)nullnet_buf, &temp_message, sizeof(temp_message));

    // set length to message struct
    nullnet_len = sizeof(message_format_t);
    NETSTACK_NETWORK.output(NULL);
}
