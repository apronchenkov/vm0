#ifndef U7_VM_PL0_H_
#define U7_VM_PL0_H_

#include "@/public/input.h"
#include "@/public/output.h"

#include <github.com/apronchenkov/error/public/error.h>
#include <github.com/apronchenkov/vm/public/instruction.h>
#include <github.com/apronchenkov/vm/public/state.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Endpoints for interaction with VM.

struct u7_vm0_globals {
  u7_error error;
  struct u7_vm0_input* input;
  struct u7_vm0_output* output;
};

extern struct u7_vm_stack_frame_layout const* const u7_vm0_globals_frame_layout;

// Returns pointer to the global structure.
static inline struct u7_vm0_globals* u7_vm0_state_globals(
    struct u7_vm_state* state) {
  return (struct u7_vm0_globals*)u7_vm_state_globals(state);
}

// Returns pointer to the input interface stored in the global structure.
static inline struct u7_vm0_input* u7_vm0_state_global_input(
    struct u7_vm_state* state) {
  struct u7_vm0_input* result = u7_vm0_state_globals(state)->input;
  assert(result != NULL);
  return result;
}

// Returns pointer to the output interface stored in the global structure.
static inline struct u7_vm0_output* u7_vm0_state_global_output(
    struct u7_vm_state* state) {
  struct u7_vm0_output* result = u7_vm0_state_globals(state)->output;
  assert(result != NULL);
  return result;
}

// Returns pointer to a local variable.
static inline int32_t* u7_vm0_state_local_i32(struct u7_vm_state* state,
                                              int64_t offset) {
  assert(offset % u7_vm_alignof(int32_t) == 0);
  assert(offset >= 0);
  assert(offset + sizeof(int32_t) <=
         u7_vm_stack_current_frame_layout(&state->stack)->locals_size);
  return (int32_t*)u7_vm_memory_add_offset(u7_vm_state_locals(state), offset);
}

static inline int64_t* u7_vm0_state_local_i64(struct u7_vm_state* state,
                                              int64_t offset) {
  assert(offset % u7_vm_alignof(int64_t) == 0);
  assert(offset >= 0);
  assert(offset + sizeof(int64_t) <=
         u7_vm_stack_current_frame_layout(&state->stack)->locals_size);
  return (int64_t*)u7_vm_memory_add_offset(u7_vm_state_locals(state), offset);
}

static inline float* u7_vm0_state_local_f32(struct u7_vm_state* state,
                                            int64_t offset) {
  assert(offset % u7_vm_alignof(float) == 0);
  assert(offset >= 0);
  assert(offset + sizeof(float) <=
         u7_vm_stack_current_frame_layout(&state->stack)->locals_size);
  return (float*)u7_vm_memory_add_offset(u7_vm_state_locals(state), offset);
}

static inline double* u7_vm0_state_local_f64(struct u7_vm_state* state,
                                             int64_t offset) {
  assert(offset % u7_vm_alignof(double) == 0);
  assert(offset >= 0);
  assert(offset + sizeof(double) <=
         u7_vm_stack_current_frame_layout(&state->stack)->locals_size);
  return (double*)u7_vm_memory_add_offset(u7_vm_state_locals(state), offset);
}

// Instructions.

union u7_vm0_value {
  int32_t i32;
  int64_t i64;
  float f32;
  double f64;
};

enum u7_vm0_arg_kind {
  U7_VM0_ARG_KIND_I32_CONSTANT,
  U7_VM0_ARG_KIND_I64_CONSTANT,
  U7_VM0_ARG_KIND_F32_CONSTANT,
  U7_VM0_ARG_KIND_F64_CONSTANT,
  U7_VM0_ARG_KIND_I32_VARIABLE,
  U7_VM0_ARG_KIND_I64_VARIABLE,
  U7_VM0_ARG_KIND_F32_VARIABLE,
  U7_VM0_ARG_KIND_F64_VARIABLE,
};

struct u7_vm0_arg {
  enum u7_vm0_arg_kind kind;
  union u7_vm0_value value;
};

struct u7_vm0_label {
  int64_t offset;
};

struct u7_vm0_instruction {
  struct u7_vm_instruction base;
  union u7_vm0_value arg1;
  union u7_vm0_value arg2;
  union u7_vm0_value arg3;
};

struct u7_vm0_instruction u7_vm0_input(u7_error* error, struct u7_vm0_arg dst);

struct u7_vm0_instruction u7_vm0_output(u7_error* error, struct u7_vm0_arg dst);

