// Child.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <glib.h>

#define NLINES 1000
#define NCHARS 10

gint main(gint argc, gchar *argv[])
{
	for (gint idx = 0; idx < NLINES; idx++)
	{
		for (gint idx2 = 0; idx2 < NCHARS; idx2++)
		{
			for (gint idx1 = 0; idx1 < NCHARS; idx1++)
			{
				fprintf(stdout, "%u", idx1);
			}
		}
		fprintf(stdout, "\n");
	}
  return 0;
}

