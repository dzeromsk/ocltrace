// Copyright (c) 2016 Dominik Zeromski <dzeromsk@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <CL/opencl.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include "opencl.h"

extern FILE *fout;
extern int emit_time;
extern int emit_time_spent;
extern int emit_stats;

struct emit {
  int name;
  double start_time;
};

enum {
#define F(a, b) a,
  FUNCTIONS(F)
#undef F
  FUNCTIONS_MAX,
};

const char *func_names[] = {
#define F(a, b) #b,
    FUNCTIONS(F)
#undef F
    NULL,
};

void *opencl_fptr[FUNCTIONS_MAX] = {0};
void *trace_fptr[FUNCTIONS_MAX] = {0};
struct stats func_stats[FUNCTIONS_MAX] = {0};

__extension__ const char *error_codes[] = {
        [0] = "CL_SUCCESS",
        [1] = "CL_DEVICE_NOT_FOUND",
        [2] = "CL_DEVICE_NOT_AVAILABLE",
        [3] = "CL_COMPILER_NOT_AVAILABLE",
        [4] = "CL_MEM_OBJECT_ALLOCATION_FAILURE",
        [5] = "CL_OUT_OF_RESOURCES",
        [6] = "CL_OUT_OF_HOST_MEMORY",
        [7] = "CL_PROFILING_INFO_NOT_AVAILABLE",
        [8] = "CL_MEM_COPY_OVERLAP",
        [9] = "CL_IMAGE_FORMAT_MISMATCH",
        [10] = "CL_IMAGE_FORMAT_NOT_SUPPORTED",
        [11] = "CL_BUILD_PROGRAM_FAILURE",
        [12] = "CL_MAP_FAILURE",
        [13] = "CL_MISALIGNED_SUB_BUFFER_OFFSET",
        [14] = "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST",
        [15] = "CL_COMPILE_PROGRAM_FAILURE",
        [16] = "CL_LINKER_NOT_AVAILABLE",
        [17] = "CL_LINK_PROGRAM_FAILURE",
        [18] = "CL_DEVICE_PARTITION_FAILED",
        [19] = "CL_KERNEL_ARG_INFO_NOT_AVAILABLE",
        [30] = "CL_INVALID_VALUE",
        [31] = "CL_INVALID_DEVICE_TYPE",
        [32] = "CL_INVALID_PLATFORM",
        [33] = "CL_INVALID_DEVICE",
        [34] = "CL_INVALID_CONTEXT",
        [35] = "CL_INVALID_QUEUE_PROPERTIES",
        [36] = "CL_INVALID_COMMAND_QUEUE",
        [37] = "CL_INVALID_HOST_PTR",
        [38] = "CL_INVALID_MEM_OBJECT",
        [39] = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
        [40] = "CL_INVALID_IMAGE_SIZE",
        [41] = "CL_INVALID_SAMPLER",
        [42] = "CL_INVALID_BINARY",
        [43] = "CL_INVALID_BUILD_OPTIONS",
        [44] = "CL_INVALID_PROGRAM",
        [45] = "CL_INVALID_PROGRAM_EXECUTABLE",
        [46] = "CL_INVALID_KERNEL_NAME",
        [47] = "CL_INVALID_KERNEL_DEFINITION",
        [48] = "CL_INVALID_KERNEL",
        [49] = "CL_INVALID_ARG_INDEX",
        [50] = "CL_INVALID_ARG_VALUE",
        [51] = "CL_INVALID_ARG_SIZE",
        [52] = "CL_INVALID_KERNEL_ARGS",
        [53] = "CL_INVALID_WORK_DIMENSION",
        [54] = "CL_INVALID_WORK_GROUP_SIZE",
        [55] = "CL_INVALID_WORK_ITEM_SIZE",
        [56] = "CL_INVALID_GLOBAL_OFFSET",
        [57] = "CL_INVALID_EVENT_WAIT_LIST",
        [58] = "CL_INVALID_EVENT",
        [59] = "CL_INVALID_OPERATION",
        [60] = "CL_INVALID_GL_OBJECT",
        [61] = "CL_INVALID_BUFFER_SIZE",
        [62] = "CL_INVALID_MIP_LEVEL",
        [63] = "CL_INVALID_GLOBAL_WORK_SIZE",
        [64] = "CL_INVALID_PROPERTY",
        [65] = "CL_INVALID_IMAGE_DESCRIPTOR",
        [66] = "CL_INVALID_COMPILER_OPTIONS",
        [67] = "CL_INVALID_LINKER_OPTIONS",
        [68] = "CL_INVALID_DEVICE_PARTITION_COUNT",
        [69] = "CL_INVALID_PIPE_SIZE",
        [70] = "CL_INVALID_DEVICE_QUEUE",
};

__extension__ const char *bools[] = {
        [0] = "CL_FALSE", [1] = "CL_TRUE",
};

