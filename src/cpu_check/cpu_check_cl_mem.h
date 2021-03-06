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


/*! \file cpu_check_cl_mem.h
 */

#ifndef __CPU_CHECK_CL_MEM_H
#define __CPU_CHECK_CL_MEM_H

#include <stdint.h>
#include <CL/cl.h>

#include "meta_data_lists/cl_kernel_lists.h"

/*!
 * Find any overflows in cl_mem buffer objects.
 *
 * \param kern_info
 *      information about the kernel that had the last opportunity
 *      to write to this buffer.
 * \param num_cl_mem
 *      number of cl_mem buffers in the array buffer_ptrs
 * \param buffer_ptrs
 *      array of void* that are actuall cl_mem objects
 * \param dupe
 *      array that describes which, if any, arguments to the kernel
 *      are duplicates of one another. See cpu_check_utils.h for a
 *      full description.
 * \param evt
 *      The cl_event that tells us when the real kernel has completed,
 *      so that we can start checking its canaries.
 */
void verify_cl_mem(kernel_info *kern_info, uint32_t num_cl_mem,
        void **buffer_ptrs, uint32_t *dupe, const cl_event *evt);

#endif // __CPU_CHECK_CL_MEM_H
