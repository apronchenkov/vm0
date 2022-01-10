#include "@/public/vm0.h"

#include <errno.h>
#include <github.com/apronchenkov/error/public/error.h>
#include <github.com/apronchenkov/vm/public/state.h>
#include <inttypes.h>
#include <stdio.h>

static u7_error read_i32(struct u7_vm0_input* self, int32_t* result) {
  (void)self;
  int ret = scanf("%" SCNd32, result);
  if (1 == ret) {
    return u7_ok();
  } else if (0 == ret) {
    return u7_errnof(EINVAL, "read_i32: incompatible input");
  } else {
    return u7_errnof(errno, "read_i32: failed");
  }
}

static u7_error read_i64(struct u7_vm0_input* self, int64_t* result) {
  (void)self;
  int ret = scanf("%" SCNd64, result);
  if (1 == ret) {
    return u7_ok();
  } else if (0 == ret) {
    return u7_errnof(EINVAL, "read_i64: incompatible input");
  } else {
    return u7_errnof(errno, "read_i64: failed");
  }
}

static u7_error read_f32(struct u7_vm0_input* self, float* result) {
  (void)self;
  int ret = scanf("%f", result);
  if (1 == ret) {
    return u7_ok();
  } else if (0 == ret) {
    return u7_errnof(EINVAL, "read_f32: incompatible input");
  } else {
    return u7_errnof(errno, "read_f32: failed");
  }
}

static u7_error read_f64(struct u7_vm0_input* self, double* result) {
  (void)self;
  int ret = scanf("%lf", result);
  if (1 == ret) {
    return u7_ok();
  } else if (0 == ret) {
    return u7_errnof(EINVAL, "read_f64: incompatible input");
  } else {
    return u7_errnof(errno, "read_f64: failed");
  }
}

static u7_error write_i32(struct u7_vm0_output* self, int32_t value) {
  (void)self;
  if (0 > printf("%" PRId32, value)) {
    return u7_errnof(errno, "write_i32: failed");
  }
  return u7_ok();
}

static u7_error write_i64(struct u7_vm0_output* self, int64_t value) {
  (void)self;
  if (0 > printf("%" PRId64, value)) {
    return u7_errnof(errno, "write_i64: failed");
  }
  return u7_ok();
}

static u7_error write_f32(struct u7_vm0_output* self, float value) {
  (void)self;
  if (0 > printf("%f", value)) {
    return u7_errnof(errno, "write_f32: failed");
  }
  return u7_ok();
}

static u7_error write_f64(struct u7_vm0_output* self, double value) {
  (void)self;
  if (1 != printf("%lf", value)) {
    return u7_errnof(errno, "write_f64: failed");
  }
  return u7_ok();
}

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
      u7_vm0_yield(),
  };

  struct u7_vm_instruction const* js[sizeof(is) / sizeof(is[0])];
  for (size_t i = 0; i < sizeof(is) / sizeof(is[0]); ++i) {
    js[i] = &is[i].base;
  }

  U7_RETURN_IF_ERROR(u7_vm_state_init(&state, u7_vm0_globals_frame_layout,
                                      &js[0], sizeof(js) / sizeof(js[0])));

  struct u7_vm0_input input = {
      .read_i32_fn = &read_i32,
      .read_i64_fn = &read_i64,
      .read_f32_fn = &read_f32,
      .read_f64_fn = &read_f64,
  };
  struct u7_vm0_output output = {
      .write_i32_fn = &write_i32,
      .write_i64_fn = &write_i64,
      .write_f32_fn = &write_f32,
      .write_f64_fn = &write_f64,
  };
  u7_vm0_state_globals(&state)->input = &input;
  u7_vm0_state_globals(&state)->output = &output;
  u7_vm_state_run(&state);
  u7_error result = u7_error_acquire(u7_vm0_state_globals(&state)->error);
  u7_vm_state_destroy(&state);
  return result;
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
