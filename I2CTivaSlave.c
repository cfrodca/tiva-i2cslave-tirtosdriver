/*
 * I2CTivaSlave.c
 *
 *  Created on: 6/09/2019
 *      Author: Cristian
 */

#include <stdint.h>
#include <stdbool.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Types.h>

#include "I2CTivaSlave.h"
#include <ti/drivers/I2C.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>

/* driverlib header files */
#include <inc/hw_memmap.h>
#include "inc/hw_i2c.h"
#include <inc/hw_ints.h>
#include <inc/hw_types.h>
#include <driverlib/i2c.h>

#if !defined(CCWARE)
#include <driverlib/sysctl.h>
#endif

/* I2CTivaSlave functions */
void            I2CTivaSlave_close(I2CSlave_Handle handle);
void            I2CTivaSlave_init(I2CSlave_Handle handle);
I2CSlave_Handle I2CTivaSlave_open(I2CSlave_Handle handle, I2CSlave_Params *params);
int             I2CTivaSlave_read(I2CSlave_Handle handle, void *buffer, size_t size);
int             I2CTivaSlave_write(I2CSlave_Handle handle, const void *buffer,
                                   size_t size);

/* Static functions */
static void readBlockingTimeout(UArg arg);
static bool readIsrBinaryBlocking(I2CSlave_Handle handle);
static void readSemCallback(I2CSlave_Handle handle, void *buffer, size_t count);
static int  readTaskBlocking(I2CSlave_Handle handle);
static void writeData(I2CSlave_Handle handle);
static void writeSemCallback(I2CSlave_Handle handle, void *buffer, size_t count);

/* I2CSlave function table for I2CTivaSlave implementation */
const I2CSlave_FxnTable I2CTivaSlave_fxnTable = {
    I2CTivaSlave_close,
    I2CTivaSlave_init,
    I2CTivaSlave_open,
    I2CTivaSlave_read,
    I2CTivaSlave_write,
};

/*
 *  ======== staticFxnTable ========
 *  This is a function lookup table to simplify the I2CSlave driver modes.
 */
static const I2CTivaSlave_FxnSet staticFxnTable = {
    .readIsrFxn  = readIsrBinaryBlocking,
    .readTaskFxn = readTaskBlocking
};

/*
 *  ======== I2CTivaSlave_close ========
 */
void I2CTivaSlave_close(I2CSlave_Handle handle)
{
    I2CTivaSlave_Object           *object = handle->object;
    I2CTivaSlave_HWAttrs const    *hwAttrs = handle->hwAttrs;

    /* Disable I2CSlave and interrupts. */
    I2CSlaveIntDisable(hwAttrs->baseAddr);
    I2CSlaveDisable(hwAttrs->baseAddr);

    Hwi_destruct(&(object->hwi));

    Semaphore_destruct(&object->writeSem);

    Semaphore_destruct(&object->readSem);
    Clock_destruct(&object->timeoutClk);

    object->state.opened = false;
}

/*
 *  ======== I2CTivaSlave_hwiIntFxn ========
  */
static void I2CTivaSlave_hwiIntFxn(UArg arg)
{
    uint32_t                     status;
    uint32_t                     i2cstatus;
    I2CTivaSlave_Object         *object = ((I2CSlave_Handle)arg)->object;
    I2CTivaSlave_HWAttrs const  *hwAttrs = ((I2CSlave_Handle)arg)->hwAttrs;

    /* Clear interrupts */
    i2cstatus = I2CSlaveStatus(hwAttrs->baseAddr);
    status = I2CSlaveIntStatusEx(hwAttrs->baseAddr, true);
    I2CSlaveIntClearEx(hwAttrs->baseAddr, status);
    I2CSlaveIntClear(hwAttrs->baseAddr);

    if (i2cstatus & I2C_SLAVE_ACT_RREQ) {
        if (status & I2C_SLAVE_INT_DATA) {
            object->readFxns.readIsrFxn((I2CSlave_Handle)arg);
        }
    }

    if (i2cstatus & I2C_SLAVE_ACT_TREQ) {
        writeData((I2CSlave_Handle)arg);
    }
}

/*
 *  ======== I2CTivaSlave_init ========
 */
void I2CTivaSlave_init(I2CSlave_Handle handle)
{
    I2CTivaSlave_Object    *object = handle->object;

    object->state.opened = false;
}

/*
 *  ======== I2CTivaSlave_open ========
 */
