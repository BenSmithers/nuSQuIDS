#include "nuSQuIDS/nuSQuIDS.h"
#include "nuSQuIDS/complex_nsi.h"

namespace nusquids{

void nuSQUIDSNSIATM::Set_mutau(double eps, double eps_imag){
    for (nuSQUIDSNSI& nsq: this->GetnuSQuIDS()){
        nsq.Set_mutau(eps, eps_imag);
    }
}


}