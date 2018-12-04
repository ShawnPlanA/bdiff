/* 
 * bdiff.c
 *
 * Compare binary file and mark the differences.
 *
 *
 * Copyright (C) by Jackie Lee <jackielee524@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio_ext.h>

#include "bdiff.h"

static unsigned long global_pos = 0; 

static inline void print_diff(unsigned long global_pos, unsigned int diff_mask, 
		uint8 *line1, uint8 *line2, int length)
{
	int i;
	unsigned int mask;

	printf("%08lx: ", global_pos);

	for(i = 0; i < length; i++)	
	{
		if (line1[i] == 0xff && line1[i+1] == 0xff)
			break;
		mask = diff_mask & (1 << i);
		if(mask)
			printf("\x1b[32m%c%02X%c\x1b[0m", '[', (unsigned int)line1[i], ']');
		else
			printf("%c%02X%c", ' ', (unsigned int)line1[i], ' ');
	}

	if (i < LINE_LENGTH)
	{
		i = LINE_LENGTH - i;
		while(i--)
			printf("    ");
	}

	printf(" || ");

	for(i = 0; i < length; i++)	
	{
		if (line2[i] == 0xff && line2[i+1] == 0xff)
			break;
		mask = diff_mask & (1 << i);
		if(mask)
			printf("\x1b[32m%c%02X%c\x1b[0m", '[', (unsigned int)line2[i], ']');
		else
			printf("%c%02X%c", ' ', (unsigned int)line2[i], ' ');
	}

	printf("\n");
}

static inline void print_file1(uint8 *buf, long length)
{
	int i;
	long least = length;
	int len;
	uint8 *p = buf;

	while(least > 0)
	{
		printf("%08lx: ", global_pos);

		len = min((long)least, (long)LINE_LENGTH);

		for(i = 0; i < len; i++)
		{
			printf("[%02X]", (unsigned int)*p++);
		}

		if (least < LINE_LENGTH)
		{
			i = LINE_LENGTH - least;
			least -= i;
			while(i--)
				printf("    ");
		}

		printf(" || \n");

		least -= len;
		global_pos += len;
	}
}

static inline void print_file2(uint8 *buf, long length)
{
	int i;
	long least = length;
	int len;
	uint8 *p = buf;

	while(least > 0)
	{
		printf("%08lx: ", global_pos);

		i = LINE_LENGTH;
		while(i--)
			printf("    ");

		printf(" || ");

		len = min((long)least, (long)LINE_LENGTH);
		for(i = 0; i <  len; i++)
		{
			printf("[%02X]", (unsigned int)*p++);
		}

		printf("\n");

		least -= len;
		global_pos += len;
	}
}

static inline long get_file_length(int fd)
{
	struct stat f_stat;

	if (fstat(fd, &f_stat) < 0)
		return -1;
	else
		return (long)f_stat.st_size;
}

static inline int get_block_size(void )
{
	struct stat f_stat;
	int err = 0;
	int fd;
	
	fd = open(".", O_RDONLY);
	if (fd == -1)
	{
		return -1;
	}

	err = fstat(fd, &f_stat);

	close(fd);

	if (err < 0)
		return -2;
	else
		return f_stat.st_blksize;
}

int main(int argc, char *argv[])
{
	int fd1, fd2;
	char *file1, *file2;
	long length1, length2;
	int len1, len2, len;
	uint8 *buf1, *buf2;
	uint8 *line1, *line2;
	uint8 *p1, *p2;
	unsigned int mask;
	long length, least;
	int i, pos;
	int blksize;
	int bufsize, buf_least, print_size;
	int read_size;

	if (argc < 2)
	{
		printf("Make sure that the input parameters !\n");
		printf("please input as:\n");
		printf("          %s file1 file2\n", argv[0]);

		goto EXIT1;
	}
	else
	{
		file1 = argv[1];
		file2 = argv[2];
	}

	blksize = get_block_size();

	fd1 = open(file1, O_RDONLY);
	if (fd1 == -1)
	{	
		printf("Please check %s is right !\n", file1);
		goto EXIT1;
	}
	length1 = get_file_length(fd1);

	fd2 = open(file2, O_RDONLY);	
	if (fd2 == -1)
	{	
		printf("Please check %s is right !\n", file2);
		goto EXIT2;
	}
	length2 = get_file_length(fd2);

	least = length = max(length1, length2); 
	bufsize = min(length, (long)blksize);
	bufsize = alignment(bufsize, LINE_LENGTH);

	buf1 = malloc(bufsize * sizeof(uint8));
	buf2 = malloc(bufsize * sizeof(uint8));

	read_size = bufsize;

	printf("\n---------------------------------------------------------------------------------------------------------------------------------------------\n");
	printf("\t ");
	for(i=0; i<16; i++)
		printf("  %2X", i);
	printf("\x1b[32m  #\x1b[0m");
	for(i=0; i<16; i++)
		printf("  %2X", i);
	printf("\n");
	printf("---------------------------------------------------------------------------------------------------------------------------------------------\n");
	while(least > 0)
	{
		len1 = read(fd1, buf1, read_size);
		len2 = read(fd2, buf2, read_size);

		pos = 0;
		buf_least = min(len1, len2);
		if (len1 < len2)
			buf1[len1] = buf1[len1 + 1] = 0xff;
		else
			buf2[len2] = buf2[len2 + 1] = 0xff;
			
		print_size = buf_least = min(alignment(buf_least, LINE_LENGTH), max(len1, len2));
		while(buf_least > 0)
		{
			line1 = buf1 + pos;
			line2 = buf2 + pos;

			p1 = line1;
			p2 = line2;

			len = min(buf_least, LINE_LENGTH);
			mask = 0;
			for(i = 0; i < len; i++)
			{
				if (*p1 != *p2)
					mask |= 1 << i;

				p1++; p2++;
			}	

			if (mask)
				print_diff(global_pos, mask, line1, line2, len);

			pos += len;
			global_pos += len;
			buf_least -= len;
		}

		if (len1 - print_size > 0)
		{
			print_file1(buf1, len1 - print_size);
		}
		else if (len2 - print_size > 0)
		{
			print_file2(buf2, len2 - print_size);
		}

		least -= max(len1, len2);

		if (least < 0)
		{
			printf("least OUT (%ld) "
				"len1=%d, len2=%d\n", least, len1, len2);

			goto EXIT2;
		}
	}

	if (length1 - length > 0)
	{
		least = length1 - length;
		read_size = min((long)least, (long)bufsize);
		while(least > 0)
		{
			len1 = read(fd1, buf1, read_size);
			print_file1(buf1, len1);

			least -= len1;
		}
	}
	else if (length2 - length > 0)
	{
		least = length2 - length;
		read_size = min((long)least, (long)bufsize);
		while(least > 0)
		{
			len2 = read(fd2, buf2, read_size);
			print_file2(buf2, len2);

			least -= len2;
		}
	}

EXIT:
	close(fd2);
	close(fd1);

	return 0;

EXIT2:
	close(fd1);
EXIT1:
	return -1;
}
