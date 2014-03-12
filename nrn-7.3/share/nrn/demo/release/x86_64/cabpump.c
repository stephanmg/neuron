/* Created by Language version: 6.2.0 */
/* NOT VECTORIZED */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "scoplib.h"
#undef PI
 
#include "md1redef.h"
#include "section.h"
#include "md2redef.h"

#if METHOD3
extern int _method3;
#endif

#undef exp
#define exp hoc_Exp
extern double hoc_Exp();
 
#define _threadargscomma_ /**/
#define _threadargs_ /**/
 	/*SUPPRESS 761*/
	/*SUPPRESS 762*/
	/*SUPPRESS 763*/
	/*SUPPRESS 765*/
	 extern double *getarg();
 static double *_p; static Datum *_ppvar;
 
#define t nrn_threads->_t
#define dt nrn_threads->_dt
#define ica_pmp _p[0]
#define ica_pmp_last _p[1]
#define ca (_p + 2)
#define pump _p[12]
#define pumpca _p[13]
#define cao _p[14]
#define cai _p[15]
#define ica _p[16]
#define area1 _p[17]
#define c1 _p[18]
#define c2 _p[19]
#define c3 _p[20]
#define c4 _p[21]
#define Dca (_p + 22)
#define Dpump _p[32]
#define Dpumpca _p[33]
#define _g _p[34]
#define _ion_cao	*_ppvar[0]._pval
#define _ion_ica	*_ppvar[1]._pval
#define _ion_cai	*_ppvar[2]._pval
#define _ion_dicadv	*_ppvar[3]._pval
#define _style_ca	*((int*)_ppvar[4]._pvoid)
#define diam	*_ppvar[5]._pval
 
#if MAC
#if !defined(v)
#define v _mlhv
#endif
#if !defined(h)
#define h _mlhh
#endif
#endif
 static int hoc_nrnpointerindex =  -1;
 /* external NEURON variables */
 extern double celsius;
 /* declaration of user functions */
 static int _hoc_coord();
 static int _hoc_parms();
 static int _hoc_ss();
 static int _mechtype;
extern int nrn_get_mechtype();
 extern void _nrn_setdata_reg(int, void(*)(Prop*));
 static void _setdata(Prop* _prop) {
 _p = _prop->param; _ppvar = _prop->dparam;
 }
 static _hoc_setdata() {
 Prop *_prop, *hoc_getdata_range();
 _prop = hoc_getdata_range(_mechtype);
   _setdata(_prop);
 ret(1.);
}
 /* connect user functions to hoc names */
 static IntFunc hoc_intfunc[] = {
 "setdata_cadifpmp", _hoc_setdata,
 "coord_cadifpmp", _hoc_coord,
 "parms_cadifpmp", _hoc_parms,
 "ss_cadifpmp", _hoc_ss,
 0, 0
};
#define ss ss_cadifpmp
 extern double ss();
 /* declare global and static user variables */
#define DFree DFree_cadifpmp
 double DFree = 0.6;
#define beta beta_cadifpmp
 double beta = 50;
#define k4 k4_cadifpmp
 double k4 = 5;
#define k3 k3_cadifpmp
 double k3 = 500;
#define k2 k2_cadifpmp
 double k2 = 250000;
#define k1 k1_cadifpmp
 double k1 = 5e+08;
#define pump0 pump0_cadifpmp
 double pump0 = 3e-14;
