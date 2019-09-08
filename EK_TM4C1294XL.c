/*
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== EK_TM4C1294XL.c ========
 *  This file is responsible for setting up the board specific items for the
 *  EK_TM4C1294XL board.
 */

#include <stdint.h>
#include <stdbool.h>

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_gpio.h>

#include <driverlib/flash.h>
#include <driverlib/gpio.h>
#include <driverlib/i2c.h>
#include <driverlib/pin_map.h>
#include <driverlib/pwm.h>
#include <driverlib/ssi.h>
#include <driverlib/sysctl.h>
#include <driverlib/uart.h>
#include <driverlib/udma.h>
#include <driverlib/timer.h>

#include "EK_TM4C1294XL.h"

/*
 *  =============================== I2C Slave ===============================
 */
/* Place into subsections to allow the TI linker to remove items properly */

#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(I2CSlave_config, ".const:I2CSlave_config")
#pragma DATA_SECTION(i2cTivaSlaveHWAttrs, ".const:i2cTivaSlaveHWAttrs")
#endif

#include "I2CSlave.h"
#include "I2CTivaSlave.h"

I2CTivaSlave_Object i2cTivaSlaveObjects[EK_TM4C1294XL_I2CSLAVECOUNT];
unsigned char i2cTivaSlaveRingBuffer[EK_TM4C1294XL_I2CSLAVECOUNT][32];

const I2CTivaSlave_HWAttrs i2cTivaSlaveHWAttrs[EK_TM4C1294XL_I2CSLAVECOUNT] = {
    {
        .baseAddr = I2C5_BASE,
        .intNum = INT_I2C5,
        .intPriority = (~0),
        .ringBufPtr  = i2cTivaSlaveRingBuffer[0],
        .ringBufSize = sizeof(i2cTivaSlaveRingBuffer[0])
    }
};

const I2CSlave_Config I2CSlave_config[] = {
    {
        .fxnTablePtr = &I2CTivaSlave_fxnTable,
        .object = &i2cTivaSlaveObjects[0],
        .hwAttrs = &i2cTivaSlaveHWAttrs[0]
    },
    {NULL, NULL}
};

void EK_TM4C1294XL_initI2CSlave(void)
{
    /* I2C5 Init */
    /* Enable the peripheral */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C5);

    /* Configure the appropriate pins to be I2C instead of GPIO. */
    GPIOPinConfigure(GPIO_PB0_I2C5SCL);
    GPIOPinConfigure(GPIO_PB1_I2C5SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_0);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_1);

    I2CSlave_init();
}
