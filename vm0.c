#include "@/public/vm0.h"

#include <errno.h>
#include <github.com/apronchenkov/vm/public/stack_push_pop.h>
#include <github.com/apronchenkov/vm/public/state.h>
#include <github.com/apronchenkov/yalog/public/logging_printf.h>
#include <inttypes.h>
#include <math.h>

static void u7_vm0_stack_frame_layout_deinit(
    struct u7_vm_stack_frame_layout const* self, void* memory) {
  (void)self;
  u7_error_clear(&((struct u7_vm0_globals*)memory)->error);
}

static struct u7_vm_stack_frame_layout u7_vm0_globals_frame_layout_impl = {
    .locals_size = u7_vm_align_size(sizeof(struct u7_vm0_globals),
                                    U7_VM_DEFAULT_ALIGNMENT),
    .deinit_fn = &u7_vm0_stack_frame_layout_deinit,
    .extra_capacity = 0,
    .description = "u7_vm0_globals",
};

struct u7_vm_stack_frame_layout const* const u7_vm0_globals_frame_layout =
    &u7_vm0_globals_frame_layout_impl;

__attribute__((noinline)) static bool u7_vm0_panic(struct u7_vm_state* state,
                                                   u7_error err) {
  assert(err.error_code != 0);
  struct u7_vm0_globals* globals = u7_vm0_state_globals(state);
  assert(globals->error.error_code == 0);
  if (globals->error.error_code != 0) {
    u7_error_clear(&globals->error);
  }
  globals->error = err;
  state->ip = 0;  // Reset to the beginning.
  assert(state->stack.top_offset >=
         state->stack.base_offset + U7_VM_STACK_FRAME_HEADER_SIZE +
             u7_vm_stack_current_frame_layout(&state->stack)->locals_size);
  state->stack.top_offset =
      state->stack.base_offset +       // Assume, that the stack values
      U7_VM_STACK_FRAME_HEADER_SIZE +  // need no destruction work.
      u7_vm_stack_current_frame_layout(&state->stack)->locals_size;
  return false;
}

__attribute__((noinline)) static u7_error u7_vm0_unsupported_arg_kind_error(
    const char* instruction_name, const char* arg_name,
    enum u7_vm0_arg_kind arg_kind) {
  switch (arg_kind) {
    case U7_VM0_ARG_KIND_I32_CONSTANT:
      return u7_errnof(EINVAL,
                       "%s: incompatible argument kind: %s: int32 constant",
                       instruction_name, arg_name);
    case U7_VM0_ARG_KIND_I64_CONSTANT:
      return u7_errnof(EINVAL,
                       "%s: incompatible argument kind: %s: int64 constant",
                       instruction_name, arg_name);
    case U7_VM0_ARG_KIND_F32_CONSTANT:
      return u7_errnof(EINVAL,
                       "%s: incompatible argument kind: %s: float32 constant",
                       instruction_name, arg_name);
    case U7_VM0_ARG_KIND_F64_CONSTANT:
      return u7_errnof(EINVAL,
                       "%s: incompatible argument kind: %s: float64 constant",
                       instruction_name, arg_name);
    case U7_VM0_ARG_KIND_I32_VARIABLE:
      return u7_errnof(EINVAL,
                       "%s: incompatible argument kind: %s: int32 variable",
                       instruction_name, arg_name);
    case U7_VM0_ARG_KIND_I64_VARIABLE:
      return u7_errnof(EINVAL,
                       "%s: incompatible argument kind: %s: int64 variable",
                       instruction_name, arg_name);
    case U7_VM0_ARG_KIND_F32_VARIABLE:
      return u7_errnof(EINVAL,
                       "%s: incompatible argument kind: %s: float32 variable",
                       instruction_name, arg_name);
    case U7_VM0_ARG_KIND_F64_VARIABLE:
      return u7_errnof(EINVAL,
                       "%s: incompatible argument kind: %s: float64 variable",
                       instruction_name, arg_name);
  }
  return u7_errnof(EINVAL,
                   "%s: incompatible argument kind: %s: unknown arg kind (%d)",
                   instruction_name, arg_name, (int)arg_kind);
}