#define vol vol_cadifpmp
 double vol[10];
 /* some parameters have upper and lower limits */
 static HocParmLimits _hoc_parm_limits[] = {
 "DFree_cadifpmp", 0, 1e+09,
 "beta_cadifpmp", 0, 1e+09,
 "k4_cadifpmp", 0, 1e+10,
 "k3_cadifpmp", 0, 1e+10,
 "k2_cadifpmp", 0, 1e+10,
 "k1_cadifpmp", 0, 1e+10,
 "pump0_cadifpmp", 0, 1e+09,
 0,0,0
};
 static HocParmUnits _hoc_parm_units[] = {
 "DFree_cadifpmp", "um2/ms",
 "k1_cadifpmp", "/mM-s",
 "k2_cadifpmp", "/s",
 "k3_cadifpmp", "/s",
 "k4_cadifpmp", "/mM-s",
 "pump0_cadifpmp", "mol/cm2",
 "vol_cadifpmp", "1",
 "ca_cadifpmp", "mM",
 "pump_cadifpmp", "mol/cm2",
 "pumpca_cadifpmp", "mol/cm2",
 "ica_pmp_cadifpmp", "mA/cm2",
 "ica_pmp_last_cadifpmp", "mA/cm2",
 0,0
};
 static double ca0 = 0;
 static double delta_t = 0.01;
 static double pumpca0 = 0;
 static double v = 0;
 /* connect global user variables to hoc */
 static DoubScal hoc_scdoub[] = {
 "DFree_cadifpmp", &DFree_cadifpmp,
 "beta_cadifpmp", &beta_cadifpmp,
 "k1_cadifpmp", &k1_cadifpmp,
 "k2_cadifpmp", &k2_cadifpmp,
 "k3_cadifpmp", &k3_cadifpmp,
 "k4_cadifpmp", &k4_cadifpmp,
 "pump0_cadifpmp", &pump0_cadifpmp,
 0,0
};
 static DoubVec hoc_vdoub[] = {
 "vol_cadifpmp", vol_cadifpmp, 10,
 0,0,0
};
 static double _sav_indep;
 static void nrn_alloc(), nrn_init(), nrn_state();
 static void nrn_cur(), nrn_jacob();
 
static int _ode_count(), _ode_map(), _ode_spec(), _ode_matsol();
 
#define _cvode_ieq _ppvar[6]._i
 static _ode_synonym();
 /* connect range variables in _p that hoc is supposed to know about */
 static char *_mechanism[] = {
 "6.2.0",
"cadifpmp",
 0,
 "ica_pmp_cadifpmp",
 "ica_pmp_last_cadifpmp",
 0,
 "ca_cadifpmp[10]",
 "pump_cadifpmp",
 "pumpca_cadifpmp",
 0,
 0};
 static Symbol* _morphology_sym;
 static Symbol* _ca_sym;
 
static void nrn_alloc(_prop)
	Prop *_prop;
{
	Prop *prop_ion, *need_memb();
	double *_p; Datum *_ppvar;
 	_p = nrn_prop_data_alloc(_mechtype, 35, _prop);
 	/*initialize range parameters*/
 	_prop->param = _p;
 	_prop->param_size = 35;
 	_ppvar = nrn_prop_datum_alloc(_mechtype, 7, _prop);
 	_prop->dparam = _ppvar;
 	/*connect ionic variables to this model*/
 prop_ion = need_memb(_morphology_sym);
 	_ppvar[5]._pval = &prop_ion->param[0]; /* diam */
 prop_ion = need_memb(_ca_sym);
 nrn_check_conc_write(_prop, prop_ion, 1);
 nrn_promote(prop_ion, 3, 0);
 	_ppvar[0]._pval = &prop_ion->param[2]; /* cao */
 	_ppvar[1]._pval = &prop_ion->param[3]; /* ica */
 	_ppvar[2]._pval = &prop_ion->param[1]; /* cai */
 	_ppvar[3]._pval = &prop_ion->param[4]; /* _ion_dicadv */
 	_ppvar[4]._pvoid = (void*)(&(prop_ion->dparam[0]._i)); /* iontype for ca */
 
}
 static _initlists();
  /* some states have an absolute tolerance */
 static Symbol** _atollist;
 static HocStateTolerance _hoc_state_tol[] = {
 "ca_cadifpmp", 1e-06,
 "pump_cadifpmp", 1e-15,
 "pumpca_cadifpmp", 1e-15,
 0,0
};
 static void _update_ion_pointer(Datum*);
 _cabpump_reg() {
	int _vectorized = 0;
  _initlists();
 	ion_reg("ca", -10000.);
 	_morphology_sym = hoc_lookup("morphology");
 	_ca_sym = hoc_lookup("ca_ion");
 	register_mech(_mechanism, nrn_alloc,nrn_cur, nrn_jacob, nrn_state, nrn_init, hoc_nrnpointerindex, 0);
 _mechtype = nrn_get_mechtype(_mechanism[1]);
     _nrn_setdata_reg(_mechtype, _setdata);
     _nrn_thread_reg(_mechtype, 2, _update_ion_pointer);
  hoc_register_dparam_size(_mechtype, 7);
 	nrn_writes_conc(_mechtype, 0);
 	hoc_register_cvode(_mechtype, _ode_count, _ode_map, _ode_spec, _ode_matsol);
 	hoc_register_tolerance(_mechtype, _hoc_state_tol, &_atollist);
 	hoc_register_synonym(_mechtype, _ode_synonym);
 	hoc_register_var(hoc_scdoub, hoc_vdoub, hoc_intfunc);
 	ivoc_help("help ?1 cadifpmp /home/stephan/hg/nrn/nrn-7.3/share/nrn/demo/release/x86_64/cabpump.mod\n");
 hoc_register_limits(_mechtype, _hoc_parm_limits);
 hoc_register_units(_mechtype, _hoc_parm_units);
 }
 static double FARADAY = 96485.3;
 static double PI = 3.14159;
 static double R = 8.31342;
 static double volo = 1;
 static double _zfrat [ 10 ] ;
