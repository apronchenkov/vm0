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
    case U7_VM0_ARG_KIND_I64_LABEL:
      return u7_errnof(EINVAL, "%s: incompatible argument kind: %s: label",
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
  struct u7_vm0_instruction result = {
      .arg1 = dst.value,
  };
  if (error->error_code != 0) {
    return result;
  }
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
  struct u7_vm0_instruction result = {
      .arg1 = src.value,
  };
  if (error->error_code != 0) {
    return result;
  }
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
  struct u7_vm0_instruction result = {
      .arg1 = dst.value,
      .arg2 = src.value,
  };
  if (error->error_code != 0) {
    return result;
  }
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
  struct u7_vm0_instruction result = {
      .arg1 = dst.value,
      .arg2 = lhs.value,
      .arg3 = rhs.value,
  };
  if (error->error_code != 0) {
    return result;
  }
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
  struct u7_vm0_instruction result = {
      .arg1 = dst.value,
      .arg2 = lhs.value,
      .arg3 = rhs.value,
  };
  if (error->error_code != 0) {
    return result;
  }
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
  struct u7_vm0_instruction result = {
      .arg1 = dst.value,
      .arg2 = lhs.value,
      .arg3 = rhs.value,
  };
  if (error->error_code != 0) {
    return result;
  }
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
  struct u7_vm0_instruction result = {
      .arg1 = dst.value,
      .arg2 = lhs.value,
      .arg3 = rhs.value,
  };
  if (error->error_code != 0) {
    return result;
  }
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
                                              struct u7_vm0_arg label) {
  struct u7_vm0_instruction result = {
      .arg1 = src.value,
      .arg2 = label.value,
  };
  if (error->error_code != 0) {
    return result;
  }
  if (label.kind != U7_VM0_ARG_KIND_I64_LABEL) {
    *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_jump_if_zero", "label",
                                               label.kind);
    return result;
  }
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
                                                  struct u7_vm0_arg label) {
  struct u7_vm0_instruction result = {
      .arg1 = src.value,
      .arg2 = label.value,
  };
  if (error->error_code != 0) {
    return result;
  }
  if (label.kind != U7_VM0_ARG_KIND_I64_LABEL) {
    *error = u7_vm0_unsupported_arg_kind_error("u7_vm0_jump_if_not_zero",
                                               "label", label.kind);
    return result;
  }
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
