## (1) Setup Arduino IDE 2 for Black Pill STM32F411CEU6

Follow the images for applying the setting for the Black Pill STM32F411CEU6.

### Selecting Board.

<img src="blackpill-stm32f411ceu-board-selection-1.png" alt="Board" width="1000" height="1000"/>

### Selecting Board part number.

<img src="blackpill-stm32f411ceu-board-selection-2.png" alt="Board part number" width="1000" height="1000"/>

### Selecting Upload method.

<img src="blackpill-stm32f411ceu-board-selection-3.png" alt="Upload method" width="1000" height="1000"/>


## (2) Connecting through ST-Link V2 clone

Connect 3.3V, GND, SWCLK, SWDIO according the images.

<img src="IMG_7511.jpg" alt="Entire picture" width="1000" height="1000"/>


<img src="IMG_7513.jpg" alt="target side" width="1000" height="1000"/>


<img src="IMG_7512-r-1.jpg" alt="stlink side" width="1000" height="1000"/>


## (3) Build the source and upload the binary to the target

Trying the LED blinking program.

The LED is connected to PC13 on the Black Pill board.

```
#include <Arduino.h>

// LED_BUILTIN is assigned to an LED connected pin on most Arduino boards.
#define LED_BUILTIN PC13
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

## (4) Upload

Sometimes it requires pressing BOOT0 and RESET button before uploading the binary.

1. Press both `RESET` and `BOOT0`
2. Hold boot0, release `RESET`
3. Release `BOOT0` 2 sec later

This is the screenshot when the upload is successful.

<img src="stlinkv2clone-blackpillstm32f411ceu.png" alt="Upload" width="1000" height="1000"/>
<br/>

## (5) Video of LED blinking

[![Video of LED blinking](led-blinking-1.png)](IMG_7514.mp4)
