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
  U7_VM0_ARG_KIND_I64_LABEL,
};

struct u7_vm0_arg {
  enum u7_vm0_arg_kind kind;
  union u7_vm0_value value;
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
                                              struct u7_vm0_arg label);

struct u7_vm0_instruction u7_vm0_jump_if_not_zero(u7_error* error,
                                                  struct u7_vm0_arg src,
                                                  struct u7_vm0_arg label);

struct u7_vm0_instruction u7_vm0_yield();
struct u7_vm0_instruction u7_vm0_ret();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // U7_VM_PL0_H_
