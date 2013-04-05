#include "LPC17xx.h"

#define LED_PORTNUM 1
#define LED_PINNUM 26

volatile uint32_t ticks = 0;

void SysTick_Handler(void) {
    ticks++;
}

void _delay(uint32_t ms) {
    uint32_t now = ticks;
    while((ticks - now ) < ms);
}

int main (void) {
    SysTick_Config(SystemCoreClock / 1000);
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
        _delay(1000);
        LPC_GPIO1->FIOPIN &= ~( LED_PORTNUM << LED_PINNUM ); // make PLED_PORTNUM.LED_PINNUM low
        LPC_GPIO1->FIOPIN &= ~( LED_PORTNUM << 23 ); // make PLED_PORTNUM.LED_PINNUM low
        LPC_GPIO1->FIOPIN &= ~( LED_PORTNUM << 24 ); // make PLED_PORTNUM.LED_PINNUM low
        _delay(1000);
    }
    return 0;

}