static int _reset;
static char *modelname = "Calcium ion accumulation and diffusion with pump";

static int error;
static int _ninits = 0;
static int _match_recurse=1;
static _modl_cleanup(){ _match_recurse=1;}
static coord();
static parms();
 extern double *_getelm();
 
#define _MATELM1(_row,_col)	*(_getelm(_row + 1, _col + 1))
 
#define _RHS1(_arg) _coef1[_arg + 1]
 static double *_coef1;
 
#define _linmat1  0
 static void* _sparseobj1;
 static void* _cvsparseobj1;
 
static int _ode_spec1(), _ode_matsol1();
 static int _slist1[12], _dlist1[12]; static double *_temp1;
 static int state();
 
static int  coord (  )  {
   double _lr , _ldr2 ;
 _lr = 1.0 / 2.0 ;
   _ldr2 = _lr / ( 10.0 - 1.0 ) / 2.0 ;
   vol [ 0 ] = 0.0 ;
   _zfrat [ 0 ] = 2.0 * _lr ;
   {int  _li ;for ( _li = 0 ; _li <= 10 - 2 ; _li ++ ) {
     vol [ _li ] = vol [ _li ] + PI * ( _lr - _ldr2 / 2.0 ) * 2.0 * _ldr2 ;
     _lr = _lr - _ldr2 ;
     _zfrat [ _li + 1 ] = 2.0 * PI * _lr / ( 2.0 * _ldr2 ) ;
     _lr = _lr - _ldr2 ;
     vol [ _li + 1 ] = PI * ( _lr + _ldr2 / 2.0 ) * 2.0 * _ldr2 ;
     } }
    return 0; }
 
static int _hoc_coord() {
  double _r;
   _r = 1.;
 coord (  ) ;
 ret(_r);
}
 
