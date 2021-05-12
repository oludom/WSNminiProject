
#include "shared.h"


unsigned int msg_buffer = 11;

const unsigned int channel_map[] = {11, 13, 16, 12, 17, 20, 26};
unsigned int current_channel_index = 0;

const linkaddr_t mote1_src = {{0x51, 0xf6, 0x6e, 0x14, 0x00, 0x74, 0x12, 0x00}}; // "51f6.6e14.0074.1200";
// static linkaddr_t mote2_src = {{0xb9, 0xce, 0x6e, 0x14, 0x00, 0x74, 0x12, 0x00}};
const linkaddr_t mote2_src = {{0x46, 0x95, 0x08, 0x15, 0x00, 0x74, 0x12, 0x00}};

/* Initialize NullNet */
void nullnet_init(void){
    nullnet_buf = (uint8_t *)&msg_buffer;
    nullnet_len = sizeof(msg_buffer);
    nullnet_set_input_callback(input_callback);
}

void nullnet_sendcurrentchannel(void){
    memcpy(nullnet_buf, &CURRENT_CHANNEL, sizeof(CURRENT_CHANNEL));
    nullnet_len = sizeof(CURRENT_CHANNEL);
    NETSTACK_NETWORK.output(NULL);
}

void increase_channel_index(void){
    if(current_channel_index < MAX_CHANNEL_INDEX) {
        current_channel_index += 1;
    } else {
        current_channel_index = 0;
    }
}

void set_current_channel(unsigned int channel){

    if(channel > 11 && channel <= 26){
        unsigned int i;
        for (i = 0; i<MAX_CHANNEL_INDEX; i++){
            if(channel_map[i] == channel){
                current_channel_index = i;
                return;
            }
        }
    }
    // if channel not in channel list 
    // then set to default, index 0
    current_channel_index = 0;
}