#define U7_VM0_DEFINE_INSTRUCTION_EXEC(fn_name) \
  U7_VM_DEFINE_INSTRUCTION_EXEC(fn_name##_exec, struct u7_vm0_instruction)

#define U7_VM0_DEFINE_INSTRUCTION_0(fn_name)     \
  struct u7_vm0_instruction u7_vm0_##fn_name() { \
    struct u7_vm0_instruction result = {         \
        .base = {.execute_fn = fn_name##_exec}}; \
    return result;                               \
  }

U7_VM0_DEFINE_INSTRUCTION_EXEC(yield) { return false; }

U7_VM0_DEFINE_INSTRUCTION_0(yield)

U7_VM0_DEFINE_INSTRUCTION_EXEC(ret) {
  state->ip = 0;  // Reset to the beginning.
  assert(state->stack.top_offset ==
         state->stack.base_offset + U7_VM_STACK_FRAME_HEADER_SIZE +
             u7_vm_stack_current_frame_layout(&state->stack)->locals_size);
  return false;
}

U7_VM0_DEFINE_INSTRUCTION_0(ret)

U7_VM0_DEFINE_INSTRUCTION_EXEC(input_i32v) {
  struct u7_vm0_input* input = u7_vm0_state_global_input(state);
  u7_error error =
      input->read_i32_fn(input, u7_vm0_state_local_i32(state, self->arg1.i64));
  if (error.error_code != 0) {
    return u7_vm0_panic(state, error);
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(input_i64v) {
  struct u7_vm0_input* input = u7_vm0_state_global_input(state);
  u7_error error =
      input->read_i64_fn(input, u7_vm0_state_local_i64(state, self->arg1.i64));
  if (error.error_code != 0) {
    return u7_vm0_panic(state, error);
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(input_f32v) {
  struct u7_vm0_input* input = u7_vm0_state_global_input(state);
  u7_error err =
      input->read_f32_fn(input, u7_vm0_state_local_f32(state, self->arg1.i64));
  if (err.error_code != 0) {
    return u7_vm0_panic(state, err);
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(input_f64v) {
  struct u7_vm0_input* input = u7_vm0_state_global_input(state);
  u7_error err =
      input->read_f64_fn(input, u7_vm0_state_local_f64(state, self->arg1.i64));
  if (err.error_code != 0) {
    return u7_vm0_panic(state, err);
  }
  return true;
}

struct u7_vm0_instruction u7_vm0_input(u7_error* error, struct u7_vm0_arg dst) {
  struct u7_vm0_instruction result = {0};
  if (error->error_code != 0) {
    return result;
  }
  result.arg1 = dst.value;
  switch (dst.kind) {
    case U7_VM0_ARG_KIND_I32_VARIABLE:
      result.base.execute_fn = input_i32v_exec;
      break;
    case U7_VM0_ARG_KIND_I64_VARIABLE:
      result.base.execute_fn = input_i64v_exec;
      break;
    case U7_VM0_ARG_KIND_F32_VARIABLE:
      result.base.execute_fn = input_f32v_exec;
      break;
    case U7_VM0_ARG_KIND_F64_VARIABLE:
      result.base.execute_fn = input_f64v_exec;
      break;
    default:
      *error =
          u7_vm0_unsupported_arg_kind_error("u7_vm0_input", "dst", dst.kind);
  }
  return result;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(output_i32c) {
  struct u7_vm0_output* output = u7_vm0_state_global_output(state);
  u7_error error = output->write_i32_fn(output, self->arg1.i32);
  if (error.error_code != 0) {
    return u7_vm0_panic(state, error);
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(output_i64c) {
  struct u7_vm0_output* output = u7_vm0_state_global_output(state);
  u7_error error = output->write_i64_fn(output, self->arg1.i64);
  if (error.error_code != 0) {
    return u7_vm0_panic(state, error);
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(output_f32c) {
  struct u7_vm0_output* output = u7_vm0_state_global_output(state);
  u7_error error = output->write_f32_fn(output, self->arg1.f32);
  if (error.error_code != 0) {
    return u7_vm0_panic(state, error);
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(output_f64c) {
  struct u7_vm0_output* output = u7_vm0_state_global_output(state);
  u7_error error = output->write_f64_fn(output, self->arg1.f64);
  if (error.error_code != 0) {
    return u7_vm0_panic(state, error);
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(output_i32v) {
  struct u7_vm0_output* output = u7_vm0_state_global_output(state);
  u7_error error = output->write_i32_fn(
      output, *u7_vm0_state_local_i32(state, self->arg1.i64));
  if (error.error_code != 0) {
    return u7_vm0_panic(state, error);
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(output_i64v) {
  struct u7_vm0_output* output = u7_vm0_state_global_output(state);
  u7_error error = output->write_i64_fn(
      output, *u7_vm0_state_local_i64(state, self->arg1.i64));
  if (error.error_code != 0) {
    return u7_vm0_panic(state, error);
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(output_f32v) {
  struct u7_vm0_output* output = u7_vm0_state_global_output(state);
  u7_error error = output->write_f32_fn(
      output, *u7_vm0_state_local_f32(state, self->arg1.i64));
  if (error.error_code != 0) {
    return u7_vm0_panic(state, error);
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(output_f64v) {
  struct u7_vm0_output* output = u7_vm0_state_global_output(state);
  u7_error error = output->write_f64_fn(
      output, *u7_vm0_state_local_f64(state, self->arg1.i64));
  if (error.error_code != 0) {
    return u7_vm0_panic(state, error);
  }
  return true;
}

struct u7_vm0_instruction u7_vm0_output(u7_error* error,
                                        struct u7_vm0_arg src) {
  struct u7_vm0_instruction result = {0};
  if (error->error_code != 0) {
    return result;
  }
  result.arg1 = src.value;
  switch (src.kind) {
    case U7_VM0_ARG_KIND_I32_CONSTANT:
      result.base.execute_fn = output_i32c_exec;
      break;
    case U7_VM0_ARG_KIND_I64_CONSTANT:
      result.base.execute_fn = output_i64c_exec;
      break;
    case U7_VM0_ARG_KIND_F32_CONSTANT:
      result.base.execute_fn = output_f32c_exec;
      break;
    case U7_VM0_ARG_KIND_F64_CONSTANT:
      result.base.execute_fn = output_f64c_exec;
      break;
    case U7_VM0_ARG_KIND_I32_VARIABLE:
      result.base.execute_fn = output_i32v_exec;
      break;
    case U7_VM0_ARG_KIND_I64_VARIABLE:
      result.base.execute_fn = output_i64v_exec;
      break;
    case U7_VM0_ARG_KIND_F32_VARIABLE:
      result.base.execute_fn = output_f32v_exec;
      break;
    case U7_VM0_ARG_KIND_F64_VARIABLE:
      result.base.execute_fn = output_f64v_exec;
      break;
    default:
      *error =
          u7_vm0_unsupported_arg_kind_error("u7_vm0_output", "src", src.kind);
  }
  return result;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(copy_i32c) {
  *u7_vm0_state_local_i32(state, self->arg1.i64) = self->arg2.i32;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(copy_i64c) {
  *u7_vm0_state_local_i64(state, self->arg1.i64) = self->arg2.i64;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(copy_f32c) {
  *u7_vm0_state_local_f32(state, self->arg1.i64) = self->arg2.f32;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(copy_f64c) {
  *u7_vm0_state_local_f64(state, self->arg1.i64) = self->arg2.f64;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(copy_i32v) {
  *u7_vm0_state_local_i32(state, self->arg1.i64) =
      *u7_vm0_state_local_i32(state, self->arg2.i64);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(copy_i64v) {
  *u7_vm0_state_local_i64(state, self->arg1.i64) =
      *u7_vm0_state_local_i64(state, self->arg2.i64);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(copy_f32v) {
  *u7_vm0_state_local_f32(state, self->arg1.i64) =
      *u7_vm0_state_local_f32(state, self->arg2.i64);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(copy_f64v) {
  *u7_vm0_state_local_f64(state, self->arg1.i64) =
      *u7_vm0_state_local_f64(state, self->arg2.i64);
  return true;
}

struct u7_vm0_instruction u7_vm0_copy(u7_error* error, struct u7_vm0_arg dst,
                                      struct u7_vm0_arg src) {
  struct u7_vm0_instruction result = {0};
  if (error->error_code != 0) {
    return result;
  }
  result.arg1 = dst.value;
  result.arg2 = src.value;
  switch (src.kind) {
    case U7_VM0_ARG_KIND_I32_CONSTANT:
      if (dst.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
        result.base.execute_fn = copy_i32c_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_copy_i32", "dst",
                                                   dst.kind);
      }
      break;
    case U7_VM0_ARG_KIND_I64_CONSTANT:
      if (dst.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
        result.base.execute_fn = copy_i64c_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_copy_i64", "dst",
                                                   dst.kind);
      }
      break;
    case U7_VM0_ARG_KIND_F32_CONSTANT:
      if (dst.kind == U7_VM0_ARG_KIND_F32_VARIABLE) {
        result.base.execute_fn = copy_f32c_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_copy_f32", "dst",
                                                   dst.kind);
      }
      break;
    case U7_VM0_ARG_KIND_F64_CONSTANT:
      if (dst.kind == U7_VM0_ARG_KIND_F64_VARIABLE) {
        result.base.execute_fn = copy_f64c_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_copy_f64", "dst",
                                                   dst.kind);
      }
      break;
    case U7_VM0_ARG_KIND_I32_VARIABLE:
      if (dst.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
        result.base.execute_fn = copy_i32v_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_copy_i32", "dst",
                                                   dst.kind);
      }
      break;
    case U7_VM0_ARG_KIND_I64_VARIABLE:
      if (dst.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
        result.base.execute_fn = copy_i64v_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_copy_i64", "dst",
                                                   dst.kind);
      }
      break;
    case U7_VM0_ARG_KIND_F32_VARIABLE:
      if (dst.kind == U7_VM0_ARG_KIND_F32_VARIABLE) {
        result.base.execute_fn = copy_f32v_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_copy_f32", "dst",
                                                   dst.kind);
      }
      break;
    case U7_VM0_ARG_KIND_F64_VARIABLE:
      if (dst.kind == U7_VM0_ARG_KIND_F64_VARIABLE) {
        result.base.execute_fn = copy_f64v_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_copy_f64", "dst",
                                                   dst.kind);
      }
      break;
    default:
      *error =
          u7_vm0_unsupported_arg_kind_error("u7_vm0_copy", "src", src.kind);
  }
  return result;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_and_i32vc) {
  *u7_vm0_state_local_i32(state, self->arg1.i64) =
      *u7_vm0_state_local_i32(state, self->arg2.i64) & self->arg3.i32;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_and_i64vc) {
  *u7_vm0_state_local_i64(state, self->arg1.i64) =
      *u7_vm0_state_local_i64(state, self->arg2.i64) & self->arg3.i64;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_and_i32vv) {
  *u7_vm0_state_local_i32(state, self->arg1.i64) =
      *u7_vm0_state_local_i32(state, self->arg2.i64) &
      *u7_vm0_state_local_i32(state, self->arg3.i64);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_and_i64vv) {
  *u7_vm0_state_local_i64(state, self->arg1.i64) =
      *u7_vm0_state_local_i64(state, self->arg2.i64) &
      *u7_vm0_state_local_i64(state, self->arg3.i64);
  return true;
}

struct u7_vm0_instruction u7_vm0_bitwise_and(u7_error* error,
                                             struct u7_vm0_arg dst,
                                             struct u7_vm0_arg lhs,
                                             struct u7_vm0_arg rhs) {
  struct u7_vm0_instruction result = {0};
  if (error->error_code != 0) {
    return result;
  }
  result.arg1 = dst.value;
  result.arg2 = lhs.value;
  result.arg3 = rhs.value;
  if (dst.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
    if (lhs.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
      if (rhs.kind == U7_VM0_ARG_KIND_I32_CONSTANT) {
        result.base.execute_fn = bitwise_and_i32vc_exec;
      } else if (rhs.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
        result.base.execute_fn = bitwise_and_i32vv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_bitwise_and_i32",
                                                   "rhs", rhs.kind);
      }
    } else {
      *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_bitwise_and_i32",
                                                 "lhs", lhs.kind);
    }
  } else if (dst.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
    if (lhs.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
      if (rhs.kind == U7_VM0_ARG_KIND_I64_CONSTANT) {
        result.base.execute_fn = bitwise_and_i64vc_exec;
      } else if (rhs.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
        result.base.execute_fn = bitwise_and_i64vv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_bitwise_and_i64",
                                                   "rhs", rhs.kind);
      }
    } else {
      *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_bitwise_and_i64",
                                                 "lhs", lhs.kind);
    }
  } else {
    *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_bitwise_and", "dst",
                                               dst.kind);
  }
  return result;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_left_shift_i32cv) {
  int32_t* const dst = u7_vm0_state_local_i32(state, self->arg1.i64);
  const int32_t lhs = self->arg2.i32;
  const int32_t rhs = *u7_vm0_state_local_i32(state, self->arg3.i64);
  if (rhs < -31) {
    return u7_vm0_panic(
        state, u7_errnof(EINVAL, "bitwise_left_shift_i32: rhs=%" PRId32, rhs));
  } else if (rhs > 31) {
    return u7_vm0_panic(
        state, u7_errnof(EINVAL, "bitwise_left_shift_i32: rhs=%" PRId32, rhs));
  }
  *dst = (rhs < 0 ? (lhs >> -rhs) : (lhs << rhs));
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_left_shift_i32vc) {
  *u7_vm0_state_local_i32(state, self->arg1.i64) =
      *u7_vm0_state_local_i32(state, self->arg2.i64) << self->arg3.i32;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_right_shift_i32vc) {
  *u7_vm0_state_local_i32(state, self->arg1.i64) =
      *u7_vm0_state_local_i32(state, self->arg2.i64) >> self->arg3.i32;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_left_shift_i32vv) {
  int32_t* const dst = u7_vm0_state_local_i32(state, self->arg1.i64);
  const int32_t lhs = *u7_vm0_state_local_i32(state, self->arg2.i64);
  const int32_t rhs = *u7_vm0_state_local_i32(state, self->arg3.i64);
  if (rhs < -31) {
    return u7_vm0_panic(
        state, u7_errnof(EINVAL, "bitwise_left_shift_i32: rhs=%" PRId32, rhs));
  } else if (rhs > 31) {
    return u7_vm0_panic(
        state, u7_errnof(EINVAL, "bitwise_left_shift_i32: rhs=%" PRId32, rhs));
  }
  *dst = (rhs < 0 ? (lhs >> -rhs) : (lhs << rhs));
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_left_shift_i64cv) {
  int64_t* const dst = u7_vm0_state_local_i64(state, self->arg1.i64);
  const int64_t lhs = self->arg2.i64;
  const int64_t rhs = *u7_vm0_state_local_i64(state, self->arg3.i64);
  if (rhs < -63) {
    return u7_vm0_panic(
        state, u7_errnof(EINVAL, "bitwise_left_shift_i64: rhs=%" PRId64, rhs));
  } else if (rhs > 63) {
    return u7_vm0_panic(
        state, u7_errnof(EINVAL, "bitwise_left_shift_i64: rhs=%" PRId64, rhs));
  }
  *dst = (rhs < 0 ? (lhs >> -rhs) : (lhs << rhs));
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_left_shift_i64vc) {
  *u7_vm0_state_local_i64(state, self->arg1.i64) =
      *u7_vm0_state_local_i64(state, self->arg2.i64) << self->arg3.i64;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_right_shift_i64vc) {
  *u7_vm0_state_local_i64(state, self->arg1.i64) =
      *u7_vm0_state_local_i64(state, self->arg2.i64) >> self->arg3.i64;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_left_shift_i64vv) {
  int64_t* const dst = u7_vm0_state_local_i64(state, self->arg1.i64);
  const int64_t lhs = *u7_vm0_state_local_i64(state, self->arg2.i64);
  const int64_t rhs = *u7_vm0_state_local_i64(state, self->arg3.i64);
  if (rhs < -63) {
    return u7_vm0_panic(
        state, u7_errnof(EINVAL, "bitwise_left_shift_i64: rhs=%" PRId64, rhs));
  } else if (rhs > 63) {
    return u7_vm0_panic(
        state, u7_errnof(EINVAL, "bitwise_left_shift_i64: rhs=%" PRId64, rhs));
  }
  *dst = (rhs < 0 ? (lhs >> -rhs) : (lhs << rhs));
  return true;
}

struct u7_vm0_instruction u7_vm0_bitwise_left_shift(u7_error* error,
                                                    struct u7_vm0_arg dst,
                                                    struct u7_vm0_arg lhs,
                                                    struct u7_vm0_arg rhs) {
  struct u7_vm0_instruction result = {0};
  if (error->error_code != 0) {
    return result;
  }
  result.arg1 = dst.value;
  result.arg2 = lhs.value;
  result.arg3 = rhs.value;
  if (dst.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
    if (lhs.kind == U7_VM0_ARG_KIND_I32_CONSTANT) {
      if (rhs.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
        result.base.execute_fn = bitwise_left_shift_i32cv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error(
            "u7_vm0_bitwise_left_shift_i32c", "rhs", rhs.kind);
      }
    } else if (lhs.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
      if (rhs.kind == U7_VM0_ARG_KIND_I32_CONSTANT) {
        if (rhs.value.i32 < -31) {
          *error =
              u7_errnof(EINVAL, "u7_vm0_bitwise_left_shift_i32vc: rhs < -31");
        } else if (rhs.value.i64 > 31) {
          *error =
              u7_errnof(EINVAL, "u7_vm0_bitwise_left_shift_i32vc: rhs > 31");
        } else if (rhs.value.i32 == 0) {
          *error =
              u7_errnof(EINVAL, "u7_vm0_bitwise_left_shift_i32vc: rhs = 0");
        } else if (rhs.value.i32 < 0) {
          result.arg3.i32 = -result.arg3.i32;
          result.base.execute_fn = bitwise_right_shift_i32vc_exec;
        } else {
          result.base.execute_fn = bitwise_left_shift_i32vc_exec;
        }
      } else if (rhs.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
        result.base.execute_fn = bitwise_left_shift_i32vv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error(
            "u7_vm0_bitwise_left_shift_i32v", "rhs", rhs.kind);
      }
    } else {
      *error = u7_vm0_unsupported_arg_kind_error(
          "u7_vm0_bitwise_left_shift_i32", "lhs", lhs.kind);
    }
  } else if (dst.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
    if (lhs.kind == U7_VM0_ARG_KIND_I64_CONSTANT) {
      if (rhs.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
        result.base.execute_fn = bitwise_left_shift_i64cv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error(
            "u7_vm0_bitwise_left_shift_i64c", "rhs", rhs.kind);
      }
    } else if (lhs.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
      if (rhs.kind == U7_VM0_ARG_KIND_I64_CONSTANT) {
        if (rhs.value.i64 < -63) {
          *error =
              u7_errnof(EINVAL, "u7_vm0_bitwise_left_shift_i64vc: rhs < -63");
        } else if (rhs.value.i64 > 63) {
          *error =
              u7_errnof(EINVAL, "u7_vm0_bitwise_left_shift_i64vc: rhs > 63");
        } else if (rhs.value.i64 == 0) {
          *error =
              u7_errnof(EINVAL, "u7_vm0_bitwise_left_shift_i64vc: rhs = 0");
        } else if (rhs.value.i64 < 0) {
          result.arg3.i64 = -result.arg3.i64;
          result.base.execute_fn = bitwise_right_shift_i64vc_exec;
        } else {
          result.base.execute_fn = bitwise_left_shift_i64vc_exec;
        }
      } else if (rhs.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
        result.base.execute_fn = bitwise_left_shift_i64vv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error(
            "u7_vm0_bitwise_left_shift_i64v", "rhs", rhs.kind);
      }
    } else {
      *error = u7_vm0_unsupported_arg_kind_error(
          "u7_vm0_bitwise_left_shift_i64", "lhs", lhs.kind);
    }
  } else {
    *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_bitwise_left_shift",
                                               "dst", dst.kind);
  }
  return result;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_add_i32vc) {
  int32_t* const dst = u7_vm0_state_local_i32(state, self->arg1.i64);
  const int32_t lhs = *u7_vm0_state_local_i32(state, self->arg2.i64);
  const int32_t rhs = self->arg3.i32;
  if (__builtin_add_overflow(lhs, rhs, dst)) {
    return u7_vm0_panic(
        state, u7_errnof(ERANGE,
                         "u7_vm0_math_add: integer overflow: lhs=%" PRId32
                         " rhs=%" PRId32,
                         lhs, rhs));
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_add_i32vv) {
  int32_t* const dst = u7_vm0_state_local_i32(state, self->arg1.i64);
  const int32_t lhs = *u7_vm0_state_local_i32(state, self->arg2.i64);
  const int32_t rhs = *u7_vm0_state_local_i32(state, self->arg3.i64);
  if (__builtin_add_overflow(lhs, rhs, dst)) {
    return u7_vm0_panic(
        state, u7_errnof(ERANGE,
                         "u7_vm0_math_add: integer overflow: lhs=%" PRId32
                         " rhs=%" PRId32,
                         lhs, rhs));
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_add_i64vc) {
  int64_t* const dst = u7_vm0_state_local_i64(state, self->arg1.i64);
  const int64_t lhs = *u7_vm0_state_local_i64(state, self->arg2.i64);
  const int64_t rhs = self->arg3.i64;
  if (__builtin_add_overflow(lhs, rhs, dst)) {
    return u7_vm0_panic(
        state, u7_errnof(ERANGE,
                         "u7_vm0_math_add: integer overflow: lhs=%" PRId64
                         " rhs=%" PRId64,
                         lhs, rhs));
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_add_i64vv) {
  int64_t* const dst = u7_vm0_state_local_i64(state, self->arg1.i64);
  const int64_t lhs = *u7_vm0_state_local_i64(state, self->arg2.i64);
  const int64_t rhs = *u7_vm0_state_local_i64(state, self->arg3.i64);
  if (__builtin_add_overflow(lhs, rhs, dst)) {
    return u7_vm0_panic(
        state, u7_errnof(ERANGE,
                         "u7_vm0_math_add: integer overflow: lhs=%" PRId64
                         " rhs=%" PRId64,
                         lhs, rhs));
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_add_f32vc) {
  *u7_vm0_state_local_f32(state, self->arg1.i64) =
      *u7_vm0_state_local_f32(state, self->arg2.i64) + self->arg3.f32;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_add_f32vv) {
  *u7_vm0_state_local_f32(state, self->arg1.i64) =
      *u7_vm0_state_local_f32(state, self->arg2.i64) +
      *u7_vm0_state_local_f32(state, self->arg3.i64);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_add_f64vc) {
  *u7_vm0_state_local_f64(state, self->arg1.i64) =
      *u7_vm0_state_local_f64(state, self->arg2.i64) + self->arg3.f64;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_add_f64vv) {
  *u7_vm0_state_local_f64(state, self->arg1.i64) =
      *u7_vm0_state_local_f64(state, self->arg2.i64) +
      *u7_vm0_state_local_f64(state, self->arg3.i64);
  return true;
}

struct u7_vm0_instruction u7_vm0_math_add(u7_error* error,
                                          struct u7_vm0_arg dst,
                                          struct u7_vm0_arg lhs,
                                          struct u7_vm0_arg rhs) {
  struct u7_vm0_instruction result = {0};
  if (error->error_code != 0) {
    return result;
  }
  result.arg1 = dst.value;
  result.arg2 = lhs.value;
  result.arg3 = rhs.value;
  if (dst.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
    if (lhs.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
      if (rhs.kind == U7_VM0_ARG_KIND_I32_CONSTANT) {
        result.base.execute_fn = math_add_i32vc_exec;
      } else if (rhs.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
        result.base.execute_fn = math_add_i32vv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_add_i32", "rhs",
                                                   rhs.kind);
      }
    } else {
      *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_add_i32", "lhs",
                                                 lhs.kind);
    }
  } else if (dst.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
    if (lhs.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
      if (rhs.kind == U7_VM0_ARG_KIND_I64_CONSTANT) {
        result.base.execute_fn = math_add_i64vc_exec;
      } else if (rhs.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
        result.base.execute_fn = math_add_i64vv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_add_i64", "rhs",
                                                   rhs.kind);
      }
    } else {
      *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_add_i64", "lhs",
                                                 lhs.kind);
    }
  } else if (dst.kind == U7_VM0_ARG_KIND_F32_VARIABLE) {
    if (lhs.kind == U7_VM0_ARG_KIND_F32_VARIABLE) {
      if (rhs.kind == U7_VM0_ARG_KIND_F32_CONSTANT) {
        result.base.execute_fn = math_add_f32vc_exec;
      } else if (rhs.kind == U7_VM0_ARG_KIND_F32_VARIABLE) {
        result.base.execute_fn = math_add_f32vv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_add_f32", "rhs",
                                                   rhs.kind);
      }
    } else {
      *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_add_f32", "lhs",
                                                 lhs.kind);
    }
  } else if (dst.kind == U7_VM0_ARG_KIND_F64_VARIABLE) {
    if (lhs.kind == U7_VM0_ARG_KIND_F64_VARIABLE) {
      if (rhs.kind == U7_VM0_ARG_KIND_F64_CONSTANT) {
        result.base.execute_fn = math_add_f64vc_exec;
      } else if (rhs.kind == U7_VM0_ARG_KIND_F64_VARIABLE) {
        result.base.execute_fn = math_add_f64vv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_add_f64", "rhs",
                                                   rhs.kind);
      }
    } else {
      *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_add_f64", "lhs",
                                                 lhs.kind);
    }
  } else {
    *error =
        u7_vm0_unsupported_arg_kind_error("u7_vm0_math_add", "dst", dst.kind);
  }
  return result;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_multiply_i32vc) {
  int32_t* const dst = u7_vm0_state_local_i32(state, self->arg1.i64);
  const int32_t lhs = *u7_vm0_state_local_i32(state, self->arg2.i64);
  const int32_t rhs = self->arg3.i32;
  if (__builtin_smul_overflow(lhs, rhs, dst)) {
    return u7_vm0_panic(
        state, u7_errnof(ERANGE,
                         "u7_vm0_math_multiply: integer overflow: lhs=%" PRId32
                         " rhs=%" PRId32,
                         lhs, rhs));
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_multiply_i32vv) {
  int32_t* const dst = u7_vm0_state_local_i32(state, self->arg1.i64);
  const int32_t lhs = *u7_vm0_state_local_i32(state, self->arg2.i64);
  const int32_t rhs = *u7_vm0_state_local_i32(state, self->arg3.i64);
  if (__builtin_mul_overflow(lhs, rhs, dst)) {
    return u7_vm0_panic(
        state, u7_errnof(ERANGE,
                         "u7_vm0_math_multiply: integer overflow: lhs=%" PRId32
                         " rhs=%" PRId32,
                         lhs, rhs));
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_multiply_i64vc) {
  int64_t* const dst = u7_vm0_state_local_i64(state, self->arg1.i64);
  const int64_t lhs = *u7_vm0_state_local_i64(state, self->arg2.i64);
  const int64_t rhs = self->arg3.i64;
  if (__builtin_mul_overflow(lhs, rhs, dst)) {
    return u7_vm0_panic(
        state, u7_errnof(ERANGE,
                         "u7_vm0_math_multiply: integer overflow: lhs=%" PRId64
                         " rhs=%" PRId64,
                         lhs, rhs));
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_multiply_i64vv) {
  int64_t* const dst = u7_vm0_state_local_i64(state, self->arg1.i64);
  const int64_t lhs = *u7_vm0_state_local_i64(state, self->arg2.i64);
  const int64_t rhs = *u7_vm0_state_local_i64(state, self->arg3.i64);
  if (__builtin_mul_overflow(lhs, rhs, dst)) {
    return u7_vm0_panic(
        state, u7_errnof(ERANGE,
                         "u7_vm0_math_multiply: integer overflow: lhs=%" PRId64
                         " rhs=%" PRId64,
                         lhs, rhs));
  }
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_multiply_f32vc) {
  *u7_vm0_state_local_f32(state, self->arg1.i64) =
      *u7_vm0_state_local_f32(state, self->arg2.i64) * self->arg3.f32;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_multiply_f32vv) {
  *u7_vm0_state_local_f32(state, self->arg1.i64) =
      *u7_vm0_state_local_f32(state, self->arg2.i64) *
      *u7_vm0_state_local_f32(state, self->arg3.i64);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_multiply_f64vc) {
  *u7_vm0_state_local_f64(state, self->arg1.i64) =
      *u7_vm0_state_local_f64(state, self->arg2.i64) * self->arg3.f64;
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(math_multiply_f64vv) {
  *u7_vm0_state_local_f64(state, self->arg1.i64) =
      *u7_vm0_state_local_f64(state, self->arg2.i64) *
      *u7_vm0_state_local_f64(state, self->arg3.i64);
  return true;
}

struct u7_vm0_instruction u7_vm0_math_multiply(u7_error* error,
                                               struct u7_vm0_arg dst,
                                               struct u7_vm0_arg lhs,
                                               struct u7_vm0_arg rhs) {
  struct u7_vm0_instruction result = {0};
  if (error->error_code != 0) {
    return result;
  }
  result.arg1 = dst.value;
  result.arg2 = lhs.value;
  result.arg3 = rhs.value;
  if (dst.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
    if (lhs.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
      if (rhs.kind == U7_VM0_ARG_KIND_I32_CONSTANT) {
        result.base.execute_fn = math_multiply_i32vc_exec;
      } else if (rhs.kind == U7_VM0_ARG_KIND_I32_VARIABLE) {
        result.base.execute_fn = math_multiply_i32vv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_multiply_i32",
                                                   "rhs", rhs.kind);
      }
    } else {
      *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_multiply_i32",
                                                 "lhs", lhs.kind);
    }
  } else if (dst.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
    if (lhs.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
      if (rhs.kind == U7_VM0_ARG_KIND_I64_CONSTANT) {
        result.base.execute_fn = math_multiply_i64vc_exec;
      } else if (rhs.kind == U7_VM0_ARG_KIND_I64_VARIABLE) {
        result.base.execute_fn = math_multiply_i64vv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_multiply_i64",
                                                   "rhs", rhs.kind);
      }
    } else {
      *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_multiply_i64",
                                                 "lhs", lhs.kind);
    }
  } else if (dst.kind == U7_VM0_ARG_KIND_F32_VARIABLE) {
    if (lhs.kind == U7_VM0_ARG_KIND_F32_VARIABLE) {
      if (rhs.kind == U7_VM0_ARG_KIND_F32_CONSTANT) {
        result.base.execute_fn = math_multiply_f32vc_exec;
      } else if (rhs.kind == U7_VM0_ARG_KIND_F32_VARIABLE) {
        result.base.execute_fn = math_multiply_f32vv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_multiply_f32",
                                                   "rhs", rhs.kind);
      }
    } else {
      *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_multiply_f32",
                                                 "lhs", lhs.kind);
    }
  } else if (dst.kind == U7_VM0_ARG_KIND_F64_VARIABLE) {
    if (lhs.kind == U7_VM0_ARG_KIND_F64_VARIABLE) {
      if (rhs.kind == U7_VM0_ARG_KIND_F64_CONSTANT) {
        result.base.execute_fn = math_multiply_f64vc_exec;
      } else if (rhs.kind == U7_VM0_ARG_KIND_F64_VARIABLE) {
        result.base.execute_fn = math_multiply_f64vv_exec;
      } else {
        *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_multiply_f64",
                                                   "rhs", rhs.kind);
      }
    } else {
      *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_multiply_f64",
                                                 "lhs", lhs.kind);
    }
  } else {
    *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_math_multiply", "dst",
                                               dst.kind);
  }
  return result;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_zero_i32) {
  const int32_t src = *u7_vm0_state_local_i32(state, self->arg1.i64);
  const size_t target = (size_t)self->arg2.i64;
  if (target >= state->instructions_size) {
    return u7_vm0_panic(
        state, u7_errnof(ERANGE, "u7_jump_if_zero: label is out of range: %zu",
                         target));
  }
  state->ip = (src == 0 ? target : state->ip);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_zero_i64) {
  const int64_t src = *u7_vm0_state_local_i64(state, self->arg1.i64);
  const size_t target = (size_t)self->arg2.i64;
  if (target >= state->instructions_size) {
    return u7_vm0_panic(
        state, u7_errnof(ERANGE, "u7_jump_if_zero: label is out of range: %zu",
                         target));
  }
  state->ip = (src == 0 ? target : state->ip);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_zero_f32) {
  const float src = *u7_vm0_state_local_f32(state, self->arg1.i64);
  const size_t target = (size_t)self->arg2.i64;
  if (target >= state->instructions_size) {
    return u7_vm0_panic(
        state, u7_errnof(ERANGE, "u7_jump_if_zero: label is out of range: %zu",
                         target));
  }
  state->ip = (src == 0 ? target : state->ip);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_zero_f64) {
  const double src = *u7_vm0_state_local_f64(state, self->arg1.i64);
  const size_t target = (size_t)self->arg2.i64;
  if (target >= state->instructions_size) {
    return u7_vm0_panic(
        state, u7_errnof(ERANGE, "u7_jump_if_zero: label is out of range: %zu",
                         target));
  }
  state->ip = (src == 0 ? target : state->ip);
  return true;
}

struct u7_vm0_instruction u7_vm0_jump_if_zero(u7_error* error,
                                              struct u7_vm0_arg src,
                                              struct u7_vm0_label label) {
  struct u7_vm0_instruction result = {0};
  if (error->error_code != 0) {
    return result;
  }
  result.arg1 = src.value;
  result.arg2.i64 = (int64_t)label.offset;
  switch (src.kind) {
    case U7_VM0_ARG_KIND_I32_VARIABLE:
      result.base.execute_fn = jump_if_zero_i32_exec;
      break;
    case U7_VM0_ARG_KIND_I64_VARIABLE:
      result.base.execute_fn = jump_if_zero_i64_exec;
      break;
    case U7_VM0_ARG_KIND_F32_VARIABLE:
      result.base.execute_fn = jump_if_zero_f32_exec;
      break;
    case U7_VM0_ARG_KIND_F64_VARIABLE:
      result.base.execute_fn = jump_if_zero_f64_exec;
      break;
    default:
      *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_jump_if_zero", "src",
                                                 src.kind);
  }
  return result;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_not_zero_i32) {
  const int32_t src = *u7_vm0_state_local_i32(state, self->arg1.i64);
  const size_t target = (size_t)self->arg2.i64;
  if (target >= state->instructions_size) {
    return u7_vm0_panic(
        state,
        u7_errnof(ERANGE, "u7_jump_if_not_zero: label is out of range: %zu",
                  target));
  }
  state->ip = (src != 0 ? target : state->ip);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_not_zero_i64) {
  const int64_t src = *u7_vm0_state_local_i64(state, self->arg1.i64);
  const size_t target = (size_t)self->arg2.i64;
  if (target >= state->instructions_size) {
    return u7_vm0_panic(
        state,
        u7_errnof(ERANGE, "u7_jump_if_not_zero: label is out of range: %zu",
                  target));
  }
  state->ip = (src != 0 ? target : state->ip);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_not_zero_f32) {
  const float src = *u7_vm0_state_local_f32(state, self->arg1.i64);
  const size_t target = (size_t)self->arg2.i64;
  if (target >= state->instructions_size) {
    return u7_vm0_panic(
        state,
        u7_errnof(ERANGE, "u7_jump_if_not_zero: label is out of range: %zu",
                  target));
  }
  state->ip = (src != 0 ? target : state->ip);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_not_zero_f64) {
  const double src = *u7_vm0_state_local_f64(state, self->arg1.i64);
  const size_t target = (size_t)self->arg2.i64;
  if (target >= state->instructions_size) {
    return u7_vm0_panic(
        state,
        u7_errnof(ERANGE, "u7_jump_if_not_zero: label is out of range: %zu",
                  target));
  }
  state->ip = (src != 0 ? target : state->ip);
  return true;
}

struct u7_vm0_instruction u7_vm0_jump_if_not_zero(u7_error* error,
                                                  struct u7_vm0_arg src,
                                                  struct u7_vm0_label label) {
  struct u7_vm0_instruction result = {0};
  if (error->error_code != 0) {
    return result;
  }
  result.arg1 = src.value;
  result.arg2.i64 = (int64_t)label.offset;
  switch (src.kind) {
    case U7_VM0_ARG_KIND_I32_VARIABLE:
      result.base.execute_fn = jump_if_not_zero_i32_exec;
      break;
    case U7_VM0_ARG_KIND_I64_VARIABLE:
      result.base.execute_fn = jump_if_not_zero_i64_exec;
      break;
    case U7_VM0_ARG_KIND_F32_VARIABLE:
      result.base.execute_fn = jump_if_not_zero_f32_exec;
      break;
    case U7_VM0_ARG_KIND_F64_VARIABLE:
      result.base.execute_fn = jump_if_not_zero_f64_exec;
      break;
    default:
      *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_jump_if_not_zero",
                                                 "src", src.kind);
  }
  return result;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(read_i32) {
  struct u7_vm0_globals* globals = u7_vm0_state_globals(state);
  assert(globals->input != NULL);
  int32_t value;
  u7_error err = globals->input->read_i32_fn(globals->input, &value);
  if (err.error_code != 0) {
    return u7_vm0_panic(state, err);
  }
  u7_vm_stack_push_i32(&state->stack, value);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_0(read_i32)

U7_VM0_DEFINE_INSTRUCTION_EXEC(read_i64) {
  struct u7_vm0_globals* globals = u7_vm0_state_globals(state);
  assert(globals->input != NULL);
  int64_t value;
  u7_error err = globals->input->read_i64_fn(globals->input, &value);
  if (err.error_code != 0) {
    return u7_vm0_panic(state, err);
  }
  u7_vm_stack_push_i64(&state->stack, value);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_0(read_i64)

U7_VM0_DEFINE_INSTRUCTION_EXEC(read_f32) {
  struct u7_vm0_globals* globals = u7_vm0_state_globals(state);
  assert(globals->input != NULL);
  float value;
  u7_error err = globals->input->read_f32_fn(globals->input, &value);
  if (err.error_code != 0) {
    return u7_vm0_panic(state, err);
  }
  u7_vm_stack_push_f32(&state->stack, value);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_0(read_f32)

U7_VM0_DEFINE_INSTRUCTION_EXEC(read_f64) {
  struct u7_vm0_globals* globals = u7_vm0_state_globals(state);
  assert(globals->input != NULL);
  double value;
  u7_error err = globals->input->read_f64_fn(globals->input, &value);
  if (err.error_code != 0) {
    return u7_vm0_panic(state, err);
  }
  u7_vm_stack_push_f64(&state->stack, value);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_0(read_f64)

U7_VM0_DEFINE_INSTRUCTION_EXEC(write_i32) {
  struct u7_vm0_globals* globals = u7_vm0_state_globals(state);
  assert(globals->output != NULL);
  assert(globals->error.error_code == 0);
  u7_error err = globals->output->write_i32_fn(
      globals->output, *u7_vm_stack_peek_i32(&state->stack));
  if (err.error_code != 0) {
    return u7_vm0_panic(state, err);
  }
  u7_vm_stack_pop_i32(&state->stack);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_0(write_i32)

U7_VM0_DEFINE_INSTRUCTION_EXEC(write_i64) {
  struct u7_vm0_globals* globals = u7_vm0_state_globals(state);
  assert(globals->output != NULL);
  assert(globals->error.error_code == 0);
  u7_error err = globals->output->write_i64_fn(
      globals->output, *u7_vm_stack_peek_i64(&state->stack));
  if (err.error_code != 0) {
    return u7_vm0_panic(state, err);
  }
  u7_vm_stack_pop_i64(&state->stack);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_0(write_i64)

U7_VM0_DEFINE_INSTRUCTION_EXEC(write_f32) {
  struct u7_vm0_globals* globals = u7_vm0_state_globals(state);
  assert(globals->output != NULL);
  assert(globals->error.error_code == 0);
  u7_error err = globals->output->write_f32_fn(
      globals->output, *u7_vm_stack_peek_f32(&state->stack));
  if (err.error_code != 0) {
    return u7_vm0_panic(state, err);
  }
  u7_vm_stack_pop_f32(&state->stack);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_0(write_f32)

U7_VM0_DEFINE_INSTRUCTION_EXEC(write_f64) {
  struct u7_vm0_globals* globals = u7_vm0_state_globals(state);
  assert(globals->output != NULL);
  assert(globals->error.error_code == 0);
  u7_error err = globals->output->write_f64_fn(
      globals->output, *u7_vm_stack_peek_f64(&state->stack));
  if (err.error_code != 0) {
    return u7_vm0_panic(state, err);
  }
  u7_vm_stack_pop_f64(&state->stack);
  return true;
}

U7_VM0_DEFINE_INSTRUCTION_0(write_f64)

U7_VM0_DEFINE_INSTRUCTION_EXEC(push_constant_i32) {
  u7_vm_stack_push_i32(&state->stack, self->arg1.i32);
  return true;
}

struct u7_vm0_instruction u7_vm0_push_constant_i32(int32_t value) {
  struct u7_vm0_instruction result = {
      .base = {.execute_fn = &push_constant_i32_exec},
      .arg1 = {.i32 = value},
  };
  return result;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(push_constant_i64) {
  u7_vm_stack_push_i64(&state->stack, self->arg1.i64);
  return true;
}

struct u7_vm0_instruction u7_vm0_push_constant_i64(int64_t value) {
  struct u7_vm0_instruction result = {
      .base = {.execute_fn = &push_constant_i64_exec},
      .arg1 = {.i64 = value},
  };
  return result;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(push_constant_f32) {
  u7_vm_stack_push_f32(&state->stack, self->arg1.f32);
  return true;
}

struct u7_vm0_instruction u7_vm0_push_constant_f32(float value) {
  struct u7_vm0_instruction result = {
      .base = {.execute_fn = &push_constant_f32_exec},
      .arg1 = {.f32 = value},
  };
  return result;
}

U7_VM0_DEFINE_INSTRUCTION_EXEC(push_constant_f64) {
  u7_vm_stack_push_f64(&state->stack, self->arg1.f64);
  return true;
}

struct u7_vm0_instruction u7_vm0_push_constant_f64(double value) {
  struct u7_vm0_instruction result = {
      .base = {.execute_fn = &push_constant_f64_exec},
      .arg1 = {.f64 = value},
  };
  return result;
}

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(push_local_i32) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert(offset % u7_vm_alignof(int32_t) == 0); */
/*   assert(offset >= 0); */
/*   assert(offset + sizeof(int32_t) <= */
/*          u7_vm_stack_current_frame_layout(&state->stack)->locals_size); */
/*   u7_vm_stack_push_i32(&state->stack, *(int32_t*)u7_vm_memory_add_offset(
 */
/*                                           u7_vm_state_locals(state),
 * offset)); */
/*   return true; */
/* } */

/* struct u7_vm0_instruction u7_vm0_push_local_i32( */
/*     struct u7_vm0_local_variable var) { */
/*   assert(var.offset >= 0); */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &push_local_i32_exec}, */
/*       .arg1 = {.i64 = var.offset}, */
/*   }; */
/*   return result; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(push_local_i64) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert(offset % u7_vm_alignof(int64_t) == 0); */
/*   assert(offset >= 0); */
/*   assert(offset + sizeof(int64_t) <= */
/*          u7_vm_stack_current_frame_layout(&state->stack)->locals_size); */
/*   u7_vm_stack_push_i64(&state->stack, *(int64_t*)u7_vm_memory_add_offset(
 */
/*                                           u7_vm_state_locals(state),
 * offset)); */
/*   return true; */
/* } */

/* struct u7_vm0_instruction u7_vm0_push_local_i64( */
/*     struct u7_vm0_local_variable var) { */
/*   assert(var.offset >= 0); */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &push_local_i64_exec}, */
/*       .arg1 = {.i64 = var.offset}, */
/*   }; */
/*   return result; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(push_local_f32) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert(offset % u7_vm_alignof(float) == 0); */
/*   assert(offset >= 0); */
/*   assert(offset + sizeof(float) <= */
/*          u7_vm_stack_current_frame_layout(&state->stack)->locals_size); */
/*   u7_vm_stack_push_f32(&state->stack, *(float*)u7_vm_memory_add_offset( */
/*                                           u7_vm_state_locals(state),
 * offset)); */
/*   return true; */
/* } */

/* struct u7_vm0_instruction u7_vm0_push_local_f32( */
/*     struct u7_vm0_local_variable var) { */
/*   assert(var.offset >= 0); */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &push_local_f32_exec}, */
/*       .arg1 = {.i64 = var.offset}, */
/*   }; */
/*   return result; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(push_local_f64) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert(offset % u7_vm_alignof(double) == 0); */
/*   assert(offset >= 0); */
/*   assert(offset + sizeof(double) <= */
/*          u7_vm_stack_current_frame_layout(&state->stack)->locals_size); */
/*   u7_vm_stack_push_f32(&state->stack, *(double*)u7_vm_memory_add_offset( */
/*                                           u7_vm_state_locals(state),
 * offset)); */
/*   return true; */
/* } */

/* struct u7_vm0_instruction u7_vm0_push_local_f64( */
/*     struct u7_vm0_local_variable var) { */
/*   assert(var.offset >= 0); */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &push_local_f64_exec}, */
/*       .arg1 = {.i64 = var.offset}, */
/*   }; */
/*   return result; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(store_local_i32) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert(offset % u7_vm_alignof(int32_t) == 0); */
/*   assert(offset >= 0); */
/*   assert(offset + sizeof(int32_t) <= */
/*          u7_vm_stack_current_frame_layout(&state->stack)->locals_size); */
/*   *(int32_t*)u7_vm_memory_add_offset(u7_vm_state_locals(state), offset) =
 */
/*       u7_vm_stack_pop_i32(&state->stack); */
/*   return true; */
/* } */

/* struct u7_vm0_instruction u7_vm0_store_local_i32( */
/*     struct u7_vm0_local_variable var) { */
/*   assert(var.offset >= 0); */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &store_local_i32_exec}, */
/*       .arg1 = {.i64 = var.offset}, */
/*   }; */
/*   return result; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(store_local_i64) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert(offset % u7_vm_alignof(int64_t) == 0); */
/*   assert(offset >= 0); */
/*   assert(offset + sizeof(int64_t) <= */
/*          u7_vm_stack_current_frame_layout(&state->stack)->locals_size); */
/*   *(int64_t*)u7_vm_memory_add_offset(u7_vm_state_locals(state), offset) =
 */
/*       u7_vm_stack_pop_i64(&state->stack); */
/*   return true; */
/* } */

/* struct u7_vm0_instruction u7_vm0_store_local_i64( */
/*     struct u7_vm0_local_variable var) { */
/*   assert(var.offset >= 0); */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &store_local_i64_exec}, */
/*       .arg1 = {.i64 = var.offset}, */
/*   }; */
/*   return result; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(store_local_f32) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert(offset % u7_vm_alignof(float) == 0); */
/*   assert(offset >= 0); */
/*   assert(offset + sizeof(float) <= */
/*          u7_vm_stack_current_frame_layout(&state->stack)->locals_size); */
/*   *(float*)u7_vm_memory_add_offset(u7_vm_state_locals(state), offset) = */
/*       u7_vm_stack_pop_f32(&state->stack); */
/*   return true; */
/* } */

/* struct u7_vm0_instruction u7_vm0_store_local_f32( */
/*     struct u7_vm0_local_variable var) { */
/*   assert(var.offset >= 0); */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &store_local_f32_exec}, */
/*       .arg1 = {.i64 = var.offset}, */
/*   }; */
/*   return result; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(store_local_f64) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert(offset % u7_vm_alignof(double) == 0); */
/*   assert(offset >= 0); */
/*   assert(offset + sizeof(double) <= */
/*          u7_vm_stack_current_frame_layout(&state->stack)->locals_size); */
/*   *(double*)u7_vm_memory_add_offset(u7_vm_state_locals(state), offset) = */
/*       u7_vm_stack_pop_f64(&state->stack); */
/*   return true; */
/* } */

/* struct u7_vm0_instruction u7_vm0_store_local_f64( */
/*     struct u7_vm0_local_variable var) { */
/*   assert(var.offset >= 0); */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &store_local_f64_exec}, */
/*       .arg1 = {.i64 = var.offset}, */
/*   }; */
/*   return result; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(compare_i32) { */
/*   int32_t rhs = u7_vm_stack_pop_i32(&state->stack); */
/*   int32_t lhs = u7_vm_stack_pop_i32(&state->stack); */
/*   u7_vm_stack_push_i32(&state->stack, (lhs < rhs ? -1 : (lhs > rhs ? 1 :
 * 0))); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(compare_i32) */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(compare_i64_exec) { */
/*   int64_t rhs = u7_vm_stack_pop_i64(&state->stack); */
/*   int64_t lhs = u7_vm_stack_pop_i64(&state->stack); */
/*   u7_vm_stack_push_i32(&state->stack, (lhs < rhs ? -1 : (lhs > rhs ? 1 :
 * 0))); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(compare_i64) */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(compare_f32_exec) { */
/*   float rhs = u7_vm_stack_pop_f32(&state->stack); */
/*   float lhs = u7_vm_stack_pop_f32(&state->stack); */
/*   u7_vm_stack_push_i32(&state->stack, (lhs < rhs ? -1 : (lhs > rhs ? 1 :
 * 0))); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(compare_f32) */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(compare_f64_exec) { */
/*   double rhs = u7_vm_stack_pop_f64(&state->stack); */
/*   double lhs = u7_vm_stack_pop_f64(&state->stack); */
/*   u7_vm_stack_push_i32(&state->stack, (lhs < rhs ? -1 : (lhs > rhs ? 1 :
 * 0))); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(compare_f64) */

/* // jump */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_i32_zero_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   if (u7_vm_stack_pop_i32(&state->stack) == 0) { */
/*     state->ip += offset; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_i32_negative_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   if (u7_vm_stack_pop_i32(&state->stack) < 0) { */
/*     state->ip += offset; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_i32_positive_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   if (u7_vm_stack_pop_i32(&state->stack) > 0) { */
/*     state->ip += offset; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_i32_not_zero_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   if (u7_vm_stack_pop_i32(&state->stack) != 0) { */
/*     state->ip += offset; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_i32_negative_or_zero_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   if (u7_vm_stack_pop_i32(&state->stack) <= 0) { */
/*     state->ip += offset; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_i32_positive_or_zero_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   if (u7_vm_stack_pop_i32(&state->stack) >= 0) { */
/*     state->ip += offset; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_i64_zero_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   if (u7_vm_stack_pop_i64(&state->stack) == 0) { */
/*     state->ip += offset; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_i64_negative_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   if (u7_vm_stack_pop_i64(&state->stack) < 0) { */
/*     state->ip += offset; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_i64_positive_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   if (u7_vm_stack_pop_i64(&state->stack) > 0) { */
/*     state->ip += offset; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_i64_not_zero_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   if (u7_vm_stack_pop_i64(&state->stack) != 0) { */
/*     state->ip += offset; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_i64_negative_or_zero_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   if (u7_vm_stack_pop_i64(&state->stack) <= 0) { */
/*     state->ip += offset; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_if_i64_positive_or_zero_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   if (u7_vm_stack_pop_i64(&state->stack) >= 0) { */
/*     state->ip += offset; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(jump_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   assert((offset >= 0 && state->ip + offset < state->instructions_size) ||
 */
/*          (offset < 0 && state->ip >= -(size_t)offset)); */
/*   state->ip += offset; */
/*   return true; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump_if_i32_zero( */
/*     struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_if_i32_zero_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump_if_i32_not_zero( */
/*     struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_if_i32_not_zero_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump_if_i32_negative( */
/*     struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_if_i32_negative_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump_if_i32_negative_or_zero( */
/*     struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_if_i32_negative_or_zero_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump_if_i32_positive( */
/*     struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_if_i32_positive_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump_if_i32_positive_or_zero( */
/*     struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_if_i32_positive_or_zero_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump_if_i64_zero( */
/*     struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_if_i64_zero_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump_if_i64_not_zero( */
/*     struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_if_i64_not_zero_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump_if_i64_negative( */
/*     struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_if_i64_negative_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump_if_i64_negative_or_zero( */
/*     struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_if_i64_negative_or_zero_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump_if_i64_positive( */
/*     struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_if_i64_positive_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump_if_i64_positive_or_zero( */
/*     struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_if_i64_positive_or_zero_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_jump(struct u7_vm0_local_label label) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &jump_exec}, */
/*       .arg1 = {.i64 = label.offset}, */
/*   }; */
/*   return result; */
/* } */

/* // u7_vm0_operation */

/* // duplicate */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(duplicate_i32_exec) { */
/*   u7_vm_stack_duplicate_i32(&state->stack); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(duplicate_i64_exec) { */
/*   u7_vm_stack_duplicate_i64(&state->stack); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(duplicate_f32_exec) { */
/*   u7_vm_stack_duplicate_f32(&state->stack); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(duplicate_f64_exec) { */
/*   u7_vm_stack_duplicate_f64(&state->stack); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(duplicate_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(duplicate_i64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(duplicate_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(duplicate_f64) */

/* // not */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_not_i32_exec) { */
/*   int32_t* p = u7_vm_stack_peek_i32(&state->stack); */
/*   *p = ~(*p); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_not_i64_exec) { */
/*   int64_t* p = u7_vm_stack_peek_i64(&state->stack); */
/*   *p = ~(*p); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(bitwise_not_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(bitwise_not_i64) */

/* // and */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_and_i32_exec) { */
/*   int32_t rhs = u7_vm_stack_pop_i32(&state->stack); */
/*   int32_t* p = u7_vm_stack_peek_i32(&state->stack); */
/*   *p &= rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_and_i64_exec) { */
/*   int64_t rhs = u7_vm_stack_pop_i64(&state->stack); */
/*   int64_t* p = u7_vm_stack_peek_i64(&state->stack); */
/*   *p &= rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(bitwise_and_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(bitwise_and_i64) */

/* // or */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_or_i32_exec) { */
/*   int32_t rhs = u7_vm_stack_pop_i32(&state->stack); */
/*   int32_t* p = u7_vm_stack_peek_i32(&state->stack); */
/*   *p |= rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_or_i64_exec) { */
/*   int64_t rhs = u7_vm_stack_pop_i64(&state->stack); */
/*   int64_t* p = u7_vm_stack_peek_i64(&state->stack); */
/*   *p |= rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(bitwise_or_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(bitwise_or_i64) */

/* // xor */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_xor_i32_exec) { */
/*   int32_t rhs = u7_vm_stack_pop_i32(&state->stack); */
/*   int32_t* p = u7_vm_stack_peek_i32(&state->stack); */
/*   *p |= rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(bitwise_xor_i64_exec) { */
/*   int64_t rhs = u7_vm_stack_pop_i64(&state->stack); */
/*   int64_t* p = u7_vm_stack_peek_i64(&state->stack); */
/*   *p |= rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(bitwise_xor_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(bitwise_xor_i64) */

/* // abs */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(abs_i32_exec) { */
/*   int32_t* p = u7_vm_stack_peek_i32(&state->stack); */
/*   int32_t x = *p; */
/*   if (x == INT32_MIN) { */
/*     state->error = u7_errorf(u7_errno_category(), ERANGE, */
/*                              "integer overflow: u7_vm0_abs_i32 x=%" PRId32,
 * x); */
/*     return false; */
/*   } else { */
/*     *p = (x >= 0 ? x : -x); */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(abs_i64_exec) { */
/*   int64_t* p = u7_vm_stack_peek_i64(&state->stack); */
/*   int64_t x = *p; */
/*   if (x == INT64_MIN) { */
/*     state->error = u7_errorf(u7_errno_category(), ERANGE, */
/*                              "integer overflow: u7_vm0_abs_i64 x=%" PRId64,
 * x); */
/*     return false; */
/*   } else { */
/*     *p = (x >= 0 ? x : -x); */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(abs_f32_exec) { */
/*   float* x = u7_vm_stack_peek_f32(&state->stack); */
/*   *x = __builtin_fabsf(*x); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(abs_f64_exec) { */
/*   double* x = u7_vm_stack_peek_f64(&state->stack); */
/*   *x = __builtin_fabs(*x); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(abs_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(abs_i64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(abs_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(abs_f64) */

/* // neg */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(neg_i32_exec) { */
/*   int32_t* p = u7_vm_stack_peek_i32(&state->stack); */
/*   int32_t x = *p; */
/*   if (x == INT32_MIN) { */
/*     state->error = u7_errorf(u7_errno_category(), ERANGE, */
/*                              "integer overflow: u7_vm0_neg_i32 x=%" PRId32,
 * x); */
/*     return false; */
/*   } else { */
/*     *p = -x; */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(neg_i64_exec) { */
/*   int64_t* p = u7_vm_stack_peek_i64(&state->stack); */
/*   int64_t x = *p; */
/*   if (x == INT64_MIN) { */
/*     state->error = u7_errorf(u7_errno_category(), ERANGE, */
/*                              "integer overflow: u7_vm0_neg_i64 x=%" PRId64,
 * x); */
/*     return false; */
/*   } else { */
/*     *p = -x; */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(neg_f32_exec) { */
/*   float* p = u7_vm_stack_peek_f32(&state->stack); */
/*   float x = *p; */
/*   *p = -x; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(neg_f64_exec) { */
/*   double* p = u7_vm_stack_peek_f64(&state->stack); */
/*   double x = *p; */
/*   *p = -x; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(neg_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(neg_i64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(neg_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(neg_f64) */

/* // inc */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(inc_i32_exec) { */
/*   int32_t* p = u7_vm_stack_peek_i32(&state->stack); */
/*   int32_t x = *p; */
/*   int32_t delta = self->arg1.i32; */
/*   if (__builtin_add_overflow(x, delta, p)) { */
/*     state->error = u7_errorf(u7_errno_category(), ERANGE, */
/*                              "integer overflow: u7_vm0_inc_i32 x=%" PRId32
 */
/*                              ", delta=%" PRId32, */
/*                              x, delta); */
/*     return false; */
/*   } else { */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(inc_i64_exec) { */
/*   int64_t* p = u7_vm_stack_peek_i64(&state->stack); */
/*   int64_t x = *p; */
/*   int64_t delta = self->arg1.i64; */
/*   if (__builtin_add_overflow(x, delta, p)) { */
/*     state->error = u7_errorf(u7_errno_category(), ERANGE, */
/*                              "integer overflow: u7_vm0_inc_i64 x=%" PRId64
 */
/*                              ", delta=%" PRId64, */
/*                              x, delta); */
/*     return false; */
/*   } else { */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(inc_f32_exec) { */
/*   float* lhs = u7_vm_stack_peek_f32(&state->stack); */
/*   *lhs += self->arg1.f32; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(inc_f64_exec) { */
/*   double* lhs = u7_vm_stack_peek_f64(&state->stack); */
/*   *lhs += self->arg1.f64; */
/*   return true; */
/* } */

/* struct u7_vm0_instruction u7_vm0_inc_i32(int32_t delta) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &inc_i32_exec}, */
/*       .arg1 = {.i32 = delta}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_inc_i64(int64_t delta) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &inc_i64_exec}, */
/*       .arg1 = {.i64 = delta}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_inc_f32(float delta) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &inc_f32_exec}, */
/*       .arg1 = {.f32 = delta}, */
/*   }; */
/*   return result; */
/* } */

/* struct u7_vm0_instruction u7_vm0_inc_f64(double delta) { */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &inc_f64_exec}, */
/*       .arg1 = {.f64 = delta}, */
/*   }; */
/*   return result; */
/* } */

/* // inc_local */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(inc_local_i64_exec) { */
/*   int64_t offset = self->arg1.i64; */
/*   int64_t delta = self->arg2.i64; */
/*   assert(offset % u7_vm_alignof(int64_t) == 0); */
/*   assert(offset >= 0); */
/*   assert(offset + sizeof(int64_t) <= */
/*          u7_vm_stack_current_frame_layout(&state->stack)->locals_size); */
/*   int64_t* p = (int64_t*)u7_vm_memory_add_offset( */
/*       u7_vm_stack_current_locals(&state->stack), offset); */
/*   int64_t x = *p; */
/*   if (__builtin_add_overflow(x, delta, p)) { */
/*     state->error = u7_errorf(u7_errno_category(), ERANGE, */
/*                              "integer overflow: u7_vm0_inc_local_i64 x=%"
 * PRId64 */
/*                              ", delta=%" PRId64, */
/*                              x, delta); */
/*     return false; */
/*   } */
/*   u7_vm_stack_push_i64(&state->stack, *p); */
/*   return true; */
/* } */

/* struct u7_vm0_instruction u7_vm0_inc_local_i64(struct u7_vm0_local_variable
 * var, */
/*                                                int64_t delta) { */
/*   assert(var.offset >= 0); */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &inc_local_i64_exec}, */
/*       .arg1 = {.i64 = var.offset}, */
/*       .arg2 = {.i64 = delta}, */
/*   }; */
/*   return result; */
/* } */

/* // add */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(add_i32_exec) { */
/*   int32_t rhs = u7_vm_stack_pop_i32(&state->stack); */
/*   int32_t* p = u7_vm_stack_peek_i32(&state->stack); */
/*   int32_t lhs = *p; */
/*   if (__builtin_add_overflow(lhs, rhs, p)) { */
/*     state->error = u7_errorf(u7_errno_category(), ERANGE, */
/*                              "integer overflow: u7_vm0_add_i32 lhs=%"
 * PRId32
 */
/*                              ", rhs=%" PRId32, */
/*                              lhs, rhs); */
/*     return false; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(add_i64_exec) { */
/*   int64_t rhs = u7_vm_stack_pop_i64(&state->stack); */
/*   int64_t* p = u7_vm_stack_peek_i64(&state->stack); */
/*   int64_t lhs = *p; */
/*   if (__builtin_add_overflow(lhs, rhs, p)) { */
/*     state->error = u7_errorf(u7_errno_category(), ERANGE, */
/*                              "integer overflow: u7_vm0_add_i64 lhs=%"
 * PRId64
 */
/*                              ", rhs=%" PRId64, */
/*                              lhs, rhs); */
/*     return false; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(add_f32_exec) { */
/*   float rhs = u7_vm_stack_pop_f32(&state->stack); */
/*   float* lhs = u7_vm_stack_peek_f32(&state->stack); */
/*   *lhs += rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(add_f64_exec) { */
/*   double rhs = u7_vm_stack_pop_f64(&state->stack); */
/*   double* lhs = u7_vm_stack_peek_f64(&state->stack); */
/*   *lhs += rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(add_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(add_i64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(add_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(add_f64) */

/* // subtract */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(subtract_i32_exec) { */
/*   int32_t rhs = u7_vm_stack_pop_i32(&state->stack); */
/*   int32_t* p = u7_vm_stack_peek_i32(&state->stack); */
/*   int32_t lhs = *p; */
/*   if (__builtin_sub_overflow(lhs, rhs, p)) { */
/*     state->error = u7_errorf( */
/*         u7_errno_category(), ERANGE, */
/*         "integer overflow: u7_vm0_subtract_i32 lhs=%" PRId32 ", rhs=%"
 * PRId32, */
/*         lhs, rhs); */
/*     return false; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(subtract_i64_exec) { */
/*   int64_t rhs = u7_vm_stack_pop_i64(&state->stack); */
/*   int64_t* p = u7_vm_stack_peek_i64(&state->stack); */
/*   int64_t lhs = *p; */
/*   if (__builtin_sub_overflow(lhs, rhs, p)) { */
/*     state->error = u7_errorf( */
/*         u7_errno_category(), ERANGE, */
/*         "integer overflow: u7_vm0_subtract_i64 lhs=%" PRId64 ", rhs=%"
 * PRId64, */
/*         lhs, rhs); */
/*     return false; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(subtract_f32_exec) { */
/*   float rhs = u7_vm_stack_pop_f32(&state->stack); */
/*   float* lhs = u7_vm_stack_peek_f32(&state->stack); */
/*   *lhs -= rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(subtract_f64_exec) { */
/*   double rhs = u7_vm_stack_pop_f64(&state->stack); */
/*   double* lhs = u7_vm_stack_peek_f64(&state->stack); */
/*   *lhs -= rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(subtract_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(subtract_i64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(subtract_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(subtract_f64) */

/* // multiply */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(multiply_i32_exec) { */
/*   int32_t rhs = u7_vm_stack_pop_i32(&state->stack); */
/*   int32_t* p = u7_vm_stack_peek_i32(&state->stack); */
/*   int32_t lhs = *p; */
/*   if (__builtin_mul_overflow(lhs, rhs, p)) { */
/*     state->error = u7_errorf( */
/*         u7_errno_category(), ERANGE, */
/*         "integer overflow: u7_vm0_multiply_i32 lhs=%" PRId32 ", rhs=%"
 * PRId32, */
/*         lhs, rhs); */
/*     return false; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(multiply_i64_exec) { */
/*   int64_t rhs = u7_vm_stack_pop_i64(&state->stack); */
/*   int64_t* p = u7_vm_stack_peek_i64(&state->stack); */
/*   int64_t lhs = *p; */
/*   if (__builtin_mul_overflow(lhs, rhs, p)) { */
/*     state->error = u7_errorf( */
/*         u7_errno_category(), ERANGE, */
/*         "integer overflow: u7_vm0_multiply_i64 lhs=%" PRId64 ", rhs=%"
 * PRId64, */
/*         lhs, rhs); */
/*     return false; */
/*   } */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(multiply_f32_exec) { */
/*   float rhs = u7_vm_stack_pop_f32(&state->stack); */
/*   float* lhs = u7_vm_stack_peek_f32(&state->stack); */
/*   *lhs *= rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(multiply_f64_exec) { */
/*   double rhs = u7_vm_stack_pop_f64(&state->stack); */
/*   double* lhs = u7_vm_stack_peek_f64(&state->stack); */
/*   *lhs *= rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(multiply_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(multiply_i64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(multiply_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(multiply_f64) */

/* // divide */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(divide_f32_exec) { */
/*   float rhs = u7_vm_stack_pop_f32(&state->stack); */
/*   float* lhs = u7_vm_stack_peek_f32(&state->stack); */
/*   *lhs /= rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(divide_f64_exec) { */
/*   double rhs = u7_vm_stack_pop_f64(&state->stack); */
/*   double* lhs = u7_vm_stack_peek_f64(&state->stack); */
/*   *lhs /= rhs; */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(divide_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(divide_f64) */

/* // floordiv */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(floordiv_u32_exec) { */
/*   uint32_t rhs = (uint32_t)u7_vm_stack_pop_i32(&state->stack); */
/*   uint32_t* lhs = (uint32_t*)u7_vm_stack_peek_i32(&state->stack); */
/*   if (rhs == 0) { */
/*     state->error = u7_errorf( */
/*         u7_errno_category(), ERANGE, */
/*         "division by zero: u7_vm0_floordiv_u32 lhs=%" PRIu32 ", rhs=%"
 * PRIu32, */
/*         *lhs, rhs); */
/*     return false; */
/*   } else { */
/*     *lhs /= rhs; */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(floordiv_u64_exec) { */
/*   uint64_t rhs = (uint64_t)u7_vm_stack_pop_i64(&state->stack); */
/*   uint64_t* lhs = (uint64_t*)u7_vm_stack_peek_i64(&state->stack); */
/*   if (rhs == 0) { */
/*     state->error = u7_errorf( */
/*         u7_errno_category(), ERANGE, */
/*         "division by zero: u7_vm0_floordiv_u64 lhs=%" PRIu64 ", rhs=%"
 * PRIu64, */
/*         *lhs, rhs); */
/*     return false; */
/*   } else { */
/*     *lhs /= rhs; */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(floordiv_u32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(floordiv_u64) */

/* // floormod */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(floormod_u32_exec) { */
/*   uint32_t rhs = (uint32_t)u7_vm_stack_pop_i32(&state->stack); */
/*   uint32_t* lhs = (uint32_t*)u7_vm_stack_peek_i32(&state->stack); */
/*   if (rhs == 0) { */
/*     state->error = u7_errorf( */
/*         u7_errno_category(), ERANGE, */
/*         "modision by zero: u7_vm0_floormod_u32 lhs=%" PRIu32 ", rhs=%"
 * PRIu32, */
/*         *lhs, rhs); */
/*     return false; */
/*   } else { */
/*     *lhs %= rhs; */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(floormod_u64_exec) { */
/*   uint64_t rhs = (uint64_t)u7_vm_stack_pop_i64(&state->stack); */
/*   uint64_t* lhs = (uint64_t*)u7_vm_stack_peek_i64(&state->stack); */
/*   if (rhs == 0) { */
/*     state->error = u7_errorf( */
/*         u7_errno_category(), ERANGE, */
/*         "modision by zero: u7_vm0_floormod_u64 lhs=%" PRIu64 ", rhs=%"
 * PRIu64, */
/*         *lhs, rhs); */
/*     return false; */
/*   } else { */
/*     *lhs %= rhs; */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(floormod_u32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(floormod_u64) */

/* // floormod_local */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(floormod_local_u64_exec) { */
/*   int64_t lhs_offset = self->arg1.i64; */
/*   assert(lhs_offset % u7_vm_alignof(int64_t) == 0); */
/*   assert(lhs_offset >= 0); */
/*   assert(lhs_offset + sizeof(int64_t) <= */
/*          u7_vm_stack_current_frame_layout(&state->stack)->locals_size); */
/*   uint64_t lhs = *(int64_t*)u7_vm_memory_add_offset( */
/*       u7_vm_stack_current_locals(&state->stack), lhs_offset); */
/*   int64_t rhs_offset = self->arg2.i64; */
/*   assert(rhs_offset % u7_vm_alignof(int64_t) == 0); */
/*   assert(rhs_offset >= 0); */
/*   assert(rhs_offset + sizeof(int64_t) <= */
/*          u7_vm_stack_current_frame_layout(&state->stack)->locals_size); */
/*   uint64_t rhs = *(int64_t*)u7_vm_memory_add_offset( */
/*       u7_vm_stack_current_locals(&state->stack), rhs_offset); */
/*   if (rhs == 0) { */
/*     state->error = u7_errorf( */
/*         u7_errno_category(), ERANGE, */
/*         "modision by zero: u7_vm0_floormod_u64 lhs=%" PRIu64 ", rhs=%"
 * PRIu64, */
/*         lhs, rhs); */
/*     return false; */
/*   } else { */
/*     u7_vm_stack_push_i64(&state->stack, (int64_t)(lhs % rhs)); */
/*     return true; */
/*   } */
/* } */

/* struct u7_vm0_instruction u7_vm0_floormod_local_u64( */
/*     struct u7_vm0_local_variable lhs, struct u7_vm0_local_variable rhs) {
 */
/*   assert(lhs.offset >= 0); */
/*   assert(rhs.offset >= 0); */
/*   struct u7_vm0_instruction result = { */
/*       .base = {.execute_fn = &floormod_local_u64_exec}, */
/*       .arg1 = {.i64 = lhs.offset}, */
/*       .arg2 = {.i64 = rhs.offset}, */
/*   }; */
/*   return result; */
/* } */

/* // rounding */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(floor_f32_exec) { */
/*   float* p = u7_vm_stack_peek_f32(&state->stack); */
/*   *p = __builtin_floorf(*p); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(floor_f64_exec) { */
/*   double* p = u7_vm_stack_peek_f64(&state->stack); */
/*   *p = __builtin_floor(*p); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(floor_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(floor_f64) */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(ceil_f32_exec) { */
/*   float* p = u7_vm_stack_peek_f32(&state->stack); */
/*   *p = __builtin_ceilf(*p); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(ceil_f64_exec) { */
/*   double* p = u7_vm_stack_peek_f64(&state->stack); */
/*   *p = __builtin_ceil(*p); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(ceil_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(ceil_f64) */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(round_f32_exec) { */
/*   float* p = u7_vm_stack_peek_f32(&state->stack); */
/*   *p = __builtin_roundf(*p); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(round_f64_exec) { */
/*   double* p = u7_vm_stack_peek_f64(&state->stack); */
/*   *p = __builtin_round(*p); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(round_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(round_f64) */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(trunc_f32_exec) { */
/*   float* p = u7_vm_stack_peek_f32(&state->stack); */
/*   *p = __builtin_truncf(*p); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(trunc_f64_exec) { */
/*   double* p = u7_vm_stack_peek_f64(&state->stack); */
/*   *p = __builtin_trunc(*p); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(trunc_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(trunc_f64) */

/* // sqrt */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(sqrt_f32_exec) { */
/*   float* p = u7_vm_stack_peek_f32(&state->stack); */
/*   *p = __builtin_sqrtf(*p); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(sqrt_f64_exec) { */
/*   double* p = u7_vm_stack_peek_f64(&state->stack); */
/*   *p = __builtin_sqrt(*p); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(sqrt_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(sqrt_f64) */

/* // cast */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(cast_i32_to_i64_exec) { */
/*   u7_vm_stack_push_i64(&state->stack, u7_vm_stack_pop_i32(&state->stack));
 */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(cast_i32_to_f32_exec) { */
/*   u7_vm_stack_push_f32(&state->stack, u7_vm_stack_pop_i32(&state->stack));
 */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(cast_i32_to_f64_exec) { */
/*   u7_vm_stack_push_f64(&state->stack, u7_vm_stack_pop_i32(&state->stack));
 */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(cast_i64_to_i32_exec) { */
/*   int64_t x = u7_vm_stack_pop_i64(&state->stack); */
/*   if (x < INT32_MIN || x > INT32_MAX) { */
/*     state->error = */
/*         u7_errorf(u7_errno_category(), ERANGE, */
/*                   "integer overflow: u7_vm0_cast_i64_to_i32 x=%" PRId64,
 * x);
 */
/*     return false; */
/*   } else { */
/*     u7_vm_stack_push_i32(&state->stack, x); */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(cast_i64_to_f32_exec) { */
/*   u7_vm_stack_push_f32(&state->stack, u7_vm_stack_pop_i64(&state->stack));
 */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(cast_i64_to_f64_exec) { */
/*   u7_vm_stack_push_f64(&state->stack, u7_vm_stack_pop_i64(&state->stack));
 */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(cast_f32_to_f64_exec) { */
/*   u7_vm_stack_push_f64(&state->stack, u7_vm_stack_pop_f32(&state->stack));
 */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(cast_f64_to_f32_exec) { */
/*   u7_vm_stack_push_f32(&state->stack, u7_vm_stack_pop_f64(&state->stack));
 */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(cast_i32_to_i64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(cast_i32_to_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(cast_i32_to_f64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(cast_i64_to_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(cast_i64_to_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(cast_i64_to_f64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(cast_f32_to_f64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(cast_f64_to_f32) */

/* // trunc_f_to_i */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(trunc_f32_to_i32_exec) { */
/*   float x = u7_vm_stack_pop_f32(&state->stack); */
/*   // FIXME(apronchenkov): Make an check exact. */
/*   if (x < (float)INT32_MIN || x > (float)INT32_MAX) { */
/*     state->error = */
/*         u7_errorf(u7_errno_category(), ERANGE, */
/*                   "integer overflow: u7_vm0_trunc_f32_to_i32 x=%f", x); */
/*     return false; */
/*   } else { */
/*     u7_vm_stack_push_i32(&state->stack, x); */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(trunc_f32_to_i64_exec) { */
/*   float x = u7_vm_stack_pop_f32(&state->stack); */
/*   // FIXME(apronchenkov): Make an check exact. */
/*   if (x < (float)INT64_MIN || x > (float)INT64_MAX) { */
/*     state->error = */
/*         u7_errorf(u7_errno_category(), ERANGE, */
/*                   "integer overflow: u7_vm0_trunc_f32_to_i64 x=%f", x); */
/*     return false; */
/*   } else { */
/*     u7_vm_stack_push_i64(&state->stack, x); */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(trunc_f64_to_i32_exec) { */
/*   double x = u7_vm_stack_pop_f64(&state->stack); */
/*   // FIXME(apronchenkov): Make an check exact. */
/*   if (x < (double)INT32_MIN || x > (double)INT32_MAX) { */
/*     state->error = */
/*         u7_errorf(u7_errno_category(), ERANGE, */
/*                   "integer overflow: u7_vm0_trunc_f64_to_i32 x=%f", x); */
/*     return false; */
/*   } else { */
/*     u7_vm_stack_push_i32(&state->stack, x); */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(trunc_f64_to_i64_exec) { */
/*   double x = u7_vm_stack_pop_f64(&state->stack); */
/*   // FIXME(apronchenkov): Make an check exact. */ /*   if (x <
                                                        (double)INT64_MIN || x
                                                        >
                                                        (double)INT64_MAX) {
                                                      */
/*     state->error = */
/*         u7_errorf(u7_errno_category(), ERANGE, */
/*                   "integer overflow: u7_vm0_trunc_f64_to_i64 x=%f", x);
 */
/*     return false; */
/*   } else { */
/*     u7_vm_stack_push_i64(&state->stack, x); */
/*     return true; */
/*   } */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(trunc_f32_to_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(trunc_f32_to_i64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(trunc_f64_to_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(trunc_f64_to_i64) */

/* // print */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(print_i32_exec) { */
/*   printf("%" PRId32, u7_vm_stack_pop_i32(&state->stack)); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(print_i64_exec) { */
/*   printf("%" PRId64, u7_vm_stack_pop_i64(&state->stack)); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(print_f32_exec) { */
/*   printf("%f", u7_vm_stack_pop_f32(&state->stack)); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(print_f64_exec) { */
/*   printf("%f", u7_vm_stack_pop_f64(&state->stack)); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(println_exec) { */
/*   printf("\n"); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(print_i32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(print_i64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(print_f32) */
/* U7_VM0_DEFINE_INSTRUCTION_0(print_f64) */
/* U7_VM0_DEFINE_INSTRUCTION_0(println) */

/* // dump_state */

/* U7_VM0_DEFINE_INSTRUCTION_EXEC(dump_state_exec) { */
/*   printf("ip=%zd  bp=%zd  sp=%zd\n", state->ip, state->stack.base_offset,
 */
/*          state->stack.top_offset); */
/*   return true; */
/* } */

/* U7_VM0_DEFINE_INSTRUCTION_0(dump_state) */
