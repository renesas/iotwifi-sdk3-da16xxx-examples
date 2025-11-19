/**
 ****************************************************************************************
 *
 * @file cma_debug.h
 *
 * @brief logging interface
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

#ifndef CMA_DEBUG_H_
#define CMA_DEBUG_H_

#include "da16x_system.h"
#include "task.h"

#define configLOG_SIMPLE	(1)

#define LOG_FATAL    (1)
#define LOG_ERR      (2)
#define LOG_WARN     (3)
#define LOG_INFO     (4)
#define LOG_DBG      (5)
#define LOG_OFF      (0)

#if configLOG_SIMPLE
#define LOG(level, ...) do {  \
                            if (level <= debug_level) { \
                                PRINTF(__VA_ARGS__); \
                                PRINTF("\n"); \
                            } \
                        } while (0)
#else
#define LOG(level, ...) do {  \
                            if (level <= debug_level) { \
                            	PRINTF("%dms:", xTaskGetTickCount() * portTICK_PERIOD_MS); \
                                PRINTF("%s:%d:", __FILE__, __LINE__); \
                                PRINTF(__VA_ARGS__); \
                                PRINTF("\n"); \
                            } \
                        } while (0)
#endif

void cma_assert_func(const uint8_t *pcFileName, uint32_t ulLineNumber);
#define cma_assert( x ) if( ( x ) == 0 ) { cma_assert_func((const uint8_t *)__func__, __LINE__ ); }

extern int32_t debug_level;

/**
 ****************************************************************************************
 * @brief Start a function for checking lowest heap with interval periodically.
 *
 * @param[in] None
 *
 * @return None.
 ****************************************************************************************
 */
void cma_heap_check_start(void);
/**
 ****************************************************************************************
 * @brief Stop a function for checking lowest heap.
 *
 * @param[in] None
 *
 * @return None.
 ****************************************************************************************
 */
void cma_heap_check_stop(void);

/**
 ****************************************************************************************
 * @brief Print out wakeup reason.
 *
 * @param[in] None
 *
 * @return None.
 ****************************************************************************************
 */
void cma_printout_wakeup_reason(void);

#endif /* CMA_DEBUG_H_ */
