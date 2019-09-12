/*
 * Copyright (C) 2019 Hackerspace San Salvador
 * 
 * This driver is a port for RIOT-OS based on the Wiegand protocol for
 * Arduino implementation from monkeyboards.org.
 * 
 * Original implementation available at:
 *   https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     drivers_wiegand
 * @{
 *
 * @file
 * @brief       Driver for the Wiegand Protocol.
 *
 * @author      Mario Gomez <mario.gomez@teubi.co>
 *
 * @}
 */

#include "xtimer.h"
#include "wiegand_params.h"

/**
 * @brief Handler for D0 signal in Wiegand protocol
 * 
 */ 
void wg_read_d0(void *arg)
{
  wiegand_t *dev = (wiegand_t *)arg;
  // Increament bit count for Interrupt connected to D0
  dev->wg_bit_count++;
  // If bit count more than 31, process high bits
  if (dev->wg_bit_count>31)
  {
    // shift value to high bits
    dev->wg_card_temp_high |= ((0x80000000 & dev->wg_card_temp)>>31); 
    dev->wg_card_temp_high <<= 1;
    dev->wg_card_temp <<=1;
  }
  else
  {
    // D0 represent binary 0, so just left shift card data
    dev->wg_card_temp <<= 1; 
  }
  // Keep track of last wiegand bit received
  dev->wg_last_wiegand = xtimer_now_usec64();
}

/**
 * @brief Handler for D1 signal in Wiegand protocol
 * 
 */
void wg_read_d1(void *arg)
{
  wiegand_t *dev = (wiegand_t *)arg;
  
  // Increment bit count for Interrupt connected to D1
  dev->wg_bit_count ++;
  
  // If bit count more than 31, process high bits
  if (dev->wg_bit_count>31)
  { 
    // shift value to high bits
    dev->wg_card_temp_high |= ((0x80000000 & dev->wg_card_temp)>>31);
    dev->wg_card_temp_high <<= 1;
    dev->wg_card_temp |= 1;
    dev->wg_card_temp <<=1;
  }
  else
  {
    // D1 represent binary 1, so OR card data with 1 then
    dev->wg_card_temp |= 1;
    // left shift card data
    dev->wg_card_temp <<= 1;
  }
  // Keep track of last wiegand bit received
  dev->wg_last_wiegand = xtimer_now_usec64();
}

/**
 * @brief Returns the last code read from the Wiegand device.
 * 
 */
unsigned long wg_get_code(wiegand_t *dev)
{
  return dev->code;
}

/**
 * @brief Returns the Wiegand type.
 * 
 */
int16_t wg_get_wiegand_type(wiegand_t *dev)
{
  return dev->type;
}

/**
 * @brief Translates * and # keys to [ENTER] and [ESCAPE] keys.
 * 
 */
char translate_enter_escape_key_press(char originalKeyPress) {
  switch(originalKeyPress) {
    case 0x0b:     // 11 or * key
      return 0x0d; // 13 or ASCII ENTER
      
    case 0x0a:     // 10 or # key
      return 0x1b; // 27 or ASCII ESCAPE
      
    default:
      return originalKeyPress;
  }
}

/**
 * @brief Disables interrupt handling for D0 and D1 lines.
 * 
 */
static inline void wg_disable_int(wiegand_t *dev)
{
  gpio_irq_disable(dev->conn->d0);
  gpio_irq_disable(dev->conn->d1);
}

/**
 * @brief Enables interrupt handling for D0 and D1 lines.
 * 
 */
static inline void wg_enable_int(wiegand_t *dev)
{
  gpio_irq_enable(dev->conn->d0);
  gpio_irq_enable(dev->conn->d1);
}

/**
 * @brief Returns the card identification.
 * 
 */
unsigned long wg_get_card_id(volatile unsigned long *codehigh, volatile unsigned long *codelow, char bitlength)
{
  
  if (bitlength==26) // EM tag
    return (*codelow & 0x1FFFFFE) >>1;
  
  if (bitlength==34) // Mifare 
  {
    *codehigh = *codehigh & 0x03; // only need the 2 LSB of the codehigh
    *codehigh <<= 30; // shift 2 LSB to MSB		
    *codelow >>=1;
    return *codehigh | *codelow;
  }
  return *codelow; // EM tag or Mifare without parity bits
}

/**
 * @brief Process the temporary Wiegand buffer and decodes the data.
 * 
 */
