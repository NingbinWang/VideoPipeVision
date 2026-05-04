#if 1
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <setjmp.h>
#include <errno.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>

#include "blkdev.h"
#include "fdisk.h"

#ifdef HAVE_LINUX_COMPILER_H
#include <linux/compiler.h>
#endif
#ifdef HAVE_LINUX_BLKPG_H
#include <linux/blkpg.h>
#endif
//#include    "stor_sata_linux.h"

#define LINE_LENGTH	800
#define pt_offset(b, n)	((struct partition *)((b) + 0x1be + \
				(n) * sizeof(struct partition)))

#define set_hsc(h,s,c,sector) { \
				s = sector % sectors + 1;	\
				sector /= sectors;	\
				h = sector % heads;	\
				sector /= heads;	\
				c = sector & 0xff;	\
				s |= (sector >> 2) & 0xc0;	\
			}

//#define FDISK_DBG
#ifdef FDISK_DBG
#define FDISK_INFO(fmt, args...) fprintf(stderr, fmt, ##args);
#else
#define FDISK_INFO(fmt, args...)
#endif
struct systypes i386_sys_types[] = {
	{0x00, N_("Empty")},
	{0x01, N_("FAT12")},
	{0x02, N_("XENIX root")},
	{0x03, N_("XENIX usr")},
	{0x04, N_("FAT16 <32M")},
	{0x05, N_("Extended")},		/* DOS 3.3+ extended partition */
	{0x06, N_("FAT16")},		/* DOS 16-bit >=32M */
	{0x07, N_("HPFS/NTFS")},	/* OS/2 IFS, eg, HPFS or NTFS or QNX */
	{0x08, N_("AIX")},		/* AIX boot (AIX -- PS/2 port) or SplitDrive */
	{0x09, N_("AIX bootable")},	/* AIX data or Coherent */
	{0x0a, N_("OS/2 Boot Manager")},/* OS/2 Boot Manager */
	{0x0b, N_("W95 FAT32")},
	{0x0c, N_("W95 FAT32 (LBA)")},/* LBA really is `Extended Int 13h' */
	{0x0e, N_("W95 FAT16 (LBA)")},
	{0x0f, N_("W95 Ext'd (LBA)")},
	{0x10, N_("OPUS")},
	{0x11, N_("Hidden FAT12")},
	{0x12, N_("Compaq diagnostics")},
	{0x14, N_("Hidden FAT16 <32M")},
	{0x16, N_("Hidden FAT16")},
	{0x17, N_("Hidden HPFS/NTFS")},
	{0x18, N_("AST SmartSleep")},
	{0x1b, N_("Hidden W95 FAT32")},
	{0x1c, N_("Hidden W95 FAT32 (LBA)")},
	{0x1e, N_("Hidden W95 FAT16 (LBA)")},
	{0x24, N_("NEC DOS")},
	{0x39, N_("Plan 9")},
	{0x3c, N_("PartitionMagic recovery")},
	{0x40, N_("Venix 80286")},
	{0x41, N_("PPC PReP Boot")},
	{0x42, N_("SFS")},
	{0x4d, N_("QNX4.x")},
	{0x4e, N_("QNX4.x 2nd part")},
	{0x4f, N_("QNX4.x 3rd part")},
	{0x50, N_("OnTrack DM")},
	{0x51, N_("OnTrack DM6 Aux1")},	/* (or Novell) */
	{0x52, N_("CP/M")},		/* CP/M or Microport SysV/AT */
	{0x53, N_("OnTrack DM6 Aux3")},
	{0x54, N_("OnTrackDM6")},
	{0x55, N_("EZ-Drive")},
	{0x56, N_("Golden Bow")},
	{0x5c, N_("Priam Edisk")},
	{0x61, N_("SpeedStor")},
	{0x63, N_("GNU HURD or SysV")},	/* GNU HURD or Mach or Sys V/386 (such as
ISC UNIX) */
	{0x64, N_("Novell Netware 286")},
	{0x65, N_("Novell Netware 386")},
	{0x70, N_("DiskSecure Multi-Boot")},
	{0x75, N_("PC/IX")},
	{0x80, N_("Old Minix")},	/* Minix 1.4a and earlier */
	{0x81, N_("Minix / old Linux")},/* Minix 1.4b and later */
	{0x82, N_("Linux swap / Solaris")},
	{0x83, N_("Linux")},
	{0x84, N_("OS/2 hidden C: drive")},
	{0x85, N_("Linux extended")},
	{0x86, N_("NTFS volume set")},
	{0x87, N_("NTFS volume set")},
	{0x88, N_("Linux plaintext")},
	{0x8e, N_("Linux LVM")},
	{0x93, N_("Amoeba")},
	{0x94, N_("Amoeba BBT")},	/* (bad block table) */
	{0x9f, N_("BSD/OS")},		/* BSDI */
	{0xa0, N_("IBM Thinkpad hibernation")},
	{0xa5, N_("FreeBSD")},		/* various BSD flavours */
	{0xa6, N_("OpenBSD")},
	{0xa7, N_("NeXTSTEP")},
	{0xa8, N_("Darwin UFS")},
	{0xa9, N_("NetBSD")},
	{0xab, N_("Darwin boot")},
	{0xb7, N_("BSDI fs")},
	{0xb8, N_("BSDI swap")},
	{0xbb, N_("Boot Wizard hidden")},
	{0xbe, N_("Solaris boot")},
	{0xbf, N_("Solaris")},
	{0xc1, N_("DRDOS/sec (FAT-12)")},
	{0xc4, N_("DRDOS/sec (FAT-16 < 32M)")},
	{0xc6, N_("DRDOS/sec (FAT-16)")},
	{0xc7, N_("Syrinx")},
	{0xda, N_("Non-FS data")},
	{0xdb, N_("CP/M / CTOS / ...")},/* CP/M or Concurrent CP/M or
					   Concurrent DOS or CTOS */
	{0xde, N_("Dell Utility")},	/* Dell PowerEdge Server utilities */
	{0xdf, N_("BootIt")},		/* BootIt EMBRM */
	{0xe1, N_("DOS access")},	/* DOS access or SpeedStor 12-bit FAT
					   extended partition */
	{0xe3, N_("DOS R/O")},		/* DOS R/O or SpeedStor */
	{0xe4, N_("SpeedStor")},	/* SpeedStor 16-bit FAT extended
					   partition < 1024 cyl. */
	{0xeb, N_("BeOS fs")},
	{0xee, N_("GPT")},		/* Intel EFI GUID Partition Table */
	{0xef, N_("EFI (FAT-12/16/32)")},/* Intel EFI System Partition */
	{0xf0, N_("Linux/PA-RISC boot")},/* Linux/PA-RISC boot loader */
	{0xf1, N_("SpeedStor")},
	{0xf4, N_("SpeedStor")},	/* SpeedStor large partition */
	{0xf2, N_("DOS secondary")},	/* DOS 3.3+ secondary */
	{0xfb, N_("VMware VMFS")},
	{0xfc, N_("VMware VMKCORE")},	/* VMware kernel dump partition */
	{0xfd, N_("Linux raid autodetect")},/* New (2.2.x) raid partition with
							autodetect using persistent
							superblock */
	{0xfe, N_("LANstep")},		/* SpeedStor >1024 cyl. or LANstep */
	{0xff, N_("BBT")},		/* Xenix Bad Block Table */
	{ 0, 0 }
};
int isDVDDevice(const char *devname)
{
	if (strncmp(DVD_DEVICE_PATTERN, devname+5, sizeof(DVD_DEVICE_PATTERN))
		&& strncmp(SATA_DVD_PATTERN, devname+5, strlen(SATA_DVD_PATTERN)))
	{
		return 0;
	}

	return 1;
}

