/**
 ****************************************************************************************
 *
 * @file cma_debug.c
 *
 * @brief APIs for debug
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
#include "da16x_system.h"
#include "da16x_sys_watchdog.h"
#include "cma_debug.h"
#include "cma_osal.h"
#include "cma_gpio.h"
//#include "user_hw_pin_config.h"

#define CMA_HEAP_INIT_SIZE          (200 * 1024) //200K
#define CMA_HEAP_LOWEST_SIZE        (30 * 1024) //30K
#define CMA_HEAP_DOWN_LIMIT         100
#define CMA_HEAP_CHECK_PERIOD_MS    1000 // 1 sec
#define CMA_HEAP_REPORT_PERIOD_SEC  ((5 * CMA_HEAP_CHECK_PERIOD_MS) / 1000) // 10 times of CMA_HEAP_CHECK_PERIOD_MS

static OS_TIMER cma_heap_check_timer = NULL;
static uint32_t cma_pre_lowest_heap;
static uint32_t cma_heap_down_number;
static uint32_t cma_heap_check_number;

static void cma_heap_check_timer_callback(TimerHandle_t pxTime)
{
    size_t size;

    DA16X_UNUSED_ARG(pxTime);

    size = xPortGetMinimumEverFreeHeapSize ();
    if (cma_pre_lowest_heap > size)
    {
        cma_heap_down_number++;
        cma_pre_lowest_heap = size;
    }

    if ((cma_heap_check_number % CMA_HEAP_REPORT_PERIOD_SEC) == 0)
    {
        //cma_gpio_set_output_level(USER_LED1_GPIO_PORT, USER_LED1_GPIO_NUM, CMA_GPIO_LEVEL_LOW);
        //OS_DELAY(100); //1 sec
        //cma_gpio_set_output_level(USER_LED1_GPIO_PORT, USER_LED1_GPIO_NUM, CMA_GPIO_LEVEL_HIGH);

        LOG(LOG_DBG, "Lowest free heap size = %d byte. ", cma_pre_lowest_heap);
    }

    if (cma_heap_down_number > CMA_HEAP_DOWN_LIMIT)
    {
        LOG(LOG_WARN, "Heap leakage is suspected! Lowest free heap size = %d byte. ", cma_pre_lowest_heap);
    }

    if (cma_pre_lowest_heap < CMA_HEAP_LOWEST_SIZE)
    {
        LOG(LOG_WARN, "Heap shortage is suspected! Lowest free heap size = %d byte. ", cma_pre_lowest_heap);
    }

    cma_heap_check_number++;
}

void cma_heap_check_start(void)
{
    if (cma_heap_check_timer != NULL)
        return;

    cma_pre_lowest_heap = CMA_HEAP_INIT_SIZE;
    cma_heap_check_timer = OS_TIMER_CREATE("CMA_HEAP", portCONVERT_MS_2_TICKS(CMA_HEAP_CHECK_PERIOD_MS), pdTRUE,
                                           (void* )0, cma_heap_check_timer_callback);
    if (cma_heap_check_timer != NULL)
        OS_TIMER_START(cma_heap_check_timer, OS_TIMER_FOREVER);

    //cma_gpio_set_output_level(USER_LED1_GPIO_PORT, USER_LED1_GPIO_NUM, CMA_GPIO_LEVEL_HIGH);
}

void cma_heap_check_stop(void)
{
    if (cma_heap_check_timer == NULL)
        return;

    OS_TIMER_STOP(cma_heap_check_timer, OS_TIMER_FOREVER);
    OS_TIMER_DELETE(cma_heap_check_timer, OS_TIMER_FOREVER);
    cma_heap_check_timer = NULL;
}

void cma_assert_func(const uint8_t *pcFileName, uint32_t ulLineNumber)
{
    Printf ("\r\n>> ASSERT %s %d !!!\r\n", pcFileName, ulLineNumber);

    da16x_sys_watchdog_disable ();

    taskENTER_CRITICAL();
    for (;;)
        ;
}

void cma_debug_gpio_high_and_low(CMA_GPIO_SET_FUNC_TYPE type, CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin)
{
    cma_gpio_set_func (type);

    cma_gpio_set_output (port, pin);
    cma_gpio_set_output_level (port, pin, 1);

    __NOP ();
    __NOP ();
    __NOP ();
    __NOP ();
    __NOP ();

    cma_gpio_set_output_level (port, pin, 0);
}

void cma_debug_gpio_led_on(CMA_GPIO_SET_FUNC_TYPE type, CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin,
        CMA_GPIO_LEVEL_TYPE level)
{
    cma_gpio_set_func (type);
    cma_gpio_set_output (port, pin);
    if (level == CMA_GPIO_LEVEL_HIGH)
        cma_gpio_set_output_level (port, pin, 1);
    else
        cma_gpio_set_output_level (port, pin, 0);
}

void cma_printout_wakeup_reason(void)
{
    uint32_t reason;

    reason = da16x_boot_get_wakeupmode ();

    LOG(LOG_DBG, "Wakeup reason is 0x%x.", reason);
    switch (reason)
    {
        case WAKEUP_SOURCE_POR: //0x04 //Confirmed
            LOG(LOG_DBG, "Wakeup reason is RTC_PWR_KEY from Power off.");
        break;
        case WAKEUP_COUNTER_WITH_RETENTION: //0x82 //Confirmed
            LOG(LOG_DBG, "Wakeup reason is RTC timer from sleep3.");
        break;

        case WAKEUP_EXT_SIG_WAKEUP_COUNTER_WITH_RETENTION: //0x83 RTC_WAKEUP_KEY from slee3 confirmed.
            LOG(LOG_DBG, "Wakeup reason is EXT KEY from sleep3.");
        break;

        case WAKEUP_RESET: //0x00
            LOG(LOG_DBG, "Wakeup reason is RESET.");
        break;
        case WAKEUP_SENSOR_WAKEUP_COUNTER_WITH_RETENTION: //0x92
            LOG(LOG_DBG, "Wakeup reason is RTC timer from sleep3.");
        break;
        case WAKEUP_SOURCE_EXT_SIGNAL: //0x01
            LOG(LOG_DBG, "Wakeup reason is External siganl.");
        break;
        case WAKEUP_SOURCE_WAKEUP_COUNTER: //0x02
            LOG(LOG_DBG, "Wakeup reason is RTC or external signal from Sleep2.");
        break;

        case WAKEUP_SOURCE_POR_EXT_SIGNAL: //0x05
            LOG(LOG_DBG, "Wakeup reason is RTC_PWR_KEY from Power off.");
        break;
        case WAKEUP_WATCHDOG: //0x08
            LOG(LOG_DBG, "Wakeup reason is Watchdog.");
        break;
        case WAKEUP_WATCHDOG_EXT_SIGNAL: //0x09
        case WAKEUP_SENSOR_WATCHDOG: //0x24
        case WAKEUP_SENSOR_EXT_WATCHDOG: //0x25
        case WAKEUP_WATCHDOG_WITH_RETENTION: //0x88
        case WAKEUP_WATCHDOG_EXT_SIGNAL_WITH_RETENTION: //0x90
        case WAKEUP_SENSOR_WATCHDOG_WITH_RETENTION: //0x98
        case WAKEUP_SENSOR_EXT_WATCHDOG_WITH_RETENTION: //0x99
        case WAKEUP_RESET_WITH_RETENTION: //0x80
        case WAKEUP_SOURCE_UNKNOWN: //0xff
        case WAKEUP_SENSOR: //0x10
        case WAKEUP_SENSOR_EXT_SIGNAL: //0x11
        case WAKEUP_SENSOR_WAKEUP_COUNTER: //0x12
        case WAKEUP_SENSOR_EXT_WAKEUP_COUNTER: //0x13
        break;
        default:
        break;
    }
}
