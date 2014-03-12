#include <../../nrnconf.h>
/* /local/src/master/nrn/src/nrnoc/cabcode.c,v 1.37 1999/07/08 14:24:59 hines Exp */

#define HOC_L_LIST 1
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <nrnpython_config.h>
#include "section.h"
#include "membfunc.h"
#include "parse.h"
#include "membdef.h"

extern Object* hoc_thisobject;
extern Object** hoc_objgetarg();
extern Objectdata* hoc_top_level_data;
extern Symlist* hoc_top_level_symlist;
extern Symbol* hoc_table_lookup();
extern int hoc_execerror_messages;
#define symlist	hoc_symlist
extern void sec_free(), section_order();
extern Prop *prop_alloc();
extern char *secname();
extern Symlist *symlist;
extern Section* nrn_sec_pop();
extern Node	*node_ptr();
extern double	*nrn_vext_pd();
extern double	*dprop();
extern double	*nrnpy_dprop();
extern double	chkarg();
extern double	nrn_section_orientation(); /* x=0,1 end connected to parent */
extern double	nrn_connection_position(); /* x of parent for this section */
extern Prop	*nrn_mechanism(); /*returns prop given mech type, node */
extern Prop	*nrn_mechanism_check(); /*returns prop given mech type, section, and inode */
					/* error if mech not at this position */

int	tree_changed = 1;	/* installing section, changeing nseg
				and connecting sections set this flag.
				The flag is set to 0 when the topology
				is set up */
int	diam_changed = 1;	/* changing diameter, length set this flag
				The flag is set to 0 when node.a and node.b
				is set up */
extern int nrn_shape_changed_;
extern int v_structure_change;

char* (*nrnpy_pysec_name_p_)(Section*);
Object* (*nrnpy_pysec_cell_p_)(Section*);
int (*nrnpy_pysec_cell_equals_p_)(Section*, Object*);
extern Object* nrn_sec2cell(Section*);
extern int nrn_sec2cell_equals(Section*, Object*);

/* switching from Ra being a global variable to it being a section variable
opens up the possibility of a great deal of confusion and inadvertant wrong
results. To help avoid this we print a warning message whenever the value
in one section is set but no others. But only the first time through treeset.
*/
#if RA_WARNING
int nrn_ra_set = 0;
#endif

#define NSECSTACK 200
static Section *secstack[NSECSTACK + 1];
static int isecstack = 0; /* stack index */
/* don't do section stack auto correction
 in the interval between push_section() ... pop_section in hoc (also for
 point process get_loc. These should be deprecated in favor of doing
 everything with SectionRef
*/
static int skip_secstack_check = 0;

extern char* hoc_object_name();
extern double hoc_opasgn();
extern int state_discon_allowed_;
extern section_object_seen;
static int range_vec_indx();

nrn_isecstack() {
	return isecstack;
}
nrn_secstack(i) int i; {
	if (skip_secstack_check) { return; }
#if 1
	if (isecstack > i) {
		printf("The sectionstack index should be %d but it is %d\n", i, isecstack);
hoc_warning("prior to version 5.3 the section stack would not have been properly popped\n\
and the currently accessed section would have been ", secname(secstack[isecstack]));
	}
#endif
	while (isecstack > i) {
		nrn_popsec();
	}
}

nrn_initcode() {
	while(isecstack > 0) {
		nrn_popsec();
	}
	isecstack = 0;
	section_object_seen = 0;
	state_discon_allowed_ = 1;
	skip_secstack_check = 0;
}

oc_save_cabcode(a1, a2)
	int	*a1;
	int	*a2;
{
	*a1 = isecstack;
	*a2 = section_object_seen;
}

oc_restore_cabcode(a1, a2)
	int	*a1;
	int	*a2;
{
	while(isecstack > *a1) {
		nrn_popsec();
	}
	isecstack = *a1;
	section_object_seen = *a2;
}

nrn_pushsec(sec)
	Section *sec;
{
	isecstack++;
	if (isecstack >= NSECSTACK) {
		int i = NSECSTACK;
		hoc_warning("section stack overflow", (char *)0);
		while (--i >= 0) {
			fprintf(stderr, "%*s%s\n", i--, "",  secname(secstack[i]));
		}
		hoc_execerror("section stack overflow", (char *)0);
	}
	secstack[isecstack] = sec;
	if (sec) {
#if 0
		if (sec->prop && sec->prop->dparam[0].sym) {
			printf("pushsec %s\n", sec->prop->dparam[0].sym->name);
		}else{
			printf("pushsec unnamed or deleted section\n");
		}
#endif
		++sec->refcount;
	}
}

nrn_popsec() {
	if (isecstack > 0) {
		Section* sec = secstack[isecstack--];
		if (!sec) {
			return;
		}
#if 0
		if (sec->prop && sec->prop->dparam[0].sym) {
			printf("popsec %s\n", sec->prop->dparam[0].sym->name);
		}else{
			printf("popsec unnamed or with no properties\n");
		}
#endif
		if (--sec->refcount <= 0) {
#if 0
			printf("sec freed after pop\n");
#endif
			free((char*)sec);
		}
	}
}

sec_access_pop() {
	nrn_popsec();
}

#if 0
static
free_point_process() {
	free_all_point();
	free_clamp();
	free_stim();
	free_syn();
}
#endif

#if 0
clear_sectionlist()	/* merely change all SECTION to UNDEF */
{
printf("clear_sectionlist not fixed yet, doing nothing\n");
return;
	Symbol *s;
	
	free_point_process();
	if (symlist) for (s=symlist->first; s; s = s->next) {
		if (s->type == SECTION) {
			s->type = UNDEF;
no longer done this way see OPARINFO
			if (s->arayinfo) {
				free((char *)s->arayinfo);
				s->arayinfo = (Arrayinfo *)0;
			}
		}
	nrn_initcode();
	secstack[0] = (Section *)0;
	}
	if (section) {
		sec_free(section, nsection);
		section = (Section *)0;
		nsection = 0;
	}
}
#endif

add_section() /* symbol at pc+1, number of indices at pc+2 */
{
	Symbol *sym;
	int nsub, size;
	Item** pitm;
	
	sym = (pc++)->sym;
/*printf("declaring %s as section\n", sym->name);*/
	if (sym->type == SECTION) { int total, i;
		total = hoc_total_array(sym);
		for (i=0; i<total; ++i) {
			sec_free(*(OPSECITM(sym) + i));
		}
		free((char*)OPSECITM(sym));
		hoc_freearay(sym);
	}else{
		assert (sym->type == UNDEF);
		sym->type = SECTION;
		hoc_install_object_data_index(sym);
	}
	nsub = (pc++)->i;
	if (nsub) {
		size = hoc_arayinfo_install(sym, nsub);
	}else{
		size = 1;
	}
hoc_objectdata[sym->u.oboff].psecitm = pitm = (Item **)emalloc(size*sizeof(Item*));
	if (hoc_objectdata == hoc_top_level_data) {
		new_sections((Object*)0, sym, pitm, size);
	}else{
		new_sections(hoc_thisobject, sym, pitm, size);
	}
#if 0
printf("%s", s->name);
for (i=0; i<ndim; i++) {printf("[%d]",s->arayinfo->sub[i]);}
printf(" is a section name\n");
#endif
}

Object* nrn_sec2cell(Section* sec) {
	if (sec->prop) {
		if (sec->prop->dparam[6].obj) {
			return sec->prop->dparam[6].obj;
		}else if (nrnpy_pysec_cell_p_) {
			Object* o = (*nrnpy_pysec_cell_p_)(sec);
			if (o) {
				--o->refcount;
			}
			return o;
		}
	}
	return (Object*)0;
}

int nrn_sec2cell_equals(Section* sec, Object* obj) {
	if (sec->prop) {
		if (sec->prop->dparam[6].obj) {
			return sec->prop->dparam[6].obj == obj;
		}else if (nrnpy_pysec_cell_equals_p_) {
			return (*nrnpy_pysec_cell_equals_p_)(sec, obj);
		}
	}
	return 0;
}

