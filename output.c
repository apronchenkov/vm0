#include "@/public/output.h"

#include <errno.h>
#include <github.com/apronchenkov/error/public/error.h>
#include <inttypes.h>
#include <stdio.h>

static u7_error write_i32(struct u7_vm0_output* self, int32_t value) {
  (void)self;
  if (0 > printf("%" PRId32 " ", value)) {
    return u7_errnof(errno, "write_i32: failed");
  }
  return u7_ok();
}

static u7_error write_i64(struct u7_vm0_output* self, int64_t value) {
  (void)self;
  if (0 > printf("%" PRId64 " ", value)) {
    return u7_errnof(errno, "write_i64: failed");
  }
  return u7_ok();
}

static u7_error write_f32(struct u7_vm0_output* self, float value) {
  (void)self;
  if (0 > printf("%f ", value)) {
    return u7_errnof(errno, "write_f32: failed");
  }
  return u7_ok();
}

static u7_error write_f64(struct u7_vm0_output* self, double value) {
  (void)self;
  if (0 > printf("%lf ", value)) {
    return u7_errnof(errno, "write_f64: failed");
  }
  return u7_ok();
}

struct u7_vm0_output u7_vm0_output_printf = {
    .write_i32_fn = &write_i32,
    .write_i64_fn = &write_i64,
    .write_f32_fn = &write_f32,
    .write_f64_fn = &write_f64,
};
