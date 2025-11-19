/**
 ****************************************************************************************
 *
 * @file cma_gpio.c
 *
 * @brief User GPIO Functions.
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

#include "sdk_type.h"
#include "da16x_system.h"
#include "da16x_types.h"
#include "gpio.h"
#include "cma_debug.h"
#include "cma_gpio.h"

#define CMA_GPIO_SET_FUNC_DEBUG       (0)

#if CMA_GPIO_SET_FUNC_DEBUG
static uint32_t cma_gpio_status;
#endif

/* Usage : cma_gpio_set_func -> cma_gpio_set_input/cma_gpio_set_output/each peripheral function(I2C/SPI/UART) */
/* -> cma_gpio_get_input/cma_gpio_set_output_level */
CMA_GPIO_LEVEL_TYPE cma_gpio_get_input(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin)
{
    HANDLE handle;
    uint16_t read_data;
    uint8_t ret;
    uint32_t ioctldata;

    handle = GPIO_GET_INSTANCE (port);
    if (handle == NULL)
    {
        return CMA_GPIO_LEVEL_ERR;
    }

    ioctldata = (UINT32) (0x01 << pin);
    ret = GPIO_READ (handle, ioctldata, &read_data, sizeof(UINT16));

    if (ret == FALSE)
        return CMA_GPIO_LEVEL_ERR;
    else if ((read_data & ioctldata) != 0)
        return CMA_GPIO_LEVEL_HIGH;
    else
        return CMA_GPIO_LEVEL_LOW;

}

CMA_STATUS_TYPE cma_gpio_set_input(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin, CMA_GPIO_PULL_STATE state)
{
    HANDLE handle;
    uint32_t ioctldata;

    handle = GPIO_CREATE (port);
    if (handle == NULL)
    {
        return CMA_STATUS_FAIL;
    }

    GPIO_INIT (handle);

    ioctldata = (UINT32) (0x01 << pin);
    GPIO_IOCTL (handle, GPIO_SET_INPUT, &ioctldata);

    switch (state)
    {
        case CMA_GPIO_PULL_DOWN:
            _da16x_gpio_set_pull ((GPIO_UNIT_TYPE) port, (uint16_t) (0x01 << pin), PULL_DOWN);
        break;
        case CMA_GPIO_PULL_UP:
            _da16x_gpio_set_pull ((GPIO_UNIT_TYPE) port, (uint16_t) (0x01 << pin), PULL_UP);
        break;
        case CMA_GPIO_HIGH_Z:
            _da16x_gpio_set_pull ((GPIO_UNIT_TYPE) port, (uint16_t) (0x01 << pin), HIGH_Z);
        break;
    }

    return CMA_STATUS_OK;

}

CMA_STATUS_TYPE cma_gpio_set_output_level(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin, CMA_GPIO_LEVEL_TYPE level)
{
    HANDLE handle;
    uint16_t data;
    uint32_t ioctldata;

    handle = GPIO_GET_INSTANCE ((UINT32) port);
    if (handle == NULL)
    {
        return CMA_STATUS_FAIL;
    }

    ioctldata = (UINT32) (0x01 << pin);
    if (level == CMA_GPIO_LEVEL_LOW)
        data = (UINT16) (0x0 << pin);
    else
        data = (UINT16) (0x1 << pin);

    GPIO_WRITE (handle, (UINT32) ioctldata, &data, sizeof(UINT16));
    return CMA_STATUS_OK;
}

CMA_STATUS_TYPE cma_gpio_set_output(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin)
{
    HANDLE handle;
    uint32_t ioctldata;

    handle = GPIO_CREATE (port);
    if (handle == NULL)
    {
        return CMA_STATUS_FAIL;
    }

    GPIO_INIT (handle);

    ioctldata = (UINT32) (0x01 << pin);
    GPIO_IOCTL (handle, GPIO_SET_OUTPUT, &ioctldata);

    return CMA_STATUS_OK;
}

