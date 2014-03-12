/* Created by Language version: 6.2.0 */
/* VECTORIZED */
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
 
#define _threadargscomma_ _p, _ppvar, _thread, _nt,
#define _threadargs_ _p, _ppvar, _thread, _nt
 	/*SUPPRESS 761*/
	/*SUPPRESS 762*/
	/*SUPPRESS 763*/
	/*SUPPRESS 765*/
	 extern double *getarg();
 /* Thread safe. No static _p or _ppvar. */
 
#define t _nt->_t
#define dt _nt->_dt
#define pcabar _p[0]
#define ica _p[1]
#define oca _p[2]
#define cai _p[3]
#define cao _p[4]
#define Doca _p[5]
#define v _p[6]
#define _g _p[7]
#define _ion_cai	*_ppvar[0]._pval
#define _ion_cao	*_ppvar[1]._pval
#define _ion_ica	*_ppvar[2]._pval
#define _ion_dicadv	*_ppvar[3]._pval
 
#if MAC
#if !defined(v)
#define v _mlhv
#endif
#if !defined(h)
#define h _mlhh
#endif
#endif
 static int hoc_nrnpointerindex =  -1;
 static Datum* _extcall_thread;
 static Prop* _extcall_prop;
 /* external NEURON variables */
 extern double celsius;
 /* declaration of user functions */
 static int _hoc_efun();
 static int _hoc_ghk();
 static int _hoc_oca_tau();
 static int _hoc_oca_ss();
 static int _mechtype;
extern int nrn_get_mechtype();
 extern void _nrn_setdata_reg(int, void(*)(Prop*));
 static void _setdata(Prop* _prop) {
 _extcall_prop = _prop;
 }
 static _hoc_setdata() {
 Prop *_prop, *hoc_getdata_range();
 _prop = hoc_getdata_range(_mechtype);
   _setdata(_prop);
 ret(1.);
}
 /* connect user functions to hoc names */
 static IntFunc hoc_intfunc[] = {
 "setdata_cachan", _hoc_setdata,
 "efun_cachan", _hoc_efun,
 "ghk_cachan", _hoc_ghk,
 "oca_tau_cachan", _hoc_oca_tau,
 "oca_ss_cachan", _hoc_oca_ss,
 0, 0
};
#define _f_oca_tau _f_oca_tau_cachan
#define _f_oca_ss _f_oca_ss_cachan
#define efun efun_cachan
#define ghk ghk_cachan
#define oca_tau oca_tau_cachan
#define oca_ss oca_ss_cachan
 extern double _f_oca_tau();
 extern double _f_oca_ss();
 extern double efun();
 extern double ghk();
 extern double oca_tau();
 extern double oca_ss();
 
static void _check_oca_ss(double*, Datum*, Datum*, _NrnThread*); 
static void _check_oca_tau(double*, Datum*, Datum*, _NrnThread*); 
static void _check_table_thread(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt, int _type) {
   _check_oca_ss(_p, _ppvar, _thread, _nt);
   _check_oca_tau(_p, _ppvar, _thread, _nt);
 }
 /* declare global and static user variables */
#define taufactor taufactor_cachan
 double taufactor = 2;
#define usetable usetable_cachan
 double usetable = 1;
 /* some parameters have upper and lower limits */
 static HocParmLimits _hoc_parm_limits[] = {
 "pcabar_cachan", 0, 1e+09,
 "taufactor_cachan", 1e-06, 1e+06,
 "usetable_cachan", 0, 1,
 0,0,0
};
 static HocParmUnits _hoc_parm_units[] = {
 "taufactor_cachan", "1e-6",
 "pcabar_cachan", "cm/s",
 "ica_cachan", "mA/cm2",
 0,0
};
 static double delta_t = 0.01;
 static double oca0 = 0;
 /* connect global user variables to hoc */
 static DoubScal hoc_scdoub[] = {
 "taufactor_cachan", &taufactor_cachan,
 "usetable_cachan", &usetable_cachan,
 0,0
};
 static DoubVec hoc_vdoub[] = {
 0,0,0
};
 static double _sav_indep;
 static void nrn_alloc(), nrn_init(), nrn_state();
 static void nrn_cur(), nrn_jacob();
 
static int _ode_count(), _ode_map(), _ode_spec(), _ode_matsol();
 
#define _cvode_ieq _ppvar[4]._i
 /* connect range variables in _p that hoc is supposed to know about */
 static char *_mechanism[] = {
 "6.2.0",
"cachan",
 "pcabar_cachan",
 0,
 "ica_cachan",
 0,
 "oca_cachan",
 0,
 0};
 static Symbol* _ca_sym;
 
