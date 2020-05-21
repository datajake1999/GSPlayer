#include "logfile.h"
#include <stdio.h>
#include <stdarg.h>

static FILE *logfile;

void LogFileOpen(const char *filename)
{
	if (logfile == NULL)
	{
		logfile = fopen(filename, "w");
		if (logfile == NULL)
		{
			printf("Failed to open %s.\n", filename);
			return;
		}
	}
	else
	{
		rewind(logfile);
	}
}

void LogFileWrite(const char *text, ...)
{
	va_list args;
	if (logfile == NULL)
	{
		return;
	}
	va_start(args, text);
	vfprintf(logfile, text, args);
	va_end(args);
}

void LogFileClose()
{
	if (logfile == NULL)
	{
		return;
	}
	fclose(logfile);
	logfile = NULL;
}
