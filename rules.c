/*
 * Control of the implicit suffix rules
 */

#include "h.h"

/*
 * Return a pointer to the suffix of a name
 */
char *
suffix(name)
char *name;
{
    return rindex(name, '.');
}


/*
 * Dynamic dependency.	This routine applies the suffix rules to try
 * and find a source and a set of rules for a missing target.	If
 * found, np is made into a target with the implicit source name, and
 * rules.	Returns TRUE if np was made into a target.
 */
bool
dyndep(np)
struct name *np;
{
    register char *p;
    register char *q;
    register char *suff;	/* Old suffix  */
    register char *basename;	/* Name without suffix	 */
    struct name *op;		/* New dependent  */
    struct name *sp;		/* Suffix  */
    struct line *lp;
    struct depend *dp;
    char *newsuff;


    p = str1;
    q = np->n_name;
    if (!(suff = suffix(q)))
	return FALSE;		/* No suffix */
    while (q < suff)
	*p++ = *q++;
    *p = '\0';
    basename = setmacro("*", str1)->m_val;

#if MSDOS || OS2
    setname("*D", "*F", "*N", str1);
#endif

#if VM
    setname("*F",str1);
#endif

    if (!((sp = newname(".SUFFIXES"))->n_flag & N_TARG))
	return FALSE;

    for (lp = sp->n_line; lp; lp = lp->l_next)
	for (dp = lp->l_dep; dp; dp = dp->d_next) {
	    newsuff = dp->d_name->n_name;
	    if (strlen(suff) + strlen(newsuff) + 1 >= LZ)
		fatal("Suffix rule too long");
	    p = str1;
	    q = newsuff;
	    while (*p++ = *q++);
	    p--;
	    q = suff;
	    while (*p++ = *q++);
	    sp = newname(str1);
	    if (sp->n_flag & N_TARG) {
		p = str1;
		q = basename;
		if (strlen(basename) + strlen(newsuff) + 1 >= LZ)
		    fatal("Implicit name too long");
		while (*p++ = *q++);
		p--;
		q = newsuff;
		while (*p++ = *q++);
		op = newname(str1);
		if (!op->n_time)
		    modtime(op);
		if (op->n_time) {
		    dp = newdep(op, 0);
		    newline(np, dp, sp->n_line->l_cmd, 0);
		    setmacro("<", op->n_name);
#if MSDOS || OS2
		    setname("<D", "<F", "<N", op->n_name);
#endif

#if VM
		    setname("<F",op->n_name);
#endif
		    return TRUE;
		}
	    }
	}
    return FALSE;
}


/*
 * Make the default rules
 */
