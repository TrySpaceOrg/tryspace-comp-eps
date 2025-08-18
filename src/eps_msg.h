#ifndef _EPS_MSG_H_
#define _EPS_MSG_H_

#include "cfe.h"
#include "eps_device.h"

/*
** Ground Command Codes
*/
#define EPS_NOOP_CC           0
#define EPS_RESET_COUNTERS_CC 1
#define EPS_ENABLE_CC         2
#define EPS_DISABLE_CC        3
#define EPS_CONFIG_CC         4

/*
** Telemetry Request Command Codes
*/
#define EPS_REQ_HK_TLM   0
#define EPS_REQ_DATA_TLM 1

/*
** Generic "no arguments" command type definition
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;

} EPS_NoArgs_cmd_t;

/*
** EPS write configuration command
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
    uint16                  DeviceCfg;

} EPS_Config_cmd_t;

/*
** EPS device telemetry definition
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    EPS_Device_Data_tlm_t    Eps;

} __attribute__((packed)) EPS_Device_tlm_t;
#define EPS_DEVICE_TLM_LNGTH sizeof(EPS_Device_tlm_t)

/*
** EPS housekeeping type definition
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    uint8                     CommandErrorCount;
    uint8                     CommandCount;
    uint8                     DeviceErrorCount;
    uint8                     DeviceCount;

    /*
    ** Edit and add specific telemetry values to this struct
    */
    uint8                     DeviceEnabled;
    EPS_Device_HK_tlm_t      DeviceHK;

} __attribute__((packed)) EPS_Hk_tlm_t;
#define EPS_HK_TLM_LNGTH sizeof(EPS_Hk_tlm_t)

#endif /* _EPS_MSG_H_ */
