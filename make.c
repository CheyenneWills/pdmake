/*
 * Do the actual making for make
 */

#include "h.h"

/*
 * Exec a shell that returns exit status correctly (/bin/esh). The
 * standard EON shell returns the process number of the last async
 * command, used by the debugger (ugg). [exec on eon is like a
 * fork+exec on unix]
 */
int
dosh(string, shell)
char *string;
char *shell;
{
    int number;

    fflush(stdout);
#if unix || OS2 || MSDOS
    return system(string);
#endif

#if VM
    return vmsystem(string);
#endif

#if eon
    return ((number = execl(shell, shell, "-c", string, 0)) == -1) ?
	-1 :			/* couldn't start the shell */
	wait(number);		/* return its exit status */
#endif

#if os9
    int status, pid;

    strcat(string, "\n");
    if ((number = os9fork(shell, strlen(string), string, 0, 0, 0)) == -1)
	return -1;		/* Couldn't start a shell */
    do {
	if ((pid = wait(&status)) == -1)
	    return -1;		/* child already died!?!? */
    } while (pid != number);

    return status;
#endif
}


/*
 * Do commands to make a target
 */
void
docmds1(np, lp)
struct name *np;
struct line *lp;
{
    bool ssilent;
    bool signore;
    int estat;
    register char *q;
    register char *p;
    char *shell;
    register struct cmd *cp;


    if (*(shell = getmacro("SHELL")) == '\0')

#if eon
	shell = ":bin/esh";
#endif

#if unix
    shell = "/bin/sh";
#endif

#if os9
    shell = "shell";
#endif

#if MSDOS
    shell = "command";
#endif

#if OS2
    shell = "cmd";
#endif
#if VM
   shell = "shell";
#endif

    for (cp = lp->l_cmd; cp; cp = cp->c_next) {
	strcpy(str1, cp->c_cmd);
	expand(str1);
	q = str1;
	ssilent = silent;
	signore = ignore;
	while ((*q == '@') || (*q == '-')) {
	    if (*q == '@')      /* Specific silent  */
		ssilent = TRUE;
	    else		/* Specific ignore  */
		signore = TRUE;
	    q++;		/* Not part of the command  */
	}

	if (!domake)
	    ssilent = 0;

	if (!ssilent)
	    fputs("MAKE:", stdout);

	for (p = q; *p; p++) {
	    if (*p == '\n' && p[1] != '\0') {
		*p = ' ';
		if (!ssilent)
		    fputs("\\\n", stdout);
	    } else if (!ssilent)
		putchar(*p);
	}
	if (!ssilent)
	    putchar('\n');

	if (domake) {		/* Get the shell to execute it	 */
	    if ((estat = dosh(q, shell)) != 0) {
		if (estat == -1)
		    fatal("Couldn't execute %s", shell);
		else {
		    printf("%s: Error code %d", myname, estat);
		    if (signore)
			fputs(" (Ignored)\n", stdout);
		    else {
			putchar('\n');
#if !VM
			if (!(np->n_flag & N_PREC))
			    if (unlink(np->n_name) == 0)
				printf("%s: '%s' removed.\n", myname, np->n_name);
#endif /* vm */
			exit(estat);
		    }
		}
	    }
	}
    }
}


docmds(np)
struct name *np;
{
    register struct line *lp;


    for (lp = np->n_line; lp; lp = lp->l_next)
	docmds1(np, lp);
}


#if os9
/*
 * Some stuffing around to get the modified time of a file in an os9
 * file system
 */
getmdate(fd, tbp)
struct sgtbuf *tbp;
{
    struct registers regs;
    static struct fildes fdbuf;


    regs.rg_a = fd;
    regs.rg_b = SS_FD;
    regs.rg_x = &fdbuf;
    regs.rg_y = sizeof(fdbuf);

