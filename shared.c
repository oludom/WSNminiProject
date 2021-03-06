
#include "shared.h"


unsigned int msg_buffer[3] = {0};

const unsigned int channel_map[] = {11, 24, 18, 22, 13, 16, 12, 19, 23, 17, 20, 15, 26, 14, 21, 25};
unsigned int current_channel_index = 0;

const linkaddr_t master_src = {{0x2f, 0x96, 0x08, 0x15, 0x00, 0x74, 0x12, 0x00}}; // "2f96.0815.0074.1200" Master node (Balazs);
const linkaddr_t mote3_src = {{0x51, 0xf6, 0x6e, 0x14, 0x00, 0x74, 0x12, 0x00}}; // "51f6.6e14.0074.1200" Jonas;
// static linkaddr_t mote2_src = {{0xb9, 0xce, 0x6e, 0x14, 0x00, 0x74, 0x12, 0x00}};
const linkaddr_t mote1_src = {{0x46, 0x95, 0x08, 0x15, 0x00, 0x74, 0x12, 0x00}}; //Peter
const linkaddr_t mote2_src = {{0xea, 0x9a, 0x08, 0x15, 0x00, 0x74, 0x12, 0x00}}; //Marta

/* Initialize NullNet */
void nullnet_init(void){
    nullnet_buf = (uint8_t *)&msg_buffer[0];
    nullnet_len = sizeof(msg_buffer[0]);
    nullnet_set_input_callback(input_callback);
}

void nullnet_sendcurrentchannel(void){
    memcpy(nullnet_buf, &CURRENT_CHANNEL, sizeof(CURRENT_CHANNEL));
    nullnet_len = sizeof(CURRENT_CHANNEL);
    NETSTACK_NETWORK.output(NULL);
    // printf(".\n");
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