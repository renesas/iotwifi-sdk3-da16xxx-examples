/**
 ****************************************************************************************
 *
 * @file user_rtm_data.c
 *
 * @brief User rtm data task
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
#include "da16x_sys_watchdog.h"
#include "sys_feature.h"
#include "cma_debug.h"
#include "cma_osal.h"
#include "cma_sleep.h"
#include "cma_rtm_data.h"
#include "user_rtm_data.h"

#include "util_api.h"
#include "common_config.h"

/* internal defines */
#define USER_RTM_DATA_TASK_NAME       	"USER_RTM_DATA"
#define USER_RTM_DATA_TASK_STACK_SZ   	(512 * 4)
#define USER_RTM_DATA_TASK_PRI        	OS_TASK_PRIORITY_USER

#define USER_RTM_DATA_SLEEP_EV         	(1 << 0)

#define USER_BRTM_DATA_IDLE_TIME       	2000  /* 2 sec */
#define USER_BRTM_DATA_SLEEP_TIME       2000  /* 2 sec */

/* USER Buffer */
#define USER_BUFFER_TAG         "user_buff"
#define USER_BUFFER_LENGTH      (128)

struct user_buffer_t
{
    uint16_t idx;
};

int32_t debug_level = 4;

/* Local variable */
static OS_TASK xTask = NULL;
static OS_TIMER user_rtm_data_timer = NULL;

static struct user_buffer_t *g_user_buff = NULL;

/* Local functions */

static void user_rtm_data_timer_callback(TimerHandle_t pxTime)
{
    DA16X_UNUSED_ARG(pxTime);

    if (xTask)
    {
        OS_TASK_NOTIFY(xTask, USER_RTM_DATA_SLEEP_EV, OS_NOTIFY_SET_BITS);
    }
}

static CMA_STATUS_TYPE user_rtm_data_app_init(void)
{
    cma_rtm_data_alloc_and_read ((uint8_t*) USER_BUFFER_TAG, (uint8_t**) &g_user_buff, sizeof(struct user_buffer_t));
    if (g_user_buff == NULL)
    {
        cma_assert(0);
    }

    LOG(LOG_INFO, " Index = %d\n", g_user_buff->idx);

    user_rtm_data_timer = OS_TIMER_CREATE("BLINKY_RTOS", portCONVERT_MS_2_TICKS(USER_BRTM_DATA_IDLE_TIME), pdTRUE,
                                          (void* )0, user_rtm_data_timer_callback);

    OS_TIMER_START(user_rtm_data_timer, OS_TIMER_FOREVER);

    return CMA_STATUS_OK;
}

/**
 * @brief Enable or disable the STA profile in DA16X00
 *
 * This function enables or disables the STA profile based on the input parameter.
 * If enabling, it reads the configuration for "sta0" and performs necessary actions.
 *
 * @param enable 1 to enable the STA profile, 0 to disable it.
 */
void sta_profile_enable(uint8_t enable)
{
    da16x_set_config_int(DA16X_CONF_INT_STA_PROF_DISABLED, enable ? 0 : 1);
}

/**
 * @brief Check if STA profile is disabled
 *
 * This function checks if the STA profile is disabled in DA16X00.
 *
 * @param disabled Pointer to an integer where the result will be stored (1 if disabled, 0 otherwise).
 * @return Returns 1 if the operation was successful, otherwise returns 0.
 */
uint8_t sta_profile_disabled(int *disabled)
{
    return (da16x_get_config_int(DA16X_CONF_INT_STA_PROF_DISABLED, disabled) == CC_SUCCESS);
}

/**
 * @brief Connect to a backup network.
 *
 * This function attempts to connect to a backup network by disabling the STA profile
 * and then configuring the network settings for the backup network.
 *
 * @return CMA_STATUS_TYPE indicating success or failure of the operation.
 */
