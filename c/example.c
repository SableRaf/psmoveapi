
 /**
 * PS Move API - An interface for the PS Move Motion Controller
 * Copyright (C) 2011 Thomas Perl <m@thp.io>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "psmove.h"

int main(int argc, char* argv[])
{
    PSMove *move;
    enum PSMove_Connection_Type ctype;
    int i;

    i = psmove_count_connected();
    printf("Connected controllers: %d\n", i);

    move = psmove_connect();

    if (move == NULL) {
        printf("Could not connect to default Move controller.\n"
               "Please connect one via USB or Bluetooth.\n");
        exit(1);
    }

    ctype = psmove_connection_type(move);
    switch (ctype) {
        case Conn_USB:
            printf("Connected via USB.\n");
            break;
        case Conn_Bluetooth:
            printf("Connected via Bluetooth.\n");
            break;
        case Conn_Unknown:
            printf("Unknown connection type.\n");
            break;
    }


    if (ctype == Conn_USB) {
        PSMove_Data_BTAddr addr;
        psmove_get_btaddr(move, &addr);
        printf("Current BT Host: ");
        for (i=0; i<6; i++) {
            printf("%02x ", addr[i]);
        }
        printf("\n");

#if 0
        /* This is the easy method (pair to this host): */
        if (psmove_pair(move)) {
            printf("Paired. Press the PS Button now :)\n");
        } else {
            printf("psmove_pair() failed :/\n");
        }

        /* This is the advanced method: */

        /* Example BT Address: 01:23:45:67:89:ab */
        const PSMove_Data_BTAddr newhost = {
            0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
        };
        if (!psmove_set_btaddr(move, &newhost)) {
            printf("Could not set BT address!\n");
        }
#endif
    }

    for (i=0; i<10; i++) {
        psmove_set_leds(move, 0, 255*(i%3==0), 0);
        psmove_set_rumble(move, 255*(i%2));
        psmove_update_leds(move);
        usleep(10000*(i%10));
    }

    for (i=250; i>=0; i-=5) {
        psmove_set_leds(move, i, i, 0);
        psmove_set_rumble(move, 0);
        psmove_update_leds(move);
    }

    psmove_set_leds(move, 0, 0, 0);
    psmove_set_rumble(move, 0);
    psmove_update_leds(move);

    while (!(psmove_get_buttons(move) & Btn_PS)) {
        int res = psmove_poll(move);
        if (res) {
            if (psmove_get_buttons(move) & Btn_TRIANGLE) {
                printf("Triangle pressed, with trigger value: %d\n",
                        psmove_get_trigger(move));
                psmove_set_rumble(move, psmove_get_trigger(move));
            } else {
                psmove_set_rumble(move, 0x00);
            }

            psmove_set_leds(move, 0, 0, psmove_get_trigger(move));

            int x, y, z;
            psmove_get_accelerometer(move, &x, &y, &z);
            printf("accel: %5d %5d %5d\n", x, y, z);
            psmove_get_gyroscope(move, &x, &y, &z);
            printf("gyro: %5d %5d %5d\n", x, y, z);
            psmove_get_magnetometer(move, &x, &y, &z);
            printf("magnetometer: %5d %5d %5d\n", x, y, z);
            printf("buttons: %x\n", psmove_get_buttons(move));

            int battery = psmove_get_battery(move);

            if (battery == Batt_CHARGING) {
                printf("battery charging\n");
            } else if (battery >= Batt_MIN && battery <= Batt_MAX) {
                printf("battery level: %d / %d\n", battery, Batt_MAX);
            } else {
                printf("battery level: unknown (%x)\n", battery);
            }

            psmove_update_leds(move);
        }
    }

    psmove_disconnect(move);

    return 0;
}

