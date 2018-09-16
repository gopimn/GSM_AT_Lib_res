// gsm_dev_os.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "gsm/gsm.h"

static void main_thread(void* arg);
DWORD main_thread_id;

extern gsm_device_driver_t gsm_device_sim800_900;

static gsmr_t gsm_cb(gsm_cb_t* cb);

gsm_operator_t operators[10];
size_t operators_len;

gsm_sms_entry_t sms_entry;

gsm_sms_entry_t sms_entries[10];
size_t sms_entries_read;

gsm_pb_entry_t pb_entries[10];
size_t pb_entries_read;

gsm_operator_curr_t operator_curr;

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

/**
 * \brief           Main thread for init purposes
 */
static void
main_thread(void* arg) {
    size_t i;
    int16_t rssi;

    /*
     * Init GSM library
     */
    gsm_init(gsm_cb, 1);
    gsm_delay(1000);
    gsm_sim_enter_pin("6636", 1);
    gsm_delay(1000);
    gsm_sim_remove_pin("6636", 1);

    gsm_delay(5000);
    
    gsm_operator_scan(operators, GSM_ARRAYSIZE(operators), &operators_len, 1);

    gsm_call_enable(1);

    gsm_sms_enable(1);
    gsm_sms_send("+38640167724", "Tilen MAJERLE", 1);

    //gsm_delay(5000);

    printf("Attaching...\r\n");
    gsm_network_attach("internet", "", "", 1);

    printf("Attached to network!\r\n");

    gsm_delay(10000);

    //printf("Detaching...\r\n");
    gsm_network_detach(1);

    gsm_delay(5000);

    /*
     * Terminate thread
     */
    gsm_sys_thread_terminate(NULL);
}

/**
 * \brief           Global GSM event function callback
 * \param[in]       cb: Event information
 * \return          gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
gsm_cb(gsm_cb_t* cb) {
    switch (cb->type) {
        case GSM_CB_INIT_FINISH: {
            gsm_set_at_baudrate(115200, 0);
            break;
        }
        case GSM_CB_RESET_FINISH: {
            printf("Reset finished!\r\n");
            break;
        }
        case GSM_CB_RESET: {
            printf("Device reset!\r\n");
            break;
        }
        case GSM_CB_DEVICE_IDENTIFIED: {
            printf("Device has been identified!\r\n");
            gsm_device_set_driver(&gsm_device_sim800_900);
            break;
        }
        case GSM_CB_OPERATOR_CURRENT: {
            const gsm_operator_curr_t* op = cb->cb.operator_current.operator_current;
            if (op->format == GSM_OPERATOR_FORMAT_LONG_NAME) {
                printf("Operator long name: %s\r\n", op->data.long_name);
            } else if (op->format == GSM_OPERATOR_FORMAT_SHORT_NAME) {
                printf("Operator short name: %s\r\n", op->data.short_name);
            } else if (op->format == GSM_OPERATOR_FORMAT_NUMBER) {
                printf("Operator number: %d\r\n", (int)op->data.num);
            }
            break;
        }
#if GSM_CFG_CALL
        case GSM_CB_CALL_READY: {
            printf("Call is ready!\r\n");
            //gsm_call_start("+38640167724", 0);
            break;
        }
        case GSM_CB_CALL_CHANGED: {
            const gsm_call_t* call = cb->cb.call_changed.call;
            printf("Call changed!\r\n");
            if (call->state == GSM_CALL_STATE_ACTIVE) {
                printf("Call active!\r\n");
                gsm_call_hangup(0);
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
        case GSM_CB_SMS_READY: {
            printf("SMS is ready!\r\n");
            //gsm_sms_send("+38640167724", "Device reset and ready for more operations!", 0);
            break;
        }
        case GSM_CB_SMS_SENT: {
            printf("SMS has been sent!\r\n");
            break;
        }
        case GSM_CB_SMS_SEND_ERROR: {
            printf("SMS was not sent!\r\n");
            break;
        }
        case GSM_CB_SMS_RECV: {
            printf("SMS received: %d\r\n", (int)cb->cb.sms_recv.pos);
            gsm_sms_read(cb->cb.sms_recv.mem, cb->cb.sms_recv.pos, &sms_entry, 0, 0);
            //gsm_sms_read(GSM_MEM_CURRENT, cb->cb.sms_recv.pos, &sms_entry, 0, 0);
            gsm_sms_delete(cb->cb.sms_recv.mem, cb->cb.sms_recv.pos, 0);
            //gsm_sms_delete(cb->cb.sms_recv.mem, cb->cb.sms_recv.pos, 0);
            break;
        }
        case GSM_CB_SMS_READ: {
            gsm_sms_entry_t* e = cb->cb.sms_read.entry;
            printf("SMS read: num: %s, name: %s, data: %s\r\n", e->number, e->name, e->data);
            break;
        }
        case GSM_CB_SMS_LIST: {
            gsm_sms_entry_t* e = cb->cb.sms_list.entries;
            size_t i;

            for (i = 0; i < cb->cb.sms_list.size; i++) {
                printf("SMS LIST: pos: %d, num: %s, content: %s\r\n",
                    (int)e->pos, e->number, e->data);
                e++;
            }
            break;
        }
#endif /* GSM_CFG_SMS */
#if GSM_CFG_PHONEBOOK
        case GSM_CB_PB_LIST: {
            gsm_pb_entry_t* e = cb->cb.pb_list.entries;
            size_t i;

            for (i = 0; i < cb->cb.pb_list.size; i++) {
                printf("PB LIST: pos: %d, num: %s, name: %s\r\n",
                    (int)e->pos, e->number, e->name);
                e++;
            }
            break;
        }
        case GSM_CB_PB_SEARCH: {
            gsm_pb_entry_t* e = cb->cb.pb_search.entries;
            size_t i;

            for (i = 0; i < cb->cb.pb_search.size; i++) {
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