bool wg_do_conversion(wiegand_t *dev)
{
  unsigned long cardID;
  uint64_t sysTick;
  char highNibble;
  char lowNibble;
  
  sysTick = xtimer_now_usec64();
  
  if ((sysTick - dev->wg_last_wiegand) > 25000) // if no more signal coming through after 25ms
  {
    if (
      (dev->wg_bit_count==24) || (dev->wg_bit_count==26) || (dev->wg_bit_count==32) ||
      (dev->wg_bit_count==34) || (dev->wg_bit_count==8) || (dev->wg_bit_count==4))// bitCount for keypress=4 or 8, Wiegand 26=24 or 26, Wiegand 34=32 or 34
    {
      dev->wg_card_temp >>= 1;			// shift right 1 bit to get back the real value - interrupt done 1 left shift in advance
      if (dev->wg_bit_count>32)			// bit count more than 32 bits, shift high bits right to make adjustment
        dev->wg_card_temp_high >>= 1;	
      
      if (dev->wg_bit_count==8)		// keypress wiegand with integrity
      {
        // 8-bit Wiegand keyboard data, high nibble is the "NOT" of low nibble
        // eg if key 1 pressed, data=E1 in binary 11100001 , high nibble=1110 , low nibble = 0001 
        highNibble = (dev->wg_card_temp & 0xf0) >>4;
        lowNibble = (dev->wg_card_temp & 0x0f);
        dev->type=dev->wg_bit_count;					
        dev->wg_bit_count=0;
        dev->wg_card_temp=0;
        dev->wg_card_temp_high=0;
        
        if (lowNibble == (~highNibble & 0x0f))		// check if low nibble matches the "NOT" of high nibble.
        {
          dev->code = (int)translate_enter_escape_key_press(lowNibble);
          return true;
        }
        else {
          dev->wg_last_wiegand=sysTick;
          dev->wg_bit_count=0;
          dev->wg_card_temp=0;
          dev->wg_card_temp_high=0;
          return false;
        }
        
        // TODO: Handle validation failure case!
      }
      else if (4 == dev->wg_bit_count) {
        // 4-bit Wiegand codes have no data integrity check so we just
        // read the LOW nibble.
        dev->code = (int)translate_enter_escape_key_press(dev->wg_card_temp & 0x0000000F);
        
        dev->type = dev->wg_bit_count;
        dev->wg_bit_count = 0;
        dev->wg_card_temp = 0;
        dev->wg_card_temp_high = 0;
        
        return true;
      }
      else // wiegand 26 or wiegand 34
      {
        cardID = wg_get_card_id (&(dev->wg_card_temp_high), &(dev->wg_card_temp), dev->wg_bit_count);
        dev->type=dev->wg_bit_count;
        dev->wg_bit_count=0;
        dev->wg_card_temp=0;
        dev->wg_card_temp_high=0;
        dev->code=cardID;
        return true;
      }
    }
    else
    {
      // well time over 25 ms and bitCount !=8 , !=26, !=34 , must be noise or nothing then.
      dev->wg_last_wiegand=sysTick;
      dev->wg_bit_count=0;			
      dev->wg_card_temp=0;
      dev->wg_card_temp_high=0;
      return false;
    }
  }
  else
  {
    return false;
  }
}

/**
 * @brief Tries to process the wiegand data and returns true if successfull.
 * 
 */
bool wg_available(wiegand_t *dev) {
  bool ret;
  wg_disable_int(dev);
  ret=wg_do_conversion(dev);
  wg_enable_int(dev);
  return ret;
}

/**
 * @brief Initializes the wiegand device.
 * 
 */
int16_t wg_init(wiegand_t *dev, const wiegand_params_t *params)
{
  dev->code = 0;
  dev->type = 0;
  dev->wg_card_temp_high = 0;
  dev->wg_card_temp = 0;
  dev->wg_last_wiegand = 0;
  dev->wg_bit_count = 0;
  dev->conn = params;
  
  if(gpio_init_int(
    dev->conn->d0,
    GPIO_IN,
    dev->conn->flank,
    wg_read_d0,
    (void *)dev
  ))
  {
    return -1;
  }
  
  if(gpio_init_int(
    dev->conn->d1,
    GPIO_IN,
    dev->conn->flank,
    wg_read_d1,
    (void *)dev
  ))
  {
    return -1;
  }
  
  return 0;
}
