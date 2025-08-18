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
    if (cmd->i2c_addr != EPS_I2C_DEVICE_ADDR)
    {
        printf("EPS SIM: Wrong I2C address 0x%02X (expected 0x%02X)\n", cmd->i2c_addr, EPS_I2C_DEVICE_ADDR);
        return;
    }

    /* Verify CRC */
    if (!EPS_Verify_CRC8(data, EPS_COMMAND_SIZE - 1, cmd->crc))
    {
        printf("EPS SIM: CRC check failed\n");
        return;
    }

    printf("EPS SIM: Received command 0x%02X with payload 0x%02X\n", cmd->command, cmd->payload);

    switch (cmd->command)
    {
        case EPS_CMD_NOOP:
            printf("EPS SIM: NOOP command\n");
            break;

        case EPS_CMD_GET_HK:
            printf("EPS SIM: Housekeeping request\n");
            /* Calculate CRC for housekeeping data */
            state->hk.crc = EPS_Calculate_CRC8((const uint8_t*)&state->hk, sizeof(state->hk) - 1);
            /* Send housekeeping data back via I2C */
            if (simulith_i2c_write(&state->i2c_device, (const uint8_t*)&state->hk, sizeof(state->hk)) < 0)
            {
                printf("EPS SIM: Failed to send housekeeping data\n");
            }
            else
            {
                printf("EPS SIM: Sent housekeeping data (%zu bytes)\n", sizeof(state->hk));
            }
            break;

        case EPS_CMD_SWITCH_OFF:
            if (cmd->payload < EPS_NUM_SWITCHES)
            {
                state->hk.switches[cmd->payload].state = EPS_SWITCH_OFF;
                printf("EPS SIM: Switch %d turned OFF\n", cmd->payload);
            }
            else
            {
                printf("EPS SIM: Invalid switch number %d\n", cmd->payload);
            }
            break;

        case EPS_CMD_SWITCH_ON:
            if (cmd->payload < EPS_NUM_SWITCHES)
            {
                state->hk.switches[cmd->payload].state = EPS_SWITCH_ON;
                printf("EPS SIM: Switch %d turned ON\n", cmd->payload);
            }
            else
            {
                printf("EPS SIM: Invalid switch number %d\n", cmd->payload);
            }
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
    static uint64_t last_cmd_check = 0;
    const uint64_t hk_update_interval = 1000000000ULL; /* 1 second in nanoseconds */
    const uint64_t cmd_check_interval = 100000000ULL;  /* 100ms in nanoseconds */
    
    /* Check for incoming I2C commands */
    if (tick_time_ns - last_cmd_check >= cmd_check_interval)
    {
        uint8_t cmd_buffer[256];
        int bytes_read = simulith_i2c_read(&eps_state->i2c_device, cmd_buffer, sizeof(cmd_buffer));
        if (bytes_read > 0)
        {
            printf("EPS SIM: Received %d bytes via I2C\n", bytes_read);
            handle_eps_command(eps_state, cmd_buffer, bytes_read);
        }
        last_cmd_check = tick_time_ns;
    }
    
    /* Update housekeeping data every second */
    if (tick_time_ns - last_hk_update >= hk_update_interval)
    {
        /* Update battery voltage (simulate slight variation) */
        eps_state->hk.battery_voltage = 200 + (eps_state->device_counter % 20);
        
        /* Update battery temperature */
        eps_state->hk.battery_temperature = 100 + (eps_state->device_counter % 10);
        
        /* Update solar voltage */
        eps_state->hk.solar_voltage = 180 + (eps_state->device_counter % 30);
        
        /* Update solar temperature */
        eps_state->hk.solar_temperature = 80 + (eps_state->device_counter % 15);
        
        /* Update switch voltages and currents based on state */
        for (int i = 0; i < EPS_NUM_SWITCHES; i++)
        {
            if (eps_state->hk.switches[i].state == EPS_SWITCH_ON)
            {
                eps_state->hk.switches[i].voltage = 240 + (i * 2);  /* ~30V */
                eps_state->hk.switches[i].current = 25 + i;         /* ~1A */
            }
            else
            {
                eps_state->hk.switches[i].voltage = 0;
                eps_state->hk.switches[i].current = 0;
            }
        }
        
        last_hk_update = tick_time_ns;
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
    state->hk.battery_voltage = 200;      /* ~25V */
    state->hk.battery_temperature = 100;  /* ~98C */
    state->hk.solar_voltage = 180;        /* ~22.5V */
    state->hk.solar_temperature = 80;     /* ~78C */
    
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
    printf("EPS SIM: Starting EPS simulation component\n");
    
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
    eps_state->i2c_device.bus_id = EPS_I2C_BUS_ID;
    eps_state->i2c_device.device_addr = EPS_I2C_DEVICE_ADDR;
    eps_state->i2c_device.is_server = 1;  /* This simulation acts as the I2C device (server) */
    
    /* Set up ZMQ address for this device */
    snprintf(eps_state->i2c_device.address, sizeof(eps_state->i2c_device.address), 
             "tcp://*:%d", SIMULITH_I2C_BASE_PORT + eps_state->i2c_device.bus_id * 100 + eps_state->i2c_device.device_addr);
    snprintf(eps_state->i2c_device.name, sizeof(eps_state->i2c_device.name), 
             "eps_sim_bus%d_addr0x%02X", eps_state->i2c_device.bus_id, eps_state->i2c_device.device_addr);
    
    if (simulith_i2c_init(&eps_state->i2c_device) != 0)
    {
        printf("EPS SIM: Failed to initialize I2C device\n");
        eps_sim_cleanup(eps_state);
        free(eps_state);
        return COMPONENT_ERROR;
    }
    
    *state = (component_state_t*)eps_state;
    printf("EPS SIM: Component initialized successfully\n");
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
        simulith_i2c_close(&eps_state->i2c_device);
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
    .description = "EPS device simulation component with I2C interface",
    .init = eps_component_init,
    .tick = eps_component_tick,
    .cleanup = eps_component_cleanup,
    .configure = NULL  /* No configuration needed */
};

/*
** Component registration function required by simulith director
*/
const component_interface_t* get_component_interface(void)
{
    return &eps_component_interface;
}

/*
** Standalone tick function for main execution
*/
static void eps_standalone_tick(uint64_t tick_time_ns)
{
    static component_state_t* g_state = NULL;
    
    /* Initialize on first call */
    if (!g_state)
    {
        if (eps_component_init(&g_state) != COMPONENT_SUCCESS)
        {
            printf("EPS SIM: Failed to initialize component state\n");
            return;
        }
    }
    
    /* Call the component tick function */
    eps_component_tick(g_state, tick_time_ns, NULL);
}

/*
** Main function for standalone execution
*/
int main(void)
{
    printf("EPS SIM: Starting standalone EPS simulation\n");
    
    /* Run the simulith client loop */
    simulith_client_run_loop(eps_standalone_tick);
    
    /* Note: Cleanup will happen when the loop exits */
    return 0;
}
