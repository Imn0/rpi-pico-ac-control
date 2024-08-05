#pragma once

/**
 * @brief Initializes wifi, turns the onboard LED on
 * 
 * @return int 0 on success, 1 otherwise
 */
int wifi_init_and_connect();


// not needed
void wifi_keep_alive();
int wifi_recconect();
