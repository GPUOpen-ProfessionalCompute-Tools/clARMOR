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


// A test to make sure that programs without a buffer overflow complete without
// causing the buffer overflow detector to find a false overflow.
#include "common_test_functions.h"

int main(int argc, char** argv)
{
    cl_int cl_err;
    uint32_t platform_to_use = 0;
    uint32_t device_to_use = 0;
    cl_device_type dev_type = CL_DEVICE_TYPE_DEFAULT;
    uint64_t buffer_size = DEFAULT_BUFFER_SIZE;

    // Check input options.
    check_opts(argc, argv, "cl_mem without Overflow",
            &platform_to_use, &device_to_use, &dev_type);

    // Set up the OpenCL environment.
    cl_platform_id platform = setup_platform(platform_to_use);
    cl_device_id device = setup_device(device_to_use, platform_to_use,
            platform, dev_type);
    cl_context context = setup_context(platform, device);
    cl_command_queue cmd_queue = setup_cmd_queue(context, device);

    // Run the actual test.
    printf("\n\nRunning Bad cl_mem API Test...\n");
    printf("    Using buffer size: %llu\n", (long long unsigned)(buffer_size-10));

    // In this case, we are going to create a cl_mem buffer of the appropriate size.
    // This will not create a buffer overflow.
    cl_mem bad_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
        (buffer_size-10),  NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_mem good_buffer2 = clCreateBuffer(context, CL_MEM_READ_WRITE,
        buffer_size,  NULL, &cl_err);
    check_cl_error(__FILE__, __LINE__, cl_err);


    // Each work item will touch  sizeof(cl_uint) bytes.
    // This calculates how many work items we can have and still stay within
    // the buffer size.
    // If the maximum work items we can launch won't reach the end of the
    // buffer, that is OK. Then we definitely won't have an overflow.
    uint64_t num_entries_in_buf = buffer_size / sizeof(cl_uint);
    size_t work_items_to_use;
    if (num_entries_in_buf > SIZE_MAX)
        work_items_to_use = SIZE_MAX;
    else
        work_items_to_use = num_entries_in_buf;
    uint64_t bytes_written = (uint64_t)work_items_to_use * sizeof(cl_uint);

    printf("Launching %zu work items to write up to %llu entries.\n",
            work_items_to_use, (long long unsigned)num_entries_in_buf);
    printf("This will write %llu out of %llu bytes in the buffer.\n",
            (long long unsigned)bytes_written,
            (long long unsigned)(buffer_size-10));

    char *host_ptr;
    host_ptr = calloc(1, buffer_size);

    cl_err = clEnqueueReadBuffer(cmd_queue, bad_buffer, CL_TRUE, 0, buffer_size, host_ptr, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
    cl_err = clEnqueueWriteBuffer(cmd_queue, bad_buffer, CL_TRUE, 0, buffer_size, host_ptr, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
#ifdef CL_VERSION_1_2
    char fill = '5';
    cl_err = clEnqueueFillBuffer(cmd_queue, bad_buffer, &fill, sizeof(char), 0, buffer_size, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
#else
    // Run another test again to make sure we see the same number of errors
    cl_err = clEnqueueWriteBuffer(cmd_queue, bad_buffer, CL_TRUE, 0, buffer_size, host_ptr, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);
#endif
    cl_err = clEnqueueCopyBuffer(cmd_queue, good_buffer2, bad_buffer, 0, 0, buffer_size, 0, NULL, NULL);
    check_cl_error(__FILE__, __LINE__, cl_err);


    clFinish(cmd_queue);
    free(host_ptr);
    printf("Done Running Bad cl_mem API Test.\n");
    return 0;
}