static Section* new_section(ob, sym, i)
	Object* ob;
	Symbol* sym;
	int i;
{
	Section* sec;
	Prop* prop;
	static Symbol* nseg;
	double d;

	if (!nseg) {
		nseg = hoc_lookup("nseg");
	}
	sec = sec_alloc();
	section_ref(sec);
	prop = prop_alloc(&(sec->prop), CABLESECTION, (Node*)0);
	prop->dparam[0].sym = sym;
	prop->dparam[5].i = i;
	if (ob) {
		prop->dparam[6].obj = ob;
	}else{
		prop->dparam[6].obj = (Object*)0;
	}
#if USE_PYTHON
	prop->dparam[PROP_PY_INDEX]._pvoid = (void*)0;
#endif
	nrn_pushsec(sec);
	d = (double)DEF_nseg;
	cable_prop_assign(nseg, &d, 0);
	tree_changed = 1;
/*printf("new_section %s\n", secname(sec));*/
	return sec;
}
	
new_sections(ob, sym, pitm, size)
	Object* ob;
	Symbol* sym;
	Item** pitm;
	int size;
{
	int i;
	for (i=0; i < size; ++i) {
		Section* sec = new_section(ob, sym, i);
		if (ob) {
			if (ob->secelm_) {
				pitm[i] = insertsec(ob->secelm_->next, sec);
			}else{
				pitm[i] = lappendsec(section_list, sec);
			}
			ob->secelm_ = pitm[i];
		}else{
			pitm[i] = lappendsec(section_list, sec);
		}
		sec->prop->dparam[8].itm = pitm[i];
	}
}

#if USE_PYTHON
Section* nrnpy_newsection(void* v) {
	Item* itm;
	Section* sec;
	sec = new_section((Object*)0, (Symbol*)0, 0);
	sec->prop->dparam[PROP_PY_INDEX]._pvoid = v;
	itm = lappendsec(section_list, sec);
	sec->prop->dparam[8].itm = itm;
	return sec;
}
#endif

delete_section() {
	Section* sec;
	Object* ob;
	Item** pitm;
	Symbol* sym;
	int i;
	sec = chk_access();
	if (!sec->prop->dparam[0].sym) {
		hoc_execerror("Cannot delete an unnamed section", (char*)0);
	}
	sym = sec->prop->dparam[0].sym;
	ob = sec->prop->dparam[6].obj;
	i = sec->prop->dparam[5].i;
	if (ob) {
		pitm = ob->u.dataspace[sym->u.oboff].psecitm + i;
	}else{
		pitm = hoc_top_level_data[sym->u.oboff].psecitm + i;
	}
	sec_free(*pitm);
	*pitm = 0;
	ret(0.);
}

/*
At this point, all the sections are cables and each section has the following
properties (except for section 0). Only one property with 9 Datums

section[i].prop->dparam[0].sym 	pointer to section symbol
			[1].val	position (0--1) of connection to parent
			[2].val	length of section in microns
			[3].val	first node at position 0 or 1
			[4].val rall branch
			[5].i	aray index
			[6].obj pointer to the object pointer
			[7].val Ra
			[8].itm hoc_Item* with Section* as element
			[9]._pvoid NrnThread*
			[PROP_PY_INDEX].pvoid pointer to the Python Section object
The first element allows us to find the symbol when we know the section number.
If an array section the index can be computed with
i - (section[i].sym)->u.u_auto
*/

double section_length(sec)
	Section* sec;
{
	double x;
	if (sec->recalc_area_ && sec->npt3d) {
		sec->prop->dparam[2].val = sec->pt3d[sec->npt3d - 1].arc;
	}
	x = sec->prop->dparam[2].val;
	if (x <= 1e-9) {
		x = sec->prop->dparam[2].val = 1e-9;
	}
	return x;
}

int arc0at0(sec)
	Section* sec;
{
	return ((sec->prop->dparam[3].val) ? 0 : 1);
}

double nrn_ra(sec)
	Section* sec;
{
	return sec->prop->dparam[7].val;
}

cab_alloc(p)
	Prop *p;
{
	Datum *pd;
#if USE_PYTHON
#define CAB_SIZE 11
#else
#define CAB_SIZE 10
#endif
	pd = nrn_prop_datum_alloc(CABLESECTION, CAB_SIZE, p);
	pd[1].val = pd[3].val = 0.;
	pd[2].val = DEF_L;
	pd[4].val = DEF_rallbranch;
	pd[7].val = DEF_Ra;
	p->dparam = pd;
	p->param_size = CAB_SIZE; /* this one is special since it refers to dparam */
}

morph_alloc(p)
	Prop *p;
{
	
	double *pd;
	pd = nrn_prop_data_alloc(MORPHOLOGY, 1, p);
	pd[0] = DEF_diam; /* microns */
	diam_changed = 1;
	p->param = pd;
	p->param_size = 1;
}
	
double nrn_diameter(nd)
	Node* nd;
{
	Prop* p;
	p = nrn_mechanism(MORPHOLOGY, nd);
	return p->param[0];
}	

nrn_chk_section(s)
	Symbol *s;
{
	if (s->type != SECTION) {
		execerror("Not a SECTION name:", s->name);
	}
}

Section*
chk_access()
{
	Section* sec = secstack[isecstack];
	if (!sec || !sec->prop) {
		/* use any existing section as a default section */
		hoc_Item* qsec;
		ForAllSections(lsec)
			if (lsec->prop) {
				sec = lsec;
				++sec->refcount;
				secstack[isecstack] = sec;
/*printf("automatic default section %s\n", secname(sec));*/
				break;
			}
		}
	}
	if (!sec) {
		execerror("Section access unspecified", (char *)0);
	}
	if (sec->prop) {
		return sec;
	}else{
		execerror("Accessing a deleted section", (char*)0);
	}
}

Section*
nrn_noerr_access() /* return 0 if no accessed section */
{
	Section* sec = secstack[isecstack];
	if (!sec || !sec->prop) {
		/* use any existing section as a default section */
		hoc_Item* qsec;
		ForAllSections(lsec)
			if (lsec->prop) {
				sec = lsec;
				++sec->refcount;
				secstack[isecstack] = sec;
/*printf("automatic default section %s\n", secname(sec));*/
				break;
			}
		}
	}
	if (!sec) {
		return (Section*)0;
	}
	if (sec->prop) {
		return sec;
	}else{
		return (Section*)0;
	}
}

/*sibling and child pointers do not ref sections to avoid mutual references */
/* the sibling list is ordered according to increasing distance from parent */

nrn_remove_sibling_list(sec)
	Section* sec;
{
	Section* s;
	if (sec->parentsec) {
		if (sec->parentsec->child == sec) {
			sec->parentsec->child = sec->sibling;
			return;
		}
		for (s = sec->parentsec->child; s; s = s->sibling) {
			if (s->sibling == sec) {
				s->sibling = sec->sibling;
				return;
			}
		}
	}
}

static double ncp_abs(sec) Section* sec; {
	double x = nrn_connection_position(sec);
	if (sec->parentsec) {
		if (!arc0at0(sec->parentsec)) {
			return 1. - x;
		}
	}
	return x;
}

nrn_add_sibling_list(sec)
	Section* sec;
{
	Section* s;
	double x;
	if (sec->parentsec) {
		x = ncp_abs(sec);
		s = sec->parentsec->child;
		if (!s || x <= ncp_abs(s)) {
			sec->sibling = s;
			sec->parentsec->child = sec;
			return;
		}
		for (; s->sibling; s = s->sibling) {
			if (x <= ncp_abs(s->sibling)) {
				sec->sibling = s->sibling;
				s->sibling = sec;
				return;
			}
		}
		s->sibling = sec;
		sec->sibling = 0;
	}
}

static
reverse_sibling_list(sec)
	Section* sec;
{
	int scnt;
	Section* ch;
	Section** pch;
	for (scnt=0, ch = sec->child; ch; ++scnt, ch=ch->sibling) {
		hoc_pushobj((Object**)ch);			
	}
	pch = &sec->child;
	while(scnt--) {
		Object** hoc_objpop();
		ch = (Section*)hoc_objpop();
		*pch = ch;
		pch = &ch->sibling;
		ch->parentnode = 0;
	}
	*pch = 0;
}

disconnect() {
	nrn_disconnect(chk_access());
	ret(0.);
}

