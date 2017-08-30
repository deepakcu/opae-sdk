// Copyright(c) 2017, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include "opae/access.h"
#include "opae/utils.h"
#include "common_int.h"
#include "intel-fpga.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <stdint.h>

/* Port UAFU */
#define AFU_PERMISSION (FPGA_REGION_READ | FPGA_REGION_WRITE | FPGA_REGION_MMAP)
#define AFU_SIZE	0x40000
#define AFU_OFFSET	0

fpga_result __FPGA_API__ fpgaWriteMMIO32(fpga_handle handle,
					 uint32_t mmio_num,
					 uint64_t offset,
					 uint32_t value)
{

	struct _fpga_handle *_handle = (struct _fpga_handle *)handle;
	fpga_result result = FPGA_OK;

	if (offset % sizeof(uint32_t) != 0) {
		FPGA_MSG("Misaligned MMIO access");
		return FPGA_INVALID_PARAM;
	}

	result = handle_check_and_lock(_handle);
	if (result)
		return result;

	struct wsid_map *wm = wsid_find_by_index(_handle->mmio_root, mmio_num);
	if (!wm) {
		FPGA_MSG("Trying to access MMIO before calling fpgaMapMMIO()");
		result = FPGA_NOT_FOUND;
		goto out_unlock;
	}

	if (offset > wm->len) {
		FPGA_MSG("offset out of bounds");
		result = FPGA_INVALID_PARAM;
		goto out_unlock;
	}

	*((volatile uint32_t *) ((uint8_t *)wm->offset + offset)) = value;

out_unlock:
	pthread_mutex_unlock(&_handle->lock);
	return result;
}

fpga_result __FPGA_API__ fpgaReadMMIO32(fpga_handle handle,
					uint32_t mmio_num,
					uint64_t offset,
					uint32_t *value)
{
	struct _fpga_handle *_handle = (struct _fpga_handle *)handle;
	fpga_result result = FPGA_OK;

	if (offset % sizeof(uint32_t) != 0) {
		FPGA_MSG("Misaligned MMIO access");
		return FPGA_INVALID_PARAM;
	}

	result = handle_check_and_lock(_handle);
	if (result)
		return result;

	struct wsid_map *wm = wsid_find_by_index(_handle->mmio_root, mmio_num);
	if (!wm) {
		FPGA_MSG("Trying to access MMIO before calling fpgaMapMMIO()");
		result = FPGA_INVALID_PARAM;
		goto out_unlock;
	}

	if (offset > wm->len) {
		FPGA_MSG("offset out of bounds");
		result = FPGA_INVALID_PARAM;
		goto out_unlock;
	}

	*value = *((volatile uint32_t *) ((uint8_t *)wm->offset + offset));

out_unlock:
	pthread_mutex_unlock(&_handle->lock);
	return result;
}

fpga_result __FPGA_API__ fpgaWriteMMIO64(fpga_handle handle,
					 uint32_t mmio_num,
					 uint64_t offset,
					 uint64_t value)
{
	struct _fpga_handle *_handle = (struct _fpga_handle *)handle;
	fpga_result result = FPGA_OK;

	if (offset % sizeof(uint64_t) != 0) {
		FPGA_MSG("Misaligned MMIO access");
		return FPGA_INVALID_PARAM;
	}

	result = handle_check_and_lock(_handle);
	if (result)
		return result;

	struct wsid_map *wm = wsid_find_by_index(_handle->mmio_root, mmio_num);
	if (!wm) {
		FPGA_MSG("Trying to access MMIO before calling fpgaMapMMIO()");
		result = FPGA_NOT_FOUND;
		goto out_unlock;
	}

	if (offset > wm->len) {
		FPGA_MSG("offset out of bounds");
		result = FPGA_INVALID_PARAM;
		goto out_unlock;
	}

	*((volatile uint64_t *) ((uint8_t *)wm->offset + offset)) = value;

out_unlock:
	pthread_mutex_unlock(&_handle->lock);
	return result;
}

fpga_result __FPGA_API__ fpgaReadMMIO64(fpga_handle handle,
					uint32_t mmio_num,
					uint64_t offset,
					uint64_t *value)
{
	struct _fpga_handle *_handle = (struct _fpga_handle *)handle;
	fpga_result result = FPGA_OK;

	if (offset % sizeof(uint64_t) != 0) {
		FPGA_MSG("Misaligned MMIO access");
		return FPGA_INVALID_PARAM;
	}

	result = handle_check_and_lock(_handle);
	if (result)
		return result;

	struct wsid_map *wm = wsid_find_by_index(_handle->mmio_root, mmio_num);
	if (!wm) {
		FPGA_MSG("Trying to access MMIO before calling fpgaMapMMIO()");
		result = FPGA_NOT_FOUND;
		goto out_unlock;
	}

	if (offset > wm->len) {
		FPGA_MSG("offset out of bounds");
		result = FPGA_INVALID_PARAM;
		goto out_unlock;
	}

	*value = *((volatile uint64_t *) ((uint8_t *)wm->offset + offset));

out_unlock:
	pthread_mutex_unlock(&_handle->lock);
	return result;
}

