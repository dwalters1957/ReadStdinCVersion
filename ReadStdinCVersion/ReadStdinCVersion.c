// ReadStdinCVersion.c : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "ProcessHelper.h"

const gchar *args[3] = { "--extcap-config","--extcap-interface","\\\\.\\USBPcap3" };
const gchar cmd[] = "C:\\Program Files\\USBPcap\\USBPcapCMD.exe";
const gchar cmd1[] = "Y:\\Visual Studio .NET 2017 Projects\\Projects\\ReadStdinCVersion\\x64\\Debug\\Child.exe";
const gchar fname[] = "d:\\print\\testfile2.txt";

gint main(gint argc, gchar *argv[])
{
  gint rtn;
  FILE *fout;
  gchar *rtnstr = NULL;

	fprintf(stderr, "ReadStdinCVerion\n");
	//rtn = ExecuteShellCommand((gchar *)cmd, 3, (gchar **)args, 10000, &rtnstr);
	rtn = ExecuteShellCommand((gchar *)cmd1, 0, (gchar **)NULL, 10000, &rtnstr);

  if (rtnstr != NULL)
  {
    fprintf(stderr, "Now writting rtnstr to %s\n", fname);
    fopen_s(&fout, fname, "wb");
    fwrite(rtnstr, sizeof(gchar), strlen(rtnstr), fout);
    fclose(fout);
    g_free(rtnstr);  // Need to free the buffer that held the returned data!
  }
  return (EXIT_SUCCESS);
}

