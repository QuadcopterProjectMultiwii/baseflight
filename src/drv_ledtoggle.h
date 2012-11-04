#pragma once

//Init the led gpio port when enabled
void ledToggleInit( bool enable );

//Update the leds, enabled signals that the leds are enabled
void ledToggleUpdate( bool active );