    if (_os9(I_GETSTT, &regs) == -1) {
	errno = regs.rg_b & 0xff;
	return -1;
    }
    if (tbp) {
	_strass(tbp, fdbuf.fd_date, sizeof(fdbuf.fd_date));
	tbp->t_second = 0;	/* Files are only acurate to mins */
    }
    return 0;
}


/*
 * Kludge routine to return an aproximation of how many seconds since
 * 1980. Dates will be in order, but will not be lineer
 */
time_t
cnvtime(tbp)
struct sgtbuf *tbp;
{
    long acc;


    acc = tbp->t_year - 80;	/* Baseyear is 1980 */
    acc = acc * 12 + tbp->t_month;
    acc = acc * 31 + tbp->t_day;
    acc = acc * 24 + tbp->t_hour;
    acc = acc * 60 + tbp->t_minute;
    acc = acc * 60 + tbp->t_second;

    return acc;
}


/*
 * Get the current time in the internal format
 */
time(tp)
time_t *tp;
{
    struct sgtbuf tbuf;


    if (getime(&tbuf) < 0)
	return -1;

    if (tp)
	*tp = cnvtime(&tbuf);

    return 0;
}
#endif /* os9 */


/*
 * Get the modification time of a file.  If the first doesn't exist,
 * it's modtime is set to 0.
 */
void
modtime(np)
struct name *np;
{

#if unix || MSDOS || OS2
    struct stat info;
    int fd;


    if (stat(np->n_name, &info) < 0) {
#if OS2 || MSDOS /* Possible bug with microsoft 5.1 stat function */
	if (errno != ENOENT && errno != 0)
#else
	if (errno != ENOENT)
#endif
	    fatal("Can't open %s; error %d", np->n_name, errno);

	np->n_time = 0L;
    } else
	np->n_time = info.st_mtime;
#endif

#if eon
    struct stat info;
    int fd;


    if ((fd = open(np->n_name, 0)) < 0) {
	if (errno != ER_NOTF)
	    fatal("Can't open %s; error %02x", np->n_name, errno);

	np->n_time = 0L;
    } else if (getstat(fd, &info) < 0)
	fatal("Can't getstat %s; error %02x", np->n_name, errno);
    else
	np->n_time = info.st_mod;

    close(fd);
#endif

#if os9
    struct sgtbuf info;
    int fd;


    if ((fd = open(np->n_name, 0)) < 0) {
	if (errno != E_PNNF)
	    fatal("Can't open %s; error %02x", np->n_name, errno);

	np->n_time = 0L;
    } else if (getmdate(fd, &info) < 0)
	fatal("Can't getstat %s; error %02x", np->n_name, errno);
    else
	np->n_time = cnvtime(&info);

    close(fd);
#endif

#if VM
    extern long getmtime();
    np->n_time = getmtime(np->n_name);
    if (np->n_time == -(FILE_NOT_FOUND))
	np->n_time = 0L;
    else if (np->n_time < 0)
	fatal("Can't get modified time for %s; error %d",
	    np->n_name, -(np->n_time));
#endif /* vm */

}


/*
 * Update the mod time of a file to now.
 */
void
touch(np)
struct name *np;
{
    char c;
    int fd;


    if (!domake || !silent)
	printf("    touch(%s)\n", np->n_name);

