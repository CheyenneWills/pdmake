/*
 * Operating system dependent code -- the following codes is required
 * when building MAKE with the VM switch.
 */

#include "h.h"

long
getmtime(file_name)
char *file_name;
{
    /* Routine to get the last-modified time of a file in sec.	*/
    char listf_cmd[80], listf_data[80], vm_file_name[20], *p;
    long month, day, year, hour, minute, second, mtime;
    int listf_rc, i;
    system("SET CMSTYPE HT");
    system("MAKEBUF");
    strcpy(listf_cmd, "LISTF ");
    strcat(listf_cmd, file_name);
    strcat(listf_cmd, " * (DATE STACK");
    listf_rc = vmsystem(listf_cmd);
    system("SET CMSTYPE RT");
    /* Get the data on the file from the LISTF command	*/
    if (listf_rc == 0) {
	/* Read the data from the stack.  */
	gets(listf_data);
	system("DROPBUF");
	/* Skip the file name */
	strtok(listf_data, " ");
	/* Skip the type, mode, format, lrecl, records and blocks */
	for (i = 0; i < 6; i++)
	    strtok(NULL, " ");
	/* Point to the month */
	p = strtok(NULL, "/");
	month = strtol(p, &p, 10);
	day = strtol(p + 1, &p, 10);
	year = strtol(p + 1, &p, 10) - 87;
	/* If your file is before 1987, you will have to "touch" it */
	/*
	 * This whole thing breaks on the century, or when the long
	 * integer runs out (whichever comes first).
	 */
	if (year < 0)
	    year = 0;
	/*
	 * But maybe it will keep working as long as all your files
	 * are in the same year
	 */
	hour = strtol(p + 1, &p, 10);
	minute = strtol(p + 1, &p, 10);
	second = strtol(p + 1, &p, 10);
	return second + (minute + (hour + (day + (month + year * 12)
		    * 31) * 24) * 60) * 60;
    } else {
	system("DROPBUF");
	return -28;
    }
}
/* Set up the @F, <F and *F macros */
setname(m1, n)
char *m1, *n;
{
    int i;
    char fpart[LZ];

    /* Make a macro for the filename only without filetype  */
    for (i = 0; ((n[i] != '.') && (n[i] != '??/0')); i++)
	fpart[i] = n[i];
    fpart[i] = '??/0';
    setmacro(m1, fpart);
}


int
vmsystem(command_string)
char *command_string;
/*
 * Issue a command after removing all periods not enclosed in square
 * brackets.  (These are presumable all the periods separating
 * filenames and filetypes.)
 */
{
    int i, remove_period = 1;	/* period removal is the default */
    char *p, *q;

    /* Test for a comment */
    if (command_string[0] == '*')
	return (0);

    for (i = 0; (command_string[i] != '??/0'); i++) {
	/* "Good" periods must be put in square brackets []  */
	if ((command_string[i] == '[') || (command_string[i] == ']')) {
	    /*
	     * Open bracket = turn off period removal;	close bracket
	     * = turn on period removal
	     */
	    remove_period = (command_string[i] == ']');
	    p = &command_string[i];	/* Take the square bracket
					 * out	*/
	    q = &command_string[i + 1];
	    while (*p++ = *q++);
	    i--;
	} else {
	    if ((command_string[i] == '.') && (remove_period))
		command_string[i] = ' ';
	}
	/* If it is a period, remove it  */
    }
    return (system(command_string));
}