CMA_STATUS_TYPE cma_gpio_set_interrupt_disable(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin)
{
    HANDLE handle;
    uint32_t int_en_status;

    handle = GPIO_GET_INSTANCE ((UINT32) port);
    if (handle == NULL)
    {
        return CMA_STATUS_FAIL;
    }

    pin = 0x01 << pin;
    GPIO_IOCTL (handle, GPIO_GET_INTR_ENABLE, &int_en_status);
    int_en_status |= pin;
    GPIO_IOCTL (handle, GPIO_SET_INTR_DISABLE, &int_en_status);

    return CMA_STATUS_OK;
}

CMA_STATUS_TYPE cma_gpio_set_interrupt(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin, CMA_INT_TYPE int_type,
        void *callback_func)
{
    uint32_t ioctldata[3] = { 0, };
    HANDLE handle;
    uint32_t shift_pin, int_en_status;

    int type = 0, level = 0;

    handle = GPIO_GET_INSTANCE ((UINT32) port);
    if (handle == NULL)
    {
        return CMA_STATUS_FAIL;
    }

    shift_pin = 0x01 << pin;
    GPIO_IOCTL (handle, GPIO_SET_INPUT, &shift_pin);

    switch (int_type)
    {
        case CMA_INT_EDGE_ACTIVE_HIGH:
            type = 1;
            level = 1;
        break;
        case CMA_INT_EDGE_ACTIVE_LOW:
            type = 1;
            level = 0;
        break;
        case CMA_INT_LEVEL_ACTIVE_HIGH:
            type = 0;
            level = 1;
        break;
        case CMA_INT_LEVEL_ACTIVE_LOW:
            type = 0;
            level = 0;
        break;
    }

    GPIO_IOCTL (handle, GPIO_GET_INTR_MODE, &ioctldata[0]);
    /* interrupt type 1: edge, 0: level*/
    ioctldata[0] &= ~(shift_pin);
    ioctldata[0] |= (type << pin);
    /* interrupt pol 1: high active, 0: low active */
    ioctldata[1] &= ~(shift_pin);
    ioctldata[1] |= (level << pin);
    GPIO_IOCTL (handle, GPIO_SET_INTR_MODE, &ioctldata[0]);

    /* register callback function */
    ioctldata[0] = shift_pin; /* interrupt pin */
    ioctldata[1] = (UINT32) callback_func; /* callback function */
    ioctldata[2] = (UINT32) pin; /* param data */
    GPIO_IOCTL (handle, GPIO_SET_CALLACK, ioctldata);

    GPIO_IOCTL (handle, GPIO_GET_INTR_ENABLE, &int_en_status);
    int_en_status |= shift_pin;
    GPIO_IOCTL (handle, GPIO_SET_INTR_ENABLE, &int_en_status);

    return CMA_STATUS_OK;
}

