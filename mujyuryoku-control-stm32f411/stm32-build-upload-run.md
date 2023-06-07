# Zero Gravity Test program on STM32F411 Black Pill

## Connecting STM32F411 for Serial output

Connect the USB-C connector to the development PC for the printf() output.

<img src="stm32-serial-print-connect.jpg" alt="Connecting" width="1000"/>
<br/>

## Setting Arduino for SerialUSB and supporting float

To enable Serial print through USB-C cable.

<img src="stm32-serial-print-setting.png" alt="SerialUSB1" width="1000"/>
<br/>

<img src="stm32-serial-print-setting-1.png" alt="SerialUSB2" width="1000"/>
<br/>

Change the Newlib to enable to user type float in the printf() function.

[![printf_float](enable_float_printf.png)](stm32-serial-setting-ss.mp4)
<br/>

## Explanation of porting the Test program to STM32F411

Mostly there was two parts required for porting ESP32 program to STM32F411.


### Change the GPIO Pins.

The SW_GPIO_NUM is `KEY` button on the Black Pill.
The PC13 is the LED on the Black Pill.
```
#define EN_GPIO_NUM         PA8
#define STEP_GPIO_NUM       PA9
#define DIR_GPIO_NUM        PA10

#define EN_GPIO_NUM2        PA5
#define STEP_GPIO_NUM2      PA6
#define DIR_GPIO_NUM2       PA7

#define SW_GPIO_NUM         PA0  // KEY button on Black Pill

#define LED_GPIO_NUM        PC13 // LED on Black Pill
#define LED2_GPIO_NUM       PC14

#define HALL_IC_GPIO_NUM    PB8
#define HALL_PW_GPIO_NUM    PB9
```

### Creating print function using SerialUSB.

The macro STM32F411xE is define by Arduino setting when selecting Black Pill STM32F411.
```
// printf function to support Serial or printf
void local_printf(char *fmt, ...)
{
  va_list argptr;
  va_start(argptr, fmt);
#ifdef STM32F411xE
  SerialUSB.printf(fmt, argptr);
#else
  printf(fmt, argptr);
#endif
  va_end(argptr);
}
```

### Location to find STM32F411 definitions in the STM32duino to port programs on Black Pill STM32F4.


The board setting at `line 3986` in `boards.txt`.
```
$ vi +3986 /home/USER/.arduino15/packages/STMicroelectronics/hardware/stm32/2.5.0/boards.txt
```

Definition in `boards.txt`.
```
GenF4.menu.pnum.GENERIC_F411CEUX=Generic F411CEUx
GenF4.menu.pnum.GENERIC_F411CEUX.upload.maximum_size=524288
GenF4.menu.pnum.GENERIC_F411CEUX.upload.maximum_data_size=131072
GenF4.menu.pnum.GENERIC_F411CEUX.build.board=GENERIC_F411CEUX
GenF4.menu.pnum.GENERIC_F411CEUX.build.product_line=STM32F411xE
GenF4.menu.pnum.GENERIC_F411CEUX.build.variant=STM32F4xx/F411C(C-E)(U-Y)
```

The definitions of Black Pill STM32F411.
```
/home/USER/.arduino15/packages/STMicroelectronics/hardware/stm32/2.5.0/variants/STM32F4xx/F411C\(C-E\)\(U-Y\)/variant_BLACKPILL_F411CE.h
```

The definitions of STM32F411.
```
/home/USER/.arduino15/packages/STMicroelectronics/hardware/stm32/2.5.0/system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/stm32f411xe.h
```

The definitions of generic STM32F4xx.
```
/home/USER/.arduino15/packages/STMicroelectronics/hardware/stm32/2.5.0/system/Drivers/CMSIS/Device/ST/STM32F4xx/Include/stm32f4xx.h
```

The definitions of generic ARM Cortex-M4.
```
/home/USER/.arduino15/packages/STMicroelectronics/tools/CMSIS/5.7.0/CMSIS/Core/Include/core_cm4.h
```


### Entire Source code of Zero Gravity Test program

[Original ESP32](ESP32_ZeroG_ControlPrg.ino)

[Ported source code on Black Pill STM32F411](mujyuryoku-control-stm32f411.ino)


## Build and upload Zero Gravity Test program

[![build_upload](stm32-build-upload.png)](stm32-build-upload-ss.mp4)