static reverse_nodes(sec)
	Section* sec;
{
	int i, j;
	Node* nd;
/*	printf("reverse %d nodes for %s\n", sec->nnode-1, secname(sec));*/
	for (i=0, j=sec->nnode-2; i < j; ++i, --j) {
		nd = sec->pnode[i];
		sec->pnode[i] = sec->pnode[j];
		sec->pnode[i]->sec_node_index_ = i;
		sec->pnode[j] = nd;
		nd->sec_node_index_ = j;
	}
}

nrn_disconnect(sec)
	Section* sec;
{
	Section* ch;
	Section* oldpsec = sec->parentsec;
	Node* oldpnode = sec->parentnode;
	if (!oldpsec) {
		return;
	}
	nrn_remove_sibling_list(sec);
	sec->parentsec = (Section*)0;
	sec->parentnode = (Node*)0;
	nrn_parent_info(sec);
	nrn_relocate_old_points(sec, oldpnode, sec, sec->parentnode);
	for (ch = sec->child; ch; ch = ch->sibling) if (nrn_at_beginning(ch)){
		ch->parentnode = sec->parentnode;
		nrn_relocate_old_points(ch, oldpnode, ch, ch->parentnode);
	}
	section_unref(oldpsec);
	tree_changed = 1;
}

static connectsec_impl(parent, sec) Section* parent, *sec;
{
	Section* ch;
	Section* oldpsec = sec->parentsec;
	Node* oldpnode = sec->parentnode;
	double d1, d2;
	Datum *pd;
	d2 = xpop();
	d1 = xpop();
	if (d1 != 0. && d1 != 1.) {
		hoc_execerror(secname(sec), " must connect at position 0 or 1");
	}
	if (d2 < 0. || d2 > 1.) {
		hoc_execerror(secname(sec), " must connect from 0<=x<=1 of parent");
	}
	if (sec->parentsec) {
		fprintf(stderr, "Notice: %s(%g)",
			secname(sec), nrn_section_orientation(sec)
		);
		fprintf(stderr, " had previously been connected to parent %s(%g)\n",
			secname(sec->parentsec), nrn_connection_position(sec)
		);
		nrn_remove_sibling_list(sec);
	}
	if (nrn_section_orientation(sec) != d1) {
		reverse_sibling_list(sec);
		reverse_nodes(sec);
	}
	pd = sec->prop->dparam;
	pd[1].val = d2;
	pd[3].val = d1;
	section_ref(parent);
	sec->parentsec = parent;
	nrn_add_sibling_list(sec);
	sec->parentnode = (Node*)0;
	nrn_parent_info(sec);
	nrn_relocate_old_points(sec, oldpnode, sec, sec->parentnode);
	for (ch = sec->child; ch; ch = ch->sibling) if (nrn_at_beginning(ch)){
		ch->parentnode = sec->parentnode;
		nrn_relocate_old_points(ch, oldpnode, ch, ch->parentnode);
	}
	if (oldpsec) {
		section_unref(oldpsec);
	}else if (oldpnode) {
		nrn_node_destruct1(oldpnode);
	}
	tree_changed = 1;
	diam_changed = 1;
}

simpleconnectsection() /* 2 expr on stack and two sections on section stack */
	/* for a prettier syntax: connect sec1(x), sec2(x) */
{
	Section* parent, *child;
	parent = nrn_sec_pop();
	child = nrn_sec_pop();
	connectsec_impl(parent, child);	
}

connectsection() /* 2 expr on stack and section symbol on section stack */
{
	Section* parent, *child;
	child = nrn_sec_pop();
	parent = chk_access();
	connectsec_impl(parent, child);
}

static Section *
Sec_access()	/* section symbol at pc */
{
	Objectdata* odsav;
	Object* obsav = 0;
	Symlist* slsav;
	Item* itm;
	Symbol *s = (pc++)->sym;

	if (!s) {
		return chk_access();
	}
    if (s->public == 2) {
	s = s->u.sym;
	odsav = hoc_objectdata_save();
	obsav = hoc_thisobject;
	slsav = hoc_symlist;
	hoc_objectdata = hoc_top_level_data;
	hoc_thisobject = 0;
	hoc_symlist = hoc_top_level_symlist;
    }
	nrn_chk_section(s);
	itm = OPSECITM(s)[range_vec_indx(s)];
    if (obsav) {
	hoc_objectdata = hoc_objectdata_restore(odsav);
	hoc_thisobject = obsav;
	hoc_symlist = slsav;
    }
	if (!itm) {
		hoc_execerror(s->name, ": section was deleted");
	}
	return itm->element.sec;
#if 0
printf("accessing %s with index %d\n", s->name, access_index);	
#endif
}

sec_access() { /* access section */
	Section** psec;
	Section* sec = chk_access();
	++sec->refcount;
	nrn_popsec();
	psec = secstack + isecstack;
	if (*psec) {
		section_unref(*psec);
	}
	*psec = sec;
}

sec_access_object() { /* access section */
	Section** psec;
	Section* sec;
	if (! section_object_seen) {
		hoc_execerror("Access: Not a section", (char*)0);
	}
	sec = chk_access();
	++sec->refcount;
	nrn_popsec();
	psec = secstack + isecstack;
	if (*psec) {
		section_unref(*psec);
	}
	*psec = sec;
	section_object_seen = 0;
}

sec_access_push()
{
	nrn_pushsec(Sec_access());
}

Section*
nrn_sec_pop()
{
	Section* sec = chk_access();
	nrn_popsec();
	return sec;
}

/* in an object syntax a section may either be last or next to last
in either case it is pushed when it is seen in hoc_object_component
and section_object_seen is set.
If it was last then this is called with it set and we do nothing.
ie the section is on the stack till the end of the next statement.
If it was second to last then it hoc_object_component will set it
back to 0 and pop the section. Therefore we need to push a section
below to keep the stack ok when it is popped at the end of the next
statement.
*/

ob_sec_access_push(qsec)
	Item* qsec;
{
	if (!qsec) {
		hoc_execerror("section in the object was deleted", (char*)0);
	}
	nrn_pushsec(qsec->element.sec);
}

ob_sec_access() {
	if (!section_object_seen) {
		hoc_nopop();
		nrn_pushsec(secstack[isecstack]);
	}
	section_object_seen = 0;
}

/* For now there is always one more node than segments and the last
node has no properties. This fact is spread everywhere in which
nnode is dealt with */

mech_access() {	/* section symbol at pc */
	Section* sec = chk_access();
	Symbol* s = (pc++)->sym;
	mech_insert1(sec, s->subtype);
}

mech_insert1(sec, type)
	Section* sec;
	int type;
{
	int n, i;
	Node *nd, **pnd;
	Prop *m;
	/* make sure that all nodes but last of the current section have this
	   membrane type */
	/* subsequent range variables refer to this mechanism */
	/* For now, we assume:
	   1) Parameters for different mechanisms do not have to
	      have distinct names. Thus access mech marks those
	      symbols which are allowed.
	      Another access section gets back to the geometry properties.
	   2) Membrane is installed in entire section. (except last node)
	*/
	/* Is the property already allocated*/
	n = sec->nnode - 1;
	pnd = sec->pnode;
	nd = pnd[0];
#if METHOD3
	/* For method3 the parent must also have the property. Also
	   the last node may have it because one of the connecting sections
	   have it. */
	if (_method3) {
		Section* psec = sec->parentsec;

		/* must be in parent */
		Node* pnd = sec->parentnode;
		if (!nrn_mechanism(s->subtype, pnd)) {
			prop_alloc(&pnd->prop, type, pnd);
		}
		/* method3 has mechanisms in the "zero area" node. */
		n = sec->nnode;

		/* may already be in last node */
		if (nrn_mechanism(type, sec->pnode[n - 1])) {
			--n;
		}
	}
#endif
	m = nrn_mechanism(type, nd);
	if (!m) { /* all nodes get the property */
		for (i = n - 1; i >= 0; i--) {
			IGNORE(prop_alloc(&(pnd[i]->prop), type, pnd[i]));
		}
#if EXTRACELLULAR
		if (type == EXTRACELL) {
			prop_alloc(&(pnd[n]->prop), type, pnd[n]); /*even last node*/
			/* if rootnode owned by section, it gets property as well*/
			if (!sec->parentsec) {
				nd = sec->parentnode;
				if (nd) {
					prop_alloc(&nd->prop, type, nd);
				}
			}
			extcell_2d_alloc(sec);
			diam_changed = 1;
		}
#endif
	}
}

