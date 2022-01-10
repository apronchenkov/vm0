#ifndef U7_VM0_OUTPUT_H_
#define U7_VM0_OUTPUT_H_

#include <github.com/apronchenkov/error/public/error.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

struct u7_vm0_output;

typedef u7_error (*u7_vm0_output_write_i32_fn_t)(struct u7_vm0_output* self,
                                                 int32_t value);
typedef u7_error (*u7_vm0_output_write_i64_fn_t)(struct u7_vm0_output* self,
                                                 int64_t value);
typedef u7_error (*u7_vm0_output_write_f32_fn_t)(struct u7_vm0_output* self,
                                                 float value);
typedef u7_error (*u7_vm0_output_write_f64_fn_t)(struct u7_vm0_output* self,
                                                 double value);

struct u7_vm0_output {
  u7_vm0_output_write_i32_fn_t write_i32_fn;
  u7_vm0_output_write_i64_fn_t write_i64_fn;
  u7_vm0_output_write_f32_fn_t write_f32_fn;
  u7_vm0_output_write_f64_fn_t write_f64_fn;
};

extern struct u7_vm0_output u7_vm0_output_printf;

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // U7_VM0_OUTPUT_H_