/* Function: is_probably_full_disk
 * Description: Check whether the device is a full disk.
 * Input:	name - device path name
 * Output:  none
 * Return:	non-zero if it is a full disk, zero if not
 */
int
is_probably_full_disk(const char *name)
{
#ifdef HDIO_GETGEO
	struct hd_geometry geometry;
	int fd, i = 0;

	if (isDVDDevice(name))
	{
		/* flow go down! */
	}
	else
	{
		fd = open(name, O_RDONLY);
		if (fd >= 0)
		{
			i = ioctl(fd, HDIO_GETGEO, &geometry);
			close(fd);
		}
		else
		{
			fprintf(stderr, "is_probably_full_disk: open disk %s fail, errno = %d(%s)\n", name, errno, strerror(errno));
		}
		if (i==0)
		{
			return (fd >= 0 && geometry.start == 0);
		}
	}
#endif
	/*
	 * The "silly heuristic" is still sexy for us, because
	 * for example Xen doesn't implement HDIO_GETGEO for virtual
	 * block devices (/dev/xvda).
	 *
	 * -- kzak@redhat.com (23-Feb-2006)
	 */
	while (*name)
	{
		name++;
	}
	return !isdigit(name[-1]);
}

/* A valid partition table sector ends in 0x55 0xaa */

/* Function: part_table_flag
 * Description: Get the end flag of a valid partition table sector.
 * Input:	b - the partition table sector buffer
 * Output:  none
 * Return:	the end flag of the sector
 */
static unsigned int
part_table_flag(unsigned char *b)
{
	return ((unsigned int) b[510]) + (((unsigned int) b[511]) << 8);
}

/* Function: valid_part_table_flag
 * Description: Check whether the partition table sector's flag is valid.
 * Input:	b - the partition table sector buffer
 * Output:  none
 * Return:	none-zero if it is valid, zero if not
 */
static int
valid_part_table_flag(unsigned char *b)
{
	return (b[510] == 0x55 && b[511] == 0xaa);
}

/* Function: write_part_table_flag
 * Description: Write the end flag to the partition table sector.
 * Input:	b - the partition table sector buffer
 * Output:	b - the partition table sector buffer
 * Return:	none
 */
static void
write_part_table_flag(unsigned char *b)
{
	b[510] = 0x55;
	b[511] = 0xaa;
}

/* start_sect and nr_sects are stored little endian on all machines */
/* moreover, they are not aligned correctly */

/* Function: store4_little_endian
 * Description: Store the starting sector or the partition size in sectors.
 * Input:	val - the starting sector or the partition size in sectors
 * Output:	cp - little endian value of the starting sector or the partition size in sectors
 * Return:	none
 */
static void
store4_little_endian(unsigned char *cp, unsigned int val)
{
	cp[0] = (val & 0xff);
	cp[1] = ((val >> 8) & 0xff);
	cp[2] = ((val >> 16) & 0xff);
	cp[3] = ((val >> 24) & 0xff);
}

/* Function: read4_little_endian
 * Description: Read the starting sector or the partition size in sectors.
 * Input:	cp - little endian value of the starting sector or the partition size in sectors
 * Output:	none
 * Return:	the starting sector, or the partition size in sectors
 */
static unsigned int
read4_little_endian(const unsigned char *cp)
{
	return (unsigned int)(cp[0]) + ((unsigned int)(cp[1]) << 8)
		+ ((unsigned int)(cp[2]) << 16)
		+ ((unsigned int)(cp[3]) << 24);
}

/* Function: set_start_sect
 * Description: Set the starting sector to the partition table.
 * Input:	start_sect - the starting sector
 * Output:	p - the partition table
 * Return:	none
 */
static void
set_start_sect(struct partition *p, unsigned int start_sect)
{
	store4_little_endian(p->start4, start_sect);
}

/* Function: get_start_sect
 * Description: Get the starting sector from the partition table.
 * Input:	p - the partition table
 * Output:	none
 * Return:	the starting sector
 */
static unsigned long long
get_start_sect(struct partition *p)
{
	return read4_little_endian(p->start4);
}

/* Function: set_nr_sects
 * Description: Set the partition size in sectors to the partition table.
 * Input:	nr_sects - the partition size in sectors
 * Output:	p - the partition table
 * Return:	none
 */
static void
set_nr_sects(struct partition *p, unsigned long long nr_sects)
{
	store4_little_endian(p->size4, nr_sects);
}

/* Function: get_nr_sects
 * Description: Get the partition size from the partition table.
 * Input:	p - the partition table
 * Output:	none
 * Return:	the partition size in sectors
 */
static unsigned long long
get_nr_sects(struct partition *p)
{
	return read4_little_endian(p->size4);
}

/*
 * Raw disk label. For DOS-type partition tables the MBR,
 * with descriptions of the primary partitions.
 */
unsigned char MBRbuffer[MAX_SECTOR_SIZE];

/*
 * per partition table entry data
 *
 * The four primary partitions have the same sectorbuffer (MBRbuffer)
 * and have NULL ext_pointer.
 * Each logical partition table entry has two pointers, one for the
 * partition and one link to the next one.
 */
struct pte {
	struct partition *part_table;	/* points into sectorbuffer */
	struct partition *ext_pointer;	/* points into sectorbuffer */
	char changed;			/* boolean */
	unsigned long long offset;	/* disk sector number */
	unsigned char *sectorbuffer;	/* disk sector contents */
} ptes[MAXIMUM_PARTS];

const char *disk_device; /* the disk */
int	ext_index,			/* the prime extended partition */
	dos_compatible_flag = ~0,
	partitions = 4;			/* maximum partition + 1 */

unsigned int	pt_heads, pt_sectors;
unsigned int	kern_heads, kern_sectors;

unsigned long long sector_offset = 1, extended_offset = 0, sectors;

unsigned int	heads,
	cylinders,
	sector_size = DEFAULT_SECTOR_SIZE,
	units_per_sector = 1,
	display_in_cyl_units = 1;

unsigned long long total_number_of_sectors;

#define dos_label 1

/* Function: fatals
 * Description:	To process vary failure.
 * Input:	why - why failure
 * Output:	none
 * Return:	none
 */
static void
fatals(enum failure why)
{
	char	error[LINE_LENGTH],
			*message = error;

	switch (why)
	{
		case unable_to_open:
			snprintf(error, sizeof(error),
					_("Unable to open %s\n"), disk_device);
			break;
		case unable_to_read:
			snprintf(error, sizeof(error),
					_("Unable to read %s\n"), disk_device);
			break;
		case unable_to_seek:
			snprintf(error, sizeof(error),
					_("Unable to seek on %s\n"),disk_device);
			break;
		case unable_to_write:
			snprintf(error, sizeof(error),
					_("Unable to write %s\n"), disk_device);
			break;
		case ioctl_error:
			snprintf(error, sizeof(error),
					_("BLKGETSIZE ioctl failed on %s\n"),
					disk_device);
			break;
		case out_of_memory:
			message = _("Unable to allocate any more memory\n");
			break;
		default:
			message = _("Fatal error\n");
	}

	fputc('\n', stderr);
	fputs(message, stderr);
}

