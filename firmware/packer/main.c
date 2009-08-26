/***************************************************************************
 *
 * OpenHC:                          ___                 _  _  ___
 *  Open source                    / _ \ _ __  ___ _ _ | || |/ __|
 *  Home                          | (_) | '_ \/ -_) ' \| __ | (__ 
 *  Control                        \___/| .__/\___|_||_|_||_|\___|
 * http://openhc.sourceforge.net/       |_| 
 *
 * Copyright (C) 2005 by Joerg Hohensohn
 *
 * All files in this archive are subject to the GNU General Public License.
 * See http://www.gnu.org/licenses/gpl-3.0.txt for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/ 

/*! \file main.c
    \brief Entry point of the AM module
*/

#define __FILENUM__ 13 // every file needs to have a unique (8bit) ID

#include <stdint.h>
#include "hal.h"
#include "busuart.h"
#include "timer.h"
#include "msg.h"
#include "bus.h"
#include "random.h"
#include "sender.h"
#include "settings.h"

int main(void)
{
    uint16_t addr;
    hal_sysinit();
    settings_read();
    addr = settings_getidhash();
    uart_init(addr>>8); // timeout affects collision recovery, use address

    rand_seed(addr);
    //bus_init();
    //packet_init();
    timer_init(sender_tick, addr>>8); // init with system-wide unique value
    sender_init();
    hal_watchdog_enable();
    sender_mainloop();
    return 0; // we won't get here
}