static void nrn_alloc(_prop)
	Prop *_prop;
{
	Prop *prop_ion, *need_memb();
	double *_p; Datum *_ppvar;
 	_p = nrn_prop_data_alloc(_mechtype, 8, _prop);
 	/*initialize range parameters*/
 	pcabar = 2e-08;
 	_prop->param = _p;
 	_prop->param_size = 8;
 	_ppvar = nrn_prop_datum_alloc(_mechtype, 5, _prop);
 	_prop->dparam = _ppvar;
 	/*connect ionic variables to this model*/
 prop_ion = need_memb(_ca_sym);
 nrn_promote(prop_ion, 1, 0);
 	_ppvar[0]._pval = &prop_ion->param[1]; /* cai */
 	_ppvar[1]._pval = &prop_ion->param[2]; /* cao */
 	_ppvar[2]._pval = &prop_ion->param[3]; /* ica */
 	_ppvar[3]._pval = &prop_ion->param[4]; /* _ion_dicadv */
 
}
 static _initlists();
  /* some states have an absolute tolerance */
 static Symbol** _atollist;
 static HocStateTolerance _hoc_state_tol[] = {
 0,0
};
 static void _update_ion_pointer(Datum*);
 _camchan_reg() {
	int _vectorized = 1;
  _initlists();
 	ion_reg("ca", -10000.);
 	_ca_sym = hoc_lookup("ca_ion");
 	register_mech(_mechanism, nrn_alloc,nrn_cur, nrn_jacob, nrn_state, nrn_init, hoc_nrnpointerindex, 1);
 _mechtype = nrn_get_mechtype(_mechanism[1]);
     _nrn_setdata_reg(_mechtype, _setdata);
     _nrn_thread_reg(_mechtype, 2, _update_ion_pointer);
     _nrn_thread_table_reg(_mechtype, _check_table_thread);
  hoc_register_dparam_size(_mechtype, 5);
 	hoc_register_cvode(_mechtype, _ode_count, _ode_map, _ode_spec, _ode_matsol);
 	hoc_register_tolerance(_mechtype, _hoc_state_tol, &_atollist);
 	hoc_register_var(hoc_scdoub, hoc_vdoub, hoc_intfunc);
 	ivoc_help("help ?1 cachan /home/stephan/hg/nrn/nrn-7.3/share/nrn/demo/release/x86_64/camchan.mod\n");
 hoc_register_limits(_mechtype, _hoc_parm_limits);
 hoc_register_units(_mechtype, _hoc_parm_units);
 }
 static double FARADAY = 96485.3;
 static double R = 8.31342;
 static double *_t_oca_ss;
 static double *_t_oca_tau;
static int _reset;
static char *modelname = "CaChan";

static int error;
static int _ninits = 0;
static int _match_recurse=1;
static _modl_cleanup(){ _match_recurse=1;}
 
static int _ode_spec1(), _ode_matsol1();
 static double _n_oca_tau();
 static double _n_oca_ss();
 static int _slist1[1], _dlist1[1];
 static int castate();
 
/*CVODE*/
 static int _ode_spec1 (double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) {int _reset = 0; {
   double _linf , _ltau ;
 _linf = oca_ss ( _threadargscomma_ v ) ;
   _ltau = oca_tau ( _threadargscomma_ v ) ;
   Doca = ( _linf - oca ) / _ltau ;
   }
 return _reset;
}
 static int _ode_matsol1 (double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) {
 double _linf , _ltau ;
 _linf = oca_ss ( _threadargscomma_ v ) ;
 _ltau = oca_tau ( _threadargscomma_ v ) ;
 Doca = Doca  / (1. - dt*( ( ( ( - 1.0 ) ) ) / _ltau )) ;
}
 /*END CVODE*/
 static int castate (double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) { {
   double _linf , _ltau ;
 _linf = oca_ss ( _threadargscomma_ v ) ;
   _ltau = oca_tau ( _threadargscomma_ v ) ;
    oca = oca + (1. - exp(dt*(( ( ( - 1.0 ) ) ) / _ltau)))*(- ( ( ( _linf ) ) / _ltau ) / ( ( ( ( - 1.0) ) ) / _ltau ) - oca) ;
   }
  return 0;
}
 