CMA_STATUS_TYPE cma_gpio_set_rtc_wakeup_pin(CMA_RTC_WAKEUP_PIN_NUM pin_num, CMA_INT_TYPE int_type, void *callback_func)
{
    uint32_t intr_src;

    if (int_type > CMA_INT_EDGE_ACTIVE_LOW)
    {
        return CMA_STATUS_FAIL;
    }

    if (pin_num == CMA_RTC_WAKEUP1_PIN)
    {
        RTC_IOCTL (RTC_GET_RTC_CONTROL_REG, &intr_src);
        intr_src |= WAKEUP_PIN_ENABLE(1);
        RTC_IOCTL (RTC_GET_RTC_CONTROL_REG, &intr_src);

        RTC_IOCTL (RTC_GET_RTC_CONTROL_REG, &intr_src);
        if (int_type == CMA_INT_EDGE_ACTIVE_LOW)
            intr_src |= WAKEUP_POLARITY(1);
        else
            intr_src |= WAKEUP_POLARITY(0);
        RTC_IOCTL (RTC_GET_RTC_CONTROL_REG, &intr_src);
    }
    else if (pin_num == CMA_RTC_WAKEUP2_PIN)
    {
        RTC_IOCTL (RTC_GET_GPIO_WAKEUP_CONTROL_REG, &intr_src);
        intr_src |= RTC_WAKEUP2_EN(1);
        RTC_IOCTL (RTC_SET_GPIO_WAKEUP_CONTROL_REG, &intr_src);

        RTC_IOCTL (RTC_GET_GPIO_WAKEUP_CONFIG_REG, &intr_src);
        if (int_type == CMA_INT_EDGE_ACTIVE_LOW)
            intr_src |= RTC_WAKEUP2_SEL(1);
        else
            intr_src |= RTC_WAKEUP2_SEL(0);
        RTC_IOCTL (RTC_SET_GPIO_WAKEUP_CONFIG_REG, &intr_src);
    }

    RTC_IOCTL (RTC_GET_RTC_CONTROL_REG, &intr_src);
    intr_src |= WAKEUP_INTERRUPT_ENABLE(1);
    RTC_IOCTL (RTC_SET_RTC_CONTROL_REG, &intr_src);

    _sys_nvic_write (RTC_ExtWkInt_IRQn, (void*) callback_func, 0x7);

    return CMA_STATUS_OK;

}

static void cma_gpio_verify_and_reserve_pin(uint32_t pin)
{
#if CMA_GPIO_SET_FUNC_DEBUG
        if ((cma_gpio_status & (1 << pin))) {
                configASSERT(0);
        }

        cma_gpio_status |= (1 << pin);
#else
    (void) pin;
#endif
}

