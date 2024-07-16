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


const char* ssi_tags[] = { "Fan", "Mode", "Temp" };

u16_t ssi_handler(int iIndex, char* pcInsert, int iInsertLen) {

    size_t printed;

    char buff[128] = "\0";
    switch (iIndex) {
    case 0:
    {
        MH_fan_str(ac_state, buff);
        printed = snprintf(pcInsert, iInsertLen, "%s", buff);
    }
    break;
    case 1:
    {
        MH_mode_str(ac_state, buff);
        printed = snprintf(pcInsert, iInsertLen, "%s", buff);
    }
    break;
    case 2:
    {
        MH_temperature_str(ac_state, buff);
        printed = snprintf(pcInsert, iInsertLen, "%s", buff);
    }
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

        }
    }
    MH_send(ac_state);
    return "/index.shtml";
}

const char* cgi_temp_handler(int iIndex, int iNumParams, char* pcParam[], char* pcValue[]) {

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

static const tCGI cgi_handlers[] = {
    {
        "/mode.cgi", cgi_mode_handler
    },{
        "/temp.cgi", cgi_temp_handler
    },
};

void cgi_init() {
    http_set_cgi_handlers(cgi_handlers, 2);
}