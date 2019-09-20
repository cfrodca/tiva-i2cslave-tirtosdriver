# tiva-i2cslave-tirtosdriver
Unofficial driver for the I2C slave, based on TIRTOS 2.16

To facilitate development, use the code composer studio.

Copy and adjust the code of the files Board.h, EK_TM4C1294XL.c, EK_TM4C1294XL.h and main.c in a new TIRTOS project.

### Installation
Add the following files to your project.
```
I2CSlave.c
I2CSlave.h
I2CTivaSlave.c
I2CTivaSlave.h
```
Add the code from files EK_TM4C1294XL.* to your own EK_TM4C1294XL.* files. In this files the necessary objects for the initialization of the I2C slave are defined.
```
EK_TM4C1294XL.c
EK_TM4C1294XL.h
```

### Use 
In your own Board.h file, define the following items.
```
#define Board_initI2CSlave EK_TM4C1294XL_initI2CSlave     // Init function in EK_TM4C1294XL.c 
#define Board_I2CSlave5 EK_TM4C1294XL_I2CSLAVE5           // The name of the peripheral defined in EK_TM4C1294XL.h
```
In the main.c file, include the header I2CSlave.h
```
#include "I2CSlave.h"
```
Call function Board_initI2CSlave() within the main() function. Subsequently, in a task create the i2c slave object with its respective parameters. 
```
    i2cslaveParams.readTimeout = 10;    // system ticks
    i2cslaveParams.writeTimeout = 100;  
    i2cslaveParams.writePendingTimeout = 40;  // Time that the Driver waits before writing a byte (0x00) in the bus. 
                                              // For the case where the application never writes some data and the bus hangs.
    i2cslaveParams.slaveAddress = 0x1D;
```
