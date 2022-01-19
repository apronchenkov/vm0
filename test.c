#include "@/public/vm0.h"

#include <errno.h>
#include <github.com/apronchenkov/error/public/error.h>
#include <github.com/apronchenkov/vm/public/state.h>
#include <inttypes.h>
#include <stdio.h>

/* struct Locals { */
/*   int64_t n; */
/*   int64_t t; */
/*   int64_t x[2][2]; */
/*   int64_t m[2][2]; */
/*   int64_t s[8] */
/* }; */

/*
00  read n
01  x00 = 1
02  x01 = 0
03  x10 = 0
04  x11 = 1

05  m00 = 0
06  m01 = 1
07  m10 = 1
08  m11 = 1

loop:

09  t = n & 1
10  jz s00, next
11  s00 = x00 * m00
12  s01 = x01 * m10
13  s02 = x00 * m01
14  s03 = x01 * m11
17  s04 = x10 * m00
18  s05 = x11 * m10
19  s06 = x10 * m01
20  s07 = x11 * m11
15  x00 = s00 + s01
16  x01 = s02 + s03
21  x10 = s04 + s05
22  x11 = s06 + s07
next:
23  s00 = m00 * m00
24  s01 = m01 * m10
25  s02 = m00 * m01
26  s03 = m01 * m11
27  s04 = m10 * m00
28  s05 = m11 * m10
29  s06 = m10 * m01
30  s07 = m11 * m11
31  m00 = s00 + s01
32  m01 = s02 + s03
33  m10 = s04 + s05
34  m11 = s06 + s07
35  n = n >> 1
36  jnz n, loop
37  write x00

*/

/*

[x] read_i64
[x] write_i64
store_i64(local_variable, c)
bitwise_and_i64(local_variable, local_variable, c)
bitwise_shift_right_i64(local_variable, local_variable, c)
jump_if_zero(local_variable, local_label)
jump_if_not_zero(local_variable, local_label)
math_multiply(local_variable, local_variable, local_variable)
math_add(local_variable, local_variable, local_variable)

*/