/* Function: seek_sector
 * Description: To seek the sector offset in the disk device.
 * Input:	fd - the file descriptor of the disk device
 *			secno - the sector number in the disk device
 * Output:	none
 * Return:	none
 */
static void
seek_sector(int fd, unsigned long long secno)
{
	off_t offset = (off_t) secno * sector_size;
	if (lseek(fd, offset, SEEK_SET) == (off_t) -1)
	{
		fatals(unable_to_seek);
	}
}

/* Function: read_sector
 * Description: To read the certain sector content in the disk device.
 * Input:	fd - the file descriptor of the disk device
 *			secno - the sector number in the disk device
 * Output:	buf - the read sector content
 * Return:	none
 */
static void
read_sector(int fd, unsigned long long secno, unsigned char *buf)
{
	seek_sector(fd, secno);
	if (read(fd, buf, sector_size) != sector_size)
	{
		fatals(unable_to_read);
	}
}

/* Function: write_sector
 * Description: Write the sector content in the disk device.
 * Input:	fd - the file descriptor of the disk device
 *			secno - the sector number in the disk device
 *			buf - the sector content to write
 * Output:	none
 * Return:	none
 */
static void
write_sector(int fd, unsigned long long secno, unsigned char *buf)
{
	seek_sector(fd, secno);
	if (write(fd, buf, sector_size) != sector_size)
	{
		fatals(unable_to_write);
	}
}

/* Function: read_pte
 * Description: Allocate a buffer and read a partition table sector.
 * Input:	fd - the file descriptor of the disk device
 *			pno - the partition table entry index
 *			offset - the sector number in the disk device
 * Output:	none
 * Return:	none
 */
static void
read_pte(int fd, int pno, unsigned long long offset)
{
	struct pte *pe = &ptes[pno];

	pe->offset = offset;
	pe->sectorbuffer = malloc(sector_size);
	if (NULL == pe->sectorbuffer)
	{
		fatals(out_of_memory);
		return ;
	}
	read_sector(fd, offset, pe->sectorbuffer);
	pe->changed = 0;
	pe->part_table = pe->ext_pointer = NULL;
	FDISK_INFO("read_pte: pno = %d\n", pno);
}

/* Function: get_partition_start
 * Description: Get the first sector of the partition in LBA.
 * Input:	pte - the partition table sector
 * Output:	none
 * Return:	the first sector of the partition in LBA
 */
static unsigned long long
get_partition_start(struct pte *pe)
{
	return pe->offset + get_start_sect(pe->part_table);
}

/* Function: is_garbage_table
 * Description: Check whether the partition table is malformed. According to the first byte of the partition table,
 *			which indicates the status of the partition: 0x80 = bootable, 0x00 = non-bootable, other = malformed.
 * Input:	none
 * Output:	none
 * Return:	non-zero if the partition table is malformed, zero if not
 */
int
is_garbage_table(void)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		struct pte *pe = &ptes[i];
		struct partition *p = pe->part_table;

		FDISK_INFO("is_garbage_table: i = %d, boot_ind = 0x%x\n", i, p->boot_ind);
		if (p->boot_ind != 0 && p->boot_ind != 0x80)
		{
			return 1;
		}
	}
	return 0;
}

/* Function: get_sys_types
 * Description: Get the partition type, currently we assume to use the i386 system types.
 * Input:	none
 * Output:	none
 * Return:	the partition type
 */
static struct systypes *
get_sys_types(void)
{
	return (i386_sys_types);
}

/* Function: partition_type
 * Description: Get the readable string of the partition type.
 * Input:	type - the partition type index
 * Output:	none
 * Return:	the readable string of the partition type
 */
char *
partition_type(unsigned char type)
{
	int i;
	struct systypes *types = get_sys_types();

	for (i=0; types[i].name; i++)
	{
		if (types[i].type == type)
		{
			return _(types[i].name);
		}
	}

	return NULL;
}

/* Function: set_partition
 * Description: Set a 16-bytes partition table.
 * Input:	i - the partition table entry index
 *			doext - whether to set a extend partition
 *			start - the first sector of the partition in LBA
 *			stop - the end sector of the partition in LBA
 *			sysid - the partition type id
 * Output:	none
 * Return:	none
 */
static void
set_partition(int i, int doext, unsigned long long start, unsigned long long stop, int sysid)
{
	struct partition *p;
	unsigned long long offset;

	if (doext)
	{
		p = ptes[i].ext_pointer;
		offset = extended_offset;
	}
	else
	{
		p = ptes[i].part_table;
		offset = ptes[i].offset;
	}
	p->boot_ind = 0;
	p->sys_ind = sysid;
	set_start_sect(p, start - offset);
	set_nr_sects(p, stop - start + 1);
	if (dos_compatible_flag && (start/(sectors*heads) > 1023))
	{
		start = heads*sectors*1024 - 1;
	}
	set_hsc(p->head, p->sector, p->cyl, start);
	if (dos_compatible_flag && (stop/(sectors*heads) > 1023))
	{
		stop = heads*sectors*1024 - 1;
	}
	set_hsc(p->end_head, p->end_sector, p->end_cyl, stop);
	ptes[i].changed = 1;
}

/* Function: update_units
 * Description: Update the units to display.
 * Input:	none
 * Output:	none
 * Return:	none
 */
static void
update_units(void)
{
	int cyl_units = heads * sectors;

	if (display_in_cyl_units && cyl_units)
	{
		units_per_sector = cyl_units;
	}
	else
	{
		units_per_sector = 1; 	/* in sectors */
	}
}

/* Function: clear_partition
 * Description: Clear a partition table entry.
 * Input:	p - the dirty partition table
 * Output:	p - the clean partition table
 * Return:	none
 */
static void
clear_partition(struct partition *p)
{
	if (!p)
	{
		return;
	}
	p->boot_ind = 0;
	p->head = 0;
	p->sector = 0;
	p->cyl = 0;
	p->sys_ind = 0;
	p->end_head = 0;
	p->end_sector = 0;
	p->end_cyl = 0;
	set_start_sect(p,0);
	set_nr_sects(p,0);
}

/* Function: delete_partition
 * Description: Delete a partition.
 * Input:	i - the partition table entry index
 * Output:	none
 * Return:	none
 */