static int state ()
 {_reset=0;
 {
   double b_flux, f_flux, _term; int _i;
 {int _i; double _dt1 = 1.0/dt;
for(_i=0;_i<12;_i++){
  	_RHS1(_i) = -_dt1*(_p[_slist1[_i]] - _p[_dlist1[_i]]);
	_MATELM1(_i, _i) = _dt1;
      
}  
_RHS1(10) *= ( ( 1e10 ) * area1) ;
_MATELM1(10, 10) *= ( ( 1e10 ) * area1); 
_RHS1(11) *= ( ( 1e10 ) * area1) ;
_MATELM1(11, 11) *= ( ( 1e10 ) * area1);  
for (_i=0; _i < 10; _i++) {
  	_RHS1(_i + 0) *= ( ( 1.0 + beta ) * diam * diam * vol [ ((int) _i ) ] * 1.0) ;
_MATELM1(_i + 0, _i + 0) *= ( ( 1.0 + beta ) * diam * diam * vol [ ((int) _i ) ] * 1.0);  } }
 /* COMPARTMENT _li , ( 1.0 + beta ) * diam * diam * vol [ ((int) _i ) ] * 1.0 {
     ca }
   */
 /* COMPARTMENT ( 1e10 ) * area1 {
     pump pumpca }
   */
 /* COMPARTMENT volo * ( 1e15 ) {
     }
   */
 /* ~ pumpca <-> pump + cao ( c3 , c4 )*/
 f_flux =  c3 * pumpca ;
 b_flux =  c4 * cao * pump ;
 _RHS1( 10) -= (f_flux - b_flux);
 _RHS1( 11) += (f_flux - b_flux);
 
 _term =  c3 ;
 _MATELM1( 10 ,10)  += _term;
 _MATELM1( 11 ,10)  -= _term;
 _term =  c4 * cao ;
 _MATELM1( 10 ,11)  -= _term;
 _MATELM1( 11 ,11)  += _term;
 /*REACTION*/
  ica_pmp = ( 1e-4 ) * 2.0 * FARADAY * ( f_flux - b_flux ) / area1 ;
   /* ~ ca [ 0 ] < < ( - ( ica - ica_pmp_last ) * PI * diam * 1.0 * ( 1e4 ) * _zfrat [ 0 ] / ( 2.0 * FARADAY ) )*/
 f_flux = b_flux = 0.;
 _RHS1( 0 +  0) += (b_flux =   ( - ( ica - ica_pmp_last ) * PI * diam * 1.0 * ( 1e4 ) * _zfrat [ 0 ] / ( 2.0 * FARADAY ) ) );
 /*FLUX*/
  {int  _li ;for ( _li = 0 ; _li <= 10 - 2 ; _li ++ ) {
     /* ~ ca [ _li ] <-> ca [ _li + 1 ] ( DFree * _zfrat [ _li + 1 ] * 1.0 , DFree * _zfrat [ _li + 1 ] * 1.0 )*/
 f_flux =  DFree * _zfrat [ _li + 1 ] * 1.0 * ca [ _li] ;
 b_flux =  DFree * _zfrat [ _li + 1 ] * 1.0 * ca [ _li + 1] ;
 _RHS1( 0 +  _li) -= (f_flux - b_flux);
 _RHS1( 0 +  _li + 1) += (f_flux - b_flux);
 
 _term =  DFree * _zfrat [ _li + 1 ] * 1.0 ;
 _MATELM1( 0 +  _li ,0 +  _li)  += _term;
 _MATELM1( 0 +  _li + 1 ,0 +  _li)  -= _term;
 _term =  DFree * _zfrat [ _li + 1 ] * 1.0 ;
 _MATELM1( 0 +  _li ,0 +  _li + 1)  -= _term;
 _MATELM1( 0 +  _li + 1 ,0 +  _li + 1)  += _term;
 /*REACTION*/
  } }
   /* ~ ca [ 0 ] + pump <-> pumpca ( c1 , c2 )*/
 f_flux =  c1 * pump * ca [ 0] ;
 b_flux =  c2 * pumpca ;
 _RHS1( 11) -= (f_flux - b_flux);
 _RHS1( 0 +  0) -= (f_flux - b_flux);
 _RHS1( 10) += (f_flux - b_flux);
 
 _term =  c1 * ca [ 0] ;
 _MATELM1( 11 ,11)  += _term;
 _MATELM1( 0 +  0 ,11)  += _term;
 _MATELM1( 10 ,11)  -= _term;
 _term =  c1 * pump ;
 _MATELM1( 11 ,0 +  0)  += _term;
 _MATELM1( 0 +  0 ,0 +  0)  += _term;
 _MATELM1( 10 ,0 +  0)  -= _term;
 _term =  c2 ;
 _MATELM1( 11 ,10)  -= _term;
 _MATELM1( 0 +  0 ,10)  -= _term;
 _MATELM1( 10 ,10)  += _term;
 /*REACTION*/
  cai = ca [ 0 ] ;
     } return _reset;
 }
 
