#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/dns.h"


#include "wifi.h"
#define STR_BUFF_LEN 128
#define STR_BUFF_LEN_SCAN_SIZE "127"

void get_user_input(char* wifi_ssid, char* wifi_password, char* ip_addr, char* ip_netmask, char* ip_gateway);
int setup_ip_addr(const char* ip_addr, const char* ip_netmask, const char* ip_gateway);

int wifi_init_and_connect() {
    printf("Setting up wifi.\n");
    if (cyw43_arch_init()) {
        printf("Failed to initialise wifi.\n");
        return 1;
    }
    // extern cyw43_t cyw43_state;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    cyw43_arch_enable_sta_mode();

    int max_tries = 30;
    int i = 0;

#if (defined (WIFI_SSID) && defined (WIFI_PASSWORD))
    printf("Using defined credentals.\n");
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0 && i++ < max_tries) {
        printf("Connecting...\n");
    }
    if (i >= max_tries) {
        printf("Max tries exceeded.\n");
        return 1;
    }
    printf("Conneted.\n");
#if (defined (IP_ADDR) && defined (IP_NETMASK) && defined (IP_GATEWAY))
    if (setup_ip_addr(IP_ADDR, IP_NETMASK, IP_GATEWAY)) { return 1; }
#endif

#else // no wifi credentials defined
    {
        char wifi_ssid[STR_BUFF_LEN] = "\0";
        char wifi_password[STR_BUFF_LEN] = "\0";
        char ip_addr[STR_BUFF_LEN] = "\0";
        char ip_netmask[STR_BUFF_LEN] = "\0";
        char ip_gateway[STR_BUFF_LEN] = "\0";

        get_user_input(wifi_ssid, wifi_password, ip_addr, ip_netmask, ip_gateway);
        while (cyw43_arch_wifi_connect_timeout_ms(wifi_ssid, wifi_password, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0 && i++ < max_tries) {
            printf("Connecting...\n");
        }
        if (i >= max_tries) {
            printf("Max tries exceeded.\n");
            return 1;
        }
        printf("Conneted.\n");

        if (ip_addr[0] != '\0' && ip_netmask[0] != '\0' && ip_gateway[0] != '\0') {
            if (setup_ip_addr(ip_addr, ip_netmask, ip_gateway)) {
                printf("Please restart the device to input correct data.\n");
                return 1;
            }
        }
    }
#endif

    u32_t ip_addr = cyw43_state.netif[CYW43_ITF_STA].ip_addr.addr;
    printf("IP Address: %lu.%lu.%lu.%lu\n", ip_addr & 0xFF, (ip_addr >> 8) & 0xFF, (ip_addr >> 16) & 0xFF, ip_addr >> 24);

    return 0;
}

int setup_ip_addr(const char* ip_addr, const char* ip_netmask, const char* ip_gateway) {
    ip4_addr_t _ipaddr, _netmask, gw, dns;
    if (!ip4addr_aton(ip_addr, &_ipaddr)) {
        printf("Invalid ip address.");
        return 1;
    }
    if (!ip4addr_aton(ip_netmask, &_netmask)) {
        printf("Invalid netmask.");
        return 1;
    }
    if (!ip4addr_aton(ip_gateway, &gw)) {
        printf("Invalid gateway.");
        return 1;
    }
    struct netif* netif = &cyw43_state.netif[CYW43_ITF_STA];
    netif_set_addr(netif, &_ipaddr, &_netmask, &gw);
    netif_set_up(netif);
    return 0;
}

void get_user_input(char* wifi_ssid, char* wifi_password, char* ip_addr, char* ip_netmask, char* ip_gateway) {
    printf("SSID: ");
    scanf("%" STR_BUFF_LEN_SCAN_SIZE "s", wifi_ssid);
    printf("%s\n", wifi_ssid);
    printf("PASSWORD: ");
    scanf("%" STR_BUFF_LEN_SCAN_SIZE "s", wifi_password);
    printf("%s\n", wifi_password);
    char buff[STR_BUFF_LEN] = "\0";
    int decision = 0;
    while (true) {
        printf("Input network configuration (y/n)? ");
        scanf("%" STR_BUFF_LEN_SCAN_SIZE "s", buff);
        printf("%s\n", buff);
        if (strchr("yY", buff[0])) {
            decision = 1;
            break;
        }
        if (strchr("nN\n\r", buff[0])) {
            decision = 0;
            break;
        }
    }

    if (decision == 0) { return; }

    printf("\n");
    printf("IP ADDRESS: ");
    scanf("%" STR_BUFF_LEN_SCAN_SIZE "s", ip_addr);
    printf("%s\n", ip_addr);
    printf("NETMASK: ");
    scanf("%" STR_BUFF_LEN_SCAN_SIZE "s", ip_netmask);
    printf("%s\n", ip_netmask);
    printf("GATEWAY: ");
    scanf("%" STR_BUFF_LEN_SCAN_SIZE "s", ip_gateway);
    printf("%s\n", ip_gateway);

    return;
}
