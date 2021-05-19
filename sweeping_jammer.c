#include "contiki.h"

#include "sys/rtimer.h"
#include "dev/leds.h"
#include "shared.h"
#include "dev/spi-legacy.h"

#include "net/nullnet/nullnet.h"
#include <stdio.h>
#include <string.h>

#include "sys/log.h"
#define LOG_MODULE "broadcasting_node"
#define LOG_LEVEL LOG_LEVEL_DBG

static struct rtimer rtimer; // Create the rtimer variable
static struct rtimer rtimer2;

PROCESS(constant_jammer, "jammer");
AUTOSTART_PROCESSES(&constant_jammer);

#define NUMBER_CHANNELS 16
#define CONSTANT_MICROS 300 // 300 us was defined by the paper [1]
#define TIME_TICK 31        // The time of 1 tick is 30,51 us in rtimer for sky motes in Contiki v3.0. TIME_TICK = 1 / RTIMER_ARCH_SECOND = 1 / 32768 s


/** 
 * Writes to a register.
 * Note: the SPI_WRITE(0) seems to be needed for getting the
 * write reg working on the Z1 / MSP430X platform
 */
static void
setreg(enum cc2420_register regname, uint16_t value)
{
    CC2420_SPI_ENABLE();
    SPI_WRITE_FAST(regname);
    SPI_WRITE_FAST((uint8_t)(value >> 8));
    SPI_WRITE_FAST((uint8_t)(value & 0xff));
    SPI_WAITFORTx_ENDED();
    SPI_WRITE(0);
    CC2420_SPI_DISABLE();
}

/* Sends a strobe */
static void
strobe(enum cc2420_register regname)
{
    CC2420_SPI_ENABLE();
    SPI_WRITE(regname);
    CC2420_SPI_DISABLE();
}

static void timer_callback(struct rtimer *timer, void *ptr)
{
    uint32_t num_ticks; // Number of ticks of the time_next_period

    //The node must generate interference. Turn the carrier on.
    // Creates an unmodulated carrier by setting the appropiate registers in the CC2420
    setreg(CC2420_MANOR, 0x0100);
    setreg(CC2420_TOPTST, 0x0004);
    setreg(CC2420_MDMCTRL1, 0x0508);
    setreg(CC2420_DACTST, 0x1800);
    strobe(CC2420_STXON);

    // Set the rtimer to the time_next_period (num_ticks) by compute the next time period according to the paper [1]
    num_ticks = 10 * CONSTANT_MICROS / TIME_TICK;

    rtimer_set(&rtimer, RTIMER_NOW() + num_ticks, 1, timer_callback, NULL); // Set the rtimer again to the time_next_period (num_ticks)
}

PROCESS_THREAD(constant_jammer, ev, data)
{
    static struct etimer timer;
    static int channel, txpower = -25;
    
    static int channelNumber = 20;
    PROCESS_BEGIN();
    NETSTACK_RADIO.init();

    // NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, -25);
    etimer_set(&timer, CLOCK_SECOND*10);
    rtimer_set(&rtimer, RTIMER_NOW() + RTIMER_ARCH_SECOND, 1, timer_callback, NULL);
    while(1) {
        NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, CURRENT_CHANNEL);
        leds_toggle(LEDS_GREEN);

        LOG_INFO("Jamming channel %d\n",CURRENT_CHANNEL);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        
        // carrier frequency strobe
        increase_channel_index();
        NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, CURRENT_CHANNEL);

        etimer_reset(&timer);
    }
    
    PROCESS_END();
}
