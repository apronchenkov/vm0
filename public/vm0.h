#ifndef U7_VM_PL0_H_
#define U7_VM_PL0_H_

#include <github.com/apronchenkov/error/public/error.h>
#include <github.com/apronchenkov/vm/public/instruction.h>
#include <github.com/apronchenkov/vm/public/state.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Input facility.

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

// Output facility.

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

// Endpoints for interaction with VM.

struct u7_vm0_globals {
  u7_error error;
  struct u7_vm0_input* input;
  struct u7_vm0_output* output;
};

extern struct u7_vm_stack_frame_layout const* const u7_vm0_globals_frame_layout;

static inline struct u7_vm0_globals* u7_vm0_state_globals(
    struct u7_vm_state* state) {
  return (struct u7_vm0_globals*)u7_vm_state_globals(state);
}

// Instructions.

union u7_vm0_value {
  int32_t i32;
  int64_t i64;
  float f32;
  double f64;
};

struct u7_vm0_local_variable {
  int64_t offset;  // non-negative
};

struct u7_vm0_local_label {
  int64_t offset;
};

struct u7_vm0_instruction {
  struct u7_vm_instruction base;
  union u7_vm0_value arg1;
  union u7_vm0_value arg2;
};

// Instructions: input.
struct u7_vm0_instruction u7_vm0_read_f32();
struct u7_vm0_instruction u7_vm0_read_f64();

struct u7_vm0_instruction u7_vm0_write_f32();

// struct u7_vm0_instruction u7_vm0_load_constant_i32(int32_t value);
// struct u7_vm0_instruction u7_vm0_load_constant_i64(int64_t value);
// struct u7_vm0_instruction u7_vm0_load_constant_f32(float value);
// struct u7_vm0_instruction u7_vm0_load_constant_f64(double value);

// struct u7_vm0_instruction u7_vm0_load_local_i32(
//     struct u7_vm0_local_variable var);
// struct u7_vm0_instruction u7_vm0_load_local_i64(
//     struct u7_vm0_local_variable var);
// struct u7_vm0_instruction u7_vm0_load_local_f32(
//     struct u7_vm0_local_variable var);
// struct u7_vm0_instruction u7_vm0_load_local_f64(
//     struct u7_vm0_local_variable var);

// struct u7_vm0_instruction u7_vm0_store_local_i32(
//     struct u7_vm0_local_variable var);
// struct u7_vm0_instruction u7_vm0_store_local_i64(
//     struct u7_vm0_local_variable var);
// struct u7_vm0_instruction u7_vm0_store_local_f32(
//     struct u7_vm0_local_variable var);
// struct u7_vm0_instruction u7_vm0_store_local_f64(
//     struct u7_vm0_local_variable var);

// struct u7_vm0_instruction u7_vm0_compare_i32();
// struct u7_vm0_instruction u7_vm0_compare_i64();
// struct u7_vm0_instruction u7_vm0_compare_f32();
// struct u7_vm0_instruction u7_vm0_compare_f64();

// struct u7_vm0_instruction u7_vm0_jump_if_i32_zero(
//     struct u7_vm0_local_label label);
// struct u7_vm0_instruction u7_vm0_jump_if_i32_not_zero(
//     struct u7_vm0_local_label label);
// struct u7_vm0_instruction u7_vm0_jump_if_i32_negative(
//     struct u7_vm0_local_label label);
// struct u7_vm0_instruction u7_vm0_jump_if_i32_positive(
//     struct u7_vm0_local_label label);
// struct u7_vm0_instruction u7_vm0_jump_if_i32_negative_or_zero(
//     struct u7_vm0_local_label label);
// struct u7_vm0_instruction u7_vm0_jump_if_i32_positive_or_zero(
//     struct u7_vm0_local_label label);

// struct u7_vm0_instruction u7_vm0_jump_if_i64_zero(
//     struct u7_vm0_local_label label);
// struct u7_vm0_instruction u7_vm0_jump_if_i64_not_zero(
//     struct u7_vm0_local_label label);
// struct u7_vm0_instruction u7_vm0_jump_if_i64_negative(
//     struct u7_vm0_local_label label);
// struct u7_vm0_instruction u7_vm0_jump_if_i64_positive(
//     struct u7_vm0_local_label label);
// struct u7_vm0_instruction u7_vm0_jump_if_i64_negative_or_zero(
//     struct u7_vm0_local_label label);
// struct u7_vm0_instruction u7_vm0_jump_if_i64_positive_or_zero(
//     struct u7_vm0_local_label label);

// struct u7_vm0_instruction u7_vm0_jump(struct u7_vm0_local_label label);

// struct u7_vm0_instruction u7_vm0_duplicate_i32();
// struct u7_vm0_instruction u7_vm0_duplicate_i64();
// struct u7_vm0_instruction u7_vm0_duplicate_f32();
// struct u7_vm0_instruction u7_vm0_duplicate_f64();

