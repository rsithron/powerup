/* Copyright (c) 2012, Jan Vaughan
 * All rights reserved.
 *
 * Device specifics of the Jun-Si PowerLog 6S. Defines formats and codes, as
 * well as some functions for converting data from the device into CSV.
 */

#ifndef POWERLOG6S_H_
#define POWERLOG6S_H_

#include <stdint.h>

/* Message types */
#define POWERLOG6S_ONLINE 0x10 /* use powerlog6s */
#define POWERLOG6S_OFFLINE 0x11 /* use powerlog6s */
#define POWERLOG6S_CONTROL 0x20 /* use powerlog6s_ctl */

/* Command types for control messages */
#define POWERLOG6S_START 0x20 /* x=lines, y=interval */
#define POWERLOG6S_MID 0x22 /* x=interval, y unused */
#define POWERLOG6S_END 0x21 /* x unused, y unused */

struct _powerlog6s_base {
  uint8_t len; /* number of bytes in this message */
  uint8_t type; /* type of message */
} __attribute__((packed));

struct _powerlog6s {
  uint8_t len; /* number of bytes in this message, 41 */
  uint8_t type; /* type of message, POWERLOG6S_ONLINE or POWERLOG6S_OFFLINE */
	uint32_t interval; /* milliseconds */
	uint8_t state;
	int16_t current; /* centiamps (amps x100) */
	uint16_t voltage; /* centivolts (volts x100) */
	uint32_t energy; /* milliamp hours */
	int16_t cell[6]; /* millivolts */
	uint16_t rpm; /* revolutions per minute */
	int16_t internal_temperature; /* decidegrees celsius (degrees celsius x10) */
	int16_t temperature[3]; /* decidegrees celsius (degrees celsius x10) */
	uint16_t period;
	uint16_t pulse;
} __attribute__((packed));

struct _powerlog6s_ctl {
  uint8_t len; /* number of bytes in this message, 3, 7 or 11 */
  uint8_t type; /* type of message, POWERLOG6S_CONTROL */
  uint8_t cmd; /* command type of the control message */
  uint64_t x;
  uint64_t y;
} __attribute__((packed));

typedef struct _powerlog6s_base powerlog6s_base;
typedef struct _powerlog6s powerlog6s;
typedef struct _powerlog6s_ctl powerlog6s_ctl;

void powerlog6s_csv_header();
void powerlog6s_csv_entry(powerlog6s* log);

#endif  /* POWERLOG6S_H_ */