mech_uninsert() {
	Section* sec = chk_access();
	Symbol* s = (pc++)->sym;	
	mech_uninsert1(sec, s);
}

mech_uninsert1(sec, s)
	Section* sec;
	Symbol* s;
{
	/* remove the mechanism from the currently accessed section */
	int n, i;
	Prop *m, *mnext;
	int type = s->subtype;

	if (type == MORPHOLOGY 
#if EXTRACELLULAR
	  || type == EXTRACELL
#endif
	) {
		hoc_warning("Can't uninsert mechanism", s->name);
		return;
	}
	if (nrn_is_ion(type)) {
		hoc_warning("Not allowed to uninsert ions at this time", s->name);
		return;
	}
#if METHOD3
	if (_method3) {
		hoc_execerror("Can't uninsert mechanisms when spatial method is non_zero", s->name);
	}
#endif
	n = sec->nnode;
	for (i=0; i < n; ++i) {
		mnext = sec->pnode[i]->prop;
		if (mnext && mnext->type == type) {
			sec->pnode[i]->prop = mnext->next;
			single_prop_free(mnext);
			continue;
		}
		for (m = mnext; m; m = mnext) {
			mnext = m->next;
			if (mnext && mnext->type == type) {
				m->next = mnext->next;
				single_prop_free(mnext);
				break;
			}
		}
	}
}

nrn_rangeconst(sec, s, pd, op)
	Section* sec;
	Symbol* s;
	double *pd;
	int op;
{
	short n, i;
	Node *nd;
	int indx;
	double* dpr;
	double d = *pd;
#if METHOD3
	/* this is highly restrictive at this time. All parameters except
	morphology must be uniform across section connections because
	the density of channels is assumed to be uniform during matrix
	construction. ie the node does not know which side gets which value
	*/
	if (_method3) {
		n = sec->nnode;
	}else
#endif
	n = sec->nnode - 1;
	if (s->u.rng.type == VINDEX) {
		nd = node_ptr(sec, 0., (double *)0);
		if (op) {
			*pd = hoc_opasgn(op, NODEV(nd), d);
		}
		NODEV(nd) = *pd;
		nd = node_ptr(sec, 1., (double *)0);
		if (op) {
			*pd = hoc_opasgn(op, NODEV(nd), d);
		}
		NODEV(nd) = *pd;
		for (i=0; i<n; i++) {
			if (op) {
				*pd = hoc_opasgn(op, NODEV(sec->pnode[i]), d);
			}
			NODEV(sec->pnode[i]) = *pd;
		}
	}else{
		indx = range_vec_indx(s);
		if (s->u.rng.type == MORPHOLOGY) {
			if (!can_change_morph(sec)) {
				return;
			}
			diam_changed = 1;
			if (sec->recalc_area_ && op != 0) {
				nrn_area_ri(sec);
			}
		}
	
		for (i=0; i<n; i++) {
			dpr = dprop(s, indx, sec, i);
			if (op) {
				*pd = hoc_opasgn(op, *dpr, d);
			}
			*dpr = *pd;
		}
		if (s->u.rng.type == MORPHOLOGY) {
			sec->recalc_area_ = 1;
			nrn_diam_change(sec);
		}
#if EXTRACELLULAR
	    if (s->u.rng.type == EXTRACELL) {
		if (s->u.rng.index == 0) {
			diam_changed = 1;
		}
		dpr = nrn_vext_pd(s, indx, node_ptr(sec, 0., (double*)0));
		if (dpr) {
			if (op) {
				*dpr = hoc_opasgn(op, *dpr, d);
			}else{
				*dpr = d;
			}
		}
		dpr = nrn_vext_pd(s, indx, node_ptr(sec, 1., (double*)0));
		if (dpr) {
			if (op) {
				*dpr = hoc_opasgn(op, *dpr, d);
			}else{
				*dpr = d;
			}
		}				
	    }
#endif
#if METHOD3
		/* this is a hack and way too restrictive */
		if (_method3) { /*the parent node also gets the value */
			if (tree_changed) {
				setup_topology();
			}
			*(dprop(s, indx, sec->parentsec, sec->parentnode)) = *pd;
		}
#endif
	}
}

range_const()	/* rangevariable symbol at pc, value on stack */
{
	Section* sec;
	double d;
	int op;
	Symbol *s = (pc++)->sym;

	op = (pc++)->i;
	d = xpop();
	sec = nrn_sec_pop();
	
	nrn_rangeconst(sec, s, &d, op);
	hoc_pushx(d);
}

static int
range_vec_indx(s)
	Symbol *s;
{
	int indx;
	
	if (ISARRAY(s)) {
		indx = hoc_araypt(s, SYMBOL);
	}else{
		indx = 0;
	}
	return indx;
}

Prop *
nrn_mechanism(type, nd) /* returns property for mechanism at the node */
	int type;
	Node* nd;
{
	Prop *m;
	for (m = nd->prop; m; m = m->next) {
		if (m->type == type) {
			break;
		}
	}
	return m;
}
Prop *
nrn_mechanism_check(type, sec, inode)
	int type, inode;
	Section* sec;
{
	Prop *m;
	m = nrn_mechanism(type, sec->pnode[inode]);
	if (!m) {
		if (hoc_execerror_messages) {
		  Fprintf(stderr, "%s mechanism not inserted in section %s\n",
			memb_func[type].sym->name, secname(sec));
		}
		hoc_execerror("", (char *)0);
	}
	return m;
}
	
Prop *
hoc_getdata_range(type)
	int type;
{
	int inode;
	Section *sec;
	
	sec = chk_access();
	inode = node_index(sec, *getarg(1));
	return nrn_mechanism_check(type, sec, inode);
}

static Datum *
pdprop(s, indx, sec, inode) /* basically copied from dprop for use
				to connect a nmodl POINTER */
	Symbol *s;
	Section* sec;
	short inode;
	int indx;
{
	Prop *m;
	
	m = nrn_mechanism_check(s->u.rng.type, sec, inode);
	return 	m->dparam + s->u.rng.index + indx;
}

connectpointer() { /* pointer symbol at pc, target variable on stack, maybe
	range variable location on stack */
	Datum *dat;
	double *pd, *hoc_pxpop();
	double d;
	Symbol *s = (pc++)->sym;
	pd = hoc_pxpop();
	if (s->subtype != NRNPOINTER) {
		hoc_execerror(s->name, "not a model variable POINTER");
	}
#if 0
/* can't be since parser sees object syntax and generates different code. */
	if (s->type == NRNPNTVAR) {
		dat = ppnrnpnt(s);
	}else
#endif
	{
		short i;
		Section* sec;

		d = xpop();
		sec = nrn_sec_pop();
		i = node_index(sec, d);
		dat = pdprop(s, range_vec_indx(s), sec, i);
	}
	dat->pval = pd;
}

range_interpolate_single() /*symbol at pc, 2 values on stack*/
{
	double x, y;
	Symbol* s;
	Section* sec;
	double* pd, *nrn_rangepointer();
	int op;
	
	s = (pc++)->sym;
	op = (pc++)->i;
	y = xpop();
	x = xpop();
	sec = nrn_sec_pop();

	if (s->u.rng.type == MORPHOLOGY) {
		if (!can_change_morph(sec)) {
			return;
		}
		diam_changed = 1;
		if (sec->recalc_area_ && op != 0) {
			nrn_area_ri(sec);
		}
	}
	
	pd = nrn_rangepointer(sec, s, x);
	if (op) {
		y = hoc_opasgn(op, *pd, y);
	}
	*pd = y;

	if (s->u.rng.type == MORPHOLOGY) {
		sec->recalc_area_ = 1;
		nrn_diam_change(sec);
	}
	
#if EXTRACELLULAR
	if (s->u.rng.type == EXTRACELL && s->u.rng.index == 0) {
		diam_changed = 1;
	}
#endif
}

