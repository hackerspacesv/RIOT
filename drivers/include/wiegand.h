/*
 * Copyright (C) 2019 Hackerspace San Salvador
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_wiegand Wiegand
 * @ingroup     drivers_sensors
 * @ingroup     drivers_saul
 *
 * @brief       GPIO based Wiegand driver
 *
 * This driver provides @ref drivers_saul capabilities.
 * @{
 *
 * @file
 * @brief       Driver for the Wiegand Protocol
 *
 * @author      Mario Gomez <mario.gomez@teubi.co>
 */

#ifndef WIEGAND_H
#define WIEGAND_H

#include <stdint.h>
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
  #endif
  
  /**
   * @brief   Parameters needed for device initialization
   */
  typedef struct {
    gpio_t       d0;      /**< Wiegand D0 data line */
    gpio_t       d1;      /**< Wiegand D1 data line */
    gpio_flank_t flank;   /**< flank detection */
  } wiegand_params_t;
  
  /**
   * @brief   Device descriptor for a wiegand device
   */
  typedef struct {
    unsigned long code;       /**< Wiegand code */
    int type;                 /**< Wiegand type */
  } wiegand_t;
  
  /**
   * @brief   Initialize a Wiegand device
   *
   * @param[out] dev          device descriptor
   * @param[in] params        configuration parameters
   *
   * @return                   0 on success
   * @return                  -1 on error
   */
  int wg_init(wiegand_t *dev, const wiegand_params_t *params);
  
  /**
   * @brief   Checks and stores new Wiegand data
   *
   * @param[out] dev          device descriptor of sensor
   *
   * @return                  true if code was read
   * @return                  false if no data
   */
  bool wg_available(wiegand_t *dev);
  
  /**
   * @brief   Reads the code read from the Wiegand device
   *
   * @param[in]  dev          device descriptor of sensor
   *
   * @return                  Code read from device
   */
  unsigned long wg_get_code(wiegand_t *dev);
  
  /**
   * @brief   Reads the Wiegand device type
   *
   * @param[out] dev         device descriptor of sensor
   */
  int wg_get_wiegand_type(wiegand_t *dev);
  
  #ifdef __cplusplus
}
#endif

#endif /* WIEGAND_H */
/** @} */
