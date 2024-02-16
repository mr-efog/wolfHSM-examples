/*
 * wolfHSM Client TCP Example
 */

#include <stdint.h>
#include <stdio.h>  /* For printf */
#include <string.h> /* For memset, memcpy */
#include <unistd.h> /* For sleep */

#include <pthread.h> /* For pthread_create/cancel/join/_t */

#if 0
#ifndef WOLFSSL_USER_SETTINGS
    #include "wolfssl/options.h"
#endif
#include "wolfssl/wolfcrypt/settings.h"
#endif


#include "wolfhsm/error.h"

#if 0
#include "wolfhsm/nvm.h"
#include "wolfhsm/nvm_flash.h"
#endif

#include "wolfhsm/comm.h"
#include "wolfhsm/message.h"

#include "wolfhsm/wh_client.h"

#include "port/posix/transport_tcp.h"

/** Local declarations */
void* wh_ClientTask(void* cf);


enum {
	REPEAT_COUNT = 10,
	REQ_SIZE = 32,
	RESP_SIZE = 64,
	ONE_MS = 1000,
};

#define WH_SERVER_TCP_IPSTRING "127.0.0.1"
#define WH_SERVER_TCP_PORT 23456
#define WH_CLIENT_ID 1234



void* wh_ClientTask(void* cf)
{
    whClientConfig* config = (whClientConfig*)cf;
    int ret = 0;
    whClient client[1];
    int counter = 1;

    uint8_t  tx_req[REQ_SIZE] = {0};
    uint16_t tx_req_len = 0;

    uint8_t  rx_resp[RESP_SIZE] = {0};
    uint16_t rx_resp_len = 0;

    if (config == NULL) {
        return NULL;
    }

    ret = wh_Client_Init(client, config);
    printf("wh_Client_Init:%d\n", ret);

    if (ret != 0) {
        return NULL;
    }

    for(counter = 0; counter < REPEAT_COUNT; counter++)
    {
        sprintf((char*)tx_req,"Request:%u",counter);
        tx_req_len = strlen((char*)tx_req);
        do {
            ret = wh_Client_EchoRequest(client,
                    tx_req_len, tx_req);
            if( ret != WH_ERROR_NOTREADY) {
                printf("Client EchoRequest:%d, len:%d, %s\n",
                        ret, tx_req_len, tx_req);
            }
        } while ((ret == WH_ERROR_NOTREADY) && (usleep(ONE_MS)==0));

        if (ret != 0) {
            printf("Client had failure. Exiting\n");
            break;
        }

        rx_resp_len = 0;
        memset(rx_resp, 0, sizeof(rx_resp));

        do {
            ret = wh_Client_EchoResponse(client,
                    &rx_resp_len, rx_resp);
            printf("Client EchoResponse:%d, len:%d, %s\n",
                    ret, rx_resp_len, rx_resp);
        } while ((ret == WH_ERROR_NOTREADY) && (usleep(ONE_MS)==0));

        if (ret != 0) {
            printf("Client had failure. Exiting\n");
            break;
        }
    }

    ret = wh_Client_Cleanup(client);
    printf("wh_Client_Cleanup:%d\n", ret);
    return NULL;
}

int main(int argc, char** argv)
{
    (void)argc; (void)argv;

    /* Client configuration/contexts */
    whTransportTcpClientContext tcc[1] = {};
    whTransportTcpConfig mytcpconfig[1] = {{
            .server_ip_string = WH_SERVER_TCP_IPSTRING,
            .server_port = WH_SERVER_TCP_PORT,
    }};

    whCommClientConfig cc_conf[1] = {{
            .transport_cb = whTransportTcpClient_Cb,
            .transport_context = (void*)tcc,
            .transport_config = (void*)mytcpconfig,
            .client_id = 1234,
    }};
    whClientConfig c_conf[1] = {{
            .comm = cc_conf,
    }};

    wh_ClientTask(c_conf);

    return 0;
}