double ghk ( _p, _ppvar, _thread, _nt, _lv , _lci , _lco ) double* _p; Datum* _ppvar; Datum* _thread; _NrnThread* _nt; 
	double _lv , _lci , _lco ;
 {
   double _lghk;
 double _lz , _leci , _leco ;
 _lz = ( 1e-3 ) * 2.0 * FARADAY * _lv / ( R * ( celsius + 273.15 ) ) ;
   _leco = _lco * efun ( _threadargscomma_ _lz ) ;
   _leci = _lci * efun ( _threadargscomma_ - _lz ) ;
   _lghk = ( .001 ) * 2.0 * FARADAY * ( _leci - _leco ) ;
   
return _lghk;
 }
 
static int _hoc_ghk() {
  double _r;
   double* _p; Datum* _ppvar; Datum* _thread; _NrnThread* _nt;
   if (_extcall_prop) {_p = _extcall_prop->param; _ppvar = _extcall_prop->dparam;}else{ _p = (double*)0; _ppvar = (Datum*)0; }
  _thread = _extcall_thread;
  _nt = nrn_threads;
 _r =  ghk ( _p, _ppvar, _thread, _nt, *getarg(1) , *getarg(2) , *getarg(3) ) ;
 ret(_r);
}
 
double efun ( _p, _ppvar, _thread, _nt, _lz ) double* _p; Datum* _ppvar; Datum* _thread; _NrnThread* _nt; 
	double _lz ;
 {
   double _lefun;
 if ( fabs ( _lz ) < 1e-4 ) {
     _lefun = 1.0 - _lz / 2.0 ;
     }
   else {
     _lefun = _lz / ( exp ( _lz ) - 1.0 ) ;
     }
   
return _lefun;
 }
 
static int _hoc_efun() {
  double _r;
   double* _p; Datum* _ppvar; Datum* _thread; _NrnThread* _nt;
   if (_extcall_prop) {_p = _extcall_prop->param; _ppvar = _extcall_prop->dparam;}else{ _p = (double*)0; _ppvar = (Datum*)0; }
  _thread = _extcall_thread;
  _nt = nrn_threads;
 _r =  efun ( _p, _ppvar, _thread, _nt, *getarg(1) ) ;
 ret(_r);
}
 static double _mfac_oca_ss, _tmin_oca_ss;
  static void _check_oca_ss(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) {
  static int _maktable=1; int _i, _j, _ix = 0;
  double _xi, _tmax;
  if (!usetable) {return;}
  if (_maktable) { double _x, _dx; _maktable=0;
   _tmin_oca_ss =  - 150.0 ;
   _tmax =  150.0 ;
   _dx = (_tmax - _tmin_oca_ss)/200.; _mfac_oca_ss = 1./_dx;
   for (_i=0, _x=_tmin_oca_ss; _i < 201; _x += _dx, _i++) {
    _t_oca_ss[_i] = _f_oca_ss(_p, _ppvar, _thread, _nt, _x);
   }
  }
 }

 double oca_ss(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt, double _lv) { 
#if 0
_check_oca_ss(_p, _ppvar, _thread, _nt);
#endif
 return _n_oca_ss(_p, _ppvar, _thread, _nt, _lv);
 }

 static double _n_oca_ss(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt, double _lv){ int _i, _j;
 double _xi, _theta;
 if (!usetable) {
 return _f_oca_ss(_p, _ppvar, _thread, _nt, _lv); 
}
 _xi = _mfac_oca_ss * (_lv - _tmin_oca_ss);
 _i = (int) _xi;
 if (_xi <= 0.) {
 return _t_oca_ss[0];
 }
 if (_i >= 200) {
 return _t_oca_ss[200];
 }
 return _t_oca_ss[_i] + (_xi - (double)_i)*(_t_oca_ss[_i+1] - _t_oca_ss[_i]);
 }

 
double _f_oca_ss ( _p, _ppvar, _thread, _nt, _lv ) double* _p; Datum* _ppvar; Datum* _thread; _NrnThread* _nt; 
	double _lv ;
 {
   double _loca_ss;
 double _la , _lb ;
 _lv = _lv + 65.0 ;
   _la = 1.0 * efun ( _threadargscomma_ .1 * ( 25.0 - _lv ) ) ;
   _lb = 4.0 * exp ( - _lv / 18.0 ) ;
   _loca_ss = _la / ( _la + _lb ) ;
   
return _loca_ss;
 }
 
