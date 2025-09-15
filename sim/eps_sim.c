#include "eps_sim.h"

/*
** Function to handle EPS I2C commands
*/
static void handle_eps_command(eps_sim_state_t* state, const uint8_t* data, size_t length)
{
    if (length < EPS_COMMAND_SIZE)
    {
        printf("EPS SIM: Command too short (%zu bytes, expected %zu)\n", length, EPS_COMMAND_SIZE);
        return;
    }

    EPS_Command_t* cmd = (EPS_Command_t*)data;
    
    /* Verify I2C address */
    if (cmd->i2c_addr != EPS_CFG_I2C_DEVICE_ADDR)
    {
        printf("EPS SIM: Wrong I2C address 0x%02X (expected 0x%02X)\n", cmd->i2c_addr, EPS_CFG_I2C_DEVICE_ADDR);
        return;
    }

    /* Verify CRC */
    if (!EPS_Verify_CRC8(data, EPS_COMMAND_SIZE - 1, cmd->crc))
    {
        printf("EPS SIM: CRC check failed\n");
        return;
    }

    #ifdef EPS_CFG_DEBUG
    printf("EPS SIM: Received command 0x%02X with payload 0x%02X\n", cmd->command, cmd->payload);
    #endif

    switch (cmd->command)
    {
        case EPS_CMD_NOOP:
            #ifdef EPS_CFG_DEBUG
            printf("EPS SIM: NOOP command\n");
            #endif
            break;

        case EPS_CMD_GET_HK:
            #ifdef EPS_CFG_DEBUG
            printf("EPS SIM: Housekeeping request\n");
            #endif
            /* Calculate CRC for housekeeping data */
            state->hk.crc = EPS_Calculate_CRC8((const uint8_t*)&state->hk, sizeof(state->hk) - 1);
            /* Send housekeeping data back via I2C */
            if (simulith_transport_send(&state->i2c_device, (const uint8_t*)&state->hk, sizeof(state->hk)) < 0)
            {
                printf("EPS SIM: Failed to send housekeeping data\n");
            }
            #ifdef EPS_CFG_DEBUG
            else
            {
                printf("EPS SIM: Sent housekeeping data (%zu bytes)\n", sizeof(state->hk));
            }
            #endif
            break;

        case EPS_CMD_SWITCH_OFF:
            if (cmd->payload < EPS_NUM_SWITCHES)
            {
                state->hk.switches[cmd->payload].state = EPS_SWITCH_OFF;
                #ifdef EPS_CFG_DEBUG
                printf("EPS SIM: Switch %d turned OFF\n", cmd->payload);
                #endif
            }
            #ifdef EPS_CFG_DEBUG
            else
            {
                printf("EPS SIM: Invalid switch number %d\n", cmd->payload);
            }
            #endif
            break;

        case EPS_CMD_SWITCH_ON:
            if (cmd->payload < EPS_NUM_SWITCHES)
            {
                state->hk.switches[cmd->payload].state = EPS_SWITCH_ON;
                #ifdef EPS_CFG_DEBUG
                printf("EPS SIM: Switch %d turned ON\n", cmd->payload);
                #endif
            }
            #ifdef EPS_CFG_DEBUG
            else
            {
                printf("EPS SIM: Invalid switch number %d\n", cmd->payload);
            }
            #endif
            break;

        default:
            printf("EPS SIM: Unknown command 0x%02X\n", cmd->command);
            break;
    }

    /* Update device counter for any command */
    state->device_counter++;
}

/*
** Tick callback for simulation updates
*/
static void eps_component_tick(component_state_t* state, uint64_t tick_time_ns, const simulith_42_context_t* context_42)
{
    eps_sim_state_t* eps_state = (eps_sim_state_t*)state;
    if (!eps_state)
    {
        return;
    }

    static uint64_t last_hk_update = 0;
    const uint64_t hk_update_interval = 1000000000ULL; /* 1 second in nanoseconds */
    
    /* Update housekeeping data every second */
    if (tick_time_ns - last_hk_update >= hk_update_interval)
    {
        /* Update battery/solar voltage and temperature with random slight variation (+1, 0, or -1) */
        int v_delta = (rand() % 3) - 1; // -1, 0, or +1
        int t_delta = (rand() % 3) - 1; // -1, 0, or +1
        eps_state->hk.battery_voltage = 165 + v_delta;
        eps_state->hk.battery_temperature = 20 + t_delta;
        v_delta = (rand() % 3) - 1;
        t_delta = (rand() % 3) - 1;
        eps_state->hk.solar_voltage = 180 + v_delta;
        eps_state->hk.solar_temperature = 35 + t_delta;
        
        /* Update switch voltages and currents based on state */
        for (int i = 0; i < EPS_NUM_SWITCHES; i++)
        {
            if (eps_state->hk.switches[i].state == EPS_SWITCH_ON)
            {
                /* Set voltage according to switch index, convert to counts (32V/255 per count) */
                float voltage = 0.0f;
                if (i == 0 || i == 1)
                    voltage = 3.3f;
                else if (i == 2 || i == 3)
                    voltage = 5.0f;
                else if (i == 4 || i == 5)
                    voltage = 12.0f;
                else if (i == 6 || i == 7)
                    voltage = 24.0f;
                uint8_t voltage_count = (uint8_t)(voltage / (32.0f / 255.0f));
                eps_state->hk.switches[i].voltage = voltage_count;
                /* Current stays low as nothing is connected, convert to counts (10A/255 per count) */
                float current = 0.05f; /* 0.05A, example low value */
                uint8_t current_count = (uint8_t)(current / (10.0f / 255.0f));
                eps_state->hk.switches[i].current = current_count;
            }
            else
            {
                eps_state->hk.switches[i].voltage = 0;
                eps_state->hk.switches[i].current = 0;
            }
        }

        #ifdef EPS_CFG_DEBUG
        printf("EPS SIM: HK updated - Battery %d/255 V, %d/255 C; Solar %d/255 V, %d/255 C\n",
               eps_state->hk.battery_voltage, eps_state->hk.battery_temperature,
               eps_state->hk.solar_voltage, eps_state->hk.solar_temperature);
        #endif

        last_hk_update = tick_time_ns;
    }
        
    int available = simulith_transport_available(&eps_state->i2c_device);
    if (available > 0)
    {
        uint8_t cmd_buffer[256];
        int bytes_read = simulith_transport_receive(&eps_state->i2c_device, cmd_buffer, sizeof(cmd_buffer));
        if (bytes_read > 0)
        {
            handle_eps_command(eps_state, cmd_buffer, bytes_read);
        }
    }
}