static CMA_STATUS_TYPE cma_rtm_data_connect_backup_network(void)
{
    int ret = CMA_STATUS_OK;
    char *value_str = NULL;
    value_str = pvPortMalloc(128);
    if (value_str == NULL)  return CMA_STATUS_FAIL;

    // Disable the STA profile to prevent it from interfering with the backup network connection.
    LOG(LOG_INFO, "Disabling STA profile...");
    sta_profile_enable(0);

    // This function should implement the logic to connect to a backup network.
    // For now, we will just simulate a successful connection.
    LOG(LOG_INFO, "Connecting to backup network...");

    ret = da16x_cli_reply("remove_network 0", NULL, NULL);
    if (ret == CC_SUCCESS)
    {
        ret = da16x_cli_reply("add_network 0", NULL, NULL);
    }

    if (ret == CC_SUCCESS)
    {
        ret = da16x_cli_reply("set_network 0 ssid 'Alvin'", NULL, NULL);
    }

    if (ret == CC_SUCCESS)
    {
        ret = da16x_cli_reply("set_network 0 psk 'glasowk@2'", NULL, NULL);
    }

    if (ret == CC_SUCCESS)
    {
        ret = da16x_cli_reply("set_network 0 key_mgmt WPA-PSK", NULL, NULL);
    }

    if (ret == CC_SUCCESS)
    {
        ret = da16x_cli_reply("select_network 0", NULL, value_str);
    }

    LOG(LOG_INFO, "Network selection result: %s", value_str);
    if (ret < 0 || strcmp(value_str, "FAIL") == 0) {
        ret = CMA_STATUS_FAIL;
    }
    vPortFree(value_str);
    return ret;
}

/**
 * @brief Restore the primary network configuration.
 *
 * This function attempts to restore the primary network configuration by reading
 * the SSID, encryption key, and authentication type from NVRAM and applying them
 * to the network interface.
 *
 * @return CMA_STATUS_TYPE indicating success or failure of the operation.
 */
static CMA_STATUS_TYPE cma_rtm_data_restore_primary_network(void)
{
    int ret = CMA_STATUS_OK;
    char *value = NULL;
    char *value_str = NULL;
    value_str = pvPortMalloc(128);
    if (value_str == NULL)  return CMA_STATUS_FAIL;

    LOG(LOG_INFO, "Restoring primary network configuration...");

    ret = da16x_cli_reply("remove_network 0", NULL, NULL);
    if (ret == CC_SUCCESS)
    {
        ret = da16x_cli_reply("add_network 0", NULL, NULL);
    }

    if (ret == CC_SUCCESS)
    {
        value = read_nvram_string(NVR_KEY_SSID_0);
        if (value)
        {
            sprintf(value_str, "set_network 0 ssid %s", value);
            ret = da16x_cli_reply(value_str, NULL, NULL);
        }
    }

    if (ret == CC_SUCCESS)
    {
        value = read_nvram_string(NVR_KEY_ENCKEY_0);
        if (value)
        {
            sprintf(value_str, "set_network 0 psk %s", value);
            ret = da16x_cli_reply(value_str, NULL, NULL);
        }
    }

    if (ret == CC_SUCCESS)
    {
        value = read_nvram_string(NVR_KEY_AUTH_TYPE_0);
        if (value)
        {
            sprintf(value_str, "set_network 0 key_mgmt %s", value);
            ret = da16x_cli_reply(value_str, NULL, NULL);
        }
    }

    if (ret == CC_SUCCESS)
    {
        ret = da16x_cli_reply("select_network 0", NULL, value_str);
    }

    LOG(LOG_INFO, "Network selection result: %s", value_str);
    if (ret < CC_SUCCESS || strcmp(value_str, "FAIL") == 0) {
        ret = CMA_STATUS_FAIL;
    }

    // Re-enable the STA profile after restoring the primary network configuration.
    // This is important to ensure that the device can connect to the primary network.
    if (ret == CC_SUCCESS)
    {
        LOG(LOG_INFO, "Re-enabling STA profile...");
        sta_profile_enable(1);
    }

    vPortFree(value_str);
    return ret;
}

