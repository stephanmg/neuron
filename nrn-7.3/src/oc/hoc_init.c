#include <../../nrnconf.h>
/* /local/src/master/nrn/src/oc/hoc_init.c,v 1.25 1999/11/08 17:48:58 hines Exp */

#include "hoc.h"
#include "parse.h"
#include <math.h>
#include "equation.h"

extern double chkarg();

extern double	Log(), Log10(), hoc1_Exp(), Sqrt(), integer();
extern int	System(), Prmat(), solve(), eqinit(), Plt(), hoc_atan2();
extern int	symbols(), PRintf(), Xred(), hoc_Sred();
extern int	ropen(), wopen(), xopen(), Fscan(), Fprint();
extern int	hoc_Graph(), hoc_Graphmode(), Plot(), axis(), hoc_Sprint();
extern int 	hoc_fmenu(), hoc_Getstr(), hoc_Strcmp();
extern int	hoc_Lw(), machine_name(), hoc_Saveaudit(), hoc_Retrieveaudit();
extern int	hoc_plotx(), hoc_ploty(), hoc_regraph();
extern int	hoc_startsw(), hoc_stopsw(), hoc_object_id();
extern int	hoc_allobjects(), hoc_allobjectvars();
extern int	hoc_xpanel(), hoc_xbutton(), hoc_xmenu(), hoc_xslider();
extern int	hoc_xfixedvalue(), hoc_xvarlabel(), hoc_xradiobutton();
extern int	hoc_xvalue(), hoc_xpvalue(), hoc_xlabel(), ivoc_style();
extern int	hoc_boolean_dialog(), hoc_string_dialog(), hoc_continue_dialog();
extern int	hoc_single_event_run(), hoc_notify_iv();
extern int	hoc_pointer(), hoc_Numarg(), hoc_Argtype(), hoc_exec_cmd();
extern int	hoc_load_proc(), hoc_load_func(), hoc_load_template(), hoc_load_file();
extern int	hoc_load_java();
extern int      hoc_xcheckbox(), hoc_xstatebutton(), hoc_Symbol_limits();
extern int	hoc_coredump_on_error(), hoc_checkpoint(), hoc_quit();
extern int	hoc_object_push(), hoc_object_pop(), hoc_pwman_place();
extern int	hoc_show_errmess_always(), hoc_execute1(), hoc_secname();
extern int	hoc_neuronhome(), hoc_Execerror();
extern int	hoc_sscanf(), hoc_save_session(), hoc_print_session();
extern int	hoc_Chdir(), hoc_getcwd(), hoc_Symbol_units(), hoc_stdout();
extern int	hoc_name_declared(), hoc_unix_mac_pc(), hoc_show_winio();
extern int	hoc_usemcran4(), hoc_mcran4(), hoc_mcran4init();
extern int	hoc_nrn_load_dll(), hoc_nrnversion(), hoc_object_pushed();
extern int	hoc_mallinfo();

#if PVM
extern int      numprocs(), myproc(), psync();
#endif
#if 0
extern int	hoc_co();
#endif
extern int	hoc_regexp_compile(), hoc_regexp_search();
extern int	hoc_Setcolor();
#if	DOS || defined(WIN32) /*|| defined(MAC)*/
extern int	hoc_settext();
extern hoc_win_exec();
extern double	erf(), erfc();	/* supplied by unix */
#endif
extern Symlist	*symlist;

#if MAC
static double Fabs(x) double x; { return (x>0.) ? x : -x; }
static double Erf(x) double x; { return erf(x); }
static double Erfc(x) double x; { return erfc(x); }
#endif

