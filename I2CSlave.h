/*
 * I2CSlave.h
 *
 *  Created on: 6/09/2019
 *      Author: Cristian
 */

#ifndef I2CSLAVE_H_
#define I2CSLAVE_H_

#include <stdint.h>
#include <stddef.h>

#define I2CSLAVE_WAIT_FOREVER (~0)

#define I2CSLAVE_STATUS_SUCCESS         0

#define I2CSLAVE_STATUS_ERROR          -1

#define I2CSLAVE_ERROR  I2CSLAVE_STATUS_ERROR

/*
 *  A handle that is returned from a I2CSlave_open() call.
 */
typedef struct I2CSlave_Config      *I2CSlave_Handle;

typedef void (*I2CSlave_Callback)    (I2CSlave_Handle, void *buf, size_t count);


typedef struct I2CSlave_Params {
    unsigned int      readTimeout;      /*!< Timeout for read semaphore */
    unsigned int      writeTimeout;     /*!< Timeout for write semaphore */
    unsigned char     slaveAddress;     /*!< Slave address */
} I2CSlave_Params;

typedef void            (*I2CSlave_CloseFxn)          (I2CSlave_Handle handle);
typedef void            (*I2CSlave_InitFxn)           (I2CSlave_Handle handle);
typedef I2CSlave_Handle (*I2CSlave_OpenFxn)           (I2CSlave_Handle handle,
                                                       I2CSlave_Params *params);
typedef int             (*I2CSlave_ReadFxn)           (I2CSlave_Handle handle, void *buffer,
                                                       size_t size);
typedef int             (*I2CSlave_WriteFxn)          (I2CSlave_Handle handle,
                                                       const void *buffer,
                                                       size_t size);

typedef struct I2CSlave_FxnTable {
    /*! Function to close the specified peripheral */
    I2CSlave_CloseFxn       closeFxn;

    /*! Function to initialize the given data object */
    I2CSlave_InitFxn        initFxn;

    /*! Function to open the specified peripheral */
    I2CSlave_OpenFxn        openFxn;

    /*! Function to read from the specified peripheral */
    I2CSlave_ReadFxn        readFxn;

    /*! Function to write from the specified peripheral */
    I2CSlave_WriteFxn       writeFxn;
} I2CSlave_FxnTable;

typedef struct I2CSlave_Config {
    /*! Pointer to a table of driver-specific implementations of I2CSlave APIs */
    I2CSlave_FxnTable const    *fxnTablePtr;
    /* Pointer to a driver specific data object */
    void                 *object;
    /* Pointer to a driver specific hardware attributes structure */
    void           const *hwAttrs;
} I2CSlave_Config;

extern void I2CSlave_close(I2CSlave_Handle handle);

extern void I2CSlave_init(void);

extern I2CSlave_Handle I2CSlave_open(unsigned int index, I2CSlave_Params *params);

extern void I2CSlave_Params_init(I2CSlave_Params *params);

extern int I2CSlave_write(I2CSlave_Handle handle, const void *buffer, size_t size);

extern int I2CSlave_read(I2CSlave_Handle handle, void *buffer, size_t size);

#endif /* I2CSLAVE_H_ */
