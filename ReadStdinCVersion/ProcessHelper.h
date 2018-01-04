#pragma once
#ifndef PROCESSHELPER_H
#define PROCESSHELPER_H

/*
* ExecuteShellCommand - Creates a child process and returns any data that was output to STDOUT
*                       by the child.
*
* Returns:
*   -1 - If unable to create STDOUT handles
*   -2 - If unable to set Inherit flag on the Read handle.
*   -3 - If unable to create child process.
*    0 - If Success.  **out points to returnded data which must be freeed using g_free().
*/
extern gint ExecuteShellCommand(gchar *command, gint argc, gchar **argv, gint timeout, gchar **out);

#endif /* PROCESSHELPER_H */