I2CSlave_Handle I2CTivaSlave_open(I2CSlave_Handle handle, I2CSlave_Params *params)
{
    unsigned int               key;
    I2CTivaSlave_Object           *object = handle->object;
    I2CTivaSlave_HWAttrs const    *hwAttrs = handle->hwAttrs;
    union {
        Hwi_Params             hwiParams;
        Semaphore_Params       semParams;
        Clock_Params           clockParams;
    } paramsUnion;

    key = Hwi_disable();

    if (object->state.opened == true) {
        Hwi_restore(key);
        return (NULL);
    }
    object->state.opened = true;

    Hwi_restore(key);

    object->readTimeout          = params->readTimeout;
    object->writeTimeout         = params->writeTimeout;
    object->slaveAddress         = params->slaveAddress;
    object->readFxns             = staticFxnTable;

    /* Set I2CSlave variables to defaults. */
    object->writeBuf             = NULL;
    object->readBuf              = NULL;
    object->writeCount           = 0;
    object->readCount            = 0;
    object->writeSize            = 0;
    object->readSize             = 0;

    RingBuf_construct(&object->ringBuffer, hwAttrs->ringBufPtr,
        hwAttrs->ringBufSize);
    Hwi_Params_init(&paramsUnion.hwiParams);
    paramsUnion.hwiParams.arg = (UArg)handle;
    paramsUnion.hwiParams.priority = hwAttrs->intPriority;
    Hwi_construct(&object->hwi, hwAttrs->intNum, I2CTivaSlave_hwiIntFxn,
        &paramsUnion.hwiParams, NULL);

    Semaphore_Params_init(&paramsUnion.semParams);
    paramsUnion.semParams.mode = Semaphore_Mode_BINARY;

    /* If write mode is blocking create a semaphore and set callback. */
    Semaphore_construct(&object->writeSem, 0, &paramsUnion.semParams);
    object->writeCallback = &writeSemCallback;

    /* If read mode is blocking create a semaphore and set callback. */
    Semaphore_construct(&object->readSem, 0, &(paramsUnion.semParams));
    object->readCallback = &readSemCallback;
    Clock_Params_init(&paramsUnion.clockParams);
    paramsUnion.clockParams.period = 0;
    paramsUnion.clockParams.startFlag = FALSE;
    paramsUnion.clockParams.arg = (UArg)handle;
    Clock_construct(&object->timeoutClk,
                    readBlockingTimeout,
                    object->readTimeout,
                    &paramsUnion.clockParams);

    I2CSlaveInit(hwAttrs->baseAddr, params->slaveAddress);

    /* Enable I2CSlave and its interrupt. */
    I2CSlaveIntClearEx(hwAttrs->baseAddr, I2C_SLAVE_INT_DATA);

    I2CSlaveIntClear(hwAttrs->baseAddr);
    I2CSlaveEnable(hwAttrs->baseAddr);

    I2CSlaveIntEnableEx(hwAttrs->baseAddr, I2C_SLAVE_INT_DATA);

    /* Return the handle */
    return (handle);
}

/*
 *  ======== I2CTivaSlave_read ========
 */
int I2CTivaSlave_read(I2CSlave_Handle handle, void *buffer, size_t size)
{
    unsigned int                key;
    I2CTivaSlave_Object        *object = handle->object;

    key = Hwi_disable();

    /* Save the data to be read and restore interrupts. */
    object->readBuf = buffer;
    object->readSize = size;
    object->readCount = size;

    Hwi_restore(key);

    return (object->readFxns.readTaskFxn(handle));
}

/*
 *  ======== I2CTivaSlave_write ========
 */
int I2CTivaSlave_write(I2CSlave_Handle handle, const void *buffer, size_t size)
{
    unsigned int                   key;
    I2CTivaSlave_Object           *object = handle->object;
    I2CTivaSlave_HWAttrs const    *hwAttrs = handle->hwAttrs;
    uint32_t                       status;
    uint32_t                       writeCount;

    if (!size) {
        return 0;
    }

    key = Hwi_disable();

    if (object->writeCount) {
        Hwi_restore(key);

        return (I2CSLAVE_ERROR);
    }

    /* Save the data to be written and restore interrupts. */
    object->writeBuf = buffer;
    object->writeSize = size;
    object->writeCount = size;

    Hwi_restore(key);

    status = I2CSlaveStatus(hwAttrs->baseAddr);
    if (status & I2C_SLAVE_ACT_TREQ) {
        writeData(handle);
    }

    Semaphore_pend(Semaphore_handle(&object->writeSem), BIOS_NO_WAIT);

    if (object->writeCount) {
        /* If writeMode is blocking, block and get the state. */
        /* Pend on semaphore and wait for Hwi to finish. */
        if (!Semaphore_pend(Semaphore_handle(&object->writeSem),
                object->writeTimeout)) {
            I2CSlaveIntClearEx(hwAttrs->baseAddr, I2C_SLAVE_INT_DATA);
            I2CSlaveIntClear(hwAttrs->baseAddr);
            /* Semaphore timed out, make the write empty and log the write. */
            //object->writeCount = 0;
        }
    }

    writeCount = object->writeCount;
    object->writeCount = 0;
    return (object->writeSize - writeCount);
}

