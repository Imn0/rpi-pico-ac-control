#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"

#include "frontend.h"

void frontend_init() {
    httpd_init();
    printf("Http server initialised\n");
    ssi_init();
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");
}


const char* ssi_tags[] = { "led" };

u16_t ssi_handler(int iIndex, char* pcInsert, int iInsertLen) {
    size_t printed;
    switch (iIndex) {
    case 0:
    {
        bool led_status = cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
        if (led_status == true) {
            printed = snprintf(pcInsert, iInsertLen, "ON");
        }
        else {
            printed = snprintf(pcInsert, iInsertLen, "OFF");
        }
    }
    break;
    default:
        printed = 0;
        break;
    }

    return (u16_t)printed;
}

void ssi_init() {
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}

const char* cgi_led_handler(int iIndex, int iNumParams, char* pcParam[], char* pcValue[]) {
    if (strcmp(pcParam[0], "temp") == 0) {
        if (strcmp(pcValue[0], "0") == 0)
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        else if (strcmp(pcValue[0], "1") == 0)
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    }
    return "/index.shtml";
}

static const tCGI cgi_handlers[] = {
    {
        "/temp.cgi", cgi_led_handler
    },
};

void cgi_init() {
    http_set_cgi_handlers(cgi_handlers, 1);
}