#include <../../nrnconf.h>
#include <stdio.h>
#include "classreg.h"

#if HAVE_IV // to end of file except hoc interface stubs exist.

#include <OS/list.h>
#include <OS/string.h>
#include <OS/math.h>
#include <string.h>
#include <ivstream.h>
#include "graph.h"
#include "ivocvect.h"
#include "scenepic.h"
#include "nrnoc2iv.h"
#include "objcmd.h"
#include "utility.h"
#include "ivoc.h"

extern "C" {
extern int nrn_multisplit_active_;
extern int hoc_execerror_messages;
extern int node_index(Section*, double);
extern int structure_change_cnt, nrn_shape_changed_;
};

class SecPos {
public:
	float x;
	float len;
	Section* sec;
};

declareList(SecPosList, SecPos);
implementList(SecPosList, SecPos);

class RangeExpr {
public:
	RangeExpr(const char* expr, SecPosList*);
	virtual ~RangeExpr();
	void fill();
	void compute();
	bool exists(int);
	double* pval(int);
private:
	long n_;
	SecPosList* spl_;
	double* val_;
	bool* exist_;
	HocCommand* cmd_;
};

class RangeVarPlot : public GraphVector {
public:
	RangeVarPlot(const char*);
	virtual ~RangeVarPlot();
	virtual void save(ostream&);
	virtual void request(Requisition& req) const;
	virtual bool choose_sym(Graph*);
	virtual void update(Observable*);
	void x_begin(float);
	void x_end(float);
	void origin(float);
	double d2root();
	float left();
	float right();
	void list(Object*);
private:
	void set_x();
	void fill_pointers();
private:
	RangeExpr *rexp_;
	Section* begin_section_, *end_section_;
	float x_begin_, x_end_, origin_;
	SecPosList* sec_list_;
	CopyString expr_;
	int shape_changed_;
	int struc_changed_;
	double d2root_; // distance to root of closest point to root
};
#endif //HAVE_IV

static double s_begin(void* v) {
#if HAVE_IV
IFGUI
	((RangeVarPlot*)v)->x_begin(chkarg(1, 0., 1.));
	return 1.;
ENDGUI
#endif
}

static double s_end(void* v) {
#if HAVE_IV
IFGUI
	((RangeVarPlot*)v)->x_end(chkarg(1, 0., 1.));
ENDGUI
#endif
	return 1.;
}

static double s_origin(void* v) {
#if HAVE_IV
IFGUI
	((RangeVarPlot*)v)->origin(*getarg(1));
ENDGUI
#endif
	return 1.;
}

static double s_d2root(void* v) {
#if HAVE_IV
IFGUI
	return ((RangeVarPlot*)v)->d2root();
ENDGUI
#endif
	return 0.0;
}

static double s_left(void* v) {
#if HAVE_IV
IFGUI
	return ((RangeVarPlot*)v)->left();
ENDGUI
#endif
	return 0.0;
}

static double s_right(void* v) {
#if HAVE_IV
IFGUI
	return ((RangeVarPlot*)v)->right();
ENDGUI
#endif
	return 0.0;
}

static double s_list(void* v) {
#if HAVE_IV
IFGUI
	Object* ob = *hoc_objgetarg(1);
	check_obj_type(ob, "SectionList");
	((RangeVarPlot*)v)->list(ob);
ENDGUI
#endif
	return 0.;
}

static double s_color(void* v) {
#if HAVE_IV
IFGUI
	((RangeVarPlot*)v)->color(colors->color((int)chkarg(1,0,100)));
ENDGUI
#endif
	return 0.;
}

static double to_vector(void* v) {
#if HAVE_IV
IFGUI
	RangeVarPlot* rvp = (RangeVarPlot*)v;
	Vect* y = vector_arg(1);
	long i, cnt = rvp->py_data()->count();
	y->resize(cnt);
	for (i=0; i < cnt; ++i) {
		y->elem(i) = *rvp->py_data()->p(i);
	}
	if (ifarg(2)) {
		Vect* x = vector_arg(2);
		x->resize(cnt);
		for (i=0; i < cnt; ++i) {
			x->elem(i) = rvp->x_data()->get_val(i);
		}
	}
	return double(cnt);
ENDGUI
#endif
	return 0.0;
}

static double from_vector(void* v) {
#if HAVE_IV
IFGUI
	RangeVarPlot* rvp = (RangeVarPlot*)v;
	Vect* y = vector_arg(1);
	long i, cnt = rvp->py_data()->count();
	for (i=0; i < cnt; ++i) {
		*rvp->py_data()->p(i) = y->elem(i);
	}
	return double(cnt);
ENDGUI
#endif
	return 0.0;
}

