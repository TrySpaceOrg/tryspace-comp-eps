#include "utgenstub.h"
#include "eps_device.h"

int32_t EPS_ReadData(uart_info_t *device, uint8_t *read_data, uint8_t data_length)
{
    UT_GenStub_SetupReturnBuffer(EPS_ReadData, int32_t);

    UT_GenStub_AddParam(EPS_ReadData, uart_info_t *, device);
    UT_GenStub_AddParam(EPS_ReadData, uint8_t *, read_data);
    UT_GenStub_AddParam(EPS_ReadData, uint8_t, data_length);

    UT_GenStub_Execute(EPS_ReadData, Basic, NULL);

    return UT_GenStub_GetReturnValue(EPS_ReadData, int32_t);
}

int32_t EPS_CommandDevice(uart_info_t *device, uint16_t cmd, uint16_t payload)
{
    UT_GenStub_SetupReturnBuffer(EPS_CommandDevice, int32_t);

    UT_GenStub_AddParam(EPS_CommandDevice, uart_info_t *, device);
    UT_GenStub_AddParam(EPS_CommandDevice, uint8_t, cmd);
    UT_GenStub_AddParam(EPS_CommandDevice, uint32_t, payload);

    UT_GenStub_Execute(EPS_CommandDevice, Basic, NULL);

    return UT_GenStub_GetReturnValue(EPS_CommandDevice, int32_t);
}

int32_t EPS_RequestHK(uart_info_t *device, EPS_Device_HK_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(EPS_RequestHK, int32_t);

    UT_GenStub_AddParam(EPS_RequestHK, uart_info_t *, device);
    UT_GenStub_AddParam(EPS_RequestHK, EPS_Device_HK_tlm_t *, data);

    UT_GenStub_Execute(EPS_RequestHK, Basic, NULL);

    return UT_GenStub_GetReturnValue(EPS_RequestHK, int32_t);
}

int32_t EPS_RequestData(uart_info_t *device, EPS_Device_Data_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(EPS_RequestData, int32_t);

    UT_GenStub_AddParam(EPS_RequestData, uart_info_t *, device);
    UT_GenStub_AddParam(EPS_RequestData, EPS_Device_Data_tlm_t *, data);

    UT_GenStub_Execute(EPS_RequestData, Basic, NULL);

    return UT_GenStub_GetReturnValue(EPS_RequestData, int32_t);
}
