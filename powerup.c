/* Copyright (c) 2012, Jan Vaughan
 * All rights reserved.
 *
 * Main application for reading log data.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "flags.h"
#include "hidapi.h"
#include "hidselect.h"
#include "powerlog6s.h"
#include "rc.h"

#define USB_BUF_LEN 64

DEFINE_bool(autoend, 1, "Exit when the device indicates the end of a log."
    " Only works when interpreting device data (see --interpret)");
DEFINE_bool(binary, 0, "Dump data as raw binary rather than CSV if "
    "interpreting the data, or hex if not");
DEFINE_bool(interpret, 1, "Interpret the binary data being read to "
    "output only log entires. If false, full buffers will be written");

void fregister_powerup() {
  REGISTER(interpret);
  REGISTER(binary);
  REGISTER(autoend);
}

int print_log(powerlog6s* log);
int print_raw(unsigned char* buf, int len);
int read_log(hid_device* device);
void terminate(int sig);

hid_device* device;

int main(int argc, char** argv) {
  int i;

  fregister_powerup();
  fregister_hidselect();
  fregister_flags();

  parse_flags(&argc, &argv);
  if (argc > 1) {
    fprintf(stderr, "I've got no idea what these args mean:\n");
    for (i = 1; i < argc; i++) {
      fprintf(stderr, "%s\n", argv[i]);
    }
    exit(USER_SUCKS);
  }

  signal(SIGINT, terminate);
  device = open_device();
  if (device) {
    if (FLAGS_interpret && !FLAGS_binary) {
      powerlog6s_csv_header();
    }
    do {
      i = read_log(device);
    } while (i == READ_AGAIN);
    hid_close(device);
    return i;
  } else {
    return DEVICE_MISSING;
  }
}

int print_log(powerlog6s* log) {
  if (!FLAGS_binary) {
    powerlog6s_csv_entry(log);
  } else if (fwrite(log, sizeof(powerlog6s), 1, stdout) < 1) {
    perror("Failed to write log entry");
    return OUTPUT_ERROR;
  }
  return READ_AGAIN;
}

int print_raw(unsigned char* buf, int len) {
  int i;
  if (!FLAGS_binary) {
    for (i = 0; i < len; i++) {
      fprintf(stderr, " %02x", buf[i]);
    }
    fprintf(stderr, "\n");
  } else if (fwrite(buf, len, 1, stdout) < 1) {
    perror("Failed to write raw data");
    return OUTPUT_ERROR;
  }
  return READ_AGAIN;
}

int read_log(hid_device* device) {
  unsigned char buf[USB_BUF_LEN];
  powerlog6s_base* base = (powerlog6s_base*) buf;
  powerlog6s* log = (powerlog6s*) buf;
  powerlog6s_ctl* ctl = (powerlog6s_ctl*) buf;
  int len;

  if (!device) {
    return DEVICE_MISSING;
  }
  len = hid_read(device, buf, USB_BUF_LEN);
  if (len == -1) {
    fprintf(stderr, "Error reading from device: %ls\n", hid_error(device));
    return DEVICE_ERROR;
  } else if (len == 0) {
    /* Nothing to read, make sure we don't get too busy. */
    usleep(10);
    return READ_AGAIN;
  } else if (!FLAGS_interpret) {
    return print_raw(buf, len);
  } else if (base->len < 2) {
    fprintf(stderr, "Unexpectedly short %u byte message.\n", base->len);
    return READ_AGAIN;
  } else if (base->len >= 3 && base->type == POWERLOG6S_CONTROL) {
    switch (ctl->cmd) {
      case POWERLOG6S_START:
      case POWERLOG6S_MID:
        /* Nothing interesting to do with start and mid. */
        return READ_AGAIN;
      case POWERLOG6S_END:
        if (FLAGS_autoend) {
          return SUCCESS;
        } else {
          return READ_AGAIN;
        }
      default:
        fprintf(stderr, "Unexpected control line cmd 0x%02x len %u.\n",
            ctl->cmd, ctl->len);
        return READ_AGAIN;
    }
  } else if (base->len >= 2 && base->type != POWERLOG6S_ONLINE
      && base->type != POWERLOG6S_OFFLINE) {
    fprintf(stderr, "Unexpected %u byte message of type %u.\n",
        log->len, log->type);
    return READ_AGAIN;
  } else if (log->len != sizeof(powerlog6s)) {
    fprintf(stderr, "Expected %zu byte log entry but got %u bytes (%u read).\n",
        sizeof(powerlog6s), log->len, len);
    return BAD_MESSAGE_LENGTH;
  } else {
    return print_log(log);
  }
}

void terminate(int sig) {
  if (device) {
    hid_close(device);
  }
  exit(0);
}