static int  parms (  )  {
   coord ( _threadargs_ ) ;
   area1 = 2.0 * PI * ( diam / 2.0 ) * 1.0 ;
   c1 = ( 1e7 ) * area1 * k1 ;
   c2 = ( 1e7 ) * area1 * k2 ;
   c3 = ( 1e7 ) * area1 * k3 ;
   c4 = ( 1e7 ) * area1 * k4 ;
    return 0; }
 
static int _hoc_parms() {
  double _r;
   _r = 1.;
 parms (  ) ;
 ret(_r);
}
 
double ss (  )  {
   double _lss;
 error = _ss_sparse(&_sparseobj1, 12, _slist1, _dlist1, _p, &t, dt, state,&_coef1, _linmat1);
 if(error){fprintf(stderr,"at line 147 in file cabpump.mod:\n	SOLVE state STEADYSTATE sparse\n"); nrn_complain(_p); abort_run(error);}
 _lss = cai ;
   
return _lss;
 }
 
static int _hoc_ss() {
  double _r;
   _r =  ss (  ) ;
 ret(_r);
}
 
/*CVODE ode begin*/
 static int _ode_spec1() {_reset=0;{
 double b_flux, f_flux, _term; int _i;
 {int _i; for(_i=0;_i<12;_i++) _p[_dlist1[_i]] = 0.0;}
 /* COMPARTMENT _li , ( 1.0 + beta ) * diam * diam * vol [ ((int) _i ) ] * 1.0 {
   ca }
 */
 /* COMPARTMENT ( 1e10 ) * area1 {
   pump pumpca }
 */
 /* COMPARTMENT volo * ( 1e15 ) {
   }
 */
 /* ~ pumpca <-> pump + cao ( c3 , c4 )*/
 f_flux =  c3 * pumpca ;
 b_flux =  c4 * cao * pump ;
 Dpumpca -= (f_flux - b_flux);
 Dpump += (f_flux - b_flux);
 
 /*REACTION*/
  ica_pmp = ( 1e-4 ) * 2.0 * FARADAY * ( f_flux - b_flux ) / area1 ;
 /* ~ ca [ 0 ] < < ( - ( ica - ica_pmp_last ) * PI * diam * 1.0 * ( 1e4 ) * _zfrat [ 0 ] / ( 2.0 * FARADAY ) )*/
 f_flux = b_flux = 0.;
 Dca [ 0] += (b_flux =   ( - ( ica - ica_pmp_last ) * PI * diam * 1.0 * ( 1e4 ) * _zfrat [ 0 ] / ( 2.0 * FARADAY ) ) );
 /*FLUX*/
  {int  _li ;for ( _li = 0 ; _li <= 10 - 2 ; _li ++ ) {
   /* ~ ca [ _li ] <-> ca [ _li + 1 ] ( DFree * _zfrat [ _li + 1 ] * 1.0 , DFree * _zfrat [ _li + 1 ] * 1.0 )*/
 f_flux =  DFree * _zfrat [ _li + 1 ] * 1.0 * ca [ _li] ;
 b_flux =  DFree * _zfrat [ _li + 1 ] * 1.0 * ca [ _li + 1] ;
 Dca [ _li] -= (f_flux - b_flux);
 Dca [ _li + 1] += (f_flux - b_flux);
 
 /*REACTION*/
  } }
 /* ~ ca [ 0 ] + pump <-> pumpca ( c1 , c2 )*/
 f_flux =  c1 * pump * ca [ 0] ;
 b_flux =  c2 * pumpca ;
 Dpump -= (f_flux - b_flux);
 Dca [ 0] -= (f_flux - b_flux);
 Dpumpca += (f_flux - b_flux);
 
 /*REACTION*/
  cai = ca [ 0 ] ;
 for (_i=0; _i < 10; _i++) { _p[_dlist1[_i + 0]] /= ( ( 1.0 + beta ) * diam * diam * vol [ ((int) _i ) ] * 1.0);}
 _p[_dlist1[10]] /= ( ( 1e10 ) * area1);
 _p[_dlist1[11]] /= ( ( 1e10 ) * area1);
   } return _reset;
 }
 