CMA_STATUS_TYPE cma_gpio_set_func(CMA_GPIO_SET_FUNC_TYPE type)
{
    switch (type)
    {
        case CMA_GPIO_FUNC_ANA_A0_A1:
            cma_gpio_verify_and_reserve_pin (PIN_AMUX);
            _da16x_io_pinmux (PIN_AMUX, AMUX_AD12);
        break;
        case CMA_GPIO_FUNC_ANA_A2_A3:
            cma_gpio_verify_and_reserve_pin (PIN_BMUX);
            _da16x_io_pinmux (PIN_BMUX, BMUX_AD12);
        break;
        case CMA_GPIO_FUNC_SPIM_A6_A9:
            cma_gpio_verify_and_reserve_pin (PIN_DMUX);
            _da16x_io_pinmux (PIN_DMUX, DMUX_SPIm);
            cma_gpio_verify_and_reserve_pin (PIN_EMUX);
            _da16x_io_pinmux (PIN_EMUX, EMUX_SPIm);
        break;
        case CMA_GPIO_FUNC_QSPI_A6_A11:
            cma_gpio_verify_and_reserve_pin (PIN_DMUX);
            _da16x_io_pinmux (PIN_DMUX, DMUX_SPIm);
            cma_gpio_verify_and_reserve_pin (PIN_EMUX);
            _da16x_io_pinmux (PIN_EMUX, EMUX_SPIm);
            cma_gpio_verify_and_reserve_pin (PIN_FMUX);
            _da16x_io_pinmux (PIN_FMUX, FMUX_SPIm);
        break;
        case CMA_GPIO_FUNC_SPIS_DIO_A0_A1:
            cma_gpio_verify_and_reserve_pin (PIN_AMUX);
            _da16x_io_pinmux (PIN_AMUX, AMUX_SPIs);
        break;
        case CMA_GPIO_FUNC_SPIS_CS_CLK_A2_A3:
            cma_gpio_verify_and_reserve_pin (PIN_BMUX);
            _da16x_io_pinmux (PIN_BMUX, BMUX_SPIs);
        break;
        case CMA_GPIO_FUNC_SPIS_CS_CLK_A6_A7:
            cma_gpio_verify_and_reserve_pin (PIN_DMUX);
            _da16x_io_pinmux (PIN_DMUX, DMUX_SPIs);
        break;
        case CMA_GPIO_FUNC_SPIS_DIO_A8_A9:
            cma_gpio_verify_and_reserve_pin (PIN_EMUX);
            _da16x_io_pinmux (PIN_EMUX, EMUX_SPIs);
        break;
        case CMA_GPIO_FUNC_SPIS_DIO_A10_A11:
            cma_gpio_verify_and_reserve_pin (PIN_FMUX);
            _da16x_io_pinmux (PIN_FMUX, FMUX_SPIs);
        break;
        case CMA_GPIO_FUNC_I2CM_A0_A1:
            cma_gpio_verify_and_reserve_pin (PIN_AMUX);
            _da16x_io_pinmux (PIN_AMUX, AMUX_I2Cm);
        break;
        case CMA_GPIO_FUNC_I2CM_A4_A5:
            cma_gpio_verify_and_reserve_pin (PIN_CMUX);
            _da16x_io_pinmux (PIN_CMUX, CMUX_I2Cm);
        break;
        case CMA_GPIO_FUNC_I2CM_A8_A9:
            cma_gpio_verify_and_reserve_pin (PIN_EMUX);
            _da16x_io_pinmux (PIN_EMUX, EMUX_I2Cm);
        break;
        case CMA_GPIO_FUNC_I2CS_A0_A1:
            cma_gpio_verify_and_reserve_pin (PIN_AMUX);
            _da16x_io_pinmux (PIN_AMUX, AMUX_I2Cs);
        break;
        case CMA_GPIO_FUNC_I2CS_A2_A3:
            cma_gpio_verify_and_reserve_pin (PIN_BMUX);
            da16x_io_pinmux(PIN_BMUX, BMUX_I2Cs);
        break;
        case CMA_GPIO_FUNC_I2CS_A4_A5:
            cma_gpio_verify_and_reserve_pin (PIN_CMUX);
            _da16x_io_pinmux (PIN_CMUX, CMUX_I2Cs);
        break;
        case CMA_GPIO_FUNC_I2CS_A6_A7:
            cma_gpio_verify_and_reserve_pin (PIN_DMUX);
            _da16x_io_pinmux (PIN_DMUX, DMUX_I2Cs);
        break;
        case CMA_GPIO_FUNC_SDIOS_A4_A9:
            cma_gpio_verify_and_reserve_pin (PIN_CMUX);
            _da16x_io_pinmux (PIN_CMUX, CMUX_SDs);
            cma_gpio_verify_and_reserve_pin (PIN_DMUX);
            _da16x_io_pinmux (PIN_DMUX, DMUX_SDs);
            cma_gpio_verify_and_reserve_pin (PIN_EMUX);
            _da16x_io_pinmux (PIN_EMUX, EMUX_SDs);
        break;
        case CMA_GPIO_FUNC_SDIOM_A4_A9:
            cma_gpio_verify_and_reserve_pin (PIN_CMUX);
            _da16x_io_pinmux (PIN_CMUX, CMUX_SDm);
            cma_gpio_verify_and_reserve_pin (PIN_EMUX);
            _da16x_io_pinmux (PIN_EMUX, EMUX_SDm);
            cma_gpio_verify_and_reserve_pin (PIN_DMUX);
            _da16x_io_pinmux (PIN_DMUX, DMUX_SDm);
        break;
        case CMA_GPIO_FUNC_UART1_TX_RX_A0_A1:
            cma_gpio_verify_and_reserve_pin (PIN_AMUX);
            _da16x_io_pinmux (PIN_AMUX, AMUX_UART1d);
        break;
        case CMA_GPIO_FUNC_UART1_TX_RX_A2_A3:
            cma_gpio_verify_and_reserve_pin (PIN_BMUX);
            _da16x_io_pinmux (PIN_BMUX, BMUX_UART1d);
        break;
        case CMA_GPIO_FUNC_UART1_TX_RX_A4_A5:
            cma_gpio_verify_and_reserve_pin (PIN_CMUX);
            _da16x_io_pinmux (PIN_CMUX, CMUX_UART1d);
        break;
        case CMA_GPIO_FUNC_UART1_TX_RX_A6_A7:
            cma_gpio_verify_and_reserve_pin (PIN_DMUX);
            _da16x_io_pinmux (PIN_DMUX, DMUX_UART1d);
        break;
        case CMA_GPIO_FUNC_UART1_CTS_RTS_A4_A5:
            cma_gpio_verify_and_reserve_pin (PIN_CMUX);
            _da16x_io_pinmux (PIN_CMUX, CMUX_UART1c);
        break;
        case CMA_GPIO_FUNC_UART2_TX_RX_A10_A11:
            cma_gpio_verify_and_reserve_pin (PIN_FMUX);
            _da16x_io_pinmux (PIN_FMUX, FMUX_UART2);
        break;
        case CMA_GPIO_FUNC_UART2_TX_RX_C6_C7:
            cma_gpio_verify_and_reserve_pin (PIN_UMUX);
            _da16x_io_pinmux (PIN_UMUX, UMUX_UART2GPIO);
        break;
        case CMA_GPIO_FUNC_GPIO_A0:
        case CMA_GPIO_FUNC_GPIO_A1:
            cma_gpio_verify_and_reserve_pin (PIN_AMUX);
            _da16x_io_pinmux (PIN_AMUX, AMUX_GPIO);
        break;
        case CMA_GPIO_FUNC_GPIO_A2:
        case CMA_GPIO_FUNC_GPIO_A3:
            cma_gpio_verify_and_reserve_pin (PIN_BMUX);
            _da16x_io_pinmux (PIN_BMUX, BMUX_GPIO);
        break;
        case CMA_GPIO_FUNC_GPIO_A4:
        case CMA_GPIO_FUNC_GPIO_A5:
            cma_gpio_verify_and_reserve_pin (PIN_CMUX);
            _da16x_io_pinmux (PIN_CMUX, CMUX_GPIO);
        break;
        case CMA_GPIO_FUNC_GPIO_A6:
        case CMA_GPIO_FUNC_GPIO_A7:
            cma_gpio_verify_and_reserve_pin (PIN_DMUX);
            _da16x_io_pinmux (PIN_DMUX, DMUX_GPIO);
        break;
        case CMA_GPIO_FUNC_GPIO_A8:
        case CMA_GPIO_FUNC_GPIO_A9:
            cma_gpio_verify_and_reserve_pin (PIN_EMUX);
            _da16x_io_pinmux (PIN_EMUX, EMUX_GPIO);
        break;
        case CMA_GPIO_FUNC_GPIO_A10:
        case CMA_GPIO_FUNC_GPIO_A11:
            cma_gpio_verify_and_reserve_pin (PIN_FMUX);
            _da16x_io_pinmux (PIN_FMUX, FMUX_GPIO);
        break;
        case CMA_GPIO_FUNC_GPIO_C6:
        case CMA_GPIO_FUNC_GPIO_C7:
        case CMA_GPIO_FUNC_GPIO_C8:
            cma_gpio_verify_and_reserve_pin (PIN_UMUX);
            _da16x_io_pinmux (PIN_UMUX, UMUX_GPIO);
        break;
        default:
            return CMA_STATUS_FAIL;
    }
    return CMA_STATUS_OK;
}

