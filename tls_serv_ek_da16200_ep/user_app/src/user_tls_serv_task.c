/**
 ****************************************************************************************
 *
 * @file user_tls_serv_task.c
 *
 * @brief Sample app of TLS server.
 *
 * Copyright (c) 2016-2025 Renesas Electronics. All rights reserved.
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
#include "task.h"

#include "da16x_crypto.h"
#include "mbedtls/config.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/error.h"
#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#endif
#include "cma_osal.h"
#include "cma_debug.h"

/*
 * Definitions for the TLS server task
 */
#define USER_TLS_SERV_TASK_NAME             "USER_TLS_SERVER"
#define USER_TLS_SERV_TASK_STACK_SZ         (256 * 3 * sizeof(StackType_t))
#define USER_TLS_SERV_TASK_PRI              (OS_TASK_PRIORITY_USER+1)

#define USER_TLS_SERV_BUF_SZ                (1024 * 4)
#define USER_TLS_SERV_DEF_PORT              10197

#define USER_TLS_MAX_NUM_TASKS                   2
#if ((USER_TLS_MAX_NUM_TASKS) > 5)
#error "USER_TLS_MAX_NUM_TASKS should not bigger than 5"
#endif

#ifdef MBEDTLS_ERROR_C
#define USER_TLS_SERV_STR_ERR_SZ                     100
#endif

/*
 * Definitions for the task handling TLS client connections
 */

#define USER_TLS_CONN_TASK_PRI                   (tskIDLE_PRIORITY + 7)
#define USER_TLS_CONN_TASK_SIZE                  (1024 * sizeof(StackType_t))
#define USER_TLS_CONN_NAME                       (8)

static OS_TASK user_tls_conn_task_handle[USER_TLS_MAX_NUM_TASKS] = {NULL, };
static uint8_t user_tls_conn_active[USER_TLS_MAX_NUM_TASKS] = {0, };
static char user_tls_conn_name[USER_TLS_CONN_NAME]; /*TLS_CN_1, TLS_CN_2 .. */

/*
 * Global variables for the TLS server task
 */
static OS_TASK user_tls_serv_handle = NULL;
static mbedtls_net_context user_listen_ctx, user_client_ctx;
static mbedtls_ssl_config user_ssl_conf;

