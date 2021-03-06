/**
 * \file boards/dlb8.h
 * \brief "LED&KEY" board with 8 LED-digits, 8 Mono-LEDs, 8 buttons. IC: TM1638 (IIC Bus)
 * \date  02.05.2019
 * \author bertw
 */
#include "../tm1638.h"

#ifndef TM1638_INCLUDE_BOARD_D8L8K8_H_
#define TM1638_INCLUDE_BOARD_D8L8K8_H_

//
#define LED_KEY_ADDR_DIGIT(n) (TM1638_REG_OUTPUT + (n)*2)
#define LED_KEY_ADDR_LED(n) (TM1638_REG_OUTPUT + (n)*2 +1)
#define LED_KEY_POS_TO_REG(pos) ((pos)*2)


/// \brief bits in output of Tm1638_read() ... not needed for dlb8_get_button()
#define LED_KEY_BUTTON_0 0x00000001
#define LED_KEY_BUTTON_1 0x00000100
#define LED_KEY_BUTTON_2 0x00010000
#define LED_KEY_BUTTON_3 0x01000000
#define LED_KEY_BUTTON_4 0x00000010
#define LED_KEY_BUTTON_5 0x00001000
#define LED_KEY_BUTTON_6 0x00100000
#define LED_KEY_BUTTON_7 0x10000000

typedef struct {
	Tm1638 tm;
	uint8_t last_buttons;
	uint8_t hold_counter;

} Dlb8;

/// \brief get pressed button
/// \return
///   - if no button is pressed: 0
///   - if one or more buttons are pressed: bits 0..7 are set representing the pressed buttons 0..7 (or 1..8)
uint8_t dlb8_get_buttons(Dlb8 *obj);

/// \brief  get only pressed buttons which were not pressed at the last call of this function
uint8_t dlb8_get_changed_buttons(Dlb8 *obj);

/// \brief  return buttons which were hold pressed while NMB_CALLS of get_buttons()/get_changed_buttons()
uint8_t dlb8_calculate_hold_buttons(Dlb8 *obj, uint8_t nmb_calls);

/// \brief           put led on or off
/// \param mask      bit mask of LEDs to be addressed
/// \param value     on=true, off=false
/// \return          if success: true
bool dlb8_put_leds(Dlb8 *obj, uint8_t mask, bool value);

/// \brief          put value to digit
/// \param mask     bit mask of Digits to be addressed
/// \param value    bits 0..6 are representing digit segments a..g. bit 7 represents decimal point.
/// \return         if success: true
bool dlb8_put_digits(Dlb8 *obj, uint8_t mask, uint8_t value);

/// \brief          put char to digit
#define dlb8_put_chars(obj,mask,c, dp) dlb8_put_digits(obj,mask, Tm1638_char_to_7s((c)) | ((dp)?0x80:0))

#endif /* TM1638_INCLUDE_BOARD_D8L8K8_H_ */