void
makerules()
{
    struct cmd *cp;
    struct name *np;
    struct depend *dp;


#ifdef eon
    setmacro("BDSCC", "asm");
    /* setmacro("BDSCFLAGS", "");      */
    cp = newcmd("$(BDSCC) $(BDSCFLAGS) -n $<", NULL);
    np = newname(".c.o");
    newline(np, 0, cp, 0);

    setmacro("CC", "c");
    setmacro("CFLAGS", "-O");
    cp = newcmd("$(CC) $(CFLAGS) -c $<", NULL);
    np = newname(".c.obj");
    newline(np, 0, cp, 0);

    setmacro("M80", "asm -n");
    /* setmacro("M80FLAGS", "");       */
    cp = newcmd("$(M80) $(M80FLAGS) $<", NULL);
    np = newname(".mac.o");
    newline(np, 0, cp, 0);

    setmacro("AS", "zas");
    /* setmacro("ASFLAGS", "");        */
    cp = newcmd("$(ZAS) $(ASFLAGS) -o $@ $<", NULL);
    np = newname(".as.obj");
    newline(np, 0, cp, 0);

    np = newname(".as");
    dp = newdep(np, 0);
    np = newname(".obj");
    dp = newdep(np, dp);
    np = newname(".c");
    dp = newdep(np, dp);
    np = newname(".o");
    dp = newdep(np, dp);
    np = newname(".mac");
    dp = newdep(np, dp);
    np = newname(".SUFFIXES");
    newline(np, dp, 0, 0);
#endif

#ifdef unix
    /*
     * Some of the UNIX implicit rules
     */
    setmacro("CC", "cc");
    setmacro("CFLAGS", "-O");
    cp = newcmd("$(CC) $(CFLAGS) -c $<", NULL);
    np = newname(".c.o");
    newline(np, 0, cp, 0);

    setmacro("AS", "as");
    cp = newcmd("$(AS) -o $@ $<", NULL);
    np = newname(".s.o");
    newline(np, 0, cp, 0);

    setmacro("YACC", "yacc");
    /* setmacro("YFLAGS", ""); */
    cp = newcmd("$(YACC) $(YFLAGS) $<", NULL);
    cp = newcmd("mv y.tab.c $@", cp);
    np = newname(".y.c");
    newline(np, 0, cp, 0);

    cp = newcmd("$(YACC) $(YFLAGS) $<", NULL);
    cp = newcmd("$(CC) $(CFLAGS) -c y.tab.c", cp);
    cp = newcmd("rm y.tab.c", cp);
    cp = newcmd("mv y.tab.o $@", cp);
    np = newname(".y.o");
    newline(np, 0, cp, 0);

    np = newname(".s");
    dp = newdep(np, 0);
    np = newname(".o");
    dp = newdep(np, dp);
    np = newname(".c");
    dp = newdep(np, dp);
    np = newname(".y");
    dp = newdep(np, dp);
    np = newname(".SUFFIXES");
    newline(np, dp, 0, 0);
#endif

#if OS2
    /*
     * Some of the MSDOS or OS2 implicit rules
     */

    setmacro("ICON", "icont");
    cp = newcmd("$(ICON) -c $(IFLAGS) $<", NULL);
    np = newname(".icn.u1");
    newline(np, 0, cp, 0);

    cp = newcmd("$(ICON) $(IFLAGS) $<", NULL);
    np = newname(".icn.icx");
    newline(np, 0, cp, 0);

    cp = newcmd("$(ICON) $(IFLAGS) $<", NULL);
    np = newname(".u1.icx");
    newline(np, 0, cp, 0);

    setmacro("CC", "icc");
    cp = newcmd("$(CC) -c $(CFLAGS) $<", NULL);
    np = newname(".c.obj");
    newline(np, 0, cp, 0);

    cp = newcmd("$(CC) -c $(CFLAGS) $<", NULL);
    np = newname(".cpp.obj");
    newline(np, 0, cp, 0);

    setmacro("AS", "masm");
    cp = newcmd("$(AS) $(ASFLAGS) $<,$@;", NULL);
    np = newname(".asm.obj");
    newline(np, 0, cp, 0);

    setmacro("YACC", "yacc");
    /* setmacro("YFLAGS", ""); */
    cp = newcmd("$(YACC) $(YFLAGS) $<", NULL);
    cp = newcmd("rename ytab.c $@", cp);
    np = newname(".y.c");
    newline(np, 0, cp, 0);

    cp = newcmd("$(YACC) $(YFLAGS) $<", NULL);
    cp = newcmd("$(CC) -c $(CFLAGS)  ytab.c", cp);
    cp = newcmd("del ytab.c", cp);
    cp = newcmd("rename ytab.obj $@", cp);
    np = newname(".y.obj");
    newline(np, 0, cp, 0);

    np = newname(".obj");
    dp = newdep(np, 0);
    np = newname(".c");
    dp = newdep(np, dp);
    np = newname(".cpp");
    dp = newdep(np, dp);
    np = newname(".y");
    dp = newdep(np, dp);
    np = newname(".asm");
    dp = newdep(np, dp);
    np = newname(".icx");
    dp = newdep(np, dp);
    np = newname(".u1");
    dp = newdep(np, dp);
    np = newname(".icn");
    dp = newdep(np, dp);
    np = newname(".SUFFIXES");
    newline(np, dp, 0, 0);
#endif
#if MSDOS
    /*
     * Some of the MSDOS or OS2 implicit rules
     */

    setmacro("CC", "cl");
    cp = newcmd("$(CC) -c $(CFLAGS) $<", NULL);
    np = newname(".c.obj");
    newline(np, 0, cp, 0);

    setmacro("AS", "masm");
    cp = newcmd("$(AS) $(ASFLAGS) $<,$@;", NULL);
    np = newname(".asm.obj");
    newline(np, 0, cp, 0);

    setmacro("YACC", "yacc");
    /* setmacro("YFLAGS", ""); */
    cp = newcmd("$(YACC) $(YFLAGS) $<", NULL);
    cp = newcmd("rename ytab.c $@", cp);
    np = newname(".y.c");
    newline(np, 0, cp, 0);

    cp = newcmd("$(YACC) $(YFLAGS) $<", NULL);
    cp = newcmd("$(CC) $(CFLAGS)  ytab.c", cp);
    cp = newcmd("del ytab.c", cp);
    cp = newcmd("rename ytab.obj $@", cp);
    np = newname(".y.obj");
    newline(np, 0, cp, 0);

    np = newname(".obj");
    dp = newdep(np, 0);
    np = newname(".c");
    dp = newdep(np, dp);
    np = newname(".y");
    dp = newdep(np, dp);
    np = newname(".asm");
    dp = newdep(np, dp);
    np = newname(".SUFFIXES");
    newline(np, dp, 0, 0);
#endif
#ifdef os9
    /*
     * Fairlight use an enhanced version of the C sub-system. They
     * have a specialised macro pre-processor.
     */
    setmacro("CC", "cc");
    setmacro("CFLAGS", "-z");
    cp = newcmd("$(CC) $(CFLAGS) -r $<", NULL);

    np = newname(".c.r");
    newline(np, 0, cp, 0);
    np = newname(".ca.r");
    newline(np, 0, cp, 0);
    np = newname(".a.r");
    newline(np, 0, cp, 0);
    np = newname(".o.r");
    newline(np, 0, cp, 0);
    np = newname(".mc.r");
    newline(np, 0, cp, 0);
    np = newname(".mca.r");
    newline(np, 0, cp, 0);
    np = newname(".ma.r");
    newline(np, 0, cp, 0);
    np = newname(".mo.r");
    newline(np, 0, cp, 0);

    np = newname(".r");
    dp = newdep(np, 0);
    np = newname(".mc");
    dp = newdep(np, dp);
    np = newname(".mca");
    dp = newdep(np, dp);
    np = newname(".c");
    dp = newdep(np, dp);
    np = newname(".ca");
    dp = newdep(np, dp);
    np = newname(".ma");
    dp = newdep(np, dp);
    np = newname(".mo");
    dp = newdep(np, dp);
    np = newname(".o");
    dp = newdep(np, dp);
    np = newname(".a");
    dp = newdep(np, dp);
    np = newname(".SUFFIXES");
    newline(np, dp, 0, 0);
#endif

#if VM
    setmacro("CC", "EXEC CC");
    cp = newcmd("$(CC) $(<F) C * $(CFLAGS)", NULL);
    np = newname(".C.TEXT");
    newline(np, 0, cp, 0);

    setmacro("AS", "ASSEMBLE");
    cp = newcmd("$(AS) $(<F) $(ASFLAGS)", NULL);
    np = newname(".ASSEMBLE.TEXT");
    newline(np, 0, cp, 0);

    np = newname(".TEXT");
    dp = newdep(np, 0);
    np = newname(".C");
    dp = newdep(np, dp);
    np = newname(".ASSEMBLE");
    dp = newdep(np, dp);
    np = newname(".SUFFIXES");
    newline(np, dp, 0, 0);
#endif
}
