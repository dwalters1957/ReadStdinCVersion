/*
 * ProcessHelper - Functions to spawn a child process and captures all the data 
 *                 that the child process writes to STDOUT.  This data is saved
 *                 in a buffer that allocated on the fly and grows to accept all 
 *                 the data returned.  It must be freed by a call to g_free() 
 *                 after use.
 *
 * Written By: David Walters
 * Date: 01Jan2017
 * Last Rev: 03Jan2017 - Modified to use a secondary thread to read the data so 
 *                       nothing is missed.
 *
*/
#include <stdio.h>
#include <windows.h>
#include <glib.h>
#include "ProcessHelper.h"

static HANDLE g_hChildStdoutRd = NULL;
static HANDLE g_hChildStdoutWr = NULL;
static DWORD g_ReadPipeThreadId = 0;
static HANDLE g_hReadPipeThread = NULL;
static gboolean g_ReadPipeThreadActive = FALSE;
static gchar *DataPt = NULL;

static gboolean CreateChildProcess(gchar *command, gint argc, gchar **argv, gint timeout);
static DWORD WINAPI ReadPipeThread(LPVOID lpParam);

/*
 * ExecuteShellCommand - Creates a child process and returns any data that was output to STDOUT
 *                       by the child.
 *
 * Returns:
 *   -1 - If unable to create STDOUT handles
 *   -2 - If unable to set Inherit flag on the Read handle.
 *   -3 - If unable to create child process.
 *    0 - If Success.  **out points to returned data which must be freeed using g_free().
*/
gint ExecuteShellCommand(gchar *command, gint argc, gchar **argv, gint timeout, gchar **out)
{
  SECURITY_ATTRIBUTES saAttr;
  
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

	// Create the STDOUT read and write handles - Exit if failure
  if (!CreatePipe(&g_hChildStdoutRd, &g_hChildStdoutWr, &saAttr, 0))
    return -1;

	// Inherit the Read handle
  if (!SetHandleInformation(g_hChildStdoutRd, HANDLE_FLAG_INHERIT, 0))
    return -2;

	// Create the child process.
  if (!CreateChildProcess(command, argc, argv, timeout))
    return -3;

	// Return the data read from the child process.
	*out = DataPt;

  return 0;
}

static gboolean CreateChildProcess(gchar *command, gint argc, gchar **argv, gint timeout)
{
  PROCESS_INFORMATION piProcInfo;
  STARTUPINFO siStartInfo;
  gboolean bSuccess;
  gchar cmdline[512], *p;

  ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

  ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

  siStartInfo.cb = sizeof(STARTUPINFO);
  siStartInfo.hStdInput = (HANDLE) -1;
  siStartInfo.hStdOutput = g_hChildStdoutWr;
  siStartInfo.hStdError = (HANDLE) -1;
  siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

  p = &cmdline[0];

	// Parse the command line.
  p += sprintf_s(p, sizeof(cmdline), "%s", command);
  for (gint idx = 0; idx < argc; idx++)
    p += sprintf_s(p, sizeof(cmdline) - strlen(&cmdline[0]), " %s", argv[idx]);

	// Pipe Read Thread starts immediatly.
	g_ReadPipeThreadActive = TRUE;
	g_hReadPipeThread = CreateThread(NULL, 0, &ReadPipeThread, NULL, 0, &g_ReadPipeThreadId);

  // Create the child process
  bSuccess = CreateProcess(
    NULL,									  // 
		(LPSTR) &cmdline[0],    // command line 
    NULL,									  // process security attributes 
    NULL,									  // primary thread security attributes 
    TRUE,										// handles are inherited 
    0,											// creation flags 
    NULL,										// use parent's environment 
    NULL,										// use parent's current directory 
    &siStartInfo,						// STARTUPINFO pointer 
    &piProcInfo);						// receives PROCESS_INFORMATION

  // If an error occurs, exit the application. 
  if (!bSuccess) return FALSE;

	// Wait for the child process to complete
  WaitForSingleObject(piProcInfo.hProcess, timeout);

	// Stop the Read Pipe Thread.
	g_ReadPipeThreadActive = FALSE;

	// Close appropriate handles
	CloseHandle(g_hChildStdoutWr);
  CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);

	// Wait for the read pipe thread to complete
	WaitForSingleObject(g_hReadPipeThread, timeout);
	// Close the thread handle.
	CloseHandle(g_hReadPipeThread);

	return TRUE;
}

static DWORD WINAPI ReadPipeThread(LPVOID lpParam)
{
	DWORD dwRead = 0;
	DWORD bytes_avail = 0;
	DWORD total_bytes = 0;
	gint offset = 0;

	while (g_ReadPipeThreadActive)
	{
		// Anything to read?? Continue if not.
		if (!PeekNamedPipe(g_hChildStdoutRd, NULL, 0, NULL, &bytes_avail, NULL) || bytes_avail == 0)
			continue;

		total_bytes += bytes_avail;

		// Allocate or Re-Allocate the buffer to save the data.
		if (DataPt == NULL) DataPt = g_malloc(total_bytes);
		else                DataPt = g_realloc(DataPt, total_bytes);

		// Read the data - continue if nothing read.
		if (!ReadFile(g_hChildStdoutRd, DataPt + offset, bytes_avail, &dwRead, NULL) || dwRead == 0)
			continue;

		offset += dwRead;
	}

	// Make sure the string is null terminated.
	*(DataPt + total_bytes) = '\0';

	// Close the Read Handle.
	CloseHandle(g_hChildStdoutRd);
	return 0;
}