range_interpolate() /*symbol at pc, 4 values on stack*/
{
	short i, i1, i2, di;
	Section* sec;
	double y1, y2, x1, x2, x, dx, thet, y;
	double* dpr;
	Symbol *s = (pc++)->sym;
	int indx, op;
	Node *nd;

	op = (pc++)->i;
#if METHOD3
	if (_method3) {
		hoc_execerror("Range variable interpolation not implemented",
		"for new spatial discretization methods");
	}
#endif
	y2 = xpop(); y1 = xpop();
	x2 = xpop(); x1 = xpop();
	dx = x2 - x1;
	if (dx < 1e-10) {
		hoc_execerror("range variable notation r(x1:x2) requires"," x1 > x2");
	}
	sec = nrn_sec_pop();
	di = nrn_section_orientation(sec) ? -1 : 1;
	i2 = node_index(sec, x2) + di;
	i1 = node_index(sec, x1);
	
	if (s->u.rng.type == VINDEX) {
		if (x1 == 0. || x1 == 1.) {
			nd = node_ptr(sec, x1, (double *)0);
			if (op) {
				NODEV(nd) = hoc_opasgn(op, NODEV(nd), y1);
			}else{
				NODEV(nd) = y1;
			}
		}
		if (x2 == 1. || x2 == 0.) {
			nd = node_ptr(sec, x2, (double *)0);
			if (op) {
				NODEV(nd) = hoc_opasgn(op, NODEV(nd), y2);
			}else{
				NODEV(nd) = y2;
			}
		}
		for (i=i1; i != i2; i += di) {
			nd = sec->pnode[i];
			x = ((double)i + .5)/(sec->nnode - 1);
			if (di < 0) {
				x = 1. - x;
			}
			thet = (x - x1)/dx;
			if (thet >= -1e-9 && thet <= 1.+1e-9) {
				y = y1*(1.- thet) + y2*thet;
				if (op) {
					NODEV(nd) = hoc_opasgn(op, NODEV(nd), y);
				}else{
					NODEV(nd) = y;
				}
			}
		}
		return;
	}
	if (s->u.rng.type == MORPHOLOGY) {
		if (!can_change_morph(sec)) {
			return;
		}
		diam_changed = 1;
	}
	indx = range_vec_indx(s);
	for (i=i1; i != i2; i += di) {
		dpr = dprop(s, indx, sec, i);
		x = ((double)i + .5)/(sec->nnode - 1);
		if (di < 0) {
			x = 1. - x;
		}
		thet = (x - x1)/dx;
		if (thet >= -1e-9 && thet <= 1.+1e-9) {
			y = y1*(1.- thet) + y2*thet;
			if (op) {
				*dpr = hoc_opasgn(op, *dpr, y);
			}else{
				*dpr = y;
			}
		}
	}
	if (s->u.rng.type == MORPHOLOGY) {
		sec->recalc_area_ = 1;
		nrn_diam_change(sec);
	}
#if EXTRACELLULAR
	if (s->u.rng.type == EXTRACELL && s->u.rng.index == 0) {
		diam_changed = 1;
	}
    if (s->u.rng.type == EXTRACELL) {
	if (x1 == 0. || x1 == 1.) {
		dpr = nrn_vext_pd(s, indx, node_ptr(sec, x1, (double*)0));
		if (dpr) {
			if (op) {
				*dpr = hoc_opasgn(op, *dpr, y1);
			}else{
				*dpr = y1;
			}
		}
	}
	if (x2 == 1. || x2 == 0.) {
		dpr = nrn_vext_pd(s, indx, node_ptr(sec, x2, (double*)0));
		if (dpr) {
			if (op) {
				*dpr = hoc_opasgn(op, *dpr, y2);
			}else{
				*dpr = y2;
			}
		}
	}
    }
#endif
}

int nrn_exists(s, node) 
	Symbol* s;
	Node* node;
{
	if (s->u.rng.type == VINDEX) {
		return 1;
	}else if (nrn_mechanism(s->u.rng.type, node)) {
		return 1;
#if EXTRACELLULAR
	}else if (nrn_vext_pd(s, 0, node)) {
		return 1;
#endif
	}else{
		return 0;
	}
}

double* nrn_rangepointer(sec, s, d)
	Section* sec;
	Symbol* s;
	double d;
{
	/* if you change this change nrnpy_rangepointer as well */
	short i;
	Node *nd;
	int indx;

	if (s->u.rng.type == VINDEX) {
		nd = node_ptr(sec, d, (double *)0);
		return &NODEV(nd);
	}
	indx = range_vec_indx(s);
#if EXTRACELLULAR
	if (s->u.rng.type == EXTRACELL) {
		double* pd;
		pd = nrn_vext_pd(s, indx, node_ptr(sec, d, (double*)0));
		if (pd) {
			return pd;
		}
	}
#endif
	i = node_index(sec, d);
	return dprop(s, indx, sec, i);
}

/* return nil if failure instead of hoc_execerror
   and return pointer to the 0 element if an array
*/
double* nrnpy_rangepointer(sec, s, d, err)
	Section* sec;
	Symbol* s;
	double d;
	int* err;
{
	/* if you change this change nrnpy_rangepointer as well */
	short i;
	Node *nd;

	*err = 0;
	if (s->u.rng.type == VINDEX) {
		return &NODEV(node_ptr(sec, d, (double *)0));
	}
#if EXTRACELLULAR
	if (s->u.rng.type == EXTRACELL) {
		double* pd;
		nd = node_ptr(sec, d, (double*)0);
		pd = nrn_vext_pd(s, 0, nd);
		if (pd) {
			return pd;
		}
	}
#endif
	i = node_index(sec, d);
	return nrnpy_dprop(s, 0, sec, i, err);
}

rangevarevalpointer() /* symbol at pc, location on stack, return pointer on stack */
{
	short i;
	Section* sec;
	double d;
	Symbol *s = (pc++)->sym;
	Node *nd;
	int indx;

	d = xpop();
	sec = nrn_sec_pop();
	if (s->u.rng.type == VINDEX) {
		nd = node_ptr(sec, d, (double *)0);
		hoc_pushpx(&NODEV(nd));
		return;
	}
	indx = range_vec_indx(s);
	if (s->u.rng.type == MORPHOLOGY && sec->recalc_area_) {
		nrn_area_ri(sec);
	}
	if (s->u.rng.type == EXTRACELL) {
		double* pd;
		pd = nrn_vext_pd(s, indx, node_ptr(sec, d, (double*)0));
		if (pd) {
			hoc_pushpx(pd);
			return;
		}
	}
	i = node_index(sec, d);
	hoc_pushpx(dprop(s, indx, sec, i));
}

rangevareval() /* symbol at pc, location on stack, return value on stack */
{
	double *pd, *hoc_pxpop();

	rangevarevalpointer();
	pd = hoc_pxpop();
	hoc_pushx(*pd);
}

rangepoint() /* symbol at pc, return value on stack */
{
	hoc_pushx(.5);
	rangevareval();
}

int
node_index(sec, x) /* returns nearest index to x */
	Section *sec;
	double x;
{
	int i;
	double n;

	if (x < 0. || x > 1.) {
		hoc_execerror("range variable domain is 0<=x<=1", (char *)0);
	}
#if METHOD3
	if (_method3) {
		n = sec->nnode;
	}else
#endif
	n = (double)(sec->nnode - 1);
	assert(n >= 0.);
	i = n*x;
	if (i == (int)n) {
		i = n - 1;
	}
	if (sec->prop->dparam[3].val) {
		i = n - i - 1;
	}
	return i;
}

/* return -1 if x at connection end, nnode-1 if at other end */
int
node_index_exact(sec, x)
	Section* sec;
	double x;
{
	if (x == 0.) {
		if (arc0at0(sec)) {
			return -1;
		}else{
			return sec->nnode - 1;
		}
	}else if ( x == 1.) {
		if (arc0at0(sec)) {
			return sec->nnode - 1;
		}else{
			return -1;
		}
	}else{
		return node_index(sec, x);
	}
}
/*USERPROPERTY subtype of VAR used for non range variables associated
with section property
may have special actions (eg. nseg)*/

double
cable_prop_eval(sym)
	Symbol *sym;
{
	Section* sec;
	sec = nrn_sec_pop();
	switch(sym->u.rng.type) {
	
	case 0: /* not in property list so must be nnode */
#if METHOD3
		if (_method3) {
			return (double) sec->nnode;
		}else
#endif
		return (double)	sec->nnode - 1;
	case CABLESECTION:
		return sec->prop->dparam[sym->u.rng.index].val;
	default:
		hoc_execerror(sym->name, " not a USERPROPERTY");
	}
	return 0.;
}
double*
cable_prop_eval_pointer(sym)
	Symbol *sym;
{
	Section* sec;
	sec = nrn_sec_pop();
	switch(sym->u.rng.type) {
	
	case CABLESECTION:
		return &sec->prop->dparam[sym->u.rng.index].val;
	default:
		hoc_execerror(sym->name, " not a USERPROPERTY that can be pointed to");
	}
	return (double *)0;
}

