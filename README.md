<p align="center">
  <h1 align="center">Modbus C library</h1>
</p>

<p align="right">
  <img alt="Test passing" src="https://github.com/DrDeLaBill/modbus_rtu_puk/workflows/C-Build-Test/badge.svg">
</p>

Modbus master-slave RTU library for microcontrollers.

### Master functions

Sets user request function for sending request data array:

```modbus_master_set_request_data_sender(request_data_sender)```

Sets user function that calls when modbus master has internal error:

```modbus_master_set_internal_error_handler(response_error_handler)```

Sets user function that called when the response receives:

```modbus_master_set_response_packet_handler(response_packet_handler)```

Function that must be called when a new byte received:

```modbus_master_recieve_data_byte(byte)```

Calls when the response waiting time is running out:

```modbus_master_timeout()```

Request functions:
```C
modbus_master_read_coils(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count);
modbus_master_read_input_status(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count);
modbus_master_read_holding_registers(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count);
modbus_master_read_input_registers(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count);
modbus_master_force_single_coil(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_val);
modbus_master_preset_single_register(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_val);
modbus_master_force_multiple_coils(uint8_t slave_id, uint16_t reg_addr, bool* data, uint16_t reg_count);
modbus_master_preset_multiple_registers(uint8_t slave_id, uint16_t reg_addr, uint16_t* data, uint16_t reg_count);
```

### Master example:
```C
#include <stdio.h>
#include <stdint.h>

#include "modbus_rtu_master.h"


void request_data_sender(uint8_t* data, uint8_t len)
{
    for (int i = 0; i < len; i++) {
        // send_request_byte_function(data[i]);
    }
}

void master_internal_error_handler()
{
    printf("MASTER ERROR\n");
}

void response_packet_handler(modbus_response_t* packet)
{
    /* YOUR CODE */
    printf("Recieved data: ");
    for (int i = 0; i < MODBUS_MESSAGE_DATA_SIZE; i++) {
        printf("%02x %02x   ", (uint8_t)(packet->response[i] >> 8), (uint8_t)(packet->response[i]));
    }
    printf("\n");

    if (packet->status == MODBUS_NO_ERROR) {
        printf("SUCCESS\n");
    } else {
        printf("ERROR\n");
    }
}


int main()
{
    // Setup modbus master
    modbus_master_set_request_data_sender(&request_data_sender);
    modbus_master_set_response_packet_handler(&response_packet_handler);
    modbus_master_set_internal_error_handler(&master_internal_error_handler);

    // Send request
    modbus_master_read_coils(0x01, 0x0000, 0x0000);

    while (/* new byte available */) {
        // modbus_master_recieve_data_byte(new byte);
    }

    return 0;
}
```

### Slave example:
```C
#include <stdio.h>
#include <stdint.h>

#include "modbus_rtu_slave.h"


void response_data_handler(uint8_t* data, uint8_t len)
{
    for (int i = 0; i < len; i++) {
        // send_response_byte_function(data[i]);
    }
}

void slave_internal_error_handler()
{
    printf("SLAVE ERROR\n");
}

int main()
{
    // Setup modbus slave
    modbus_slave_set_slave_id(0x01);
    modbus_slave_set_response_data_handler(&response_data_handler);
    modbus_slave_set_request_error_handler(&slave_internal_error_handler);
	
	return 0;
}
```