/*
 *  ======== readBlockingTimeout ========
 */
static Void readBlockingTimeout(UArg arg)
{
    I2CTivaSlave_Object *object = ((I2CSlave_Handle)arg)->object;
    object->state.bufTimeout = true;
    Semaphore_post(Semaphore_handle(&object->readSem));
}

/*
 *  ======== readIsrBinaryBlocking ========
 *  Function that is called by the ISR
 */
static bool readIsrBinaryBlocking(I2CSlave_Handle handle)
{
    I2CTivaSlave_Object           *object = handle->object;
    I2CTivaSlave_HWAttrs const    *hwAttrs = handle->hwAttrs;
    uint32_t                       readIn;

    readIn = I2CSlaveDataGet(hwAttrs->baseAddr);

    if (RingBuf_put(&object->ringBuffer, (unsigned char)readIn) == -1) {
        return (false);
    }

    if (object->state.callCallback) {
        object->state.callCallback = false;
        object->readCallback(handle, NULL, 0);
    }

    return (true);
}

/*
 *  ======== readSemCallback ========
 *  Simple callback to post a semaphore for the blocking mode.
 */
static void readSemCallback(I2CSlave_Handle handle, void *buffer, size_t count)
{
    I2CTivaSlave_Object *object = handle->object;

    Semaphore_post(Semaphore_handle(&object->readSem));
}

/*
 *  ======== readTaskBlocking ========
 */
static int readTaskBlocking(I2CSlave_Handle handle)
{
    unsigned char            readIn;
    uintptr_t                key;
    I2CTivaSlave_Object     *object = handle->object;
    unsigned char           *buffer = object->readBuf;

    object->state.bufTimeout = false;
    /*
     * It is possible for the object->timeoutClk and the callback function to
     * have posted the object->readSem Semaphore from the previous I2CSlave_read
     * call (if the code below didn't get to stop the clock object in time).
     * To clear this, we simply do a NO_WAIT pend on (binary) object->readSem
     * so that it resets the Semaphore count.
     */
    Semaphore_pend(Semaphore_handle(&object->readSem), BIOS_NO_WAIT);
    if (object->readTimeout != 0) {
        Clock_start(Clock_handle(&object->timeoutClk));
    }

    while (object->readCount) {
        key = Hwi_disable();

        if (RingBuf_get(&object->ringBuffer, &readIn) < 0) {
            object->state.callCallback = true;
            Hwi_restore(key);

            if (object->readTimeout == 0) {
                break;
            }

            Semaphore_pend(Semaphore_handle(&object->readSem),
                BIOS_WAIT_FOREVER);
            if (object->state.bufTimeout == true) {
                break;
            }
            RingBuf_get(&object->ringBuffer, &readIn);
        }
        else {
            Hwi_restore(key);
        }

        *buffer = readIn;
        buffer++;
        /* In blocking mode, readCount doesn't not need a lock */
        object->readCount--;
    }

    Clock_stop(Clock_handle(&object->timeoutClk));
    return (object->readSize - object->readCount);
}

/*
 *  ======== writeData ========
 */
static void writeData(I2CSlave_Handle handle)
{
    I2CTivaSlave_Object           *object = handle->object;
    I2CTivaSlave_HWAttrs const    *hwAttrs = handle->hwAttrs;
    unsigned char                 *writeOffset;

    writeOffset = (unsigned char *)object->writeBuf +
        object->writeSize * sizeof(unsigned char);
    if (object->writeCount) {
        I2CSlaveDataPut(hwAttrs->baseAddr, *(writeOffset - object->writeCount));
        object->writeCount--;
    }

    if (!object->writeCount) {
        object->writeCallback(handle, (void *)object->writeBuf,
            object->writeSize);
    }
}

/*
 *  ======== writeSemCallback ========
 *  Simple callback to post a semaphore for the blocking mode.
 */
static void writeSemCallback(I2CSlave_Handle handle, void *buffer, size_t count)
{
    I2CTivaSlave_Object *object = handle->object;

    Semaphore_post(Semaphore_handle(&object->writeSem));
}
