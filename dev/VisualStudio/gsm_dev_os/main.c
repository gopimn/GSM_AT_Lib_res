// gsm_dev_os.cpp : Defines the entry point for the console application.
//

#include "windows.h"
#include "gsm/gsm.h"

#include "gsm/apps/gsm_mqtt_client_api.h"
#include "gsm/gsm_mem.h"

#include "mqtt_client_api.h"
#include "netconn_client.h"

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
    .pin = "1593",
    .puk = "86220404",
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

char model_str[10];

void
pin_evt(gsmr_t res, void* arg) {
    printf("PIN EVT function!\r\n");
}

void
puk_evt(gsmr_t res, void* arg) {
    printf("PUK EVT function!\r\n");
}

/**
 * \brief           Main thread for init purposes
 */
static void
main_thread(void* arg) {
    gsm_sim_state_t sim_state;

    /* Init GSM library */
    gsm_init(gsm_evt, 1);

    if (gsm_sim_pin_enter(sim.pin, NULL, NULL, 1) == gsmOK) {
        printf("PIN ENTERED OK!\r\n");
    } else {
        printf("PIN NOT ENTERED OK!\r\n");
    }

    gsm_device_get_manufacturer(model_str, sizeof(model_str), NULL, NULL, 1);
    printf("Manuf: %s\r\n", model_str);
    gsm_device_get_model(model_str, sizeof(model_str), NULL, NULL, 1);
    printf("Model: %s\r\n", model_str);
    gsm_device_get_serial_number(model_str, sizeof(model_str), NULL, NULL, 1);
    printf("Serial: %s\r\n", model_str);
    gsm_device_get_revision(model_str, sizeof(model_str), NULL, NULL, 1);
    printf("Revision: %s\r\n", model_str);

    while (1) {
        gsm_delay(1000);
    }

    /* Check for sim card */
    while ((sim_state = gsm_sim_get_current_state()) != GSM_SIM_STATE_READY) {
        if (sim_state == GSM_SIM_STATE_PIN) {
            printf("GSM state PIN\r\n");
            gsm_sim_pin_enter(sim.pin, pin_evt, NULL, 1);
        } else if (sim_state == GSM_SIM_STATE_PUK) {
            printf("GSM state PUK\r\n");
            gsm_sim_puk_enter(sim.puk, sim.pin, puk_evt, NULL, 1);
        } else if (sim_state == GSM_SIM_STATE_NOT_READY) {
            printf("GSM SIM state not ready!\r\n");
        } else if (sim_state == GSM_SIM_STATE_NOT_INSERTED) {
            printf("GSM SIM not inserted!\r\n");
        }
        gsm_delay(1000);
    }
    
    printf("Waiting home network...\r\n");
    while (gsm_network_get_reg_status() != GSM_NETWORK_REG_STATUS_CONNECTED) {
        gsm_delay(1000);
    }
    printf("Connected to home network...\r\n");

#if GSM_CFG_CALL
    gsm_call_enable(NULL, NULL, 1);

    /* Enable SIM with first call */
    //gsm_call_start("+38640167724", 0);
    //gsm_call_start("+38640167724", 1);
#endif /* GSM_CFG_CALL */

#if GSM_CFG_SMS
    gsm_sms_enable(NULL, NULL, 1);
    gsm_sms_send("7070", "PORABA", NULL, NULL, 1);
#endif /* GSM_CFG_SMS */

#if GSM_CFG_PHONEBOOK
    gsm_pb_enable(NULL, NULL, 1);
    gsm_pb_read(GSM_MEM_CURRENT, 1, pb_entries, NULL, NULL, 1);
#endif /* GSM_CFG_PHONEBOOK */

    printf("Enabling PDP context...\r\n");
    if (gsm_network_attach("internet", "", "", NULL, NULL, 1) == gsmOK) {
        printf("Attached to network\r\n");
    } else {
        printf("Cannot attach to network!\r\n");
    }

    //gsm_sys_thread_create(NULL, "mqtt_client_api", (gsm_sys_thread_fn)mqtt_client_api_thread, NULL, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO);
    //gsm_sys_thread_create(NULL, "netconn_client", (gsm_sys_thread_fn)netconn_client_thread, NULL, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO);

    gsm_delay(10000);

    printf("Detaching...\r\n");
    gsm_network_detach(NULL, NULL, 1);

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
            break;
        }
        case GSM_EVT_RESET: {
            if (gsm_evt_reset_get_result(evt) == gsmOK) {
                printf("Reset sequence finished with success!\r\n");
            }
            break;
        }
        case GSM_EVT_SIM_STATE_CHANGED: {            
            break;
        }
        case GSM_EVT_DEVICE_IDENTIFIED: {
            printf("Device has been identified!\r\n");
            break;
        }
        case GSM_EVT_SIGNAL_STRENGTH: {
            int16_t rssi = gsm_evt_signal_strength_get_rssi(evt);
            printf("Signal strength: %d\r\n", (int)rssi);
            break;
        }
        case GSM_EVT_NETWORK_REG_CHANGED: {
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
            gsm_call_start("+38640167724", NULL, NULL, 0);
            break;
        }
        case GSM_EVT_CALL_CHANGED: {
            const gsm_call_t* call = evt->evt.call_changed.call;
            printf("Call changed!\r\n");
            if (call->state == GSM_CALL_STATE_ACTIVE) {
                printf("Call active!\r\n");
                if (call->dir == GSM_CALL_DIR_MT) {
                    gsm_call_hangup(NULL, NULL, 0);
                }
            } else if (call->state == GSM_CALL_STATE_INCOMING) {
                printf("Incoming call. Answering...\r\n");
                //gsm_call_answer(0);
                gsm_call_hangup(NULL, NULL, 0);
                gsm_sms_send(call->number, "Device does not accept calls!", NULL, NULL, 0);
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
            gsm_sms_read(evt->evt.sms_recv.mem, evt->evt.sms_recv.pos, &sms_entry, 0, NULL, NULL, 0);
            gsm_sms_delete(evt->evt.sms_recv.mem, evt->evt.sms_recv.pos, NULL, NULL, 0);
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