static void
delete_partition(int i)
{
	struct pte *pe = &ptes[i];
	struct partition *p = pe->part_table;
	struct partition *q = pe->ext_pointer;

/* Note that for the fifth partition (i == 4) we don't actually
 * decrement partitions.
 */
	pe->changed = 1;

	if (i < 4)
	{
		if (IS_EXTENDED (p->sys_ind) && i == ext_index)
		{
			partitions = 4;
			ptes[ext_index].ext_pointer = NULL;
			extended_offset = 0;
		}
		clear_partition(p);
		return;
	}

	if (!q->sys_ind && i > 4)
	{
		/* the last one in the chain - just delete */
		--partitions;
		--i;
		clear_partition(ptes[i].ext_pointer);
		ptes[i].changed = 1;
	}
	else
	{
		/* not the last one - further ones will be moved down */
		if (i > 4)
		{
			/* delete this link in the chain */
			p = ptes[i-1].ext_pointer;
			*p = *q;
			set_start_sect(p, get_start_sect(q));
			set_nr_sects(p, get_nr_sects(q));
			ptes[i-1].changed = 1;
		}
		else if (partitions > 5)
		{    /* 5 will be moved to 4 */
			/* the first logical in a longer chain */
			struct pte *pe = &ptes[5];

			if (pe->part_table) /* prevent SEGFAULT */
				set_start_sect(pe->part_table,
							get_partition_start(pe) -
							extended_offset);
			pe->offset = extended_offset;
			pe->changed = 1;
		}

		if (partitions > 5) {
			partitions--;
			while (i < partitions) {
				ptes[i] = ptes[i+1];
				i++;
			}
		} else
			/* the only logical: clear only */
			clear_partition(ptes[i].part_table);
	}
}

/* Function: read_extended
 * Description: Read all logical partitions information.
 * Input:	fd - the file descriptor of the disk device
 *			ext - the extended partition entry index
 * Output:	none
 * Return:	none
 */
static void
read_extended(int fd, int ext)
{
	int i;
	struct pte *pex;
	struct partition *p, *q;

	ext_index = ext;
	pex = &ptes[ext];
	pex->ext_pointer = pex->part_table;

	p = pex->part_table;
	if (!get_start_sect(p))
	{
		fprintf(stderr, _("Bad offset in primary extended partition\n"));
		return;
	}

	while (IS_EXTENDED (p->sys_ind))
	{
		struct pte *pe = &ptes[partitions];

		if (partitions >= MAXIMUM_PARTS)
		{
			/* This is not a Linux restriction, but
				this program uses arrays of size MAXIMUM_PARTS.
			   Do not try to `improve' this test. */
			struct pte *pre = &ptes[partitions-1];

			fprintf(stderr,
				_("Warning: omitting partitions after #%d.\n"
					"They will be deleted "
					"if you save this partition table.\n"),
				partitions);
			clear_partition(pre->ext_pointer);
			pre->changed = 1;
			return;
		}

		read_pte(fd, partitions, extended_offset + get_start_sect(p));

		if (!extended_offset)
		{
			extended_offset = get_start_sect(p);
		}

		q = p = pt_offset(pe->sectorbuffer, 0);
		for (i = 0; i < 4; i++, p++)
		{
			if (get_nr_sects(p))
			{
				if (IS_EXTENDED (p->sys_ind))
				{
					if (pe->ext_pointer)
					{
						fprintf(stderr,
								_("Warning: extra link "
									"pointer in partition table"
									" %d\n"), partitions + 1);
					}
					else
					{
						pe->ext_pointer = p;
					}
				}
				else if (p->sys_ind)
				{
					if (pe->part_table)
					{
						fprintf(stderr,
								_("Warning: ignoring extra "
									"data in partition table"
									" %d\n"), partitions + 1);
					}
					else
					{
						pe->part_table = p;
					}
				}
			}
		}

		/* very strange code here... */
		if (!pe->part_table)
		{
			if (q != pe->ext_pointer)
			{
				pe->part_table = q;
			}
			else
			{
				pe->part_table = q + 1;
			}
		}
		if (!pe->ext_pointer)
		{
			if (q != pe->part_table)
			{
				pe->ext_pointer = q;
			}
			else
			{
				pe->ext_pointer = q + 1;
			}
		}

		p = pe->ext_pointer;
		partitions++;
		FDISK_INFO("read_extended: partitions = %d\n", partitions);
	}

	/* remove empty links */
 remove:
	for (i = 4; i < partitions; i++)
	{
		struct pte *pe = &ptes[i];

		if (!get_nr_sects(pe->part_table) &&
			(partitions > 5 || ptes[4].part_table->sys_ind))
		{
			printf("omitting empty partition (%d)\n", i+1);
			delete_partition(i);
			goto remove; 	/* numbering changed */
		}
	}
}

/* Function: get_sectorsize
 * Description: Get the sector size of the disk device.
 * Input:	fd - the file descriptor of the disk device
 * Output:	none
 * Return:	none
 */
static void
get_sectorsize(int fd)
{
	int arg;

	if (blkdev_get_sector_size(fd, &arg) == 0)
	{
		sector_size = arg;
	}
	if (sector_size != DEFAULT_SECTOR_SIZE)
	{
		printf(_("Note: sector size is %d (not %d)\n"), sector_size, DEFAULT_SECTOR_SIZE);
	}
}

/* Function: get_kernel_geometry
 * Description: Get the geometry of the disk device via ioctl command.
 * Input:	fd - the file descriptor of the disk device
 * Output:	none
 * Return:	none
 */
static void
get_kernel_geometry(int fd)
{
#ifdef HDIO_GETGEO
	struct hd_geometry geometry;

	if (!ioctl(fd, HDIO_GETGEO, &geometry))
	{
		kern_heads = geometry.heads;
		kern_sectors = geometry.sectors;
		/* never use geometry.cylinders - it is truncated */
	}
#endif
}

/* Function: get_partition_table_geometry
 * Description: Get the geometry of the disk device from the partition tables.
 * Input:	none
 * Output:	none
 * Return:	none
 */
static void
get_partition_table_geometry(void)
{
	unsigned char *bufp = MBRbuffer;
	struct partition *p;
	int i, h, s, hh, ss;
	int first = 1;
	int bad = 0;

	if (!(valid_part_table_flag(bufp)))
	{
		return;
	}

	hh = ss = 0;
	for (i=0; i<4; i++)
	{
		p = pt_offset(bufp, i);
		if (p->sys_ind != 0)
		{
			h = p->end_head + 1;
			s = (p->end_sector & 077);
			if (first)
			{
				hh = h;
				ss = s;
				first = 0;
			}
			else if (hh != h || ss != s)
			{
				bad = 1;
			}
		}
	}

	if (!first && !bad)
	{
		pt_heads = hh;
		pt_sectors = ss;
	}
}

/* Function: get_geometry
 * Description: Get the geometry of the disk device.
 * Input:	fd - the file descriptor of the disk device
 * Output:	g - the device geometry information
 * Return:	none
 */
void
get_geometry(int fd, struct geom *g)
{
	int sec_fac;
	unsigned long long llsectors, llcyls;

	get_sectorsize(fd);
	sec_fac = sector_size / 512;
	heads = cylinders = sectors = 0;
	kern_heads = kern_sectors = 0;
	pt_heads = pt_sectors = 0;

	get_kernel_geometry(fd);
	get_partition_table_geometry();

	heads = pt_heads ? pt_heads :
		kern_heads ? kern_heads : 255;
	sectors = pt_sectors ? pt_sectors :
		kern_sectors ? kern_sectors : 63;

	if (blkdev_get_sectors(fd, &llsectors) == -1)
	{
		llsectors = 0;
	}

	total_number_of_sectors = llsectors;

	sector_offset = 1;
	if (dos_compatible_flag)
	{
		sector_offset = sectors;
	}

	llcyls = total_number_of_sectors / (heads * sectors * sec_fac);
	cylinders = llcyls;
	if (cylinders != llcyls)	/* truncated? */
	{
		cylinders = ~0;
	}

	if (g)
	{
		g->heads = heads;
		g->sectors = sectors;
		g->cylinders = cylinders;
	}
}

