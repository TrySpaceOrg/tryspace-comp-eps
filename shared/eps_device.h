#ifndef _EPS_DEVICE_H_
#define _EPS_DEVICE_H_

/*
** Required header files.
*/
#include "device_cfg.h"
#include "hwlib.h"

/*
** Type definitions
*/
#define EPS_DEVICE_HDR_0 0xC0
#define EPS_DEVICE_HDR_1 0xFF
#define EPS_DEVICE_HDR   ((EPS_DEVICE_HDR_0 << 8) | EPS_DEVICE_HDR_1)

#define EPS_DEVICE_NOOP_CMD     0x00
#define EPS_DEVICE_REQ_HK_CMD   0x01
#define EPS_DEVICE_REQ_DATA_CMD 0x02
#define EPS_DEVICE_CFG_CMD      0x03

#define EPS_DEVICE_TRAILER_0 0xFE
#define EPS_DEVICE_TRAILER_1 0xFE
#define EPS_DEVICE_TRAILER   ((EPS_DEVICE_TRAILER_0 << 8) | EPS_DEVICE_TRAILER_1)

#define EPS_DEVICE_CMD_SIZE    8
#define EPS_DEVICE_HDR_TRL_LEN 4

/*
** EPS device housekeeping telemetry definition
*/
typedef struct
{
    uint16_t DeviceCounter;
    uint16_t DeviceConfig;

} __attribute__((packed)) EPS_Device_HK_tlm_t;
#define EPS_DEVICE_HK_LNGTH sizeof(EPS_Device_HK_tlm_t)
#define EPS_DEVICE_HK_SIZE  EPS_DEVICE_HK_LNGTH + EPS_DEVICE_HDR_TRL_LEN

/*
** EPS device data telemetry definition
*/
typedef struct
{
    uint16_t Chan1;
    uint16_t Chan2;
    uint16_t Chan3;

} __attribute__((packed)) EPS_Device_Data_tlm_t;
#define EPS_DEVICE_DATA_LNGTH sizeof(EPS_Device_Data_tlm_t)
#define EPS_DEVICE_DATA_SIZE  EPS_DEVICE_DATA_LNGTH + EPS_DEVICE_HDR_TRL_LEN

/*
** Prototypes
*/
int32_t EPS_ReadData(uart_info_t *device, uint8_t *read_data, uint8_t data_length);
int32_t EPS_CommandDevice(uart_info_t *device, uint16_t cmd, uint16_t payload);
int32_t EPS_RequestHK(uart_info_t *device, EPS_Device_HK_tlm_t *data);
int32_t EPS_RequestData(uart_info_t *device, EPS_Device_Data_tlm_t *data);

#endif /* _EPS_DEVICE_H_ */
