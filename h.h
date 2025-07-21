/*
 * Include header for make
 */

/*
 *  Standard include files.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#if unix
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#endif /* unix */

#if os9
#include <time.h>
#include <os9.h>
#include <modes.h>
#include <direct.h>
#include <errno.h>
#endif

#if OS2
#undef MSDOS
#ifdef MSC
#include <stdlib.h>
#include <fcntl.h>
#include <dos.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#ifdef IBMCSET2
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/utime.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#endif

#if MSDOS
#include <stdlib.h>
#include <fcntl.h>
#include <dos.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#if VM
#include <stdefs.h>
#endif
#if eon
#include <sys/stat.h>
#include <sys/err.h>
#endif

#ifndef uchar
#if os9
#define uchar	       char
#define void	       int
#define fputc	       putc
#else
#define uchar	       unsigned char
#endif
#endif

#define bool	       uchar
#define time_t	       long
#define TRUE	       (1)
#define FALSE	       (0)

#define DEFN1	       "makefile"       /* Default names  */
#if unix
#define DEFN2	       "Makefile"
#endif
#if eon
#define DEFN2	       "Makefile"
#endif
#if VM
#define DEFN2	       "Makefile $"
#endif
/* os9 is case insensitive */

#define LZ	       (4096)	/* Line size  */



/*
 * A name.	This represents a file, either to be made, or
 * existant
 */

struct name {
    struct name *n_next;	/* Next in the list of names */
    char *n_name;		/* Called */
    struct line *n_line;	/* Dependencies */
    char *n_dyn;		/* Dynamic defined dependancy */
    time_t n_time;		/* Modify time of this name */
    uchar n_flag;		/* Info about the name */
};
#define N_MARK	       0x01	/* For cycle check */
#define N_DONE	       0x02	/* Name looked at */
#define N_TARG	       0x04	/* Name is a target */
#define N_PREC	       0x08	/* Target is precious */
#define N_DOUBLE       0x10	/* Double colon target */
#define N_DYN	       0x20	/* Use n_dyn for dependancy */
/*
 * Definition of a target line.
 */
struct line {
    struct line *l_next;	/* Next line (for ::) */
    struct depend *l_dep;	/* Dependents for this line */
    struct cmd *l_cmd;		/* Commands for this line */
};


/*
 * List of dependents for a line
 */
struct depend {
    struct depend *d_next;	/* Next dependent */
    struct name *d_name;	/* Name of dependent */
};


/*
 * Commands for a line
 */
struct cmd {
    struct cmd *c_next; 	/* Next command line */
    char *c_cmd;		/* Command line */
};


/*
 * Macro storage
 */
struct macro {
    struct macro *m_next;	/* Next variable */
    char *m_name;		/* Called ... */
    char *m_val;		/* Its value */
    uchar m_flag;		/* Infinite loop check */
};

extern char *myname;
extern struct name namehead;
extern struct macro *macrohead;
#if MSDOS || OS2
extern struct macro *macroheade;/* Environmental macros */
extern struct macro *macroheadc;/* command line macros */
#endif
extern struct name *firstname;
extern bool silent;
extern bool ignore;
extern bool rules;
extern bool dotouch;
extern bool quest;
extern bool domake;
extern char str1[];
extern char str2[];
extern int lineno;
extern char contchar;


char *fgets();
char *index();
char *rindex();

#if MSDOS || OS2
extern bool envirf;
extern bool display;
#define index	       strchr
#define rindex	       strrchr
#endif

#ifdef VM
#define index	       strchr
#define rindex	       strrchr
#define FILE_NOT_FOUND 28
#define max(a,b)       ((a)>(b)?(a):(b))
char * malloc();
#endif

extern	char *getmacro(char *name);
extern	char *gettok(char * *ptr);
extern	char *suffix(char *name);
extern	int docmds(struct name *np);
extern	int dosh(char *string,char *shell);
extern	int make(struct name *np,int level);
extern	int make1(struct name *np,struct line *lp,struct depend *qdp);
extern	int match(char *a,char *b);
extern	int setname(char *m1,char *m2,char *m3,char *n);
extern	int usage(void );
extern	struct cmd *newcmd(char *str,struct cmd *cp);
extern	struct depend *newdep(struct name *np,struct depend *dp);
extern	struct macro *getmp(char *name);
extern	struct macro *setmacro(char *name,char *val);
extern	struct macro *setmacroc(char *name,char *val);
extern	struct macro *setmacroe(char *name,char *val);
extern	struct name *newname(char *name);
extern	unsigned char dyndep(struct name *np);
extern	unsigned char getline(char *str,FILE *fd);
extern	void check(struct name *np);
extern	void circh(void );
extern	void docmds1(struct name *np,struct line *lp);
extern	void doexp(char * *to,char *from,int *len,char *buf);
extern	void expand(char *str);
extern	void input(FILE *fd);
extern	void main(int argc,char * *argv,char * *envp);
extern	void makerules(void );
extern	void modtime(struct name *np);
extern	void newline(struct name *np,struct depend *dp,struct cmd *cp,int flag);
extern	void precious(void );
extern	void prt(void );
extern	void touch(struct name *np);
static	struct macro *setmacrox(char *name,char *val,struct macro * *macrohead);
extern	void fatal(char *msg,...);