__extension__ const char *common_string[] = {
        [0x0900] = "CL_PLATFORM_PROFILE",
        [0x0901] = "CL_PLATFORM_VERSION",
        [0x0902] = "CL_PLATFORM_NAME",
        [0x0903] = "CL_PLATFORM_VENDOR",
        [0x0904] = "CL_PLATFORM_EXTENSIONS",
        [0x1000] = "CL_DEVICE_TYPE",
        [0x1001] = "CL_DEVICE_VENDOR_ID",
        [0x1002] = "CL_DEVICE_MAX_COMPUTE_UNITS",
        [0x1003] = "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS",
        [0x1004] = "CL_DEVICE_MAX_WORK_GROUP_SIZE",
        [0x1005] = "CL_DEVICE_MAX_WORK_ITEM_SIZES",
        [0x1006] = "CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR",
        [0x1007] = "CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT",
        [0x1008] = "CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT",
        [0x1009] = "CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG",
        [0x100A] = "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT",
        [0x100B] = "CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE",
        [0x100C] = "CL_DEVICE_MAX_CLOCK_FREQUENCY",
        [0x100D] = "CL_DEVICE_ADDRESS_BITS",
        [0x100E] = "CL_DEVICE_MAX_READ_IMAGE_ARGS",
        [0x100F] = "CL_DEVICE_MAX_WRITE_IMAGE_ARGS",
        [0x1010] = "CL_DEVICE_MAX_MEM_ALLOC_SIZE",
        [0x1011] = "CL_DEVICE_IMAGE2D_MAX_WIDTH",
        [0x1012] = "CL_DEVICE_IMAGE2D_MAX_HEIGHT",
        [0x1013] = "CL_DEVICE_IMAGE3D_MAX_WIDTH",
        [0x1014] = "CL_DEVICE_IMAGE3D_MAX_HEIGHT",
        [0x1015] = "CL_DEVICE_IMAGE3D_MAX_DEPTH",
        [0x1016] = "CL_DEVICE_IMAGE_SUPPORT",
        [0x1017] = "CL_DEVICE_MAX_PARAMETER_SIZE",
        [0x1018] = "CL_DEVICE_MAX_SAMPLERS",
        [0x1019] = "CL_DEVICE_MEM_BASE_ADDR_ALIGN",
        [0x101A] = "CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE",
        [0x101B] = "CL_DEVICE_SINGLE_FP_CONFIG",
        [0x101C] = "CL_DEVICE_GLOBAL_MEM_CACHE_TYPE",
        [0x101D] = "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE",
        [0x101E] = "CL_DEVICE_GLOBAL_MEM_CACHE_SIZE",
        [0x101F] = "CL_DEVICE_GLOBAL_MEM_SIZE",
        [0x1020] = "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE",
        [0x1021] = "CL_DEVICE_MAX_CONSTANT_ARGS",
        [0x1022] = "CL_DEVICE_LOCAL_MEM_TYPE",
        [0x1023] = "CL_DEVICE_LOCAL_MEM_SIZE",
        [0x1024] = "CL_DEVICE_ERROR_CORRECTION_SUPPORT",
        [0x1025] = "CL_DEVICE_PROFILING_TIMER_RESOLUTION",
        [0x1026] = "CL_DEVICE_ENDIAN_LITTLE",
        [0x1027] = "CL_DEVICE_AVAILABLE",
        [0x1028] = "CL_DEVICE_COMPILER_AVAILABLE",
        [0x1029] = "CL_DEVICE_EXECUTION_CAPABILITIES",
        [0x102A] = "CL_DEVICE_QUEUE_PROPERTIES",
        [0x102A] = "CL_DEVICE_QUEUE_ON_HOST_PROPERTIES",
        [0x102B] = "CL_DEVICE_NAME",
        [0x102C] = "CL_DEVICE_VENDOR",
        [0x102D] = "CL_DRIVER_VERSION",
        [0x102E] = "CL_DEVICE_PROFILE",
        [0x102F] = "CL_DEVICE_VERSION",
        [0x1030] = "CL_DEVICE_EXTENSIONS",
        [0x1031] = "CL_DEVICE_PLATFORM",
        [0x1032] = "CL_DEVICE_DOUBLE_FP_CONFIG",
        [0x1034] = "CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF",
        [0x1035] = "CL_DEVICE_HOST_UNIFIED_MEMORY",
        [0x1036] = "CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR",
        [0x1037] = "CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT",
        [0x1038] = "CL_DEVICE_NATIVE_VECTOR_WIDTH_INT",
        [0x1039] = "CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG",
        [0x103A] = "CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT",
        [0x103B] = "CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE",
        [0x103C] = "CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF",
        [0x103D] = "CL_DEVICE_OPENCL_C_VERSION",
        [0x103E] = "CL_DEVICE_LINKER_AVAILABLE",
        [0x103F] = "CL_DEVICE_BUILT_IN_KERNELS",
        [0x1040] = "CL_DEVICE_IMAGE_MAX_BUFFER_SIZE",
        [0x1041] = "CL_DEVICE_IMAGE_MAX_ARRAY_SIZE",
        [0x1042] = "CL_DEVICE_PARENT_DEVICE",
        [0x1043] = "CL_DEVICE_PARTITION_MAX_SUB_DEVICES",
        [0x1044] = "CL_DEVICE_PARTITION_PROPERTIES",
        [0x1045] = "CL_DEVICE_PARTITION_AFFINITY_DOMAIN",
        [0x1046] = "CL_DEVICE_PARTITION_TYPE",
        [0x1047] = "CL_DEVICE_REFERENCE_COUNT",
        [0x1048] = "CL_DEVICE_PREFERRED_INTEROP_USER_SYNC",
        [0x1049] = "CL_DEVICE_PRINTF_BUFFER_SIZE",
        [0x104A] = "CL_DEVICE_IMAGE_PITCH_ALIGNMENT",
        [0x104B] = "CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT",
        [0x104C] = "CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS",
        [0x104D] = "CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE",
        [0x104E] = "CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES",
        [0x104F] = "CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE",
        [0x1050] = "CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE",
        [0x1051] = "CL_DEVICE_MAX_ON_DEVICE_QUEUES",
        [0x1052] = "CL_DEVICE_MAX_ON_DEVICE_EVENTS",
        [0x1053] = "CL_DEVICE_SVM_CAPABILITIES",
        [0x1054] = "CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE",
        [0x1055] = "CL_DEVICE_MAX_PIPE_ARGS",
        [0x1056] = "CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS",
        [0x1057] = "CL_DEVICE_PIPE_MAX_PACKET_SIZE",
        [0x1058] = "CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT",
        [0x1059] = "CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT",
        [0x105A] = "CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT",
        [0x1080] = "CL_CONTEXT_REFERENCE_COUNT",
        [0x1081] = "CL_CONTEXT_DEVICES",
        [0x1082] = "CL_CONTEXT_PROPERTIES",
        [0x1083] = "CL_CONTEXT_NUM_DEVICES",
        [0x1084] = "CL_CONTEXT_PLATFORM",
        [0x1085] = "CL_CONTEXT_INTEROP_USER_SYNC",
        [0x1086] = "CL_DEVICE_PARTITION_EQUALLY",
        [0x1087] = "CL_DEVICE_PARTITION_BY_COUNTS",
        [0x1088] = "CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN",
        [0x1090] = "CL_QUEUE_CONTEXT",
        [0x1091] = "CL_QUEUE_DEVICE",
        [0x1092] = "CL_QUEUE_REFERENCE_COUNT",
        [0x1093] = "CL_QUEUE_PROPERTIES",
        [0x1094] = "CL_QUEUE_SIZE",
        [0x10B0] = "CL_R",
        [0x10B1] = "CL_A",
        [0x10B2] = "CL_RG",
        [0x10B3] = "CL_RA",
        [0x10B4] = "CL_RGB",
        [0x10B5] = "CL_RGBA",
        [0x10B6] = "CL_BGRA",
        [0x10B7] = "CL_ARGB",
        [0x10B8] = "CL_INTENSITY",
        [0x10B9] = "CL_LUMINANCE",
        [0x10BA] = "CL_Rx",
        [0x10BB] = "CL_RGx",
        [0x10BC] = "CL_RGBx",
        [0x10BD] = "CL_DEPTH",
        [0x10BE] = "CL_DEPTH_STENCIL",
        [0x10BF] = "CL_sRGB",
        [0x10C0] = "CL_sRGBx",
        [0x10C1] = "CL_sRGBA",
        [0x10C2] = "CL_sBGRA",
        [0x10C3] = "CL_ABGR",
        [0x10D0] = "CL_SNORM_INT8",
        [0x10D1] = "CL_SNORM_INT16",
        [0x10D2] = "CL_UNORM_INT8",
        [0x10D3] = "CL_UNORM_INT16",
        [0x10D4] = "CL_UNORM_SHORT_565",
        [0x10D5] = "CL_UNORM_SHORT_555",
        [0x10D6] = "CL_UNORM_INT_101010",
        [0x10D7] = "CL_SIGNED_INT8",
        [0x10D8] = "CL_SIGNED_INT16",
        [0x10D9] = "CL_SIGNED_INT32",
        [0x10DA] = "CL_UNSIGNED_INT8",
        [0x10DB] = "CL_UNSIGNED_INT16",
        [0x10DC] = "CL_UNSIGNED_INT32",
        [0x10DD] = "CL_HALF_FLOAT",
        [0x10DE] = "CL_FLOAT",
        [0x10DF] = "CL_UNORM_INT24",
        [0x10F0] = "CL_MEM_OBJECT_BUFFER",
        [0x10F1] = "CL_MEM_OBJECT_IMAGE2D",
        [0x10F2] = "CL_MEM_OBJECT_IMAGE3D",
        [0x10F3] = "CL_MEM_OBJECT_IMAGE2D_ARRAY",
        [0x10F4] = "CL_MEM_OBJECT_IMAGE1D",
        [0x10F5] = "CL_MEM_OBJECT_IMAGE1D_ARRAY",
        [0x10F6] = "CL_MEM_OBJECT_IMAGE1D_BUFFER",
        [0x10F7] = "CL_MEM_OBJECT_PIPE",
        [0x1100] = "CL_MEM_TYPE",
        [0x1101] = "CL_MEM_FLAGS",
        [0x1102] = "CL_MEM_SIZE",
        [0x1103] = "CL_MEM_HOST_PTR",
        [0x1104] = "CL_MEM_MAP_COUNT",
        [0x1105] = "CL_MEM_REFERENCE_COUNT",
        [0x1106] = "CL_MEM_CONTEXT",
        [0x1107] = "CL_MEM_ASSOCIATED_MEMOBJECT",
        [0x1108] = "CL_MEM_OFFSET",
        [0x1109] = "CL_MEM_USES_SVM_POINTER",
        [0x1110] = "CL_IMAGE_FORMAT",
        [0x1111] = "CL_IMAGE_ELEMENT_SIZE",
        [0x1112] = "CL_IMAGE_ROW_PITCH",
        [0x1113] = "CL_IMAGE_SLICE_PITCH",
        [0x1114] = "CL_IMAGE_WIDTH",
        [0x1115] = "CL_IMAGE_HEIGHT",
        [0x1116] = "CL_IMAGE_DEPTH",
        [0x1117] = "CL_IMAGE_ARRAY_SIZE",
        [0x1118] = "CL_IMAGE_BUFFER",
        [0x1119] = "CL_IMAGE_NUM_MIP_LEVELS",
        [0x111A] = "CL_IMAGE_NUM_SAMPLES",
        [0x1120] = "CL_PIPE_PACKET_SIZE",
        [0x1121] = "CL_PIPE_MAX_PACKETS",
        [0x1130] = "CL_ADDRESS_NONE",
        [0x1131] = "CL_ADDRESS_CLAMP_TO_EDGE",
        [0x1132] = "CL_ADDRESS_CLAMP",
        [0x1133] = "CL_ADDRESS_REPEAT",
        [0x1134] = "CL_ADDRESS_MIRRORED_REPEAT",
        [0x1140] = "CL_FILTER_NEAREST",
        [0x1141] = "CL_FILTER_LINEAR",
        [0x1150] = "CL_SAMPLER_REFERENCE_COUNT",
        [0x1151] = "CL_SAMPLER_CONTEXT",
        [0x1152] = "CL_SAMPLER_NORMALIZED_COORDS",
        [0x1153] = "CL_SAMPLER_ADDRESSING_MODE",
        [0x1154] = "CL_SAMPLER_FILTER_MODE",
        [0x1155] = "CL_SAMPLER_MIP_FILTER_MODE",
        [0x1156] = "CL_SAMPLER_LOD_MIN",
        [0x1157] = "CL_SAMPLER_LOD_MAX",
        [0x1160] = "CL_PROGRAM_REFERENCE_COUNT",
        [0x1161] = "CL_PROGRAM_CONTEXT",
        [0x1162] = "CL_PROGRAM_NUM_DEVICES",
        [0x1163] = "CL_PROGRAM_DEVICES",
        [0x1164] = "CL_PROGRAM_SOURCE",
        [0x1165] = "CL_PROGRAM_BINARY_SIZES",
        [0x1166] = "CL_PROGRAM_BINARIES",
        [0x1167] = "CL_PROGRAM_NUM_KERNELS",
        [0x1168] = "CL_PROGRAM_KERNEL_NAMES",
        [0x1181] = "CL_PROGRAM_BUILD_STATUS",
        [0x1182] = "CL_PROGRAM_BUILD_OPTIONS",
        [0x1183] = "CL_PROGRAM_BUILD_LOG",
        [0x1184] = "CL_PROGRAM_BINARY_TYPE",
        [0x1185] = "CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE",
        [0x1190] = "CL_KERNEL_FUNCTION_NAME",
        [0x1191] = "CL_KERNEL_NUM_ARGS",
        [0x1192] = "CL_KERNEL_REFERENCE_COUNT",
        [0x1193] = "CL_KERNEL_CONTEXT",
        [0x1194] = "CL_KERNEL_PROGRAM",
        [0x1195] = "CL_KERNEL_ATTRIBUTES",
        [0x1196] = "CL_KERNEL_ARG_ADDRESS_QUALIFIER",
        [0x1197] = "CL_KERNEL_ARG_ACCESS_QUALIFIER",
        [0x1198] = "CL_KERNEL_ARG_TYPE_NAME",
        [0x1199] = "CL_KERNEL_ARG_TYPE_QUALIFIER",
        [0x119A] = "CL_KERNEL_ARG_NAME",
        [0x119B] = "CL_KERNEL_ARG_ADDRESS_GLOBAL",
        [0x119C] = "CL_KERNEL_ARG_ADDRESS_LOCAL",
        [0x119D] = "CL_KERNEL_ARG_ADDRESS_CONSTANT",
        [0x119E] = "CL_KERNEL_ARG_ADDRESS_PRIVATE",
        [0x11A0] = "CL_KERNEL_ARG_ACCESS_READ_ONLY",
        [0x11A1] = "CL_KERNEL_ARG_ACCESS_WRITE_ONLY",
        [0x11A2] = "CL_KERNEL_ARG_ACCESS_READ_WRITE",
        [0x11A3] = "CL_KERNEL_ARG_ACCESS_NONE",
        [0x11B0] = "CL_KERNEL_WORK_GROUP_SIZE",
        [0x11B1] = "CL_KERNEL_COMPILE_WORK_GROUP_SIZE",
        [0x11B2] = "CL_KERNEL_LOCAL_MEM_SIZE",
        [0x11B3] = "CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE",
        [0x11B4] = "CL_KERNEL_PRIVATE_MEM_SIZE",
        [0x11B5] = "CL_KERNEL_GLOBAL_WORK_SIZE",
        [0x11B6] = "CL_KERNEL_EXEC_INFO_SVM_PTRS",
        [0x11B7] = "CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM",
        [0x11D0] = "CL_EVENT_COMMAND_QUEUE",
        [0x11D1] = "CL_EVENT_COMMAND_TYPE",
        [0x11D2] = "CL_EVENT_REFERENCE_COUNT",
        [0x11D3] = "CL_EVENT_COMMAND_EXECUTION_STATUS",
        [0x11D4] = "CL_EVENT_CONTEXT",
        [0x11F0] = "CL_COMMAND_NDRANGE_KERNEL",
        [0x11F1] = "CL_COMMAND_TASK",
        [0x11F2] = "CL_COMMAND_NATIVE_KERNEL",
        [0x11F3] = "CL_COMMAND_READ_BUFFER",
        [0x11F4] = "CL_COMMAND_WRITE_BUFFER",
        [0x11F5] = "CL_COMMAND_COPY_BUFFER",
        [0x11F6] = "CL_COMMAND_READ_IMAGE",
        [0x11F7] = "CL_COMMAND_WRITE_IMAGE",
        [0x11F8] = "CL_COMMAND_COPY_IMAGE",
        [0x11F9] = "CL_COMMAND_COPY_IMAGE_TO_BUFFER",
        [0x11FA] = "CL_COMMAND_COPY_BUFFER_TO_IMAGE",
        [0x11FB] = "CL_COMMAND_MAP_BUFFER",
        [0x11FC] = "CL_COMMAND_MAP_IMAGE",
        [0x11FD] = "CL_COMMAND_UNMAP_MEM_OBJECT",
        [0x11FE] = "CL_COMMAND_MARKER",
        [0x11FF] = "CL_COMMAND_ACQUIRE_GL_OBJECTS",
        [0x1200] = "CL_COMMAND_RELEASE_GL_OBJECTS",
        [0x1201] = "CL_COMMAND_READ_BUFFER_RECT",
        [0x1202] = "CL_COMMAND_WRITE_BUFFER_RECT",
        [0x1203] = "CL_COMMAND_COPY_BUFFER_RECT",
        [0x1204] = "CL_COMMAND_USER",
        [0x1205] = "CL_COMMAND_BARRIER",
        [0x1206] = "CL_COMMAND_MIGRATE_MEM_OBJECTS",
        [0x1207] = "CL_COMMAND_FILL_BUFFER",
        [0x1208] = "CL_COMMAND_FILL_IMAGE",
        [0x1209] = "CL_COMMAND_SVM_FREE",
        [0x120A] = "CL_COMMAND_SVM_MEMCPY",
        [0x120B] = "CL_COMMAND_SVM_MEMFILL",
        [0x120C] = "CL_COMMAND_SVM_MAP",
        [0x120D] = "CL_COMMAND_SVM_UNMAP",
        [0x1220] = "CL_BUFFER_CREATE_TYPE_REGION",
        [0x1280] = "CL_PROFILING_COMMAND_QUEUED",
        [0x1281] = "CL_PROFILING_COMMAND_SUBMIT",
        [0x1282] = "CL_PROFILING_COMMAND_START",
        [0x1283] = "CL_PROFILING_COMMAND_END",
        [0x1284] = "CL_PROFILING_COMMAND_COMPLETE",
};