static int _hoc_oca_ss() {
  double _r;
   double* _p; Datum* _ppvar; Datum* _thread; _NrnThread* _nt;
   if (_extcall_prop) {_p = _extcall_prop->param; _ppvar = _extcall_prop->dparam;}else{ _p = (double*)0; _ppvar = (Datum*)0; }
  _thread = _extcall_thread;
  _nt = nrn_threads;
 
#if 1
 _check_oca_ss(_p, _ppvar, _thread, _nt);
#endif
 _r =  oca_ss ( _p, _ppvar, _thread, _nt, *getarg(1) ) ;
 ret(_r);
}
 static double _mfac_oca_tau, _tmin_oca_tau;
  static void _check_oca_tau(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) {
  static int _maktable=1; int _i, _j, _ix = 0;
  double _xi, _tmax;
  static double _sav_celsius;
  static double _sav_taufactor;
  if (!usetable) {return;}
  if (_sav_celsius != celsius) { _maktable = 1;}
  if (_sav_taufactor != taufactor) { _maktable = 1;}
  if (_maktable) { double _x, _dx; _maktable=0;
   _tmin_oca_tau =  - 150.0 ;
   _tmax =  150.0 ;
   _dx = (_tmax - _tmin_oca_tau)/200.; _mfac_oca_tau = 1./_dx;
   for (_i=0, _x=_tmin_oca_tau; _i < 201; _x += _dx, _i++) {
    _t_oca_tau[_i] = _f_oca_tau(_p, _ppvar, _thread, _nt, _x);
   }
   _sav_celsius = celsius;
   _sav_taufactor = taufactor;
  }
 }

 double oca_tau(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt, double _lv) { 
#if 0
_check_oca_tau(_p, _ppvar, _thread, _nt);
#endif
 return _n_oca_tau(_p, _ppvar, _thread, _nt, _lv);
 }

 static double _n_oca_tau(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt, double _lv){ int _i, _j;
 double _xi, _theta;
 if (!usetable) {
 return _f_oca_tau(_p, _ppvar, _thread, _nt, _lv); 
}
 _xi = _mfac_oca_tau * (_lv - _tmin_oca_tau);
 _i = (int) _xi;
 if (_xi <= 0.) {
 return _t_oca_tau[0];
 }
 if (_i >= 200) {
 return _t_oca_tau[200];
 }
 return _t_oca_tau[_i] + (_xi - (double)_i)*(_t_oca_tau[_i+1] - _t_oca_tau[_i]);
 }

 
double _f_oca_tau ( _p, _ppvar, _thread, _nt, _lv ) double* _p; Datum* _ppvar; Datum* _thread; _NrnThread* _nt; 
	double _lv ;
 {
   double _loca_tau;
 double _la , _lb , _lq ;
 _lq = pow( 3.0 , ( ( celsius - 6.3 ) / 10.0 ) ) ;
   _lv = _lv + 65.0 ;
   _la = 1.0 * efun ( _threadargscomma_ .1 * ( 25.0 - _lv ) ) ;
   _lb = 4.0 * exp ( - _lv / 18.0 ) ;
   _loca_tau = taufactor / ( _la + _lb ) ;
   
return _loca_tau;
 }
 
static int _hoc_oca_tau() {
  double _r;
   double* _p; Datum* _ppvar; Datum* _thread; _NrnThread* _nt;
   if (_extcall_prop) {_p = _extcall_prop->param; _ppvar = _extcall_prop->dparam;}else{ _p = (double*)0; _ppvar = (Datum*)0; }
  _thread = _extcall_thread;
  _nt = nrn_threads;
 
#if 1
 _check_oca_tau(_p, _ppvar, _thread, _nt);
#endif
 _r =  oca_tau ( _p, _ppvar, _thread, _nt, *getarg(1) ) ;
 ret(_r);
}
 
static int _ode_count(_type) int _type;{ return 1;}
 
static int _ode_spec(_NrnThread* _nt, _Memb_list* _ml, int _type) {
   double* _p; Datum* _ppvar; Datum* _thread;
   Node* _nd; double _v; int _iml, _cntml;
  _cntml = _ml->_nodecount;
  _thread = _ml->_thread;
  for (_iml = 0; _iml < _cntml; ++_iml) {
    _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
    _nd = _ml->_nodelist[_iml];
    v = NODEV(_nd);
  cai = _ion_cai;
  cao = _ion_cao;
     _ode_spec1 (_p, _ppvar, _thread, _nt);
  }}
 
static int _ode_map(_ieq, _pv, _pvdot, _pp, _ppd, _atol, _type) int _ieq, _type; double** _pv, **_pvdot, *_pp, *_atol; Datum* _ppd; { 
	double* _p; Datum* _ppvar;
 	int _i; _p = _pp; _ppvar = _ppd;
	_cvode_ieq = _ieq;
	for (_i=0; _i < 1; ++_i) {
		_pv[_i] = _pp + _slist1[_i];  _pvdot[_i] = _pp + _dlist1[_i];
		_cvode_abstol(_atollist, _atol, _i);
	}
 }
 
