## (1) Setup Arduino IDE 2 for Nucleo STM32F411RE

Follow the images for applying the setting for the Nucleo STM32F411RE.

### Selecting Board.

<img src="nucleo-stm32f411re-board-selection-1.png" alt="Board" width="1000" height="1000"/>

### Selecting Board part number.

<img src="nucleo-stm32f411re-board-selection-2.png" alt="Board part number" width="1000" height="1000"/>

### Selecting Upload method.

<img src="nucleo-stm32f411re-board-selection-3.png" alt="Upload method" width="1000" height="1000"/>


## (2) Connecting through ST-Link V2 clone

Connect 3.3V, GND, SWCLK, SWDIO according the images.

You must connect the microB connector to the PC for supplying the power to the Nucleo.

<img src="IMG_7500.jpg" alt="Entire picture" width="1000" height="1000"/>


<img src="IMG_7503.jpg" alt="target side" width="1000" height="1000"/>


<img src="IMG_7502-r-1.jpg" alt="stlink side" width="1000" height="1000"/>


## (3) Build the source and upload the binary to the target

Trying the LED blinking program.

The LED is connected to LED_BUILTIN.

```
#include <Arduino.h>

// LED_BUILTIN is assigned to an LED connected pin on most Arduino boards.
int led = LED_BUILTIN;

void setup() {
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
}

void loop() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second
}
```


<img src="stlinkv2clone-nucleost32f411re.png" alt="Upload" width="1000" height="1000"/>


