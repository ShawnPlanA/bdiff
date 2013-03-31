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

static inline void print_diff(unsigned long global_pos, unsigned int diff_mask, unsigned char *line1, unsigned char *line2, int length)
{
	int i;
	unsigned int mask;

	printf("%08lx: ", global_pos);

	for(i = 0; i < length; i++)	
	{
		mask = diff_mask & (1 << i);
		printf("%c%02X%c", mask ? '[' : ' ', (unsigned int)line1[i], mask ? ']' : ' ');
	}

	if (length < LINE_LENGTH)
	{
		i = LINE_LENGTH - length;
		while(i--)
			printf("    ");
	}

	printf(" || ");

	for(i = 0; i < length; i++)	
	{
		mask = diff_mask & (1 << i);
		printf("%c%02X%c", mask ? '[' : ' ', (unsigned int)line2[i], mask ? ']' : ' ');
	}

	printf("\n");
}

static inline void print_file1(unsigned char *buf, long length)
{
	int i;
	long least = length;
	int len;
	unsigned char *p = buf;

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

static inline void print_file2(unsigned char *buf, long length)
{
	int i;
	long least = length;
	int len;
	unsigned char *p = buf;

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

static inline int get_file_stat(int fd, long *file_size, int *block_size)
{
	struct stat f_stat;

	if (fstat(fd, &f_stat) < 0)
	{
		*file_size = -1;
		*block_size = -1;

		return -1;
	}
	else
	{
		*block_size = f_stat.st_blksize;
		*file_size = (long)f_stat.st_size;
	
		return 0;
	}
}

int main(int argc, char *argv[])
{
	int fd1, fd2;
	char *file1, *file2;
	long length1, length2;
	int len1, len2, len;
	unsigned char *buf1, *buf2;
	unsigned char *line1, *line2;
	unsigned char *p1, *p2;
	unsigned int mask;
	long length, least;
	int i, pos;
	int blksize;
	int bufsize, buf_least;
	int read_size;

	if (argc < 2)
	{
		printf("Make sure that the input parameters !\n");
		printf("please as:\n");
		printf("          %s file1 file2\n", argv[0]);

		goto EXIT1;
	}

	file1 = argv[1];
	file2 = argv[2];

	fd1 = open(file1, O_RDONLY);
	if (fd1 == -1)
	{	
		printf("Please check %s is right !\n", file1);
		goto EXIT1;
	}

	get_file_stat(fd1, &length1, &blksize);

	fd2 = open(file2, O_RDONLY);	
	if (fd2 == -1)
	{	
		printf("Please check %s is right !\n", file2);
		goto EXIT2;
	}

	get_file_stat(fd2, &length2, &blksize);

	least = length = min(length1, length2), LINE_LENGTH; 
	bufsize = (max(length1, length2) < blksize) ? length : blksize;

	buf1 = malloc(bufsize * sizeof(unsigned char));
	buf2 = malloc(bufsize * sizeof(unsigned char));

	while(least > 0)
	{
		read_size = min((long)bufsize, least);
		len1 = read(fd1, buf1, read_size);
		len2 = read(fd2, buf2, read_size);

		if (len1 != len2)
		{
			printf("read file error (len1=%d, len2=%d)\n", len1, len2);
			goto EXIT2;
		}

		pos = 0;
		buf_least = min(len1, len2);
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

		least -= min(len1, len2);

		if (least < 0)
		{
			printf("least OUT (%ld)\n", least);
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