static struct { /* Keywords */
	char	*name;
	int	kval;
} keywords[] = {
	"proc",		parsePROC,
	"func",		FUNC,
	"obfunc",	HOCOBJFUNC,
	"return",	RETURN,
	"break",	BREAK,
	"continue",	CONTINUE,
	"stop",		STOPSTMT,
	"if",		IF,
	"else",		ELSE,
	"while",	WHILE,
	"for",		FOR,
	"print",	PRINT,
	"delete",	parseDELETE,
	"read",		READ,
	"debug",	DEBUG,
	"double",	parseDOUBLE,
	"em",		EDIT,
	"depvar",	DEPENDENT,
	"eqn",		EQUATION,
	"local",	LOCAL,
	"localobj",	LOCALOBJ,
	"strdef",	STRDEF,
	"parallel",	PARALLEL,
	"help",		HELP,
	"iterator",	ITERKEYWORD,
	"iterator_statement", ITERSTMT,
#if CABLE
	"create",	SECTIONKEYWORD,
	"connect",	CONNECTKEYWORD,
	"setpointer",	SETPOINTERKEYWORD,
	"access",	ACCESSKEYWORD,
	"insert",	INSERTKEYWORD,
	"uninsert",	UNINSERTKEYWORD,
	"forall",	FORALL,
	"ifsec",	IFSEC,
	"forsec",	FORSEC,
#endif /*CABLE*/
#if OOP
	"begintemplate", BEGINTEMPLATE,
	"endtemplate", ENDTEMPLATE,
	"objectvar",	OBJVARDECL,
	"objref",	OBJVARDECL,
	"public",	PUBLICDECL,
	"external",	EXTERNALDECL,
	"new",		NEW,
#endif
	0,		0
};
static struct {	/* Constants */
	char	*name;
	double cval;
} consts[] = {
	"PI",	3.14159265358979323846,
	"E",	2.71828182845904523536,
	"GAMMA",0.57721566490153286060,	/* Euler */
	"DEG", 57.29577951308232087680,	/* deg/radian */
	"PHI",	1.61803398874989484820,	/* golden ratio */
	"FARADAY", 96485.309,	/*coulombs/mole*/
	"R", 8.31441,		/*molar gas constant, joules/mole/deg-K*/
	0,	0
};