static cma_user_callback_t cma_gpio_ext_wakeup_user_cb;

static void cma_gpio_ext_wakup_cb(void *data)
{
    unsigned long long time_old;
    uint32_t ioctl = 0;
    CMA_GPIO_WAKEUP_PIN pin;

    DA16X_UNUSED_ARG(data);

    time_old = RTC_GET_COUNTER ();

    /* Waits until the RTC register is updated*/
    while (RTC_GET_COUNTER () < (time_old + 10))
    {
        ;
    }

    RTC_IOCTL (RTC_GET_GPIO_WAKEUP_CONTROL_REG, &ioctl);

    if (ioctl & RTC_WAKEUP_STATUS)
    {
        LOG(LOG_INFO, ">>> rtc1 wakeup interrupt ...\r\n");
        pin = CMA_GPIO_WAKEUP_PIN1;
    }

    if (ioctl & RTC_WAKEUP2_STATUS)
    {
        LOG(LOG_INFO, ">>> rtc2 wakeup interrupt ...\r\n");
        pin = CMA_GPIO_WAKEUP_PIN2;
    }

    /* clear wakeup source */
    time_old = RTC_GET_COUNTER ();
    RTC_CLEAR_EXT_SIGNAL ();

    while (RTC_GET_COUNTER () < (time_old + 1))
    {
        ;
    }

    time_old = RTC_GET_COUNTER ();
    ioctl = 0;
    RTC_IOCTL (RTC_SET_WAKEUP_SOURCE_REG, &ioctl);

    while (RTC_GET_COUNTER () < (time_old + 1))
    {
        ;
    }

    if (cma_gpio_ext_wakeup_user_cb != NULL)
    {
        cma_gpio_ext_wakeup_user_cb (pin);
    }
}
static void cma_gpio_ext_wakup_intr(void)
{
    INTR_CNTXT_CALL(cma_gpio_ext_wakup_cb);
}

