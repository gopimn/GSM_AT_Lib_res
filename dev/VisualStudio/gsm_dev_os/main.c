// gsm_dev_os.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "gsm/gsm.h"

#include "gsm/apps/gsm_mqtt_client_api.h"
#include "gsm/gsm_mem.h"

static void main_thread(void* arg);
DWORD main_thread_id;

static gsmr_t gsm_evt(gsm_evt_t* evt);
static gsmr_t gsm_conn_evt(gsm_evt_t* evt);

gsm_operator_t operators[10];
size_t operators_len;

gsm_sms_entry_t sms_entry;

gsm_sms_entry_t sms_entries[10];
size_t sms_entries_read;

gsm_pb_entry_t pb_entries[10];
size_t pb_entries_read;

gsm_operator_curr_t operator_curr;

typedef struct {
    const char* pin_default;
    const char* pin;
    const char* puk;
} my_sim_t;
my_sim_t sim = {
    .pin = "1234",
    .puk = "81641429",
};

/**
 * \brief           Program entry point
 */
int
main() {
    printf("App start!\r\n");

    /* Create start main thread */
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)main_thread, NULL, 0, &main_thread_id);

    /* Do nothing at this point but do not close the program */
	while (1) {
        gsm_delay(1000);
	}
}

/* COnnection request data */
static const
uint8_t request_data[] = ""
"GET / HTTP/1.1\r\n"
"Host: example.com\r\n"
"Connection: Close\r\n"
"\r\n";

/**
 * \brief           Client ID is structured from ESP station MAC address
 */
static char
mqtt_client_id[13];

/**
 * \brief           Connection information for MQTT CONNECT packet
 */
static const gsm_mqtt_client_info_t
mqtt_client_info = {
    /* Server login data */
    .user = "8a215f70-a644-11e8-ac49-e932ed599553",
    .pass = "26aa943f702e5e780f015cd048a91e8fb54cca28",

    /* Device identifier address */
    .id = "2c3573a0-0176-11e9-a056-c5cffe7f75f9",

    .keep_alive = 10,
};

/**
 * \brief           MQTT client API thread
 */
void
mqtt_client_api_thread(void const* arg) {
    gsm_mqtt_client_api_p client;
    gsm_mqtt_conn_status_t conn_status;
    gsm_mqtt_client_api_buf_p buf;
    gsmr_t res;

    /* Create new MQTT API */
    client = gsm_mqtt_client_api_new(256, 128);
    if (client == NULL) {
        goto terminate;
    }

    while (1) {
        /* Make a connection */
        printf("Joining MQTT server\r\n");

        /* Try to join */
        conn_status = gsm_mqtt_client_api_connect(client, "mqtt.mydevices.com", 1883, &mqtt_client_info);
        if (conn_status == GSM_MQTT_CONN_STATUS_ACCEPTED) {
            printf("Connected and accepted!\r\n");
            printf("Client is ready to subscribe and publish to new messages\r\n");
        } else {
            printf("Connect API response: %d\r\n", (int)conn_status);
            gsm_delay(5000);
            continue;
        }

        /* Subscribe to topics */
        if (gsm_mqtt_client_api_subscribe(client,
            "v1/8a215f70-a644-11e8-ac49-e932ed599553/things/b24908a0-a652-11e8-af33-cfeebe848e6e/cmd/#",
            GSM_MQTT_QOS_AT_LEAST_ONCE) == gsmOK) {
            printf("Subscribed to esp8266_mqtt_topic\r\n");
        } else {
            printf("Problem subscribing to topic!\r\n");
        }

        while (1) {
            /* Receive MQTT packet with 1000ms timeout */
            res = gsm_mqtt_client_api_receive(client, &buf, 1000);
            if (res == gsmOK) {
                if (buf != NULL) {
                    const char* payload;
                    printf("Publish received!\r\n");
                    printf("Topic: %s, payload: %s\r\n", buf->topic, buf->payload);

                    /* Check for light switch */
                    if ((strstr(buf->topic, "/cmd/25")) != NULL) {
                        payload = strstr((void *)buf->payload, ",");
                        if (payload != NULL) {
                            payload++;
                            if (*payload == '1') {
                                printf("RELAY ENABLED!\r\n");
                            } else {
                                printf("RELAY DISABLED!\r\n");
                            }

                            /* Format topic name */
                            gsm_mqtt_client_api_publish(client, 
                                "v1/8a215f70-a644-11e8-ac49-e932ed599553/things/b24908a0-a652-11e8-af33-cfeebe848e6e/data/25",
                                payload, 1, GSM_MQTT_QOS_AT_LEAST_ONCE, 1);
                        }
                    }
                    gsm_mqtt_client_api_buf_free(buf);
                    buf = NULL;
                }
            } else if (res == gsmCLOSED) {
                printf("MQTT connection closed!\r\n");
                break;
            } else if (res == gsmTIMEOUT) {
                /* Receive timeout */
                /* printf("Timeout\r\n"); */
            }
        }
    }

terminate: 
    gsm_mqtt_client_api_delete(client);
    printf("MQTT client thread terminate\r\n");
    gsm_sys_thread_terminate(NULL);
}