static struct {	/* Built-ins */
	char	*name;
	double	(*func)();
} builtins[] = {
	"sin",	sin,
	"cos",	cos,
	"atan",	atan,
	"tanh", tanh,
	"log",	Log,	/* checks argument */
	"log10",Log10,	/* checks argument */
	"exp",	hoc1_Exp,	/* checks argument */
	"sqrt",	Sqrt,	/* checks argument */
	"int",	integer,
#if MAC
	"abs",	Fabs,
	"erf",	Erf,
	"erfc", Erfc,
#else
	"abs",	fabs,
	"erf",	erf,
	"erfc", erfc,
#endif
	0,	0
};
static struct { /* Builtin functions with multiple or variable args */
	char 	*name;
	int	(*fun_blt)();
} fun_bltin[] = {
	"atan2",	hoc_atan2,
	"system",	System,
	"prmat",	Prmat,
	"solve",	solve,
	"eqinit",	eqinit,
	"plt",		Plt,
	"axis",		axis,
	"plot",		Plot,
	"plotx",	hoc_plotx,
	"ploty",	hoc_ploty,
	"regraph",	hoc_regraph,
	"symbols",	symbols,
	"printf",	PRintf,
	"xred",		Xred,
	"sred",		hoc_Sred,
	"ropen",	ropen,
	"wopen",	wopen,
	"xopen",	xopen,
	"hoc_stdout",	hoc_stdout,
	"chdir",	hoc_Chdir,
	"fprint",	Fprint,
	"fscan",	Fscan,
	"sscanf", hoc_sscanf,
	"sprint",	hoc_Sprint,
	"graph",	hoc_Graph,
	"graphmode",	hoc_Graphmode,
	"fmenu",	hoc_fmenu,
	"lw",		hoc_Lw,
	"getstr",	hoc_Getstr,
	"strcmp",	hoc_Strcmp,
	"setcolor",	hoc_Setcolor,
	"startsw",	hoc_startsw,
	"stopsw",	hoc_stopsw,
	"object_id",	hoc_object_id,
	"allobjectvars", hoc_allobjectvars,
	"allobjects",	hoc_allobjects,
	"xpanel",	hoc_xpanel,
	"xbutton",	hoc_xbutton,
	"xcheckbox",    hoc_xcheckbox,
	"xstatebutton", hoc_xstatebutton,
	"xlabel",	hoc_xlabel,
	"xmenu",	hoc_xmenu,
	"xvalue",	hoc_xvalue,
	"xpvalue",	hoc_xpvalue,
	"xradiobutton",	hoc_xradiobutton,
	"xfixedvalue",	hoc_xfixedvalue,
	"xvarlabel",	hoc_xvarlabel,
	"xslider",	hoc_xslider,
	"boolean_dialog",	hoc_boolean_dialog,
	"continue_dialog",	hoc_continue_dialog,
	"string_dialog",	hoc_string_dialog,
	"doEvents",	hoc_single_event_run,
	"doNotify",	hoc_notify_iv,
	"ivoc_style",	ivoc_style,
	"numarg",	hoc_Numarg,
	"argtype",	hoc_Argtype,
	"hoc_pointer_",	hoc_pointer,		/* for internal use */
	"nrn_mallinfo", hoc_mallinfo,
	"execute",	hoc_exec_cmd,
	"execute1",	hoc_execute1,
	"load_proc",	hoc_load_proc,
	"load_func",	hoc_load_func,
	"load_template", hoc_load_template,
	"load_file", hoc_load_file,
	"load_java", hoc_load_java,
	"unix_mac_pc", hoc_unix_mac_pc,
	"show_winio", hoc_show_winio,
	"nrn_load_dll", hoc_nrn_load_dll,
	"machine_name", machine_name,
	"saveaudit", hoc_Saveaudit,
	"retrieveaudit", hoc_Retrieveaudit,
	"coredump_on_error", hoc_coredump_on_error,
	"checkpoint", hoc_checkpoint,
	"quit",		 hoc_quit,
	"object_push",	hoc_object_push,
	"object_pop",	hoc_object_pop,
	"pwman_place",	hoc_pwman_place,
	"save_session", hoc_save_session,
	"print_session", hoc_print_session,
	"show_errmess_always", hoc_show_errmess_always,
	"execerror", hoc_Execerror,
	"variable_domain", hoc_Symbol_limits,
	"name_declared", hoc_name_declared,
	"use_mcell_ran4", hoc_usemcran4,
	"mcell_ran4", hoc_mcran4,
	"mcell_ran4_init", hoc_mcran4init,
#if PVM
        "numprocs", numprocs,
        "myproc", myproc,
        "psync", psync,
#endif
#if	DOS
	"settext",	hoc_settext,
#endif
#if defined(WIN32)
   "WinExec", hoc_win_exec,
#endif
	0,	0
};

static struct { /* functions that return a string */
	char 	*name;
	int	(*strfun_blt)();
} strfun_bltin[] = {
	"secname",	hoc_secname,
	"units", hoc_Symbol_units,
	"neuronhome", hoc_neuronhome,
	"getcwd",	hoc_getcwd,
	"nrnversion", hoc_nrnversion,
	0,	0
};

static struct { /* functions that return an object */
	char 	*name;
	int	(*objfun_blt)();
} objfun_bltin[] = {
	"object_pushed", hoc_object_pushed,
	0,	0
};

double hoc_epsilon = 1.e-11;
double hoc_ac_; /*known to the interpreter to evaluate expressions with hoc_oc() */
double* hoc_varpointer; /* executing hoc_pointer(&var) will put the address of
			the variable in this location */

double hoc_cross_x_, hoc_cross_y_; /* For Graph class in ivoc */
double hoc_default_dll_loaded_;

char* neuron_home;
char* nrn_mech_dll; /* but actually only for NEURON mswin and linux */
int use_mcell_ran4_;
int nrn_xopen_broadcast_;