static void user_rtm_data_task(void *arg)
{
    DA16X_UNUSED_ARG(arg);

    int8_t wdog_id;

    LOG(LOG_INFO, "Start user rtm data task!");

    user_rtm_data_app_init ();

    /* Register pxp_reporter_task to be monitored by watchdog */
    wdog_id = da16x_sys_watchdog_register (pdFALSE);

    for (;;)
    {
        BaseType_t ret __attribute__((unused));
        uint32_t notif;

        /* Notify watchdog on each loop */
        da16x_sys_watchdog_notify (wdog_id);

        /* Suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
        da16x_sys_watchdog_suspend (wdog_id);

        /*
         * Wait on any of the notification bits, then clear them all
         */
        ret = OS_TASK_NOTIFY_WAIT(0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);
        /* Blocks forever waiting for the task notification. Therefore, the return value must
         * always be OS_OK
         */
        configASSERT(ret == pdTRUE);

        /* Resume watchdog */
        da16x_sys_watchdog_notify_and_resume (wdog_id);

        if (notif & USER_RTM_DATA_SLEEP_EV)
        {
            int disabled = 0;

            // Check if the user buffer is initialized
            if (g_user_buff == NULL)    return;

            if (sta_profile_disabled(&disabled))
            {
                // If the STA profile is disabled, we will try to restore the primary network
                // after 10 attempts, otherwise we will connect to the backup network.
                if (disabled)
                {
                    LOG(LOG_INFO, "STA profile is disabled, index = %d", g_user_buff->idx++);
                    if (g_user_buff->idx == 10)
                    {
                        LOG(LOG_INFO, "Attempting to restore primary network...");

                        // Reset the index to 0 after 10 attempts
                        g_user_buff->idx = 0;

                        // Try to restore the primary network
                        LOG(LOG_INFO, "Restoring primary network...");
                        if (cma_rtm_data_restore_primary_network() == CMA_STATUS_OK)
                        {
                            // Successfully restored the primary network
                            LOG(LOG_INFO, "Selected primary network successfully!");

                            // trigger a sleep to allow the network to stabilize
                            // the network will be restored after the sleep
                            cma_sleep_trigger(CMA_SLEEP_TYPE_3, USER_BRTM_DATA_SLEEP_TIME);
                        }
                        else
                        {
                            // Failed to restore the primary network
                            LOG(LOG_ERR, "Failed to select primary network!");
                        }
                    }
                }
                else
                {
                    // If the STA profile is enabled, we will try to connect to the backup network
                    // after 10 attempts.
                    LOG(LOG_INFO, "STA profile is enabled, index = %d", g_user_buff->idx++);
                    if (g_user_buff->idx == 10)
                    {
                        // Try to connect to the backup network
                        LOG(LOG_INFO, "Attempting to connect to backup network...");
                        if (cma_rtm_data_connect_backup_network() == CMA_STATUS_OK)
                        {
                            // Successfully connected to the backup network
                            LOG(LOG_INFO, "Selected backup network successfully!");
                        }
                        else
                        {
                            LOG(LOG_ERR, "Failed to select backup network!");
                        }

                        // It takes some time to connect to the backup network
                        // Note: If the device goes to sleep before the connection is established,
                        // it may not be able to connect to the backup network.
                    }
                }
            } else {
                // If we cannot check the STA profile status, log an error
                g_user_buff->idx = 0; // Reset index if we cannot check the status
                LOG(LOG_ERR, "Failed to check STA profile disabled status!");
            }
        }
    }
}

void user_rtm_data_init(void)
{
    cma_sleep_init ();
    cma_rtm_data_init ();

    if (xTask != NULL)
    {
        configASSERT(0);
    }

    if (pdPASS
            != OS_TASK_CREATE(USER_RTM_DATA_TASK_NAME, user_rtm_data_task, NULL, USER_RTM_DATA_TASK_STACK_SZ,
                              USER_RTM_DATA_TASK_PRI, xTask))
    {
        LOG(LOG_ERR, "Failed to start user rtm data task!");
        configASSERT(0);
    }
}

