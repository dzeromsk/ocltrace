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

#ifndef OEPNCL_H_
#define OEPNCL_H_

#include <CL/opencl.h>

#define FUNCTIONS(F)                                                         \
  F(CL_GET_PLATFORM_I_DS, clGetPlatformIDs)                                  \
  F(CL_GET_PLATFORM_INFO, clGetPlatformInfo)                                 \
  F(CL_GET_DEVICE_I_DS, clGetDeviceIDs)                                      \
  F(CL_GET_DEVICE_INFO, clGetDeviceInfo)                                     \
  F(CL_CREATE_SUB_DEVICES, clCreateSubDevices)                               \
  F(CL_RETAIN_DEVICE, clRetainDevice)                                        \
  F(CL_RELEASE_DEVICE, clReleaseDevice)                                      \
  F(CL_CREATE_CONTEXT, clCreateContext)                                      \
  F(CL_CREATE_CONTEXT_FROM_TYPE, clCreateContextFromType)                    \
  F(CL_RETAIN_CONTEXT, clRetainContext)                                      \
  F(CL_RELEASE_CONTEXT, clReleaseContext)                                    \
  F(CL_GET_CONTEXT_INFO, clGetContextInfo)                                   \
  F(CL_CREATE_COMMAND_QUEUE_WITH_PROPERTIES,                                 \
    clCreateCommandQueueWithProperties)                                      \
  F(CL_RETAIN_COMMAND_QUEUE, clRetainCommandQueue)                           \
  F(CL_RELEASE_COMMAND_QUEUE, clReleaseCommandQueue)                         \
  F(CL_GET_COMMAND_QUEUE_INFO, clGetCommandQueueInfo)                        \
  F(CL_CREATE_BUFFER, clCreateBuffer)                                        \
  F(CL_CREATE_SUB_BUFFER, clCreateSubBuffer)                                 \
  F(CL_CREATE_IMAGE, clCreateImage)                                          \
  F(CL_CREATE_PIPE, clCreatePipe)                                            \
  F(CL_RETAIN_MEM_OBJECT, clRetainMemObject)                                 \
  F(CL_RELEASE_MEM_OBJECT, clReleaseMemObject)                               \
  F(CL_GET_SUPPORTED_IMAGE_FORMATS, clGetSupportedImageFormats)              \
  F(CL_GET_MEM_OBJECT_INFO, clGetMemObjectInfo)                              \
  F(CL_GET_IMAGE_INFO, clGetImageInfo)                                       \
  F(CL_GET_PIPE_INFO, clGetPipeInfo)                                         \
  F(CL_SET_MEM_OBJECT_DESTRUCTOR_CALLBACK, clSetMemObjectDestructorCallback) \
  F(CL_SVM_ALLOC, clSVMAlloc)                                                \
  F(CL_SVM_FREE, clSVMFree)                                                  \
  F(CL_CREATE_SAMPLER_WITH_PROPERTIES, clCreateSamplerWithProperties)        \
  F(CL_RETAIN_SAMPLER, clRetainSampler)                                      \
  F(CL_RELEASE_SAMPLER, clReleaseSampler)                                    \
  F(CL_GET_SAMPLER_INFO, clGetSamplerInfo)                                   \
  F(CL_CREATE_PROGRAM_WITH_SOURCE, clCreateProgramWithSource)                \
  F(CL_CREATE_PROGRAM_WITH_BINARY, clCreateProgramWithBinary)                \
  F(CL_CREATE_PROGRAM_WITH_BUILT_IN_KERNELS,                                 \
    clCreateProgramWithBuiltInKernels)                                       \
  F(CL_RETAIN_PROGRAM, clRetainProgram)                                      \
  F(CL_RELEASE_PROGRAM, clReleaseProgram)                                    \
  F(CL_BUILD_PROGRAM, clBuildProgram)                                        \
  F(CL_COMPILE_PROGRAM, clCompileProgram)                                    \
  F(CL_LINK_PROGRAM, clLinkProgram)                                          \
  F(CL_UNLOAD_PLATFORM_COMPILER, clUnloadPlatformCompiler)                   \
  F(CL_GET_PROGRAM_INFO, clGetProgramInfo)                                   \
  F(CL_GET_PROGRAM_BUILD_INFO, clGetProgramBuildInfo)                        \
  F(CL_CREATE_KERNEL, clCreateKernel)                                        \
  F(CL_CREATE_KERNELS_IN_PROGRAM, clCreateKernelsInProgram)                  \
  F(CL_RETAIN_KERNEL, clRetainKernel)                                        \
  F(CL_RELEASE_KERNEL, clReleaseKernel)                                      \
  F(CL_SET_KERNEL_ARG, clSetKernelArg)                                       \
  F(CL_SET_KERNEL_ARG_SVM_POINTER, clSetKernelArgSVMPointer)                 \
  F(CL_SET_KERNEL_EXEC_INFO, clSetKernelExecInfo)                            \
  F(CL_GET_KERNEL_INFO, clGetKernelInfo)                                     \
  F(CL_GET_KERNEL_ARG_INFO, clGetKernelArgInfo)                              \
  F(CL_GET_KERNEL_WORK_GROUP_INFO, clGetKernelWorkGroupInfo)                 \
  F(CL_WAIT_FOR_EVENTS, clWaitForEvents)                                     \
  F(CL_GET_EVENT_INFO, clGetEventInfo)                                       \
  F(CL_CREATE_USER_EVENT, clCreateUserEvent)                                 \
  F(CL_RETAIN_EVENT, clRetainEvent)                                          \
  F(CL_RELEASE_EVENT, clReleaseEvent)                                        \
  F(CL_SET_USER_EVENT_STATUS, clSetUserEventStatus)                          \
  F(CL_SET_EVENT_CALLBACK, clSetEventCallback)                               \
  F(CL_GET_EVENT_PROFILING_INFO, clGetEventProfilingInfo)                    \
  F(CL_FLUSH, clFlush)                                                       \
  F(CL_FINISH, clFinish)                                                     \
  F(CL_ENQUEUE_READ_BUFFER, clEnqueueReadBuffer)                             \
  F(CL_ENQUEUE_READ_BUFFER_RECT, clEnqueueReadBufferRect)                    \
  F(CL_ENQUEUE_WRITE_BUFFER, clEnqueueWriteBuffer)                           \
  F(CL_ENQUEUE_WRITE_BUFFER_RECT, clEnqueueWriteBufferRect)                  \
  F(CL_ENQUEUE_FILL_BUFFER, clEnqueueFillBuffer)                             \
  F(CL_ENQUEUE_COPY_BUFFER, clEnqueueCopyBuffer)                             \
  F(CL_ENQUEUE_COPY_BUFFER_RECT, clEnqueueCopyBufferRect)                    \
  F(CL_ENQUEUE_READ_IMAGE, clEnqueueReadImage)                               \
  F(CL_ENQUEUE_WRITE_IMAGE, clEnqueueWriteImage)                             \
  F(CL_ENQUEUE_FILL_IMAGE, clEnqueueFillImage)                               \
  F(CL_ENQUEUE_COPY_IMAGE, clEnqueueCopyImage)                               \
  F(CL_ENQUEUE_COPY_IMAGE_TO_BUFFER, clEnqueueCopyImageToBuffer)             \
  F(CL_ENQUEUE_COPY_BUFFER_TO_IMAGE, clEnqueueCopyBufferToImage)             \
  F(CL_ENQUEUE_MAP_BUFFER, clEnqueueMapBuffer)                               \
  F(CL_ENQUEUE_MAP_IMAGE, clEnqueueMapImage)                                 \
  F(CL_ENQUEUE_UNMAP_MEM_OBJECT, clEnqueueUnmapMemObject)                    \
  F(CL_ENQUEUE_MIGRATE_MEM_OBJECTS, clEnqueueMigrateMemObjects)              \
  F(CL_ENQUEUE_ND_RANGE_KERNEL, clEnqueueNDRangeKernel)                      \
  F(CL_ENQUEUE_NATIVE_KERNEL, clEnqueueNativeKernel)                         \
  F(CL_ENQUEUE_MARKER_WITH_WAIT_LIST, clEnqueueMarkerWithWaitList)           \
  F(CL_ENQUEUE_BARRIER_WITH_WAIT_LIST, clEnqueueBarrierWithWaitList)         \
  F(CL_ENQUEUE_SVM_FREE, clEnqueueSVMFree)                                   \
  F(CL_ENQUEUE_SVM_MEMCPY, clEnqueueSVMMemcpy)                               \
  F(CL_ENQUEUE_SVM_MEM_FILL, clEnqueueSVMMemFill)                            \
  F(CL_ENQUEUE_SVM_MAP, clEnqueueSVMMap)                                     \
  F(CL_ENQUEUE_SVM_UNMAP, clEnqueueSVMUnmap)                                 \
  F(CL_GET_EXTENSION_FUNCTION_ADDRESS_FOR_PLATFORM,                          \
    clGetExtensionFunctionAddressForPlatform)                                \
  F(CL_CREATE_IMAGE2_D, clCreateImage2D)                                     \
  F(CL_CREATE_IMAGE3_D, clCreateImage3D)                                     \
  F(CL_ENQUEUE_MARKER, clEnqueueMarker)                                      \
  F(CL_ENQUEUE_WAIT_FOR_EVENTS, clEnqueueWaitForEvents)                      \
  F(CL_ENQUEUE_BARRIER, clEnqueueBarrier)                                    \
  F(CL_UNLOAD_COMPILER, clUnloadCompiler)                                    \
  F(CL_GET_EXTENSION_FUNCTION_ADDRESS, clGetExtensionFunctionAddress)        \
  F(CL_CREATE_COMMAND_QUEUE, clCreateCommandQueue)                           \
  F(CL_CREATE_SAMPLER, clCreateSampler)                                      \
  F(CL_ENQUEUE_TASK, clEnqueueTask)