init()	/* install constants and built-ins table */
{
	int i;
	Symbol *s;

	use_mcell_ran4_ = 0;
	nrn_xopen_broadcast_ = 255;
	hoc_init_space();
	for (i = 0; keywords[i].name; i++)
		IGNORE(install(keywords[i].name, keywords[i].kval, 0.0, &symlist));
	for (i = 0; consts[i].name; i++) {
		s = install(consts[i].name, UNDEF, consts[i].cval, &symlist);
		s->type = VAR;
		s->u.pval = &consts[i].cval;
		s->subtype = USERDOUBLE;
	}
	for (i = 0; builtins[i].name; i++)
	{
		s = install(builtins[i].name, BLTIN, 0.0, &symlist);
		s->u.ptr = builtins[i].func;
	}
	for (i = 0; fun_bltin[i].name; i++)
	{
		s = install(fun_bltin[i].name, FUN_BLTIN, 0.0, &symlist);
		s->u.u_proc->defn.pf = fun_bltin[i].fun_blt;
		s->u.u_proc->nauto = 0;
		s->u.u_proc->nobjauto = 0;
	}
	for (i = 0; strfun_bltin[i].name; i++)
	{
		s = install(strfun_bltin[i].name, FUN_BLTIN, 0.0, &symlist);
		s->type = STRINGFUNC;
		s->u.u_proc->defn.pf = strfun_bltin[i].strfun_blt;
		s->u.u_proc->nauto = 0;
		s->u.u_proc->nobjauto = 0;
	}
	for (i = 0; objfun_bltin[i].name; i++)
	{
		s = install(objfun_bltin[i].name, FUN_BLTIN, 0.0, &symlist);
		s->type = OBJECTFUNC;
		s->u.u_proc->defn.pf = objfun_bltin[i].objfun_blt;
		s->u.u_proc->nauto = 0;
	}
	/* hoc_ac_ is a way to evaluate an expression using the interpreter */
	hoc_install_var("hoc_ac_", &hoc_ac_);
	hoc_install_var("float_epsilon", &hoc_epsilon);
	hoc_install_var("hoc_cross_x_", &hoc_cross_x_);
	hoc_install_var("hoc_cross_y_", &hoc_cross_y_);
	hoc_install_var("default_dll_loaded_", &hoc_default_dll_loaded_);

	s = install("xopen_broadcast_", UNDEF, 0.0, &hoc_symlist);
	s->type = VAR;
	s->subtype = USERINT;
	s->u.pvalint = &nrn_xopen_broadcast_;

	/* initialize pointers ( why doesn't Vax do this?) */
	hoc_access = (int *)0;
	spinit();
#if OOP
	class_registration();
	{extern Symlist *hoc_built_in_symlist, *hoc_top_level_symlist;
	 hoc_built_in_symlist = symlist;
	 symlist = (Symlist *)0;
	 /* start symlist and top level the same list */
	 hoc_top_level_symlist = symlist = (Symlist *)emalloc(sizeof(Symlist));
	 symlist->first = symlist->last = (Symbol *)0;
	}	
	hoc_install_hoc_obj();
#endif
}

hoc_unix_mac_pc() {
	hoc_ret();
#if defined(DARWIN)
	hoc_pushx(4.);
#else
#if MAC
	hoc_pushx(2.);
#else
#if defined(WIN32)
	hoc_pushx(3.);
#else
	hoc_pushx(1.);
#endif
#endif
#endif
}
hoc_show_winio() {
    int b;
    b = (int)chkarg(1, 0., 1.);
#if MAC
    hoc_sioux_show(b);
#endif
#if defined(WIN32)
	hoc_winio_show(b);
#endif
    hoc_ret();
    hoc_pushx(0.);
}

int nrn_main_launch;

hoc_nrnversion() {
	extern char* nrn_version();
	static char* p;
	int i;
	i = 1;
	if (ifarg(1)) {
		i = (int)chkarg(1, 0., 20.);
	}
	hoc_ret();
	p = nrn_version(i);
	hoc_pushstr(&p);
}

hoc_Execerror() {
	extern hoc_execerror_mes(char*, char*, int);
	char* c2 = (char*)0;
	if (ifarg(2)) {
		c2 = gargstr(2);
	}
	if (ifarg(1)) {
		hoc_execerror(gargstr(1), c2);
	}else{
		hoc_execerror_mes(c2, c2, 0);
	}
	/* never get here */
}
