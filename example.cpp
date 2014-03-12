// define nil (how to circumvent that define?)
#define nil NULL
// necessary NEURON includes
#include "nrn-7.3/src/ivoc/oc2iv.h"
#include "nrn-7.3/src/ivoc/ivocmain.cpp"
#include "nrn-7.3/src/ivoc/ocjump.cpp"
// necessary stdlib includes
#include <string>
#include <vector>
#include <sstream>
#include <map>

// extern C functions
extern bool hoc_valid_stmt(const char* stmt, Object* ob);
extern int ivocmain(int, char**, char**);
extern double hoc_ac_;

// oc environment
static char* env[] = {0};

/*!
 * make sure you have loaded a hoc geometry before,
 * then returns the number of available sections!
 */
static double initExtractVm() {
   std::vector<std::string> stmts;
   stmts.push_back("objref current_section");
   stmts.push_back("current_section = new SectionList()");
   stmts.push_back("forall { current_section_index = current_section_index + 1 }");
   stmts.push_back("hoc_ac_ = current_section_index");
   
bool ret_gen;  

   std::cout << "Executing a vector of statements" << std::endl;
   for (std::vector<std::string>::const_iterator it = stmts.begin(); it != stmts.end(); it++) {
      ret_gen = hoc_valid_stmt( (*it).c_str(), 0);
      std::cout << "Executing hoc statement (" << (*it).c_str() << ") succeded? " << (ret_gen ? "Yes." : "No.") << std::endl;
      std::cout << "Accumulator was: " << hoc_ac_ << std::endl;
   }
   std::cout << "... done." << std::endl;
   return hoc_ac_;
}

static std::map<double, std::vector<double> > extractVm(int limit=1) {
   int no_sections = initExtractVm();
   std::cout << "No sections: " << no_sections << std::endl;
   
   std::map<double, std::vector<double> > values;
   for (int k = 0; k < no_sections+1; k++) {
   hoc_valid_stmt("j = 0", 0);
     hoc_valid_stmt("current_section = new SectionList()", 0);
     std::stringstream cmd;
     cmd << "forall{ if (j == " << k << ") { {current_section.append() } } { j = j + 1} }";
   hoc_valid_stmt(cmd.str().c_str(), 0);
   cmd.str("");

 //  bool ret = hoc_valid_stmt("hoc_ac_ = n3d() - 1", 0);
   bool ret = hoc_valid_stmt("forsec current_section { hoc_ac_ = n3d() - 1}", 0);
   int no_points = hoc_ac_;
   std::cout << "no_points: " << no_points;
   if (ret) {
      for (int i=1; i < limit+1; i++) {
         bool ret_outer = hoc_valid_stmt("fadvance()", 0);
         if (ret_outer) {
         for (int j=0; j < no_points; j++) {
            std::stringstream stmt;
            stmt << "hoc_ac_ = v(" << j << ")";
           bool ret_inner = hoc_valid_stmt(stmt.str().c_str(), 0);
            double vm = hoc_ac_;
            stmt.str("");
            values[i].push_back(vm);
            }
         }  
      }
   }
   }
return values;
}

// main
int main(int argc, char** argv) {
   const int init = 0;
   ivocmain(init, argv, env);
   const char* stmt = "x = 5";
   const char* stmt2 = "hoc_ac_ = x";
   const bool ret = hoc_valid_stmt(stmt, 0);
   std::cout << "Executing hoc statement (" << stmt << ") succeded? " << (ret ? "Yes." : "No.") << std::endl;
   const bool ret2 = hoc_valid_stmt(stmt2, 0);
   
   std::vector<std::string> stmts;
   const double tstop = 1.0;
   const double dt = 0.0001;
   const double finitialize = -65.0;
   std::stringstream ss;
   ss << "dt = " << dt;
   stmts.push_back(ss.str());
   ss.str("");
   ss << "tstop = " << tstop;
   stmts.push_back(ss.str());
   ss.str("");
   ss << "finitialize(" << finitialize << ")";
   stmts.push_back(ss.str());
   ss.str("");
   ss << "if (t <= t) { {fadvance()} {a = 10} }";
   stmts.push_back(ss.str());
   ss.str("");
   stmts.push_back("hoc_ac_ = a");
   stmts.push_back("load_file(\"foo.hoc\")");
   stmts.push_back("hoc_ac_ = foobar");
   stmts.push_back("forall for i=0,n3d()-1 print(v(i))");
  // initExtractVm();
     
   bool ret_gen;
   std::cout << "Executing a vector of statements" << std::endl;
   for (std::vector<std::string>::const_iterator it = stmts.begin(); it != stmts.end(); it++) {
      ret_gen = hoc_valid_stmt( (*it).c_str(), 0);
      std::cout << "Executing hoc statement (" << (*it).c_str() << ") succeded? " << (ret_gen ? "Yes." : "No.") << std::endl;
      std::cout << "Accumulator was: " << hoc_ac_ << std::endl;
   }
   std::cout << "... done." << std::endl; 
   /*
       dt = $7 $                                                                       
 45 tstop = $8$                                                                     
 46 finitialize($9)$                                                                
 47     while (t <= tstop) {$                                                       
 48             strdef fname$                                                       
 49             sprint(fname, "timestep%f.csv", t/1000)$                            
 50             outfile = wopen(fname)$                                             
 51             forall for i=0,n3d()-1 fprint("%f %f %f %f\n", x3d(i), y3d(i), z3d(i), v(i)) $
 52             wopen()$                                                            
 53         fadvance()$                                                             
 54    }$                    
*/

   std::cout << "Executing hoc statement (" << stmt2 << ") succeded? " << (ret ? "Yes." : "No.") << std::endl;
   std::cout << "hoc_ac_: " << hoc_ac_ << std::endl;
   std::map<double, std::vector<double> > vals = extractVm();
   for (std::map<double, std::vector<double> >::const_iterator it = vals.begin(); it != vals.end(); it++) {
      std::vector<double> vms = it->second;
      for (std::vector<double>::const_iterator it2 = vms.begin(); it2 != vms.end(); it2++) {
         std::cout << *it2 << std::endl;
      }
   }
   return 1;
}
