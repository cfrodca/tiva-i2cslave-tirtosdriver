/*
 * I2CSlave.c
 *
 *  Created on: 6/09/2019
 *      Author: Cristian
 */

/*
 *  ======== I2CSlave.c ========
 */

#include <stdint.h>
#include "I2CSlave.h"

/* Externs */
extern const I2CSlave_Config I2CSlave_config[];

/* Used to check status and initialization */
static int I2CSlave_count = -1;

/* Default I2CSlave parameters structure */
const I2CSlave_Params I2CSlave_defaultParams = {
    I2CSLAVE_WAIT_FOREVER,    /* readTimeout */
    I2CSLAVE_WAIT_FOREVER,    /* writeTimeout */
    100,                      /* writePendingTimeout */
    0x1D,                     /* address */
};

/*
 *  ======== I2CSlave_close ========
 */
void I2CSlave_close(I2CSlave_Handle handle)
{
    handle->fxnTablePtr->closeFxn(handle);
}

/*
 *  ======== I2CSlave_init ========
 */
void I2CSlave_init(void)
{
    if (I2CSlave_count == -1) {
        /* Call each driver's init function */
        for (I2CSlave_count = 0; I2CSlave_config[I2CSlave_count].fxnTablePtr != NULL; I2CSlave_count++) {
            I2CSlave_config[I2CSlave_count].fxnTablePtr->initFxn((I2CSlave_Handle)&(I2CSlave_config[I2CSlave_count]));
        }
    }
}

/*
 *  ======== I2CSlave_open ========
 */
I2CSlave_Handle I2CSlave_open(unsigned int index, I2CSlave_Params *params)
{
    I2CSlave_Handle         handle;

    if (index >= I2CSlave_count) {
        return (NULL);
    }

    /* If params are NULL use defaults */
    if (params == NULL) {
        params = (I2CSlave_Params *) &I2CSlave_defaultParams;
    }

    /* Get handle for this driver instance */
    handle = (I2CSlave_Handle)&(I2CSlave_config[index]);

    return (handle->fxnTablePtr->openFxn(handle, params));

}

/*
 *  ======== I2CSlave_Params_init ========
 */
void I2CSlave_Params_init(I2CSlave_Params *params)
{
    *params = I2CSlave_defaultParams;
}

/*
 *  ======== I2CSlave_read ========
 */
int I2CSlave_read(I2CSlave_Handle handle, void *buffer, size_t size)
{
    return (handle->fxnTablePtr->readFxn(handle, buffer, size));
}

/*
 *  ======== I2CSlave_write ========
 */
int I2CSlave_write(I2CSlave_Handle handle, const void *buffer, size_t size)
{
    return (handle->fxnTablePtr->writeFxn(handle, buffer, size));
}