static const unsigned char user_tls_serv_cert[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDBjCCAe4CCQCg5xtL5ap/CDANBgkqhkiG9w0BAQsFADBFMQswCQYDVQQGEwJB\n"
    "VTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0\n"
    "cyBQdHkgTHRkMB4XDTE4MDgyMTA3Mzg0M1oXDTI4MDgxODA3Mzg0M1owRTELMAkG\n"
    "A1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0\n"
    "IFdpZGdpdHMgUHR5IEx0ZDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n"
    "ALMgpDjh0c0ZThAxE/B2mBcDAp2KUaeoXqY5+03bZWRQ7McE0og3DV9u14FGwhDQ\n"
    "J93itsHXnWGZ6Zh1xtPJzoMiaxxaAe+1WdognwlTD8LUr7qS/7VtFXXmvVaIQ7E5\n"
    "AKLxlFsgVc3soSLo4f06DWOGl7pSIC2EttYFKMvBmlWcksEA1+DXP/NJy8h3wqTF\n"
    "MC+iydE5cXd3lKJ3CQ+0zTJli4DOF5ZudbrkWOQXwfZ1ylB24umvbNMSfltlSkuC\n"
    "km0VhF8VCLFwek+WNcrqeDkAAyMyRyVlDi+9r1qTHPF+EYwcJHWh+zFegztGc1w6\n"
    "7mxBBh+JRBH7GzWLT9kBg7kCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAWJVjI3po\n"
    "zjCpsfrPw1u/K/0AJGRpLChvBPMYz11/Ay2I2vyT8akRM+7km0W/d+wiyL0YT9QI\n"
    "PHT4gLEV4KeiQSAPindtJgbg2Wi53EBGeNrDWHMwH3islDr9h293zhUqZMie2WDb\n"
    "kpEcSh79DAhZDZ43d/qHgWeyWWkyC8JLQIzYPsdTJYf8qq7yNZlr7OZCHXdq2sG8\n"
    "SjE4I1j4ANkkJpnX6yFDZZGDDrUzPcJRPVtZzmytJUIbzoTU9E6nSDIlvBTT273o\n"
    "9gJXvkU7L4cwNCAg0g3RRwyqIUq8zyWsIeNSdqz64EIH2wX4XvWkVzDzfxW76HL7\n"
    "NOds4a5SIHfFZw==\n"
    "-----END CERTIFICATE-----\n";
static const size_t user_tls_serv_cert_len = sizeof(user_tls_serv_cert);

static const unsigned char user_tls_serv_key[] =
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIIEpAIBAAKCAQEAsyCkOOHRzRlOEDET8HaYFwMCnYpRp6hepjn7TdtlZFDsxwTS\n"
    "iDcNX27XgUbCENAn3eK2wdedYZnpmHXG08nOgyJrHFoB77VZ2iCfCVMPwtSvupL/\n"
    "tW0Vdea9VohDsTkAovGUWyBVzeyhIujh/ToNY4aXulIgLYS21gUoy8GaVZySwQDX\n"
    "4Nc/80nLyHfCpMUwL6LJ0Tlxd3eUoncJD7TNMmWLgM4Xlm51uuRY5BfB9nXKUHbi\n"
    "6a9s0xJ+W2VKS4KSbRWEXxUIsXB6T5Y1yup4OQADIzJHJWUOL72vWpMc8X4RjBwk\n"
    "daH7MV6DO0ZzXDrubEEGH4lEEfsbNYtP2QGDuQIDAQABAoIBADniYG8pOhznAnzk\n"
    "/yaDjF5TULMMEZr2I6/fqL/eGAO0yu79NfNipuWh8e4KqYe5XEitjJVTUb5KeFwW\n"
    "IywpWJyzsJ020M1fcyuzwvDGcJ9rD2ZhPlSobXjuGV0vJ4DLhNMi8egIqPGkd+XK\n"
    "D80+xzjUM4+4HkHXUyYSAL7nTzI+nh9mm3yCntFSJsNj1tRX7IVjnjh1RIS7MR5B\n"
    "f0u9xVGf3UbBveK/RyVGd/QWJacktBcr3WyCxHkwo64TETuj5D3NxqIrmiChBauB\n"
    "ediqoe3RuxPw6vgE3+z3mfDdD0Xb58CsvN27dW//euf1xZjjw6xLqAo3Zd8fAMME\n"
    "EFDURiECgYEA1mX/Z0OoNId3SoFhg5NkQlEBSb2Lna35JtqhuylPLarjapoGj3C/\n"
    "4z0hAIU6d1W3L12P175XaHrDuA51Tc9bnMp/JuSrV6r5CvmZJ81MftYlJzl41zJd\n"
    "JIf1nILxlXvQS9rVHSqIO/0dPwEAkGKJHqqhyWTM5sh3CcgpQy7JsL0CgYEA1eKZ\n"
    "pDdgkLRQgmtkW0FDIkR1aG6J6gE/TpmmSY64xqEYr5rnGRObvYrGQXHp9oD/QuCH\n"
    "fiVeudvuyQYXt0KsEv0NyhoLWmAlTij1eKEjUmvHrTV3ydMSdEIs5p5k6w3Ht8KU\n"
    "4YAjjFglL+0xDV9EC6nwB4swNwKfBpifuwkspK0CgYB1RFjMHJ92C9pdsCKsGwQt\n"
    "ma0ArmIdHrk2XUM04cVjDyNQfWq1LlBmdFsGs9hkyUdm6t/wezXH+c3vcEkNBCvx\n"
    "uHiPx2dIjkWlkRwKPypl/a9YowDLg8qaXpsiviRxRMWLl+gVCdx2I13JxjyOvLaP\n"
    "RXk0dKP2XxNtEEQxcPf0aQKBgQDHXzzcqIopGQvbJoQb1E/iB3Jx8Gg6awM6H1u0\n"
    "QYfYD57VQk2dQHvySQPZSXhPwZswGd/zJJ6SHYMOe9FrkIiaAqzx8SkYC3t6yg9X\n"
    "bM1iLPmqaabJySjwmicEqi1kNiovDwB821dHoXq4nB8XWfAx9yy5u3MsNBNMsMRk\n"
    "Mn8c2QKBgQC0OKboeUgzk9i06KE6CqGO43WpPDKNG+4ylnCnIxjeGIYLNKHJ+RCv\n"
    "4lt+FHWcwciM3JoNJWSfqXGcqp2B97I/YDW87L6He4ShUmzvdlAG1CecGqTYBrYB\n"
    "lcr/+RukWzIMUCA24KeAoheKsC+HJGg4U08nWz5n9K30kqNya+nz9g==\n"
    "-----END RSA PRIVATE KEY-----\n";
static const size_t user_tls_serv_key_len = sizeof(user_tls_serv_key);

static void user_tls_serv_hex_dump(const char *title, unsigned char *buf, size_t len);
static int user_tls_serv_rsa_decrypt_func(void *ctx, int mode, size_t *olen,
                                       const unsigned char *input,
                                       unsigned char *output,
                                       size_t output_max_len);
static int user_tls_serv_rsa_sign_func(void *ctx,
                                    int (*f_rng)(void*, unsigned char*, size_t),
                                    void *p_rng, int mode,
                                    mbedtls_md_type_t md_alg, 
                                    unsigned int hashlen, 
                                    const unsigned char *hash, 
                                    unsigned char *sig);
static size_t user_tls_serv_rsa_key_len_func(void *ctx);
static void user_tls_serv_task(void *param);
static void user_tls_conn_task(void *param);

static void user_tls_serv_hex_dump(const char *title, unsigned char *buf, size_t len)
{
#if defined (USER_TLS_SERV_ENABLED_HEXDUMP)
    extern void hex_dump(unsigned char *data, unsigned int length);

    if (len) {
        LOG(LOG_INFO,"%s(%ld)", title, len);
        hex_dump(buf, len);
    }
#else
    DA16X_UNUSED_ARG(title);
    DA16X_UNUSED_ARG(buf);
    DA16X_UNUSED_ARG(len);
#endif

    return;
}

static int user_tls_serv_rsa_decrypt_func(void *ctx, int mode, size_t *olen,
                                       const unsigned char *input,
                                       unsigned char *output,
                                       size_t output_max_len)
{
    return (mbedtls_rsa_pkcs1_decrypt((mbedtls_rsa_context*)ctx, 
            NULL, NULL, mode, olen, input, output, output_max_len));
}

static int user_tls_serv_rsa_sign_func(void *ctx,
                                    int (*f_rng)(void*, unsigned char*, size_t),
                                    void *p_rng, int mode, 
                                    mbedtls_md_type_t md_alg,
                                    unsigned int hashlen,
                                    const unsigned char *hash,
                                    unsigned char *sig)
{
    return (mbedtls_rsa_pkcs1_sign((mbedtls_rsa_context*)ctx, f_rng,
                                    p_rng, mode, md_alg, hashlen, hash, sig));
}

static size_t user_tls_serv_rsa_key_len_func(void *ctx)
{
    return (((const mbedtls_rsa_context*)ctx)->len);
}
#ifdef USER_TLS_SERV_MULTI_CLIENTS_CONN_DEBUG
void user_tls_serv_debug(void *ctx, int level, const char *file,
                    int line, const char *str)
{
    const char *p, *basename;

    /* Extract basename from file */
    for( p = basename = file; *p != '\0'; p++ )
        if( *p == '/' || *p == '\\' )
            basename = p + 1;
    (void)ctx;
    LOG(LOG_INFO,"%s:%04d: |%d| %s", basename, line, level, str );
}
#endif

static void user_tls_serv_task(void *param)
{
    DA16X_UNUSED_ARG(param);

    const char *pers = "tls_server";
    int8_t i = 0;
    int32_t ret = 0;
    char str_port[8] = {0x00, };

    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    mbedtls_x509_crt cert;
    mbedtls_pk_context pkey;
    mbedtls_pk_context pkey_alt;

    LOG(LOG_INFO, "[%s] Start of TLS Server task", __func__);

    //Init session
    mbedtls_net_init(&user_listen_ctx);
    mbedtls_net_init(&user_client_ctx);

    //Init SSL config
    mbedtls_ssl_config_init(&user_ssl_conf);

#ifdef USER_TLS_SERV_MULTI_CLIENTS_CONN_DEBUG
    mbedtls_ssl_conf_dbg(&user_ssl_conf, user_tls_serv_debug, NULL);
#endif

    //Init CTR-DRBG context
    mbedtls_ctr_drbg_init(&ctr_drbg);

    //Init Entropy context
    mbedtls_entropy_init(&entropy);

    //Init Certificate context
    mbedtls_x509_crt_init(&cert);

    //Init Private key context
    mbedtls_pk_init(&pkey);

    //Init Private key context for ALT
    mbedtls_pk_init(&pkey_alt);

    //Parse certificate
    ret = mbedtls_x509_crt_parse(&cert, user_tls_serv_cert, user_tls_serv_cert_len);
    if (ret) {
        LOG(LOG_ERR, "[%s] Failed to parse certificate(0x%x)", __func__, -ret);
        goto end_of_task;
    }

    //Parse private key
    ret = mbedtls_pk_parse_key(&pkey, user_tls_serv_key, user_tls_serv_key_len, NULL, 0);
    if (ret) {
        LOG(LOG_ERR, "[%s] Failed to parse private key(0x%x)", __func__, -ret);
        goto end_of_task;
    }

    LOG(LOG_INFO, "Bind on tls/*:%d...", USER_TLS_SERV_DEF_PORT);

    snprintf(str_port, sizeof(str_port), "%d", USER_TLS_SERV_DEF_PORT);

    ret = mbedtls_net_bind(&user_listen_ctx, NULL, str_port, MBEDTLS_NET_PROTO_TCP);
    if (ret) {
        LOG(LOG_ERR, "[%s] Failed to bind socket(0x%x)", __func__, -ret);
        goto end_of_task;
    }

    LOG(LOG_DBG, "Seeding the random number generator...");

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*)pers, strlen(pers));
    if (ret) {
        LOG(LOG_ERR, "[%s] Failed to set CTR-DRBG(0x%x)", __func__, -ret);
        goto end_of_task;
    }

    LOG(LOG_DBG, "Setting up the TLS structure...");

    //Set default configuration
    ret = mbedtls_ssl_config_defaults(&user_ssl_conf, MBEDTLS_SSL_IS_SERVER, 
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret) {
        LOG(LOG_ERR, "[%s] Failed to init tls config(0x%x)", __func__, -ret);
        goto end_of_task;
    }

    mbedtls_ssl_conf_rng(&user_ssl_conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    //Import certificate & private key
    if (mbedtls_pk_get_type(&pkey) == MBEDTLS_PK_RSA) {
        ret = mbedtls_pk_setup_rsa_alt(&pkey_alt, (void*)mbedtls_pk_rsa(pkey),
                                       user_tls_serv_rsa_decrypt_func,
                                       user_tls_serv_rsa_sign_func, 
                                       user_tls_serv_rsa_key_len_func);
        if (ret) {
            LOG(LOG_ERR, "[%s] Failed to set RSA-ALT(0x%x)", __func__, -ret);
            goto end_of_task;
        }

        ret = mbedtls_ssl_conf_own_cert(&user_ssl_conf, &cert, &pkey_alt);
        if (ret) {
            LOG(LOG_ERR, "[%s] Failed to set certificate(0x%x)", __func__, -ret);
            goto end_of_task;
        }
    } else {
        ret = mbedtls_ssl_conf_own_cert(&user_ssl_conf, &cert, &pkey);
        if (ret) {
            LOG(LOG_ERR, "[%s] Failed to set certificate(0x%x)", __func__, -ret);
            goto end_of_task;
        }
    }

    //No need to verify peer certificate.
    mbedtls_ssl_conf_authmode(&user_ssl_conf, MBEDTLS_SSL_VERIFY_NONE);

reset:
    //Wait until a client connects
    LOG(LOG_INFO, "Waiting for a remote connection...");

    ret = mbedtls_net_accept(&user_listen_ctx, &user_client_ctx, NULL, 0, NULL);
    if (ret) {
        LOG(LOG_ERR, "[%s] Failed to accept socket(0x%x)", __func__, -ret);
        goto end_of_task;
    }

    LOG(LOG_DBG, "mbedtls_net_accept done, move to the each tasks");
    for (i = 0; i < USER_TLS_MAX_NUM_TASKS; i++) {
        if (user_tls_conn_active[i] == 0) {
            break;
        }
    }

    if (i < USER_TLS_MAX_NUM_TASKS) {
        if (user_tls_conn_task_handle[i]) {
            user_tls_conn_task_handle[i] = NULL;
        }

        sprintf(user_tls_conn_name, "%s_%d", "TLS_CN", i);
        LOG(LOG_ERR, "user_tls_conn_name = %s", user_tls_conn_name);
        user_tls_conn_active[i] = 1;

        if (pdPASS
            != OS_TASK_CREATE(user_tls_conn_name,
                              user_tls_conn_task,
                              (void* )&i, USER_TLS_CONN_TASK_SIZE,
                              USER_TLS_CONN_TASK_PRI, user_tls_conn_task_handle[i]))
        {
            LOG(LOG_ERR, "Failed to create TLS connection task!");
        }
    } else {
        LOG(LOG_ERR, "No more connections available. close connection.");
        mbedtls_net_free(&user_client_ctx);
    }

    goto reset;

end_of_task:
    LOG(LOG_INFO, "[%s] End of TLS Server task", __func__);

    //Deinit session
    mbedtls_net_free(&user_listen_ctx);
    mbedtls_net_free(&user_client_ctx);

    //Deinit SSL config
    mbedtls_ssl_config_free(&user_ssl_conf);

    //Deinit CTR-DRBG context
    mbedtls_ctr_drbg_free(&ctr_drbg);

    //Deinit Entropy context
    mbedtls_entropy_free(&entropy);

    //Deinit Certificate context
    mbedtls_x509_crt_free(&cert);

    //Deinit Private key context
    mbedtls_pk_free(&pkey);

    //Deinit Private key context for ALT
    mbedtls_pk_free(&pkey_alt);

    OS_TASK_DELETE(user_tls_serv_handle);

    return;
}

