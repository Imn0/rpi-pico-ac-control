#pragma once

#include "ac_control/mitsubishi_heavy.h"

int frontend_init(struct MH_ac_state* _ac_state);
void ssi_init();
void cgi_init();