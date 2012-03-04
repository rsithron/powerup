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

#ifndef FLAGS_H_
#define FLAGS_H_

#include <stdint.h>

enum flag_type {
  FT_bool,
  FT_int64,
  FT_uint64,
  FT_string
};
typedef uint8_t FS_bool;
typedef int64_t FS_int64;
typedef uint64_t FS_uint64;
typedef char* FS_string;

#define DECLARE_FLAG(type, name) extern FS_##type FLAGS_##name

#define DECLARE_bool(name) DECLARE_FLAG(bool, name)
#define DECLARE_int64(name) DECLARE_FLAG(int64, name)
#define DECLARE_uint64(name) DECLARE_FLAG(uint64, name)
#define DECLARE_string(name) DECLARE_FLAG(string, name)

struct flag_info;
struct flag_info {
  char* name;
  enum flag_type type;
  char* type_str;
  char* help;
  char* file;
  void* current_val;
  void* default_val;
  struct flag_info* next;
};

/* FLAG_<name> is the actual flag variable. FLAG_no<name> doubles up as storing
 * the default value as well as causing compiler errors if someone tries to
 * define <name> and no<name> flags, which is illegal (--foo and --nofoo both
 * affect the "foo" flag).
 */
#define DEFINE_FLAG(type, name, value, help)   \
  FS_##type FLAGS_##name = value;              \
  const FS_##type FLAGS_no##name = value;      \
  struct flag_info FLAG_INFO_##name = {        \
      #name, FT_##type, #type, help, __FILE__, \
      (void*) &FLAGS_##name,                   \
      (void*) &FLAGS_no##name,                 \
      NULL                                     \
    }

#define DEFINE_bool(name, val, txt) DEFINE_FLAG(bool, name, val, txt)
#define DEFINE_int64(name, val, txt) DEFINE_FLAG(int64, name, val, txt)
#define DEFINE_uint64(name, val, txt) DEFINE_FLAG(uint64, name, val, txt)
#define DEFINE_string(name, val, txt) DEFINE_FLAG(string, name, val, txt)

#define REGISTER(name) register_flag(&FLAG_INFO_##name)

void parse_flags(int* argc, char*** argv);
void register_flag(struct flag_info*);

void fregister_flags();

#endif  /* FLAGS_H_ */
