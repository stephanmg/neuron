#include <../../nrnconf.h>
#include "classreg.h"

#if HAVE_IV

#include <InterViews/layout.h>
#include <IV-look/kit.h>
#include <OS/string.h>
#include <stdio.h>
#include "apwindow.h"
#include "secbrows.h"
#include "nrnoc2iv.h"
#include "oclist.h"
#include "ivoc.h"
#include "objcmd.h"
extern "C" {
#include "membfunc.h"
}
#endif

//-----------------------------------------
static double sb_select(void* v) {
#if HAVE_IV
IFGUI
	Section* sec = chk_access();
	((OcSectionBrowser*)v)->select_section(sec);
ENDGUI
#endif
	return 1.;
}
static double sb_select_action(void* v) {
#if HAVE_IV
IFGUI
	((OcSectionBrowser*)v)->set_select_action(gargstr(1));
ENDGUI
#endif
	return 1.;
}
static double sb_accept_action(void* v) {
#if HAVE_IV
IFGUI
	((OcSectionBrowser*)v)->set_accept_action(gargstr(1));
ENDGUI
#endif
	return 1.;
}
static Member_func sb_members[] = {
	"select", sb_select,
	"select_action", sb_select_action,
	"accept_action", sb_accept_action,
	0, 0
};
static void* sb_cons(Object*) {
	Object* ob;
#if HAVE_IV
	OcSectionBrowser* b = nil;
IFGUI
	if (ifarg(1)) {
		ob = *hoc_objgetarg(1);
		b = new OcSectionBrowser(ob);
	}else{
		b = new OcSectionBrowser(nil);
	}
	b->ref();
	Window* w = new StandardWindow(b->standard_glyph());
	w->map();
ENDGUI
	return (void*)b;
#else
	return 0;
#endif
}
static void sb_destruct(void* v) {
#if HAVE_IV
	Resource::unref((OcSectionBrowser*)v);
#endif
}
void SectionBrowser_reg() {
	class2oc("SectionBrowser", sb_cons, sb_destruct, sb_members);
}

#if HAVE_IV

OcSectionBrowser::OcSectionBrowser(Object* ob) : OcBrowser() {
	long i;
	if (ob) {
		SectionList sl(ob);
		Section* sec;
		scnt_ = 0;
		for (sec = sl.begin(); sec; sec = sl.next()) {
			++scnt_;
		}
		if (scnt_) {
			psec_ = new Section*[scnt_];
		}
		scnt_=0;
		for (sec = sl.begin(); sec; sec = sl.next()) {
			psec_[scnt_++] = sec;
		}
	}else{
		struct hoc_Item* qsec;
		scnt_=0;
		ForAllSections(sec) //{
			++scnt_;
		}
		psec_ = new Section*[scnt_];
		scnt_=0;
		ForAllSections(sec) //{
			psec_[scnt_++] = sec;
		}
	}
	for (i=0; i < scnt_; ++i) {
		append_item(secname(psec_[i]));
		section_ref(psec_[i]);
	}
	select_ = nil;
	accept_ = nil;
}

OcSectionBrowser::~OcSectionBrowser() {
	long i;
	for (i=0; i < scnt_; ++i) {
		section_unref(psec_[i]);
	}
	delete [] psec_;
	if (select_) {
		delete select_;
	}
	if (accept_) {
		delete accept_;
	}
}

void OcSectionBrowser::accept(){
	if (accept_) {
		long i = selected();
		if (i < 0) {
			return;
		}
		nrn_pushsec(psec_[i]);
		accept_->execute();
		nrn_popsec();
	}
}
void OcSectionBrowser::select_section(Section* sec){
	long i;
	if (sec->prop) for (i=0; i < scnt_; ++i) {
		if (psec_[i] == sec) {
			OcBrowser::select_and_adjust(i);
			return;
		}
	}
	OcBrowser::select(-1);
}
void OcSectionBrowser::set_select_action(const char* s){
	if (select_) {
		delete select_;
	}
	select_ = new HocCommand(s);
}
void OcSectionBrowser::set_accept_action(const char* s){
	if (accept_) {
		delete accept_;
	}
	accept_ = new HocCommand(s);
}
void OcSectionBrowser::select(GlyphIndex i){
	GlyphIndex old = selected();
	OcBrowser::select(i);
	if (i >= 0 && old != i && select_) {
	    if (psec_[i]->prop) {
		nrn_pushsec(psec_[i]);
		select_->execute();
		nrn_popsec();
	    }else{
		state(i)->set(TelltaleState::is_enabled, false);
		OcBrowser::select(old);
	    }
	}
}
//-----------------------------------------
MechVarType::MechVarType() : MonoGlyph(nil) {
	int i;
	LayoutKit& lk = *LayoutKit::instance();
	WidgetKit& wk = *WidgetKit::instance();
	Button* b[3];
	b[0] = wk.palette_button("Parameters", nil);
	b[1] = wk.palette_button("States", nil);
	b[2] = wk.palette_button("Assigned", nil);
	PolyGlyph* vb = lk.vbox(b[0],b[1],b[2]);
	for (i=0; i < 3; ++i) {
		tts_[i] = b[i]->state();
		Resource::ref(tts_[i]);
	}
	tts_[0]->set(TelltaleState::is_chosen, true);
	body(wk.inset_frame(lk.margin(vb, 5)));
}

