/* System dependant stuff for Microsoft C family */

#include "h.h"

match(a, b)
char *a, *b;
{
    while (*a++ == *b++)
	if (*a == '\0' && *b == '\0')
	    return 1;
    return 0;
}
/* set the macros $(@D) $(@F)... */
setname(m1, m2, m3, n)
char *m1, *m2, *m3;
char *n;
{
    static char drive[_MAX_DRIVE];
    static char path[_MAX_DIR];
    static char node[_MAX_FNAME];
    static char ext[_MAX_EXT];

    static char dpart[_MAX_PATH];
    static char fpart[_MAX_PATH];

    _splitpath(n, drive, path, node, ext);

    strcpy(dpart, drive);
    strcat(dpart, path);

    strcpy(fpart, node);
    strcat(fpart, ".");
    strcat(fpart, ext);

    setmacro(m1, dpart);
    setmacro(m2, fpart);
    setmacro(m3, node);
}