// struct u7_vm0_instruction u7_vm0_bitwise_not_i32();
// struct u7_vm0_instruction u7_vm0_bitwise_not_i64();
// struct u7_vm0_instruction u7_vm0_bitwise_and_i32();
// struct u7_vm0_instruction u7_vm0_bitwise_and_i64();
// struct u7_vm0_instruction u7_vm0_bitwise_or_i32();
// struct u7_vm0_instruction u7_vm0_bitwise_or_i64();
// struct u7_vm0_instruction u7_vm0_bitwise_xor_i32();
// struct u7_vm0_instruction u7_vm0_bitwise_xor_i64();

// struct u7_vm0_instruction u7_vm0_abs_i32();
// struct u7_vm0_instruction u7_vm0_abs_i64();
// struct u7_vm0_instruction u7_vm0_abs_f32();
// struct u7_vm0_instruction u7_vm0_abs_f64();

// struct u7_vm0_instruction u7_vm0_neg_i32();
// struct u7_vm0_instruction u7_vm0_neg_i64();
// struct u7_vm0_instruction u7_vm0_neg_f32();
// struct u7_vm0_instruction u7_vm0_neg_f64();

// struct u7_vm0_instruction u7_vm0_inc_i32(int32_t delta);
// struct u7_vm0_instruction u7_vm0_inc_i64(int64_t delta);
// struct u7_vm0_instruction u7_vm0_inc_f32(float delta);
// struct u7_vm0_instruction u7_vm0_inc_f64(double delta);

// struct u7_vm0_instruction u7_vm0_inc_local_i64(struct u7_vm0_local_variable
// var,
//                                                int64_t delta);

// struct u7_vm0_instruction u7_vm0_add_i32();
// struct u7_vm0_instruction u7_vm0_add_i64();
// struct u7_vm0_instruction u7_vm0_add_f32();
// struct u7_vm0_instruction u7_vm0_add_f64();

// struct u7_vm0_instruction u7_vm0_subtract_i32();
// struct u7_vm0_instruction u7_vm0_subtract_i64();
// struct u7_vm0_instruction u7_vm0_subtract_f32();
// struct u7_vm0_instruction u7_vm0_subtract_f64();

// struct u7_vm0_instruction u7_vm0_multiply_i32();
// struct u7_vm0_instruction u7_vm0_multiply_i64();
// struct u7_vm0_instruction u7_vm0_multiply_f32();
// struct u7_vm0_instruction u7_vm0_multiply_f64();

// struct u7_vm0_instruction u7_vm0_divide_f32();
// struct u7_vm0_instruction u7_vm0_divide_f64();

// struct u7_vm0_instruction u7_vm0_floordiv_u32();
// struct u7_vm0_instruction u7_vm0_floordiv_u64();

// struct u7_vm0_instruction u7_vm0_floormod_u32();
// struct u7_vm0_instruction u7_vm0_floormod_u64();

// struct u7_vm0_instruction u7_vm0_floormod_local_u64(
//     struct u7_vm0_local_variable lhs, struct u7_vm0_local_variable rhs);

// struct u7_vm0_instruction u7_vm0_floor_f32();
// struct u7_vm0_instruction u7_vm0_floor_f64();
// struct u7_vm0_instruction u7_vm0_ceil_f32();
// struct u7_vm0_instruction u7_vm0_ceil_f64();
// struct u7_vm0_instruction u7_vm0_round_f32();
// struct u7_vm0_instruction u7_vm0_round_f64();
// struct u7_vm0_instruction u7_vm0_trunc_f32();
// struct u7_vm0_instruction u7_vm0_trunc_f64();

// struct u7_vm0_instruction u7_vm0_sqrt_f32();
// struct u7_vm0_instruction u7_vm0_sqrt_f64();

// struct u7_vm0_instruction u7_vm0_cast_i32_to_i64();
// struct u7_vm0_instruction u7_vm0_cast_i32_to_f32();
// struct u7_vm0_instruction u7_vm0_cast_i32_to_f64();
// struct u7_vm0_instruction u7_vm0_cast_i64_to_i32();
// struct u7_vm0_instruction u7_vm0_cast_i64_to_f32();
// struct u7_vm0_instruction u7_vm0_cast_i64_to_f64();
// struct u7_vm0_instruction u7_vm0_cast_f32_to_f64();
// struct u7_vm0_instruction u7_vm0_cast_f64_to_f32();

// struct u7_vm0_instruction u7_vm0_trunc_f32_to_i32();
// struct u7_vm0_instruction u7_vm0_trunc_f32_to_i64();
// struct u7_vm0_instruction u7_vm0_trunc_f64_to_i32();
// struct u7_vm0_instruction u7_vm0_trunc_f64_to_i64();

// struct u7_vm0_instruction u7_vm0_print_i32();
// struct u7_vm0_instruction u7_vm0_print_i64();
// struct u7_vm0_instruction u7_vm0_print_f32();
// struct u7_vm0_instruction u7_vm0_print_f64();
// struct u7_vm0_instruction u7_vm0_println();

struct u7_vm0_instruction u7_vm0_yield();

// struct u7_vm0_instruction u7_vm0_dump_state();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // U7_VM_PL0_H_
