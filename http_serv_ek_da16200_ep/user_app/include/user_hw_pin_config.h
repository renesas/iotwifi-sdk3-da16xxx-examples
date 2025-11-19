/**
 ****************************************************************************************
 *
 * @file user_hw_pin_config.h
 *
 * @brief define hw pin configuration
 *
 * Copyright (c) 2016-2024 Renesas Electronics. All rights reserved.
 *
 * This software ("Software") is owned by Renesas Electronics.
 *
 * By using this Software you agree that Renesas Electronics retains all
 * intellectual property and proprietary rights in and to this Software and any
 * use, reproduction, disclosure or distribution of the Software without express
 * written permission or a license agreement from Renesas Electronics is
 * strictly prohibited. This Software is solely for use on or in conjunction
 * with Renesas Electronics products.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE
 * SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. EXCEPT AS OTHERWISE
 * PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * RENESAS ELECTRONICS BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 *
 ****************************************************************************************
 */

#ifndef USER_HW_PIN_CONFIG_H_
#define USER_HW_PIN_CONFIG_H_

#include "cma_gpio.h"

#define USER_FACTORY_GPIO_PORT          CMA_GPIO_PORT_A
#define USER_APP_BTN_GPIO_PORT          CMA_GPIO_PORT_A
#define USER_FACTORY_GPIO_NUM           CMA_GPIO_PIN_7     /* GPIOA 7 */
#define USER_APP_BTN_GPIO_NUM           CMA_GPIO_PIN_6     /* GPIOA 6 */
#define USER_FACTORY_GPIO_FUNC_PIN      CMA_GPIO_FUNC_GPIO_A7
#define USER_APP_BTN_GPIO_FUNC_PIN      CMA_GPIO_FUNC_GPIO_A6

#endif /* USER_HW_PIN_CONFIG_H_ */