u7_error Main() {
  struct locals {
    int64_t n;
    int64_t t;
    double x00, x01, x10, x11;
    double m00, m01, m10, m11;
    double s0, s1, s2, s3, s4, s5, s6, s7;
  };
  struct u7_vm0_arg var_n = {
      .kind = U7_VM0_ARG_KIND_I64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, n)},
  };
  struct u7_vm0_arg var_t = {
      .kind = U7_VM0_ARG_KIND_I64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, t)},
  };

  struct u7_vm0_arg var_x00 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, x00)},
  };
  struct u7_vm0_arg var_x01 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, x01)},
  };
  struct u7_vm0_arg var_x10 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, x10)},
  };
  struct u7_vm0_arg var_x11 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, x11)},
  };

  struct u7_vm0_arg var_m00 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, m00)},
  };
  struct u7_vm0_arg var_m01 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, m01)},
  };
  struct u7_vm0_arg var_m10 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, m10)},
  };
  struct u7_vm0_arg var_m11 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, m11)},
  };

  struct u7_vm0_arg var_s0 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, s0)},
  };
  struct u7_vm0_arg var_s1 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, s1)},
  };
  struct u7_vm0_arg var_s2 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, s2)},
  };
  struct u7_vm0_arg var_s3 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, s3)},
  };
  struct u7_vm0_arg var_s4 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, s4)},
  };
  struct u7_vm0_arg var_s5 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, s5)},
  };
  struct u7_vm0_arg var_s6 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, s6)},
  };
  struct u7_vm0_arg var_s7 = {
      .kind = U7_VM0_ARG_KIND_F64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, s7)},
  };

  struct u7_vm0_arg const_f64_0 = {
      .kind = U7_VM0_ARG_KIND_F64_CONSTANT,
      .value = {.f64 = 0.0},
  };
  struct u7_vm0_arg const_f64_1 = {
      .kind = U7_VM0_ARG_KIND_F64_CONSTANT,
      .value = {.f64 = 1.0},
  };
  struct u7_vm0_arg const_i64_1 = {
      .kind = U7_VM0_ARG_KIND_I64_CONSTANT,
      .value = {.i64 = 1},
  };
  struct u7_vm0_arg const_i64_neg_1 = {
      .kind = U7_VM0_ARG_KIND_I64_CONSTANT,
      .value = {.i64 = -1},
  };

  struct u7_vm0_label label_loop = {.offset = 9};
  struct u7_vm0_label label_next = {.offset = 23};

  u7_error error = u7_ok();
  struct u7_vm0_instruction is[] = {
      u7_vm0_input(&error, var_n),  // 00

      u7_vm0_copy(&error, var_x00, const_f64_1),  // 01
      u7_vm0_copy(&error, var_x01, const_f64_0),  // 02
      u7_vm0_copy(&error, var_x10, const_f64_0),  // 03
      u7_vm0_copy(&error, var_x11, const_f64_1),  // 04

      u7_vm0_copy(&error, var_m00, const_f64_0),  // 05
      u7_vm0_copy(&error, var_m01, const_f64_1),  // 06
      u7_vm0_copy(&error, var_m10, const_f64_1),  // 07
      u7_vm0_copy(&error, var_m11, const_f64_1),  // 08

      // loop:
      u7_vm0_bitwise_and(&error, var_t, var_n, const_i64_1),  // 09
      u7_vm0_jump_if_zero(&error, var_t, label_next),         // 10

      u7_vm0_math_multiply(&error, var_s0, var_x00, var_m00),  // 11
      u7_vm0_math_multiply(&error, var_s1, var_x01, var_m10),  // 12
      u7_vm0_math_multiply(&error, var_s2, var_x00, var_m01),  // 13
      u7_vm0_math_multiply(&error, var_s3, var_x01, var_m11),  // 14
      u7_vm0_math_multiply(&error, var_s4, var_x10, var_m00),  // 15
      u7_vm0_math_multiply(&error, var_s5, var_x11, var_m10),  // 16
      u7_vm0_math_multiply(&error, var_s6, var_x10, var_m01),  // 17
      u7_vm0_math_multiply(&error, var_s7, var_x11, var_m11),  // 18
      u7_vm0_math_add(&error, var_x00, var_s0, var_s1),        // 19
      u7_vm0_math_add(&error, var_x01, var_s2, var_s3),        // 20
      u7_vm0_math_add(&error, var_x10, var_s4, var_s5),        // 21
      u7_vm0_math_add(&error, var_x11, var_s6, var_s7),        // 22

      // next:
      u7_vm0_math_multiply(&error, var_s0, var_m00, var_m00),  // 23
      u7_vm0_math_multiply(&error, var_s1, var_m01, var_m10),  // 24
      u7_vm0_math_multiply(&error, var_s2, var_m00, var_m01),  // 25
      u7_vm0_math_multiply(&error, var_s3, var_m01, var_m11),  // 26
      u7_vm0_math_multiply(&error, var_s4, var_m10, var_m00),  // 27
      u7_vm0_math_multiply(&error, var_s5, var_m11, var_m10),  // 28
      u7_vm0_math_multiply(&error, var_s6, var_m10, var_m01),  // 29
      u7_vm0_math_multiply(&error, var_s7, var_m11, var_m11),  // 30
      u7_vm0_math_add(&error, var_m00, var_s0, var_s1),        // 31
      u7_vm0_math_add(&error, var_m01, var_s2, var_s3),        // 32
      u7_vm0_math_add(&error, var_m10, var_s4, var_s5),        // 33
      u7_vm0_math_add(&error, var_m11, var_s6, var_s7),        // 34

      u7_vm0_bitwise_left_shift(&error, var_n, var_n, const_i64_neg_1),  // 35
      u7_vm0_jump_if_not_zero(&error, var_n, label_loop),                // 36
      u7_vm0_output(&error, var_x01),                                    // 37
      u7_vm0_ret(),                                                      // 38
  };
  if (error.error_code != 0) {
    return error;
  }

  const size_t isn = sizeof(is) / sizeof(is[0]);
  struct u7_vm_instruction const* js[sizeof(is) / sizeof(is[0])];
  for (size_t i = 0; i < sizeof(is) / sizeof(is[0]); ++i) {
    js[i] = &is[i].base;
  }

  struct u7_vm_state state;
  error = u7_vm_state_init(&state, u7_vm0_globals_frame_layout, &js[0], isn);
  if (error.error_code != 0) {
    return error;
  }

  static struct u7_vm_stack_frame_layout local_frame_layout = {
      .locals_size = sizeof(struct locals),
      .description = "locals",
  };
  error = u7_vm_stack_push_frame(&state.stack, &local_frame_layout);
  if (error.error_code != 0) {
    return error;
  }

  u7_vm0_state_globals(&state)->input = &u7_vm0_input_scanf;
  u7_vm0_state_globals(&state)->output = &u7_vm0_output_printf;

  while (error.error_code == 0) {
    u7_vm_state_run(&state);
    error = u7_error_move(&u7_vm0_state_globals(&state)->error);
  }
  u7_vm_state_destroy(&state);
  return error;
}

int main() {
  u7_error error = Main();
  if (error.error_code) {
    fprintf(stderr, "Main: %" U7_ERROR_FMT "\n", U7_ERROR_FMT_PARAMS(error));
    u7_error_release(error);
    return -1;
  }
  return 0;
}