static int _ode_matsol(_NrnThread* _nt, _Memb_list* _ml, int _type) {
   double* _p; Datum* _ppvar; Datum* _thread;
   Node* _nd; double _v; int _iml, _cntml;
  _cntml = _ml->_nodecount;
  _thread = _ml->_thread;
  for (_iml = 0; _iml < _cntml; ++_iml) {
    _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];
    _nd = _ml->_nodelist[_iml];
    v = NODEV(_nd);
  cai = _ion_cai;
  cao = _ion_cao;
 _ode_matsol1 (_p, _ppvar, _thread, _nt);
 }}
 extern void nrn_update_ion_pointer(Symbol*, Datum*, int, int);
 static void _update_ion_pointer(Datum* _ppvar) {
   nrn_update_ion_pointer(_ca_sym, _ppvar, 0, 1);
   nrn_update_ion_pointer(_ca_sym, _ppvar, 1, 2);
   nrn_update_ion_pointer(_ca_sym, _ppvar, 2, 3);
   nrn_update_ion_pointer(_ca_sym, _ppvar, 3, 4);
 }

static void initmodel(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt) {
  int _i; double _save;{
  oca = oca0;
 {
   oca = oca_ss ( _threadargscomma_ v ) ;
   }
 
}
}

static void nrn_init(_NrnThread* _nt, _Memb_list* _ml, int _type){
double* _p; Datum* _ppvar; Datum* _thread;
Node *_nd; double _v; int* _ni; int _iml, _cntml;
#if CACHEVEC
    _ni = _ml->_nodeindices;
#endif
_cntml = _ml->_nodecount;
_thread = _ml->_thread;
for (_iml = 0; _iml < _cntml; ++_iml) {
 _p = _ml->_data[_iml]; _ppvar = _ml->_pdata[_iml];

#if 0
 _check_oca_ss(_p, _ppvar, _thread, _nt);
 _check_oca_tau(_p, _ppvar, _thread, _nt);
#endif
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
  cai = _ion_cai;
  cao = _ion_cao;
 initmodel(_p, _ppvar, _thread, _nt);
 }}

static double _nrn_current(double* _p, Datum* _ppvar, Datum* _thread, _NrnThread* _nt, double _v){double _current=0.;v=_v;{ {
   ica = pcabar * oca * oca * ghk ( _threadargscomma_ v , cai , cao ) ;
   }
 _current += ica;

} return _current;
}

static void nrn_cur(_NrnThread* _nt, _Memb_list* _ml, int _type) {
double* _p; Datum* _ppvar; Datum* _thread;
Node *_nd; int* _ni; double _rhs, _v; int _iml, _cntml;
#if CACHEVEC
    _ni = _ml->_nodeindices;
#endif
_cntml = _ml->_nodecount;
_thread = _ml->_thread;
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
  cai = _ion_cai;
  cao = _ion_cao;
 _g = _nrn_current(_p, _ppvar, _thread, _nt, _v + .001);
 	{ double _dica;
  _dica = ica;
 _rhs = _nrn_current(_p, _ppvar, _thread, _nt, _v);
  _ion_dicadv += (_dica - ica)/.001 ;
 	}
 _g = (_g - _rhs)/.001;
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

static void nrn_jacob(_NrnThread* _nt, _Memb_list* _ml, int _type) {
double* _p; Datum* _ppvar; Datum* _thread;
Node *_nd; int* _ni; int _iml, _cntml;
#if CACHEVEC
    _ni = _ml->_nodeindices;
#endif
_cntml = _ml->_nodecount;
_thread = _ml->_thread;
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

static void nrn_state(_NrnThread* _nt, _Memb_list* _ml, int _type) {
 double _break, _save;
double* _p; Datum* _ppvar; Datum* _thread;
Node *_nd; double _v; int* _ni; int _iml, _cntml;
#if CACHEVEC
    _ni = _ml->_nodeindices;
#endif
_cntml = _ml->_nodecount;
_thread = _ml->_thread;
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
  cai = _ion_cai;
  cao = _ion_cao;
 { {
 for (; t < _break; t += dt) {
   castate(_p, _ppvar, _thread, _nt);
  
}}
 t = _save;
 } }}

}

static terminal(){}

static _initlists(){
 double _x; double* _p = &_x;
 int _i; static int _first = 1;
  if (!_first) return;
 _slist1[0] = &(oca) - _p;  _dlist1[0] = &(Doca) - _p;
   _t_oca_ss = makevector(201*sizeof(double));
   _t_oca_tau = makevector(201*sizeof(double));
_first = 0;
}