typedef cl_int (*cl_get_platform_i_ds)(cl_uint, cl_platform_id *, cl_uint *);
typedef cl_int (*cl_get_platform_info)(cl_platform_id, cl_platform_info, size_t,
                                       void *, size_t *);
typedef cl_int (*cl_get_device_i_ds)(cl_platform_id, cl_device_type, cl_uint,
                                     cl_device_id *, cl_uint *);
typedef cl_int (*cl_get_device_info)(cl_device_id, cl_device_info, size_t,
                                     void *, size_t *);
typedef cl_int (*cl_create_sub_devices)(cl_device_id,
                                        const cl_device_partition_property *,
                                        cl_uint, cl_device_id *, cl_uint *);
typedef cl_int (*cl_retain_device)(cl_device_id);
typedef cl_int (*cl_release_device)(cl_device_id);
typedef cl_context (*cl_create_context)(
    const cl_context_properties *, cl_uint, const cl_device_id *,
    void (*)(const char *, const void *, size_t, void *), void *, cl_int *);
typedef cl_context (*cl_create_context_from_type)(
    const cl_context_properties *, cl_device_type,
    void (*)(const char *, const void *, size_t, void *), void *, cl_int *);
typedef cl_int (*cl_retain_context)(cl_context);
typedef cl_int (*cl_release_context)(cl_context);
typedef cl_int (*cl_get_context_info)(cl_context, cl_context_info, size_t,
                                      void *, size_t *);