/*CVODE matsol*/
 static int _ode_matsol1() {_reset=0;{
 double b_flux, f_flux, _term; int _i;
   b_flux = f_flux = 0.;
 {int _i; double _dt1 = 1.0/dt;
for(_i=0;_i<12;_i++){
  	_RHS1(_i) = _dt1*(_p[_dlist1[_i]]);
	_MATELM1(_i, _i) = _dt1;
      
}  
_RHS1(10) *= ( ( 1e10 ) * area1) ;
_MATELM1(10, 10) *= ( ( 1e10 ) * area1); 
_RHS1(11) *= ( ( 1e10 ) * area1) ;
_MATELM1(11, 11) *= ( ( 1e10 ) * area1);  
for (_i=0; _i < 10; _i++) {
  	_RHS1(_i + 0) *= ( ( 1.0 + beta ) * diam * diam * vol [ ((int) _i ) ] * 1.0) ;
_MATELM1(_i + 0, _i + 0) *= ( ( 1.0 + beta ) * diam * diam * vol [ ((int) _i ) ] * 1.0);  } }
 /* COMPARTMENT _li , ( 1.0 + beta ) * diam * diam * vol [ ((int) _i ) ] * 1.0 {
 ca }
 */
 /* COMPARTMENT ( 1e10 ) * area1 {
 pump pumpca }
 */
 /* COMPARTMENT volo * ( 1e15 ) {
 }
 */
 /* ~ pumpca <-> pump + cao ( c3 , c4 )*/
 _term =  c3 ;
 _MATELM1( 10 ,10)  += _term;
 _MATELM1( 11 ,10)  -= _term;
 _term =  c4 * cao ;
 _MATELM1( 10 ,11)  -= _term;
 _MATELM1( 11 ,11)  += _term;
 /* ~ ca [ 0 ] < < ( - ( ica - ica_pmp_last ) * PI * diam * 1.0 * ( 1e4 ) * _zfrat [ 0 ] / ( 2.0 * FARADAY ) )*/
 /*FLUX*/
  {int  _li ;for ( _li = 0 ; _li <= 10 - 2 ; _li ++ ) {
 /* ~ ca [ _li ] <-> ca [ _li + 1 ] ( DFree * _zfrat [ _li + 1 ] * 1.0 , DFree * _zfrat [ _li + 1 ] * 1.0 )*/
 _term =  DFree * _zfrat [ _li + 1 ] * 1.0 ;
 _MATELM1( 0 +  _li ,0 +  _li)  += _term;
 _MATELM1( 0 +  _li + 1 ,0 +  _li)  -= _term;
 _term =  DFree * _zfrat [ _li + 1 ] * 1.0 ;
 _MATELM1( 0 +  _li ,0 +  _li + 1)  -= _term;
 _MATELM1( 0 +  _li + 1 ,0 +  _li + 1)  += _term;
 /*REACTION*/
  } }
 /* ~ ca [ 0 ] + pump <-> pumpca ( c1 , c2 )*/
 _term =  c1 * ca [ 0] ;
 _MATELM1( 11 ,11)  += _term;
 _MATELM1( 0 +  0 ,11)  += _term;
 _MATELM1( 10 ,11)  -= _term;
 _term =  c1 * pump ;
 _MATELM1( 11 ,0 +  0)  += _term;
 _MATELM1( 0 +  0 ,0 +  0)  += _term;
 _MATELM1( 10 ,0 +  0)  -= _term;
 _term =  c2 ;
 _MATELM1( 11 ,10)  -= _term;
 _MATELM1( 0 +  0 ,10)  -= _term;
 _MATELM1( 10 ,10)  += _term;
 /*REACTION*/
  cai = ca [ 0 ] ;
   } return _reset;
 }
 
/*CVODE end*/
 
static int _ode_count(_type) int _type;{ return 12;}
 
