


#ifndef _SHARED_H
# define  _SHARED_H

#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

//include the radio (CC2420 driver)
#include "cc2420.h"
#include "cc2420_const.h"

#include <math.h>
#include <string.h>

#define SEND_INTERVAL (2)
#define CURRENT_CHANNEL channel_map[current_channel_index]
#define MAX_CHANNEL_INDEX 6
#define MAX_MESSAGE_ERROR_COUNT 10

extern unsigned int msg_buffer;
extern unsigned int updated_channel;

extern const unsigned int channel_map[];
extern unsigned int current_channel_index;

extern const linkaddr_t mote1_src;
extern const linkaddr_t mote2_src;


void input_callback(const void *data, uint16_t len, const linkaddr_t *src, const linkaddr_t *dest);

void nullnet_init(void);
void nullnet_sendcurrentchannel(void);

void increase_channel_index(void);
void set_current_channel(unsigned int channel);

#endif