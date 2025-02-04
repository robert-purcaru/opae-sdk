// Copyright(c) 2017-2023, Intel Corporation
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

#ifndef _UAPI_LINUX_FPGA_DFL_H
#define _UAPI_LINUX_FPGA_DFL_H

#include <linux/types.h>
#include <linux/ioctl.h>

#define DFL_FPGA_API_VERSION 0

/*
 * The IOCTL interface for DFL based FPGA is designed for extensibility by
 * embedding the structure length (argsz) and flags into structures passed
 * between kernel and userspace. This design referenced the VFIO IOCTL
 * interface (include/uapi/linux/vfio.h).
 */

#define DFL_FPGA_MAGIC 0xB6

#define DFL_FPGA_BASE 0
#define DFL_PORT_BASE 0x40
#define DFL_FME_BASE 0x80
#define DFL_CXL_CACHE_BASE 0xA0
#define DFL_PCI_SVA_BASE 0xf8

/* Common IOCTLs for both FME and AFU file descriptor */

/**
 * DFL_FPGA_GET_API_VERSION - _IO(DFL_FPGA_MAGIC, DFL_FPGA_BASE + 0)
 *
 * Report the version of the driver API.
 * Return: Driver API Version.
 */

#define DFL_FPGA_GET_API_VERSION	_IO(DFL_FPGA_MAGIC, DFL_FPGA_BASE + 0)

/**
 * DFL_FPGA_CHECK_EXTENSION - _IO(DFL_FPGA_MAGIC, DFL_FPGA_BASE + 1)
 *
 * Check whether an extension is supported.
 * Return: 0 if not supported, otherwise the extension is supported.
 */

#define DFL_FPGA_CHECK_EXTENSION	_IO(DFL_FPGA_MAGIC, DFL_FPGA_BASE + 1)

/* IOCTLs for AFU file descriptor */

/**
 * DFL_FPGA_PORT_RESET - _IO(DFL_FPGA_MAGIC, DFL_PORT_BASE + 0)
 *
 * Reset the FPGA Port and its AFU. No parameters are supported.
 * Userspace can do Port reset at any time, e.g. during DMA or PR. But
 * it should never cause any system level issue, only functional failure
 * (e.g. DMA or PR operation failure) and be recoverable from the failure.
 * Return: 0 on success, -errno of failure
 */

#define DFL_FPGA_PORT_RESET		_IO(DFL_FPGA_MAGIC, DFL_PORT_BASE + 0)

/**
 * DFL_FPGA_PORT_GET_INFO - _IOR(DFL_FPGA_MAGIC, DFL_PORT_BASE + 1,
 *						struct dfl_fpga_port_info)
 *
 * Retrieve information about the fpga port.
 * Driver fills the info in provided struct dfl_fpga_port_info.
 * Return: 0 on success, -errno on failure.
 */
struct dfl_fpga_port_info {
	/* Input */
	__u32 argsz;		/* Structure length */
	/* Output */
	__u32 flags;		/* Zero for now */
	__u32 num_regions;	/* The number of supported regions */
	__u32 num_umsgs;	/* The number of allocated umsgs */
};

#define DFL_FPGA_PORT_GET_INFO		_IO(DFL_FPGA_MAGIC, DFL_PORT_BASE + 1)

/**
 * FPGA_PORT_GET_REGION_INFO - _IOWR(FPGA_MAGIC, PORT_BASE + 2,
 *					struct dfl_fpga_port_region_info)
 *
 * Retrieve information about a device memory region.
 * Caller provides struct dfl_fpga_port_region_info with index value set.
 * Driver returns the region info in other fields.
 * Return: 0 on success, -errno on failure.
 */
struct dfl_fpga_port_region_info {
	/* input */
	__u32 argsz;		/* Structure length */
	/* Output */
	__u32 flags;		/* Access permission */
#define DFL_PORT_REGION_READ	(1 << 0)	/* Region is readable */
#define DFL_PORT_REGION_WRITE	(1 << 1)	/* Region is writable */
#define DFL_PORT_REGION_MMAP	(1 << 2)	/* Can be mmaped to userspace */
	/* Input */
	__u32 index;		/* Region index */
#define DFL_PORT_REGION_INDEX_AFU	0	/* AFU */
#define DFL_PORT_REGION_INDEX_STP	1	/* Signal Tap */
	__u32 padding;
	/* Output */
	__u64 size;		/* Region size (bytes) */
	__u64 offset;		/* Region offset from start of device fd */
};