static Member_func s_members[] = {
	"begin", s_begin,
	"end", s_end,
	"origin", s_origin,
	"d2root", s_d2root,
	"left", s_left,
	"right", s_right,
	"list", s_list,
	"color", s_color,
	"to_vector", to_vector,
	"from_vector", from_vector,
	0, 0
};

static void* s_cons(Object*) {
#if HAVE_IV
IFGUI
	RangeVarPlot* s = new RangeVarPlot(gargstr(1));
	s->ref();
	return (void*)s;
ENDGUI
#endif
	return 0;
}

static void s_destruct(void* v) {
#if HAVE_IV
IFGUI
	Resource::unref((RangeVarPlot*)v);
ENDGUI
#endif
}

void RangeVarPlot_reg() {
//printf("RangeVarPlot_reg\n");
	class2oc("RangeVarPlot", s_cons, s_destruct, s_members);
}

#if HAVE_IV // to end of file

RangeVarPlot::RangeVarPlot(const char* var) : GraphVector(var) {
	begin_section_ = 0;
	end_section_ = 0;
	sec_list_ = new SecPosList(50);
	struc_changed_ = structure_change_cnt;
	shape_changed_ = nrn_shape_changed_;
	Oc oc;
	oc.notify_attach(this);
	if (strstr(var, "$1")) {
		rexp_ = new RangeExpr(var, sec_list_);
	}else{
		rexp_ = nil;
	}
	expr_ = var;
	origin_ = 0.;
	d2root_ = 0.;
}

RangeVarPlot::~RangeVarPlot() {
	if (begin_section_) {
		section_unref(begin_section_);
		begin_section_ = nil;
	}
	if (end_section_) {
		section_unref(end_section_);
		end_section_ = nil;
	}
	delete sec_list_;
	if (rexp_) {
		delete rexp_;
	}
	Oc oc;
	oc.notify_detach(this);
}

void RangeVarPlot::update(Observable* o) {
	if (o) { // must be Oc::notify_change_ because free is nil
		// but do not update if multisplit active
		if (shape_changed_ != nrn_shape_changed_ && !nrn_multisplit_active_) {
//printf("RangeVarPlot::update shape_changed %d %d\n", shape_changed_, nrn_shape_changed_);
			shape_changed_ = nrn_shape_changed_;
			set_x();
			fill_pointers();
		}
	}else{
//printf("RangeVarPlot::update -> GraphVector::update\n");
		GraphVector::update(o);
	}
}

void RangeVarPlot::origin(float x) {
	origin_ = x;
	fill_pointers();
}

double RangeVarPlot::d2root() {
	return d2root_;
}

void RangeVarPlot::x_begin(float x) {
	if (begin_section_) { section_unref(begin_section_); }
	begin_section_ = chk_access();
	section_ref(begin_section_);
	x_begin_ = x;
	set_x();
	fill_pointers();
}

void RangeVarPlot::x_end(float x) {
	if (end_section_) { section_unref(end_section_); }
	end_section_ = chk_access();
	section_ref(end_section_);
	x_end_ = x;
	set_x();
	fill_pointers();
}

float RangeVarPlot::left() {
	if (sec_list_->count()) {
		return sec_list_->item(0).len + origin_;
	}else{
		return origin_;
	}
}

float RangeVarPlot::right() {
	if (sec_list_->count()) {
		return sec_list_->item(sec_list_->count() - 1).len + origin_;
	}else{
		return origin_;
	}
}

void RangeVarPlot::request(Requisition& req) const {
	if (rexp_) {
		rexp_->compute();
	}
	GraphVector::request(req);
}

void RangeVarPlot::save(ostream& o) {
	char buf[256];
	o << "objectvar rvp_" << endl;
	sprintf(buf, "rvp_ = new RangeVarPlot(\"%s\")", expr_.string());
	o << buf << endl;
	sprintf(buf, "%s rvp_.begin(%g)",hoc_section_pathname(begin_section_), x_begin_);
	o << buf << endl;
	sprintf(buf, "%s rvp_.end(%g)",hoc_section_pathname(end_section_), x_end_);
	o << buf << endl;
	sprintf(buf, "rvp_.origin(%g)", origin_);
	o << buf << endl;
	Coord x, y;
	label_loc(x, y);
	sprintf(buf, "save_window_.addobject(rvp_, %d, %d, %g, %g)",
		colors->color(color()), brushes->brush(brush()), x, y);
	o << buf << endl;
}