typedef cl_command_queue (*cl_create_command_queue_with_properties)(
    cl_context, cl_device_id, const cl_queue_properties *, cl_int *);
typedef cl_int (*cl_retain_command_queue)(cl_command_queue);
typedef cl_int (*cl_release_command_queue)(cl_command_queue);
typedef cl_int (*cl_get_command_queue_info)(cl_command_queue,
                                            cl_command_queue_info, size_t,
                                            void *, size_t *);
typedef cl_mem (*cl_create_buffer)(cl_context, cl_mem_flags, size_t, void *,
                                   cl_int *);
typedef cl_mem (*cl_create_sub_buffer)(cl_mem, cl_mem_flags,
                                       cl_buffer_create_type, const void *,
                                       cl_int *);
typedef cl_mem (*cl_create_image)(cl_context, cl_mem_flags,
                                  const cl_image_format *,
                                  const cl_image_desc *, void *, cl_int *);
typedef cl_mem (*cl_create_pipe)(cl_context, cl_mem_flags, cl_uint, cl_uint,
                                 const cl_pipe_properties *, cl_int *);
typedef cl_int (*cl_retain_mem_object)(cl_mem);
typedef cl_int (*cl_release_mem_object)(cl_mem);
typedef cl_int (*cl_get_supported_image_formats)(cl_context, cl_mem_flags,
                                                 cl_mem_object_type, cl_uint,
                                                 cl_image_format *, cl_uint *);