static void user_tls_conn_task(void *param)
{
    DA16X_UNUSED_ARG(param);

    int32_t ret = 0;
    uint8_t *idx_ptr = (uint8_t*)param;
    uint8_t idx = *idx_ptr;
    uint8_t *data_buf = NULL;
    size_t len = USER_TLS_SERV_BUF_SZ;

    mbedtls_ssl_context ssl;
    mbedtls_net_context client_fd;
    mbedtls_ssl_config ssl_config;

    LOG(LOG_DBG, "[%s] Task created index = %d ", __func__, idx);
    memcpy(&client_fd, &user_client_ctx, sizeof(mbedtls_net_context));
    memcpy(&ssl_config, &user_ssl_conf, sizeof(mbedtls_ssl_config));

    mbedtls_ssl_init(&ssl);

    //Set up an SSL context for use.
    ret = mbedtls_ssl_setup(&ssl, &ssl_config);
    if (ret) {
        LOG(LOG_ERR, "[%s] Failed to set ssl config(0x%x)", __func__, -ret);
        goto task_exit;
    }

    LOG(LOG_INFO, "Start mbedtls_ssl_set_bio");
    mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    LOG(LOG_INFO, "Performing the TLS handshake...");
    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if ((ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE)) {
            LOG(LOG_ERR, "[%s] Failed to do handshake(0x%x)", __func__, -ret);
            goto task_exit;
        }
    }

    data_buf = OS_MALLOC(USER_TLS_SERV_BUF_SZ);
    if (data_buf == NULL) {
        LOG(LOG_ERR, "[%s] Failed to alloc ! ", __func__);
        goto task_exit;
    }