bool RangeVarPlot::choose_sym(Graph* g) {
//	printf("RangeVarPlot::choose_sym\n");
	char s[256];	
	s[0] = '\0';
        while (str_chooser("Range Variable or expr involving $1", s,
	  XYView::current_pick_view()->canvas()->window())) {
		RangeVarPlot* rvp = new RangeVarPlot(s);
		rvp->ref();		

		rvp->begin_section_ = begin_section_;
		rvp->x_begin_ = x_begin_;
		rvp->end_section_ = end_section_;
		rvp->x_end_ = x_end_;
		rvp->set_x();
		rvp->origin(origin_);
		// check to see if there is anything to plot
		if (!rvp->trivial()) {
			g->add_graphVector(rvp);
			rvp->label(g->label(s));
			rvp->unref();
			break;
		}else{
printf("%s doesn't exist along the path %s(%g)", s,
	secname(begin_section_), x_begin_);
printf(" to %s(%g)\n", secname(end_section_), x_end_);
		}
		rvp->unref();
	}
	
	return true;
}

#if 0
void SpacePlot::expr(const char* expr) {
	is_var_ = false;
	if (gv_) {
		gv_->begin();
	}
	Graph::add_var(expr);
	set_x();
}
#endif

void RangeVarPlot::fill_pointers() {
	long xcnt = sec_list_->count();
	if (xcnt) {
		Symbol* sym;
		char buf[200];
		begin();
		if (rexp_) {
			rexp_->fill();
		}else{
			sscanf(expr_.string(), "%[^[]", buf);
			sym = hoc_lookup(buf);
			if (!sym) {
				return;
			}
			sprintf(buf, "%s(hoc_ac_)", expr_.string());
		}
		int noexist=0;// don't plot single points that don't exist
		bool does_exist;
		double* pval = nil;
		for (long i=0; i < xcnt; ++i) {
			Section* sec = sec_list_->item(i).sec;
			hoc_ac_ = sec_list_->item(i).x;
			if (rexp_) {
				does_exist = rexp_->exists(int(i));
			}else{
				nrn_pushsec(sec);
does_exist = nrn_exists(sym, node_exact(sec, hoc_ac_));
//does_exist = nrn_exists(sym, sec->pnode[node_index(sec, hoc_ac_)]);
			}
			if (does_exist) {
				if (rexp_) {
					pval = rexp_->pval(int(i));
				}else{
					pval = hoc_val_pointer(buf);
				}
				if (noexist > 1) {
	add (sec_list_->item(i-1).len + origin_, 0);
	add (sec_list_->item(i-1).len + origin_, pval);
				}
				if (i == 1 && noexist == 1) {
	add(sec_list_->item(i-1).len + origin_, pval);
				}
				add(sec_list_->item(i).len + origin_, pval);
				noexist = 0;
			}else{
				if (noexist == 1) {
	add (sec_list_->item(i-1).len + origin_, pval);
	add (sec_list_->item(i-1).len + origin_, 0);
				}
				if (i == xcnt - 1 && noexist == 0) {
	add (sec_list_->item(i).len + origin_, pval);
				}					
				++noexist;
			}
			nrn_popsec();
		}
	}
}

void RangeVarPlot::list(Object* ob) {
	hoc_List* l = (hoc_List*)ob->u.this_pointer;
	long i, cnt = sec_list_->count();
	Section* sec = nil;
	for (i = 0; i < cnt; ++i) {
		if (sec_list_->item(i).sec != sec) {
			sec = sec_list_->item(i).sec;
			if (sec) {
				hoc_l_lappendsec(l, sec);
				section_ref(sec);
			}
		}
	}
}

#if 0
void SpacePlot::plot() {
	Graph* g = (Graph*)this;
	long xcnt = sec_list_->count();
	double* x = hoc_val_pointer("x");
	if (xcnt) {
		g->begin();
		for (long i=0; i < xcnt; ++i) {
			nrn_pushsec(sec_list_->item(i).sec);
			*x = sec_list_->item(i).x;
			gr_->plot(sec_list_->item(i).len);
			nrn_popsec();
		}
	}
	gr_->flush();
}

void SpacePlot::plot() {
	gr_->flush();
}
#endif