__extension__ const char *memory_flags[] = {
        [0] = "CL_MEM_READ_WRITE",             [1] = "CL_MEM_WRITE_ONLY",
        [2] = "CL_MEM_READ_ONLY",              [3] = "CL_MEM_USE_HOST_PTR",
        [4] = "CL_MEM_ALLOC_HOST_PTR",         [5] = "CL_MEM_COPY_HOST_PTR",
        [6] = "reserved",                      [7] = "CL_MEM_HOST_WRITE_ONLY",
        [8] = "CL_MEM_HOST_READ_ONLY",         [9] = "CL_MEM_HOST_NO_ACCESS",
        [10] = "CL_MEM_SVM_FINE_GRAIN_BUFFER", [11] = "CL_MEM_SVM_ATOMICS",
        [12] = "CL_MEM_KERNEL_READ_AND_WRITE",
};

// print functions

inline double timetodouble(const struct timespec a) {
  return ((double)a.tv_sec + (a.tv_nsec / 1000000000.0));
}

inline void emit_open(struct emit *e, int name) {
  if (emit_time) {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    fprintf(fout, "%5f ", timetodouble(t));
  }
  fputs(func_names[name], fout);
  fputs("(", fout);

  if (emit_time_spent || emit_stats) {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    e->start_time = timetodouble(t);
  }

  if (emit_stats) {
    e->name = name;
    func_stats[name].name = name;
    func_stats[name].calls++;
  }
}

inline void emit_separator(struct emit *e) { fputs(", ", fout); }

inline void emit_close(struct emit *e) {
  fputs(")", fout);
  fflush(fout);
}

inline void emit_ret(struct emit *e) { fputs(" = ", fout); }

inline void emit_end(struct emit *e) {
  double diff = 0;
  if (emit_time_spent || emit_stats) {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    diff = timetodouble(t) - e->start_time;
  }
  if (emit_time_spent) {
    fputs(" <", fout);
    fprintf(fout, "%5f", diff);
    fputs(">", fout);
  }
  if (emit_stats) {
    func_stats[e->name].seconds += diff;
  }
  fputs("\n", fout);
  fflush(fout);
}

inline void emit_flush(struct emit *e) { fflush(fout); }

inline void emit_char_ptr(struct emit *e, const char *x) {
  fprintf(fout, "\"%s\"", x);
}