/* Function: get_boot
 * Description: Read MBR.
 * Input:	fd - the file descriptor of the disk device
 * Output:	none
 * Return:	-1: no 0xaa55 flag present; 0: found; 1: I/O error
 */
int
get_boot(int fd)
{
	int i;

	partitions = 4;
	ext_index = 0;
	extended_offset = 0;

	for (i = 0; i < 4; i++)
	{
		struct pte *pe = &ptes[i];

		pe->part_table = pt_offset(MBRbuffer, i);
		pe->ext_pointer = NULL;
		pe->offset = 0;
		pe->sectorbuffer = MBRbuffer;
		pe->changed = 0;
	}

	memset(MBRbuffer, 0, 512);

	if (512 != read(fd, MBRbuffer, 512))
	{
		return 1;
	}

	get_geometry(fd, NULL);

	update_units();

	if (!valid_part_table_flag(MBRbuffer))
	{
		return -1;
	}

	for (i = 0; i < 4; i++)
	{
		struct pte *pe = &ptes[i];

		if (IS_EXTENDED (pe->part_table->sys_ind))
		{
			if (partitions != 4)
			{
				fprintf(stderr, _("Ignoring extra extended "
							"partition %d\n"), i + 1);
			}
			else
			{
				FDISK_INFO("get_boot: is extended, i = %d, partitions = %d\n", i, partitions);
				read_extended(fd, i);
			}
		}
	}

	for (i = 3; i < partitions; i++)
	{
		struct pte *pe = &ptes[i];

		if (!valid_part_table_flag(pe->sectorbuffer))
		{
			fprintf(stderr,
					_("Warning: invalid flag 0x%04x of partition "
						"table %d will be corrected by w(rite)\n"),
					part_table_flag(pe->sectorbuffer), i + 1);
			pe->changed = 1;
		}
	}

	return 0;
}

/* Function: get_disk_capacity
 * Description: Get the disk capacity.
 * Input:	devInfo - the disk capacity unassigned
 * Output:	devInfo - the disk device information
 * Return:	none
 */
void
get_disk_capacity(DEVICE_INFO *devInfo)
{
	long long bytes = (total_number_of_sectors << 9);
	long megabytes = bytes/1000000;

	devInfo->megabytes = megabytes;
}

/* Function: wrong_p_order
 * Description: Check whether partition entries are ordered by their starting positions.
 *			Two separate checks: primary and logical partitions.
 * Input:	none
 * Output:	prev - the last partition in correct order
 * Return:	0 if OK; i if partition i should have been earlier.
 */
int
wrong_p_order(int *prev)
{
	struct pte *pe;
	struct partition *p;
	unsigned int last_p_start_pos = 0, p_start_pos;
	int i, last_i = 0;

	for (i = 0 ; i < partitions; i++)
	{
		if (i == 4)
		{
			last_i = 4;
			last_p_start_pos = 0;
		}
		pe = &ptes[i];
		if ((p = pe->part_table)->sys_ind)
		{
			p_start_pos = get_partition_start(pe);

			if (last_p_start_pos > p_start_pos)
			{
				if (prev)
				{
					*prev = last_i;
				}
				return i;
			}

			last_p_start_pos = p_start_pos;
			last_i = i;
		}
	}
	return 0;
}

/* Function: is_cleared_partition
 * Description: Check whether the special partition table is cleared.
 * Input:	p - the partition table
 * Output:	none
 * Return:	non-zero if it is cleared, zero if not
 */
int
is_cleared_partition(struct partition *p)
{
	return !(!p || p->boot_ind || p->head || p->sector || p->cyl ||
		p->sys_ind || p->end_head || p->end_sector || p->end_cyl ||
		get_start_sect(p) || get_nr_sects(p));
}

/* Function: freeLogicalPtes
 * Description: Free the memory allocated to the sector buffer of the logical partition table entries.
 * Input:	none
 * Output:	none
 * Return:	none
 */
void
freeLogicalPtes()
{
	int i;
	struct pte *p;

	for (i = 4; i < partitions; i++)
	{
		p = &ptes[i];
		free(p->sectorbuffer);
		FDISK_INFO("freeLogicalPtes: i = %d, partitions = %d\n", i, partitions);
	}
}
#if 0
/* Function: get_disk_info
 * Description: Get the disk and it's partitions(if exist) information.
 * Input:	devInfo - the unassigned disk device information
 * Output:	devInfo - the assigned disk device information
 * Return:	0 if success, -1 if failure
 */
static int
get_disk_info(DEVICE_INFO *devInfo)
{
	struct partition *p;
	char *type;
	int i;
	char pname[32];
	PARTITION_INFO *partInfo = NULL;
	int fd;

	get_disk_capacity(devInfo);

	if (is_garbage_table())
	{
		printf(_("This doesn't look like a partition table\n"
			"Probably you selected the wrong device.\n\n"));
	}

	FDISK_INFO("get_disk_info: partitions = %d\n", partitions);
	for (i = 0; i < partitions; i++)
	{
		struct pte *pe = &ptes[i];

		p = pe->part_table;
#ifdef FDISK_DBG
		FDISK_INFO("get_disk_info: i = %d, p = %p, is_cleared_partition = %d\n", i, p, is_cleared_partition(p));
		sleep(1);
#endif
		if (p && !is_cleared_partition(p))
		{
			unsigned int psects = get_nr_sects(p);
			unsigned int pblocks = psects;
			unsigned int podd = 0;

			if (sector_size < 1024)
			{
				pblocks /= (1024 / sector_size);
				podd = psects % (1024 / sector_size);
			}
			if (sector_size > 1024)
			{
				pblocks *= (sector_size / 1024);
			}

			if (!IS_EXTENDED(p->sys_ind))
			{
				sprintf(pname, "%s%d", devInfo->name, i+1);
				fd = open(pname, O_RDONLY);
				if (-1 == fd)
				{
					printf(_("!!!open %s fail, errno = %d(%s)\n"), pname, errno, strerror(errno));
					continue;
				}
				else
				{
					/* don't forget to reclaim file descriptor */
					close(fd);
				}

				partInfo = (PARTITION_INFO *)malloc(sizeof(PARTITION_INFO));
				if (NULL == partInfo)
				{
					printf(_("get_disk_info: allocate memory fail\n"));
					return -1;
				}

				memset((char *)partInfo, 0, sizeof(PARTITION_INFO));
				partInfo->megabytes = (unsigned long long)psects * sector_size / 1000000;
				partInfo->sysId = p->sys_ind;
				partInfo->partNumber = i+1;
				list_add(&devInfo->partitionList, &(partInfo->node));
			}
		}
	}

	/* Is partition table in disk order? It need not be, but... */
	/* partition table entries are not checked for correct order if this
	   is a sgi, sun or aix labeled disk... */
	if (dos_label && wrong_p_order(NULL))
	{
		printf(_("\nPartition table entries are not in disk order\n"));
	}

	/* !!!We allocate memory for logical partitions, so don't forget to free it. */
	freeLogicalPtes();

	return 0;
}
#endif

