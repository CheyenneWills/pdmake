/*
 * Macro control for make
 */

#include "h.h"


struct macro *macrohead = NULL;
#if MSDOS || OS2
struct macro *macroheade = NULL;
struct macro *macroheadc = NULL;
#endif


struct macro *
getmp(name)
char *name;
{
    register struct macro *rp;

    for (rp = macrohead; rp; rp = rp->m_next)
	if (strcmp(name, rp->m_name) == 0)
	    return rp;
#if MSDOS || OS2
    for (rp = macroheade; rp; rp = rp->m_next)
	if (strcmp(name, rp->m_name) == 0)
	    return rp;
    for (rp = macroheadc; rp; rp = rp->m_next)
	if (strcmp(name, rp->m_name) == 0)
	    return rp;
#endif
    return (struct macro *) 0;
}


char *
getmacro(name)
char *name;
{
    struct macro *mp;

    if (mp = getmp(name))
	return mp->m_val;
    else
	return "";
}
#if MSDOS || OS2
static struct macro *setmacrox(char *, char *, struct macro **);
struct macro *
setmacro(name, val)
char *name, *val;
{
    return setmacrox(name, val, &macrohead);
}
struct macro *
setmacroe(name, val)
char *name, *val;
{
    return setmacrox(name, val, &macroheade);
}
struct macro *
setmacroc(name, val)
char *name, *val;
{
    return setmacrox(name, val, &macroheadc);
}

static struct macro *
setmacrox(name, val, macrohead)
char *name, *val;
struct macro **macrohead;

#else

struct macro *
setmacro(name, val)
char *name;
char *val;

#endif
{
    register struct macro *rp;
    register char *cp;


    /* Replace macro definition if it exists  */
#if MSDOS || OS2
    for (rp = *macrohead; rp; rp = rp->m_next)
#else
    for (rp = macrohead; rp; rp = rp->m_next)
#endif
	if (strcmp(name, rp->m_name) == 0) {
	    free(rp->m_val);	/* Free space from old	 */
	    break;
	}
    if (!rp) {			/* If not defined, allocate space for
				 * new	*/
	if ((rp = (struct macro *) malloc(sizeof(struct macro)))
	    == (struct macro *) 0)
	    fatal("No memory for macro");

#if MSDOS || OS2
	rp->m_next = *macrohead;
	*macrohead = rp;
#else
	rp->m_next = macrohead;
	macrohead = rp;
#endif
	rp->m_flag = FALSE;

	if ((cp = malloc(strlen(name) + 1)) == (char *) 0)
	    fatal("No memory for macro");
	strcpy(cp, name);
	rp->m_name = cp;
    }
    if ((cp = malloc(strlen(val) + 1)) == (char *) 0)
	fatal("No memory for macro");
    strcpy(cp, val);		/* Copy in new value  */
    rp->m_val = cp;

    return rp;
}


/*
 * Do the dirty work for expand
 */
void
doexp(to, from, len, buf)
char **to;
char *from;
int *len;
char *buf;
{
    register char *rp;
    register char *p;
    register char *q;
    register struct macro *mp;


    rp = from;
    p = *to;
    while (*rp) {
	if (*rp != '$') {
	    *p++ = *rp++;
	    (*len)--;
	} else {
	    q = buf;
	    if (*++rp == '{')
		while (*++rp && *rp != '}')
		    *q++ = *rp;
	    else if (*rp == '(')
		while (*++rp && *rp != ')')
		    *q++ = *rp;
	    else if (!*rp) {
		*p++ = '$';
		break;
	    } else
		*q++ = *rp;
	    *q = '\0';
	    if (*rp)
		rp++;
	    if (!(mp = getmp(buf)))
		mp = setmacro(buf, "");
	    if (mp->m_flag)
		fatal("Infinitely recursive macro %s", mp->m_name);
	    mp->m_flag = TRUE;
	    *to = p;
	    doexp(to, mp->m_val, len, buf);
	    p = *to;
	    mp->m_flag = FALSE;
	}
	if (*len <= 0)
	    error("Expanded line too long");
    }
    *p = '\0';
    *to = p;
}


/*
 * Expand any macros in str.
 */
void
expand(str)
char *str;
{
    static char a[LZ];
    static char b[LZ];
    char *p = str;
    int len = LZ - 1;

    strcpy(a, str);
    doexp(&p, a, &len, b);
}