typedef cl_int (*cl_get_mem_object_info)(cl_mem, cl_mem_info, size_t, void *,
                                         size_t *);
typedef cl_int (*cl_get_image_info)(cl_mem, cl_image_info, size_t, void *,
                                    size_t *);
typedef cl_int (*cl_get_pipe_info)(cl_mem, cl_pipe_info, size_t, void *,
                                   size_t *);
typedef cl_int (*cl_set_mem_object_destructor_callback)(
    cl_mem, void (*)(cl_mem, void *), void *);
typedef void *(*cl_svm_alloc)(cl_context, cl_svm_mem_flags, size_t, cl_uint);
typedef void (*cl_svm_free)(cl_context, void *);
typedef cl_sampler (*cl_create_sampler_with_properties)(
    cl_context, const cl_sampler_properties *, cl_int *);
typedef cl_int (*cl_retain_sampler)(cl_sampler);
typedef cl_int (*cl_release_sampler)(cl_sampler);
typedef cl_int (*cl_get_sampler_info)(cl_sampler, cl_sampler_info, size_t,
                                      void *, size_t *);
typedef cl_program (*cl_create_program_with_source)(cl_context, cl_uint,
                                                    const char **,
                                                    const size_t *, cl_int *);
typedef cl_program (*cl_create_program_with_binary)(cl_context, cl_uint,
                                                    const cl_device_id *,
                                                    const size_t *,
                                                    const unsigned char **,
                                                    cl_int *, cl_int *);
typedef cl_program (*cl_create_program_with_built_in_kernels)(
    cl_context, cl_uint, const cl_device_id *, const char *, cl_int *);
typedef cl_int (*cl_retain_program)(cl_program);
typedef cl_int (*cl_release_program)(cl_program);
typedef cl_int (*cl_build_program)(cl_program, cl_uint, const cl_device_id *,
                                   const char *, void (*)(cl_program, void *),
                                   void *);
typedef cl_int (*cl_compile_program)(cl_program, cl_uint, const cl_device_id *,
                                     const char *, cl_uint, const cl_program *,
                                     const char **,
                                     void (*)(cl_program, void *), void *);
typedef cl_program (*cl_link_program)(cl_context, cl_uint, const cl_device_id *,
                                      const char *, cl_uint, const cl_program *,
                                      void (*)(cl_program, void *), void *,
                                      cl_int *);
typedef cl_int (*cl_unload_platform_compiler)(cl_platform_id);
typedef cl_int (*cl_get_program_info)(cl_program, cl_program_info, size_t,
                                      void *, size_t *);
typedef cl_int (*cl_get_program_build_info)(cl_program, cl_device_id,
                                            cl_program_build_info, size_t,
                                            void *, size_t *);
typedef cl_kernel (*cl_create_kernel)(cl_program, const char *, cl_int *);
typedef cl_int (*cl_create_kernels_in_program)(cl_program, cl_uint, cl_kernel *,
                                               cl_uint *);
typedef cl_int (*cl_retain_kernel)(cl_kernel);
typedef cl_int (*cl_release_kernel)(cl_kernel);
typedef cl_int (*cl_set_kernel_arg)(cl_kernel, cl_uint, size_t, const void *);
typedef cl_int (*cl_set_kernel_arg_svm_pointer)(cl_kernel, cl_uint,
                                                const void *);
