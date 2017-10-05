/********************************************************************************
 * Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ********************************************************************************/


// A test to make sure that programs with a buffer overflow complete and
// cause the buffer overflow detector to find the overflow.
#include "common_test_functions.h"

const char *kernel_source = "\n"\
"__kernel void test(__global uint *svm_buffer, uint len) {\n"\
"    uint i = get_global_id(0);\n"\
"    if (i < len) {\n"\
"        *(__global uint *)(&svm_buffer[i] - 10) = i;\n"\
"    }\n"\
"}\n";

int main(int argc, char** argv)
{
#ifdef CL_VERSION_2_0
    cl_int cl_err;
    uint32_t platform_to_use = 0;
    uint32_t device_to_use = 0;
    cl_device_type dev_type = CL_DEVICE_TYPE_DEFAULT;
    uint64_t buffer_size = DEFAULT_BUFFER_SIZE;

    // Check input options.
    check_opts(argc, argv, "SVM with Overflow",
            &platform_to_use, &device_to_use, &dev_type);

    // Set up the OpenCL environment.
    cl_platform_id platform = setup_platform(platform_to_use);
    cl_device_id device = setup_device(device_to_use, platform_to_use,
            platform, dev_type);

    if(!device_supports_svm(device, 0))
    {
        output_fake_errors(OUTPUT_FILE_NAME, EXPECTED_ERRORS);
        printf("Coarse-grained SVM not supported. Skipping Underflow SVM Test.\n");
        return 0;
    }

    cl_context context = setup_context(platform, device);
    cl_command_queue cmd_queue = setup_cmd_queue(context, device);

    // Build the program and kernel
    cl_program program = setup_program(context, 1, &kernel_source, device);
    cl_kernel test_kernel = setup_kernel(program, "test");

    // Run the actual test.
    printf("\n\nRunning Underflow SVM Test...\n");
    printf("    Using buffer size: %llu\n", (long long unsigned)buffer_size);

    // In this case, we will create an SVM buffer that is slightly too
    // small for the amount of writes we'll do it.
    // This will create a buffer overflow because of the "buffer_size-10" below
    void *bad_buffer = clSVMAlloc(context, CL_MEM_READ_WRITE, buffer_size, 0);
    if (bad_buffer == NULL)
    {
        fprintf(stderr, "clSVMAlloc near %s:%d failed.\n", __FILE__, __LINE__);
        exit(-1);
    }

    cl_err = clSetKernelArgSVMPointer(test_kernel, 0, bad_buffer);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clSetKernelArg(test_kernel, 1, sizeof(cl_uint), &buffer_size);
    check_cl_error(__FILE__, __LINE__, cl_err);

    // Each work item will touch sizeof(cl_uint) bytes.
    // This calculates how many work items we can have and still stay within
    // the nominal buffer size.
    // However, we shrank the real buffer size above, so overflow territory.
    uint64_t num_entries_in_buf = buffer_size / sizeof(cl_uint);
    size_t work_items_to_use;
    if (num_entries_in_buf > SIZE_MAX)
    {
        work_items_to_use = SIZE_MAX;
        fprintf(stderr, "\n\nWARNING -- TEST WILL NOT WORK PROPERLY.\n");
        fprintf(stderr, "\tYou are asking for too large a buffer.\n");
        fprintf(stderr, "\tReduce the buffer size to let the test to work.\n");
        fprintf(stderr, "\n\n");
    }
    else
        work_items_to_use = num_entries_in_buf;
    uint64_t bytes_written = (uint64_t)work_items_to_use * sizeof(cl_uint);

    printf("Launching %zu work items to write up to %llu entries.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)buffer_size);
    cl_err = clEnqueueNDRangeKernel(cmd_queue, test_kernel, 1, NULL,
        &work_items_to_use, NULL, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);

    clFinish(cmd_queue);
    printf("Done Running Underflow SVM Test.\n");
#else // CL_VERSION_2_0
    (void)argc;
    (void)argv;
    output_fake_errors(OUTPUT_FILE_NAME, EXPECTED_ERRORS);
    printf("OpenCL 2.0 not supported. Skipping Underflow SVM Test.\n");
#endif // CL_VERSION_2_0
    return 0;
}