inline void emit_cl_addressing_mode(struct emit *e,
                                    const cl_addressing_mode x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_bool(struct emit *e, const cl_bool x) {
  fputs(bools[x], fout);
}

inline void emit_cl_buffer_create_type(struct emit *e,
                                       const cl_buffer_create_type x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_command_queue(struct emit *e, const cl_command_queue x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_command_queue_info(struct emit *e,
                                       const cl_command_queue_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_command_queue_properties(
    struct emit *e, const cl_command_queue_properties x) {
  fprintf(fout, "%d", x);
}

inline void emit_cl_context(struct emit *e, const cl_context x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_context_info(struct emit *e, const cl_context_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_context_properties_ptr(struct emit *e,
                                           const cl_context_properties *x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_device_id(struct emit *e, const cl_device_id x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_device_id_ptr(struct emit *e, const cl_device_id *x) {
  if (x != NULL) {
    fputs("{", fout);
    fprintf(fout, "%p", *x);
    fputs("}", fout);
  } else
    fputs("NULL", fout);
}

inline void emit_cl_device_info(struct emit *e, const cl_device_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_device_partition_property_ptr(
    struct emit *e, const cl_device_partition_property *x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_device_type(struct emit *e, const cl_device_type x) {
  fprintf(fout, "%d", x);
}

inline void emit_cl_event(struct emit *e, const cl_event x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_event_info(struct emit *e, const cl_event_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_event_ptr(struct emit *e, const cl_event *x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_filter_mode(struct emit *e, const cl_filter_mode x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_image_desc_ptr(struct emit *e, const cl_image_desc *x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_image_format_ptr(struct emit *e, const cl_image_format *x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_image_info(struct emit *e, const cl_image_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_int(struct emit *e, const cl_int x) {
  fputs(error_codes[-x], fout);
  if (emit_stats && x != 0) {
    func_stats[e->name].errors++;
  }
}

inline void emit_cl_int_ptr(struct emit *e, const cl_int *x) {
  if (x != NULL) {
    fputs("{", fout);
    fputs(error_codes[-*x], fout);
    fputs("}", fout);
    if (emit_stats && *x != 0) {
      func_stats[e->name].errors++;
    }
  } else {
    fputs("NULL", fout);
  }
}

inline void emit_cl_kernel_arg_info(struct emit *e,
                                    const cl_kernel_arg_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_kernel(struct emit *e, const cl_kernel x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_kernel_exec_info(struct emit *e,
                                     const cl_kernel_exec_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_kernel_info(struct emit *e, const cl_kernel_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_kernel_ptr(struct emit *e, const cl_kernel *x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_kernel_work_group_info(struct emit *e,
                                           const cl_kernel_work_group_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_map_flags(struct emit *e, const cl_map_flags x) {
  fprintf(fout, "%d", x);
}

inline void emit_cl_mem(struct emit *e, const cl_mem x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_mem_flags(struct emit *e, const cl_mem_flags x) {
  int i;
  for (i = 0; i < sizeof(memory_flags) / sizeof(memory_flags[0]); i++) {
    if (x & (1 << i)) {
      fputs(memory_flags[i], fout);
    }
  }
}

inline void emit_cl_mem_info(struct emit *e, const cl_mem_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_mem_migration_flags(struct emit *e,
                                        const cl_mem_migration_flags x) {
  fprintf(fout, "%d", x);
}

inline void emit_cl_mem_object_type(struct emit *e,
                                    const cl_mem_object_type x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_mem_ptr(struct emit *e, const cl_mem *x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_pipe_info(struct emit *e, const cl_pipe_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_pipe_properties_ptr(struct emit *e,
                                        const cl_pipe_properties *x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_platform_id(struct emit *e, const cl_platform_id x) {
  fprintf(fout, "%d", x);
}

inline void emit_cl_platform_id_ptr(struct emit *e, const cl_platform_id *x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_platform_info(struct emit *e, const cl_platform_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_profiling_info(struct emit *e, const cl_profiling_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_program_build_info(struct emit *e,
                                       const cl_program_build_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_program(struct emit *e, const cl_program x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_program_info(struct emit *e, const cl_program_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_program_ptr(struct emit *e, const cl_program *x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_queue_properties_ptr(struct emit *e,
                                         const cl_queue_properties *x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_sampler(struct emit *e, const cl_sampler x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_sampler_info(struct emit *e, const cl_sampler_info x) {
  fputs(common_string[x], fout);
}

inline void emit_cl_sampler_properties_ptr(struct emit *e,
                                           const cl_sampler_properties *x) {
  fprintf(fout, "%p", x);
}

inline void emit_cl_svm_mem_flags(struct emit *e, const cl_svm_mem_flags x) {
  fprintf(fout, "%d", x);
}

inline void emit_cl_uint(struct emit *e, const cl_uint x) {
  fprintf(fout, "%d", x);
}

inline void emit_cl_uint_ptr(struct emit *e, const cl_uint *x) {
  fprintf(fout, "%p", x);
}

inline void emit_size_t(struct emit *e, const size_t x) {
  fprintf(fout, "%d", x);
}

inline void emit_size_t_ptr(struct emit *e, const size_t *x) {
  if (x != NULL)
    fprintf(fout, "{%d}", *x);
  else
    fputs("NULL", fout);
}

inline void emit_void_ptr(struct emit *e, const void *x) {
  fprintf(fout, "%p", x);
}

// opencl api

cl_int clGetPlatformIDs(cl_uint _0, cl_platform_id *_1, cl_uint *_2) {
  struct emit e;
  emit_open(&e, CL_GET_PLATFORM_I_DS);
  emit_cl_uint(&e, _0);
  emit_separator(&e);
  emit_cl_platform_id_ptr(&e, _1);
  emit_separator(&e);
  emit_cl_uint_ptr(&e, _2);
  emit_close(&e);
  cl_int ret =
      ((cl_get_platform_i_ds)opencl_fptr[CL_GET_PLATFORM_I_DS])(_0, _1, _2);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetPlatformInfo(cl_platform_id _0, cl_platform_info _1, size_t _2,
                         void *_3, size_t *_4) {
  struct emit e;
  emit_open(&e, CL_GET_PLATFORM_INFO);
  emit_cl_platform_id(&e, _0);
  emit_separator(&e);
  emit_cl_platform_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_close(&e);
  cl_int ret = ((cl_get_platform_info)opencl_fptr[CL_GET_PLATFORM_INFO])(
      _0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetDeviceIDs(cl_platform_id _0, cl_device_type _1, cl_uint _2,
                      cl_device_id *_3, cl_uint *_4) {
  struct emit e;
  emit_open(&e, CL_GET_DEVICE_I_DS);
  emit_cl_platform_id(&e, _0);
  emit_separator(&e);
  emit_cl_device_type(&e, _1);
  emit_separator(&e);
  emit_cl_uint(&e, _2);
  emit_separator(&e);
  emit_cl_device_id_ptr(&e, _3);
  emit_separator(&e);
  emit_cl_uint_ptr(&e, _4);
  emit_close(&e);
  cl_int ret =
      ((cl_get_device_i_ds)opencl_fptr[CL_GET_DEVICE_I_DS])(_0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetDeviceInfo(cl_device_id _0, cl_device_info _1, size_t _2, void *_3,
                       size_t *_4) {
  struct emit e;
  emit_open(&e, CL_GET_DEVICE_INFO);
  emit_cl_device_id(&e, _0);
  emit_separator(&e);
  emit_cl_device_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_close(&e);
  cl_int ret =
      ((cl_get_device_info)opencl_fptr[CL_GET_DEVICE_INFO])(_0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clCreateSubDevices(cl_device_id _0,
                          const cl_device_partition_property *_1, cl_uint _2,
                          cl_device_id *_3, cl_uint *_4) {
  struct emit e;
  emit_open(&e, CL_CREATE_SUB_DEVICES);
  emit_cl_device_id(&e, _0);
  emit_separator(&e);
  emit_cl_device_partition_property_ptr(&e, _1);
  emit_separator(&e);
  emit_cl_uint(&e, _2);
  emit_separator(&e);
  emit_cl_device_id_ptr(&e, _3);
  emit_separator(&e);
  emit_cl_uint_ptr(&e, _4);
  emit_close(&e);
  cl_int ret = ((cl_create_sub_devices)opencl_fptr[CL_CREATE_SUB_DEVICES])(
      _0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clRetainDevice(cl_device_id _0) {
  struct emit e;
  emit_open(&e, CL_RETAIN_DEVICE);
  emit_cl_device_id(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_retain_device)opencl_fptr[CL_RETAIN_DEVICE])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clReleaseDevice(cl_device_id _0) {
  struct emit e;
  emit_open(&e, CL_RELEASE_DEVICE);
  emit_cl_device_id(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_release_device)opencl_fptr[CL_RELEASE_DEVICE])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_context clCreateContext(const cl_context_properties *_0, cl_uint _1,
                           const cl_device_id *_2,
                           void (*_3)(const char *, const void *, size_t,
                                      void *),
                           void *_4, cl_int *_5) {
  struct emit e;
  emit_open(&e, CL_CREATE_CONTEXT);
  emit_cl_context_properties_ptr(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_cl_device_id_ptr(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_void_ptr(&e, _4);
  emit_separator(&e);
  emit_flush(&e);
  cl_context ret = ((cl_create_context)opencl_fptr[CL_CREATE_CONTEXT])(
      _0, _1, _2, _3, _4, _5);
  emit_cl_int_ptr(&e, _5);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_context(&e, ret);
  emit_end(&e);
  return ret;
}

cl_context clCreateContextFromType(const cl_context_properties *_0,
                                   cl_device_type _1,
                                   void (*_2)(const char *, const void *,
                                              size_t, void *),
                                   void *_3, cl_int *_4) {
  struct emit e;
  emit_open(&e, CL_CREATE_CONTEXT_FROM_TYPE);
  emit_cl_context_properties_ptr(&e, _0);
  emit_separator(&e);
  emit_cl_device_type(&e, _1);
  emit_separator(&e);
  emit_void_ptr(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_flush(&e);
  cl_context ret =
      ((cl_create_context_from_type)opencl_fptr[CL_CREATE_CONTEXT_FROM_TYPE])(
          _0, _1, _2, _3, _4);
  emit_cl_int_ptr(&e, _4);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_context(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clRetainContext(cl_context _0) {
  struct emit e;
  emit_open(&e, CL_RETAIN_CONTEXT);
  emit_cl_context(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_retain_context)opencl_fptr[CL_RETAIN_CONTEXT])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clReleaseContext(cl_context _0) {
  struct emit e;
  emit_open(&e, CL_RELEASE_CONTEXT);
  emit_cl_context(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_release_context)opencl_fptr[CL_RELEASE_CONTEXT])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetContextInfo(cl_context _0, cl_context_info _1, size_t _2, void *_3,
                        size_t *_4) {
  struct emit e;
  emit_open(&e, CL_GET_CONTEXT_INFO);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_context_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_close(&e);
  cl_int ret = ((cl_get_context_info)opencl_fptr[CL_GET_CONTEXT_INFO])(
      _0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_command_queue clCreateCommandQueueWithProperties(
    cl_context _0, cl_device_id _1, const cl_queue_properties *_2, cl_int *_3) {
  struct emit e;
  emit_open(&e, CL_CREATE_COMMAND_QUEUE_WITH_PROPERTIES);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_device_id(&e, _1);
  emit_separator(&e);
  emit_cl_queue_properties_ptr(&e, _2);
  emit_separator(&e);
  emit_flush(&e);
  cl_command_queue ret =
      ((cl_create_command_queue_with_properties)
       opencl_fptr[CL_CREATE_COMMAND_QUEUE_WITH_PROPERTIES])(_0, _1, _2, _3);
  emit_cl_int_ptr(&e, _3);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_command_queue(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clRetainCommandQueue(cl_command_queue _0) {
  struct emit e;
  emit_open(&e, CL_RETAIN_COMMAND_QUEUE);
  emit_cl_command_queue(&e, _0);
  emit_close(&e);
  cl_int ret =
      ((cl_retain_command_queue)opencl_fptr[CL_RETAIN_COMMAND_QUEUE])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clReleaseCommandQueue(cl_command_queue _0) {
  struct emit e;
  emit_open(&e, CL_RELEASE_COMMAND_QUEUE);
  emit_cl_command_queue(&e, _0);
  emit_close(&e);
  cl_int ret =
      ((cl_release_command_queue)opencl_fptr[CL_RELEASE_COMMAND_QUEUE])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetCommandQueueInfo(cl_command_queue _0, cl_command_queue_info _1,
                             size_t _2, void *_3, size_t *_4) {
  struct emit e;
  emit_open(&e, CL_GET_COMMAND_QUEUE_INFO);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_command_queue_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_close(&e);
  cl_int ret =
      ((cl_get_command_queue_info)opencl_fptr[CL_GET_COMMAND_QUEUE_INFO])(
          _0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_mem clCreateBuffer(cl_context _0, cl_mem_flags _1, size_t _2, void *_3,
                      cl_int *_4) {
  struct emit e;
  emit_open(&e, CL_CREATE_BUFFER);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_mem_flags(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_flush(&e);
  cl_mem ret =
      ((cl_create_buffer)opencl_fptr[CL_CREATE_BUFFER])(_0, _1, _2, _3, _4);
  emit_cl_int_ptr(&e, _4);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_mem(&e, ret);
  emit_end(&e);
  return ret;
}

cl_mem clCreateSubBuffer(cl_mem _0, cl_mem_flags _1, cl_buffer_create_type _2,
                         const void *_3, cl_int *_4) {
  struct emit e;
  emit_open(&e, CL_CREATE_SUB_BUFFER);
  emit_cl_mem(&e, _0);
  emit_separator(&e);
  emit_cl_mem_flags(&e, _1);
  emit_separator(&e);
  emit_cl_buffer_create_type(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_flush(&e);
  cl_mem ret = ((cl_create_sub_buffer)opencl_fptr[CL_CREATE_SUB_BUFFER])(
      _0, _1, _2, _3, _4);
  emit_cl_int_ptr(&e, _4);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_mem(&e, ret);
  emit_end(&e);
  return ret;
}

cl_mem clCreateImage(cl_context _0, cl_mem_flags _1, const cl_image_format *_2,
                     const cl_image_desc *_3, void *_4, cl_int *_5) {
  struct emit e;
  emit_open(&e, CL_CREATE_IMAGE);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_mem_flags(&e, _1);
  emit_separator(&e);
  emit_cl_image_format_ptr(&e, _2);
  emit_separator(&e);
  emit_cl_image_desc_ptr(&e, _3);
  emit_separator(&e);
  emit_void_ptr(&e, _4);
  emit_separator(&e);
  emit_flush(&e);
  cl_mem ret =
      ((cl_create_image)opencl_fptr[CL_CREATE_IMAGE])(_0, _1, _2, _3, _4, _5);
  emit_cl_int_ptr(&e, _5);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_mem(&e, ret);
  emit_end(&e);
  return ret;
}

cl_mem clCreatePipe(cl_context _0, cl_mem_flags _1, cl_uint _2, cl_uint _3,
                    const cl_pipe_properties *_4, cl_int *_5) {
  struct emit e;
  emit_open(&e, CL_CREATE_PIPE);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_mem_flags(&e, _1);
  emit_separator(&e);
  emit_cl_uint(&e, _2);
  emit_separator(&e);
  emit_cl_uint(&e, _3);
  emit_separator(&e);
  emit_cl_pipe_properties_ptr(&e, _4);
  emit_separator(&e);
  emit_flush(&e);
  cl_mem ret =
      ((cl_create_pipe)opencl_fptr[CL_CREATE_PIPE])(_0, _1, _2, _3, _4, _5);
  emit_cl_int_ptr(&e, _5);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_mem(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clRetainMemObject(cl_mem _0) {
  struct emit e;
  emit_open(&e, CL_RETAIN_MEM_OBJECT);
  emit_cl_mem(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_retain_mem_object)opencl_fptr[CL_RETAIN_MEM_OBJECT])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clReleaseMemObject(cl_mem _0) {
  struct emit e;
  emit_open(&e, CL_RELEASE_MEM_OBJECT);
  emit_cl_mem(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_release_mem_object)opencl_fptr[CL_RELEASE_MEM_OBJECT])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetSupportedImageFormats(cl_context _0, cl_mem_flags _1,
                                  cl_mem_object_type _2, cl_uint _3,
                                  cl_image_format *_4, cl_uint *_5) {
  struct emit e;
  emit_open(&e, CL_GET_SUPPORTED_IMAGE_FORMATS);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_mem_flags(&e, _1);
  emit_separator(&e);
  emit_cl_mem_object_type(&e, _2);
  emit_separator(&e);
  emit_cl_uint(&e, _3);
  emit_separator(&e);
  emit_cl_image_format_ptr(&e, _4);
  emit_separator(&e);
  emit_cl_uint_ptr(&e, _5);
  emit_close(&e);
  cl_int ret =
      ((cl_get_supported_image_formats)
       opencl_fptr[CL_GET_SUPPORTED_IMAGE_FORMATS])(_0, _1, _2, _3, _4, _5);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetMemObjectInfo(cl_mem _0, cl_mem_info _1, size_t _2, void *_3,
                          size_t *_4) {
  struct emit e;
  emit_open(&e, CL_GET_MEM_OBJECT_INFO);
  emit_cl_mem(&e, _0);
  emit_separator(&e);
  emit_cl_mem_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_close(&e);
  cl_int ret = ((cl_get_mem_object_info)opencl_fptr[CL_GET_MEM_OBJECT_INFO])(
      _0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetImageInfo(cl_mem _0, cl_image_info _1, size_t _2, void *_3,
                      size_t *_4) {
  struct emit e;
  emit_open(&e, CL_GET_IMAGE_INFO);
  emit_cl_mem(&e, _0);
  emit_separator(&e);
  emit_cl_image_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_close(&e);
  cl_int ret =
      ((cl_get_image_info)opencl_fptr[CL_GET_IMAGE_INFO])(_0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetPipeInfo(cl_mem _0, cl_pipe_info _1, size_t _2, void *_3,
                     size_t *_4) {
  struct emit e;
  emit_open(&e, CL_GET_PIPE_INFO);
  emit_cl_mem(&e, _0);
  emit_separator(&e);
  emit_cl_pipe_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_close(&e);
  cl_int ret =
      ((cl_get_pipe_info)opencl_fptr[CL_GET_PIPE_INFO])(_0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clSetMemObjectDestructorCallback(cl_mem _0, void (*_1)(cl_mem, void *),
                                        void *_2) {
  struct emit e;
  emit_open(&e, CL_SET_MEM_OBJECT_DESTRUCTOR_CALLBACK);
  emit_cl_mem(&e, _0);
  emit_separator(&e);
  emit_void_ptr(&e, _1);
  emit_separator(&e);
  emit_void_ptr(&e, _2);
  emit_close(&e);
  cl_int ret = ((cl_set_mem_object_destructor_callback)
                opencl_fptr[CL_SET_MEM_OBJECT_DESTRUCTOR_CALLBACK])(_0, _1, _2);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

void *clSVMAlloc(cl_context _0, cl_svm_mem_flags _1, size_t _2, cl_uint _3) {
  struct emit e;
  emit_open(&e, CL_SVM_ALLOC);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_svm_mem_flags(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_cl_uint(&e, _3);
  emit_close(&e);
  void *ret = ((cl_svm_alloc)opencl_fptr[CL_SVM_ALLOC])(_0, _1, _2, _3);
  emit_ret(&e);
  emit_void_ptr(&e, ret);
  emit_end(&e);
  return ret;
}

void clSVMFree(cl_context _0, void *_1) {
  struct emit e;
  emit_open(&e, CL_SVM_FREE);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_void_ptr(&e, _1);
  emit_close(&e);
  ((cl_svm_free)opencl_fptr[CL_SVM_FREE])(_0, _1);
  emit_end(&e);
}

cl_sampler clCreateSamplerWithProperties(cl_context _0,
                                         const cl_sampler_properties *_1,
                                         cl_int *_2) {
  struct emit e;
  emit_open(&e, CL_CREATE_SAMPLER_WITH_PROPERTIES);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_sampler_properties_ptr(&e, _1);
  emit_separator(&e);
  emit_flush(&e);
  cl_sampler ret = ((cl_create_sampler_with_properties)
                    opencl_fptr[CL_CREATE_SAMPLER_WITH_PROPERTIES])(_0, _1, _2);
  emit_cl_int_ptr(&e, _2);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_sampler(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clRetainSampler(cl_sampler _0) {
  struct emit e;
  emit_open(&e, CL_RETAIN_SAMPLER);
  emit_cl_sampler(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_retain_sampler)opencl_fptr[CL_RETAIN_SAMPLER])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clReleaseSampler(cl_sampler _0) {
  struct emit e;
  emit_open(&e, CL_RELEASE_SAMPLER);
  emit_cl_sampler(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_release_sampler)opencl_fptr[CL_RELEASE_SAMPLER])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetSamplerInfo(cl_sampler _0, cl_sampler_info _1, size_t _2, void *_3,
                        size_t *_4) {
  struct emit e;
  emit_open(&e, CL_GET_SAMPLER_INFO);
  emit_cl_sampler(&e, _0);
  emit_separator(&e);
  emit_cl_sampler_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_close(&e);
  cl_int ret = ((cl_get_sampler_info)opencl_fptr[CL_GET_SAMPLER_INFO])(
      _0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_program clCreateProgramWithSource(cl_context _0, cl_uint _1, const char **_2,
                                     const size_t *_3, cl_int *_4) {
  struct emit e;
  emit_open(&e, CL_CREATE_PROGRAM_WITH_SOURCE);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_void_ptr(&e, _2);
  emit_separator(&e);
  emit_size_t_ptr(&e, _3);
  emit_separator(&e);
  emit_flush(&e);
  cl_program ret = ((
      cl_create_program_with_source)opencl_fptr[CL_CREATE_PROGRAM_WITH_SOURCE])(
      _0, _1, _2, _3, _4);
  emit_cl_int_ptr(&e, _4);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_program(&e, ret);
  emit_end(&e);
  return ret;
}

cl_program clCreateProgramWithBinary(cl_context _0, cl_uint _1,
                                     const cl_device_id *_2, const size_t *_3,
                                     const unsigned char **_4, cl_int *_5,
                                     cl_int *_6) {
  struct emit e;
  emit_open(&e, CL_CREATE_PROGRAM_WITH_BINARY);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_cl_device_id_ptr(&e, _2);
  emit_separator(&e);
  emit_size_t_ptr(&e, _3);
  emit_separator(&e);
  emit_void_ptr(&e, _4);
  emit_separator(&e);
  emit_cl_int_ptr(&e, _5);
  emit_separator(&e);
  emit_flush(&e);
  cl_program ret = ((
      cl_create_program_with_binary)opencl_fptr[CL_CREATE_PROGRAM_WITH_BINARY])(
      _0, _1, _2, _3, _4, _5, _6);
  emit_cl_int_ptr(&e, _6);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_program(&e, ret);
  emit_end(&e);
  return ret;
}

cl_program clCreateProgramWithBuiltInKernels(cl_context _0, cl_uint _1,
                                             const cl_device_id *_2,
                                             const char *_3, cl_int *_4) {
  struct emit e;
  emit_open(&e, CL_CREATE_PROGRAM_WITH_BUILT_IN_KERNELS);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_cl_device_id_ptr(&e, _2);
  emit_separator(&e);
  emit_char_ptr(&e, _3);
  emit_separator(&e);
  emit_flush(&e);
  cl_program ret = ((cl_create_program_with_built_in_kernels)
                    opencl_fptr[CL_CREATE_PROGRAM_WITH_BUILT_IN_KERNELS])(
      _0, _1, _2, _3, _4);
  emit_cl_int_ptr(&e, _4);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_program(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clRetainProgram(cl_program _0) {
  struct emit e;
  emit_open(&e, CL_RETAIN_PROGRAM);
  emit_cl_program(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_retain_program)opencl_fptr[CL_RETAIN_PROGRAM])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clReleaseProgram(cl_program _0) {
  struct emit e;
  emit_open(&e, CL_RELEASE_PROGRAM);
  emit_cl_program(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_release_program)opencl_fptr[CL_RELEASE_PROGRAM])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clBuildProgram(cl_program _0, cl_uint _1, const cl_device_id *_2,
                      const char *_3, void (*_4)(cl_program, void *),
                      void *_5) {
  struct emit e;
  emit_open(&e, CL_BUILD_PROGRAM);
  emit_cl_program(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_cl_device_id_ptr(&e, _2);
  emit_separator(&e);
  emit_char_ptr(&e, _3);
  emit_separator(&e);
  emit_void_ptr(&e, _4);
  emit_separator(&e);
  emit_void_ptr(&e, _5);
  emit_close(&e);
  cl_int ret =
      ((cl_build_program)opencl_fptr[CL_BUILD_PROGRAM])(_0, _1, _2, _3, _4, _5);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clCompileProgram(cl_program _0, cl_uint _1, const cl_device_id *_2,
                        const char *_3, cl_uint _4, const cl_program *_5,
                        const char **_6, void (*_7)(cl_program, void *),
                        void *_8) {
  struct emit e;
  emit_open(&e, CL_COMPILE_PROGRAM);
  emit_cl_program(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_cl_device_id_ptr(&e, _2);
  emit_separator(&e);
  emit_char_ptr(&e, _3);
  emit_separator(&e);
  emit_cl_uint(&e, _4);
  emit_separator(&e);
  emit_cl_program_ptr(&e, _5);
  emit_separator(&e);
  emit_void_ptr(&e, _6);
  emit_separator(&e);
  emit_void_ptr(&e, _7);
  emit_separator(&e);
  emit_void_ptr(&e, _8);
  emit_close(&e);
  cl_int ret = ((cl_compile_program)opencl_fptr[CL_COMPILE_PROGRAM])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_program clLinkProgram(cl_context _0, cl_uint _1, const cl_device_id *_2,
                         const char *_3, cl_uint _4, const cl_program *_5,
                         void (*_6)(cl_program, void *), void *_7, cl_int *_8) {
  struct emit e;
  emit_open(&e, CL_LINK_PROGRAM);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_cl_device_id_ptr(&e, _2);
  emit_separator(&e);
  emit_char_ptr(&e, _3);
  emit_separator(&e);
  emit_cl_uint(&e, _4);
  emit_separator(&e);
  emit_cl_program_ptr(&e, _5);
  emit_separator(&e);
  emit_void_ptr(&e, _6);
  emit_separator(&e);
  emit_void_ptr(&e, _7);
  emit_separator(&e);
  emit_flush(&e);
  cl_program ret = ((cl_link_program)opencl_fptr[CL_LINK_PROGRAM])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8);
  emit_cl_int_ptr(&e, _8);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_program(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clUnloadPlatformCompiler(cl_platform_id _0) {
  struct emit e;
  emit_open(&e, CL_UNLOAD_PLATFORM_COMPILER);
  emit_cl_platform_id(&e, _0);
  emit_close(&e);
  cl_int ret = ((
      cl_unload_platform_compiler)opencl_fptr[CL_UNLOAD_PLATFORM_COMPILER])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetProgramInfo(cl_program _0, cl_program_info _1, size_t _2, void *_3,
                        size_t *_4) {
  struct emit e;
  emit_open(&e, CL_GET_PROGRAM_INFO);
  emit_cl_program(&e, _0);
  emit_separator(&e);
  emit_cl_program_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_close(&e);
  cl_int ret = ((cl_get_program_info)opencl_fptr[CL_GET_PROGRAM_INFO])(
      _0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetProgramBuildInfo(cl_program _0, cl_device_id _1,
                             cl_program_build_info _2, size_t _3, void *_4,
                             size_t *_5) {
  struct emit e;
  emit_open(&e, CL_GET_PROGRAM_BUILD_INFO);
  emit_cl_program(&e, _0);
  emit_separator(&e);
  emit_cl_device_id(&e, _1);
  emit_separator(&e);
  emit_cl_program_build_info(&e, _2);
  emit_separator(&e);
  emit_size_t(&e, _3);
  emit_separator(&e);
  emit_void_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t_ptr(&e, _5);
  emit_close(&e);
  cl_int ret =
      ((cl_get_program_build_info)opencl_fptr[CL_GET_PROGRAM_BUILD_INFO])(
          _0, _1, _2, _3, _4, _5);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_kernel clCreateKernel(cl_program _0, const char *_1, cl_int *_2) {
  struct emit e;
  emit_open(&e, CL_CREATE_KERNEL);
  emit_cl_program(&e, _0);
  emit_separator(&e);
  emit_char_ptr(&e, _1);
  emit_separator(&e);
  emit_flush(&e);
  cl_kernel ret = ((cl_create_kernel)opencl_fptr[CL_CREATE_KERNEL])(_0, _1, _2);
  emit_cl_int_ptr(&e, _2);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_kernel(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clCreateKernelsInProgram(cl_program _0, cl_uint _1, cl_kernel *_2,
                                cl_uint *_3) {
  struct emit e;
  emit_open(&e, CL_CREATE_KERNELS_IN_PROGRAM);
  emit_cl_program(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_cl_kernel_ptr(&e, _2);
  emit_separator(&e);
  emit_cl_uint_ptr(&e, _3);
  emit_close(&e);
  cl_int ret =
      ((cl_create_kernels_in_program)opencl_fptr[CL_CREATE_KERNELS_IN_PROGRAM])(
          _0, _1, _2, _3);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clRetainKernel(cl_kernel _0) {
  struct emit e;
  emit_open(&e, CL_RETAIN_KERNEL);
  emit_cl_kernel(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_retain_kernel)opencl_fptr[CL_RETAIN_KERNEL])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clReleaseKernel(cl_kernel _0) {
  struct emit e;
  emit_open(&e, CL_RELEASE_KERNEL);
  emit_cl_kernel(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_release_kernel)opencl_fptr[CL_RELEASE_KERNEL])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clSetKernelArg(cl_kernel _0, cl_uint _1, size_t _2, const void *_3) {
  struct emit e;
  emit_open(&e, CL_SET_KERNEL_ARG);
  emit_cl_kernel(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_close(&e);
  cl_int ret =
      ((cl_set_kernel_arg)opencl_fptr[CL_SET_KERNEL_ARG])(_0, _1, _2, _3);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clSetKernelArgSVMPointer(cl_kernel _0, cl_uint _1, const void *_2) {
  struct emit e;
  emit_open(&e, CL_SET_KERNEL_ARG_SVM_POINTER);
  emit_cl_kernel(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_void_ptr(&e, _2);
  emit_close(&e);
  cl_int ret = ((
      cl_set_kernel_arg_svm_pointer)opencl_fptr[CL_SET_KERNEL_ARG_SVM_POINTER])(
      _0, _1, _2);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clSetKernelExecInfo(cl_kernel _0, cl_kernel_exec_info _1, size_t _2,
                           const void *_3) {
  struct emit e;
  emit_open(&e, CL_SET_KERNEL_EXEC_INFO);
  emit_cl_kernel(&e, _0);
  emit_separator(&e);
  emit_cl_kernel_exec_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_close(&e);
  cl_int ret = ((cl_set_kernel_exec_info)opencl_fptr[CL_SET_KERNEL_EXEC_INFO])(
      _0, _1, _2, _3);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetKernelInfo(cl_kernel _0, cl_kernel_info _1, size_t _2, void *_3,
                       size_t *_4) {
  struct emit e;
  emit_open(&e, CL_GET_KERNEL_INFO);
  emit_cl_kernel(&e, _0);
  emit_separator(&e);
  emit_cl_kernel_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_close(&e);
  cl_int ret =
      ((cl_get_kernel_info)opencl_fptr[CL_GET_KERNEL_INFO])(_0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetKernelArgInfo(cl_kernel _0, cl_uint _1, cl_kernel_arg_info _2,
                          size_t _3, void *_4, size_t *_5) {
  struct emit e;
  emit_open(&e, CL_GET_KERNEL_ARG_INFO);
  emit_cl_kernel(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_cl_kernel_arg_info(&e, _2);
  emit_separator(&e);
  emit_size_t(&e, _3);
  emit_separator(&e);
  emit_void_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t_ptr(&e, _5);
  emit_close(&e);
  cl_int ret = ((cl_get_kernel_arg_info)opencl_fptr[CL_GET_KERNEL_ARG_INFO])(
      _0, _1, _2, _3, _4, _5);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetKernelWorkGroupInfo(cl_kernel _0, cl_device_id _1,
                                cl_kernel_work_group_info _2, size_t _3,
                                void *_4, size_t *_5) {
  struct emit e;
  emit_open(&e, CL_GET_KERNEL_WORK_GROUP_INFO);
  emit_cl_kernel(&e, _0);
  emit_separator(&e);
  emit_cl_device_id(&e, _1);
  emit_separator(&e);
  emit_cl_kernel_work_group_info(&e, _2);
  emit_separator(&e);
  emit_size_t(&e, _3);
  emit_separator(&e);
  emit_void_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t_ptr(&e, _5);
  emit_close(&e);
  cl_int ret = ((
      cl_get_kernel_work_group_info)opencl_fptr[CL_GET_KERNEL_WORK_GROUP_INFO])(
      _0, _1, _2, _3, _4, _5);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clWaitForEvents(cl_uint _0, const cl_event *_1) {
  struct emit e;
  emit_open(&e, CL_WAIT_FOR_EVENTS);
  emit_cl_uint(&e, _0);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _1);
  emit_close(&e);
  cl_int ret = ((cl_wait_for_events)opencl_fptr[CL_WAIT_FOR_EVENTS])(_0, _1);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetEventInfo(cl_event _0, cl_event_info _1, size_t _2, void *_3,
                      size_t *_4) {
  struct emit e;
  emit_open(&e, CL_GET_EVENT_INFO);
  emit_cl_event(&e, _0);
  emit_separator(&e);
  emit_cl_event_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_close(&e);
  cl_int ret =
      ((cl_get_event_info)opencl_fptr[CL_GET_EVENT_INFO])(_0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_event clCreateUserEvent(cl_context _0, cl_int *_1) {
  struct emit e;
  emit_open(&e, CL_CREATE_USER_EVENT);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_flush(&e);
  cl_event ret =
      ((cl_create_user_event)opencl_fptr[CL_CREATE_USER_EVENT])(_0, _1);
  emit_cl_int_ptr(&e, _1);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_event(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clRetainEvent(cl_event _0) {
  struct emit e;
  emit_open(&e, CL_RETAIN_EVENT);
  emit_cl_event(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_retain_event)opencl_fptr[CL_RETAIN_EVENT])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clReleaseEvent(cl_event _0) {
  struct emit e;
  emit_open(&e, CL_RELEASE_EVENT);
  emit_cl_event(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_release_event)opencl_fptr[CL_RELEASE_EVENT])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clSetUserEventStatus(cl_event _0, cl_int _1) {
  struct emit e;
  emit_open(&e, CL_SET_USER_EVENT_STATUS);
  emit_cl_event(&e, _0);
  emit_separator(&e);
  emit_cl_int(&e, _1);
  emit_close(&e);
  cl_int ret =
      ((cl_set_user_event_status)opencl_fptr[CL_SET_USER_EVENT_STATUS])(_0, _1);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clSetEventCallback(cl_event _0, cl_int _1,
                          void (*_2)(cl_event, cl_int, void *), void *_3) {
  struct emit e;
  emit_open(&e, CL_SET_EVENT_CALLBACK);
  emit_cl_event(&e, _0);
  emit_separator(&e);
  emit_cl_int(&e, _1);
  emit_separator(&e);
  emit_void_ptr(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_close(&e);
  cl_int ret = ((cl_set_event_callback)opencl_fptr[CL_SET_EVENT_CALLBACK])(
      _0, _1, _2, _3);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clGetEventProfilingInfo(cl_event _0, cl_profiling_info _1, size_t _2,
                               void *_3, size_t *_4) {
  struct emit e;
  emit_open(&e, CL_GET_EVENT_PROFILING_INFO);
  emit_cl_event(&e, _0);
  emit_separator(&e);
  emit_cl_profiling_info(&e, _1);
  emit_separator(&e);
  emit_size_t(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_close(&e);
  cl_int ret =
      ((cl_get_event_profiling_info)opencl_fptr[CL_GET_EVENT_PROFILING_INFO])(
          _0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clFlush(cl_command_queue _0) {
  struct emit e;
  emit_open(&e, CL_FLUSH);
  emit_cl_command_queue(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_flush)opencl_fptr[CL_FLUSH])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clFinish(cl_command_queue _0) {
  struct emit e;
  emit_open(&e, CL_FINISH);
  emit_cl_command_queue(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_finish)opencl_fptr[CL_FINISH])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueReadBuffer(cl_command_queue _0, cl_mem _1, cl_bool _2,
                           size_t _3, size_t _4, void *_5, cl_uint _6,
                           const cl_event *_7, cl_event *_8) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_READ_BUFFER);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_bool(&e, _2);
  emit_separator(&e);
  emit_size_t(&e, _3);
  emit_separator(&e);
  emit_size_t(&e, _4);
  emit_separator(&e);
  emit_void_ptr(&e, _5);
  emit_separator(&e);
  emit_cl_uint(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _8);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_read_buffer)opencl_fptr[CL_ENQUEUE_READ_BUFFER])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueReadBufferRect(cl_command_queue _0, cl_mem _1, cl_bool _2,
                               const size_t *_3, const size_t *_4,
                               const size_t *_5, size_t _6, size_t _7,
                               size_t _8, size_t _9, void *_10, cl_uint _11,
                               const cl_event *_12, cl_event *_13) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_READ_BUFFER_RECT);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_bool(&e, _2);
  emit_separator(&e);
  emit_size_t_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t_ptr(&e, _5);
  emit_separator(&e);
  emit_size_t(&e, _6);
  emit_separator(&e);
  emit_size_t(&e, _7);
  emit_separator(&e);
  emit_size_t(&e, _8);
  emit_separator(&e);
  emit_size_t(&e, _9);
  emit_separator(&e);
  emit_void_ptr(&e, _10);
  emit_separator(&e);
  emit_cl_uint(&e, _11);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _12);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _13);
  emit_close(&e);
  cl_int ret =
      ((cl_enqueue_read_buffer_rect)opencl_fptr[CL_ENQUEUE_READ_BUFFER_RECT])(
          _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueWriteBuffer(cl_command_queue _0, cl_mem _1, cl_bool _2,
                            size_t _3, size_t _4, const void *_5, cl_uint _6,
                            const cl_event *_7, cl_event *_8) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_WRITE_BUFFER);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_bool(&e, _2);
  emit_separator(&e);
  emit_size_t(&e, _3);
  emit_separator(&e);
  emit_size_t(&e, _4);
  emit_separator(&e);
  emit_void_ptr(&e, _5);
  emit_separator(&e);
  emit_cl_uint(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _8);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_write_buffer)opencl_fptr[CL_ENQUEUE_WRITE_BUFFER])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueWriteBufferRect(cl_command_queue _0, cl_mem _1, cl_bool _2,
                                const size_t *_3, const size_t *_4,
                                const size_t *_5, size_t _6, size_t _7,
                                size_t _8, size_t _9, const void *_10,
                                cl_uint _11, const cl_event *_12,
                                cl_event *_13) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_WRITE_BUFFER_RECT);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_bool(&e, _2);
  emit_separator(&e);
  emit_size_t_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t_ptr(&e, _5);
  emit_separator(&e);
  emit_size_t(&e, _6);
  emit_separator(&e);
  emit_size_t(&e, _7);
  emit_separator(&e);
  emit_size_t(&e, _8);
  emit_separator(&e);
  emit_size_t(&e, _9);
  emit_separator(&e);
  emit_void_ptr(&e, _10);
  emit_separator(&e);
  emit_cl_uint(&e, _11);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _12);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _13);
  emit_close(&e);
  cl_int ret =
      ((cl_enqueue_write_buffer_rect)opencl_fptr[CL_ENQUEUE_WRITE_BUFFER_RECT])(
          _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueFillBuffer(cl_command_queue _0, cl_mem _1, const void *_2,
                           size_t _3, size_t _4, size_t _5, cl_uint _6,
                           const cl_event *_7, cl_event *_8) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_FILL_BUFFER);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_void_ptr(&e, _2);
  emit_separator(&e);
  emit_size_t(&e, _3);
  emit_separator(&e);
  emit_size_t(&e, _4);
  emit_separator(&e);
  emit_size_t(&e, _5);
  emit_separator(&e);
  emit_cl_uint(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _8);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_fill_buffer)opencl_fptr[CL_ENQUEUE_FILL_BUFFER])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueCopyBuffer(cl_command_queue _0, cl_mem _1, cl_mem _2, size_t _3,
                           size_t _4, size_t _5, cl_uint _6, const cl_event *_7,
                           cl_event *_8) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_COPY_BUFFER);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_mem(&e, _2);
  emit_separator(&e);
  emit_size_t(&e, _3);
  emit_separator(&e);
  emit_size_t(&e, _4);
  emit_separator(&e);
  emit_size_t(&e, _5);
  emit_separator(&e);
  emit_cl_uint(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _8);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_copy_buffer)opencl_fptr[CL_ENQUEUE_COPY_BUFFER])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueCopyBufferRect(cl_command_queue _0, cl_mem _1, cl_mem _2,
                               const size_t *_3, const size_t *_4,
                               const size_t *_5, size_t _6, size_t _7,
                               size_t _8, size_t _9, cl_uint _10,
                               const cl_event *_11, cl_event *_12) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_COPY_BUFFER_RECT);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_mem(&e, _2);
  emit_separator(&e);
  emit_size_t_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t_ptr(&e, _5);
  emit_separator(&e);
  emit_size_t(&e, _6);
  emit_separator(&e);
  emit_size_t(&e, _7);
  emit_separator(&e);
  emit_size_t(&e, _8);
  emit_separator(&e);
  emit_size_t(&e, _9);
  emit_separator(&e);
  emit_cl_uint(&e, _10);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _11);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _12);
  emit_close(&e);
  cl_int ret =
      ((cl_enqueue_copy_buffer_rect)opencl_fptr[CL_ENQUEUE_COPY_BUFFER_RECT])(
          _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueReadImage(cl_command_queue _0, cl_mem _1, cl_bool _2,
                          const size_t *_3, const size_t *_4, size_t _5,
                          size_t _6, void *_7, cl_uint _8, const cl_event *_9,
                          cl_event *_10) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_READ_IMAGE);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_bool(&e, _2);
  emit_separator(&e);
  emit_size_t_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t(&e, _5);
  emit_separator(&e);
  emit_size_t(&e, _6);
  emit_separator(&e);
  emit_void_ptr(&e, _7);
  emit_separator(&e);
  emit_cl_uint(&e, _8);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _9);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _10);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_read_image)opencl_fptr[CL_ENQUEUE_READ_IMAGE])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueWriteImage(cl_command_queue _0, cl_mem _1, cl_bool _2,
                           const size_t *_3, const size_t *_4, size_t _5,
                           size_t _6, const void *_7, cl_uint _8,
                           const cl_event *_9, cl_event *_10) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_WRITE_IMAGE);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_bool(&e, _2);
  emit_separator(&e);
  emit_size_t_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t(&e, _5);
  emit_separator(&e);
  emit_size_t(&e, _6);
  emit_separator(&e);
  emit_void_ptr(&e, _7);
  emit_separator(&e);
  emit_cl_uint(&e, _8);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _9);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _10);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_write_image)opencl_fptr[CL_ENQUEUE_WRITE_IMAGE])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueFillImage(cl_command_queue _0, cl_mem _1, const void *_2,
                          const size_t *_3, const size_t *_4, cl_uint _5,
                          const cl_event *_6, cl_event *_7) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_FILL_IMAGE);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_void_ptr(&e, _2);
  emit_separator(&e);
  emit_size_t_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_separator(&e);
  emit_cl_uint(&e, _5);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_fill_image)opencl_fptr[CL_ENQUEUE_FILL_IMAGE])(
      _0, _1, _2, _3, _4, _5, _6, _7);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueCopyImage(cl_command_queue _0, cl_mem _1, cl_mem _2,
                          const size_t *_3, const size_t *_4, const size_t *_5,
                          cl_uint _6, const cl_event *_7, cl_event *_8) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_COPY_IMAGE);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_mem(&e, _2);
  emit_separator(&e);
  emit_size_t_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t_ptr(&e, _5);
  emit_separator(&e);
  emit_cl_uint(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _8);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_copy_image)opencl_fptr[CL_ENQUEUE_COPY_IMAGE])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueCopyImageToBuffer(cl_command_queue _0, cl_mem _1, cl_mem _2,
                                  const size_t *_3, const size_t *_4, size_t _5,
                                  cl_uint _6, const cl_event *_7,
                                  cl_event *_8) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_COPY_IMAGE_TO_BUFFER);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_mem(&e, _2);
  emit_separator(&e);
  emit_size_t_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t(&e, _5);
  emit_separator(&e);
  emit_cl_uint(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _8);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_copy_image_to_buffer)
                opencl_fptr[CL_ENQUEUE_COPY_IMAGE_TO_BUFFER])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueCopyBufferToImage(cl_command_queue _0, cl_mem _1, cl_mem _2,
                                  size_t _3, const size_t *_4, const size_t *_5,
                                  cl_uint _6, const cl_event *_7,
                                  cl_event *_8) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_COPY_BUFFER_TO_IMAGE);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_mem(&e, _2);
  emit_separator(&e);
  emit_size_t(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t_ptr(&e, _5);
  emit_separator(&e);
  emit_cl_uint(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _8);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_copy_buffer_to_image)
                opencl_fptr[CL_ENQUEUE_COPY_BUFFER_TO_IMAGE])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

void *clEnqueueMapBuffer(cl_command_queue _0, cl_mem _1, cl_bool _2,
                         cl_map_flags _3, size_t _4, size_t _5, cl_uint _6,
                         const cl_event *_7, cl_event *_8, cl_int *_9) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_MAP_BUFFER);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_bool(&e, _2);
  emit_separator(&e);
  emit_cl_map_flags(&e, _3);
  emit_separator(&e);
  emit_size_t(&e, _4);
  emit_separator(&e);
  emit_size_t(&e, _5);
  emit_separator(&e);
  emit_cl_uint(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _8);
  emit_separator(&e);
  emit_flush(&e);
  void *ret = ((cl_enqueue_map_buffer)opencl_fptr[CL_ENQUEUE_MAP_BUFFER])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8, _9);
  emit_cl_int_ptr(&e, _9);
  emit_close(&e);
  emit_ret(&e);
  emit_void_ptr(&e, ret);
  emit_end(&e);
  return ret;
}

void *clEnqueueMapImage(cl_command_queue _0, cl_mem _1, cl_bool _2,
                        cl_map_flags _3, const size_t *_4, const size_t *_5,
                        size_t *_6, size_t *_7, cl_uint _8, const cl_event *_9,
                        cl_event *_10, cl_int *_11) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_MAP_IMAGE);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_cl_bool(&e, _2);
  emit_separator(&e);
  emit_cl_map_flags(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t_ptr(&e, _5);
  emit_separator(&e);
  emit_size_t_ptr(&e, _6);
  emit_separator(&e);
  emit_size_t_ptr(&e, _7);
  emit_separator(&e);
  emit_cl_uint(&e, _8);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _9);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _10);
  emit_separator(&e);
  emit_flush(&e);
  void *ret = ((cl_enqueue_map_image)opencl_fptr[CL_ENQUEUE_MAP_IMAGE])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11);
  emit_cl_int_ptr(&e, _11);
  emit_close(&e);
  emit_ret(&e);
  emit_void_ptr(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueUnmapMemObject(cl_command_queue _0, cl_mem _1, void *_2,
                               cl_uint _3, const cl_event *_4, cl_event *_5) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_UNMAP_MEM_OBJECT);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_mem(&e, _1);
  emit_separator(&e);
  emit_void_ptr(&e, _2);
  emit_separator(&e);
  emit_cl_uint(&e, _3);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _4);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _5);
  emit_close(&e);
  cl_int ret =
      ((cl_enqueue_unmap_mem_object)opencl_fptr[CL_ENQUEUE_UNMAP_MEM_OBJECT])(
          _0, _1, _2, _3, _4, _5);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueMigrateMemObjects(cl_command_queue _0, cl_uint _1,
                                  const cl_mem *_2, cl_mem_migration_flags _3,
                                  cl_uint _4, const cl_event *_5,
                                  cl_event *_6) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_MIGRATE_MEM_OBJECTS);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_cl_mem_ptr(&e, _2);
  emit_separator(&e);
  emit_cl_mem_migration_flags(&e, _3);
  emit_separator(&e);
  emit_cl_uint(&e, _4);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _5);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _6);
  emit_close(&e);
  cl_int ret =
      ((cl_enqueue_migrate_mem_objects)
       opencl_fptr[CL_ENQUEUE_MIGRATE_MEM_OBJECTS])(_0, _1, _2, _3, _4, _5, _6);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueNDRangeKernel(cl_command_queue _0, cl_kernel _1, cl_uint _2,
                              const size_t *_3, const size_t *_4,
                              const size_t *_5, cl_uint _6, const cl_event *_7,
                              cl_event *_8) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_ND_RANGE_KERNEL);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_kernel(&e, _1);
  emit_separator(&e);
  emit_cl_uint(&e, _2);
  emit_separator(&e);
  emit_size_t_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t_ptr(&e, _4);
  emit_separator(&e);
  emit_size_t_ptr(&e, _5);
  emit_separator(&e);
  emit_cl_uint(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _8);
  emit_close(&e);
  cl_int ret =
      ((cl_enqueue_nd_range_kernel)opencl_fptr[CL_ENQUEUE_ND_RANGE_KERNEL])(
          _0, _1, _2, _3, _4, _5, _6, _7, _8);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueNativeKernel(cl_command_queue _0, void (*_1)(void *), void *_2,
                             size_t _3, cl_uint _4, const cl_mem *_5,
                             const void **_6, cl_uint _7, const cl_event *_8,
                             cl_event *_9) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_NATIVE_KERNEL);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_void_ptr(&e, _1);
  emit_separator(&e);
  emit_void_ptr(&e, _2);
  emit_separator(&e);
  emit_size_t(&e, _3);
  emit_separator(&e);
  emit_cl_uint(&e, _4);
  emit_separator(&e);
  emit_cl_mem_ptr(&e, _5);
  emit_separator(&e);
  emit_void_ptr(&e, _6);
  emit_separator(&e);
  emit_cl_uint(&e, _7);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _8);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _9);
  emit_close(&e);
  cl_int ret =
      ((cl_enqueue_native_kernel)opencl_fptr[CL_ENQUEUE_NATIVE_KERNEL])(
          _0, _1, _2, _3, _4, _5, _6, _7, _8, _9);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueMarkerWithWaitList(cl_command_queue _0, cl_uint _1,
                                   const cl_event *_2, cl_event *_3) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_MARKER_WITH_WAIT_LIST);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _2);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _3);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_marker_with_wait_list)
                opencl_fptr[CL_ENQUEUE_MARKER_WITH_WAIT_LIST])(_0, _1, _2, _3);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueBarrierWithWaitList(cl_command_queue _0, cl_uint _1,
                                    const cl_event *_2, cl_event *_3) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_BARRIER_WITH_WAIT_LIST);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _2);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _3);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_barrier_with_wait_list)
                opencl_fptr[CL_ENQUEUE_BARRIER_WITH_WAIT_LIST])(_0, _1, _2, _3);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueSVMFree(cl_command_queue _0, cl_uint _1, void *_2[],
                        void (*_3)(cl_command_queue, cl_uint, void *[], void *),
                        void *_4, cl_uint _5, const cl_event *_6,
                        cl_event *_7) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_SVM_FREE);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_void_ptr(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_close(&e);
  emit_separator(&e);
  emit_void_ptr(&e, _4);
  emit_separator(&e);
  emit_cl_uint(&e, _5);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_svm_free)opencl_fptr[CL_ENQUEUE_SVM_FREE])(
      _0, _1, _2, _3, _4, _5, _6, _7);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueSVMMemcpy(cl_command_queue _0, cl_bool _1, void *_2,
                          const void *_3, size_t _4, cl_uint _5,
                          const cl_event *_6, cl_event *_7) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_SVM_MEMCPY);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_bool(&e, _1);
  emit_separator(&e);
  emit_void_ptr(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t(&e, _4);
  emit_separator(&e);
  emit_cl_uint(&e, _5);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_svm_memcpy)opencl_fptr[CL_ENQUEUE_SVM_MEMCPY])(
      _0, _1, _2, _3, _4, _5, _6, _7);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueSVMMemFill(cl_command_queue _0, void *_1, const void *_2,
                           size_t _3, size_t _4, cl_uint _5, const cl_event *_6,
                           cl_event *_7) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_SVM_MEM_FILL);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_void_ptr(&e, _1);
  emit_separator(&e);
  emit_void_ptr(&e, _2);
  emit_separator(&e);
  emit_size_t(&e, _3);
  emit_separator(&e);
  emit_size_t(&e, _4);
  emit_separator(&e);
  emit_cl_uint(&e, _5);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_svm_mem_fill)opencl_fptr[CL_ENQUEUE_SVM_MEM_FILL])(
      _0, _1, _2, _3, _4, _5, _6, _7);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueSVMMap(cl_command_queue _0, cl_bool _1, cl_map_flags _2,
                       void *_3, size_t _4, cl_uint _5, const cl_event *_6,
                       cl_event *_7) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_SVM_MAP);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_bool(&e, _1);
  emit_separator(&e);
  emit_cl_map_flags(&e, _2);
  emit_separator(&e);
  emit_void_ptr(&e, _3);
  emit_separator(&e);
  emit_size_t(&e, _4);
  emit_separator(&e);
  emit_cl_uint(&e, _5);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _6);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _7);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_svm_map)opencl_fptr[CL_ENQUEUE_SVM_MAP])(
      _0, _1, _2, _3, _4, _5, _6, _7);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueSVMUnmap(cl_command_queue _0, void *_1, cl_uint _2,
                         const cl_event *_3, cl_event *_4) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_SVM_UNMAP);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_void_ptr(&e, _1);
  emit_separator(&e);
  emit_cl_uint(&e, _2);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _3);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _4);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_svm_unmap)opencl_fptr[CL_ENQUEUE_SVM_UNMAP])(
      _0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

void *clGetExtensionFunctionAddressForPlatform(cl_platform_id _0,
                                               const char *_1) {
  struct emit e;
  emit_open(&e, CL_GET_EXTENSION_FUNCTION_ADDRESS_FOR_PLATFORM);
  emit_cl_platform_id(&e, _0);
  emit_separator(&e);
  emit_char_ptr(&e, _1);
  emit_close(&e);
  void *ret =
      ((cl_get_extension_function_address_for_platform)
       opencl_fptr[CL_GET_EXTENSION_FUNCTION_ADDRESS_FOR_PLATFORM])(_0, _1);
  emit_ret(&e);
  emit_void_ptr(&e, ret);
  emit_end(&e);
  return ret;
}

cl_mem clCreateImage2D(cl_context _0, cl_mem_flags _1,
                       const cl_image_format *_2, size_t _3, size_t _4,
                       size_t _5, void *_6, cl_int *_7) {
  struct emit e;
  emit_open(&e, CL_CREATE_IMAGE2_D);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_mem_flags(&e, _1);
  emit_separator(&e);
  emit_cl_image_format_ptr(&e, _2);
  emit_separator(&e);
  emit_size_t(&e, _3);
  emit_separator(&e);
  emit_size_t(&e, _4);
  emit_separator(&e);
  emit_size_t(&e, _5);
  emit_separator(&e);
  emit_void_ptr(&e, _6);
  emit_separator(&e);
  emit_flush(&e);
  cl_mem ret = ((cl_create_image2_d)opencl_fptr[CL_CREATE_IMAGE2_D])(
      _0, _1, _2, _3, _4, _5, _6, _7);
  emit_cl_int_ptr(&e, _7);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_mem(&e, ret);
  emit_end(&e);
  return ret;
}

cl_mem clCreateImage3D(cl_context _0, cl_mem_flags _1,
                       const cl_image_format *_2, size_t _3, size_t _4,
                       size_t _5, size_t _6, size_t _7, void *_8, cl_int *_9) {
  struct emit e;
  emit_open(&e, CL_CREATE_IMAGE3_D);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_mem_flags(&e, _1);
  emit_separator(&e);
  emit_cl_image_format_ptr(&e, _2);
  emit_separator(&e);
  emit_size_t(&e, _3);
  emit_separator(&e);
  emit_size_t(&e, _4);
  emit_separator(&e);
  emit_size_t(&e, _5);
  emit_separator(&e);
  emit_size_t(&e, _6);
  emit_separator(&e);
  emit_size_t(&e, _7);
  emit_separator(&e);
  emit_void_ptr(&e, _8);
  emit_separator(&e);
  emit_flush(&e);
  cl_mem ret = ((cl_create_image3_d)opencl_fptr[CL_CREATE_IMAGE3_D])(
      _0, _1, _2, _3, _4, _5, _6, _7, _8, _9);
  emit_cl_int_ptr(&e, _9);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_mem(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueMarker(cl_command_queue _0, cl_event *_1) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_MARKER);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _1);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_marker)opencl_fptr[CL_ENQUEUE_MARKER])(_0, _1);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueWaitForEvents(cl_command_queue _0, cl_uint _1,
                              const cl_event *_2) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_WAIT_FOR_EVENTS);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_uint(&e, _1);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _2);
  emit_close(&e);
  cl_int ret =
      ((cl_enqueue_wait_for_events)opencl_fptr[CL_ENQUEUE_WAIT_FOR_EVENTS])(
          _0, _1, _2);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueBarrier(cl_command_queue _0) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_BARRIER);
  emit_cl_command_queue(&e, _0);
  emit_close(&e);
  cl_int ret = ((cl_enqueue_barrier)opencl_fptr[CL_ENQUEUE_BARRIER])(_0);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clUnloadCompiler(void) {
  struct emit e;
  emit_open(&e, CL_UNLOAD_COMPILER);
  emit_close(&e);
  cl_int ret = ((cl_unload_compiler)opencl_fptr[CL_UNLOAD_COMPILER])();
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}

void *clGetExtensionFunctionAddress(const char *_0) {
  struct emit e;
  emit_open(&e, CL_GET_EXTENSION_FUNCTION_ADDRESS);
  emit_char_ptr(&e, _0);
  emit_close(&e);
  void *ret = ((cl_get_extension_function_address)
               opencl_fptr[CL_GET_EXTENSION_FUNCTION_ADDRESS])(_0);
  emit_ret(&e);
  emit_void_ptr(&e, ret);
  emit_end(&e);
  return ret;
}

cl_command_queue clCreateCommandQueue(cl_context _0, cl_device_id _1,
                                      cl_command_queue_properties _2,
                                      cl_int *_3) {
  struct emit e;
  emit_open(&e, CL_CREATE_COMMAND_QUEUE);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_device_id(&e, _1);
  emit_separator(&e);
  emit_cl_command_queue_properties(&e, _2);
  emit_separator(&e);
  emit_flush(&e);
  cl_command_queue ret =
      ((cl_create_command_queue)opencl_fptr[CL_CREATE_COMMAND_QUEUE])(_0, _1,
                                                                      _2, _3);
  emit_cl_int_ptr(&e, _3);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_command_queue(&e, ret);
  emit_end(&e);
  return ret;
}

cl_sampler clCreateSampler(cl_context _0, cl_bool _1, cl_addressing_mode _2,
                           cl_filter_mode _3, cl_int *_4) {
  struct emit e;
  emit_open(&e, CL_CREATE_SAMPLER);
  emit_cl_context(&e, _0);
  emit_separator(&e);
  emit_cl_bool(&e, _1);
  emit_separator(&e);
  emit_cl_addressing_mode(&e, _2);
  emit_separator(&e);
  emit_cl_filter_mode(&e, _3);
  emit_separator(&e);
  emit_flush(&e);
  cl_sampler ret =
      ((cl_create_sampler)opencl_fptr[CL_CREATE_SAMPLER])(_0, _1, _2, _3, _4);
  emit_cl_int_ptr(&e, _4);
  emit_close(&e);
  emit_ret(&e);
  emit_cl_sampler(&e, ret);
  emit_end(&e);
  return ret;
}

cl_int clEnqueueTask(cl_command_queue _0, cl_kernel _1, cl_uint _2,
                     const cl_event *_3, cl_event *_4) {
  struct emit e;
  emit_open(&e, CL_ENQUEUE_TASK);
  emit_cl_command_queue(&e, _0);
  emit_separator(&e);
  emit_cl_kernel(&e, _1);
  emit_separator(&e);
  emit_cl_uint(&e, _2);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _3);
  emit_separator(&e);
  emit_cl_event_ptr(&e, _4);
  emit_close(&e);
  cl_int ret =
      ((cl_enqueue_task)opencl_fptr[CL_ENQUEUE_TASK])(_0, _1, _2, _3, _4);
  emit_ret(&e);
  emit_cl_int(&e, ret);
  emit_end(&e);
  return ret;
}