typedef cl_int (*cl_set_kernel_exec_info)(cl_kernel, cl_kernel_exec_info,
                                          size_t, const void *);
typedef cl_int (*cl_get_kernel_info)(cl_kernel, cl_kernel_info, size_t, void *,
                                     size_t *);
typedef cl_int (*cl_get_kernel_arg_info)(cl_kernel, cl_uint, cl_kernel_arg_info,
                                         size_t, void *, size_t *);
typedef cl_int (*cl_get_kernel_work_group_info)(cl_kernel, cl_device_id,
                                                cl_kernel_work_group_info,
                                                size_t, void *, size_t *);
typedef cl_int (*cl_wait_for_events)(cl_uint, const cl_event *);
typedef cl_int (*cl_get_event_info)(cl_event, cl_event_info, size_t, void *,
                                    size_t *);
typedef cl_event (*cl_create_user_event)(cl_context, cl_int *);
typedef cl_int (*cl_retain_event)(cl_event);
typedef cl_int (*cl_release_event)(cl_event);
typedef cl_int (*cl_set_user_event_status)(cl_event, cl_int);
typedef cl_int (*cl_set_event_callback)(cl_event, cl_int,
                                        void (*)(cl_event, cl_int, void *),
                                        void *);
typedef cl_int (*cl_get_event_profiling_info)(cl_event, cl_profiling_info,
                                              size_t, void *, size_t *);
typedef cl_int (*cl_flush)(cl_command_queue);
typedef cl_int (*cl_finish)(cl_command_queue);
typedef cl_int (*cl_enqueue_read_buffer)(cl_command_queue, cl_mem, cl_bool,
                                         size_t, size_t, void *, cl_uint,
                                         const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_read_buffer_rect)(cl_command_queue, cl_mem, cl_bool,
                                              const size_t *, const size_t *,
                                              const size_t *, size_t, size_t,
                                              size_t, size_t, void *, cl_uint,
                                              const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_write_buffer)(cl_command_queue, cl_mem, cl_bool,
                                          size_t, size_t, const void *, cl_uint,
                                          const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_write_buffer_rect)(cl_command_queue, cl_mem,
                                               cl_bool, const size_t *,
                                               const size_t *, const size_t *,
                                               size_t, size_t, size_t, size_t,
                                               const void *, cl_uint,
                                               const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_fill_buffer)(cl_command_queue, cl_mem, const void *,
                                         size_t, size_t, size_t, cl_uint,
                                         const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_copy_buffer)(cl_command_queue, cl_mem, cl_mem,
                                         size_t, size_t, size_t, cl_uint,
                                         const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_copy_buffer_rect)(cl_command_queue, cl_mem, cl_mem,
                                              const size_t *, const size_t *,
                                              const size_t *, size_t, size_t,
                                              size_t, size_t, cl_uint,
                                              const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_read_image)(cl_command_queue, cl_mem, cl_bool,
                                        const size_t *, const size_t *, size_t,
                                        size_t, void *, cl_uint,
                                        const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_write_image)(cl_command_queue, cl_mem, cl_bool,
                                         const size_t *, const size_t *, size_t,
                                         size_t, const void *, cl_uint,
                                         const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_fill_image)(cl_command_queue, cl_mem, const void *,
                                        const size_t *, const size_t *, cl_uint,
                                        const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_copy_image)(cl_command_queue, cl_mem, cl_mem,
                                        const size_t *, const size_t *,
                                        const size_t *, cl_uint,
                                        const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_copy_image_to_buffer)(cl_command_queue, cl_mem,
                                                  cl_mem, const size_t *,
                                                  const size_t *, size_t,
                                                  cl_uint, const cl_event *,
                                                  cl_event *);
typedef cl_int (*cl_enqueue_copy_buffer_to_image)(cl_command_queue, cl_mem,
                                                  cl_mem, size_t,
                                                  const size_t *,
                                                  const size_t *, cl_uint,
                                                  const cl_event *, cl_event *);
typedef void *(*cl_enqueue_map_buffer)(cl_command_queue, cl_mem, cl_bool,
                                       cl_map_flags, size_t, size_t, cl_uint,
                                       const cl_event *, cl_event *, cl_int *);
