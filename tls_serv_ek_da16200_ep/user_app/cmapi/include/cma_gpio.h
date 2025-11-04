/**
 ****************************************************************************************
 *
 * @file cma_gpio.h
 *
 * @brief User GPIO functions.
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

#ifndef CMA_GPIO_H_

#define CMA_GPIO_H_

#include "da16x_types.h"
#include "da16200_ioconfig.h"
#include "cma_status.h"

typedef enum
{
    CMA_GPIO_PORT_A = GPIO_UNIT_A,
    CMA_GPIO_PORT_B,
    CMA_GPIO_PORT_C,
} CMA_GPIO_PORT_TYPE;

typedef enum
{
    CMA_GPIO_PIN_0,
    CMA_GPIO_PIN_1,
    CMA_GPIO_PIN_2,
    CMA_GPIO_PIN_3,
    CMA_GPIO_PIN_4,
    CMA_GPIO_PIN_5,
    CMA_GPIO_PIN_6,
    CMA_GPIO_PIN_7,
    CMA_GPIO_PIN_8,
    CMA_GPIO_PIN_9,
    CMA_GPIO_PIN_10,
    CMA_GPIO_PIN_11,
} CMA_GPIO_PIN_TYPE;

typedef enum
{
    CMA_INT_EDGE_ACTIVE_HIGH = 0,
    CMA_INT_EDGE_ACTIVE_LOW,
    CMA_INT_LEVEL_ACTIVE_HIGH,
    CMA_INT_LEVEL_ACTIVE_LOW,
} CMA_INT_TYPE;

typedef enum
{
    CMA_RTC_WAKEUP1_PIN = 0,
    CMA_RTC_WAKEUP2_PIN,
} CMA_RTC_WAKEUP_PIN_NUM;

typedef enum
{
    CMA_GPIO_FUNC_ANA_A0_A1 = 0,
    CMA_GPIO_FUNC_ANA_A2_A3,
    CMA_GPIO_FUNC_SPIM_A6_A9,
    CMA_GPIO_FUNC_QSPI_A6_A11,
    CMA_GPIO_FUNC_SPIS_DIO_A0_A1,
    CMA_GPIO_FUNC_SPIS_CS_CLK_A2_A3,
    CMA_GPIO_FUNC_SPIS_CS_CLK_A6_A7,
    CMA_GPIO_FUNC_SPIS_DIO_A8_A9,
    CMA_GPIO_FUNC_SPIS_DIO_A10_A11,
    CMA_GPIO_FUNC_I2CM_A0_A1,
    CMA_GPIO_FUNC_I2CM_A4_A5,
    CMA_GPIO_FUNC_I2CM_A8_A9,
    CMA_GPIO_FUNC_I2CS_A0_A1,
    CMA_GPIO_FUNC_I2CS_A2_A3,
    CMA_GPIO_FUNC_I2CS_A4_A5,
    CMA_GPIO_FUNC_I2CS_A6_A7,
    CMA_GPIO_FUNC_SDIOS_A4_A9,
    CMA_GPIO_FUNC_SDIOM_A4_A9,
    CMA_GPIO_FUNC_I2S_BCLK_MCLK_A0_A1,
    CMA_GPIO_FUNC_I2S_SDO_LCLK_A2_A3,
    CMA_GPIO_FUNC_I2S_BCLK_MCLK_A4_A5,
    CMA_GPIO_FUNC_I2S_SDO_LCLK_A6_A7,
    CMA_GPIO_FUNC_I2S_BCLK_MCLK_A8_A9,
    CMA_GPIO_FUNC_I2S_CLK_IN_A3,
    CMA_GPIO_FUNC_I2S_CLK_IN_A10,
    CMA_GPIO_FUNC_UART1_TX_RX_A0_A1,
    CMA_GPIO_FUNC_UART1_TX_RX_A2_A3,
    CMA_GPIO_FUNC_UART1_TX_RX_A4_A5,
    CMA_GPIO_FUNC_UART1_TX_RX_A6_A7,
    CMA_GPIO_FUNC_UART1_CTS_RTS_A4_A5,
    CMA_GPIO_FUNC_UART2_TX_RX_A10_A11,
    CMA_GPIO_FUNC_UART2_TX_RX_C6_C7,
    CMA_GPIO_FUNC_GPIO_A0,
    CMA_GPIO_FUNC_GPIO_A1,
    CMA_GPIO_FUNC_GPIO_A2,
    CMA_GPIO_FUNC_GPIO_A3,
    CMA_GPIO_FUNC_GPIO_A4,
    CMA_GPIO_FUNC_GPIO_A5,
    CMA_GPIO_FUNC_GPIO_A6,
    CMA_GPIO_FUNC_GPIO_A7,
    CMA_GPIO_FUNC_GPIO_A8,
    CMA_GPIO_FUNC_GPIO_A9,
    CMA_GPIO_FUNC_GPIO_A10,
    CMA_GPIO_FUNC_GPIO_A11,
    CMA_GPIO_FUNC_GPIO_C6,
    CMA_GPIO_FUNC_GPIO_C7,
    CMA_GPIO_FUNC_GPIO_C8,
} CMA_GPIO_SET_FUNC_TYPE;

typedef enum
{
    CMA_GPIO_WAKEUP_PIN1 = 0,
    CMA_GPIO_WAKEUP_PIN2,
    CMA_GPIO_WAKEUP_GPIOC8,
    CMA_GPIO_WAKEUP_GPIOA6
} CMA_GPIO_WAKEUP_PIN;

typedef enum
{
    CMA_GPIO_WAKEUP_ACTIVE_HIGH = 0,
    CMA_GPIO_WAKEUP_ACTIVE_LOW
} CMA_GPIO_WAKEUP_TYPE;

typedef enum
{
    CMA_GPIO_LEVEL_LOW,
    CMA_GPIO_LEVEL_HIGH,
    CMA_GPIO_LEVEL_ERR
} CMA_GPIO_LEVEL_TYPE;

typedef enum
{
    CMA_GPIO_PULL_DOWN,
    CMA_GPIO_PULL_UP,
    CMA_GPIO_HIGH_Z
} CMA_GPIO_PULL_STATE;

typedef void (*cma_user_callback_t)(CMA_GPIO_WAKEUP_PIN pin);

/**
 ****************************************************************************************
 * @brief Get input level of GPIO.
 *
 * @param[in] Port number.
 * @param[in] GPIO pin number.
 *
 * @return the state of GPIO.
 *
 * cma_gpio_set_input is required at least one time. cma_gpio_set_input -> cma_gpio_get_input
 ****************************************************************************************
 */