/**
 * \brief           Main thread for init purposes
 */
static void
main_thread(void* arg) {
    size_t i;
    int16_t rssi;
    gsm_netconn_p nc;

    /* Init GSM library */
    gsm_init(gsm_evt, 1);
    gsm_delay(1000);

    //gsm_delay(5000);
    //gsm_delay(5000);
    
    gsm_operator_scan(operators, GSM_ARRAYSIZE(operators), &operators_len, 1);

#if GSM_CFG_CALL
    gsm_call_enable(1);
    //gsm_call_start("+38640167724", 1);
#endif /* GSM_CFG_CALL */

#if GSM_CFG_SMS
    gsm_sms_enable(1);
    //gsm_sms_send("+38640167724", "Tilen MAJERLE", 1);
#endif /* GSM_CFG_SMS */

    gsm_delay(8000);

    gsm_pb_enable(1);
    gsm_pb_read(GSM_MEM_CURRENT, 1, pb_entries, 1);

    //while (1) {
    //    printf("Attaching...\r\n");
    //    if (gsm_network_attach("internet", "", "", 1) == gsmOK) {
    //        printf("Attached to network\r\n");
    //        gsm_network_detach(1);
    //    } else {
    //        printf("Cannot attach to network!\r\n");
    //    }
    //}

#if GSM_CFG_NETCONN
    gsm_sys_thread_create(NULL, "mqtt_thread", (gsm_sys_thread_fn)mqtt_client_api_thread, NULL, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO);
    while (1) {
        if (!gsm_network_is_attached()) {
            /* Try to attach manually! */
            printf("Attaching...\r\n");
            if (gsm_network_attach("internet", "", "", 1) == gsmOK) {
                printf("Attached to network\r\n");
            } else {
                printf("Cannot attach to network!\r\n");
            }
        }
        gsm_delay(10000);
    }

    nc = gsm_netconn_new(GSM_NETCONN_TYPE_TCP);
    if (nc != NULL) {
        if (gsm_netconn_connect(nc, "example.com", 80) == gsmOK) {
            printf("Connected to example.com!\r\n");
            if (gsm_netconn_write(nc, request_data, sizeof(request_data) - 1) == gsmOK) {
                gsm_pbuf_p p;
                gsmr_t res;

                printf("NETCONN data written!\r\n");
                gsm_netconn_flush(nc);
                printf("NETCONN data flushed!\r\n");

                while (1) {
                    res = gsm_netconn_receive(nc, &p);
                    if (res == gsmOK) {
                        printf("New data packet received. Length: %d\r\n", (int)gsm_pbuf_length(p, 1));
                    
                    } else if (res == gsmCLOSED) {
                        printf("Netconn connection closed by remote side!\r\n");
                        break;
                    }
                }
            }
        } else {
            printf("Cannot connect to example.com!\r\n");
        }
        gsm_netconn_delete(nc);
    }

#endif /* GSM_CFG_NETCONN */

    //printf("Detaching...\r\n");
    gsm_network_detach(1);

    gsm_delay(5000);

    /* Terminate thread */
    gsm_sys_thread_terminate(NULL);
}

