#include "eps_app_coveragetest_common.h"

void Test_EPS_ReadData(void)
{
    i2c_bus_info_t device;
    uint8_t        read_data[8];
    uint8_t        data_length = 8;

    /* There is no EPS_ReadData in the I2C-based implementation; use i2c_read_transaction directly in tests */
    i2c_read_transaction(&device, EPS_CFG_I2C_DEVICE_ADDR, read_data, data_length, 0);

    UT_SetDeferredRetcode(UT_KEY(i2c_read_transaction), 1, data_length);
    i2c_read_transaction(&device, EPS_CFG_I2C_DEVICE_ADDR, read_data, data_length, 0);

    UT_SetDeferredRetcode(UT_KEY(i2c_read_transaction), 1, data_length + 1);
    i2c_read_transaction(&device, EPS_CFG_I2C_DEVICE_ADDR, read_data, data_length, 0);
}

void Test_EPS_CommandDevice(void)
{
    i2c_bus_info_t device;
    uint8_t       cmd_code = 0;
    uint8_t       payload  = 0;

    EPS_CommandDevice(&device, cmd_code, payload);

    UT_SetDeferredRetcode(UT_KEY(i2c_write_transaction), 1, I2C_ERROR);
    EPS_CommandDevice(&device, cmd_code, payload);

    UT_SetDeferredRetcode(UT_KEY(i2c_write_transaction), 1, EPS_COMMAND_SIZE);
    EPS_CommandDevice(&device, cmd_code, payload);

    UT_SetDeferredRetcode(UT_KEY(i2c_write_transaction), 1, EPS_COMMAND_SIZE);
    UT_SetDeferredRetcode(UT_KEY(i2c_read_transaction), 1, 9);
    UT_SetDeferredRetcode(UT_KEY(i2c_read_transaction), 1, 9);
    UT_SetDefaultReturnValue(UT_KEY(i2c_read_transaction), I2C_SUCCESS);
    UT_SetDeferredRetcode(UT_KEY(i2c_read_transaction), 1, I2C_SUCCESS);
    EPS_CommandDevice(&device, cmd_code, payload);
}

void Test_EPS_RequestHK(void)
{
    i2c_bus_info_t      device;
    EPS_Device_HK_tlm_t data;

    EPS_RequestHK(&device, &data);

    uint8_t read_data[] = {0xDE, 0xAD, 0x00, 0x00, 0x00, 0x07, 0x00, 0x06,
                           0x00, 0x0C, 0x00, 0x12, 0x00, 0x00, 0xBE, 0xEF};
    UT_SetDeferredRetcode(UT_KEY(i2c_read_transaction), 1, 16);
    UT_SetDeferredRetcode(UT_KEY(i2c_read_transaction), 1, 16);
    UT_SetDataBuffer(UT_KEY(i2c_read_transaction), &read_data, sizeof(read_data), false);
    EPS_RequestHK(&device, &data);

    UT_SetDeferredRetcode(UT_KEY(i2c_write_transaction), 1, I2C_ERROR);
    EPS_RequestHK(&device, &data);
}

/*
 * Setup function prior to every test
 */
void Eps_UT_Setup(void)
{
    UT_ResetState(0);
}

/*
 * Teardown function after every test
 */
void Eps_UT_TearDown(void) {}

/*
 * Register the test cases to execute with the unit test tool
 */
void UtTest_Setup(void)
{
    ADD_TEST(EPS_ReadData);
    ADD_TEST(EPS_CommandDevice);
    ADD_TEST(EPS_RequestHK);
}