/*
** Initialize EPS simulation
*/
int eps_sim_init(eps_sim_state_t* state)
{
    if (!state)
    {
        printf("EPS SIM: NULL state pointer\n");
        return -1;
    }

    /* Initialize housekeeping data */
    memset(&state->hk, 0, sizeof(state->hk));
    state->device_counter = 0;
    
    /* Set initial values */
    state->hk.battery_voltage = 165;
    state->hk.battery_temperature = 20;
    state->hk.solar_voltage = 180;
    state->hk.solar_temperature = 35;
    
    /* Initialize all switches to OFF */
    for (int i = 0; i < EPS_NUM_SWITCHES; i++)
    {
        state->hk.switches[i].state = EPS_SWITCH_OFF;
        state->hk.switches[i].voltage = 0;
        state->hk.switches[i].current = 0;
    }

    printf("EPS SIM: Initialized\n");
    return 0;
}

/*
** Cleanup EPS simulation
*/
void eps_sim_cleanup(eps_sim_state_t* state)
{
    if (state)
    {
        memset(state, 0, sizeof(*state));
    }
    printf("EPS SIM: Cleaned up\n");
}

/*
** Component initialization for simulith framework
*/
static int eps_component_init(component_state_t** state)
{    
    /* Allocate component state */
    eps_sim_state_t* eps_state = (eps_sim_state_t*)malloc(sizeof(eps_sim_state_t));
    if (!eps_state)
    {
        printf("EPS SIM: Failed to allocate component state\n");
        return COMPONENT_ERROR;
    }
    
    /* Initialize simulation state */
    if (eps_sim_init(eps_state) != 0)
    {
        printf("EPS SIM: Failed to initialize simulation state\n");
        free(eps_state);
        return COMPONENT_ERROR;
    }
    
    /* Initialize I2C device */
    memset(&eps_state->i2c_device, 0, sizeof(eps_state->i2c_device));
    /* Set up ZMQ address for this device */
    snprintf(eps_state->i2c_device.name, sizeof(eps_state->i2c_device.name), 
        "eps_sim_bus%d_addr0x%02X", EPS_CFG_I2C_BUS_ID, EPS_CFG_I2C_DEVICE_ADDR);
    snprintf(eps_state->i2c_device.address, sizeof(eps_state->i2c_device.address), 
        "ipc:///tmp/simulith_pub:%d", SIMULITH_I2C_BASE_PORT + EPS_CFG_I2C_BUS_ID * 100 + EPS_CFG_I2C_DEVICE_ADDR);
    eps_state->i2c_device.is_server = 1;  // Always server/bind for the simulator

    if (simulith_transport_init(&eps_state->i2c_device) != 0)
    {
        printf("EPS SIM: Failed to initialize I2C device\n");
        eps_sim_cleanup(eps_state);
        free(eps_state);
        return COMPONENT_ERROR;
    }
    
    *state = (component_state_t*)eps_state;
    printf("EPS SIM: Initialized successfully as %s\n", eps_state->i2c_device.name);
    return COMPONENT_SUCCESS;
}

/*
** Component cleanup for simulith framework
*/
static void eps_component_cleanup(component_state_t* state)
{
    eps_sim_state_t* eps_state = (eps_sim_state_t*)state;
    if (eps_state)
    {
        simulith_transport_close(&eps_state->i2c_device);
        eps_sim_cleanup(eps_state);
        free(eps_state);
    }
    printf("EPS SIM: Component cleaned up\n");
}

/*
** Component interface definition
*/
static const component_interface_t eps_component_interface = {
    .name = "eps_sim",
    .description = "EPS component simulation with I2C interface",
    .init = eps_component_init,
    .tick = eps_component_tick,
    .cleanup = eps_component_cleanup
};

/*
** Component registration function required by simulith director
*/
const component_interface_t* get_component_interface(void)
{
    return &eps_component_interface;
}