static int _ode_spec(_NrnThread* _nt, _Memb_list* _ml, int _type) {
   Datum* _thread;
   Node* _nd; double _v; int _iml, _cntml;
  _cntml = _ml->_nodecount;
  _thread = _ml->_thread;
  for (_iml = 0; _iml < _cntml; ++_iml) {
    _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
    _nd = _ml->_nodelist[_iml];
    v = NODEV(_nd);
  cao = _ion_cao;
  ica = _ion_ica;
  cai = _ion_cai;
     _ode_spec1 ();
  _ion_cai = cai;
  }}
 
static int _ode_map(_ieq, _pv, _pvdot, _pp, _ppd, _atol, _type) int _ieq, _type; double** _pv, **_pvdot, *_pp, *_atol; Datum* _ppd; { 
 	int _i; _p = _pp; _ppvar = _ppd;
	_cvode_ieq = _ieq;
	for (_i=0; _i < 12; ++_i) {
		_pv[_i] = _pp + _slist1[_i];  _pvdot[_i] = _pp + _dlist1[_i];
		_cvode_abstol(_atollist, _atol, _i);
	}
 }
 static _ode_synonym(_cnt, _pp, _ppd) int _cnt; double** _pp; Datum** _ppd; { 
 	int _i; 
	for (_i=0; _i < _cnt; ++_i) {_p = _pp[_i]; _ppvar = _ppd[_i];
 _ion_cai =  ca [ 0 ] ;
 }}
 
static int _ode_matsol(_NrnThread* _nt, _Memb_list* _ml, int _type) {
   Datum* _thread;
   Node* _nd; double _v; int _iml, _cntml;
  _cntml = _ml->_nodecount;
  _thread = _ml->_thread;
  for (_iml = 0; _iml < _cntml; ++_iml) {
    _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
    _nd = _ml->_nodelist[_iml];
    v = NODEV(_nd);
  cao = _ion_cao;
  ica = _ion_ica;
  cai = _ion_cai;
 _cvode_sparse(&_cvsparseobj1, 12, _dlist1, _p, _ode_matsol1, &_coef1);
 }}
 extern void nrn_update_ion_pointer(Symbol*, Datum*, int, int);
 static void _update_ion_pointer(Datum* _ppvar) {
   nrn_update_ion_pointer(_ca_sym, _ppvar, 0, 2);
   nrn_update_ion_pointer(_ca_sym, _ppvar, 1, 3);
   nrn_update_ion_pointer(_ca_sym, _ppvar, 2, 1);
   nrn_update_ion_pointer(_ca_sym, _ppvar, 3, 4);
 }

static void initmodel() {
  int _i; double _save;_ninits++;
 _save = t;
 t = 0.0;
{
 for (_i=0; _i<10; _i++) ca[_i] = ca0;
  pumpca = pumpca0;
  pump = pump0;
 {
   double _ltotal ;
 parms ( _threadargs_ ) ;
   {int  _li ;for ( _li = 0 ; _li <= 10 - 1 ; _li ++ ) {
     ca [ _li ] = cai ;
     } }
   pumpca = cai * pump * c1 / c2 ;
   _ltotal = pumpca + pump ;
   if ( _ltotal > 1e-9 ) {
     pump = pump * ( pump / _ltotal ) ;
     pumpca = pumpca * ( pump / _ltotal ) ;
     }
   ica_pmp = 0.0 ;
   ica_pmp_last = 0.0 ;
   }
  _sav_indep = t; t = _save;

}
}

static void nrn_init(_NrnThread* _nt, _Memb_list* _ml, int _type){
Node *_nd; double _v; int* _ni; int _iml, _cntml;
#if CACHEVEC
    _ni = _ml->_nodeindices;
#endif
_cntml = _ml->_nodecount;
for (_iml = 0; _iml < _cntml; ++_iml) {
 _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
#if CACHEVEC
  if (use_cachevec) {
    _v = VEC_V(_ni[_iml]);
  }else
#endif
  {
    _nd = _ml->_nodelist[_iml];
    _v = NODEV(_nd);
  }
 v = _v;
  cao = _ion_cao;
  ica = _ion_ica;
  cai = _ion_cai;
 initmodel();
  _ion_cai = cai;
   nrn_wrote_conc(_ca_sym, (&(_ion_cai)) - 1, _style_ca);
}}