struct u7_vm0_instruction u7_vm0_copy(u7_error* error, struct u7_vm0_arg dst,
                                      struct u7_vm0_arg src);

struct u7_vm0_instruction u7_vm0_bitwise_and(u7_error* error,
                                             struct u7_vm0_arg dst,
                                             struct u7_vm0_arg lhs,
                                             struct u7_vm0_arg rhs);

struct u7_vm0_instruction u7_vm0_bitwise_left_shift(u7_error* error,
                                                    struct u7_vm0_arg dst,
                                                    struct u7_vm0_arg lhs,
                                                    struct u7_vm0_arg rhs);

struct u7_vm0_instruction u7_vm0_math_add(u7_error* error,
                                          struct u7_vm0_arg dst,
                                          struct u7_vm0_arg lhs,
                                          struct u7_vm0_arg rhs);

struct u7_vm0_instruction u7_vm0_math_multiply(u7_error* error,
                                               struct u7_vm0_arg dst,
                                               struct u7_vm0_arg lhs,
                                               struct u7_vm0_arg rhs);

struct u7_vm0_instruction u7_vm0_jump_if_zero(u7_error* error,
                                              struct u7_vm0_arg src,
                                              struct u7_vm0_label label);

struct u7_vm0_instruction u7_vm0_jump_if_not_zero(u7_error* error,
                                                  struct u7_vm0_arg src,
                                                  struct u7_vm0_label label);

/*
X read_i64
X write_i64
X store_i64(local_variable, c)
X bitwise_and_i64(local_variable, local_variable, c)
X bitwise_shift_right_i64(local_variable, local_variable, c)
X math_multiply(local_variable, local_variable, local_variable)
X math_add(local_variable, local_variable, local_variable)
X jump_if_zero(local_variable, local_label)
X jump_if_not_zero(local_variable, local_label)
*/

struct u7_vm0_instruction u7_vm0_yield();
struct u7_vm0_instruction u7_vm0_ret();

// Instructions: input.
struct u7_vm0_instruction u7_vm0_read_i32();
struct u7_vm0_instruction u7_vm0_read_i64();
struct u7_vm0_instruction u7_vm0_read_f32();
struct u7_vm0_instruction u7_vm0_read_f64();

struct u7_vm0_instruction u7_vm0_write_i32();
struct u7_vm0_instruction u7_vm0_write_i64();
struct u7_vm0_instruction u7_vm0_write_f32();
struct u7_vm0_instruction u7_vm0_write_f64();

// Store constant.
struct u7_vm0_instruction u7_vm0_push_constant_i32(int32_t value);
struct u7_vm0_instruction u7_vm0_push_constant_i64(int64_t value);
struct u7_vm0_instruction u7_vm0_push_constant_f32(float value);
struct u7_vm0_instruction u7_vm0_push_constant_f64(double value);

// Load constant.
struct u7_vm0_instruction u7_vm0_push_constant_i32(int32_t value);
struct u7_vm0_instruction u7_vm0_push_constant_i64(int64_t value);
struct u7_vm0_instruction u7_vm0_push_constant_f32(float value);
struct u7_vm0_instruction u7_vm0_push_constant_f64(double value);

// Load from the current stack frame locals.
// struct u7_vm0_instruction u7_vm0_load_local_i32(
//     struct u7_vm0_local_variable var);
// struct u7_vm0_instruction u7_vm0_load_local_i64(
//     struct u7_vm0_local_variable var);
// struct u7_vm0_instruction u7_vm0_load_local_f32(
//     struct u7_vm0_local_variable var);
// struct u7_vm0_instruction u7_vm0_load_local_f64(
//     struct u7_vm0_local_variable var);

// // Store to the current frame locals.
// struct u7_vm0_instruction u7_vm0_store_local_i32(
//     struct u7_vm0_local_variable var);
// struct u7_vm0_instruction u7_vm0_store_local_i64(
//     struct u7_vm0_local_variable var);
// struct u7_vm0_instruction u7_vm0_store_local_f32(
//     struct u7_vm0_local_variable var);
// struct u7_vm0_instruction u7_vm0_store_local_f64(
//     struct u7_vm0_local_variable var);

struct u7_vm0_instruction u7_vm0_compare_i32();
struct u7_vm0_instruction u7_vm0_compare_i64();
struct u7_vm0_instruction u7_vm0_compare_f32();
struct u7_vm0_instruction u7_vm0_compare_f64();

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

// struct u7_vm0_instruction u7_vm0_dump_state();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // U7_VM_PL0_H_