#if KEEP_NSEG_PARM
int keep_nseg_parm_;
keep_nseg_parm() {
	int i = keep_nseg_parm_;
	keep_nseg_parm_ = (int)chkarg(1, 0., 1.);
	ret((double)i);
}
#endif

void nrn_change_nseg(sec, n) Section* sec; int n; {
	if (n > 32767.) {
		n = 1;
		fprintf(stderr, "requesting %s.nseg=%d but the maximum value is 32767.\n", secname(sec), n);
		hoc_warning("nseg too large, setting to 1.", (char*)0);
	}
	if (n < 1) {
		hoc_execerror("nseg", " must be positive");
	}
#if METHOD3
	if (_method3) {
		--n;
	}
#else
	if (sec->nnode == n + 1) {
		return;
	}else
#endif
	{
		Node** pnd;
		int i;
		int nold = sec->nnode;
		node_alloc(sec, (short)n + 1);
		tree_changed = 1;			
		diam_changed = 1;
		sec->recalc_area_ = 1;
		pnd = sec->pnode;
#if METHOD3
		if (_method3) {
			++n;
		}
#endif
#if KEEP_NSEG_PARM
	   if(!keep_nseg_parm_ || nold == 0)
#endif
		for (i=0; i<n; i++) {
			IGNORE(prop_alloc(&(pnd[i]->prop), MORPHOLOGY, pnd[i]));
			IGNORE(prop_alloc(&(pnd[i]->prop), CAP, pnd[i]));
		}
	}
}
cable_prop_assign(sym, pd, op)
	Symbol *sym;
	double* pd;
	int op;
{
	Section* sec;
	sec = nrn_sec_pop();
	switch(sym->u.rng.type) {
	
	case 0: /* not in property list so must be nnode */
		if (op) {
			*pd = hoc_opasgn(op, (double)(sec->nnode - 1), *pd);
		}
		nrn_change_nseg(sec, (int)(*pd));
		break;
	case CABLESECTION:
		if (sym->u.rng.index == 2) {
			if (can_change_morph(sec)) {
				if (op) {
					*pd = hoc_opasgn(op, sec->prop->dparam[2].val, *pd);
				}
				sec->prop->dparam[2].val = *pd;
				nrn_length_change(sec, *pd);
				diam_changed = 1;
				sec->recalc_area_ = 1;
			}
		}else{
			if (op) {
				*pd = hoc_opasgn(op, sec->prop->dparam[sym->u.rng.index].val, *pd);
			}
			diam_changed = 1;
			sec->recalc_area_ = 1;
			sec->prop->dparam[sym->u.rng.index].val = *pd;
		}
#if RA_WARNING
		if (sym->u.rng.index == 7) {
			++nrn_ra_set;
		}
#endif
		break;
	default:
		hoc_execerror(sym->name, " not a USERPROPERTY");
	}
}


double
nrn_connection_position(sec)
	Section* sec;
{
	return sec->prop->dparam[1].val;
}

double
nrn_section_orientation(sec)
	Section* sec;
{
	return sec->prop->dparam[3].val;
}

int nrn_at_beginning(sec)
	Section * sec;
{
	assert(sec->parentsec);
	return nrn_connection_position(sec) == nrn_section_orientation(sec->parentsec);
}

static nrn_rootnode_alloc(sec)
	Section* sec;
{
	Extnode* nde;
	Node* nrn_node_construct1();
	sec->parentnode = nrn_node_construct1();
	sec->parentnode->sec = sec;
#if EXTRACELLULAR
	if (sec->pnode[0]->extnode) {
		prop_alloc(&sec->parentnode->prop, EXTRACELL, sec->parentnode);
		extcell_node_create(sec->parentnode);
	}
#endif
}

Section*
nrn_trueparent(sec)
	Section* sec;
{
	Section* psec;
	for (psec = sec->parentsec; psec; psec = psec->parentsec) {
		if (nrn_at_beginning(sec)) {
			sec = psec;
		}else{
			break;
		}
	}
	return psec;
}

nrn_parent_info(s)
	Section* s;
{
	/* determine the parentnode using only the authoritative
	info of parentsec
	and arc length connection position */
	/* side effects are that on exit, s will have the right parentnode
	   and the true parent (not connected to any section) will have the
	   right parentnode
	*/

	Section* sec, *psec, *true_parent;
	Node* pnode;
	double x;
	
	true_parent = (Section*)0;
	for (sec = s, psec = sec->parentsec; psec; sec = psec, psec = sec->parentsec) {
		if (psec == s) {
			fprintf(stderr, "%s connection to ", secname(s));
			fprintf(stderr, "%s will form a loop\n", secname(s->parentsec));
			nrn_disconnect(s);
			hoc_execerror(secname(s), "connection will form loop");
		}
		x = nrn_connection_position(sec);
		if (x != nrn_section_orientation(psec)) {
			true_parent = psec;
			if (x == 1. || x == 0.) {
				pnode = psec->pnode[psec->nnode - 1];
			}else{
				pnode = psec->pnode[node_index(psec, x)];
			}
			break;
		}
	}
	if (true_parent == (Section*)0) {
		if (sec->parentnode) {
			/* non nil parent node in section without a parent is
				definitely valid
			*/
			pnode = sec->parentnode;
		}else{
			nrn_rootnode_alloc(sec);
			pnode = sec->parentnode;
		}
	}
	s->parentnode = pnode;
}

setup_topology()
{
	Item* qsec;
			
	/* use connection info in section property to connect nodes. */
	/* for the moment we assume uniform dx and range 0-1 */

	/* Sections may be connected to a parent at an orientation (usually
		0) at which the node belongs to some ancestor of the parent.
		All difficulties derive from that fact.
		ie. the parentnode may not belong to the parentsec. And
		parent nodes may be invalid. And if they are valid then
		we want to maintain the useful info such as point processes
		which are in them. ie, we only change the parentnode here
		if it is invalid.
	*/
	
	nrn_global_ncell = 0;

	ForAllSections(sec)
#if 0
		if (sec->nnode < 1) { /* last node is not a segment */
			hoc_execerror(secname(sec),
				" has no segments");
		}
#else
		assert(sec->nnode > 0);
#endif
		nrn_parent_info(sec);
		if (!sec->parentsec) {
			++nrn_global_ncell;
		}
	}

#if METHOD3
		if (_method3) {
			int i;
			Node* nd = root/*obsolete*/section->pnode;
			for (i=0; i<unconnected; i++) {
				IGNORE(prop_alloc(&nd[i].prop, MORPHOLOGY, nd+i));
				IGNORE(prop_alloc(&nd[i].prop, CAP, nd+i));
			}
		}
#endif
	section_order();
	tree_changed = 0;
	diam_changed = 1;
	v_structure_change = 1;
	++nrn_shape_changed_;
}

char *
secname(sec) /* name of section (for use in error messages) */
	Section *sec;
{
	extern char* hoc_araystr();
	static char name[512];
	Symbol *s;
	int indx;
	Object* ob;

	if (sec && sec->prop) {
	    if (sec->prop->dparam[0].sym) {
		s = sec->prop->dparam[0].sym;
		indx = sec->prop->dparam[5].i;
		ob = sec->prop->dparam[6].obj;
		if (ob) {
			Sprintf(name, "%s.%s%s",
				hoc_object_name(ob),
				s->name, hoc_araystr(s, indx, ob->u.dataspace));
		}else{
			Sprintf(name, "%s%s", s->name,
				hoc_araystr(s, indx, hoc_top_level_data));
		}
#if USE_PYTHON	
	    }else if (sec->prop->dparam[PROP_PY_INDEX]._pvoid) {
		assert(nrnpy_pysec_name_p_);
		return (*nrnpy_pysec_name_p_)(sec);
#endif
	    }else{
		name[0] = '\0';
	    }
	}else{
		name[0] = '\0';
	}
	return name;
}

