// Run this macro, from the Linux / Terminal command line, as
// root -l -q ePIC_Analysis.C
// The only condition is that there is a subdirectory called "data" that contains the input MC file
// (provided by Caroline on Box / copied from SDCC)
// The name of this input MC file (variable "strang") is hardcoded in this macro and must match with the file name
// CR 2024-08-14/20

//Defines names and collection of Calorimeters/acceptances
const char* nHCal_name = "nHCal";
const char* Barrel_name = "Barrel";
const char* LFHCAL_name = "LFHCAL";
const char *cals[] = {nHCal_name, Barrel_name, LFHCAL_name};
double cal_limits[sizeof(cals)][2] = {{-4.05, -1.2}, {-1.2, 1.18}, {1.18, 4.2}};

//Array containing the number of phi mesons with 0, 1, or 2 kaons for each calorimeter
float calNums[sizeof(cals)][3];

//Array containing the K1/K2 crossover within different calorimeters and an "All"
float calMatrix[4][4];

//Accepts a string cal_name containing the desired calorimeter name and returns true if an eta particle_eta is within acceptance for the given calorimeter
bool in_Cal_Tolerance(const char* cal_name, float particle_eta) {
    if (cal_name == nHCal_name) {
        return (particle_eta >= -4.05 && particle_eta <= -1.2);
    } else if (cal_name == Barrel_name) {
        return (particle_eta >= -1.2 && particle_eta <= 1.18);
    } else if (cal_name == LFHCAL_name) {
        return (particle_eta >= 1.18 && particle_eta <= 4.2);
    }
    return false;
}

//Accepts two floats, one for each kaon's eta and fills calNums according to if zero, one, or two kaons are within tolerance of each calorimeter.
void kaons_in_Cal(float p1_eta, float p2_eta) {
    for (int i = 0; i < sizeof(cals) / sizeof(cals[0]); i++) {
        int particles_in_cal = 0;
        
        if (in_Cal_Tolerance(cals[i], p1_eta)) {
            particles_in_cal++;
        }
        if (in_Cal_Tolerance(cals[i], p2_eta)) {
            particles_in_cal++;
        }
        
        calNums[i][particles_in_cal]++;
    }
}

//Accepts two floats representing each kaon's eta and fills calMatrix with the intersection of phi decay, showing how many phi mesons had decay in a given combination of calorimeters, or fell into any calorimeter acceptance
void fill_Cal_Arr(float p1_eta, float p2_eta) {
    int p1_Cal_index = -1;
    int p2_Cal_index = -1;
    
    for (int i = 0; i < sizeof(cals); i++) {
        if (in_Cal_Tolerance(cals[i], p1_eta)) {
            p1_Cal_index = i;
        }
        if (in_Cal_Tolerance(cals[i], p2_eta)) {
            p2_Cal_index = i;
        }
    }
    if (p1_Cal_index != -1 && p2_Cal_index != -1) {
        calMatrix[p1_Cal_index+1][3-(p2_Cal_index+1)]++;
        calMatrix[0][3-(p2_Cal_index+1)]++;
        calMatrix[p1_Cal_index+1][3]++;
        calMatrix[0][3]++;
    }
}

//Create a class phiDecay representing each decay and contain attributes of each decay in a single unit for access throughout the program
class phiDecay {
public:
    float eta1, eta2;
    int num_in_nHCal;
    double x_b, q2;
    
    phiDecay(float e1, float e2, double xb_val, double q2_val) {
        eta1 = e1;
        eta2 = e2;
        x_b = xb_val;
        q2 = q2_val;
        
        int particles_in_cal = 0;
        
        if (in_Cal_Tolerance("nHCal", eta1)) {
            particles_in_cal++; }
        if (in_Cal_Tolerance("nHCal", eta2)) {
            particles_in_cal++; }
        
        num_in_nHCal = particles_in_cal;
    }
    
    phiDecay() {
        
    }
};

//Array to contain all decays
phiDecay decays[1361];

