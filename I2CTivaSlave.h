/*
 * I2CTivaSlave.h
 *
 *  Created on: 6/09/2019
 *      Author: Cristian
 */

#ifndef I2CTIVASLAVE_H_
#define I2CTIVASLAVE_H_

#include <stdint.h>
#include <stdbool.h>
#include "I2CSlave.h"
#include <ti/drivers/utils/RingBuf.h>

#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#define ti_sysbios_family_arm_m3_Hwi__nolocalnames
#include <ti/sysbios/family/arm/m3/Hwi.h>

/* I2CSlave function table pointer */
extern const I2CSlave_FxnTable I2CTivaSlave_fxnTable;

typedef struct I2CTivaSlave_FxnSet {
    bool (*readIsrFxn)  (I2CSlave_Handle handle);
    int  (*readTaskFxn) (I2CSlave_Handle handle);
} I2CTivaSlave_FxnSet;

typedef struct I2CTivaSlave_HWAttrs {
    /* I2CSlave Peripheral's base address */
    unsigned int    baseAddr;
    /* I2CSlave Peripheral's interrupt vector */
    unsigned int    intNum;
    /* I2CSlave Peripheral's interrupt priority */
    unsigned int    intPriority;
    /* Pointer to a application ring buffer */
    unsigned char  *ringBufPtr;
    /* Size of ringBufPtr */
    size_t          ringBufSize;
} I2CTivaSlave_HWAttrs;

typedef struct I2CTivaSlave_Object {
    /* I2C Slave state variable */
    struct {
        bool             opened:1;         /* Has the obj been opened */
        /*
         * Flag to determine if a timeout has occurred when the user called
         * I2CSlave_read(). This flag is set by the timeoutClk clock object.
         */
        bool             bufTimeout:1;
        /*
         * Flag to determine when an ISR needs to perform a callback;
         */
        bool             callCallback:1;
    } state;

    unsigned char        slaveAddress;     /* Slave address */
    Clock_Struct         timeoutClk;       /* Clock object to for timeouts */
    RingBuf_Object       ringBuffer;

    /* A complement pair of read functions for both the ISR and I2CSlave_read() */
    I2CTivaSlave_FxnSet  readFxns;
    unsigned char       *readBuf;          /* Buffer data pointer */
    size_t               readSize;         /* Desired number of bytes to read */
    size_t               readCount;        /* Number of bytes left to read */
    Semaphore_Struct     readSem;          /* I2C read semaphore*/
    unsigned int         readTimeout;      /* Timeout for read semaphore */
    I2CSlave_Callback    readCallback;     /* Pointer to read callback */

    const unsigned char *writeBuf;         /* Buffer data pointer */
    size_t               writeSize;        /* Desired number of bytes to write*/
    size_t               writeCount;       /* Number of bytes left to write */
    Semaphore_Struct     writeSem;         /* I2C write semaphore*/
    unsigned int         writeTimeout;     /* Timeout for write semaphore */
    Clock_Struct         writeTimeoutClk;  /* Clock object to for write pending timeouts */
    unsigned int         writePendTimeout; /* Timeout for pending write */
    I2CSlave_Callback    writeCallback;    /* Pointer to write callback */

    ti_sysbios_family_arm_m3_Hwi_Struct hwi;  /* Hwi object handle */
} I2CTivaSlave_Object, *I2CTivaSlave_Handle;

/* Do not interfere with the app if they include the family Hwi module */
#undef ti_sysbios_family_arm_m3_Hwi__nolocalnames

#endif /* I2CTIVASLAVE_H_ */
