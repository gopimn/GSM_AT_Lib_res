#include "sim_manager.h"
#include "gsm/gsm.h"

/**
 * \brief           SIM card pin code
 */
const char *
pin_code = "2573";

/**
 * \brief           SIM card puk code
 */
const char *
puk_code = "26466162";

/**
 * \brief           Configure and enable SIM card
 * \return          `1` on success, `0` otherwise
 */
uint8_t
configure_sim_card(void) {
    if (pin_code != NULL && strlen(pin_code)) {
        if (gsm_sim_pin_enter(pin_code, NULL, NULL, 1) == gsmOK) {
            return 1;
        }
        return 0;
    }
    return 1;
}
