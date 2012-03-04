/* Copyright (c) 2012, Jan Vaughan
 * All rights reserved.
 *
 * Finds and opens the USB HID device from which to read log data.
 */

#ifndef HIDSELECT_H_
#define HIDSELECT_H_

#include "hidapi.h"

void fregister_hidselect();
hid_device* open_device();

#endif  /* HIDSELECT_H_ */
