#include "@/public/vm0.h"

#include <errno.h>
#include <github.com/apronchenkov/error/public/error.h>
#include <github.com/apronchenkov/vm/public/state.h>
#include <inttypes.h>
#include <stdio.h>

u7_error Main() {
  struct u7_vm_state state;

  struct u7_vm0_instruction is[] = {
      u7_vm0_read_f32(),
      u7_vm0_write_f32(),
      /*     u7_vm0_load_constant_i64(INT32_MAX), */
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

  struct u7_vm_instruction const* js[sizeof(is) / sizeof(is[0])];
  for (size_t i = 0; i < sizeof(is) / sizeof(is[0]); ++i) {
    js[i] = &is[i].base;
  }

  U7_RETURN_IF_ERROR(u7_vm_state_init(&state, u7_vm0_globals_frame_layout,
                                      &js[0], sizeof(js) / sizeof(js[0])));

  u7_vm0_state_globals(&state)->input = &u7_vm0_input_scanf;
  u7_vm0_state_globals(&state)->output = &u7_vm0_output_printf;
  u7_error error = u7_ok();
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