void RangeVarPlot::set_x() {
	sec_list_->remove_all();
	if (!begin_section_ || !end_section_ || !begin_section_->prop || !end_section_->prop) {
		return;
	}
	SecPos spos;
	double d, dist, d2r, x;
	Section* sec, *sec1, *sec2, *rootsec;
	Node* nd, *nd1, *nd2, *rootnode;
	sec1 = begin_section_;
	sec2 = end_section_;
	v_setup_vectors();
	nd1 = node_exact(sec1, x_begin_);
	nd2 = node_exact(sec2, x_end_);

	dist = topol_distance(sec1, nd1, sec2, nd2, &rootsec, &rootnode);
//printf("dist=%g\n", dist);
	if (!rootnode) {
		hoc_execerror("SpacePlot", "No path from begin to end points");
	}
	d2r = topol_distance(sec1, nd1, rootsec, rootnode, &rootsec, &rootnode);
#if 0
	gr_->erase_most();
	gr_->new_size(-d2r, -80, dist-d2r, 40);
	gr_->xaxis(0,1,1);
	gr_->yaxis(0,1,1);
	Coord y1=gr_->y1(), y2 = gr_->y2();
	gr_->new_size(-d2r - dist/10, y1 - (y2-y1)/10, dist*1.1 - d2r, y2 + (y2-y1)/10);
#endif

	nd = nd1; sec = sec1;
	d = -d2r + node_dist(sec, nd);
	while (nd != rootnode) {
		x = node_dist(sec, nd);
		spos.sec = sec;
		spos.x = nrn_arc_position(sec, nd);
		spos.len = d - x;
//printf("%s(%g) at %g  %g\n", secname(spos.sec), spos.x, spos.len, x);
		sec_list_->append(spos);
		if (x == 0.) {
			sec = nrn_trueparent(sec);
			d += node_dist(sec, nd);
		}
		nd = nrn_parent_node(nd);
	}

	if (sec) {
		spos.sec = sec;
	}else{
		spos.sec = nd->child;
	}
	spos.x = nrn_arc_position(spos.sec, nd);
	spos.len = 0.;
//printf("%s(%g) at %g root\n", secname(spos.sec), spos.x, spos.len);
	sec_list_->append(spos);

	long indx = sec_list_->count();

	nd = nd2; sec = sec2;
	d = dist - d2r - node_dist(sec, nd);
	while (nd != rootnode) {
		x = node_dist(sec, nd);
		spos.sec = sec;
		spos.x = nrn_arc_position(sec, nd);
		spos.len = d + x;
//printf("%s(%g) at %g\n", secname(spos.sec), spos.x, spos.len);
		sec_list_->insert(indx, spos);
		if (x == 0.) {
			sec = nrn_trueparent(sec);
			d -= node_dist(sec, nd);
		}
		nd = nrn_parent_node(nd);
	}
	for (sec = rootsec; sec->parentsec; sec = sec->parentsec){}
	nd = sec->parentnode;
	d2root_ = topol_distance(rootsec, rootnode, sec, nd, &sec, &nd);
//debugging
#if 0
printf("debugging\n");
	long cnt, icnt;
	cnt = sec_list_->count();
	for (icnt=0; icnt<cnt; ++icnt) {
		printf("%s(%g) at %g\n", secname(sec_list_->item(icnt).sec),
			sec_list_->item(icnt).x, sec_list_->item(icnt).len);
	}
#endif
}

RangeExpr::RangeExpr(const char* expr, SecPosList* spl) {
	spl_ = spl;
	n_ = 0;
	val_ = nil;
	exist_ = nil;
	char buf[256];
	const char* p1;
	char* p2;
	sprintf(buf, "hoc_ac_ = ");
	p2 = buf + strlen(buf);
	for (p1 = expr; *p1;) {
		if (p1[0] == '$' && p1[1] == '1') {
			p1 += 2;
			strcpy(p2, "hoc_ac_");
			p2 += 7;
		}else{
			*p2++ = *p1++;
		}
	}
	*p2 = '\0';
	cmd_ = new HocCommand(buf);
}

RangeExpr::~RangeExpr() {
	if (val_) {
		delete [] val_;
		delete [] exist_;
	}
	delete cmd_;
}


void RangeExpr::fill() {
	if (n_ != spl_->count()) {
		if (val_) {
			delete [] val_;
			delete [] exist_;
		}
		n_ = spl_->count();
		if (n_) {
			val_ = new double[n_];
			exist_ = new bool[n_];
		}
	}
	int temp = hoc_execerror_messages;
	for (long i=0; i < n_; ++i) {
		nrn_pushsec(spl_->item(i).sec);
		hoc_ac_ = spl_->item(i).x;
		hoc_execerror_messages = 0;
		if (cmd_->execute(bool(false)) == 0) {
			exist_[i] = true;
			val_[i] = 0.;
		}else{
			exist_[i] = false;
#if 0
			printf("RangeExpr: %s no exist at %s(%g)\n",
				cmd_->name(), secname(spl_->item(i).sec),
				spl_->item(i).x
			);
#endif
		}
		nrn_popsec();
	}
	hoc_execerror_messages = temp;
}

void RangeExpr::compute() {
	for (long i=0; i < n_; ++i) {
		if (exist_[i]) {
			nrn_pushsec(spl_->item(i).sec);
			hoc_ac_ = spl_->item(i).x;
			cmd_->execute(bool(false));
			nrn_popsec();
			val_[i] = hoc_ac_;
		}
	}
}

bool RangeExpr::exists(int i) {
	if (i < n_) {
		return exist_[i];
	}else{
		return false;
	}
}

double* RangeExpr::pval(int i) {
	if (i < n_) {
		return val_ + i;
	}else{
		return 0;
	}
}
#endif