typedef void *(*cl_enqueue_map_image)(cl_command_queue, cl_mem, cl_bool,
                                      cl_map_flags, const size_t *,
                                      const size_t *, size_t *, size_t *,
                                      cl_uint, const cl_event *, cl_event *,
                                      cl_int *);
typedef cl_int (*cl_enqueue_unmap_mem_object)(cl_command_queue, cl_mem, void *,
                                              cl_uint, const cl_event *,
                                              cl_event *);
typedef cl_int (*cl_enqueue_migrate_mem_objects)(cl_command_queue, cl_uint,
                                                 const cl_mem *,
                                                 cl_mem_migration_flags,
                                                 cl_uint, const cl_event *,
                                                 cl_event *);
typedef cl_int (*cl_enqueue_nd_range_kernel)(cl_command_queue, cl_kernel,
                                             cl_uint, const size_t *,
                                             const size_t *, const size_t *,
                                             cl_uint, const cl_event *,
                                             cl_event *);
typedef cl_int (*cl_enqueue_native_kernel)(cl_command_queue, void (*)(void *),
                                           void *, size_t, cl_uint,
                                           const cl_mem *, const void **,
                                           cl_uint, const cl_event *,
                                           cl_event *);
typedef cl_int (*cl_enqueue_marker_with_wait_list)(cl_command_queue, cl_uint,
                                                   const cl_event *,
                                                   cl_event *);
typedef cl_int (*cl_enqueue_barrier_with_wait_list)(cl_command_queue, cl_uint,
                                                    const cl_event *,
                                                    cl_event *);
typedef cl_int (*cl_enqueue_svm_free)(cl_command_queue, cl_uint, void *[],
                                      void (*)(cl_command_queue, cl_uint,
                                               void *[], void *),
                                      void *, cl_uint, const cl_event *,
                                      cl_event *);
typedef cl_int (*cl_enqueue_svm_memcpy)(cl_command_queue, cl_bool, void *,
                                        const void *, size_t, cl_uint,
                                        const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_svm_mem_fill)(cl_command_queue, void *,
                                          const void *, size_t, size_t, cl_uint,
                                          const cl_event *, cl_event *);
typedef cl_int (*cl_enqueue_svm_map)(cl_command_queue, cl_bool, cl_map_flags,
                                     void *, size_t, cl_uint, const cl_event *,
                                     cl_event *);
typedef cl_int (*cl_enqueue_svm_unmap)(cl_command_queue, void *, cl_uint,
                                       const cl_event *, cl_event *);
typedef void *(*cl_get_extension_function_address_for_platform)(cl_platform_id,
                                                                const char *);
typedef cl_mem (*cl_create_image2_d)(cl_context, cl_mem_flags,
                                     const cl_image_format *, size_t, size_t,
                                     size_t, void *, cl_int *);
typedef cl_mem (*cl_create_image3_d)(cl_context, cl_mem_flags,
                                     const cl_image_format *, size_t, size_t,
                                     size_t, size_t, size_t, void *, cl_int *);
typedef cl_int (*cl_enqueue_marker)(cl_command_queue, cl_event *);
typedef cl_int (*cl_enqueue_wait_for_events)(cl_command_queue, cl_uint,
                                             const cl_event *);
typedef cl_int (*cl_enqueue_barrier)(cl_command_queue);
typedef cl_int (*cl_unload_compiler)(void);
typedef void *(*cl_get_extension_function_address)(const char *);
typedef cl_command_queue (*cl_create_command_queue)(cl_context, cl_device_id,
                                                    cl_command_queue_properties,
                                                    cl_int *);
typedef cl_sampler (*cl_create_sampler)(cl_context, cl_bool, cl_addressing_mode,
                                        cl_filter_mode, cl_int *);
typedef cl_int (*cl_enqueue_task)(cl_command_queue, cl_kernel, cl_uint,
                                  const cl_event *, cl_event *);

struct stats {
  int name;
  int calls;
  int errors;
  double seconds;
};

#endif  // OEPNCL_H_