int section_owner() {
	Section* sec;
	Object* ob;
	sec = chk_access();
	ob = nrn_sec2cell(sec);
	hoc_ret();
	hoc_push_object(ob);	
}

char* hoc_section_pathname(sec)
	Section* sec;
{
	extern char* hoc_araystr(), *hoc_object_pathname();
	Symbol* s;
	int indx;
	static char name[200];
	Object* ob;

	if (sec && sec->prop && sec->prop->dparam[0].sym) {
		s = sec->prop->dparam[0].sym;
		indx = sec->prop->dparam[5].i;
		ob = sec->prop->dparam[6].obj;
		if (ob) {
			char* p = hoc_object_pathname(ob);
			if (p) {
				Sprintf(name, "%s.%s%s", p,
				s->name, hoc_araystr(s, indx, ob->u.dataspace));
			}else{
				hoc_warning("Can't find a pathname for", secname(sec));
				return secname(sec);
			}
		}else{
			Sprintf(name, "%s%s", s->name, hoc_araystr(s, indx, hoc_objectdata));
		}
	}else{
		name[0] = '\0';
	}
	return name;
}

double
nrn_arc_position(sec, node)
	Section *sec;
	Node* node;
{
	int inode;
	double x;
	assert(sec);
	if (sec->parentnode == node) {
		x = 0.;
	}else if ((inode = node->sec_node_index_) == sec->nnode-1) {
		x = 1.;
	}else{
		x = ((double)inode+.5)/((double)sec->nnode - 1.);
	}
	if (arc0at0(sec)) {
		return  x;
	}else{
		return 1. - x;
	}
}

#if 0
double nrn_arc_position(sec, i)
	Section* sec;
	int i;
{
	double x;
	int n;
	n = sec->nnode - 1;
	if (i == n) {
		x = 1.;
	}else{
#if METHOD3
		if (_method3) {
			x = (i+1)/((double)n);
		}else
#endif
		x = (i+.5)/((double)n);
	}
	if (sec->prop->dparam[3].val) {
		x = 1. - x;
	}
	return x;
}
#endif

char * sec_and_position(sec, nd)
	Section* sec;
	Node* nd;
{
	char* buf;
	static char buf1[200];
	double x;
	assert(sec);
	buf = secname(sec);
	x = nrn_arc_position(sec, nd);
	sprintf(buf1, "%s(%g)", buf, x);
	return buf1;
}

int
segment_limits(pdx)
	double *pdx;
{
	int n;
	Section* sec;
	double l;
	
	sec = chk_access();
#if METHOD3
	if (_method3) {
		n = sec->nnode;
	}else
#endif
	n = sec->nnode - 1;
/*	l = sec->prop->dparam[2].val;*/
	l = 1.;
	*pdx = l/((double)n);
	return sec->nnode;
}

#undef PI
#define PI	3.14159265358979323846

Node*
node_exact(sec, x)
	Section *sec;
	double x;
{
	/* like node_index but give proper node when
		x is 0 or 1 as well as in between
	*/
	Node* node;
		
	assert(sec);
#if METHOD3
   if (_method3) {
	inode = (int)(x*sec->nnode + .5);	/* ranges from 0 to nnode */
		if (tree_changed) {
			setup_topology();
		}
		if(sec->prop->dparam[3].val) {
			inode = sec->nnode - inode;
		}
		--inode;
		if (inode < 0) {
			inode = sec->parentnode;
			sec = sec->parentsec;
			*psec = sec;
		}
   }else
#endif
   {
	if (x <= 0. || x >= 1.) {
		x = (x < 0.)?0.:x;
		x = (x > 1.)?1.:x;
		if(sec->prop->dparam[3].val) {
		 x = 1. - x;
		}
		if (x == 0.) {
			if (tree_changed) {
				setup_topology();
			}
			node = sec->parentnode;
		} else {
			node = sec->pnode[sec->nnode - 1];
		}
	} else {		
		node = sec->pnode[node_index(sec, x)];
	}
   }
	return node;
}

Node *
node_ptr(sec, x, parea) /* returns pointer to proper node */
	Section *sec;	/* and the area of the node */
	double x, *parea;
{
	Node *nd;

	nd = node_exact(sec, x);
	if(parea) {
		if (nd->sec->recalc_area_) {
			nrn_area_ri(nd->sec);
		}
		*parea = NODEAREA(nd);
	}
	return nd;
}

int nrn_get_mechtype(mechname) char* mechname; {
	Symbol* s;
	s = hoc_lookup(mechname);
	assert(s);
	if (s->type == TEMPLATE) {
		s = hoc_table_lookup(mechname, s->u.template->symtable);
		assert(s && s->type == MECHANISM);
	}
	return s->subtype;
}

#if VECTORIZE
int nrn_instance_count(mechtype) int mechtype; {
	if (v_structure_change) {
		v_setup_vectors();
	}
	return memb_list[mechtype].nodecount;
}
#endif

#if EXTRACELLULAR
/* want to handle vext(0), vext(1) correctly. No associated i_membrane though.*/
/*
otherwise return correct pointer to vext if it exists
return pointer to zero if a child node has extnode
return 0 if symbol is not vext.
*/
double* nrn_vext_pd(s, indx, nd)
	Symbol* s;
	int indx;
	Node* nd;
{
	static double zero;
	if (s->u.rng.type != EXTRACELL) {return (double*)0;}
#if I_MEMBRANE
	if (s->u.rng.index != 3*(nlayer) + 2) {return (double*)0;}
#else /* not I_MEMBRANE */
	if (s->u.rng.index != 3*(nlayer) + 1) {return (double*)0;}
#endif

	zero = 0.;
	if (nd->extnode) {
		return nd->extnode->v + indx;
	}else{
		Section* sec;
		/* apparently not maintaining Node.child */
		/*for (sec = nd->child; sec; sec = sec->sibling) {*/
		for (sec = nd->sec->child; sec; sec = sec->sibling) {
			if (sec->pnode[0]->extnode) {
				return &zero;
			}
		}
		return (double*)0;
	}
}
#endif

