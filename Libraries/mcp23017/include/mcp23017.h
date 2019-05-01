/*
 * mcp32017.h
 *
 *  Created on: 29.04.2019
 *      Author: bertw
 */

#ifndef SRC_MCP23017_H_
#define SRC_MCP23017_H_

#include <libopencm3/stm32/i2c.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint32_t mI2c;
	uint16_t mRegs[11];
	uint8_t mAddr;
} Mcp23017;

#define Mcp23017_slave_address(a0,a1,a2) ((1U<<5) | ((a2)<<2) | ((a1)<<1) | (a0))

#define MCP23017_PORT_PINS_LOW 0U
#define MCP23017_PORT_PINS_HIGH 0xffff
#define MCP23017_PORTA_MASK 0x00ff
#define MCP23017_PORTB_MASK 0xff00

Mcp23017 *Mcp23017_construct_out(// setup object with all pins as output
		void *memory, // uninitialized object (sizeof Mcp23017)
		uint32_t i2c, // the I2C port handle of libopencm3
		uint8_t slaveAddress, // slave address generated by macro Mcp23017_SLAVE_ADDRESS
		uint16_t gpo_values // initial values of output pins
		);

Mcp23017 *Mcp23017_construct( // setup object
		void *memory, // uninitialized object (sizeof Mcp23017)
		uint32_t i2c, // the I2C port handle of libopencm3
		uint8_t slaveAddress, // slave address generated by macro Mcp23017_SLAVE_ADDRESS
		uint16_t gpio_direction, // 1=input
		uint16_t gpo_values, // initial values of output pins
		uint16_t gpi_polarity, // 1=inverse
		uint16_t gpi_pullup, // 1=100kOhm pull-up resistor
		uint16_t gpi_interrupt_enable, // 1=enable
		uint16_t gpi_default_values, // set default values to compare for generating input interrupts
		uint16_t gpi_use_default_values, // 1=default_values, 0=previous_state
		bool intpin_mirror, // IntA/IntB pins are 1=connected or 0=separate for PortA/PortB
		bool intpin_open_drain, //IntA/B pins are true=OpenDrain, false=PushPull (with polarity set by next parameter)
		bool intpin_reverse_output_polarity // true=Active-High, false=Active-Low
		);


void Mcp23017_destruct(Mcp23017 *obj);
bool Mcp23017_putBit(Mcp23017 *obj, uint8_t bitNumber, bool value);
bool Mcp23017_getBit(Mcp23017 *obj, uint8_t bitNumber, bool cached);
bool Mcp23017_putBits(Mcp23017 *obj, uint16_t bitMask, uint16_t values);
uint16_t Mcp23017_getBits(Mcp23017 *obj, uint16_t bitMask, bool cached);
bool Mcp23017_forceWrite(Mcp23017 *obj);
bool Mcp23017_statusOk(Mcp23017 *obj);




#endif /* SRC_MCP23017_H_ */
