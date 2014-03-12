#include <stdio.h>
#include "hocdec.h"
extern int nrnmpi_myid;
extern int nrn_nobanner_;
modl_reg(){
  if (!nrn_nobanner_) if (nrnmpi_myid < 1) {
    fprintf(stderr, "Additional mechanisms from files\n");

    fprintf(stderr," cabpump.mod");
    fprintf(stderr," cachan1.mod");
    fprintf(stderr," camchan.mod");
    fprintf(stderr," capump.mod");
    fprintf(stderr," invlfire.mod");
    fprintf(stderr," khhchan.mod");
    fprintf(stderr," mcna.mod");
    fprintf(stderr," nacaex.mod");
    fprintf(stderr," nachan.mod");
    fprintf(stderr," release.mod");
    fprintf(stderr, "\n");
  }
  _cabpump_reg();
  _cachan1_reg();
  _camchan_reg();
  _capump_reg();
  _invlfire_reg();
  _khhchan_reg();
  _mcna_reg();
  _nacaex_reg();
  _nachan_reg();
  _release_reg();
}
