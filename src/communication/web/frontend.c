#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"

#include "frontend.h"
#include "ac_control/mitsubishi_heavy.h"
#include "lwipopts.h"

static struct MH_ac_state* ac_state;

int frontend_init(struct MH_ac_state* _ac_state) {
    ac_state = _ac_state;
    httpd_init();
    printf("Http server initialised.\n");
    ssi_init();
    printf("SSI Handler initialised.\n");
    cgi_init();
    printf("CGI Handler initialised.\n");
    return 0;
}


const char* ssi_tags[] = { "Fan", "Mode", "Temp", "V_Swing", "H_Swing", "Other" };

u16_t ssi_handler(int iIndex, char* pcInsert, int iInsertLen) {

    size_t printed;

    char buff[128] = "\0";
    switch (iIndex) {
    case 0:
        MH_fan_str(ac_state, buff);
        printed = snprintf(pcInsert, iInsertLen, "%s", buff);
        break;
    case 1:
        MH_mode_str(ac_state, buff);
        printed = snprintf(pcInsert, iInsertLen, "%s", buff);
        break;
    case 2:
        MH_temperature_str(ac_state, buff);
        printed = snprintf(pcInsert, iInsertLen, "%s", buff);
        break;
    case 3:
        MH_vertical_air_str(ac_state, buff);
        printed = snprintf(pcInsert, iInsertLen, "%s", buff);
        break;
    case 4:
        MH_horizontal_air_str(ac_state, buff);
        printed = snprintf(pcInsert, iInsertLen, buff);
        break;
    case 5:
        MH_other_str(ac_state, buff);
        printed = snprintf(pcInsert, iInsertLen, buff);
        break;
    default:
        printed = 0;
        break;
    }

    return (u16_t)printed;
}

void ssi_init() {
    http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}

const char* cgi_mode_handler(int iIndex, int iNumParams, char* pcParam[], char* pcValue[]) {
    (void)iIndex;
    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "mode") == 0) {
            if (strcmp(pcValue[i], "auto") == 0)
                MH_mode_set(ac_state, MH_mode_auto);
            else if (strcmp(pcValue[i], "cooling") == 0)
                MH_mode_set(ac_state, MH_mode_cooling);
            else if (strcmp(pcValue[i], "heating") == 0)
                MH_mode_set(ac_state, MH_mode_heating);
            else if (strcmp(pcValue[i], "fan_only") == 0)
                MH_mode_set(ac_state, MH_mode_fan);
            else if (strcmp(pcValue[i], "drying") == 0)
                MH_mode_set(ac_state, MH_mode_drying);
            else if (strcmp(pcValue[i], "power_toggle") == 0)
                MH_power_toggle(ac_state);
        }
        else if (strcmp(pcParam[i], "fan") == 0) {
            if (strcmp(pcValue[i], "-1") == 0)
                MH_fan_cycle(ac_state, -1);
            else if (strcmp(pcValue[i], "1") == 0)
                MH_fan_cycle(ac_state, 1);
            else if (strcmp(pcValue[i], "eco") == 0)
                MH_fan_set(ac_state, MH_fan_speed_eco);
            else if (strcmp(pcValue[i], "high") == 0)
                MH_fan_set(ac_state, MH_fan_speed_high);
            else if (strcmp(pcValue[i], "auto") == 0)
                MH_fan_set(ac_state, MH_fan_speed_auto);
        }
        else if (strcmp(pcParam[i], "ex_mode") == 0) {
            if (strcmp(pcValue[i], "3d_auto") == 0)
                MH_3d_auto_toggle(ac_state);
            else if (strcmp(pcValue[i], "night") == 0)
                MH_night_setback_toggle(ac_state);
            else if (strcmp(pcValue[i], "silent") == 0)
                MH_silent_mode_toggle(ac_state);
            else if (strcmp(pcValue[i], "clean") == 0)
                MH_clean_allergen_toggle(ac_state, MH_clean_alergen_clean);
            else if (strcmp(pcValue[i], "allergen") == 0)
                MH_clean_allergen_toggle(ac_state, MH_clean_alergen_alergen);
        }
    }
    MH_send(ac_state);
    return "/index.shtml";
}

const char* cgi_temp_handler(int iIndex, int iNumParams, char* pcParam[], char* pcValue[]) {
    (void)iIndex;

    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "temp") == 0) {
            if (strcmp(pcValue[i], "-1") == 0)
                MH_temperature_change_delta(ac_state, -1);
            else if (strcmp(pcValue[i], "1") == 0)
                MH_temperature_change_delta(ac_state, 1);
        }
    }
    MH_send(ac_state);
    return "/index.shtml";
}

const char* cgi_swing_handler(int iIndex, int iNumParams, char* pcParam[], char* pcValue[]) {
    (void)iIndex;

    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "swing") == 0) {
            if (strcmp(pcValue[i], "ud") == 0)
                MH_vertical_air_cycle(ac_state);
            else if (strcmp(pcValue[i], "lr") == 0)
                MH_horizontal_air_cycle(ac_state);
            else if (strcmp(pcValue[i], "stop") == 0) {
                MH_vertical_air_set(ac_state, MH_vertical_air_stop);
                MH_horizontal_air_set(ac_state, MH_horizontal_air_stop);
            }
        }
    }
    MH_send(ac_state);
    return "/index.shtml";
}

static const tCGI cgi_handlers[] = {
    {
        "/mode.cgi", cgi_mode_handler
    },{
        "/temp.cgi", cgi_temp_handler
    },{
        "/swing.cgi", cgi_swing_handler
    },
};

void cgi_init() {
    http_set_cgi_handlers(cgi_handlers, 3);
}