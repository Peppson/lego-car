#include "neopixels.h"
#include "hardware.h"

uint8_t NeoPixels::_currentBrightness = 1;

void NeoPixels::init()
{	
    FastLED.addLeds<NEOPIXEL, PIN_NEOPIXEL_FRONT>(leds, COUNT_NEOPIXEL_FRONT);
    FastLED.addLeds<NEOPIXEL, PIN_NEOPIXEL_BACK>(leds + COUNT_NEOPIXEL_FRONT, COUNT_NEOPIXEL_BACK);
}

void NeoPixels::fadeInAllLeds()
{   
    setAllColor(Color::OFF, true);

    leds[0] = colorSettings.cabinLight;
    leds[1] = colorSettings.frontHeadLight;
    leds[2] = colorSettings.frontTurnSignal;
    leds[3] = colorSettings.frontTurnSignal;
    leds[4] = colorSettings.frontHeadLight;
    leds[5] = colorSettings.backBreakLight;
    leds[6] = colorSettings.backBreakLight;

    setBrightness(DEFAULT_BRIGHTNESS);
    animation_fadeAll(true);

    setAllColor(Color::OFF);
    leds[0] = colorSettings.cabinLight;
    leds[1] = colorSettings.frontHeadLight;
    leds[2] = colorSettings.frontTurnSignal;
    leds[3] = colorSettings.frontTurnSignal;
    leds[4] = colorSettings.frontHeadLight;
    leds[5] = colorSettings.backBreakLight;
    leds[6] = colorSettings.backBreakLight;
    
    FastLED.show();
    FastLED.show(); // sometimes doesnt apply after fade
}

void NeoPixels::setColor()
{
    leds[0] = colorSettings.cabinLight;
    leds[1] = colorSettings.frontHeadLight;
    leds[2] = colorSettings.frontTurnSignal;
    leds[3] = colorSettings.frontTurnSignal;
    leds[4] = colorSettings.frontHeadLight;
    leds[5] = colorSettings.backBreakLight;
    leds[6] = colorSettings.backBreakLight;

    FastLED.show();
}

void NeoPixels::setAllColor(uint32_t color, bool show) 
{
	for (uint8_t i = 0; i < COUNT_NEOPIXEL_TOTAL; i++) 
	{
		leds[i] = color;
	}
	if (show) FastLED.show();
}

void NeoPixels::setBrightness(uint8_t brightness)
{	
	_currentBrightness = constrain(brightness, NEOPIXEL_MIN_BRIGHTNESS, 254);
	FastLED.setBrightness(_currentBrightness);
}

void NeoPixels::AllLedsFullBrightness() 
{   
    setBrightness(255);
    setAllColor(Color::WHITE, false);
	animation_fadeAll(true);
    delay(5000);
    animation_fadeAll(false);
    STOP
}

void NeoPixels::animation_fadeAll(const bool fadeIn, uint32_t delay_uS)
{	
	// Makes sure fade takes the same amount of time regardless of ledBrightness
	// Account for about 3mS in "FastLED.show()" for each iteration
	uint totalDelay_mS = 1000 - (3 * _currentBrightness);
	uint dynamicDelay_uS = (totalDelay_mS * 1000) / _currentBrightness; 

	// Fade
	for (uint8_t i = 1; i < _currentBrightness; i++)
	{
		if (fadeIn)
			FastLED.setBrightness(i);
		else
			FastLED.setBrightness(_currentBrightness - i - 1);

		FastLED.show(); 
		delayMicroseconds(dynamicDelay_uS + delay_uS);
	}

	if (!fadeIn) 
	{ 
		setAllColor(Color::OFF);
	}
}

void NeoPixels::animation_police()
{   
    setBrightness(100);

    for (int i = 0; i < 2; i++)
    {
        leds[0] = Color::BLUE;
        leds[1] = Color::BLUE;
        leds[2] = Color::OFF;
        leds[3] = Color::OFF;
        leds[4] = Color::OFF;
        FastLED.show();
        delay(70);

        leds[0] = Color::OFF;
        leds[1] = Color::OFF;
        FastLED.show();
        delay(50);
    }

    for (int i = 0; i < 2; i++)
    {
        leds[0] = Color::RED;
        leds[1] = Color::OFF;
        leds[2] = Color::OFF;
        leds[3] = Color::OFF;
        leds[4] = Color::RED;
        FastLED.show();
        delay(70);

        leds[0] = Color::OFF;
        leds[4] = Color::OFF;
        FastLED.show();
        delay(50);
    }

    for (int i = 0; i < 3; i++)
    {
        leds[0] = Color::BLUE;
        leds[1] = Color::WHITE;
        leds[2] = Color::OFF;
        leds[3] = Color::OFF;
        leds[4] = Color::BLUE;
        FastLED.show();
        delay(40);

        leds[0] = Color::OFF;
        leds[1] = Color::BLUE;
        leds[4] = Color::WHITE;
        FastLED.show();
        delay(40);
    }

    leds[0] = Color::RED;
    leds[1] = Color::RED;
    leds[2] = Color::BLUE;
    leds[3] = Color::OFF;
    leds[4] = Color::BLUE;
    FastLED.show();
    delay(100);

    leds[0] = Color::BLUE;
    leds[1] = Color::BLUE;
    leds[2] = Color::OFF;
    leds[3] = Color::RED;
    leds[4] = Color::RED;
    FastLED.show();
    delay(100);

    leds[0] = Color::OFF;
    leds[1] = Color::OFF;
    leds[2] = Color::ORANGE;
    leds[3] = Color::OFF;
    leds[4] = Color::OFF;
    FastLED.show();
    delay(100);

    leds[2] = Color::OFF;
    leds[3] = Color::ORANGE;
    FastLED.show();
    delay(100);

    leds[2] = Color::ORANGE;
    FastLED.show();
    delay(100);

    leds[2] = Color::OFF;
    leds[3] = Color::OFF;
    FastLED.show();
    delay(100);
}
