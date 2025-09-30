/**
  * @file protocol.h
  * @author DCX (chunxiao.dai@neuroxess.com)
  * @brief Declaration and definition of communication protocol
  * @version 0.1
  * @date 2025-01-15
  *
  * @copyright Copyright (C) 2025 NeuroXess
  *
  */

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * @defgroup Macros
 *****************************************************************************/
/// Maximum size of data filed in message package
#define PDU_DATA_SIZE   256

#define MSG_FRAME_TIMEOUT   50
#define MSG_FRAME_RECEIVED  ((MSG_FRAME_TIMEOUT) + 1)

#define COMBINED(a, b) (((a) << 8) | (b))

/******************************************************************************
 * @defgroup Enums 
 *****************************************************************************/

/**
 * @brief Synchronous flag for uart communication
 */
enum {
    SDU_SYNC_HEAD_LSB = 0x55,
    SDU_SYNC_HEAD_MSB = 0xAA,
    SDU_SYNC_TAIL = 0xFF,
};

/**
 * @brief Used for analysis of uart package. Only for serial communication 
 *    links state definition in finite state machine
 */
enum {
    SDU_WAIT_HEAD_LSB = 0,
    SDU_WAIT_HEAD_MSB = 1,
    SDU_WAIT_SEQ_LSB  = 2,
    SDU_WAIT_SEQ_MSB  = 3,
    SDU_WAIT_CMD_TYPE = 4,
    SDU_WAIT_LEN_LSB  = 5,
    SDU_WAIT_LEN_MSB  = 6,
    SDU_WAIT_DATABODY = 7,
    SDU_WAIT_CHECKSUM = 8,
    SDU_WAIT_FRAMEEND = 9,
    SDU_RECV_FINISHED = 0x80,
};

/**
 * @brief Public error code definition
 */
enum {
    ACK_ERR_FORBID_CMD = 0x01, //!< Current work mode DONOT support this command
    ACK_ERR_COMMANDS = 0x02,   //!< illegal command set
    ACK_ERR_CMD_TYPE = 0x03,   //!< Type of illegal command in current command set
    ACK_ERR_DATASIZE = 0x04,   //!< Datasize error
};

typedef enum
{
    FRAME_MODE_CHANGE = 0x51,
    FRAME_MODE_GET    = 0x52,  //!< APP or BTR
    FRAME_READ        = 0x53,
    FRAME_WRITE       = 0x54,
    FRAME_DFU         = 0x55,
    FRAME_APP_CRC     = 0x56,
    FRAME_VERSION_GET = 0x57,
} FrameType;


/*******************************************************************************
 * @defgroup Structs 
 ******************************************************************************/

#pragma pack(push, 1)

/**
 * @brief PDU (Protocol Data Unit)
 */
typedef struct {
    uint8_t type;
    uint8_t dummy;
    uint16_t len;
    uint8_t data[PDU_DATA_SIZE];
} pdu_t;

/**
 * @brief SDU, application data unit based on serial commuication mode
 */
typedef struct {
    uint8_t sync_lsb;
    uint8_t sync_msb;
    uint16_t seq;
    pdu_t pdu;
    uint8_t checksum;
} sdu_t;

/**
 * @brief SDU context(ctx): communication state structure
 */
typedef struct {
    sdu_t sdu;              //!< SDU buffer
    volatile uint8_t state;  //!< communication status indicator
    uint8_t index;           //!< data field received indicator
} sdu_ctx_t;

#pragma pack(pop)

/******************************************************************************
 * @defgroup declare variables 
 *****************************************************************************/
extern sdu_ctx_t g_rx_buf;

extern volatile bool g_rx_recv_flag;
extern volatile uint16_t g_frame_timeout;

/******************************************************************************
 * @defgroup function prototypes 
 *****************************************************************************/
extern uint8_t sdu_length(const sdu_t *p_sdu);
extern void sdu_ctx_init(sdu_ctx_t *p_sdu_ctx);
extern void sdu_send (uint8_t cmd, void * data, uint8_t sz);
extern void sdu_cmd_ack(uint8_t cmd);
extern void sdu_cmd_ack_flag (uint8_t cmd, uint8_t flag);
extern bool sdu_analyze(sdu_ctx_t *p_sdu_ctx, uint8_t byte);
extern void protocol_data_parse(sdu_ctx_t *p_sdu_ctx);

#endif /* __PROTOCOL_H__ */