static gsmr_t
gsm_conn_evt(gsm_evt_t* evt) {
    gsm_conn_p c;
    c = gsm_conn_get_from_evt(evt);
    switch (gsm_evt_get_type(evt)) {
#if GSM_CFG_CONN
        case GSM_EVT_CONN_ACTIVE: {
            printf("Connection active\r\n");
            //gsm_conn_send(c, request_data, sizeof(request_data) - 1, NULL, 0);
            break;
        }
        case GSM_EVT_CONN_ERROR: {
            printf("Connection error\r\n");
            break;
        }
        case GSM_EVT_CONN_CLOSED: {
            printf("Connection closed\r\n");
            break;
        }
        case GSM_EVT_CONN_SEND: {
            gsmr_t res = gsm_evt_conn_send_get_result(evt);
            if (res == gsmOK) {
                printf("Data sent!\r\n");
            } else {
                printf("Data send error!\r\n");
            }
            break;
        }
        case GSM_EVT_CONN_RECV: {
            gsm_pbuf_p p = gsm_evt_conn_recv_get_buff(evt);
            printf("DATA RECEIVED: %d\r\n", (int)gsm_pbuf_length(p, 1));
            gsm_conn_recved(c, p);
            break;
        }
#endif /* GSM_CFG_CONN */
        default: break;
    }
    return gsmOK;
}

/**
 * \brief           Global GSM event function callback
 * \param[in]       cb: Event information
 * \return          gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
gsm_evt(gsm_evt_t* evt) {
    switch (gsm_evt_get_type(evt)) {
        case GSM_EVT_INIT_FINISH: {
            gsm_set_at_baudrate(115200, 0);
            break;
        }
        case GSM_EVT_RESET_FINISH: {
            printf("Reset finished!\r\n");
            break;
        }
        case GSM_EVT_RESET: {
            printf("Device reset!\r\n");
            break;
        }
        case GSM_EVT_DEVICE_IDENTIFIED: {
            printf("Device has been identified!\r\n");
            break;
        }
        case GSM_EVT_CPIN: {
            if (evt->evt.cpin.state == GSM_SIM_STATE_PUK) {
                gsm_sim_puk_enter(sim.puk, sim.pin, 0);
            } else if (evt->evt.cpin.state == GSM_SIM_STATE_PIN) {
                gsm_sim_pin_enter(sim.pin, 0);
            }
            break;
        }
        case GSM_EVT_SIGNAL_STRENGTH: {
            int16_t rssi = gsm_evt_signal_strength_get_rssi(evt);
            printf("Signal strength: %d\r\n", (int)rssi);
            break;
        }
        case GSM_EVT_NETWORK_REG: {
            gsm_network_reg_status_t status = gsm_network_get_reg_status();
            printf("Network registration changed. New status: %d! ", (int)status);
            switch (status) {
                case GSM_NETWORK_REG_STATUS_CONNECTED: printf("Connected to home network!\r\n"); break;
                case GSM_NETWORK_REG_STATUS_CONNECTED_ROAMING: printf("Connected to network and roaming!\r\n"); break;
                case GSM_NETWORK_REG_STATUS_SEARCHING: printf("Searching for network!\r\n"); break;
                case GSM_NETWORK_REG_STATUS_SIM_ERR: printf("SIM error\r\n"); break;
                default: break;
            }
            break;
        }
        case GSM_EVT_NETWORK_OPERATOR_CURRENT: {
            const gsm_operator_curr_t* op = gsm_evt_network_operator_get_current(evt);
            if (op != NULL) {
                if (op->format == GSM_OPERATOR_FORMAT_LONG_NAME) {
                    printf("Operator long name: %s\r\n", op->data.long_name);
                } else if (op->format == GSM_OPERATOR_FORMAT_SHORT_NAME) {
                    printf("Operator short name: %s\r\n", op->data.short_name);
                } else if (op->format == GSM_OPERATOR_FORMAT_NUMBER) {
                    printf("Operator number: %d\r\n", (int)op->data.num);
                }
            }
            break;
        }
#if GSM_CFG_NETWORK
        case GSM_EVT_NETWORK_ATTACHED: {
            gsm_ip_t ip;

            printf("\r\n---\r\n--- Network attached! ---\r\n---\r\n");
            if (gsm_network_copy_ip(&ip) == gsmOK) {
                printf("\r\n---\r\n--- IP: %d.%d.%d.%d ---\r\n---\r\n",
                    (int)ip.ip[0], (int)ip.ip[1], (int)ip.ip[2], (int)ip.ip[3]
                );
            }
            break;
        }
        case GSM_EVT_NETWORK_DETACHED: {
            printf("\r\n---\r\n--- Network detached! ---\r\n---\r\n");
            break;
        }
#endif /* GSM_CFG_NETWORK */
#if GSM_CFG_CALL
        case GSM_EVT_CALL_READY: {
            printf("Call is ready!\r\n");
            gsm_call_start("+38640167724", 0);
            break;
        }
        case GSM_EVT_CALL_CHANGED: {
            const gsm_call_t* call = evt->evt.call_changed.call;
            printf("Call changed!\r\n");
            if (call->state == GSM_CALL_STATE_ACTIVE) {
                printf("Call active!\r\n");
                if (call->dir == GSM_CALL_DIR_MT) {
                    gsm_call_hangup(0);
                }
            } else if (call->state == GSM_CALL_STATE_INCOMING) {
                printf("Incoming call. Answering...\r\n");
                //gsm_call_answer(0);
                gsm_call_hangup(0);
                gsm_sms_send(call->number, "Device does not accept calls!", 0);
            }
            break;
        }