/* Function: fill_bounds
 * Description: Fill the first and end sector of each partition in LBA.
 * Input:	first - point to the unassigned first sector array
 *			last - point to the unassigned last sector array
 *			parts - total partitions of the disk device
 * Output:	first - point to the assigned first sector array
 *			last - point to the assigned last sector array
 * Return:	none
 */
static void
fill_bounds(unsigned long long first[], unsigned long long last[], int parts)
{
	int i;
	struct pte *pe = &ptes[0];
	struct partition *p;

	for (i = 0; i < parts; pe++,i++)
	{
		p = pe->part_table;
		if (!p->sys_ind || IS_EXTENDED (p->sys_ind))
		{
			first[i] = 0xffffffff;
			last[i] = 0;
		}
		else
		{
			first[i] = get_partition_start(pe);
			last[i] = first[i] + get_nr_sects(p) - 1;
		}
	}
}

/* Function: addPartition
 * Description: Add a partition on the disk device.
 * Input:	n - the partition table entry index
 *			sys - the partition type id
 *			partCyls - partition size in cylinders
 * Output:	none
 * Return:	none
 */
static void
addPartition(int n, int sys, unsigned long long partCyls, int bPartAligned/* need partion aligned*/)
{
	int i, read = 0;
	struct partition *q = ptes[ext_index].part_table;
	long long llimit;
	unsigned long long start, stop = 0, limit, temp,
		first[partitions], last[partitions];

	fill_bounds(first, last, partitions);
	if (n < 4)
	{
		start = sector_offset;
		if (display_in_cyl_units || !total_number_of_sectors)
		{
			llimit = heads * sectors * cylinders - 1;
		}
		else
		{
			llimit = total_number_of_sectors - 1;
		}
		limit = llimit;
		if (limit != llimit)
		{
			limit = 0x7fffffff;
		}
		if (extended_offset)
		{
			first[ext_index] = extended_offset;
			last[ext_index] = get_start_sect(q) + get_nr_sects(q) - 1;
		}
	}
	else
	{
		start = extended_offset + sector_offset;
		limit = get_start_sect(q) + get_nr_sects(q) - 1;
	}
	if (display_in_cyl_units)
	{
		for (i = 0; i < partitions; i++)
		{
			first[i] = (cround(first[i]) - 1) * units_per_sector;
		}
	}

	do
	{
		temp = start;
		for (i = 0; i < partitions; i++)
		{
			unsigned long long lastplusoff;

			if (start == ptes[i].offset)
			{
				start += sector_offset;
			}
			lastplusoff = last[i] + ((n<4) ? 0 : sector_offset);
			if (start >= first[i] && start <= lastplusoff)
			{
				start = lastplusoff + 1;
			}
		}
		if (start > limit)
		{
			break;
		}
		if (start >= temp+units_per_sector && read)
		{
			printf(_("Sector %llu is already allocated\n"), temp);
			temp = start;
			read = 0;
		}
		if (!read && start == temp)
		{
			unsigned int i = start;

			start = cround(i);
			if (display_in_cyl_units)
			{
				start = (start - 1) * units_per_sector;
				if (start < i)
				{
					start = i;
				}
			}
			read = 1;
		}
	}
	while (start != temp || !read);

	for (i = 0; i < partitions; i++)
	{
		struct pte *pe = &ptes[i];

		if (start < pe->offset && limit >= pe->offset)
		{
			limit = pe->offset - 1;
		}
		if (start < first[i] && limit >= first[i])
		{
			limit = first[i] - 1;
		}
	}

	if (start > limit)
	{
		printf(_("No free sectors available\n"));
		if (n > 4)
		{
			partitions--;
		}
		return;
	}
	if (cround(start) == cround(limit))
	{
		stop = limit;
	}
	else
	{
		stop = cround(start) + partCyls;
		if (display_in_cyl_units)
		{
			stop = stop * units_per_sector - 1;
			if (stop >limit)
			{
				stop = limit;
			}
		}
	}
	/*chenxc add, supporting partion align*/
	if(1 == bPartAligned)
	{
		if(0 == n)
		{
            /* 第一个分区默认从2048开始，预留头 */
            start = 2048;
            sector_offset = start;
		}
		stop -= 1;
	}

	printf("Disk Part %d, start %llu, stop %llu\n", n, start, stop);
	/*chenxc add end*/

	set_partition(n, 0, start, stop, sys);

	return;
}

/* Function: reread_partition_table
 * Description: Synchronize the disk device's partition tables.
 * Input:	fd - the file descriptor of the disk device
 * Output:	none
 * Return:	none
 */
int
reread_partition_table(int fd)
{
	int i;
	struct stat statbuf;

	i = fstat(fd, &statbuf);
	if (i == 0 && S_ISBLK(statbuf.st_mode))
	{
		printf(_("Calling ioctl() to re-read partition table.\n"));
		i = ioctl(fd, BLKRRPART);
	}

	if (i)
	{
		printf(_("\nWARNING: Re-reading the partition table "
					"failed with error %d: %s.\n"
					"The kernel still uses the old table.\n"
					"The new table will be used "
					"at the next reboot.\n"),
				errno, strerror(errno));
        return -1;
	}

	return 0;
}

/* Function: writeTable
 * Description: Write the partition tables to the disk device, we only support maximum four primary partitions.
 * Input:	fd - the file descriptor of the disk device
 * Output:	none
 * Return:	none
 */
static void
writeTable(int fd)
{
	struct pte *pe = &ptes[0];

	write_part_table_flag(pe->sectorbuffer);
	write_sector(fd, pe->offset, pe->sectorbuffer);

	printf(_("The partition table has been altered!\n\n"));

	return;
}
#if 0
/* Function: getDevAttr
 * Description: Get the device attribute.
 * Input:	devname - the device path name
 *			devInfo - the unassigned device attribute
 * Output:	devInfo - the assigned device attribute information
 * Return:	0 if success, -1 if fail
 */
static int
getDevAttr(const char *devname, DEVICE_INFO *devInfo)
{
	struct stat stats;

	if (lstat(devname, &stats))
	{
		return -1;
	}

	if ((!S_ISBLK(stats.st_mode)) && (!S_ISCHR(stats.st_mode)))
	{
		return -1;
	}

	devInfo->maj = major(stats.st_rdev);
	devInfo->min = minor(stats.st_rdev);
	if (S_ISBLK(stats.st_mode))
	{
		devInfo->type = BLK_DEVICE;
	}
	else if (S_ISCHR(stats.st_mode))
	{
		devInfo->type = CHR_DEVICE;
	}
	devInfo->mtime = stats.st_mtime;

	return 0;
}

/* Function: try2parse
 * Description: Try to get a device attribute information.
 * Input:	devname - the device path name
 *			devInfo - the unassigned device attribute
 * Output:	devInfo - the assigned device attribute information
 * Return:	0 if success, -1 if fail
 */
