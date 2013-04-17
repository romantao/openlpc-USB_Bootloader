#include "LPC17xx.h"

#define LED_PORTNUM 1
#define LED_PINNUM 26

int main (void) {
    LPC_SC->PCONP |= ( 1 << 15 ); // power up GPIO
    LPC_GPIO1->FIODIR |= LED_PORTNUM << LED_PINNUM; // puts PLED_PORTNUM.29 into output mode.
    LPC_GPIO1->FIODIR |= LED_PORTNUM << 23; // puts PLED_PORTNUM.29 into output mode.
    LPC_GPIO1->FIODIR |= LED_PORTNUM << 24; // puts PLED_PORTNUM.29 into output mode.
    LPC_GPIO1->FIOPIN &= ~( LED_PORTNUM << LED_PINNUM ); // make PLED_PORTNUM.LED_PINNUM low
    LPC_GPIO1->FIOPIN &= ~( LED_PORTNUM << 23 ); // make PLED_PORTNUM.LED_PINNUM low
    LPC_GPIO1->FIOPIN &= ~( LED_PORTNUM << 24 ); // make PLED_PORTNUM.LED_PINNUM low
    while(1) {
        LPC_GPIO1->FIOPIN |= LED_PORTNUM << LED_PINNUM; // make P1LED_PORTNUM high
        LPC_GPIO1->FIOPIN |= LED_PORTNUM << 23; // make P1LED_PORTNUM high
        LPC_GPIO1->FIOPIN |= LED_PORTNUM << 24; // make P1LED_PORTNUM high
        delayMs(0, 500);
        LPC_GPIO1->FIOPIN &= ~( LED_PORTNUM << LED_PINNUM ); // make PLED_PORTNUM.LED_PINNUM low
        LPC_GPIO1->FIOPIN &= ~( LED_PORTNUM << 23 ); // make PLED_PORTNUM.LED_PINNUM low
        LPC_GPIO1->FIOPIN &= ~( LED_PORTNUM << 24 ); // make PLED_PORTNUM.LED_PINNUM low
        delayMs(0, 500);
    }
    return 0;
}
