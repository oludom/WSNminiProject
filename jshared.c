
#include "jshared.h"

struct rtimer rtimer;
bool jamming = false; 

/* 
COPIED FUNCTIONS FROM cc2420 driver module
*/

/*---------------------------------------------------------------------------*/
/**
 * Writes to a register.
 * Note: the SPI_WRITE(0) seems to be needed for getting the
 * write reg working on the Z1 / MSP430X platform
 */
void setreg(enum cc2420_register regname, uint16_t value){
  CC2420_SPI_ENABLE();
  SPI_WRITE_FAST(regname);
  SPI_WRITE_FAST((uint8_t) (value >> 8));
  SPI_WRITE_FAST((uint8_t) (value & 0xff));
  SPI_WAITFORTx_ENDED();
  SPI_WRITE(0);
  CC2420_SPI_DISABLE();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Sends a strobe */
void strobe(enum cc2420_register regname){
  CC2420_SPI_ENABLE();
  SPI_WRITE(regname);
  CC2420_SPI_DISABLE();
}
/*---------------------------------------------------------------------------*/

/* rtimer driven jam loop sending out a strobe from the unmodulated carrier frequency every ~.5ms */
void jam_loop(struct rtimer *timer, void *ptr){

    leds_toggle(LEDS_GREEN);

    /* 
    put transmitter into test mode 2
    unmodulated carrier strobe mode
    with I/Q DACs overridden to static values
    therefore sending out unmodulated carrier frequency
     */

    /* "manual signal OR override register", set DAC_LPF_PD to power down the TX DACs */
    setreg(CC2420_MANOR, 0x0100);
    /* "top level test register", set ATESTMOD_MODE to output I on ATEST1 and Q on ATEST2 from LPF */
    setreg(CC2420_TOPTST, 0x0004); 
    /* "modem control register 1", set TX_MODE to 2: TXFIFO looping ignores underflow in TXFIFO annd reads cyclic, infinite transmission
    other values are set to reset values, all 0 except CORR_THR[4:0] has reset value 20 */
    setreg(CC2420_MDMCTRL1, 0x0508);
    /* "DAC test register", set to 0x1800 as stated in data sheed,, transmitter test mode for unmodulated carrier */
    setreg(CC2420_DACTST, 0x1800);
    /* trigger strobe */
    strobe(CC2420_STXON);

    if (jamming)
    /* timer interval set by testing to interval lower than 1 ms, thereby shorter than message length */
        rtimer_set(&rtimer, RTIMER_NOW() + 10, 1, jam_loop, NULL); 
}

// starts jam loop after "sec" seconds
void start_jam_loop(unsigned int sec){
    if(!jamming){
        // set timer
        rtimer_set(&rtimer, RTIMER_NOW() + RTIMER_ARCH_SECOND * sec, 1, jam_loop, NULL);
        jamming = true;
    }
}

// stops jam loop after < 1ms
void stop_jam_loop(void){
    jamming = false;
}