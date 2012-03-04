/* Copyright (c) 2012, Jan Vaughan
 * All rights reserved.
 *
 * Device specific functions for converting data from the Jun-Si PowerLog 6S
 * into CSV.
 */

#include <stdio.h>

#include "powerlog6s.h"

void powerlog6s_csv_header() {
  printf("interval,state,current (cA),voltage (cV),energy (mAh),");
  printf("cell1 (mV),cell2 (mV),cell3 (mV),cell4 (mV),cell5 (mV),cell6 (mV),");
  printf("rpm,internal_temp (ddC),temp2 (ddC),temp3 (ddC),temp4 (ddC),");
  printf("period,pulse\n");
}

void powerlog6s_csv_entry(powerlog6s* log) {
  int i;
  printf("%u,%x,%d,%u,%u,", log->interval, log->state, log->current,
      log->voltage, log->energy);
  for (i = 0; i < 6; i++) {
    printf("%d,", log->cell[i]);
  }
  printf("%u,%d,", log->rpm, log->internal_temperature);
  for (i = 0; i < 3; i++) {
    printf("%d,", log->temperature[i]);
  }
  printf("%u,%u\n", log->period, log->pulse);
}
