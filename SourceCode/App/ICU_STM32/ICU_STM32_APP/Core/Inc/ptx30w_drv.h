/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PTX30W_DRV_H__
#define __PTX30W_DRV_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "ptx30w.h"

/* Exported macro ------------------------------------------------------------*/
#define PTX30W_I2C_ADDRESS  	(0x4B)

enum {
    PTX_NFC_NONE = 0,
    PTX_NFC_VALID,
};

/* Exported types ------------------------------------------------------------*/


/* Exported functions --------------------------------------------------------*/

int ptx30w_driver_init(void);
int32_t ptx30w_update_status(void);
int ptx30w_set_rt(ptxRuntimeParamType_t type, uint8_t value);

/* NFC TDC data loop */
void ptx30w_TDC_transfer(void);

#ifdef __cplusplus
}
#endif

#endif /* __PTX30W_DRV_H__ */