read_from_client:
    /* Handle Data transmission */
    do {
        memset(data_buf, 0x00, USER_TLS_SERV_BUF_SZ);

        LOG(LOG_INFO, "<< Read from client");
        ret = mbedtls_ssl_read(&ssl, data_buf, len);
        if (ret <= 0) {
            switch (ret) {
            case MBEDTLS_ERR_SSL_WANT_READ:
            case MBEDTLS_ERR_SSL_WANT_WRITE:
                LOG(LOG_INFO, "Need more data - mbedtls_ssl_write(0x%x)", -ret);
                continue;
            case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                LOG(LOG_INFO, "[%s][%d] Connection was closed gracefully", __func__, __LINE__);
                goto close_connection;
            case MBEDTLS_ERR_NET_CONN_RESET:
                LOG(LOG_INFO, "[%s][%d] Connection was reset by peer", __func__, __LINE__);
                goto close_connection;
            default:
                LOG(LOG_ERR, "[%s][%d] Failed to read data(0x%x)", -ret);
                goto close_connection;
            }
        }

        len = ret;
        LOG(LOG_INFO, ": %d bytes read", len);

        user_tls_serv_hex_dump("Received data", (unsigned char*)data_buf, len);
        if (ret > 0) {
            LOG(LOG_INFO, ">> Write back (echo) to client");
            break;
        }
    } while (1);

    while ((ret = mbedtls_ssl_write(&ssl, (uint8_t*)data_buf, len)) <= 0) {
        if (ret == MBEDTLS_ERR_NET_CONN_RESET) {
            LOG(LOG_INFO, "Failed: peer closed the connection");
            goto close_connection;
        }

        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            LOG(LOG_ERR, "Failed: mbedtls_ssl_write returned -0x%04x", (unsigned int )ret);
            goto close_connection;
        }

    }
    LOG(LOG_INFO, ": %d bytes written", len);
    goto read_from_client;

