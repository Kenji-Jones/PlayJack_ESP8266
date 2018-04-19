# PlayJack_ESP8266

Capabilities:
- color and speed of LEDs can be changed from the wifi portal
- tested 135m two way communication using pcb trace antennas
- 70m of communication was reached in Shibuya, Tokyo on a friday night.
- 40m of reliable two way communication in Shibuya


This is a proof of concept using:
- Wemos D1 Mini ESP8266 https://escapequotes.net/esp8266-wemos-d1-mini-pins-and-diagram/
- NRF24L01+ https://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
- TMR RF24 library http://tmrh20.github.io/RF24/
- WS2812B (NeoPixels) https://learn.adafruit.com/adafruit-neopixel-uberguide/the-magic-of-neopixels
- FastLED library https://github.com/FastLED/FastLED
- Wifi captive portal for adjustments https://github.com/nhaduy99/Walkgreen_v2.0 (using this for reference)

To do
- Captive portal adjustments (i want to add a color picker if possible?)
- Make it possible to update without internet connection (just between phone and esp8266)
- autocalibrate A0 sensor so it doesnt trigger when nothing is happening
- separate timers for led fade and led runs
- add LED count to adjustable settings



/////  PINOUT / WIRED CONNECTIONS  \\\\\

Wemos D1 Mini https://escapequotes.net/esp8266-wemos-d1-mini-pins-and-diagram/
NRF24L01+ http://foreglance.com/wp-content/uploads/2014/08/NRF24L01plus-pinout.png

(Wemos Printed Pin / Arduino Pin)
- A0 / A0 = Piezo Positive to ADC (optional)
- D3 / 0 = Button to ground for wifi portal // changed from D0 / 16
- D1 / 5  = Signal for WS2812B leds
- D2 / 4  = CE    (on NRF24)
- D5 / 14 = SCK   (on NRF24)
- D6 / 12 = MISO  (on NRF24)
- D7 / 13 = MOSI  (on NRF24)
- D8 / 15 = CSN   (on NRF24)
- 3v3     = VCC   (on NRF24)
- 5V      = WS2812B power (for short strips)


EDITS
in WifiManager.h
_shouldBreakAfterConfig =  = true; // was false
      this allows the device to update without connecting to internet.