static double _nrn_current(double _v){double _current=0.;v=_v;{ {
   ica_pmp_last = ica_pmp ;
   ica = ica_pmp ;
   }
 _current += ica;

} return _current;
}

static void nrn_cur(_NrnThread* _nt, _Memb_list* _ml, int _type){
Node *_nd; int* _ni; double _rhs, _v; int _iml, _cntml;
#if CACHEVEC
    _ni = _ml->_nodeindices;
#endif
_cntml = _ml->_nodecount;
for (_iml = 0; _iml < _cntml; ++_iml) {
 _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
#if CACHEVEC
  if (use_cachevec) {
    _v = VEC_V(_ni[_iml]);
  }else
#endif
  {
    _nd = _ml->_nodelist[_iml];
    _v = NODEV(_nd);
  }
  cao = _ion_cao;
  ica = _ion_ica;
  cai = _ion_cai;
if (_nt->_vcv) { _ode_spec1(); }
 _g = _nrn_current(_v + .001);
 	{ double _dica;
  _dica = ica;
 _rhs = _nrn_current(_v);
  _ion_dicadv += (_dica - ica)/.001 ;
 	}
 _g = (_g - _rhs)/.001;
  _ion_cai = cai;
  _ion_ica += ica ;
#if CACHEVEC
  if (use_cachevec) {
	VEC_RHS(_ni[_iml]) -= _rhs;
  }else
#endif
  {
	NODERHS(_nd) -= _rhs;
  }
 
}}

static void nrn_jacob(_NrnThread* _nt, _Memb_list* _ml, int _type){
Node *_nd; int* _ni; int _iml, _cntml;
#if CACHEVEC
    _ni = _ml->_nodeindices;
#endif
_cntml = _ml->_nodecount;
for (_iml = 0; _iml < _cntml; ++_iml) {
 _p = _ml->_data[_iml];
#if CACHEVEC
  if (use_cachevec) {
	VEC_D(_ni[_iml]) += _g;
  }else
#endif
  {
     _nd = _ml->_nodelist[_iml];
	NODED(_nd) += _g;
  }
 
}}

static void nrn_state(_NrnThread* _nt, _Memb_list* _ml, int _type){
 double _break, _save;
Node *_nd; double _v; int* _ni; int _iml, _cntml;
#if CACHEVEC
    _ni = _ml->_nodeindices;
#endif
_cntml = _ml->_nodecount;
for (_iml = 0; _iml < _cntml; ++_iml) {
 _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
 _nd = _ml->_nodelist[_iml];
#if CACHEVEC
  if (use_cachevec) {
    _v = VEC_V(_ni[_iml]);
  }else
#endif
  {
    _nd = _ml->_nodelist[_iml];
    _v = NODEV(_nd);
  }
 _break = t + .5*dt; _save = t;
 v=_v;
{
  cao = _ion_cao;
  ica = _ion_ica;
  cai = _ion_cai;
 { {
 for (; t < _break; t += dt) {
 error = sparse(&_sparseobj1, 12, _slist1, _dlist1, _p, &t, dt, state,&_coef1, _linmat1);
 if(error){fprintf(stderr,"at line 86 in file cabpump.mod:\n	SOLVE state METHOD sparse\n"); nrn_complain(_p); abort_run(error);}
 
}}
 t = _save;
 }  _ion_cai = cai;
 }}

}

static terminal(){}

static _initlists() {
 int _i; static int _first = 1;
  if (!_first) return;
 for(_i=0;_i<10;_i++){_slist1[0+_i] = (ca + _i) - _p;  _dlist1[0+_i] = (Dca + _i) - _p;}
 _slist1[10] = &(pumpca) - _p;  _dlist1[10] = &(Dpumpca) - _p;
 _slist1[11] = &(pump) - _p;  _dlist1[11] = &(Dpump) - _p;
_first = 0;
}
