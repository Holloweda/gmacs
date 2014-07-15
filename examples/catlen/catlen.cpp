  #include <admodel.h>
  #define echo(object) echoinput << #object << "\n" << object << endl;
  #define echotxt(object,text) echoinput << object << "\t" << text << endl;
  ofstream echoinput("echoinput.ad");
#include <admodel.h>
#include <contrib.h>

  extern "C"  {
    void ad_boundf(int i);
  }
#include <catlen.htp>

model_data::model_data(int argc,char * argv[]) : ad_comm(argc,argv)
{
  nyrs.allocate("nyrs");
  ndclass.allocate("ndclass");
  nsex.allocate("nsex");
  nshell.allocate("nshell");
  nstage.allocate("nstage");
  nclass.allocate("nclass");
  breaks.allocate(1,nclass+1,"breaks");
 int nmats = nsex + nshell + nstage;
 int mshell = nsex + 1;
 int mstage = nsex + nshell;
 echo(nyrs);
 echo(nclass);
 echo(nsex);
 echo(nshell);
 echo(nstage);
 echo(mshell);
 echo(mstage);
  N.allocate(1,nyrs,1,ncol);
 N.initialize();
  for(int yr=1; yr<=nyrs; yr++)
  {
    for(int i=1; i<=nsex; i++)
    {
      for(int j=psex(i); j<=psex(i)+(ncol/nsex)-1; j++)
      {
        N(yr,j) = i;  
      }
    }
  }
  NX.allocate(1,nyrs,1,ncol);
 NX.initialize();
  for(int yr=1; yr<=nyrs; yr++)
  {
    for(int i=1; i<=npstage; i++)
    {
      for(int j=pstage(i); j<=pstage(i)+(ncol/npstage)-1; j++)
      {
        NX(yr,j) = i;  
      }
    }
  }
 echo(N);
 echo(NX);
  obs_catch_at_size.allocate(1,nyrs,1,ncol,"obs_catch_at_size");
  effort.allocate(1,nyrs,"effort");
  M.allocate("M");
  relwt.allocate(2,nclass);
 echo(obs_catch_at_size);
 echo(effort);
 exit(1);
}

void model_parameters::initializationfunction(void)
{
  log_q.set_initial_value(-1);
  log_popscale.set_initial_value(5);
}

model_parameters::model_parameters(int sz,int argc,char * argv[]) : 
 model_data(argc,argv) , function_minimizer(sz)
{
  initializationfunction();
  log_q.allocate(1,"log_q");
  log_popscale.allocate(1,"log_popscale");
  log_sel_coff.allocate(1,nclass-1,-15.,15.,2,"log_sel_coff");
  log_relpop.allocate(1,nyrs+nclass-1,-15.,15.,2,"log_relpop");
  effort_devs.allocate(1,nyrs,-5.,5.,3,"effort_devs");
  log_sel.allocate(1,nclass,"log_sel");
  #ifndef NO_AD_INITIALIZE
    log_sel.initialize();
  #endif
  log_initpop.allocate(1,nyrs+nclass-1,"log_initpop");
  #ifndef NO_AD_INITIALIZE
    log_initpop.initialize();
  #endif
  F.allocate(1,nyrs,1,nclass,"F");
  #ifndef NO_AD_INITIALIZE
    F.initialize();
  #endif
  Z.allocate(1,nyrs,1,nclass,"Z");
  #ifndef NO_AD_INITIALIZE
    Z.initialize();
  #endif
  S.allocate(1,nyrs,1,nclass,"S");
  #ifndef NO_AD_INITIALIZE
    S.initialize();
  #endif
  N.allocate(1,nyrs,1,nclass,"N");
  #ifndef NO_AD_INITIALIZE
    N.initialize();
  #endif
  C.allocate(1,nyrs,1,nclass,"C");
  #ifndef NO_AD_INITIALIZE
    C.initialize();
  #endif
  f.allocate("f");
  prior_function_value.allocate("prior_function_value");
  likelihood_function_value.allocate("likelihood_function_value");
  recsum.allocate("recsum");
  #ifndef NO_AD_INITIALIZE
  recsum.initialize();
  #endif
  initsum.allocate("initsum");
  #ifndef NO_AD_INITIALIZE
  initsum.initialize();
  #endif
  avg_F.allocate("avg_F");
  predicted_N.allocate(2,nclass,"predicted_N");
  ratio_N.allocate(2,nclass,"ratio_N");
  pred_B.allocate("pred_B");
}