void cma_gpio_set_ext_wakeup_from_sleep(CMA_GPIO_WAKEUP_PIN pin, CMA_GPIO_WAKEUP_TYPE type,
        cma_user_callback_t user_callback_func)
{
    uint32_t intr_src, ioctldata;

    switch (pin)
    {
        case CMA_GPIO_WAKEUP_PIN1:
            RTC_IOCTL (RTC_GET_RTC_CONTROL_REG, &intr_src);
            intr_src |= WAKEUP_INTERRUPT_ENABLE(1) | WAKEUP_POLARITY(1);
            RTC_IOCTL (RTC_SET_RTC_CONTROL_REG, &intr_src);
        break;
        case CMA_GPIO_WAKEUP_PIN2:
            RTC_IOCTL (RTC_GET_GPIO_WAKEUP_CONTROL_REG, &intr_src);
            intr_src |= RTC_WAKEUP2_EN(1);
            RTC_IOCTL (RTC_SET_GPIO_WAKEUP_CONTROL_REG, &intr_src);

            RTC_IOCTL (RTC_GET_GPIO_WAKEUP_CONFIG_REG, &intr_src);
            if (type == CMA_GPIO_WAKEUP_ACTIVE_LOW)
                intr_src |= RTC_WAKEUP2_SEL(1); // active low
            else
                intr_src &= ~RTC_WAKEUP2_SEL(1); // active high
            RTC_IOCTL (RTC_SET_GPIO_WAKEUP_CONFIG_REG, &intr_src);
        break;
        case CMA_GPIO_WAKEUP_GPIOC8:
            RTC_IOCTL (RTC_GET_GPIO_WAKEUP_CONFIG_REG, &ioctldata);
            ioctldata &= ~(0x03ff0000);
            ioctldata |= GPIOA11_OR_GPIOC8(1);           // GPIOC8
            if (type == CMA_GPIO_WAKEUP_ACTIVE_LOW)
                ioctldata |= GPIOA11_OR_GPIOC8_EDGE_SEL(1);  // 0; active high 1; active low
            else
                ioctldata |= GPIOA11_OR_GPIOC8_EDGE_SEL(1);  // 0; active high 1; active low
            RTC_IOCTL (RTC_SET_GPIO_WAKEUP_CONFIG_REG, &ioctldata);

            RTC_IOCTL (RTC_GET_GPIO_WAKEUP_CONTROL_REG, &ioctldata);
            ioctldata |= GPIOA11_OR_GPIOC8_INT_EN(1);    // GPIOC8 enable
            RTC_IOCTL (RTC_SET_GPIO_WAKEUP_CONTROL_REG, &ioctldata);
        break;
        case CMA_GPIO_WAKEUP_GPIOA6:
            RTC_IOCTL (RTC_GET_GPIO_WAKEUP_CONFIG_REG, &ioctldata);
            ioctldata &= ~GPIOA6_OR_GPIOC2(1);   // clear means 0 = GPIOA6
            if (type == CMA_GPIO_WAKEUP_ACTIVE_LOW)
                ioctldata |= GPIOA6_OR_GPIOC2_EDGE_SEL(1);  // 0; active high 1; active low
            else
                ioctldata |= GPIOA6_OR_GPIOC2_EDGE_SEL(1);  // 0; active high 1; active low
            RTC_IOCTL (RTC_SET_GPIO_WAKEUP_CONFIG_REG, &ioctldata);

            RTC_IOCTL (RTC_GET_GPIO_WAKEUP_CONTROL_REG, &ioctldata);
            ioctldata |= GPIOA6_OR_GPIOC2_INT_EN(1);    // INT Enable
            RTC_IOCTL (RTC_SET_GPIO_WAKEUP_CONTROL_REG, &ioctldata);
        break;
    }

    if (pin != CMA_GPIO_WAKEUP_PIN2) //PIN2 does not support callback
    {
        cma_gpio_ext_wakeup_user_cb = user_callback_func;
    }
    _sys_nvic_write (RTC_ExtWkInt_IRQn, (void*) cma_gpio_ext_wakup_intr, 0x7);
}