MechVarType::~MechVarType() {
	int i;
	for (i=0; i < 3; ++i) {
		Resource::unref(tts_[i]);
	}
}

bool MechVarType::parameter_select() { return select(0);}
bool MechVarType::state_select() { return select(1);}
bool MechVarType::assigned_select() { return select(2);}
bool MechVarType::select(int i) {
	return tts_[i]->test(TelltaleState::is_chosen);
}
	
//---------------------------------------------------------------------

#define MSBEGIN 2
MechSelector::MechSelector() : MonoGlyph(nil){
	int i;
	LayoutKit& lk = *LayoutKit::instance();
	WidgetKit& wk = *WidgetKit::instance();
	ScrollBox* vsb = lk.vscrollbox(5);	
	Button* b;
	tts_ = new TelltaleState*[n_memb_func];
	for (i=MSBEGIN; i < n_memb_func; ++i) {
		b = wk.palette_button(memb_func[i].sym->name, nil);
		b->state()->set(TelltaleState::is_chosen, true);
		vsb->append(b);
		tts_[i] = b->state();
	}
	body(
		lk.hbox(
		   lk.vcenter(
			wk.inset_frame(
			    lk.margin(
			     	lk.natural_span(vsb, 200, 100),
			     	5
			    )
			),
			1.0
		   ),
		   lk.hspace(4),
		   wk.vscroll_bar(vsb)
		)
	);
//	body(wk.inset_frame(lk.margin(vsb, 5)));
}

MechSelector::~MechSelector(){
	delete [] tts_;
}
bool MechSelector::is_selected(int type) {
	if (type >= MSBEGIN && type < n_memb_func &&
	   tts_[type]->test(TelltaleState::is_chosen)) {
		return true;
	}
	return false;
}

int MechSelector::begin(){
	iterator_ = MSBEGIN-1;
	return next();
}
bool MechSelector::done(){
	if (iterator_ >= n_memb_func) {
		return true;
	}else{
		return false;
	}
}
int MechSelector::next(){
	while(!done()) {
		++iterator_;
		if (is_selected(iterator_)) {
			return iterator_;
		}
	}
	return 0;
}

//---------------------------------------------------------------------
class SectionBrowserImpl {
	friend class SectionBrowser;
	SectionBrowserImpl();
	~SectionBrowserImpl();
	MechSelector* ms_;
	MechVarType* mvt_;
	Section** psec_;
	int scnt_;
	void make_panel(Section*);
};
/* static */ class BrowserAccept : public Action {
public:
	BrowserAccept(SectionBrowser*);
	virtual ~BrowserAccept();
	virtual void execute();
private:
	SectionBrowser* sb_;
};
BrowserAccept::BrowserAccept(SectionBrowser* sb){sb_ = sb;}
BrowserAccept::~BrowserAccept(){}
void BrowserAccept::execute(){sb_->accept();}

SectionBrowserImpl::SectionBrowserImpl() {
	struct hoc_Item* qsec;
	scnt_=0;
	ForAllSections(sec) //{
		++scnt_;
	}
	psec_ = new Section*[scnt_];
	scnt_=0;
	ForAllSections(sec) //{
		psec_[scnt_++] = sec;
		section_ref(sec);
	}
	ms_ = new MechSelector();
	ms_->ref();
	mvt_ = new MechVarType();
	mvt_->ref();
}
SectionBrowserImpl::~SectionBrowserImpl() {
	for (int i=0; i < scnt_; ++i) {
		section_unref(psec_[i]);
	}
	delete [] psec_;
	ms_->unref();
	mvt_->unref();
	
}

