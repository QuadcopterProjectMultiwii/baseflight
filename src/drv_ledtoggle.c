
#include "board.h"
#include "mw.h"

//Is the led GPIO channel enabled
static bool ledEnabled;
static uint8_t ledIndex;

#define LEDTOGGLE_GPIO GPIOC
#define LEDTOGGLE_PIN GPIO_Pin_15

static void ledEnable() {
	digitalHi( LEDTOGGLE_GPIO, LEDTOGGLE_PIN );
}


static void ledDisable() {
	digitalLo( LEDTOGGLE_GPIO, LEDTOGGLE_PIN );
}


void ledToggleInit( bool enabled ) {
	ledEnabled = enabled;
//	printf( "Led State %d", enabled );
	if ( !ledEnabled )
		return;
    GPIO_InitTypeDef GPIO_InitStructure;
	//Set the led pin to be an output
    GPIO_InitStructure.GPIO_Pin = LEDTOGGLE_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    //Set the pin in push/pull mode
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init( LEDTOGGLE_GPIO, &GPIO_InitStructure);

    ledDisable();

}

void ledToggleUpdate( bool activated ) {
	//Ignore when we're not using leds
	if ( !ledEnabled ) {
		return;
	}
	if ( !activated ) {
		ledDisable();
		return;
	}
	uint8_t bit = (ledIndex++ >> 1) & 31;
	if ( cfg.ledtoggle_pattern & ( 1 << bit ) ) {
		ledEnable();
//		LED0_ON;
	} else {
//		LED0_OFF;
		ledDisable();
	}
}
