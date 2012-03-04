/* Copyright (c) 2012, Jan Vaughan
 * All rights reserved.
 *
 * Neater command line flag handling than getopts. Heavily based on the C++
 * gflags, but not as fully featured nor careful around edge cases.
 *   http://code.google.com/p/google-gflags/
 * Their license is included below:
 *
 * Copyright (c) 2011, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rc.h"

#include "flags.h"

DEFINE_bool(help, 0, "Show help on all flags");

void fregister_flags() {
  REGISTER(help);
}

struct flag_info* flags_list = NULL;

struct flag_info* find_flag(char* name);
int process_bool(struct flag_info* flag, char* value);
int process_flag(struct flag_info* flag, char* value);
int process_num(struct flag_info* flag, char* value);
void print_help(char* cmd);
struct flag_info* split_arg(char** arg, char** value);

void parse_flags(int* argc, char*** argv) {
  char* arg;
  char* cmd;
  char* value;
  struct flag_info* flag;
  int i;

  cmd = (*argv)[0];
  for (i = 1; i < *argc; i++) {
    arg = (*argv)[i];
    /* Must start with a dash and be > 1 char long or we're done with flags. */
    if (arg[0] != '-' || (arg[0] == '-' && arg[1] == '\0')) {
      break;
    }
    /* Allow both -flag and --flag. */
    arg++;
    if (arg[0] == '-') {
      arg++;
    }
    /* "--\0" stops option parsing. */
    if (arg[0] == '\0') {
      i++;
      break;
    }

    /* Work out flag being set and extract value if they're packed together.*/
    flag = split_arg(&arg, &value);
    if (!flag) {
      fprintf(stderr, "unknown command line flag '%s'\n", arg);
      exit(USER_SUCKS);
    }
    /* Get value from next arg if it wasn't packed in with the flag. */
    if (!value) {
      if (++i < *argc) {
        value = (*argv)[i];
      } else {
        fprintf(stderr, "flag '%s' is missing a value\n", arg);
        exit(USER_SUCKS);
      }
    }

    if (!process_flag(flag, value)) {
      exit(USER_SUCKS);
    }
  }

  /* Cut flags out of command line, leaving just program name and real args. */
  *argc -= (i - 1);
  *argv += (i - 1);
  (*argv)[0] = cmd;

  if (FLAGS_help) {
    print_help(cmd);
    exit(NOT_EXPECTED_TO_RUN);
  }
}

void register_flag(struct flag_info* info) {
  info->next = flags_list;
  flags_list = info;
}

struct flag_info* find_flag(char* name) {
  struct flag_info* curr;
  curr = flags_list;
  while (curr) {
    if (strcmp(curr->name, name) == 0) {
      return curr;
    }
    curr = curr->next;
  }
  return NULL;
}

void print_help(char* cmd) {
  struct flag_info* curr;
  char* last_file;

  printf("%s [flags...] [--] [args...]\n", cmd);
  last_file = NULL;
  curr = flags_list;
  while (curr) {
    if (curr->file != last_file) {
      printf("\nFlags for %s:\n", curr->file);
      last_file = curr->file;
    }
    printf("    --%s  %s; default: ", curr->name, curr->help);
    switch (curr->type) {
      case FT_bool:
        printf(*(FS_bool*) curr->default_val ? "true" : "false");
        break;
      case FT_int64:
        printf("%lld", *(FS_int64*) curr->default_val);
        break;
      case FT_uint64:
        printf("%llu (0x%llx)",
            *(FS_uint64*) curr->default_val, *(FS_uint64*) curr->default_val);
        break;
      case FT_string:
        printf("%s", *(FS_string*) curr->default_val);
        break;
    }
    printf("\n");
    curr = curr->next;
  }
}

/* Supported ways to express true and false. */
const char* kTrue[] = { "1", "t", "true", "y", "yes" };
const char* kFalse[] = { "0", "f", "false", "n", "no" };

int process_bool(struct flag_info* flag, char* value) {
  int i;
  for (i = 0; i < sizeof(kTrue) / sizeof(*kTrue); ++i) {
    if (strcasecmp(value, kTrue[i]) == 0) {
      *(FS_bool*) flag->current_val = 1;
      return 1;
    } else if (strcasecmp(value, kFalse[i]) == 0) {
      *(FS_bool*) flag->current_val = 0;
      return 1;
    }
  }
  fprintf(stderr, "Boolean flag '%s' was given non-boolean value '%s'.\n",
      flag->name, value);
  return 0;
}

int process_flag(struct flag_info* flag, char* value) {
  if (flag->type == FT_string) {
    *(FS_string*) flag->current_val = value;
    return 1;
  } else if (*value == '\0') {
    fprintf(stderr, "Empty value for non-string flag '%s'\n", flag->name);
    return 0;
  } else if (flag->type == FT_bool) {
    return process_bool(flag, value);
  } else if (flag->type == FT_int64 || flag->type == FT_uint64) {
    return process_num(flag, value);
  } else {
    fprintf(stderr, "Flag '%s' is of unknown type %s.\n",
        flag->name, flag->type_str);
    return 0;
  }
}

int process_num(struct flag_info* flag, char* value) {
  int base;
  char* end;
  int64_t ival;
  uint64_t uval;

  /* Leading 0x puts us in base 16. But leading 0 does not put us in base 8!
   * It caused too many bugs when gflags had that behavior. */
  base = 10;
  if (value[0] == '0') {
    if (value[1] == 'x' || value[1] == 'X') {
      base = 16;
    } else if (value[1] != '\0') {
      fprintf(stderr,
          "Base 8 value '%s' for flag '%s' is too likely to cause a bug.\n",
          value, flag->name);
      return 0;
    }
  }

  errno = 0;
  if (flag->type == FT_int64) {
    ival = strtoll(value, &end, base);
    *(FS_int64*) flag->current_val = ival;
  } else if (flag->type == FT_uint64) {
    while (*value == ' ') {
      value++;
    }
    if (*value == '-') {
      errno = 1;
    } else {
      uval = strtoull(value, &end, base);
      *(FS_uint64*) flag->current_val = uval;
    }
  }
  if (errno || end != value + strlen(value)) {
    fprintf(stderr, "Flag '%s' expected %s value, got '%s'\n",
       flag->name, flag->type_str, value);
    return 0;
  } else {
    return 1;
  }
}

/*
 * Wraps find_flag. Splits value out from arg in --flag=value, --bool_flag and
 * --nobool_flag cases.
 */
struct flag_info* split_arg(char** arg, char** value) {
  struct flag_info* flag;
  *value = strchr(*arg, '=');
  if (*value) {
    **value = '\0';
    (*value)++;
  }

  flag = find_flag(*arg);
  if (!flag) {
    if (!*value && (*arg)[0] == 'n' && (*arg)[1] == 'o') {
      flag = find_flag(*arg + 2);
    }
    if (flag && flag->type == FT_bool) {
      *arg += 2;
      *value = "0";
    } else {
      flag = NULL;
    }
  } else if (!*value && flag->type == FT_bool) {
    *value = "1";
  }

  return flag;
}
