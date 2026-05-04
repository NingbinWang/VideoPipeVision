#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "blkdev.h"
#include "linux_version.h"

/* Function: blkdev_get_size
 * Description: Get the block device's size in bytes.
 * Input:	fd - the file descriptor of the device
 * Output:	bytes - device size in bytes 
 * Return:	0 if success, -1 if fail
 */
int blkdev_get_size(int fd, unsigned long long *bytes)
{
	unsigned long size;
	int ver = get_linux_version();

	/* kernels 2.4.15-2.4.17, had a broken BLKGETSIZE64 */
	if (ver >= KERNEL_VERSION (2,6,0) ||
		(ver >= KERNEL_VERSION (2,4,18) && ver < KERNEL_VERSION (2,5,0))) 
	{

		if (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		{
			return 0;
		}
	}
	if (ioctl(fd, BLKGETSIZE, &size) >= 0) 
	{
		*bytes = ((unsigned long long)size << 9);
		return 0;
	}

	return -1;
}

/* Function: blkdev_get_sectors
 * Description: Get the 512-byte sector count of the device.
 * Input:	fd - the file descriptor of the device
 * Output:	sectors - the 512-byte sector count
 * Return:	0 if success, -1 if fail
 */
int blkdev_get_sectors(int fd, unsigned long long *sectors)
{
	unsigned long long bytes;

	if (blkdev_get_size(fd, &bytes) == 0) 
	{
		*sectors = (bytes >> 9);
		return 0;
	}

	return -1;
}

/* Function: blkdev_get_sector_size
 * Description: Get the hardware sector size.
 * Input:	fd - the file descriptor of the device
 * Output:	sector_size - the sector size
 * Return:	0 if success, -1 if fail
 */
int blkdev_get_sector_size(int fd, int *sector_size)
{
	if (get_linux_version() < KERNEL_VERSION(2,3,3)) 
	{
		*sector_size = DEFAULT_SECTOR_SIZE;
		return 0;
	}
	if (ioctl(fd, BLKSSZGET, sector_size) >= 0)
	{
		return 0;
	}

	return -1;
}

/* Function: getBlkDevSectors
 * Description: Get the 512-byte sector count of the block device.
 * Input:	devname - the block device path name
 * Output:	sectors - the 512-byte sector count of the device
 * Return:	0 if success, < 0 if fail
 */
int getBlkDevSectors(const char *devname, unsigned long long *sectors)
{
	if (NULL == sectors)
	{
		return -1;
	}

	int fd, rval;

	if ((fd = open(devname, O_RDONLY)) < 0)
	{
		return -1;
	}

	rval = blkdev_get_sectors(fd, sectors);
	close(fd);

	if (-1 == rval)
	{
		return -1;
	}

	return 0;
}


