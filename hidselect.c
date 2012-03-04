/* Copyright (c) 2012, Jan Vaughan
 * All rights reserved.
 *
 * Finds and opens the USB HID device from which to read log data.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "flags.h"
#include "rc.h"

#include "hidselect.h"

#define MAX_PATH_LEN 512

DEFINE_uint64(vendor, 0x0483, "Vendor ID of the USB device");
DEFINE_uint64(product, 0x5750, "Product ID of the USB device");
DEFINE_string(serial, NULL, "Serial number of the USB device");
DEFINE_string(device_path, NULL, "Path uniquely identifying the USB device, "
    "as an alternative to vendor/product/serial identification");

void fregister_hidselect() {
  REGISTER(device_path);
  REGISTER(serial);
  REGISTER(product);
  REGISTER(vendor);
}

void list_devices();
char* pick_device(char* path_buf);
void print_device(struct hid_device_info* info);
void strtowcs(char* str, wchar_t* buf);

hid_device* open_device() {
  hid_device* device;
  char path_buf[MAX_PATH_LEN];
  char* path;

  if (FLAGS_serial && FLAGS_device_path) {
    fprintf(stderr, "USB device should be specified by at most one of "
        "--serial and --device_path, not both.\n");
    exit(USER_SUCKS);
  }
  path = FLAGS_device_path;
  if (!path) {
    path = pick_device(path_buf);
  }
  if (!path) {
    fprintf(stderr, "PowerLog 6S not found. All detected devices:\n");
    list_devices();
    exit(DEVICE_MISSING);
  }
  device = hid_open_path(path);
  if (!device) {
    fprintf(stderr, "Failed to open device %s.\n", path);
    exit(DEVICE_ERROR);
  }
  return device;
}

void list_devices() {
  struct hid_device_info* iter;
  struct hid_device_info* curr;

  iter = hid_enumerate(0, 0);
  curr = iter;
  while (curr) {
    print_device(curr);
    curr = curr->next;
  }
  hid_free_enumeration(iter);
}

char* pick_device(char* path_buf) {
  char* path;
  wchar_t* sid;
  struct hid_device_info* iter;
  struct hid_device_info* curr;

  path = NULL;
  if (FLAGS_serial) {
    sid = (wchar_t*) malloc((strlen(FLAGS_serial) + 1) * sizeof(wchar_t));
    strtowcs(FLAGS_serial, sid);
  } else {
    sid = NULL;
  }
  iter = hid_enumerate(FLAGS_vendor, FLAGS_product);
  curr = iter;
  while (curr) {
    if (!sid || (sid && wcscmp(curr->serial_number, sid) == 0)) {
      if (path) {
        fprintf(stderr, "Ignoring extra device ");
      } else {
        fprintf(stderr, "Found ");
        strncpy(path_buf, curr->path, MAX_PATH_LEN);
        path = path_buf;
      }
      print_device(curr);
    }
    curr = curr->next;
  }
  hid_free_enumeration(iter);
  if (sid) {
    free(sid);
  }

  return path;
}

void print_device(struct hid_device_info* info) {
  fprintf(stderr, "%ls, %ls\n",
      info->product_string, info->manufacturer_string);
  fprintf(stderr, "    --vendor 0x%hx --product 0x%hx",
      info->vendor_id, info->product_id);
  if (info->serial_number && *info->serial_number != '\0') {
    fprintf(stderr, " --serial '%ls'", info->serial_number);
  }
  fprintf(stderr, "\n    --device_path '%s'\n", info->path);
}

void strtowcs(char *str, wchar_t* buf) {
  while (*str) {
    *buf++ = *str++;
  }
}
