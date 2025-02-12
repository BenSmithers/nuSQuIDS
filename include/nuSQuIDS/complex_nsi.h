#ifndef NSI_COMPLEX
#define NSI_COMPLEX

#include "nuSQuIDS/nuSQuIDS.h"

namespace nusquids{

class nuSQUIDSNSI: public nuSQUIDS {
  private:
    squids::SU_vector NSI;
    std::vector<squids::SU_vector> NSI_evol;
    std::unique_ptr<double[]> hiBuffer;
    double HI_prefactor;
    // nsi parameters
    double eps_mutau_real = 0.0;
    double eps_mutau_im = 0.0;

    double eps_etau_real = 0.0;
    double eps_etau_im = 0.0;

    double eps_emu_real = 0.0;
    double eps_emu_im = 0.0;

    gsl_matrix_complex * nsi_mat;

    void AddToPreDerive(double x){
      for(int ei = 0; ei < ne; ei++){
        // asumming same hamiltonian for neutrinos/antineutrinos
        //SU_vector h0 = H0(E_range[ei],0);
        //NSI_evol[ei] = NSI.Evolve(h0,(x-Get_t_initial()));
        NSI_evol[ei] = NSI.Evolve(H0_array[ei],(x-Get_t_initial()));
      }
    }

    void AddToReadHDF5(hid_t hdf5_loc_id){
      // here we read the new parameters now saved in the HDF5 file
      hid_t nsi = H5Gopen(hdf5_loc_id, "nsi", H5P_DEFAULT);
      H5LTget_attribute_double(hdf5_loc_id,"nsi","mu_tau_real" ,&eps_mutau_real);
      H5LTget_attribute_double(hdf5_loc_id,"nsi","mu_tau_im" ,&eps_mutau_im);
      H5LTget_attribute_double(hdf5_loc_id,"nsi","e_tau_real" ,&eps_etau_real);
      H5LTget_attribute_double(hdf5_loc_id,"nsi","e_tau_im" ,&eps_etau_im);
      H5LTget_attribute_double(hdf5_loc_id,"nsi","e_mu_real" ,&eps_emu_real);
      H5LTget_attribute_double(hdf5_loc_id,"nsi","e_mu_im" ,&eps_emu_im);
      H5Gclose(nsi);
    }

    void AddToWriteHDF5(hid_t hdf5_loc_id) const {
      // here we write the new parameters to be saved in the HDF5 file
      H5Gcreate(hdf5_loc_id, "nsi", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      H5LTset_attribute_double(hdf5_loc_id,"nsi","mu_tau_real" ,&eps_mutau_real, 1);
      H5LTset_attribute_double(hdf5_loc_id,"nsi","mu_tau_im" ,&eps_mutau_im, 1);
      H5LTset_attribute_double(hdf5_loc_id,"nsi","e_tau_real" ,&eps_etau_real, 1);
      H5LTset_attribute_double(hdf5_loc_id,"nsi","e_tau_im" ,&eps_etau_im, 1);
      H5LTset_attribute_double(hdf5_loc_id,"nsi","e_mu_real" ,&eps_emu_real, 1);
      H5LTset_attribute_double(hdf5_loc_id,"nsi","e_mu_im" ,&eps_emu_im, 1);
      
    }

    squids::SU_vector HI(unsigned int ei,unsigned int index_rho) const{
      double CC = HI_prefactor*current_density*current_ye;

      // // construct potential in flavor basis
      squids::SU_vector potential(nsun,hiBuffer.get());

      potential = (3.0*CC)*NSI_evol[ei];

      if ((index_rho == 0 and NT==both) or NT==neutrino){
          // neutrino potential
          return nuSQUIDS::HI(ei,index_rho) + potential;
      } else if ((index_rho == 1 and NT==both) or NT==antineutrino){
          // antineutrino potential
          return nuSQUIDS::HI(ei,index_rho) + (-1.0)*std::move(potential);
      } else{
          throw std::runtime_error("nuSQUIDS::HI : unknown particle or antiparticle");
      }
    }
  public:
    //Constructor

    nuSQUIDSNSI() : nuSQUIDS() {
        nsi_mat = gsl_matrix_complex_calloc(3,3);
    }

    nuSQUIDSNSI(double eps_mutau_real, marray<double,1> Erange,unsigned int numneu, NeutrinoType NT,
                bool iinteraction,double th01=0.563942, double th02=0.154085,double th12=0.785398):
                        nuSQUIDS(Erange,numneu,NT,iinteraction),
                        hiBuffer(new double[nsun*nsun]),eps_mutau_real(eps_mutau_real)
    {
        assert(numneu == 3);
        // defining a complex matrix M which will contain our flavor
        // violating flavor structure.
        nsi_mat = gsl_matrix_complex_calloc(3,3);
        gsl_complex n {{ eps_mutau_real , eps_mutau_im }};
        gsl_complex s {{ eps_etau_real , eps_etau_im }};
        gsl_complex i {{ eps_emu_real , eps_emu_im }};
        gsl_matrix_complex_set(nsi_mat,1,2,n);
        gsl_matrix_complex_set(nsi_mat,2,1,gsl_complex_conjugate(n));

        gsl_matrix_complex_set(nsi_mat,0,2,n);
        gsl_matrix_complex_set(nsi_mat,2,0,gsl_complex_conjugate(s));

        gsl_matrix_complex_set(nsi_mat,0,1,n);
        gsl_matrix_complex_set(nsi_mat,1,0,gsl_complex_conjugate(i));
        
        NSI = squids::SU_vector(nsi_mat);

        //setting the mising angle for the flavro-mass basis, 
        //they are measured by the neutrino oscillation experiments.    
        Set_MixingAngle(0,1,th01);
        Set_MixingAngle(0,2,th02);
        Set_MixingAngle(1,2,th12);
        
        // rotate to mass reprentation
        NSI.RotateToB1(params);
        NSI_evol.resize(ne);
        for(int ei = 0; ei < ne; ei++){
        NSI_evol[ei] = squids::SU_vector(nsun);
        }
        
        HI_prefactor = params.sqrt2*params.GF*params.Na*pow(params.cm,-3);
    }
    
    void Set_mutau(double eps,double epsi){
        eps_mutau_real = eps;
        eps_mutau_im = epsi;
        gsl_complex c {{ eps_mutau_real , eps_mutau_im }};
        gsl_matrix_complex_set(nsi_mat,1,2,c);
        gsl_matrix_complex_set(nsi_mat,2,1,gsl_complex_conjugate(c));
        NSI = squids::SU_vector(nsi_mat);    
        NSI.RotateToB1(params);
    }

    void Set_NSI_param(unsigned int flavor_i, unsigned int flavor_j, double eps_real, double eps_imag);
  
  
};

class nuSQUIDSNSIATM: public nuSQUIDSAtm<nuSQUIDSNSI>{
    public:
        using nuSQUIDSAtm<nuSQUIDSNSI>::nuSQUIDSAtm; 
        
        void Set_mutau(double eps, double eps_imag);

        void Set_NSI_param(unsigned int flavor_i, unsigned int flavor_j, double eps_real, double eps_imag);

};

}// namespace nusquids
#endif