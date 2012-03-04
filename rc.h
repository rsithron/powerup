/* Copyright (c) 2012, Jan Vaughan
 * All rights reserved.
 *
 * Return codes used within the powerup tool. Unrelated to the powerlog6s
 * device. Used both internally and as exit codes.
 */

#ifndef RC_H_
#define RC_H_

#define SUCCESS 0
#define NOT_EXPECTED_TO_RUN 1
#define DEVICE_MISSING 10
#define DEVICE_ERROR 11
#define READ_AGAIN 20
#define BAD_MESSAGE_LENGTH 21
#define OUTPUT_ERROR 30
#define USER_SUCKS -1

#endif  /* RC_H_ */