#define DFL_FPGA_PORT_GET_REGION_INFO	_IO(DFL_FPGA_MAGIC, DFL_PORT_BASE + 2)

/**
 * DFL_FPGA_PORT_DMA_MAP - _IOWR(DFL_FPGA_MAGIC, DFL_PORT_BASE + 3,
 *						struct dfl_fpga_port_dma_map)
 *
 * Map the dma memory per user_addr and length which are provided by caller.
 * Driver fills the iova in provided struct afu_port_dma_map.
 * This interface only accepts page-size aligned user memory for dma mapping.
 *
 * Setting only one of DFL_DMA_MAP_FLAG_READ or WRITE limits FPGA-initiated
 * DMA requests to only reads or only writes. To be back-compatiable with
 * legacy driver, setting neither flag is equivalent to setting both flags:
 * both read and write are requests permitted.
 *
 * Return: 0 on success, -errno on failure.
 */
struct dfl_fpga_port_dma_map {
	/* Input */
	__u32 argsz;		/* Structure length */
	__u32 flags;
#define DFL_DMA_MAP_FLAG_READ	(1 << 0)/* readable from device */
#define DFL_DMA_MAP_FLAG_WRITE	(1 << 1)/* writable from device */
	__u64 user_addr;        /* Process virtual address */
	__u64 length;           /* Length of mapping (bytes)*/
	/* Output */
	__u64 iova;             /* IO virtual address */
};

#define DFL_FPGA_PORT_DMA_MAP		_IO(DFL_FPGA_MAGIC, DFL_PORT_BASE + 3)

/**
 * DFL_FPGA_PORT_DMA_UNMAP - _IOW(FPGA_MAGIC, PORT_BASE + 4,
 *						struct dfl_fpga_port_dma_unmap)
 *
 * Unmap the dma memory per iova provided by caller.
 * Return: 0 on success, -errno on failure.
 */
struct dfl_fpga_port_dma_unmap {
	/* Input */
	__u32 argsz;		/* Structure length */
	__u32 flags;		/* Zero for now */
	__u64 iova;		/* IO virtual address */
};

#define DFL_FPGA_PORT_DMA_UNMAP		_IO(DFL_FPGA_MAGIC, DFL_PORT_BASE + 4)

/**
 * struct dfl_fpga_irq_set - the argument for DFL_FPGA_XXX_SET_IRQ ioctl.
 *
 * @start: Index of the first irq.
 * @count: The number of eventfd handler.
 * @evtfds: Eventfd handlers.
 */
struct dfl_fpga_irq_set {
	__u32 start;
	__u32 count;
	__s32 evtfds[];
};

/**
 * DFL_FPGA_PORT_ERR_GET_IRQ_NUM - _IOR(DFL_FPGA_MAGIC, DFL_PORT_BASE + 5,
 *								__u32 num_irqs)
 *
 * Get the number of irqs supported by the fpga port error reporting private
 * feature. Currently hardware supports up to 1 irq.
 * Return: 0 on success, -errno on failure.
 */
#define DFL_FPGA_PORT_ERR_GET_IRQ_NUM	_IOR(DFL_FPGA_MAGIC,	\
					     DFL_PORT_BASE + 5, __u32)

/**
 * DFL_FPGA_PORT_ERR_SET_IRQ - _IOW(DFL_FPGA_MAGIC, DFL_PORT_BASE + 6,
 *						struct dfl_fpga_irq_set)
 *
 * Set fpga port error reporting interrupt trigger if evtfds[n] is valid.
 * Unset related interrupt trigger if evtfds[n] is a negative value.
 * Return: 0 on success, -errno on failure.
 */
#define DFL_FPGA_PORT_ERR_SET_IRQ	_IOW(DFL_FPGA_MAGIC,	\
					     DFL_PORT_BASE + 6,	\
					     struct dfl_fpga_irq_set)

/**
 * DFL_FPGA_PORT_UINT_GET_IRQ_NUM - _IOR(DFL_FPGA_MAGIC, DFL_PORT_BASE + 7,
 *								__u32 num_irqs)
 *
 * Get the number of irqs supported by the fpga AFU interrupt private
 * feature.
 * Return: 0 on success, -errno on failure.
 */