/* if you change this then change nrnpy_dprop as well */
double* dprop(s, indx, sec, inode) /* returns location of property symbol */
	Symbol *s;
	Section* sec;
	short inode;
	int indx;
{
	Prop *m;
	
	m = nrn_mechanism_check(s->u.rng.type, sec, inode);
#if EXTRACELLULAR
/* this does not handle vext(0) and vext(1) properly at this time */
#if I_MEMBRANE
	if (m->type == EXTRACELL && s->u.rng.index == 3*(nlayer) + 2) {
#else
	if (m->type == EXTRACELL && s->u.rng.index == 3*(nlayer) + 1) {
#endif
		return sec->pnode[inode]->extnode->v + indx;
	}
#endif
	if (s->subtype != NRNPOINTER) {
		if (m->ob) {
			return m->ob->u.dataspace[s->u.rng.index].pval + indx;
		}else{
			return &(m->param[s->u.rng.index]) + indx;
		}
	}else{
double **p = &((m->dparam)[s->u.rng.index + indx].pval);
		if (!(*p)) {
			hoc_execerror(s->name, "wasn't made to point to anything");
		}
		return *p;
	}
}

/* return nil instead of hoc_execerror. */
double* nrnpy_dprop(s, indx, sec, inode, err) /* returns location of property symbol */
	Symbol *s;
	Section* sec;
	short inode;
	int indx;
	int* err;
{
	Prop *m;
	
	m = nrn_mechanism(s->u.rng.type, sec->pnode[inode]);
	if (!m) {
		*err = 1;
		return (double*)0;
	}
#if EXTRACELLULAR
/* this does not handle vext(0) and vext(1) properly at this time */
#if I_MEMBRANE
	if (m->type == EXTRACELL && s->u.rng.index == 3*(nlayer) + 2) {
#else
	if (m->type == EXTRACELL && s->u.rng.index == 3*(nlayer) + 1) {
#endif
		return sec->pnode[inode]->extnode->v + indx;
	}
#endif
	if (s->subtype != NRNPOINTER) {
		if (m->ob) {
			return m->ob->u.dataspace[s->u.rng.index].pval + indx;
		}else{
			return &(m->param[s->u.rng.index]) + indx;
		}
	}else{
double **p = &((m->dparam)[s->u.rng.index + indx].pval);
		if (!(*p)) {
			*err = 2;
		}
		return *p;
	}
}

static char* objectname() {
	static char buf[100];
	if (hoc_thisobject) {
		Sprintf(buf, "%s", hoc_object_name(hoc_thisobject));
	}else{
		buf[0] = '\0';
	}
	return buf;
}

#define relative(pc)	(pc + (pc)->i)

forall_section() {
	/*statement pointed to by pc
	continuation pointed to by pc+1. template used is shortfor in code.c
	of hoc system.
	*/
	/* if inside object then forall refers only to sections in the object */
	
	Inst *savepc = pc;
	Item* qsec, *first, *last;
	extern int hoc_returning;
	char buf[200];
	char** s;
	char** hoc_strpop();
	int istk;

	/* fast forall within an object asserts that the object sections
	   are contiguous and secelm_ is the last.
	*/
	if (hoc_thisobject) {
		qsec = hoc_thisobject->secelm_;
		if (qsec) {
			for (first = qsec; first->prev->itemtype && hocSEC(first->prev)->prop->dparam[6].obj == hoc_thisobject; first = first->prev) {
				;
			}
			last = qsec->next;
		}else{
			first = (Item*)0;
			last = (Item*)0;
		}
	}else{
		first = section_list->next;
		last = section_list;
	}
	s = hoc_strpop();
	buf[0] = '\0';
	if (s) {
		sprintf(buf, "%s.*%s.*", objectname(), *s);
	}else{
		char* o = objectname();
		if (o[0]) {
			sprintf(buf, "%s.*", o);
		}
	}
	istk = nrn_isecstack();
	/* do the iteration safely. a possible command is to delete the section*/
	for (qsec = first; qsec != last;) {
		Section* sec = hocSEC(qsec);
		qsec = qsec->next;
		if (buf[0]) {
			hoc_regexp_compile(buf);
			if (! hoc_regexp_search(secname(sec))) {
				continue;
			}
		}
		nrn_pushsec(sec);
		hoc_execute(relative(savepc));
		nrn_popsec();
		if (hoc_returning) { nrn_secstack(istk); }
		if (hoc_returning == 1 || hoc_returning == 4) {
			break;
		}else if (hoc_returning == 2) {
			hoc_returning = 0;
			break;
		}else{
			hoc_returning = 0;
		}
	}
	if (!hoc_returning)
		pc =relative(savepc+1);
}

hoc_ifsec() {
	Inst *savepc = pc;
	char buf[200];
	char** s;
	char** hoc_strpop();
	extern int hoc_returning;
	
	s = hoc_strpop();
	sprintf(buf, ".*%s.*", *s);
	hoc_regexp_compile(buf);
	if (hoc_regexp_search(secname(chk_access()))) {
		hoc_execute(relative(savepc));
	}
	if (!hoc_returning)
		pc = relative(savepc+1);
}

issection() { /* returns true if string is the access section */
	hoc_regexp_compile(gargstr(1));
	if (hoc_regexp_search(secname(chk_access()))) {
		ret(1.);
	}else{
		ret(0.);
	}
}

ismembrane() { /* return true if string is an inserted membrane in the
		access section */
	char *str;
	Prop *p;

	str = gargstr(1);
	for (p = chk_access()->pnode[0]->prop; p; p = p->next) {
		if (strcmp(memb_func[p->type].sym->name, str) == 0) {
			ret(1.);
			return;
		}
	}
	ret(0.);
}


char* secaccessname() {
	return secname(chk_access());
}

sectionname() {
	char *buf, **cpp, **hoc_pgargstr();
	
	buf = secname(chk_access());
	cpp = hoc_pgargstr(1);
	hoc_assign_str(cpp, buf);
	ret(1.);
}

hoc_secname() {
	static char *buf;
	buf = secname(chk_access());
	hoc_ret();
	hoc_pushstr(&buf);
}

this_section() {
	/* return section number of currently accessed section at
		arc length postition x */
	
	Section* sec;
	sec = chk_access();
	ret((double)(unsigned long)(sec));
}
this_node() {
	/* return node number of currently accessed section at
		arc length postition x */
	
	Section* sec;
	Node* nd;
	sec = chk_access();
	nd = node_exact(sec, *getarg(1));
	ret((double)(unsigned long)nd);
}
parent_section() {
	/* return section number of currently accessed section at
		arc length postition x */
	
	Section* sec;
	sec = chk_access();
	ret((double)(unsigned long)(sec->parentsec));
}
parent_connection() {
	Section* sec;
	sec = chk_access();
	ret(nrn_connection_position(sec));
}

section_orientation() {
	Section* sec;
	sec = chk_access();
	ret(nrn_section_orientation(sec));
}

parent_node() {
	Section* sec;
	hoc_execerror("parent_node() needs to be re-implemented", 0);
	sec = chk_access();
	ret((double)(unsigned long)(sec->parentnode));
}

pop_section() {
	--skip_secstack_check;
	if (skip_secstack_check < 0) { skip_secstack_check = 0; }
	nrn_popsec();
	ret(1.);
}

/* turn off section stack fixing (in case of return,continue,break in a section
statement) between exlicit user level push_section,etc and pop_section
*/

hoc_level_pushsec(sec) Section* sec; {
	++skip_secstack_check;
	nrn_pushsec(sec);
}
	
push_section() {
	Section* sec;
	if (hoc_is_str_arg(1)) {
		Item* qsec;
		char* s;
		sec = (Section*)0;
		s = gargstr(1);
		ForAllSections(sec1) /* I can't imagine a more inefficient way */
			if (strcmp(s, secname(sec1)) == 0) {
				sec = sec1;
				break;
			}
		}
		if (!sec) {
			hoc_execerror("push_section: arg not a sectionname:", s);
		}
	}else{
		sec = (Section*)(long)(*getarg(1));
	}
	if (!sec || !sec->prop || !sec->prop->dparam[0].sym || sec->prop->dparam[0].sym->type != SECTION) {
		hoc_execerror("Not a Section pointer", (char*)0);
	}
	hoc_level_pushsec(sec);
	ret(1.0);
}


#if FISHER
assign_hoc_str(str, val, global) 

    /* Assign hoc string in global dataspace if global == 1,
       else use current dataspace. */

    char *str, *val;
    int global;
{
    Symbol *sym;
    char** pstr;
    Objectdata* obdsave = hoc_objectdata; /* save current dataspace */

    if (!global && hoc_thisobject) {
    	sym = hoc_table_lookup(str, hoc_thisobject->template->symtable);
    }else{
	hoc_objectdata = hoc_top_level_data;
	sym = hoc_lookup(str);
    }
    if (sym && sym->type == STRING) {
	/* point to string */
	pstr = OPSTR(sym);
	hoc_assign_str(pstr, val);	
    }
    hoc_objectdata = obdsave;
}
#endif

section_exists() {
	int iarg, indx;
	Symbol* sym;
	Section* sec;
	Object* obj;
	Objectdata* obd;
	Item* itm;
	char *str, *cp, buf[100];

	obj = (Object*)0;
	sec = (Section*)0;
	iarg = 1;
	str = gargstr(iarg++);
	
	indx = 0;
	if (ifarg(iarg) && hoc_is_double_arg(iarg)) {
		indx = (int)chkarg(iarg++, 0., 1e9);
	}else{ /* if [integer] present, then extract the value and the basename */
		if (sscanf(str, "%[^[][%d", buf, &indx) == 2) {
			str = buf;
		}
	}
	if (ifarg(iarg)) {
		obj = *hoc_objgetarg(iarg);
		sym = hoc_table_lookup(str, obj->template->symtable);
		/* if external then back to top level */
		if (sym && sym->public == 2) {
			sym = sym->u.sym;
			obj = (Object*)0;
		}
	}else{
		sym = hoc_table_lookup(str, hoc_top_level_symlist);
	}
	if (sym) {
		if (sym->type == SECTION) {
			if (obj) {
				obd = obj->u.dataspace;
			}else{
				obd = hoc_top_level_data;
			}
			if (indx < hoc_total_array_data(sym, obd)) {
				itm = *(obd[sym->u.oboff].psecitm + indx);
				if (itm) {
					sec = itm->element.sec;
				}
			}
		}
	}
	ret((double)(sec && sec->prop));
}