#endif /* GSM_CFG_CALL */
#if GSM_CFG_SMS
        case GSM_EVT_SMS_READY: {
            printf("SMS is ready!\r\n");
            //gsm_sms_send("+38640167724", "Device reset and ready for more operations!", 0);
            break;
        }
        case GSM_EVT_SMS_SEND: {
            if (evt->evt.sms_send.res == gsmOK) {
                printf("SMS sent successfully!\r\n");
            } else {
                printf("SMS was not sent!\r\n");
            }
            break;
        }
        case GSM_EVT_SMS_RECV: {
            printf("SMS received: %d\r\n", (int)evt->evt.sms_recv.pos);
            gsm_sms_read(evt->evt.sms_recv.mem, evt->evt.sms_recv.pos, &sms_entry, 0, 0);
            //gsm_sms_read(GSM_MEM_CURRENT, evt->evt.sms_recv.pos, &sms_entry, 0, 0);
            gsm_sms_delete(evt->evt.sms_recv.mem, evt->evt.sms_recv.pos, 0);
            //gsm_sms_delete(evt->evt.sms_recv.mem, evt->evt.sms_recv.pos, 0);
            break;
        }
        case GSM_EVT_SMS_READ: {
            gsm_sms_entry_t* e = evt->evt.sms_read.entry;
            printf("SMS read: num: %s, name: %s, data: %s\r\n", e->number, e->name, e->data);
            break;
        }
        case GSM_EVT_SMS_LIST: {
            gsm_sms_entry_t* e = evt->evt.sms_list.entries;
            size_t i;

            for (i = 0; i < evt->evt.sms_list.size; i++) {
                printf("SMS LIST: pos: %d, num: %s, content: %s\r\n",
                    (int)e->pos, e->number, e->data);
                e++;
            }
            break;
        }
#endif /* GSM_CFG_SMS */
#if GSM_CFG_PHONEBOOK
        case GSM_EVT_PB_LIST: {
            gsm_pb_entry_t* e = evt->evt.pb_list.entries;
            size_t i;

            for (i = 0; i < evt->evt.pb_list.size; i++) {
                printf("PB LIST: pos: %d, num: %s, name: %s\r\n",
                    (int)e->pos, e->number, e->name);
                e++;
            }
            break;
        }
        case GSM_EVT_PB_SEARCH: {
            gsm_pb_entry_t* e = evt->evt.pb_search.entries;
            size_t i;

            for (i = 0; i < evt->evt.pb_search.size; i++) {
                printf("PB READ search: pos: %d, num: %s, name: %s\r\n",
                    (int)e->pos, e->number, e->name);
                e++;
            }
            break;
        }
#endif /* GSM_CFG_PHONECALL */
        default: break;
    }
	return gsmOK;
}
