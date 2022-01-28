# Temperature control for fermenters
Automated temperature control for a fermenter using an Arduino

## Hardware

I'm using an Arduino Uno, with the following eletronics pluged into it:

- 1 DS18B20 temperature sensor
- 1 relay module
- 1 310l Eletrolux Fridge
- 1 Tm1637 7 segments 4 digits display
- 1 LED
- 3 buttons

## How it works

Essentially, the fridge's thermostat is being intercepted by the relay, that is controlled by the Arduino. The temperature sensor will give the Arduino the information about the temperature, and the it will decide if de fridge should run or not. There is a display that shows the current temperature and the set temperature. An LED will flash if the fridge's temperature is too off the target. There is also three buttons to control the set temperature.