void ePIC_Analysis(){

  gSystem->Exec("date");
  
  //
  
  ////////////////////////////////////////////////////
  //// String definitions - modify here as needed ////
  /////////////////////////////////////////////////////
  
  // Define input directory if reading locally:
  const char indir[]="data";
  cout << "Input directory is: " << indir << " \n";
    
  // Define name of local input MC file:
  const char strang[]="sartre_bnonsat_Au_phi_ab_eAu_1.3998.eicrecon.tree.edm4eic"; // "strang = data string"
    //
  
  // define flavor of this macro:
  const char flavor[]="ePIC";
  
  ////////////////////////////////////
  //// end of string definitions ////
  ///////////////////////////////////

  // If reading from a locally copied file:
  TString infile_ram=indir + TString("/") + strang + TString(".root");  // remains in RAM
  const char *infile=infile_ram.Data(); // points to a valid address in RAM
  cout << "Analyzed MC file will be: " << infile << " \n";
  

  TString outfile_ram= TString("out.") + strang + TString("-") + flavor + TString(".root");
  const char *outfile=outfile_ram.Data();
  TFile *ofile = TFile::Open(outfile,"RECREATE"); // RECREATE overwrites an existing file of the same name
  
  TChain *mychain = new TChain("events");

  // if reading a single file:
  mychain->Add(infile);
  
  ///////////////////////////////////////
  //// end of automated definitions ////
  //////////////////////////////////////

  // Initialize reader
  TTreeReader tree_reader(mychain);
    
  //Get Event-Level Information
  //Get x_bjorken And q^2 Information
  TTreeReaderArray<float> partXb(tree_reader, "InclusiveKinematicsTruth.x");
  TTreeReaderArray<float> partQ2(tree_reader, "InclusiveKinematicsTruth.Q2");
    
  // Get Particle Information
  TTreeReaderArray<int> partGenStat(tree_reader, "MCParticles.generatorStatus");
  TTreeReaderArray<float> partMomX(tree_reader, "MCParticles.momentum.x");
  TTreeReaderArray<float> partMomY(tree_reader, "MCParticles.momentum.y");
  TTreeReaderArray<float> partMomZ(tree_reader, "MCParticles.momentum.z");
  TTreeReaderArray<int> partPdg(tree_reader, "MCParticles.PDG");

  // Get Reconstructed Track Information
  TTreeReaderArray<float> trackMomX(tree_reader, "ReconstructedChargedParticles.momentum.x");
  TTreeReaderArray<float> trackMomY(tree_reader, "ReconstructedChargedParticles.momentum.y");
  TTreeReaderArray<float> trackMomZ(tree_reader, "ReconstructedChargedParticles.momentum.z");

  // Get Associations Between MCParticles and ReconstructedChargedParticles
  TTreeReaderArray<unsigned int> recoAssoc(tree_reader, "ReconstructedChargedParticleAssociations.recID");
  TTreeReaderArray<unsigned int> simuAssoc(tree_reader, "ReconstructedChargedParticleAssociations.simID");

  // Get parent and daugther information
  TTreeReaderArray<int> parents_index(tree_reader, "_MCParticles_parents.index");
  TTreeReaderArray<unsigned int> parents_begin(tree_reader, "MCParticles.parents_begin");
  TTreeReaderArray<unsigned int> parents_end(tree_reader, "MCParticles.parents_end");
  
  TTreeReaderArray<int> daughters_index(tree_reader, "_MCParticles_daughters.index");
  TTreeReaderArray<unsigned int> daughters_begin(tree_reader, "MCParticles.daughters_begin");
  TTreeReaderArray<unsigned int> daughters_end(tree_reader, "MCParticles.daughters_end");
  
  // Define Histograms
    
  //Creates Kaon Occurrence Histogram
  TH2D *kaonOccurrence = new TH2D("kaonOccurrence", "Kaon HCal Acceptance (%);K_{1};K_{2}", 4, 0, 4, 4, 0, 4);
    
  //X-bjorken Histogram
  TH1F *xBjorken = new TH1F("xBjorken", "x_{b} value distribution", 100, 0, 0.075);
    
  //q^2 Histogram
  TH1F *q2 = new TH1F("q2", "q^{2} value distribution", 100, 1,10);
    
  //xB_v_q2 Graph
  TGraph *xB_v_q2 = new TGraph();
    
  //xB vs percentage Histograms for 0, 1, 2 kaons in nHCal
  int xB_percent_nBins = 100;
  TH1F *xB_v_percent_0 = new TH1F("xB_v_percent_0", "x_{b} vs percent ocurrence (%);x_{b}", xB_percent_nBins, 0.0001, 0.1);
  TH1F *xB_v_percent_1 = new TH1F("xB_v_percent_1", "x_{b} vs percent ocurrence (%);x_{b}", xB_percent_nBins, 0.0001, 0.1);
  TH1F *xB_v_percent_2 = new TH1F("xB_v_percent_2", "x_{b} vs percent ocurrence (%);x_{b}", xB_percent_nBins, 0.0001, 0.1);
  TH1F *xB_v_percent_all = new TH1F("xB_v_percent_all", "x_{b} vs percent ocurrence (%);x_{b}", xB_percent_nBins, 0.0001, 0.1);
    
  //generatorStatus
  TH1D *generatorStatus = new TH1D("generatorStatus","Status of generated particles, all; generatorStatus",101,0,100);
  
  // eta (pseudorapidity)
  TH1D *partEta = new TH1D("partEta","Eta of thrown particles; #eta",120,-6.,6.);
  TH1D *recEta = new TH1D("recEta","Eta of reconstructed tracks that have matching thrown particle; #eta",120,-6.,6.);

  TH1D *electronEta = new TH1D("electronEta", "Eta of thrown e; #eta",120,-6.,6.);
  TH1D *electronRecEta = new TH1D("electronRecEta","Eta of reco e;#eta",120,-6.,6.);
  
  TH1D *protonEta = new TH1D("protonEta","Eta of thrown p;#eta",120,-6.,6.);
  TH1D *protonRecEta = new TH1D("protonRecEta","Eta of reco p;#eta",120,-6.,6.);
  
  TH1D *muonEta = new TH1D("muonEta","Eta of thrown #mu;#eta",120,-6.,6.);
  TH1D *muonRecEta = new TH1D("muonRecEta","Eta of reco #mu;#eta",120,-6.,6.);
  
  TH1D *pionEta = new TH1D("pionEta","Eta of thrown #pi;#eta",120,-6.,6.);
  TH1D *pionRecEta = new TH1D("pionRecEta","Eta of reco #pi;#eta",120,-6.,6.);

  TH1D *pi0Eta = new TH1D("pi0Eta","Eta of thrown #pi;#eta",120,-6.,6.);
  
  TH1D *kaonEta = new TH1D("kaonEta","Eta of thrown K;#eta",120,-6.,6.);
  TH1D *kaonRecEta = new TH1D("kaonRecEta","Eta of reco K;#eta",120,-6.,6.);
  
  TH1D *rho0Eta = new TH1D("rho0Eta","Eta of thrown #rho^{0};#eta",120,-6.,6.);
  TH1D *pipmfromrho0Eta = new TH1D("pipmfromrho0Eta","generated #eta of pi^{#pm} from rho^{0} decay;#eta",120,-6.,6.);
  TH1D *pipmfromrho0RecEta = new TH1D("pipmfromrho0RecEta","reconstructed #eta of pi^{#pm} from rho^{0} decay;#eta",120,-6.,6.);
  
  // phi(1020)
  TH1D *phiEta = new TH1D("phiEta","Eta of thrown #phi(1020);#eta",120,-6.,6.);
  TH1D *kpmfromphiEta = new TH1D("kpmfromphiEta","generated #eta of K^{#pm} from #phi(1020) decay;#eta",120,-6.,6.);
  TH1D *kpmfromphiRecEta = new TH1D("kpmfromphiRecEta","reconstructed #eta of K^{#pm} from #phi(1020) decay;#eta",120,-6.,6.);
  
  // momentum
  TH1D *partMom = new TH1D("partMom","Mom of thrown particles; P [GeV]",150,0.,150.);
  TH1D *recP = new TH1D("recP","Momentum of reconstructed tracks; P [GeV]",150,0.,150.);
  
  // phi
  TH1D *partPhi = new TH1D("partPhi","Phi of thrown charged particles; #phi [rad]",150,-3.2,3.2);
  TH1D *recPhi = new TH1D("recPhi","Phi of reconstructed tracks; #phi [rad]",150,-3.2,3.2);
  
  // count events:
  int ievgen = 0;
  // count generated particles:
  int ngen_electrons = 0; //+-
  int ngen_protons = 0; //+-
  int ngen_muons = 0; //+-
  int ngen_pions = 0; //+-
  int ngen_pi0=0;
  int ngen_kaons = 0; //+-
  int ngen_rho0 = 0;
  int ngen_rhop = 0;
  int ngen_phi = 0;
  int ngen_omega = 0;
  int ngen_jpsi = 0;
  int ngen_upsilon = 0;
  int ngen_d0 = 0;
  int ngen_b0 = 0;
  // count reconstructed particles (+-):
  int nrec_electrons = 0;
  int nrec_protons = 0;
  int nrec_muons = 0;
  int nrec_pions = 0;
  int nrec_kaons = 0;
  // count number of decays:
  int ndecay_pi0_gg = 0;
  int ndecay_rho0_pp = 0;
  int ndecay_rho0_mumu = 0;
  int ndecay_rho0_ee = 0;
  int ndecay_phi_kk = 0;
  // count number of decay particles (reco level) in nHCal acceptance:
  int ndecay_rho0_pionpm_nHCal = 0;
  int ndecay_phi_kaonpm_nHCal = 0;
  int ndecay_phi_kaon_pair_nHCal = 0;
  int ndecay_phi_kaon_single_nHCal = 0;
  int ndecay_phi_kaon_none_nHCal = 0;
  // tag decays on generated particle level:
  int is_rho0decay_pp = 0; // 0 or 1 for a given generated particle
  int is_rho0decay_mumu = 0;
  int is_rho0decay_ee = 0;
  int is_phidecay_kk = 0;

  cout << "+ Ready to run over events... \n";

  
  while(tree_reader.Next()) { // Loop over events

    ievgen++;
    int kaons_in_nHCal = 0; //set the number of kaons within tolerance to 0 and reset for each particle
    float k1_eta = 0;
    float k2_eta = 0;
    cout << "+ Entering event #: " << ievgen << " \n";
    
    //cout << "Event #: " << ievgen << ", " << partGenStat.GetSize() << " gen particles, " << parents_index.GetSize() << " parent particles, " << daughters_index.GetSize() << " daughter particles \n";   // parent_index and daughter_index must be of the same length since they are in the same tree (is that what pushback does?)

    for(unsigned int i=0; i<partGenStat.GetSize(); i++) // Loop over generated particles
      {
    //cout << "++ Entering generated particle #: " << i << " \n";
    
    
    int pdg = TMath::Abs(partPdg[i]);
    TVector3 trueMom(partMomX[i],partMomY[i],partMomZ[i]);
    float trueEta = trueMom.PseudoRapidity();
    float truePhi = trueMom.Phi();
    
          

    generatorStatus->Fill(partGenStat[i]);
    // reset particle decays for each new generated particle:
    is_rho0decay_pp = 0;
    is_rho0decay_mumu = 0;
    is_rho0decay_ee = 0;
    is_phidecay_kk = 0;
    
    int i_parents_begin = parents_begin[i];
        int i_parents_end = parents_end[i];
    int i_parents = parents_end[i] - parents_begin[i];

    int i_daughters_begin = daughters_begin[i];
        int i_daughters_end = daughters_end[i] - 1;
    int i_daughters = daughters_end[i] - daughters_begin[i];

    //Consider only selected generated particles:
    if( (partGenStat[i] == 1) || (partGenStat[i] == 2) /*|| (partGenStat[i] = 4 */) // Select only stable or decay particles
    {
      //cout << "Ev#: " << ievgen << ", P-index: " << i <<", PDG: " << partPdg[i] << ", GenStatus:" << partGenStat[i] << ", i_parents: "<< i_parents<<", i_daughters: " << i_daughters << ", pb: " << parents_index[i_parents_begin] << ", pe: " << parents_index[i_parents_end] <<  ", db: " << daughters_index[i_daughters_begin] << ", de: " << daughters_index[i_daughters_end] << " \n";

      // rho0 decays
      if( partPdg[i] == 113 )
        {
          //cout << "Event " << ievgen << " with gen rho0 #: " << partPdg[i] << ", daughter 1:" << partPdg[daughters_index[i_daughters_begin]] << ", daughter 2:" << partPdg[daughters_index[i_daughters_begin]+1] << ", i_daughters:" << i_daughters << "  \n";

          // count the 2-body rho0 decays:
          if( i_daughters == 2 )
        {
          if( (partPdg[daughters_index[i_daughters_begin]] == 211 ) && ( partPdg[daughters_index[i_daughters_begin]+1] == -211 ) || (partPdg[daughters_index[i_daughters_begin]] == -211 ) && ( partPdg[daughters_index[i_daughters_begin]+1] == 211) )
            {
              //cout << "-> Event " << ievgen << " found rho0 decayed into pi+ pi-: "  << partPdg[daughters_index[i_daughters_begin]] << " and " << partPdg[daughters_index[i_daughters_begin]+1] << "  \n";
              
              // count it:
              ndecay_rho0_pp++;
              // tag it:
              is_rho0decay_pp = 1;
              
              TVector3 trueMom_rho0_pi1(partMomX[daughters_index[i_daughters_begin]],partMomY[daughters_index[i_daughters_begin]],partMomZ[daughters_index[i_daughters_begin]]);
              TVector3 trueMom_rho0_pi2(partMomX[daughters_index[i_daughters_begin]+1],partMomY[daughters_index[i_daughters_begin]+1],partMomZ[daughters_index[i_daughters_begin]+1]);
              TVector3 trueMom_rho0_pi12=trueMom_rho0_pi1 + trueMom_rho0_pi2;
              
              float trueEta_rho0_pi1 = trueMom_rho0_pi1.PseudoRapidity();
              float trueEta_rho0_pi2 = trueMom_rho0_pi2.PseudoRapidity();
              pipmfromrho0Eta->Fill(trueEta_rho0_pi1);
              pipmfromrho0Eta->Fill(trueEta_rho0_pi2);

              //          cout << "--> Event " << ievgen << " rho0 decay to 2pi: generated rho0 eta: " << trueEta << ", pi1: " << trueEta_rho0_pi1 << ", pi2: " << trueEta_rho0_pi2 << "  \n";
              //  cout << "            trueMomrho0 X: " << trueMom.X() << ", trueMomrho0 Y: " << trueMom.Y() <<", trueMomrho0 Z: " << trueMom.Z() << "  \n";
              // cout << "            trueMom_rho0_pi12 X: " << trueMom_rho0_pi12.X() << ", trueMom_rho0_pi12 Y: " << trueMompi12.Y() <<", trueMom_rho0_pi12 Z: " << trueMom_rho0_pi12.Z() << "  \n";
            
            } // end of rho0 to pi+pi- decays
          else if( (partPdg[daughters_index[i_daughters_begin]] == 13 ) && ( partPdg[daughters_index[i_daughters_begin]+1] == -13 ) || (partPdg[daughters_index[i_daughters_begin]] == -13 ) && ( partPdg[daughters_index[i_daughters_begin]+1] == 13) )
            {
              ndecay_rho0_mumu++;
            } // end of rho0 to mumu decays
          else if( (partPdg[daughters_index[i_daughters_begin]] == 11 ) && ( partPdg[daughters_index[i_daughters_begin]+1] == -11 ) || (partPdg[daughters_index[i_daughters_begin]] == -11 ) && ( partPdg[daughters_index[i_daughters_begin]] == 11) )
            {
              ndecay_rho0_ee++;
            } // end of ee decays
        } // end of 2-body decays
        } // end of rho0 decays
      // pi0 decays
      else if( partPdg[i] == 111 )
        {
          //cout << "Event with gen pi0 #: " << ievgen << ", daughter 1:" << partPdg[daughters_index[i_daughters_begin]] << ", daughter 2:" << partPdg[daughters_index[i_daughters_begin]+1] << ", i_daughters:" << i_daughters << "  \n";
          if( (partPdg[daughters_index[i_daughters_begin]] == 22) && (partPdg[daughters_index[i_daughters_begin]+1] == 22) )
        {
          ndecay_pi0_gg++;
        } // end of gg decays
        }// end of pi0 decay

      // phi(1020) decays:
            if( partPdg[i] == 333 )
        {
          //cout << "Event " << ievgen << " with gen phi(1020): " << partPdg[i] << ", daughter 1:" << partPdg[daughters_index[i_daughters_begin]] << ", daughter 2:" << partPdg[daughters_index[i_daughters_begin]+1] << ", i_daughters:" << i_daughters << "  \n";

          // count the 2-body phi(1020) decays:
          if( i_daughters == 2 )
        {
          if( (partPdg[daughters_index[i_daughters_begin]] == 321 ) && ( partPdg[daughters_index[i_daughters_begin]+1] == -321 ) || (partPdg[daughters_index[i_daughters_begin]] == -321 ) && ( partPdg[daughters_index[i_daughters_begin]+1] == 321) )
            {
              //cout << "-> Event " << ievgen << " found phi(1020) decayed into K+ K-: "  << partPdg[daughters_index[i_daughters_begin]] << " and " << partPdg[daughters_index[i_daughters_begin]+1] << "  \n";
              
              // count it:
              ndecay_phi_kk++;
              // tag it:
              is_phidecay_kk = 1;

              TVector3 trueMom_phi_k1(partMomX[daughters_index[i_daughters_begin]],partMomY[daughters_index[i_daughters_begin]],partMomZ[daughters_index[i_daughters_begin]]);
              TVector3 trueMom_phi_k2(partMomX[daughters_index[i_daughters_begin]+1],partMomY[daughters_index[i_daughters_begin]+1],partMomZ[daughters_index[i_daughters_begin]+1]);
              TVector3 trueMom_phi_k12=trueMom_phi_k1 + trueMom_phi_k2;
              
              float trueEta_phi_k1 = trueMom_phi_k1.PseudoRapidity();
              float trueEta_phi_k2 = trueMom_phi_k2.PseudoRapidity();
              kpmfromphiEta->Fill(trueEta_phi_k1);
              kpmfromphiEta->Fill(trueEta_phi_k2);

              //cout << "--> Event " << ievgen << " phi(1020) decay to 2pi: generated phi eta: " << trueEta << ", pi1: " << trueEta_phi_k1 << ", pi2: " << trueEta_phi_k2 << "  \n";
              //cout << "            trueMomphi X: " << trueMom.X() << ", trueMomphi Y: " << trueMom.Y() <<", trueMomphi Z: " << trueMom.Z() << "  \n";
              //cout << "            trueMom_phi_k12 X: " << trueMom_phi_k12.X() << ", trueMom_phi_k12 Y: " << trueMom_phi_k12.Y() <<", trueMom_phi_k12 Z: " << trueMom_phi_k12.Z() << "  \n";
            
            } // end of phi to pi+pi- decays
        } // end of 2-body decays
        } // end of phi meson decays
      
      // count any generated particles:
      if( pdg == 11){
        ngen_electrons++;
        electronEta->Fill(trueEta);
      }// electrons
      else if( pdg == 13){
        ngen_muons++;
        muonEta->Fill(trueEta);
      }// muons
      else if( pdg == 211){
        ngen_pions++;
        pionEta->Fill(trueEta);
      }//pions_pm
      else if( pdg == 111){
        ngen_pi0++;
        pi0Eta->Fill(trueEta);
      }//pions_pm
      else if( pdg == 321 ){
        ngen_kaons++;
        kaonEta->Fill(trueEta);
      } // kaons_pm
      else if( pdg == 113){
        ngen_rho0++;
        rho0Eta->Fill(trueEta);
      } // rho(770)
      else if( pdg == 443){
        ngen_jpsi++;
      } // J/Psi(1S)
      else if( pdg == 2212){
        ngen_protons++;
        protonEta->Fill(trueEta);
      }// protons
      else if( pdg == 213){
        ngen_rhop++;
      }// rhop
      else if( pdg == 333){
        ngen_phi++;
        phiEta->Fill(trueEta);
      }// phi(1020)
      else if( pdg == 223){
        ngen_omega++;
      }// omega(982)
      else if( pdg == 553){
        ngen_upsilon++;
      }// Upsilon(1S)
      else if( pdg == 421){
        ngen_d0++;
      }// D0
      else if( pdg == 511){
        ngen_b0++;
      }// B0
        
        
      //Fill all true eta:
      partEta->Fill(trueEta);
        
      // Fill all true momentum:
      partMom->Fill(trueMom.Mag());
      
      // Fill all true phi:
      partPhi->Fill(truePhi);
      
      
      // Loop over associations to find matching ReconstructedChargedParticle
      for(unsigned int j=0; j<simuAssoc.GetSize(); j++)
        {
          //cout << "*** Event " << ievgen << ", generated particle " << i << ", simID " << j << " \n";
          
          if(simuAssoc[j] == i) // Find association index matching the index of the thrown particle we are looking at
        {
          //cout << "***** Event " << ievgen << ", found association index: " << simuAssoc[j] << " \n";
          
          TVector3 recMom(trackMomX[recoAssoc[j]],trackMomY[recoAssoc[j]],trackMomZ[recoAssoc[j]]); // recoAssoc[j] is the index of the matched ReconstructedChargedParticle
          
          float CPartEta = recMom.PseudoRapidity();
          float CPartPhi = recMom.Phi();
          
          recEta->Fill(CPartEta);
          recPhi->Fill(CPartPhi);
          recP->Fill(recMom.Mag());
          
          //cout << "Particle is pdg: " << pdg << " .\n";
          
          if( pdg == 11){
            nrec_electrons++;
            electronRecEta->Fill(CPartEta);
          }// electrons
          else if( pdg == 13){
            nrec_muons++;
            muonRecEta->Fill(CPartEta);
          }// muons
          else if( pdg == 211){
            nrec_pions++;
            pionRecEta->Fill(CPartEta);
          }//pions
          else if( pdg == 321){
            nrec_kaons++;
            kaonRecEta->Fill(CPartEta);
          }//pions
          else if( pdg == 2212){
            nrec_protons++;
            protonRecEta->Fill(CPartEta);
          }// protons
        } // end of matched association gen to rec
          
          // Match the decay particles to their recos:
          // rho0 to pi+ pi-
          if( is_rho0decay_pp )
        {
          if( simuAssoc[j] == daughters_index[i_daughters_begin] ) // get the reco decay pi1 of the gen rho0, by accessing the correct MCParticle index
            {
              TVector3 recMom_rho0_pi1(trackMomX[recoAssoc[j]],trackMomY[recoAssoc[j]],trackMomZ[recoAssoc[j]]);
              float recEta_rho0_pi1 = recMom_rho0_pi1.PseudoRapidity();
              float recPhi_rho0_pi1 = recMom_rho0_pi1.Phi();
              pipmfromrho0RecEta->Fill(recEta_rho0_pi1);

              // count the decay pions (reco level) that are within the nHCal acceptance, here pion1:
              if(in_Cal_Tolerance(nHCal_name, recEta_rho0_pi1))
            {
              ndecay_rho0_pionpm_nHCal++;
            }
              //cout << "---> Event " << ievgen << " rho0 decay, reco index rho0: " << j << " \n";
              //cout << "          reco daughter-1 eta: " << recEta_rho0_pi1 << ", reco index daughter-1: " << daughters_index[i_daughters_begin] << " \n";
            }// end of rho0 decay pi1
          else if( simuAssoc[j] == daughters_index[i_daughters_begin]+1 ) // get the reco decay pi2 of the gen rho0
            {
              TVector3 recMom_rho0_pi2(trackMomX[recoAssoc[j]],trackMomY[recoAssoc[j]],trackMomZ[recoAssoc[j]]);
              float recEta_rho0_pi2 = recMom_rho0_pi2.PseudoRapidity();
              float recPhi_rho0_pi2 = recMom_rho0_pi2.Phi();
              pipmfromrho0RecEta->Fill(recEta_rho0_pi2);

              // count the decay pions (reco level) that are within the nHCal acceptance, here pion2:
              if(in_Cal_Tolerance(nHCal_name, recEta_rho0_pi2))
            {
              ndecay_rho0_pionpm_nHCal++;
            }
              
              //cout << "          reco daughter-2 eta: " << recEta_rho0_pi2  << ", reco index daughter-2: " << daughters_index[i_daughters_begin]+1 << " \n\n";
            }// end of rho0 decay pi2
        } // end of rho0 decay into pp
          // phi to K+ K-
          else if( is_phidecay_kk )
        {
            
            
            
            
          if( simuAssoc[j] == daughters_index[i_daughters_begin] ) // get the reco decay pi1 of the gen rho0, by accessing the correct MCParticle index
            {
                TVector3 recMom_phi_k1(trackMomX[recoAssoc[j]],trackMomY[recoAssoc[j]],trackMomZ[recoAssoc[j]]);
                float recEta_phi_k1 = recMom_phi_k1.PseudoRapidity();
                float recPhi_phi_k1 = recMom_phi_k1.Phi();
              kpmfromphiRecEta->Fill(recEta_phi_k1);
                k1_eta = recEta_phi_k1;
                
                //Check that the kaon is within a given HCal, in this case nHCal
                if(in_Cal_Tolerance(nHCal_name, recEta_phi_k1))
              {
                ndecay_phi_kaonpm_nHCal++;
                  kaons_in_nHCal++;
              }
              //cout << "---> Event " << ievgen << " phi(1020) decay, reco index phi(1020): " << j << " \n";
              //cout << "          reco daughter-1 eta: " << recEta_phi_k1 << ", reco index daughter-1: " << daughters_index[i_daughters_begin] << " \n";
            }// end of phi(1020) decay K1
          else if( simuAssoc[j] == daughters_index[i_daughters_begin]+1 ) // get the reco decay pi2 of the gen rho0
            {
                TVector3 recMom_phi_k2(trackMomX[recoAssoc[j]],trackMomY[recoAssoc[j]],trackMomZ[recoAssoc[j]]);
                float recEta_phi_k2 = recMom_phi_k2.PseudoRapidity();
                float recPhi_phi_k2 = recMom_phi_k2.Phi();
              kpmfromphiRecEta->Fill(recEta_phi_k2);
                k2_eta = recEta_phi_k2;
                
                //Check that the kaon is within a given HCal, in this case nHCal
                if(in_Cal_Tolerance(nHCal_name, recEta_phi_k2))
              {
                ndecay_phi_kaonpm_nHCal++;
                  kaons_in_nHCal++;
              }
              //cout << "          reco daughter-2 eta: " << recEta_phi_k2  << ", reco index daughter-2: " << daughters_index[i_daughters_begin]+1 << " \n\n";
            }// end of phi(1020) decay K2
        
            /*if (recEta_phi_k1 == recEta_phi_k2) {
                cout << "true\n";
            } else {
                cout << "false\n";
            }*/
            
            //Check if only one of the two kaons was within the nHCal acceptance
        } // end of phi(1020) decay into KK
          
        }// End loop over associations
    } // End stable or decay particles condition
      } // End loop over thrown particles, within that event
      
      //Create a phiDecay object for this decay and add it to the array
      decays[ievgen] = phiDecay(k1_eta, k2_eta, partXb[0], partQ2[0]);
      
      //Run methods to fill kaon occurrence matrices
      kaons_in_Cal(decays[ievgen].eta1, decays[ievgen].eta2);
      fill_Cal_Arr(decays[ievgen].eta1, decays[ievgen].eta2);
      
      //Fill Graphs/Histograms
      xBjorken->Fill(decays[ievgen].x_b);
      q2->Fill(decays[ievgen].q2);
      xB_v_q2->AddPoint(decays[ievgen].x_b, decays[ievgen].q2);

    //for(unsigned int k=0; k<trackMomX.GetSize(); k++){ // Loop over all reconstructed tracks, thrown or not

    //  TVector3 recMom(trackMomX[k], trackMomY[k], trackMomZ[k]);
    
    //} // End loop over all reconstructed tracks, within that event

    // now go to next event
    
  } // End loop over events
    
    //start construction of xB_v_percent plot
    
    //Create a matrix with 4 rows containing values for each bin for 0, 1, 2 kaons in nHCal and all calo(WIP)
    double inBins[5][xB_percent_nBins];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < xB_percent_nBins; j++) {
            inBins[i][j] = 0;
        }
    }
    
    for (phiDecay decay : decays) { // iterate over all the phidecays and inrement array bins
        int bin_num = xB_v_percent_0->FindBin(decay.x_b);
        inBins[decay.num_in_nHCal][bin_num] += 1.0; // increment bin of kaon in nHCal tolerance
        
        // Check if decay has eta in any calorimeter
        if (in_Cal_Tolerance(cals[0], decay.eta1) || in_Cal_Tolerance(cals[0], decay.eta2) ||
            in_Cal_Tolerance(cals[1], decay.eta1) || in_Cal_Tolerance(cals[1], decay.eta2) ||
            in_Cal_Tolerance(cals[2], decay.eta1) || in_Cal_Tolerance(cals[2], decay.eta2)) {
            inBins[3][bin_num] += 1.0;
        }
        inBins[4][bin_num] += 1.0;
    }
    
    for (int i = 0; i < xB_percent_nBins; i++) { // fill histograms with values from array
        if (inBins[4][i] == 0) { // to avoid 0/0, bins where the "all" is 0 will be set to 0
            xB_v_percent_0->SetBinContent(i, 0);
            xB_v_percent_1->SetBinContent(i, 0);
            xB_v_percent_2->SetBinContent(i, 0);
            if (inBins[3][i] == 0) { // handle edge case to make 0/0 show 100% here
                xB_v_percent_all->SetBinContent(i, 1);
            } else {
                xB_v_percent_all->SetBinContent(i, 0);
            }
        } else {
            xB_v_percent_0->SetBinContent(i, inBins[0][i] / inBins[4][i]);
            xB_v_percent_1->SetBinContent(i, inBins[1][i] / inBins[4][i]);
            xB_v_percent_2->SetBinContent(i, inBins[2][i] / inBins[4][i]);
            xB_v_percent_all->SetBinContent(i, inBins[3][i] / inBins[4][i]);
        }
    }
    
    for (int i = 0; i < 100; i++) {
        cout << "first is " << inBins[3][i] << " and " << inBins[4][i] << "so" << xB_v_percent_all->GetBinContent(i) << "\n";
    }
    
    //end construction of xB_v_percent plot
    
    //Write data to TGraph
    xB_v_q2->Write("xB_v_q2");

  // Calculate fractions:
  double fraction_rho0_pionpm_nHCal = 0.;
  fraction_rho0_pionpm_nHCal = ndecay_rho0_pp?(double(ndecay_rho0_pionpm_nHCal)/(2*double(ndecay_rho0_pp))):0;
  double fraction_phi_kaon_pair_nHCal = 0.;
  fraction_phi_kaon_pair_nHCal = ndecay_phi_kk?(double(ndecay_phi_kaon_pair_nHCal)/(double(ndecay_phi_kk))):0;
  double fraction_phi_kaon_single_nHCal = 0.;
    fraction_phi_kaon_single_nHCal = ndecay_phi_kk?(double(ndecay_phi_kaon_single_nHCal)/(double(ndecay_phi_kk))):0;
  double fraction_phi_kaon_none_nHCal =0;
    fraction_phi_kaon_none_nHCal = ndecay_phi_kk?(double(ndecay_phi_kaon_none_nHCal)/(double(ndecay_phi_kk))):0;
    
    //Calculate each value of kaonOccurrence as a percentage of the total number of kaons
    for (int i = 1; i < 5; i++) {
        for (int j = 1; j < 5; j++) {
            kaonOccurrence->SetBinContent(i,j, round(100*100*calMatrix[4-j][i-1]/ndecay_phi_kk)/100);
        }
    }
  //
  
  cout << "Number of generated events: " << ievgen << " \n\n";
  cout << "Number of generated electrons +-: " << ngen_electrons << " \n";
  cout << "Number of generated protons +-: " << ngen_protons << " \n";
  cout << "Number of generated muons +-: " << ngen_muons << " \n";
  cout << "Number of generated pions +-: " << ngen_pions << " \n";
  cout << "Number of generated pi0: " << ngen_pi0 << ", of which decay into 2 gamma: " << ndecay_pi0_gg <<  " \n";
  cout << "Number of generated kaons +-: " << ngen_kaons << " \n";
  cout << "Number of generated rho0: " << ngen_rho0 << ", of which decay into pi+ pi-: " << ndecay_rho0_pp << ", into mu+ mu-: " << ndecay_rho0_mumu << ", into e+ e-: " << ndecay_rho0_ee << " \n";
  cout << "        " << ndecay_rho0_pionpm_nHCal << " pi+ pi- make it into the nHCal acceptance, with corresponds to a fraction " << fraction_rho0_pionpm_nHCal << " \n";
  cout << "Number of generated rho+: " << ngen_rhop << " \n";
  cout << "Number of generated phi: " << ngen_phi <<", of which decay into K+ K-: " << ndecay_phi_kk << " \n";
  cout << "        " << ndecay_phi_kaon_none_nHCal << " phi decay result in no K+ K- within the nHCal acceptance at all, which corresponds to a fraction " << fraction_phi_kaon_none_nHCal << " \n";
  cout << "        " << ndecay_phi_kaon_single_nHCal << " phi decay result in one K+ or K- within into the nHCal acceptance, which corresponds to a fraction " << fraction_phi_kaon_single_nHCal << " \n";
  cout << "        " << ndecay_phi_kaon_pair_nHCal << " phi decay result in both K+ K- within nHCal acceptance, which corresponds to a fraction " << fraction_phi_kaon_pair_nHCal  << " \n";
  cout << "Number of generated omega: " << ngen_omega << " \n";
  cout << "Number of generated J/Psi: " << ngen_jpsi << " \n";
  cout << "Number of generated Upsilon: " << ngen_upsilon << " \n";
  cout << "Number of generated D0: " << ngen_d0 << " \n";
  cout << "Number of generated B0: " << ngen_b0 << " \n\n";
  cout << "Number of reconstructed electrons +-: " << nrec_electrons << " \n";
  cout << "Number of reconstructed protons +-: " << nrec_protons << " \n";
  cout << "Number of reconstructed muons +-: " << nrec_muons << " \n";
  cout << "Number of reconstructed pions +-: " << nrec_pions << " \n";
  cout << "Number of reconstructed kaons +-: " << nrec_kaons << " \n";

    //Print out calNums
    cout << "Calorimeter:    0 in   1 in   2 in \n ";
    for (int i = 0; i < 3; i++) {
        cout << cals[i] << ":          ";
        for (int j = 0; j < 3; j++) {
            cout << calNums[i][j] << "    ";
        }
        cout << "\n";
    }
    
    //Print out calMatrix
    for (int i = 0; i < 4; i++) {
        cout << calMatrix[i] << ":          ";
        for (int j = 0; j < 4; j++) {
            cout << calMatrix[i][j] << "    ";
        }
        cout << "\n";
    }
    
  ofile->Write(); // Write histograms to file
  ofile->Close(); // Close output file

  cout << "Output histograms written in: " << outfile << " \n";
  
  cout << "Thank you for running Caro's macro.\n";
  gSystem->Exec("date");
}