    if (domake) {
#if MSDOS || OS2
#ifndef IBMCSET2
	long a[2];

	a[0] = a[1] = time(0);
	if (utime(np->n_name, &a[0]) < 0)
#else
	if (utime(np->n_name, NULL) < 0)
#endif
	    printf("%s: '%s' not touched - non-existant\n",
		myname, np->n_name);
#endif

#if unix
	long a[2];

	a[0] = a[1] = time(0);
	if (utime(np->n_name, &a[0]) < 0)
	    printf("%s: '%s' not touched - non-existant\n",
		myname, np->n_name);
#endif
#if eon
	if ((fd = open(np->n_name, 0)) < 0)
	    printf("%s: '%s' not touched - non-existant\n",
		myname, np->n_name);
	else {
	    uread(fd, &c, 1, 0);
	    uwrite(fd, &c, 1);
	}
	close(fd);
#endif
#if os9
	/*
	 * Strange that something almost as totally useless as this
	 * is easy to do in os9!
	 */
	if ((fd = open(np->n_name, S_IWRITE)) < 0)
	    printf("%s: '%s' not touched - non-existant\n",
		myname, np->n_name);
	close(fd);
#endif

#if VM
	FILE *i;
	if (!(i = fopen(np->n_name, "r+")))
	    printf("%s: '%s' not touched - non-existant??/n",
		myname, np->n_name);
	else {
	    int c;
	    fseek(i, 0, SEEK_SET);
	    c = fgetc(i);
	    fseek(i, 0, SEEK_SET);
	    fputc(c, i);
	    fclose(i);
	}
#endif				/* VM5713AAH */
    }
}


/*
 * Recursive routine to make a target.
 */
int
make(np, level)
struct name *np;
int level;
{
    register struct depend *dp;
    register struct line *lp;
    register struct depend *qdp;
    time_t dtime = 1;
    bool didsomething = 0;


    if (np->n_flag & N_DONE)
	return 0;

    if (!np->n_time)
	modtime(np);		/* Gets modtime of this file  */

    if (rules) {
	for (lp = np->n_line; lp; lp = lp->l_next)
	    if (lp->l_cmd)
		break;
	if (!lp)
	    dyndep(np);
    }
    if (!(np->n_flag & N_TARG) && np->n_time == 0L)
	fatal("Don't know how to make %s", np->n_name);

    for (qdp = (struct depend *) 0, lp = np->n_line; lp; lp = lp->l_next) {
	for (dp = lp->l_dep; dp; dp = dp->d_next) {

#if DEBUG
	    if (display)
		printf("... %s Timestamp %ld\n", dp->d_name->n_name, dp->d_name->n_time);
#endif

	    make(dp->d_name, level + 1);
	    if (np->n_time < dp->d_name->n_time)
		qdp = newdep(dp->d_name, qdp);
	    dtime = max(dtime, dp->d_name->n_time);
	}
	if (!quest && (np->n_flag & N_DOUBLE) && (np->n_time < dtime)) {
	    make1(np, lp, qdp); /* free()'s qdp */
	    dtime = 1;
	    qdp = (struct depend *) 0;
	    didsomething++;
	}
    }

    np->n_flag |= N_DONE;

    if (quest) {
	long t;

	t = np->n_time;
	time(&np->n_time);
	return t < dtime;
    } else if (np->n_time < dtime && !(np->n_flag & N_DOUBLE)) {
	make1(np, (struct line *) 0, qdp);	/* free()'s qdp */
	time(&np->n_time);
    } else if (level == 0 && !didsomething)
	printf("%s: '%s' is up to date\n", myname, np->n_name);
    return 0;
}


make1(np, lp, qdp)
register struct depend *qdp;
struct line *lp;
struct name *np;
{
    register struct depend *dp;


    if (dotouch)
	touch(np);
    else {
	strcpy(str1, "");
	for (dp = qdp; dp; dp = qdp) {
	    if (strlen(str1))
		strcat(str1, " ");
	    strcat(str1, dp->d_name->n_name);
	    qdp = dp->d_next;
	    free(dp);
	}
	setmacro("?", str1);
	if( np->n_flag & N_DYN) {
	    setmacro("<",np->n_dyn);
#if MSDOS || OS2
	    setname("<D", "<F", "<N", np->n_dyn);
#endif
#if VM
	    setname("<F",np->n_dyn);
#endif
	}
	setmacro("@", np->n_name);
#if MSDOS || OS2
	setname("@D", "@F", "@N", np->n_name);  /* set $@D & $@F */
#endif
#if VM
	setname("@F", np->n_name);
#endif
	if (lp) 		/* lp set if doing a :: rule */
	    docmds1(np, lp);
	else
	    docmds(np);
    }
}
