#ifndef U7_VM0_INPUT_H_
#define U7_VM0_INPUT_H_

#include <github.com/apronchenkov/error/public/error.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

struct u7_vm0_input;

typedef u7_error (*u7_vm0_input_read_i32_fn_t)(struct u7_vm0_input* self,
                                               int32_t* result);
typedef u7_error (*u7_vm0_input_read_i64_fn_t)(struct u7_vm0_input* self,
                                               int64_t* result);
typedef u7_error (*u7_vm0_input_read_f32_fn_t)(struct u7_vm0_input* self,
                                               float* result);
typedef u7_error (*u7_vm0_input_read_f64_fn_t)(struct u7_vm0_input* self,
                                               double* result);

struct u7_vm0_input {
  u7_vm0_input_read_i32_fn_t read_i32_fn;
  u7_vm0_input_read_i64_fn_t read_i64_fn;
  u7_vm0_input_read_f32_fn_t read_f32_fn;
  u7_vm0_input_read_f64_fn_t read_f64_fn;
};

extern struct u7_vm0_input u7_vm0_input_scanf;

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // U7_VM0_INPUT_H_