int
try2parse(const char *device, DEVICE_INFO *devInfo)
{
	int gb, fd;
	PARTITION_INFO *partInfo = NULL;

	disk_device = device;
	if (isDVDDevice(device))
	{
		getDevAttr(device, devInfo);
		partInfo = (PARTITION_INFO *)malloc(sizeof(PARTITION_INFO));
		if (NULL == partInfo)
		{
			return -1;
		}

		memset((char *)partInfo, 0, sizeof(PARTITION_INFO));
		partInfo->megabytes = 0;
		partInfo->sysId = 0;
		partInfo->partNumber = 0;
		lstAdd(&devInfo->partitionList, &(partInfo->node));
		return 0;
	}
	if ((fd = open(device, O_RDONLY)) >= 0)
	{
		gb = get_boot(fd);
		if ((gb > 0) /* I/O error */
			|| (gb < 0)) /* no DOS signature */
		{
			close(fd);
			return -1;
		}
		else
		{
			close(fd);
			getDevAttr(device, devInfo);
			return get_disk_info(devInfo);
		}
	}
	else
	{
		/* Ignore other errors, since we try IDE
			and SCSI hard disks which may not be
		   	installed on the system. */
		if (errno == EACCES)
		{
			fprintf(stderr, _("Cannot open %s\n"), device);
		}
		return -1;
	}
}

/*chenxc add, supporting partion align*/

/* Function: fdiskGetLglSectSize
 * Description: get the disk logical sector size
 * Input:	devname - the device path name
 *
 * Output:	null
 * Return:	return the device logical size.
 */
int fdiskGetLglSectSize(const char *devname)
{
	IDENTIFY_INFO identify_Info;
	unsigned short  word_106;
	unsigned char log_per_phys = 0;
	int bPartAligned = 0;

	memset((char *)&identify_Info, 0, sizeof(IDENTIFY_INFO));
	sata_get_identify_result(devname, &identify_Info);

	word_106 = identify_Info.reserved104[2];

	if ((word_106 & 0xc000) == 0x4000)
	{
		if (word_106 & (1 << 13))
			log_per_phys = word_106 & 0xf;

	}
	printf("log_per_phys %d\n ", log_per_phys);
	return (1 << log_per_phys) * 512;
}
#endif

/* Function: fdiskPartionAligned
 * Description: whether the device should be partion-aligned.
 * Input:	devname - the device path name
 *
 * Output:	null
 * Return:	1 if device need to be partion aligned , 0 if not
 */
int fdiskNeedPartionAligned(const char *devname)
{
	#if 0
	int logicalsize = fdiskGetLglSectSize(devname);
	int bPartAligned = (logicalsize > 512) ? 1 : 0;

	printf("%s Logical Sector Size %d, Need Partion Aligned? %s .\n", devname, logicalsize, (bPartAligned ? "YES": "NO"));

	return bPartAligned;
	#else
	return 1;
	#endif
}
/*chenxc add end*/

/* Function: fdisk_make_part
 * Description: Automatic to partition the disk in FAT32 file system, we support maximum four partitions.
 *		So we don't to part a logical partition.
 * Input:	devname - the device path name
 *			parts - total partitions
 * Output:	none
 * Return:	0 if success, -1 if fail
 */
int fdisk_make_part(const char *dev_name, unsigned int parts_num)
{
	if (parts_num > 4 || parts_num == 0)
	{
		return -1;
	}

	int i, j, fd, ret = 0;
	long long partCyls;

	/*chenxc add, supporting partion align*/
	int bPartAligned = fdiskNeedPartionAligned(dev_name);
	/*chenxc add end*/

	/* global variable */
	partitions = 4;
	memset(MBRbuffer, 0, 512);

	for (i = 0; i < 4; i++)
	{
		struct pte *pe = &ptes[i];

		pe->part_table = pt_offset(MBRbuffer, i);
		pe->ext_pointer = NULL;
		pe->offset = 0;
		pe->sectorbuffer = MBRbuffer;
		pe->changed = 0;
	}

	if ((fd = open(dev_name, O_RDWR)) < 0)
	{
		return -1;
	}

	disk_device = dev_name;
	get_geometry(fd, NULL);
	update_units();
	partCyls = cylinders / parts_num;

	j = parts_num - 1;
	for (i = 0;  i < j; i++)
	{
		addPartition(i, WIN95_FAT32_LBA, partCyls, bPartAligned);
	}
	addPartition(i, WIN95_FAT32_LBA, cylinders - (partCyls * j), bPartAligned);

	writeTable(fd);

	printf(_("Syncing disks.\n"));
	fsync(fd);
	close(fd);

	if ((fd = open(dev_name, O_RDWR)) < 0)
	{
		return -1;
	}

	ret = reread_partition_table(fd);

	close(fd);

	return ret;
}
#endif

/**
 * @brief 打印分区信息（调试用）
 */
void print_partition_info()
{
    printf("\npartition info:\n");
    printf("device: %s\n", disk_device);
    printf("cylinders: %u, heads: %u, sectors: %llu\n", cylinders, heads, sectors);
    for (int i = 0; i < 4; i++)
    {
        struct pte *pe = &ptes[i];
        printf("partid %d: type=%#x, offset:%llu start[head sector cyl]:[%u %u %u], end[head sector cyl]:[%u %u %u]\n", 
            i+1, pe->part_table->sys_ind, pe->offset, pe->part_table->head, pe->part_table->sector, pe->part_table->cyl, 
            pe->part_table->end_head, pe->part_table->end_sector, pe->part_table->end_cyl);
    }
    printf("\r");
}

/**
 * @brief 创建指定容量占比的分区
 * @param dev_name 设备名称 (e.g., "/dev/mmcblk0")
 * @param parts_num 分区数量 (1-4)
 * @param ratios 容量占比数组 (e.g., [20, 20, 60] 表示20%、20%、60%)
 * @return 成功返回0，失败返回-1
 */