#define DFL_FPGA_PORT_UINT_GET_IRQ_NUM	_IOR(DFL_FPGA_MAGIC,	\
					     DFL_PORT_BASE + 7, __u32)

/**
 * DFL_FPGA_PORT_UINT_SET_IRQ - _IOW(DFL_FPGA_MAGIC, DFL_PORT_BASE + 8,
 *						struct dfl_fpga_irq_set)
 *
 * Set fpga AFU interrupt trigger if evtfds[n] is valid.
 * Unset related interrupt trigger if evtfds[n] is a negative value.
 * Return: 0 on success, -errno on failure.
 */
#define DFL_FPGA_PORT_UINT_SET_IRQ	_IOW(DFL_FPGA_MAGIC,	\
					     DFL_PORT_BASE + 8,	\
					     struct dfl_fpga_irq_set)

/* IOCTLs for FME file descriptor */

/**
 * DFL_FPGA_FME_PORT_PR - _IOW(DFL_FPGA_MAGIC, DFL_FME_BASE + 0,
 *						struct dfl_fpga_fme_port_pr)
 *
 * Driver does Partial Reconfiguration based on Port ID and Buffer (Image)
 * provided by caller.
 * Return: 0 on success, -errno on failure.
 * If DFL_FPGA_FME_PORT_PR returns -EIO, that indicates the HW has detected
 * some errors during PR, under this case, the user can fetch HW error info
 * from the status of FME's fpga manager.
 */

struct dfl_fpga_fme_port_pr {
	/* Input */
	__u32 argsz;		/* Structure length */
	__u32 flags;		/* Zero for now */
	__u32 port_id;
	__u32 buffer_size;
	__u64 buffer_address;	/* Userspace address to the buffer for PR */
};

#define DFL_FPGA_FME_PORT_PR	_IO(DFL_FPGA_MAGIC, DFL_FME_BASE + 0)

/**
 * DFL_FPGA_FME_PORT_RELEASE - _IOW(DFL_FPGA_MAGIC, DFL_FME_BASE + 1,
 *						int port_id)
 *
 * Driver releases the port per Port ID provided by caller.
 * Return: 0 on success, -errno on failure.
 */
#define DFL_FPGA_FME_PORT_RELEASE   _IOW(DFL_FPGA_MAGIC, DFL_FME_BASE + 1, int)

/**
 * DFL_FPGA_FME_PORT_ASSIGN - _IOW(DFL_FPGA_MAGIC, DFL_FME_BASE + 2,
 *						int port_id)
 *
 * Driver assigns the port back per Port ID provided by caller.
 * Return: 0 on success, -errno on failure.
 */
#define DFL_FPGA_FME_PORT_ASSIGN     _IOW(DFL_FPGA_MAGIC, DFL_FME_BASE + 2, int)

/**
 * DFL_FPGA_FME_ERR_GET_IRQ_NUM - _IOR(DFL_FPGA_MAGIC, DFL_FME_BASE + 3,
 *							__u32 num_irqs)
 *
 * Get the number of irqs supported by the fpga fme error reporting private
 * feature. Currently hardware supports up to 1 irq.
 * Return: 0 on success, -errno on failure.
 */
#define DFL_FPGA_FME_ERR_GET_IRQ_NUM	_IOR(DFL_FPGA_MAGIC,	\
					     DFL_FME_BASE + 3, __u32)

/**
 * DFL_FPGA_FME_ERR_SET_IRQ - _IOW(DFL_FPGA_MAGIC, DFL_FME_BASE + 4,
 *						struct dfl_fpga_irq_set)
 *
 * Set fpga fme error reporting interrupt trigger if evtfds[n] is valid.
 * Unset related interrupt trigger if evtfds[n] is a negative value.
 * Return: 0 on success, -errno on failure.
 */
#define DFL_FPGA_FME_ERR_SET_IRQ	_IOW(DFL_FPGA_MAGIC,	\
					     DFL_FME_BASE + 4,	\
					     struct dfl_fpga_irq_set)

/**
 * DFL_PCI_SVA_BIND_DEV - _IO(DFL_FPGA_MAGIC, DFL_PCI_SVA_BASE + 0)
 *
 * Ensure that a PASID is present in the user process and enable the
 * PASID on the IOMMU domain of the device associated with the file handle.
 * Returns the PASID on success, -errno on failure.
 */
#define DFL_PCI_SVA_BIND_DEV		_IO(DFL_FPGA_MAGIC,	\
					     DFL_PCI_SVA_BASE + 0)