void cma_gpio_clear_ext_wakeup_from_sleep(CMA_GPIO_WAKEUP_PIN pin)
{
    uint32_t intr_src, ioctldata;

    switch (pin)
    {
        case CMA_GPIO_WAKEUP_PIN1:
            RTC_IOCTL (RTC_GET_RTC_CONTROL_REG, &intr_src);
            intr_src |= WAKEUP_INTERRUPT_ENABLE(0) | WAKEUP_POLARITY(1);
            RTC_IOCTL (RTC_SET_RTC_CONTROL_REG, &intr_src);
        break;
        case CMA_GPIO_WAKEUP_PIN2:
            RTC_IOCTL (RTC_GET_GPIO_WAKEUP_CONTROL_REG, &intr_src);
            intr_src |= RTC_WAKEUP2_EN(0);
            RTC_IOCTL (RTC_SET_GPIO_WAKEUP_CONTROL_REG, &intr_src);
        break;
        case CMA_GPIO_WAKEUP_GPIOC8:
            RTC_IOCTL (RTC_GET_GPIO_WAKEUP_CONTROL_REG, &ioctldata);
            ioctldata |= GPIOA11_OR_GPIOC8_INT_EN(0);
            RTC_IOCTL (RTC_SET_GPIO_WAKEUP_CONTROL_REG, &ioctldata);
        break;
        case CMA_GPIO_WAKEUP_GPIOA6:
            RTC_IOCTL (RTC_GET_GPIO_WAKEUP_CONTROL_REG, &ioctldata);
            ioctldata |= GPIOA6_OR_GPIOC2_INT_EN(0);
            RTC_IOCTL (RTC_SET_GPIO_WAKEUP_CONTROL_REG, &ioctldata);
        break;
    }
}

CMA_STATUS_TYPE cma_gpio_set_spi_slave_func(CMA_GPIO_PORT_TYPE port, CMA_GPIO_PIN_TYPE pin)
{
    static HANDLE gpio;

    gpio = GPIO_CREATE (port);
    GPIO_INIT (gpio);

    GPIO_SET_ALT_FUNC (gpio, GPIO_ALT_FUNC_EXT_INTR, (GPIO_ALT_GPIO_NUM_TYPE) (pin));
    GPIO_CLOSE (gpio);

    return CMA_STATUS_OK;
}
