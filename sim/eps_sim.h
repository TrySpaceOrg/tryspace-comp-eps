#ifndef _EPS_SIM_H_
#define _EPS_SIM_H_

#include "eps_device.h"
#include "simulith.h"
#include "simulith_component.h"

/*
** EPS simulation state structure
*/
typedef struct 
{
    EPS_Device_HK_tlm_t hk;         /* Housekeeping telemetry */
    uint32_t device_counter;        /* Device counter */
    transport_port_t i2c_device;    /* I2C device handle */
} eps_sim_state_t;

/*
** Function prototypes
*/
int eps_sim_init(eps_sim_state_t* state);
void eps_sim_cleanup(eps_sim_state_t* state);

#endif /* _EPS_SIM_H_ */