/**
 * DFL_PCI_SVA_UNBIND_DEV - _IO(DFL_FPGA_MAGIC,	DFL_PCI_SVA_BASE + 1)
 *
 * Unbind the current PASID from the device.
 */
#define DFL_PCI_SVA_UNBIND_DEV		_IO(DFL_FPGA_MAGIC,	\
					    DFL_PCI_SVA_BASE + 1)

/**
 * DFL_CXL_CACHE_GET_REGION_INFO - _IOWR(DFL_FPGA_MAGIC, DFL_CXL_CACHE_BASE + 0,
 *                                      struct dfl_cxl_cache_region_info)
 *
 * Retrieve information about a device memory region.
 * Caller provides struct dfl_cxl_cache_region_info with flags.
 * Driver returns the region info in other fields.
 * Return: 0 on success, -errno on failure.
 */

#define DFL_CXL_CACHE_GET_REGION_INFO _IO(DFL_FPGA_MAGIC, DFL_CXL_CACHE_BASE + 0)

/**
 * struct dfl_cxl_cache_region_info - CXL cache region information
 * @argsz: structure length
 * @flags: access permission
 * @size: region size (bytes)
 * @offset: region offset from start of device fd
 *
 * to retrieve  information about a device memory region
 */
struct dfl_cxl_cache_region_info {
	__u32 argsz;
	__u32 flags;
#define DFL_CXL_CACHE_REGION_READ	BIT(0)
#define DFL_CXL_CACHE_REGION_WRITE	BIT(1)
#define DFL_CXL_CACHE_REGION_MMAP	BIT(2)
	__u64 size;
	__u64 offset;
};

/**
 * DFL_CXL_CACHE_NUMA_BUFFER_MAP - _IOWR(DFL_FPGA_MAGIC, DFL_CXL_CACHE_BASE + 1,
 *                                      struct dfl_cxl_cache_buffer_map)
 *
 * Map the user memory per user_addr, length and numa node which are
 * provided by caller. The driver allocates memory on the numa node,
 * converts the user's virtual addressto a continuous physical address,
 * and writes the physical address to the cxl cache read/write address table CSR.
 *
 * This interface only accepts page-size aligned user memory for mapping.
 * Return: 0 on success, -errno on failure.
 */

#define DFL_ARRAY_MAX_SIZE   0x10

#define DFL_CXL_CACHE_NUMA_BUFFER_MAP    _IO(DFL_FPGA_MAGIC,  DFL_CXL_CACHE_BASE + 1)

/**
 * struct dfl_cxl_cache_buffer_map - maps user address to physical address.
 * @argsz: structure length
 * @flags: flags
 * @user_addr: user mmap virtual address
 * @length: length of mapping (bytes)
 * @csr_array: array of region address offset
 *
 * maps user allocated virtual address to physical address.
 */
struct dfl_cxl_cache_buffer_map {
	__u32 argsz;
	__u32 flags;
	__u64 user_addr;
	__u64 length;
	__u64 csr_array[DFL_ARRAY_MAX_SIZE];
};

/**
 * DFL_CXL_CACHE_NUMA_BUFFER_UNMAP - _IOWR(DFL_FPGA_MAGIC, DFL_CXL_CACHE_BASE + 1,
 *                                      struct dfl_cxl_cache_buffer_unmap)
 *
 * Unmaps the user memory per user_addr and length which are provided by caller
 * The driver deletes the physical pages of the user address and writes a zero
 * to the read/write address table CSR.
 * Return: 0 on success, -errno on failure.
 */

#define DFL_CXL_CACHE_NUMA_BUFFER_UNMAP  _IO(DFL_FPGA_MAGIC,  DFL_CXL_CACHE_BASE + 2)

/**
 * struct dfl_cxl_cache_buffer_unmap - unmaps user allocated memory.
 * @argsz: structure length
 * @flags: flags
 * @user_addr: user mmap virtual address
 * @length: length of mapping (bytes)
 * @csr_array: array of region address offset
 *
 * unmaps user allocated memory.
 */
struct dfl_cxl_cache_buffer_unmap {
	__u32 argsz;
	__u32 flags;
	__u64 user_addr;
	__u64 length;
	__u64 csr_array[DFL_ARRAY_MAX_SIZE];
};

#endif /* _UAPI_LINUX_FPGA_DFL_H */
