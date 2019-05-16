/*
 * uart.h
 *
 *  Created on: 15.05.2019
 *      Author: bertw
 */

#ifndef UART_H_
#define UART_H_

int esp32_write(const char *data, unsigned data_len);
int esp32_read(char *buf, unsigned buf_size);
int esp32_getc(void);
void uart_setup(void);

#endif /* UART_H_ */