SectionBrowser::SectionBrowser()
  : OcBrowser(new BrowserAccept(this), nil)
{
	LayoutKit& lk = *LayoutKit::instance();
	WidgetKit& wk = *WidgetKit::instance();
	sbi_ = new SectionBrowserImpl;
	for (int i=0; i < sbi_->scnt_; ++i) {
		append_item(secname(sbi_->psec_[i]));
	}
}

SectionBrowser::SectionBrowser(Object*)
  : OcBrowser(nil, nil)
{
}

SectionBrowser::~SectionBrowser(){
	delete sbi_;
}

void SectionBrowser::accept() {
	printf("accepted %d\n", int(selected()));
	Section* sec = sbi_->psec_[int(selected())];
    if (sec->prop) {
	nrn_pushsec(sec);
	if (sbi_->mvt_->parameter_select()) {
		section_menu(-1, nrnocCONST, sbi_->ms_);
	}
	if (sbi_->mvt_->state_select()) {
		section_menu(.5, STATE, sbi_->ms_);
	}
	if (sbi_->mvt_->assigned_select()) {
		section_menu(.5, 2, sbi_->ms_);
	}
	nrn_popsec();
    }else{
	printf("This section was deleted\n");
    }
}

void SectionBrowser::select(GlyphIndex i) {
	if (sbi_->psec_[int(i)]->prop) {
		FileBrowser::select(i);
	}else{
		FileBrowser::select(-1);
	}
//	printf("selected\n");
}

void SectionBrowser::make_section_browser() {
	LayoutKit& lk = *LayoutKit::instance();
	WidgetKit& wk = *WidgetKit::instance();
	SectionBrowser* sb = new SectionBrowser();
	Window* w = new StandardWindow (
	     lk.hbox(
		sb->standard_glyph(),
		lk.hspace(5),
	     	lk.vbox(
	     	     sb->sbi_->mvt_,
		     lk.vspace(5),
		     sb->sbi_->ms_
	     	)
	     )
	);
	w->map();
}

//---------------------------------------------------
/* static */ class PBrowserAccept : public Action {
public:
	PBrowserAccept(PointProcessBrowser*);
	virtual ~PBrowserAccept();
	virtual void execute();
private:
	PointProcessBrowser* b_;
};
PBrowserAccept::PBrowserAccept(PointProcessBrowser* b){b_ = b;}
PBrowserAccept::~PBrowserAccept(){}
void PBrowserAccept::execute(){b_->accept();}

/* static */ class PPBImpl {
	friend class PointProcessBrowser;
	PPBImpl(OcList*);
	virtual ~PPBImpl();
	OcList* ocl_;
};

PPBImpl::PPBImpl(OcList* ocl) {
	ocl_ = ocl;
	Resource::ref(ocl_);
}
PPBImpl::~PPBImpl() {
	Resource::unref(ocl_);
}

PointProcessBrowser::PointProcessBrowser(OcList* ocl)
  : OcBrowser(new PBrowserAccept(this), nil)
{
	ppbi_ = new PPBImpl(ocl);
	long i, cnt = ocl->count();
	for (i=0; i < cnt; ++i) {
		append_pp(ocl->object(i));
	}
}

PointProcessBrowser::~PointProcessBrowser() {
	delete ppbi_;
}

void PointProcessBrowser::make_point_process_browser(OcList* ocl) {
	LayoutKit& lk = *LayoutKit::instance();
	WidgetKit& wk = *WidgetKit::instance();
	PointProcessBrowser* ppb = new PointProcessBrowser(ocl);	
	SectionBrowser* sb = new SectionBrowser();
	Window* w = new StandardWindow (
		lk.hbox(
		   sb->standard_glyph(),
		   lk.hspace(5),
		   ppb->standard_glyph()
		)
	);
	w->map();
}

void PointProcessBrowser::append_pp(Object* ob) {
	append_item(hoc_object_name(ob));
}

void PointProcessBrowser::remove_pp() {
	GlyphIndex i = selected();
	if (i >= 0) {
		remove_selectable(i);
		ppbi_->ocl_->remove(i);
		refresh();
	}
}

void PointProcessBrowser::add_pp(Object* ob) {
	ppbi_->ocl_->append(ob);
	append_pp(ob);
	select(ppbi_->ocl_->count() - 1);
	refresh();
}

void PointProcessBrowser::select(GlyphIndex i) {
	FileBrowser::select(i);
	Object* ob = ppbi_->ocl_->object(i);
	printf("selected %s\n", hoc_object_name(ob));
}

void PointProcessBrowser::accept() {
	printf("PointProcessBrowser::accept\n");
}
#endif