void model_parameters::preliminary_calculations(void)
{

#if defined(USE_ADPVM)

  admaster_slave_variable_interface(*this);

#endif
  // Invent some relative average weight-at-size numbers
  relwt.fill_seqadd(1.,1.);
  relwt=pow(relwt,.5);
  relwt/=max(relwt);
}

void model_parameters::userfunction(void)
{
  f =0.0;
  // Example of using FUNCTION to structure the procedure section
  get_mortality_and_survivial_rates();
  get_numbers_at_size();
  get_catch_at_size();
  evaluate_the_objective_function();
}

void model_parameters::get_mortality_and_survivial_rates(void)
{
  int i, j;
  // calculate the selectivity from the sel_coffs
  for (j=1;j<nclass;j++)
  {
    log_sel(j)=log_sel_coff(j);
  }
  // the selectivity is the same for the last two size classes
  log_sel(nclass)=log_sel_coff(nclass-1);
  // This is the same as F(i,j)=exp(q)*effert(i)*exp(log_sel(j));
  F=outer_prod(mfexp(log_q)*effort,mfexp(log_sel));
  if (active(effort_devs))
  {
    for (i=1;i<=nyrs;i++)
    {
      F(i)=F(i)*exp(effort_devs(i));
    }
  }
  // get the total mortality
  Z=F+M;
  // get the survival rate
  S=mfexp(-1.0*Z);
}

void model_parameters::get_numbers_at_size(void)
{
  int i, j;
  log_initpop=log_relpop+log_popscale;
  for (i=1;i<=nyrs;i++)
  {
    N(i,1)=mfexp(log_initpop(i));
  }
  for (j=2;j<=nclass;j++)
  {
    N(1,j)=mfexp(log_initpop(nyrs+j-1));
  }
  for (i=1;i<nyrs;i++)
  {
    for (j=1;j<nclass;j++)
    {
      N(i+1,j+1)=N(i,j)*S(i,j);
    }
  }
  // calculated predicted numbers at size for next year
  for (j=1;j<nclass;j++)
  {
    predicted_N(j+1)=N(nyrs,j)*S(nyrs,j);
    ratio_N(j+1)=predicted_N(j+1)/N(1,j+1);
  }
  // calculated predicted Biomass for next year for
  // adjusted profile likelihood
  pred_B=(predicted_N * relwt);
}

void model_parameters::get_catch_at_size(void)
{
  C=elem_prod(elem_div(F,Z),elem_prod(1.-S,N));
}

void model_parameters::evaluate_the_objective_function(void)
{
  // penalty functions to ``regularize '' the solution
  f+=.01*norm2(log_relpop);
  avg_F=sum(F)/double(size_count(F));
  if (last_phase())
  {
    // a very small penalty on the average fishing mortality
    f+= .001*square(log(avg_F/.2));
  }
  else
  {
    f+= 1000.*square(log(avg_F/.2));
  }
  f+=0.5*double(size_count(C)+size_count(effort_devs))
    * log( sum(elem_div(square(C-obs_catch_at_size),.01+C))
    + 0.1*norm2(effort_devs));
}

void model_parameters::report()
{
 adstring ad_tmp=initial_params::get_reportfile_name();
  ofstream report((char*)(adprogram_name + ad_tmp));
  if (!report)
  {
    cerr << "error trying to open report file"  << adprogram_name << ".rep";
    return;
  }
  report << "Estimated numbers of fish " << endl;
  report << N << endl;
  report << "Estimated numbers in catch " << endl;
  report << C << endl;
  report << "Observed numbers in catch " << endl;
  report << obs_catch_at_size << endl; 
  report << "Estimated fishing mortality " << endl;
  report << F << endl; 
}

model_data::~model_data()
{}

model_parameters::~model_parameters()
{}

void model_parameters::final_calcs(void){}

void model_parameters::set_runtime(void){}

#ifdef _BORLANDC_
  extern unsigned _stklen=10000U;
#endif


#ifdef __ZTC__
  extern unsigned int _stack=10000U;
#endif

  long int arrmblsize=0;

int main(int argc,char * argv[])
{
    ad_set_new_handler();
  ad_exit=&ad_boundf;
    gradient_structure::set_NO_DERIVATIVES();
    gradient_structure::set_YES_SAVE_VARIABLES_VALUES();
    if (!arrmblsize) arrmblsize=15000000;
    model_parameters mp(arrmblsize,argc,argv);
    mp.iprint=10;
    mp.preliminary_calculations();
    mp.computations(argc,argv);
    return 0;
}

extern "C"  {
  void ad_boundf(int i)
  {
    /* so we can stop here */
    exit(i);
  }
}
