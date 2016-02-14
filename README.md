# ocltrace
## Help
```
$ ocltrace --help
Usage: ocltrace [OPTION...] <PROG> [<ARGS>]
OpenCL call tracer -- like strace but for OpenCL API

  -c, --statistics           Count time and calls for each func call and report
                             a summary
  -C                         Like -c but also print regular output
  -l, --library=LIBRARY      Name of the OpenCL library (default:
                             libOpenCL.so)
  -o, --output=FILE          Output to FILE instead of standard output
  -t, --time                 Prefix each line of the trace with timestamp
  -T                         Show the time spent in system calls
  -v, --verbose              Produce verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```

## Examples
Print timestamp, duration and thread id.
```
ocltrace -t -T ocl_add_example
1454928972.292485 clGetPlatformIDs(1, 0x7fffffffdea8, (nil)) = CL_SUCCESS <0.007157>
1454928972.299680 clGetDeviceIDs(6346720, 4, 1, {(nil)}, (nil)) = CL_SUCCESS <0.000021>
1454928972.299717 clCreateContext((nil), 1, {0x6178f0}, (nil), (nil), {CL_SUCCESS}) = 0x623150 <0.002905>
1454928972.302652 clCreateCommandQueue(0x623150, 0x6178f0, 0, {CL_SUCCESS}) = 0x72cb00 <0.000083>
1454928972.302759 clCreateProgramWithSource(0x623150, 1, 0x6020f0, NULL, {CL_SUCCESS}) = 0x72d930 <0.000034>
1454928972.302804 clBuildProgram(0x72d930, 0, NULL, "(null)", (nil), (nil)) = CL_SUCCESS <0.172309>
1454928972.475147 clCreateKernel(0x72d930, "sum", {CL_SUCCESS}) = 0x733890 <0.000057>
1454928972.475219 clCreateBuffer(0x623150, CL_MEM_READ_ONLY, 256, (nil), NULL) = 0x738a70 <0.000026>
1454928972.475257 clCreateBuffer(0x623150, CL_MEM_WRITE_ONLY, 256, (nil), NULL) = 0x734af0 <0.000039>
1454928972.475313 clEnqueueWriteBuffer(0x72cb00, 0x738a70, CL_TRUE, 0, 256, 0x7fffffffdfc0, 0, (nil), (nil)) = CL_SUCCESS <0.000635>
1454928972.475975 clSetKernelArg(0x733890, 0, 8, 0x7fffffffde98) = CL_SUCCESS <0.000015>
1454928972.476002 clSetKernelArg(0x733890, 1, 8, 0x7fffffffde90) = CL_SUCCESS <0.000017>
1454928972.476030 clGetKernelWorkGroupInfo(0x733890, 0x6178f0, CL_KERNEL_WORK_GROUP_SIZE, 8, 0x7fffffffdeb0, NULL) = CL_SUCCESS <0.000020>
1454928972.476062 clEnqueueNDRangeKernel(0x72cb00, 0x733890, 1, NULL, {64}, NULL, 0, (nil), (nil)) = CL_SUCCESS <0.000037>
1454928972.476114 clFinish(0x72cb00) = CL_SUCCESS <0.000183>
1454928972.476313 clEnqueueReadBuffer(0x72cb00, 0x734af0, CL_TRUE, 0, 256, 0x7fffffffdec0, 0, (nil), (nil)) = CL_SUCCESS <0.000219>
Computed '64/64' correct values!
1454928972.476548 clReleaseMemObject(0x738a70) = CL_SUCCESS <0.000014>
1454928972.476574 clReleaseMemObject(0x734af0) = CL_SUCCESS <0.000007>
1454928972.476594 clReleaseProgram(0x72d930) = CL_SUCCESS <0.000006>
1454928972.476616 clReleaseKernel(0x733890) = CL_SUCCESS <0.000019>
1454928972.476649 clReleaseCommandQueue(0x72cb00) = CL_SUCCESS <0.000062>
1454928972.476726 clReleaseContext(0x623150) = CL_SUCCESS <0.000612>
```
Print summary at exit. 
```
$ ocltrace -C ./utest
[...]
Summary:
% time     seconds      calls     errors function
------ ----------- ---------- ---------- ------------------------------
 87.50  385.742662       2284          0 clBuildProgram
  6.49   28.621562       1647          0 clEnqueueReadImage
  2.59   11.405485       3516          0 clEnqueueReadBuffer
  2.03    8.927315       4256          0 clCreateBuffer
  0.33    1.454549       2010          0 clEnqueueWriteBuffer
  0.21    0.926857       6165          0 clReleaseMemObject
  0.13    0.578603        107          0 clCreateContext
  0.10    0.420069       1650          0 clEnqueueWriteImage
  0.07    0.314754          4          0 clCompileProgram
  0.07    0.311035       1869          0 clEnqueueMapBuffer
  0.04    0.183372      12281          0 clSetKernelArg
  0.04    0.180137        642          0 clEnqueueMapImage
  0.04    0.175311          2          0 clLinkProgram
  0.04    0.162530        107          0 clReleaseCommandQueue
  0.04    0.160809       4565          0 clEnqueueNDRangeKernel
  0.03    0.131109       2430          0 clCreateKernel
  0.03    0.128091         19          0 clEnqueueReadBufferRect
  0.03    0.125916        638          0 clFinish
[...]
------ ----------- ---------- ---------- ------------------------------
100.00  440.831339      76536          0 total
```