static fpga_result port_get_region_info(fpga_handle handle,
				 uint32_t mmio_num,
				 uint32_t *flags,
				 uint64_t *size,
				 uint64_t *offset)
{
	struct _fpga_handle *_handle = (struct _fpga_handle *)handle;
	fpga_result result = FPGA_OK;

	ASSERT_NOT_NULL(flags);
	ASSERT_NOT_NULL(size);
	ASSERT_NOT_NULL(offset);

	result = handle_check_and_lock(_handle);
	if (result)
		return result;

	/* Set ioctl fpga_port_region_info struct parameters */
	struct fpga_port_region_info rinfo = {.argsz = sizeof(rinfo),
					      .padding = 0,
					      .index = (__u32) mmio_num};

	/* Dispatch ioctl command */
	if (ioctl(_handle->fddev, FPGA_PORT_GET_REGION_INFO, &rinfo) != 0) {
		FPGA_MSG("FPGA_PORT_GET_REGION_INFO ioctl failed: %s",
			 strerror(errno));
		result = FPGA_INVALID_PARAM;
		goto out_unlock;
	}

	*flags = (uint32_t) rinfo.flags;
	*size = (uint64_t) rinfo.size;
	*offset = (uint64_t) rinfo.offset;

out_unlock:
	pthread_mutex_unlock(&_handle->lock);
	return result;
}

static fpga_result port_mmap_region(fpga_handle handle,
			     void **vaddr,
			     uint64_t size,
			     uint32_t flags,
			     uint64_t offset,
			     uint32_t mmio_num)
{
	void *addr;
	struct _fpga_handle *_handle = (struct _fpga_handle *)handle;
	fpga_result result = FPGA_OK;

	/* Assure returning pointer contains allocated memory */
	ASSERT_NOT_NULL(vaddr);

	result = handle_check_and_lock(_handle);
	if (result)
		return result;

	/* Map MMIO memory */
	addr = (void *) mmap(NULL, size, flags, MAP_SHARED, _handle->fddev, offset);
	if (vaddr == MAP_FAILED) {
		FPGA_MSG("Unable to map MMIO region. Error value is : %s",
			 strerror(errno));
		result = FPGA_INVALID_PARAM;
		goto out_unlock;
	}

	/* Save return address */
	if (vaddr)
		*vaddr = addr;

out_unlock:
	pthread_mutex_unlock(&_handle->lock);
	return result;
}

fpga_result __FPGA_API__ fpgaMapMMIO(fpga_handle handle,
				     uint32_t mmio_num,
				     uint64_t **mmio_ptr)
{
	struct _fpga_handle *_handle = (struct _fpga_handle *)handle;
	fpga_result result = FPGA_NOT_FOUND;
	void *addr;
	uint64_t wsid;

	result = handle_check_and_lock(_handle);
	if (result)
		return result;

	/* Obtain MMIO region information */
	uint32_t flags;
	uint64_t size;
	uint64_t offset;
	result = port_get_region_info(handle,
				      mmio_num,
				      &flags,
				      &size,
				      &offset);

	if (result != FPGA_OK)
		goto out_unlock;

	if (flags != AFU_PERMISSION) {
		FPGA_MSG("Invalid MMIO permission flags");
		result = FPGA_NO_ACCESS;
		goto out_unlock;
	}

	/* Map UAFU MMIO */
	result = port_mmap_region(handle,
				    (void **) &addr,
				    size,
				    PROT_READ | PROT_WRITE,
				    offset,
				    mmio_num);
	if (result != FPGA_OK)
		goto out_unlock;

	/* Add to MMIO list */
	wsid = wsid_gen();
	if (!wsid_add(&_handle->mmio_root,
		      wsid,
		      (uint64_t) NULL,
		      (uint64_t) NULL,
		      size,
		      (uint64_t) addr,
		      mmio_num,
		      0)) {
		if (munmap(addr, size)) {
			FPGA_MSG("munmap failed. Error value is : %s",
				 strerror(errno));
			result = FPGA_INVALID_PARAM;
			goto out_unlock;
		} else {
			FPGA_MSG("Failed to add MMIO id: %d", mmio_num);
			result = FPGA_NO_MEMORY;
			goto out_unlock;
		}
	}

	/* Store return value only if return pointer has allocated memory */
	if (mmio_ptr)
		*mmio_ptr = addr;

out_unlock:
	pthread_mutex_unlock(&_handle->lock);
	return result;
}

fpga_result __FPGA_API__ fpgaUnmapMMIO(fpga_handle handle,
				       uint32_t mmio_num)
{
	struct _fpga_handle *_handle = (struct _fpga_handle *)handle;
	void *mmio_ptr;
	fpga_result result = FPGA_OK;

	result = handle_check_and_lock(_handle);
	if (result)
		return result;

	/* Fetch the MMIO physical address and length */
	struct wsid_map *wm = wsid_find_by_index(_handle->mmio_root, mmio_num);
	if (!wm) {
		FPGA_MSG("MMIO region %d not found", mmio_num);
		result = FPGA_INVALID_PARAM;
		goto out_unlock;
	}

	/* Unmap UAFU MMIO */
	mmio_ptr = (void *) wm->offset;
	if (munmap((void *) mmio_ptr, wm->len)) {
		FPGA_MSG("munmap failed: %s",
			 strerror(errno));
		result = FPGA_INVALID_PARAM;
		goto out_unlock;
	}

	/* Remove MMIO */
	wsid_del(&_handle->mmio_root, wm->wsid);

out_unlock:
	pthread_mutex_unlock(&_handle->lock);
	return result;
}