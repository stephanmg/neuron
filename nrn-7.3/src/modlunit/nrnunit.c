#include <../../nmodlconf.h>
#include "model.h"
#include "parse1.h"

#define IONCUR  0
#define IONEREV 1
#define IONIN   2
#define IONOUT  3

static int point_process = 0;
static List *current, *concen, *potential;
static unit_chk();
static int iontype();

nrn_unit_init() {
	current = newlist();
	concen = newlist();
	potential = newlist();
}

#if NRNUNIT
nrn_unit_chk() {
	Item *q;
	
	unit_chk("v", "millivolt");
	unit_chk("t", "ms");
	unit_chk("dt", "ms");
	unit_chk("celsius", "degC");
	unit_chk("diam", "micron");
	unit_chk("area", "micron2");

	ITERATE(q, current) {
		if (point_process) {
			unit_chk(SYM(q)->name, "nanoamp");
		}else{
			unit_chk(SYM(q)->name, "milliamp/cm2");
		}
	}
	ITERATE(q, concen) {
		unit_chk(SYM(q)->name, "milli/liter");
	}
	ITERATE(q, potential) {
		unit_chk(SYM(q)->name, "millivolt");
	}
}

static
unit_chk(name, unit)
	char *name, *unit;
{
	Symbol *s;
	
	s = lookup(name);
	if (s) {
		Unit_push(decode_units(s));
		Unit_push(unit);
		if (!unit_cmp_exact()) {
Sprintf(buf, "%s must have the units, %s, instead of %s.\n",
			name, unit, decode_units(s));
			diag(buf, (char *)0);
		}
		unit_pop();
		unit_pop();
	}
}

nrn_list(qtype, qlist)
	Item *qtype, *qlist;
{
	List **plist;
	Item *q;
	
	switch (SYM(qtype)->type) {
	case NONSPECIFIC:
	case ELECTRODE_CURRENT:
		plist = &current;
		break;
	case SUFFIX:
		plist = (List **)0;
		if (strcmp(SYM(qtype)->name, "POINT_PROCESS") == 0) {
			point_process = 1;
		}
		if (strcmp(SYM(qtype)->name, "ARTIFICIAL_CELL") == 0) {
			point_process = 1;
		}
		break;
	default:
		plist = (List **)0;
		break;
	}
	if (plist && qlist) {
		ITERATE(q, qlist) {
			Lappendsym(*plist, SYM(q));
		}
	}
}

nrn_use(qion, qreadlist, qwritelist)
	Item *qion, *qreadlist, *qwritelist;
{
	int i;
	List *l;
	Item *q;
	Symbol *ion;
	
	ion = SYM(qion);
	for (i=0; i<2; i++) {
		if (i == 0) {
			l = (List *)qreadlist;
		}else{
			l = (List *)qwritelist;
		}
		if (l) ITERATE(q, l) {
			switch (iontype(SYM(q)->name, ion->name)) {
			case IONCUR:
				Lappendsym(current, SYM(q));
				break;
			case IONEREV:
				Lappendsym(potential, SYM(q));
				break;
			case IONIN:
			case IONOUT:
				Lappendsym(concen, SYM(q));
				break;
			}
		}
	}
}

static
int iontype(s1, s2)	/* returns index of variable in ion mechanism */
	char *s1, *s2;
{
	Sprintf(buf, "i%s", s2);
	if (strcmp(buf, s1) == 0) {
		return IONCUR;
	}
	Sprintf(buf, "e%s", s2);
	if (strcmp(buf, s1) == 0) {
		return IONEREV;
	}
	Sprintf(buf, "%si", s2);
	if (strcmp(buf, s1) == 0) {
		return IONIN;
	}
	Sprintf(buf, "%so", s2);
	if (strcmp(buf, s1) == 0) {
		return IONOUT;
	}
	Sprintf(buf, "%s is not a valid ionic variable for %s", s1, s2);
	diag(buf, (char *)0);
	return -1;
}

#endif /*NRNUNIT*/
