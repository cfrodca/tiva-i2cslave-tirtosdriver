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
/** ============================================================================
 *  @file       EK_TM4C1294XL.h
 *
 *  @brief      EK_TM4C1294XL Board Specific APIs
 *
 *  The EK_TM4C1294XL header file should be included in an application as
 *  follows:
 *  @code
 *  #include <EK_TM4C1294XL.h>
 *  @endcode
 *
 *  ============================================================================
 */

#ifndef __EK_TM4C1294XL_H
#define __EK_TM4C1294XL_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *  @def    EK_TM4C1294XL_I2CSlaveName
 *  @brief  Enum of I2C names on the EK_TM4C1294XL dev board
 */
typedef enum EK_TM4C1294XL_I2CSlaveName {
    EK_TM4C1294XL_I2CSLAVE5 = 0,

    EK_TM4C1294XL_I2CSLAVECOUNT
} EK_TM4C1294XL_I2CSlaveName;

/*!
 *  @brief  Initialize board specific I2CSlave settings
 *
 *  This function initializes the board specific I2CSlave settings and then calls
 *  the I2CSlave_init API to initialize the I2CSlave module.
 *
 *  The I2CSlave peripherals controlled by the I2CSlave module are determined by the
 *  I2CSlave_config variable.
 */
extern void EK_TM4C1294XL_initI2CSlave(void);

#ifdef __cplusplus
}
#endif

#endif /* __EK_TM4C1294XL_H */
