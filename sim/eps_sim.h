#ifndef EPS_SIM_H
#define EPS_SIM_H

//#include <stdint.h>

#include "eps_device.h"
#include "simulith.h"
#include "simulith_42_context.h"
#include "simulith_42_commands.h"

// Configuration parameters
#define EPS_SIM_UART_ID 5
#define EPS_SIM_UPDATE_RATE_HZ 10

// Status codes
#define EPS_SIM_SUCCESS 0
#define EPS_SIM_ERROR  1

// Eps simulator state
typedef struct 
{
    // Communication handles
    uint8_t uart_port;
    uint32_t uart_handle;
    void* time_handle;
    // Simulator specifics
    double last_update_time;
    // Device specifics
    EPS_Device_HK_tlm_t hk;
    EPS_Device_Data_tlm_t data;
} eps_sim_state_t;

// Function declarations
static void send_housekeeping(eps_sim_state_t* state);
static void send_eps_data(eps_sim_state_t* state);
static void handle_command(eps_sim_state_t* state, const uint8_t* data, size_t length);
static void eps_sim_on_tick(uint64_t tick_time_ns, const simulith_42_context_t* context_42);
int eps_sim_init(eps_sim_state_t* state);
void eps_sim_cleanup(eps_sim_state_t* state);

#endif /* EPS_SIM_H */ 