/**
 ****************************************************************************************
 *
 * @file cma_sleep.h
 *
 * @brief User sleep functions.
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

#ifndef CMA_SLEEP_H_
#define CMA_SLEEP_H_

#include "da16x_types.h"
#include "da16200_ioconfig.h"

typedef enum {
    CMA_SLEEP_TYPE_2,
    CMA_SLEEP_TYPE_3
} CMA_SLEEP_TYPE;

/**
 ****************************************************************************************
 * @brief init sleep api.
 *
 * @param[in] None.
 *
 * @return None
 ****************************************************************************************
 */
void cma_sleep_init(void);

/**
 ****************************************************************************************
 * @brief Trigger sleep.
 *
 * @param[in] sleep type.
 * @param[in] wakeup time(millisecond).
 *
 * @return None
 ****************************************************************************************
 */
void cma_sleep_trigger(CMA_SLEEP_TYPE type, uint64_t wakeup_time);

#endif /* CMA_SLEEP_H_ */
