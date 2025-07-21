/*
 * make [-f makefile] [-ins] [target(s) ...]
 *
 * (Better than EON mk but not quite as good as UNIX make)
 *
 * -f makefile name
 * -i ignore exit status
 * -n Pretend to make
 * -p Print all macros & targets
 * -q Question up-to-dateness of target.  Return exit status 1 if not
 * -r Don't not use inbuilt rules
 * -s Make silently
 * -t Touch files instead of making them
 * -m Change memory requirements (EON only)
 * -d Display file time stamps (MSDOS only)
 * -e Use environmental values last (MSDOS only)
 * -c change continuation character (default is backslash)
 */

#include <stdarg.h>
#include "h.h"

#ifdef eon
#define MEMSPACE       (16384)
unsigned memspace = MEMSPACE;
#endif

#if VM
int _stksize = 80000;
#endif

char *myname;
char *makefile; 		/* The make file  */

FILE *ifd;			/* Input file desciptor  */
bool domake = TRUE;		/* Go through the motions option  */
bool ignore = FALSE;		/* Ignore exit status option  */
bool silent = FALSE;		/* Silent option  */
bool print = FALSE;		/* Print debuging information  */
bool rules = TRUE;		/* Use inbuilt rules  */
bool dotouch = FALSE;		/* Touch files instead of making  */
bool quest = FALSE;		/* Question up-to-dateness of file  */
char contchar = '\\';           /* Continuation character */

#if MSDOS || OS2
bool envirf = TRUE;		/* Use environment first */
bool display = FALSE;		/* Display file timestamps */
#endif

#if MSDOS || OS2
void
main(argc, argv, envp)
int argc;
char **argv;
char **envp;
#else
void
main(argc, argv)
int argc;
char **argv;
#endif
{
    register char *p;		/* For argument processing  */
    int estat = 0;		/* For question  */
    register struct name *np;

#if MSDOS
    myname = "MAKE";
    argc--;
    argv++;
#else
    myname = (argc-- < 1) ? "make" : *argv++;
#endif

    while ((argc > 0) && (**argv == '-')) {
	argc--; 		/* One less to process	 */
	p = *argv++;		/* Now processing this one  */

	while (*++p != '\0') {
	    switch (*p) {
	      case 'f': /* Alternate file name  */
		if (*++p == '\0') {
		    if (argc-- <= 0)
			usage();
		    p = *argv++;
		}
		makefile = p;
		goto end_of_args;
	      case 'n': /* Pretend mode  */
		domake = FALSE;
		break;
	      case 'i': /* Ignore fault mode  */
		ignore = TRUE;
		break;
	      case 's': /* Silent about commands  */
		silent = TRUE;
		break;
	      case 'p':
		print = TRUE;
		break;
	      case 'r':
		rules = FALSE;
		break;
	      case 't':
		dotouch = TRUE;
		break;
	      case 'q':
		quest = TRUE;
		break;

	      case 'c':
		   if (*++p == '\0') {
		       if(argc-- <= 0)
			   usage();
		       p = *argv++;
		   }
		contchar = *p;
		goto end_of_args;  /* Skip loop on *p */
#ifdef eon
	      case 'm': /* Change space requirements  */
		if (*++p == '\0') {
		    if (argc-- <= 0)
			usage();
		    p = *argv++;
		}
		memspace = atoi(p);
		goto end_of_args;  /* Skip loop on *p */
#endif
#if MSDOS || OS2
	      case 'd':
		display = TRUE;
		break;
	      case 'e':
		envirf = FALSE;
		break;
#endif
	      default:		/* Wrong option  */
		usage();
	    }
	}
end_of_args:;
    }

#ifdef eon
    if (initalloc(memspace) == 0xffff)	/* Must get memory for alloc  */
	fatal("Cannot initalloc memory");
#endif

    if (makefile && strcmp(makefile, "-") == 0)     /* Can use stdin as makefile  */
	ifd = stdin;
    else if (!makefile) {	/* If no file, then use default */
	if ((ifd = fopen(DEFN1, "r")) == NULL)
#ifdef eon
	    if (errno != ER_NOTF)
		fatal("Can't open %s; error %02x", DEFN1, errno);
#endif
#ifdef unix
	if (errno != ENOENT)
	    fatal("Can't open %s; error %02x", DEFN1, errno);
#endif

#if MSDOS || OS2
	if (errno != ENOENT)
	    fatal("Can't open %s; error %02x", DEFN1, errno);
#else
#ifndef os9
	if ((ifd == NULL)
	    && ((ifd = fopen(DEFN2, "r")) == NULL))
	    fatal("Can't open %s", DEFN2);
#else
	fatal("Can't open %s", DEFN1);
#endif				/* os9 */
#endif

    } else if ((ifd = fopen(makefile, "r")) == NULL) {
#if VM
       /* makefile is not null */
       if(!strchr(makefile,'.')) {
	   char mfn[256];
	   strcpy(mfn,makefile);
	   strcat(mfn,".$");
	   if( (ifd = fopen(mfn,"r")) == NULL)
	       fatal("Can't open %s",makefile);
       } else
#endif
	fatal("Can't open %s", makefile);
    }
    makerules();

    setmacro("$", "$");
#if MSDOS || OS2
    {
	char *e;
	char en[256];
	int i;
	while (*envp) {
	    e = index(*envp, '=');
	    i = e - *envp;
	    strncpy(en, *envp++, i);
	    en[i] = 0;
	    if (envirf)
		setmacro(en, e + 1);
	    else
		setmacroe(en, e + 1);
	}
    }
#endif

    while (argc && (p = index(*argv, '='))) {
	char c;
#if MSDOS || OS2
	char *b;
#endif
	c = *p;
	*p = '\0';
#if MSDOS || OS2
	b = malloc(strlen(*argv) + strlen(p + 1) + 3);
	strcpy(b, *argv);
	strcat(b, "=");
	strcat(b, p + 1);
	putenv(b);
	setmacroc(*argv, p + 1);
#else
	setmacro(*argv, p + 1);
#endif
	*p = c;

	argv++;
	argc--;
    }
    if(ifd) {
	input(ifd);		    /* Input all the gunga   */
	fclose(ifd);		    /* Finished with makefile  */
    }
    lineno = 0; 		/* Any calls to error now print no
				 * line number */

    if (print)
	prt();			/* Print out structures  */

    np = newname(".SILENT");
    if (np->n_flag & N_TARG)
	silent = TRUE;

    np = newname(".IGNORE");
    if (np->n_flag & N_TARG)
	ignore = TRUE;
#if !VM
    precious();
#endif

    if (!firstname)
	fatal("No targets defined");

    circh();			/* Check circles in target
				 * definitions	 */

    if (!argc)
	estat = make(firstname, 0);
    else
	while (argc--) {
	    if (!print && !silent && strcmp(*argv, "love") == 0)
		printf("Not war!\n");
	    estat |= make(newname(*argv++), 0);
	}

    if (quest)
	exit(estat);
    else
	exit(0);
}


usage()
{
    fprintf(stderr, "Usage: %s [-f makefile] [-inpqrstc] [macro=val ...] [target(s) ...]\n", myname);
    exit(1);
}


void
fatal(char *msg, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr,msg);
    fprintf(stderr, "%s: ", myname);
    vfprintf(stderr, msg, arg_ptr);
    fputc('\n', stderr);
    va_end(arg_ptr);
    exit(1);
}
