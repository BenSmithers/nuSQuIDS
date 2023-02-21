#include "nuSQuIDS/nuSQuIDS.h"
#include "nuSQuIDS/complex_nsi.h"

namespace nusquids{

void nuSQUIDSNSI::Set_NSI_param(unsigned int flavor_i, unsigned int flavor_j, double eps_real, double eps_imag){
    gsl_complex c {{ eps_real, eps_imag}};
    if(flavor_i>=3 || flavor_j>=3 || flavor_i<0 || flavor_j<0){
        std::cout<< "No non-3 flavor models allowed!" << std::endl;
        throw std::runtime_error("Flavors must each satisfy 0<=i<3");
    }
    if(flavor_j>flavor_i){
        throw std::runtime_error("Must have flavor_j > flavor_i");
    }
    gsl_matrix_complex_set(nsi_mat, flavor_i, flavor_j, c);
    gsl_matrix_complex_set(nsi_mat, flavor_j, flavor_i, gsl_complex_conjugate(c));
    NSI = squids::SU_vector(nsi_mat);
    NSI.RotateToB1(params);
}

void nuSQUIDSNSIATM::Set_mutau(double eps, double eps_imag){
    for (nuSQUIDSNSI& nsq: this->GetnuSQuIDS()){
        nsq.Set_mutau(eps, eps_imag);
    }
}


void nuSQUIDSNSIATM::Set_NSI_param(unsigned int flavor_i, unsigned int flavor_j, double eps_real, double eps_imag){
    for (nuSQUIDSNSI& nsq: this->GetnuSQuIDS()){
        nsq.Set_NSI_param(flavor_i, flavor_j, eps_real, eps_imag);
    }
}

}