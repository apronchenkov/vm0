#include "@/public/vm0.h"

#include <errno.h>
#include <github.com/apronchenkov/error/public/error.h>
#include <github.com/apronchenkov/vm/public/state.h>
#include <inttypes.h>
#include <stdio.h>

/* struct Locals { */
/*   int64_t n; */
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
09  s00 = n & 1
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
27  s03 = m01 * m11
28  s04 = m10 * m00
29  s05 = m11 * m10
30  s06 = m10 * m01
31  s07 = m11 * m11
32  m00 = s00 + s01
33  m01 = s02 + s03
34  m10 = s04 + s05
35  m11 = s06 + s07
36  s00 = n >> 1
37  jnz s00, loop
38  write x00

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
    int64_t s;
    int64_t t;
  };
  struct u7_vm0_arg n_var = {
      .kind = U7_VM0_ARG_KIND_I64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, n)},
  };
  struct u7_vm0_arg s_var = {
      .kind = U7_VM0_ARG_KIND_I64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, s)},
  };
  struct u7_vm0_arg t_var = {
      .kind = U7_VM0_ARG_KIND_I64_VARIABLE,
      .value = {.i64 = u7_vm_offsetof(struct locals, t)},
  };
  /* struct u7_vm0_arg i64_1 = { */
  /*     .kind = U7_VM0_ARG_KIND_I64_CONSTANT, */
  /*     .value = {.i64 = 1}, */
  /* }; */

  u7_error error = u7_ok();
  struct u7_vm0_instruction is[] = {
      u7_vm0_input(&error, n_var),
      u7_vm0_input(&error, s_var),
      u7_vm0_math_multiply(&error, t_var, n_var, s_var),
      u7_vm0_output(&error, t_var),
      /* u7_vm0_load_constant_i64(INT32_MAX), */
      /* u7_vm0_write_i64(), */
      /* u7_vm0_read_f32(), */
      /* u7_vm0_write_f32(), */
      /*     u7_vm0_load_constant_i64(-1), */
      /*     u7_vm0_multiply_i64(), */
      /*     u7_vm0_duplicate_i64(), */
      /*     u7_vm0_print_i64(), */
      /*     u7_vm0_println(), */
      /*     u7_vm0_cast_i64_to_f32(), */
      /*     u7_vm0_print_f32(), */
      /*     u7_vm0_println(), */
      u7_vm0_ret(),
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
