
#include<string.h>
#include<stdio.h>
#include<sys/stat.h>
#include<errno.h>
#include<unistd.h>

#include "dlog.h"
#include "debug.h"
#include "file.h"


int file_len(char *filename)
{
	FILE *fd;
	int end;

	ASSERT(filename);

	if(!(fd = fopen(filename, "r"))){
		DLOG2(LOG_WARNING, "Couldn't open file: %s(%s)\n", 
			filename, strerror(errno));
		return 0;
	}

	fseek(fd, 0, SEEK_END);
	end = ftell(fd);
	fclose(fd);

	return end;
}


