#include "@/public/input.h"

#include <errno.h>
#include <github.com/apronchenkov/error/public/error.h>
#include <inttypes.h>
#include <stdio.h>

static u7_error read_i32(struct u7_vm0_input* self, int32_t* result) {
  (void)self;
  int ret = scanf("%" SCNd32, result);
  if (1 == ret) {
    return u7_ok();
  } else if (0 == ret) {
    return u7_errnof(EINVAL, "read_i32: incompatible input");
  } else if (errno != 0) {
    return u7_errnof(errno, "read_i32: failed");
  } else if (feof(stdin)) {
    return u7_errnof(ENODATA, "read_i32: eof");
  } else {
    return u7_errnof(EINVAL, "read_i32: unknown error");
  }
}

static u7_error read_i64(struct u7_vm0_input* self, int64_t* result) {
  (void)self;
  int ret = scanf("%" SCNd64, result);
  if (1 == ret) {
    return u7_ok();
  } else if (0 == ret) {
    return u7_errnof(EINVAL, "read_i64: incompatible input");
  } else if (errno != 0) {
    return u7_errnof(errno, "read_i64: failed");
  } else if (feof(stdin)) {
    return u7_errnof(ENODATA, "read_i64: eof");
  } else {
    return u7_errnof(EINVAL, "read_i64: unknown error");
  }
}

static u7_error read_f32(struct u7_vm0_input* self, float* result) {
  (void)self;
  int ret = scanf("%f", result);
  if (1 == ret) {
    return u7_ok();
  } else if (0 == ret) {
    return u7_errnof(EINVAL, "read_f32: incompatible input");
  } else if (errno != 0) {
    return u7_errnof(errno, "read_f32: failed");
  } else if (feof(stdin)) {
    return u7_errnof(ENODATA, "read_f32: eof");
  } else {
    return u7_errnof(EINVAL, "read_f32: unknown error");
  }
}

static u7_error read_f64(struct u7_vm0_input* self, double* result) {
  (void)self;
  int ret = scanf("%lf", result);
  if (1 == ret) {
    return u7_ok();
  } else if (0 == ret) {
    return u7_errnof(EINVAL, "read_f64: incompatible input");
  } else if (errno != 0) {
    return u7_errnof(errno, "read_f64: failed");
  } else if (feof(stdin)) {
    return u7_errnof(ENODATA, "read_f64: eof");
  } else {
    return u7_errnof(EINVAL, "read_f64: unknown error");
  }
}

struct u7_vm0_input u7_vm0_input_scanf = {
    .read_i32_fn = &read_i32,
    .read_i64_fn = &read_i64,
    .read_f32_fn = &read_f32,
    .read_f64_fn = &read_f64,
};