CMA_GPIO_LEVEL_TYPE cma_gpio_get_input(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin);

/**
 ****************************************************************************************
 * @brief Create internal instance and set GPIO as input.
 *
 * @param[in] Port number.
 * @param[in] GPIO pin number.
 *
 * @return success or fail.
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_gpio_set_input(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin, CMA_GPIO_PULL_STATE state);

/**
 ****************************************************************************************
 * @brief Set level of GPIO as output.
 *
 * @param[in] Port number.
 * @param[in] GPIO pin number.
 * @param[in] level of GPIO.
 *
 * @return success or fail.
 *
 * cma_gpio_set_output is required at least one time. cma_gpio_set_output -> cma_gpio_set_output_level
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_gpio_set_output_level(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin, CMA_GPIO_LEVEL_TYPE level);

/**
 ****************************************************************************************
 * @brief Create internal instance and set GPIO as output.
 *
 * @param[in] Port number.
 * @param[in] GPIO pin number.
 *
 * @return success or fail.
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_gpio_set_output(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin);

/**
 ****************************************************************************************
 * @brief Set(enable) interrupt on GPIO.
 *
 * @param[in] Port number.
 * @param[in] GPIO pin number.
 * @param[in] Trigger type.
 * @param[in] callback function.
 *
 * @return success or fail.
 *
 * cma_gpio_set_input is required at least one time. cma_gpio_set_input -> cma_gpio_set_interrupt
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_gpio_set_interrupt(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin, CMA_INT_TYPE int_type,
        void *callback_func);

/**
 ****************************************************************************************
 * @brief Disable GPIO interrupt.
 *
 * @param[in] Port number.
 * @param[in] GPIO pin number.
 *
 * @return success or fail.
 *
 * cma_gpio_set_input is required at least one time. cma_gpio_set_input -> cma_gpio_set_interrupt -> cma_gpio_set_interrupt_disable
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_gpio_set_interrupt_disable(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin);

/**
 ****************************************************************************************
 * @brief Set PIN MUX on GPIO.
 *
 * @param[in] Function type.(GPIO, ADC, I2C, SPI, SDIO, I2C, I2S, and UART)
 *
 * @return success or fail.
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_gpio_set_func(CMA_GPIO_SET_FUNC_TYPE type);

/**
 ****************************************************************************************
 * @brief Set wake-up on RTC WAKEUP PIN.
 *
 * @param[in] RTC WAKEUP pin number.
 * @param[in] Trigger type.
 * @param[in] callback function.
 *
 * @return success or fail.
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_gpio_set_rtc_wakeup_pin(CMA_RTC_WAKEUP_PIN_NUM pin, CMA_INT_TYPE int_type, void *callback_func);

/**
 ****************************************************************************************
 * @brief Set GPIO interrupt pin for SPI slave.
 *
 * @param[in] Port number.
 * @param[in] GPIO pin number.
 *
 * @return success or fail.
 ****************************************************************************************
 */
CMA_STATUS_TYPE cma_gpio_set_spi_slave_func(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin);

/**
 ****************************************************************************************
 * @brief Set external GPIO wakeup source.
 *
 * @param[in] Port number.
 * @param[in] trigger type.
 * @param[in] user callback function.
 *
 * @return None
 ****************************************************************************************
 */
void cma_gpio_set_ext_wakeup_from_sleep(CMA_GPIO_WAKEUP_PIN pin, CMA_GPIO_WAKEUP_TYPE type,
        cma_user_callback_t user_callback_func);

/**
 ****************************************************************************************
 * @brief Clear GPIO wakeup source.
 *
 * @param[in] Port number.
 *
 * @return None.
 ****************************************************************************************
 */
void cma_gpio_clear_ext_wakeup_from_sleep(CMA_GPIO_WAKEUP_PIN pin);

#endif /* CMA_GPIO_H_ */
