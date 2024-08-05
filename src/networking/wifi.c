#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "hardware/watchdog.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/def.h"
#include "lwip/inet_chksum.h"
#include "lwip/sys.h"

#include "wifi.h"

#define STR_BUFF_LEN 128
#define STR_BUFF_LEN_SCAN_SIZE "127"

void get_user_input(char* wifi_ssid, char* wifi_password, char* ip_addr, char* ip_netmask, char* ip_gateway);
int setup_ip_addr(const char* ip_addr, const char* ip_netmask, const char* ip_gateway);

static char wifi_ssid[STR_BUFF_LEN] = "\0";
static char wifi_password[STR_BUFF_LEN] = "\0";
static char ip_addr[STR_BUFF_LEN] = "\0";
static char ip_netmask[STR_BUFF_LEN] = "\0";
static char ip_gateway[STR_BUFF_LEN] = "\0";
static struct raw_pcb* ping_pcb = NULL;
static volatile bool ping_response_received = false;


#define PING_ID 0xAFAF
#define PING_DATA_SIZE 32

static void send_ping(const ip_addr_t* addr) {
    struct pbuf* p;
    struct icmp_echo_hdr* iecho;

    p = pbuf_alloc(PBUF_IP, sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE, PBUF_RAM);
    if (!p) {
        printf("Failed to allocate memory for ping request\n");
        return;
    }

    iecho = (struct icmp_echo_hdr*)p->payload;
    ICMPH_TYPE_SET(iecho, ICMP_ECHO);
    ICMPH_CODE_SET(iecho, 0);
    iecho->chksum = 0;
    iecho->id = PING_ID;
    iecho->seqno = lwip_htons(1);

    memset((char*)iecho + sizeof(struct icmp_echo_hdr), 0, PING_DATA_SIZE);
    iecho->chksum = inet_chksum(iecho, sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE);

    raw_sendto(ping_pcb, p, addr);
    pbuf_free(p);
}

static unsigned char ping_recv(void* arg, struct raw_pcb* pcb, struct pbuf* p, const ip_addr_t* addr) {
    (void)arg;
    (void)pcb;
    (void)addr;
    struct icmp_echo_hdr* iecho;
    if (p->tot_len >= (PBUF_IP_HLEN + sizeof(struct icmp_echo_hdr))) {
        iecho = (struct icmp_echo_hdr*)((u8_t*)p->payload + PBUF_IP_HLEN);
        if (iecho->id == PING_ID) {
            ping_response_received = true;
        }
    }
    pbuf_free(p);
    return 1;
}
void wifi_keep_alive() {

    ping_response_received = false;



    ip4_addr_t gateway_addr;
    ip4_addr_copy(gateway_addr, cyw43_state.netif[0].gw);

    send_ping(&gateway_addr);
    absolute_time_t timeout_time = make_timeout_time_ms(3000);
    while (!time_reached(timeout_time)) {
        cyw43_arch_poll();
        if (ping_response_received) {
            return;
        }
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(50);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(50);
    }
    printf("Gateway not reachable. Resetting WiFi...\n");
    wifi_recconect();
}

int wifi_recconect() {

    int max_tries = 30;
    int i = 0;
#if (defined (WIFI_SSID) && defined (WIFI_PASSWORD))
    printf("Using defined credentals.\n");

    snprintf(wifi_ssid, STR_BUFF_LEN, "%s", WIFI_SSID);
    snprintf(wifi_password, STR_BUFF_LEN, "%s", WIFI_PASSWORD);
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0 && i++ < max_tries) {
        printf("Connecting...\n");
    }
    if (i >= max_tries) {
        printf("Max tries exceeded.\n");
        return 1;
    }
    printf("Conneted.\n");
#if (defined (IP_ADDR) && defined (IP_NETMASK) && defined (IP_GATEWAY))
    snprintf(ip_addr, STR_BUFF_LEN, "%s", IP_ADDR);
    snprintf(ip_netmask, STR_BUFF_LEN, "%s", IP_NETMASK);
    snprintf(ip_gateway, STR_BUFF_LEN, "%s", IP_GATEWAY);
#endif

#else // no wifi credentials defined
    {


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

int wifi_init_and_connect() {
    printf("Setting up wifi.\n");
    if (cyw43_arch_init()) {
        printf("Failed to initialise wifi.\n");
        return 1;
    }
    // extern cyw43_t cyw43_state;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    cyw43_arch_enable_sta_mode();

    if (cyw43_wifi_pm(&cyw43_state, 0xa11140)) {
        printf("Failed to set power mode.\n");
        return 1;
    }
    int max_tries = 30;
    int i = 0;

#if (defined (WIFI_SSID) && defined (WIFI_PASSWORD))
    snprintf(wifi_ssid, STR_BUFF_LEN, "%s", WIFI_SSID);
    snprintf(wifi_password, STR_BUFF_LEN, "%s", WIFI_PASSWORD);
    printf("Using defined credentals.\n");
#if (defined (IP_ADDR) && defined (IP_NETMASK) && defined (IP_GATEWAY))
    snprintf(ip_addr, STR_BUFF_LEN, "%s", IP_ADDR);
    snprintf(ip_netmask, STR_BUFF_LEN, "%s", IP_NETMASK);
    snprintf(ip_gateway, STR_BUFF_LEN, "%s", IP_GATEWAY);
#endif

#else // no wifi credentials defined

    get_user_input(wifi_ssid, wifi_password, ip_addr, ip_netmask, ip_gateway);

#endif

    if (ip_addr[0] != '\0' && ip_netmask[0] != '\0' && ip_gateway[0] != '\0') {
        if (setup_ip_addr(ip_addr, ip_netmask, ip_gateway)) {
            printf("Please restart the device to input correct data.\n");
            return 1;
        }
        printf("Using static ip.\n");
        dhcp_release_and_stop(cyw43_state.netif);
    }

    while (cyw43_arch_wifi_connect_timeout_ms(wifi_ssid, wifi_password, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0 && i++ < max_tries) {
        printf("Connecting...\n");
    }
    if (i >= max_tries) {
        printf("Max tries exceeded.\n");
        return 1;
    }
    printf("Conneted.\n");

    u32_t ip_addr = cyw43_state.netif[CYW43_ITF_STA].ip_addr.addr;
    printf("IP Address: %lu.%lu.%lu.%lu\n", ip_addr & 0xFF, (ip_addr >> 8) & 0xFF, (ip_addr >> 16) & 0xFF, ip_addr >> 24);

    ping_pcb = raw_new(IP_PROTO_ICMP);
    if (!ping_pcb) {
        printf("Failed to create ping PCB\n");
    }

    raw_recv(ping_pcb, ping_recv, NULL);
    if (raw_bind(ping_pcb, IP_ADDR_ANY) != ERR_OK) {
        printf("Can't bind pcb.\n");
        return 1;
    }
    return 0;
}

int setup_ip_addr(const char* ip_addr, const char* ip_netmask, const char* ip_gateway) {
    ip4_addr_t _ipaddr, _netmask, gw;
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