close_connection:
    OS_FREE(data_buf);

    LOG(LOG_INFO, "SSL close notify");
    while ((ret = mbedtls_ssl_close_notify(&ssl)) < 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            LOG(LOG_INFO, "Failed: mbedtls_ssl_close_notify returned -0x%04x", (unsigned int )ret);
            goto task_exit;
        }
    }

    LOG(LOG_DBG, " ok");
    ret = 0;

task_exit:
    user_tls_conn_active[idx] = 0;
    LOG(LOG_DBG, "[%s] task_conn_active[%d] = %d ", __func__, idx, user_tls_conn_active[idx]);

#ifdef MBEDTLS_ERROR_C
    if (ret != 0) {
        char *error_buf = OS_MALLOC(USER_TLS_SERV_STR_ERR_SZ);
        if (error_buf == NULL) {
            LOG(LOG_ERR, "[%s] Failed to alloc ! ", __func__);
        }
        else
        {
            mbedtls_strerror(ret, error_buf, USER_TLS_SERV_STR_ERR_SZ);
            LOG(LOG_INFO, " Last evt/error was: -0x%04x - %s",
                (unsigned int )-ret, error_buf);
            OS_FREE(error_buf);
        }
    }
#endif

    mbedtls_net_free(&client_fd);
    mbedtls_ssl_free(&ssl);

    OS_TASK_DELETE(user_tls_conn_task_handle[idx]);

    return;
}

void user_tls_serv_entry(void)
{
    if (user_tls_serv_handle != NULL) {
        configASSERT(0);
    }

    if (pdPASS
        != OS_TASK_CREATE(USER_TLS_SERV_TASK_NAME, user_tls_serv_task, NULL,
                          USER_TLS_SERV_TASK_STACK_SZ, USER_TLS_SERV_TASK_PRI,
                          user_tls_serv_handle))
    {
        LOG(LOG_ERR, "Failed to start TLS server task!");
        configASSERT(0);
    }
}