int fdisk_make_part_ratio(const char *dev_name, unsigned int parts_num, const unsigned int *ratios)
{
    int fd = -1;
    unsigned int total_ratio = 0;
    long long int total_cyl = 0;
    int bPartAligned = 0;
    long long int allocated_cyl = 0;
    long long int part_cyl = 0;
    unsigned char default_type = 0;
    unsigned int i = 0;
    
    if (parts_num == 0 || parts_num > 4)
    {
        fprintf(stderr, "invalid parts num input %u,(1 ~ 4)\n", parts_num);
        return -1;
    }
    
    /* 验证比例数组 */
    if (ratios != NULL)
    {
        for (int i = 0; i < parts_num; i++) 
        {
            if (ratios[i] == 0)
            {
                fprintf(stderr, "ratios can not be 0\n");
                return -1;
            }
            total_ratio += ratios[i];
        }
        if (total_ratio == 0)
        {
            fprintf(stderr, "ratios sum can not be 0\n");
            return -1;
        }
    }

    /* 初始化分区表 MBR总是有4个条目 */
    partitions = 4;
    memset(MBRbuffer, 0, sizeof(MBRbuffer));

    for (int i = 0; i < 4; i++)
    {
        struct pte *pe = &ptes[i];
        pe->part_table = pt_offset(MBRbuffer, i);
        pe->ext_pointer = NULL;
        pe->offset = 0;
        pe->sectorbuffer = MBRbuffer;
        pe->changed = 0;
    }

    /* 打开设备并获取几何信息 */
    if ((fd = open(dev_name, O_RDWR)) < 0) {
        fprintf(stderr, "open device %s error\n", dev_name);
        return -1;
    }

    disk_device = dev_name;
    get_geometry(fd, NULL);
    
    /* 更新单位计算 */
    update_units();

    /* 调试信息打印 */
    print_partition_info();
    
    bPartAligned = fdiskNeedPartionAligned(dev_name);
    
    /* 计算总柱面数并验证 */
    total_cyl = cylinders;
    if (total_cyl <= 0)
    {
        fprintf(stderr, "invalid total_cyl %lld\n", total_cyl);
        close(fd);
        return -1;
    }
    
    /* TODO: 默认分区类型，后续可扩展 */
    default_type = WIN95_FAT32_LBA;
    
    /* 按比例创建分区 */
    for (i = 0; i < parts_num; i++)
    {
        /* 计算当前分区大小 */
        if (i == parts_num - 1)
        {
            /* 最后一个分区使用剩余所有空间 */
            part_cyl = total_cyl - allocated_cyl;
            printf("-->part_cyl:%lld total_cyl:%llu total_ratio:%u ratios[%d]:%u\n",part_cyl,total_cyl,total_ratio,i,ratios[i]);
        } 
        else if (ratios != NULL)
        {
            /* 按比例计算分区大小 */
            total_ratio = 0;
            for (int j = 0; j < parts_num; j++)
            {
                total_ratio += ratios[j];
            }
            part_cyl = (total_cyl * ratios[i]) / total_ratio;
            printf("-->part_cyl:%lld total_cyl:%llu total_ratio:%u ratios[%d]:%u\n",part_cyl,total_cyl,total_ratio,i,ratios[i]);
        } 
        else
        {
            /* 分区大小均分模式 */
            part_cyl = total_cyl / parts_num;
        }
        
        /* 验证分区大小 */
        if (part_cyl <= 0)
        {
            fprintf(stderr, "part %d invalid part_cyl (%lld)\n", i+1, part_cyl);
            close(fd);
            return -1;
        }
        
        /* 创建分区 */
        printf("create partition %d: part_cyl=%lld (%.2f%%)\n", 
               i+1, part_cyl, (double)part_cyl * 100.0 / total_cyl);

        /* 添加分区表信息 */
        addPartition(i, default_type, part_cyl, bPartAligned);
        
        /* 统计分区使用的柱面信息 */
        allocated_cyl += part_cyl;
    }

    /* 写入分区表 */
    writeTable(fd);

    /* 同步数据 */
    printf(_("Syncing disks.\n"));
    fsync(fd);

    /* 先关闭节点后续再打开，确保内核可以正确获取到信息 */
    close(fd);

    if ((fd = open(dev_name, O_RDWR)) < 0)
    {
        fprintf(stderr, "open device %s error\n", dev_name);
        return -1;
    }

    /* 强制内核重读分区表 */
    if (reread_partition_table(fd) != 0) {
        fprintf(stderr, "reread_partition_table error\n");
        print_partition_info();
        close(fd);
        return -1;
    }
    
    close(fd);
    print_partition_info();
    printf(_("Create Partition Success!!!\n"));
    
    return 0;
}

/**
 * @brief 获取设备节点容量
 * @param dev_name 设备名称 (e.g., "/dev/mmcblk0" "/dev/mmcblk0p1")
 * @param size     分区容量 单位MB
 * @return 成功返回0，失败返回-1
 */
int fdisk_get_dev_node_size(const char *dev_name,  unsigned int *size)
{
    int fd = -1;
    if(NULL == dev_name || NULL == size)
    {
        fprintf(stderr, "invalid param input\n");
        return -1;
    }
    /* 打开设备并获取几何信息 */
    if ((fd = open(dev_name, O_RDWR)) < 0) {
        fprintf(stderr, "open device %s error\n", dev_name);
        return -1;
    }

    disk_device = dev_name;
    get_geometry(fd, NULL);
    *size = cylinders * heads * sectors * sector_size / 1024 / 1024;
    close(fd);

    return 0;
}

/**
 * @brief 获取设备根节点分区数量
 * @param dev_name 设备名称 (e.g., "/dev/mmcblk0")
 * @param num      分区数量
 * @return 成功返回0，失败返回-1
 */
int fdisk_get_dev_node_partition_num(const char *device_path,  unsigned int *num)
{
    const char *dev_name = NULL;
    char sys_path[256] = {0};
    DIR *dir = NULL;
    char prefix[64] = {0};
    size_t prefix_len = 0;
    int partition_count = 0;
    struct dirent *entry;
    const char *suffix = NULL;
    int all_digits = 0;
    const char *char_num = NULL;
    const char *p_ptr = NULL;
    
    if(NULL == device_path || NULL == num)
    {
        fprintf(stderr, "invalid param input\n");
        return -1;
    }

    /* 检查设备路径格式 */
    if (strncmp(device_path, "/dev/", 5) != 0)
    {
        fprintf(stderr, "invalid param input: %s, msut be /dev/...\n", device_path);
        return -1;
    }
    
    /* 提取设备名 */
    dev_name = device_path + 5;
    
    /* 通过尝试打开sysfs目录检测根设备 */
    snprintf(sys_path, sizeof(sys_path), "/sys/block/%s", dev_name);
    
    /* 尝试打开设备目录 */
    dir = opendir(sys_path);
    if (!dir)
    {
        /* 检查错误类型：如果是分区设备会报ENOENT错误 */
        if (errno == ENOENT)
        {
            /* 检查是否为分区设备（包含"p"后接数字）*/
            p_ptr = strchr(dev_name, 'p');
            if (p_ptr && isdigit(*(p_ptr + 1)))
            {
                fprintf(stderr, "invalid param input, device_path: %s\n", device_path);
            }
            else
            {
                fprintf(stderr, "device_path: %s not exist\n", device_path);
            }
        }
        else
        {
            fprintf(stderr, "can not access: %s\n", device_path);
        }
        return -1;
    }
    
    /* 确定分区前缀（根据不同设备类型）*/
    if (strncmp(dev_name, "mmcblk", 6) == 0)
    {
        /* MMC设备 */
        snprintf(prefix, sizeof(prefix), "%sp", dev_name);
        prefix_len = strlen(prefix);
    }
    else
    {
        /* SATA设备 */
        strncpy(prefix, dev_name, sizeof(prefix)-1);
        prefix_len = strlen(dev_name);
    }
    
    /* 遍历目录统计分区 */
    while ((entry = readdir(dir)) != NULL)
    {
        /* 跳过当前目录跟上级目录 */
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
            continue;
        }
        
        /* 检查分区条目格式 */
        if (strncmp(entry->d_name, prefix, prefix_len) == 0)
        {
            suffix = entry->d_name + prefix_len;
            
            /* 验证后缀为纯数字 */
            all_digits = 1;
            for (char_num = suffix; *char_num; char_num++)
            {
                if (!isdigit(*char_num))
                {
                    all_digits = 0;
                    break;
                }
            }
            
            if (all_digits && *suffix)
            {
                partition_count++;
            }
        }
    }
    
    closedir(dir);
    *num = partition_count;
    return 0;
}