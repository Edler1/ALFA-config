set RandomSeed 123

####################################################################                                l
# FCC-ee IDEA detector model
#
# Authors: Elisa Fontanesi, Lorenzo Pezzotti, Massimiliano Antonello, Michele Selvaggi
# email: efontane@bo.infn.it,
#        lorenzo.pezzotti01@universitadipavia.it,
#        m.antonello@uninsubria.it,
#        michele.selvaggi@cern.ch
#####################################################################

## MOD2: set vtx mode timing to MC truth

set B 3.0
set R 2.8
set HL 2.8


#######################################
# Order of execution of various modules
#######################################

set ExecutionPath {

  TruthVertexFinder
  ParticlePropagator

  ChargedHadronTrackingEfficiency
  ElectronTrackingEfficiency
  MuonTrackingEfficiency

  TrackMergerPre
  TrackSmearing
  TimeSmearing
  TimeOfFlight

  TrackMerger
  ForwardLooperTracks
  Calorimeter

  TimeSmearingNeutrals
  TimeOfFlightNeutralHadron

  EFlowTrackMerger
  EFlowMerger

  PhotonEfficiency
  PhotonIsolation

  MuonFilter
  TowerMerger

  ElectronFilter
  ElectronEfficiency
  ElectronIsolation

  MuonEfficiency
  MuonIsolation

  MissingET

  NeutrinoFilter
  GenJetFinder
  GenMissingET

  FastJetFinder
  JetEnergyScale

  GenJetFinderDurhamN2
  FastJetFinderDurhamN2

  JetFlavorAssociation

  BTagging
  CTagging
  TauTagging

  TreeWriter
}

#################################
# Truth Vertex Finder
#################################

module TruthVertexFinder TruthVertexFinder {

  ## below this distance two vertices are assumed to be merged
  set Resolution 1E-06

  set InputArray Delphes/stableParticles
  set VertexOutputArray vertices
}


#################################
# Propagate particles in cylinder
#################################

module ParticlePropagator ParticlePropagator {
  set InputArray Delphes/stableParticles

  set OutputArray stableParticles
  set ChargedHadronOutputArray chargedHadrons
  set ElectronOutputArray electrons
  set MuonOutputArray muons

  # inner radius of the solenoid, in m
  set Radius $R

  # half-length: z of the solenoid, in m
  set HalfLength $HL

  # magnetic field, in T
  set Bz $B
}

####################################
# Charged hadron tracking efficiency
####################################

module Efficiency ChargedHadronTrackingEfficiency {
    set InputArray ParticlePropagator/chargedHadrons
    set OutputArray chargedHadrons
    set UseMomentumVector true

    set EfficiencyFormula {
        (abs(eta) > 2.56)                                  * (0.000) +
        (pt < 0.1) * (abs(eta) <= 2.56)       * (0.000) +
        (pt >= 0.1) * (abs(eta) <= 2.56)      * (1.000)
    }
}



##############################
# Electron tracking efficiency
##############################

module Efficiency ElectronTrackingEfficiency {
    set InputArray ParticlePropagator/electrons
    set OutputArray electrons
    set UseMomentumVector true

    set EfficiencyFormula {
        (abs(eta) > 2.56)                                  * (0.000) +
        (pt < 0.1) * (abs(eta) <= 2.56)                    * (0.000) +
        (pt >= 0.1) * (abs(eta) <= 2.56)                   * (1.000)
    }
}


##########################
# Muon tracking efficiency
##########################

module Efficiency MuonTrackingEfficiency {
    set InputArray ParticlePropagator/muons
    set OutputArray muons
    set UseMomentumVector true

    set EfficiencyFormula {
        (abs(eta) > 2.56)                                  * (0.000) +
        (pt < 0.1) * (abs(eta) <= 2.56)       * (0.000) +
        (pt >= 0.1) * (abs(eta) <= 2.56)      * (1.000)
    }
}

##############
# Track merger
##############

module Merger TrackMergerPre {
# add InputArray InputArray
  add InputArray ChargedHadronTrackingEfficiency/chargedHadrons
  add InputArray ElectronTrackingEfficiency/electrons
  add InputArray MuonTrackingEfficiency/muons
  set OutputArray tracks
}



########################################
# Smearing for charged tracks
########################################

module TrackCovariance TrackSmearing {

    set InputArray TrackMergerPre/tracks
    set OutputArray tracks

    ## minimum number of hits to accept a track
    set NMinHits 6

    ## magnetic field
    set Bz $B

    ## scale factors
    set ElectronScaleFactor  {1.25}


    set DetectorGeometry {

      # Layer type 1 = R (barrel) or 2 = z (forward/backward)
      # Layer label
      # Minimum dimension z for barrel or R for forward
      # Maximum dimension z for barrel or R for forward
      # R/z location of layer
      # Thickness (meters)
      # Radiation length (meters)
      # Number of measurements in layers (1D or 2D)
      # Stereo angle (rad) - 0(pi/2) = axial(z) layer - Upper side
      # Stereo angle (rad) - 0(pi/2) = axial(z) layer - Lower side
      # Resolution Upper side (meters) - 0 = no measurement
      # Resolution Lower side (meters) - 0 = no measurement
      # measurement flag = T, scattering only = F

      # barrel  name       zmin   zmax   r        w (m)      X0        n_meas  th_up (rad) th_down (rad)    reso_up (m)   reso_down (m)  flag

        1 PIPE -100 100 0.01 0.00235 0.3528 0 0 0 0 0 0
        1 VTX -0.0928 0.0928 0.013 0.00005 0.0746 2 0 1.5708 2.89E-06 2.89E-06 1
        1 VTX -0.1272 0.1272 0.0178 0.00005 0.0746 2 0 1.5708 2.89E-06 2.89E-06 1
        1 VTX -0.1696 0.1696 0.0238 0.00005 0.0746 2 0 1.5708 2.89E-06 2.89E-06 1
        1 VTX -0.212 0.212 0.0298 0.00005 0.0746 2 0 1.5708 2.89E-06 2.89E-06 1
        1 VTX -0.2544 0.2544 0.0357 0.00005 0.0746 2 0 1.5708 2.89E-06 2.89E-06 1
        1 VTXCYL -0.3032 0.3032 0.0432 0.003 0.797 0 0 0 0 0 0
        1 OUT2 -1.345 1.345 1.76603 0.00014 0.1158 2 0 1.5708 2.89E-06 2.89E-06 1
        1 OUT2 -1.345 1.345 1.71603 0.00014 0.1125 2 0 1.5708 2.89E-06 2.89E-06 1
        1 OUT1 -1.345 1.345 1.63982 0.00014 0.1158 2 0 1.5708 2.89E-06 2.89E-06 1
        1 OUT1 -1.345 1.345 1.58982 0.00014 0.1123 2 0 1.5708 2.89E-06 2.89E-06 1
        1 OUT2 -1.345 1.345 1.74103 0.05 12.7193 0 0 0 0 0 0
        1 OUT1 -1.345 1.345 1.61482 0.05 12.7187 0 0 0 0 0 0
        1 MID2 -1.345 1.345 0.94548 0.00014 0.1157 2 0 1.5708 2.89E-06 2.89E-06 1
        1 MID2 -1.345 1.345 0.89548 0.00014 0.1096 2 0 1.5708 2.89E-06 2.89E-06 1
        1 MID1 -1.345 1.345 0.81917 0.00014 0.1157 2 0 1.5708 2.89E-06 2.89E-06 1
        1 MID1 -1.345 1.345 0.76917 0.00014 0.1086 2 0 1.5708 2.89E-06 2.89E-06 1
        1 MID2 -1.345 1.345 0.92048 0.05 12.711 0 0 0 0 0 0
        1 MID1 -1.345 1.345 0.79417 0.05 12.7072 0 0 0 0 0 0
        1 VTXSUP -0.0928 0.0928 0.0154 0.0048 30.86 0 0 0 0 0 0
        1 VTXSUP -0.1272 0.1272 0.0208 0.006 50.18 0 0 0 0 0 0
        1 VTXSUP -0.1696 0.1696 0.0268 0.006 56.72 0 0 0 0 0 0
        1 VTXSUP -0.212 0.212 0.03275 0.0059 64.73 0 0 0 0 0 0
        1 VTXSUP -0.2544 0.2544 0.0387 0.006 71.26 0 0 0 0 0 0
        2 EC1 0.15 1.766 1.385 0.00014 0.1168 2 0 1.5708 2.89E-06 2.89E-06 1
        2 EC1 0.15 1.766 1.41 0.05 9.3741 0 0 0 0 0 0
        2 EC1 0.15 1.766 1.435 0.00014 0.1168 2 0 1.5708 2.89E-06 2.89E-06 1
        2 EC2 0.2 1.766 1.55387 0.00014 0.1168 2 0 1.5708 2.89E-06 2.89E-06 1
        2 EC2 0.2 1.766 1.57887 0.05 9.3741 0 0 0 0 0 0
        2 EC2 0.2 1.766 1.60387 0.00014 0.1168 2 0 1.5708 2.89E-06 2.89E-06 1
        2 EC3 0.2 1.766 1.74296 0.00014 0.1168 2 0 1.5708 2.89E-06 2.89E-06 1
        2 EC3 0.2 1.766 1.76796 0.05 9.3741 0 0 0 0 0 0
        2 EC3 0.2 1.766 1.79296 0.00014 0.1168 2 0 1.5708 2.89E-06 2.89E-06 1
        2 EC4 0.25 1.766 1.9547 0.00014 0.1168 2 0 1.5708 2.89E-06 2.89E-06 1
        2 EC4 0.25 1.766 1.9797 0.05 9.3741 0 0 0 0 0 0
        2 EC4 0.25 1.766 2.0047 0.00014 0.1168 2 0 1.5708 2.89E-06 2.89E-06 1
        2 EC1 0.15 1.766 -1.385 0.00014 0.1168 2 0 1.5708 2.88675E-06 2.88675E-06 1
        2 EC1 0.15 1.766 -1.41 0.05 9.3741 0 0 0 0 0 0
        2 EC1 0.15 1.766 -1.435 0.00014 0.1168 2 0 1.5708 2.88675E-06 2.88675E-06 1
        2 EC2 0.2 1.766 -1.55387 0.00014 0.1168 2 0 1.5708 2.88675E-06 2.88675E-06 1
        2 EC2 0.2 1.766 -1.57887 0.05 9.3741 0 0 0 0 0 0
        2 EC2 0.2 1.766 -1.60387 0.00014 0.1168 2 0 1.5708 2.88675E-06 2.88675E-06 1
        2 EC3 0.2 1.766 -1.74296 0.00014 0.1168 2 0 1.5708 2.88675E-06 2.88675E-06 1
        2 EC3 0.2 1.766 -1.76796 0.05 9.3741 0 0 0 0 0 0
        2 EC3 0.2 1.766 -1.79296 0.00014 0.1168 2 0 1.5708 2.88675E-06 2.88675E-06 1
        2 EC4 0.25 1.766 -1.9547 0.00014 0.1168 2 0 1.5708 2.88675E-06 2.88675E-06 1
        2 EC4 0.25 1.766 -1.9797 0.05 9.3741 0 0 0 0 0 0
        2 EC4 0.25 1.766 -2.0047 0.00014 0.1168 2 0 1.5708 2.88675E-06 2.88675E-06 1
        2 FLANGE 0.7567 1.7797 1.355 0.02 17.5374 0 0 0 0 0 0
        2 FLANGE 0.7567 1.7797 -1.355 0.02 17.5374 0 0 0 0 0 0
        2 BARSERV 0.7692 1.7797 1.365 0.02 7.7466 0 0 0 0 0 0
        2 BARSERV 0.7692 1.7797 -1.365 0.02 7.7466 0 0 0 0 0 0


    }

}



########################################
#   Time Smearing MIP
########################################

module TimeSmearing TimeSmearing {
  set InputArray TrackSmearing/tracks
  set OutputArray tracks

  # assume constant 30 ps resolution for now
  set TimeResolution {
                       (abs(eta) > 0.0 && abs(eta) <= 3.0)* 30E-12
                     }
}


########################################
#   Time Of Flight Measurement
########################################

module TimeOfFlight TimeOfFlight {
  set InputArray TimeSmearing/tracks
  set VertexInputArray TruthVertexFinder/vertices

  set OutputArray tracks

  # 0: assume vertex time tV from MC Truth (ideal case)
  # 1: assume vertex time tV = 0
  # 2: calculate vertex time as vertex TOF, assuming tPV=0

  set VertexTimeMode 0
}

##############
# Track merger
##############

module Merger TrackMerger {
# add InputArray InputArray
  add InputArray TimeOfFlight/tracks
  set OutputArray tracks
}


######################
# Looper Selection
######################

module Efficiency ForwardLooperTracks  {
  set InputArray TrackMerger/tracks
  set OutputArray tracks
  set UseMomentumVector False

  ## select looping tracks that end up in position |eta| > 3.142 (lost by calo)
  set EfficiencyFormula {
    (abs(eta) > 3.0 )                                 * (1.000) +
    (abs(eta) <= 3.0 )                                * (0.000)
  }

}


#############
# Calorimeter
#############
module DualReadoutCalorimeter Calorimeter {
  set ParticleInputArray ParticlePropagator/stableParticles
  set TrackInputArray TrackMerger/tracks

  set TowerOutputArray towers
  set PhotonOutputArray photons

  set EFlowTrackOutputArray eflowTracks
  set EFlowPhotonOutputArray eflowPhotons
  set EFlowNeutralHadronOutputArray eflowNeutralHadrons

  set ECalMinSignificance 2.0
  set HCalMinSignificance 2.5

  set SmearLogNormal false

  set SmearTowerCenter true
  #set SmearTowerCenter false
    set pi [expr {acos(-1)}]

    # Lists of the edges of each tower in eta and phi;
    # each list starts with the lower edge of the first tower;
    # the list ends with the higher edged of the last tower.
    # Barrel:  deta=0.02 towers up to |eta| <= 0.88 ( up to 45°)
    # Endcaps: deta=0.02 towers up to |eta| <= 3.0 (8.6° = 100 mrad)
    # Cell size: about 6 cm x 6 cm

    set EtaPhiRes 0.02
    set EtaMax 3.0

    set pi [expr {acos(-1)}]

    set nbins_phi [expr {$pi/$EtaPhiRes} ]
    set nbins_phi [expr {int($nbins_phi)} ]

    set PhiBins {}
    for {set i -$nbins_phi} {$i <= $nbins_phi} {incr i} {
      add PhiBins [expr {$i * $pi/$nbins_phi}]
    }

    set nbins_eta [expr {$EtaMax/$EtaPhiRes} ]
    set nbins_eta [expr {int($nbins_eta)} ]

    for {set i -$nbins_eta} {$i <= $nbins_eta} {incr i} {
      set eta [expr {$i * $EtaPhiRes}]
      add EtaPhiBins $eta $PhiBins
    }

    # default energy fractions {abs(PDG code)} {Fecal Fhcal}
    add EnergyFraction {0} {0.0 1.0}
    # energy fractions for e, gamma and pi0
    add EnergyFraction {11} {1.0 0.0}
    add EnergyFraction {22} {1.0 0.0}
    add EnergyFraction {111} {1.0 0.0}
    # energy fractions for muon, neutrinos and neutralinos
    add EnergyFraction {12} {0.0 0.0}
    add EnergyFraction {13} {0.0 0.0}
    add EnergyFraction {14} {0.0 0.0}
    add EnergyFraction {16} {0.0 0.0}
    add EnergyFraction {1000022} {0.0 0.0}
    add EnergyFraction {1000023} {0.0 0.0}
    add EnergyFraction {1000025} {0.0 0.0}
    add EnergyFraction {1000035} {0.0 0.0}
    add EnergyFraction {1000045} {0.0 0.0}
    # energy fractions for K0short and Lambda
    add EnergyFraction {310} {0.3 0.7}
    add EnergyFraction {130} {0.3 0.7}
    add EnergyFraction {3122} {0.3 0.7}


    ## ECAL crystals for the EM part from 2008.00338
    # set ECalResolutionFormula {resolution formula as a function of eta and energy}
    set ECalResolutionFormula {
    (abs(eta) <= 0.88 )                     * sqrt(energy^2*0.005^2 + energy*0.03^2 + 0.002^2)+
    (abs(eta) > 0.88 && abs(eta) <= 3.0)    * sqrt(energy^2*0.005^2 + energy*0.03^2 + 0.002^2)
    }


    # Dual Readout
    # set HCalResolutionFormula {resolution formula as a function of eta and energy}
    set HCalResolutionFormula {
    (abs(eta) <= 0.88 )                     * sqrt(energy^2*0.01^2 + energy*0.3^2 + 0.05^2)+
    (abs(eta) > 0.88 && abs(eta) <= 3.0)    * sqrt(energy^2*0.01^2 + energy*0.3^2 + 0.05^2)
    }
}

########################################
#   Time Smearing Neutrals
########################################

module TimeSmearing TimeSmearingNeutrals {
  set InputArray Calorimeter/eflowNeutralHadrons
  set OutputArray eflowNeutralHadrons

  # assume constant 30 ps resolution for now
  set TimeResolution {
                       (abs(eta) > 0.0 && abs(eta) <= 3.0)* 30E-12
                     }
}

########################################
#   Time Of Flight Measurement
########################################

module TimeOfFlight TimeOfFlightNeutralHadron {
  set InputArray TimeSmearingNeutrals/eflowNeutralHadrons
  set VertexInputArray TruthVertexFinder/vertices

  set OutputArray eflowNeutralHadrons

  # 0: assume vertex time tV from MC Truth (ideal case)
  # 1: assume vertex time tV = 0
  # 2: calculate vertex time as vertex TOF, assuming tPV=0

  ## TBF (add option to take hard vertex time)
  set VertexTimeMode 1
}


############################
# Energy flow track merger
############################

module Merger EFlowTrackMerger {
# add InputArray InputArray
  add InputArray Calorimeter/eflowTracks
  add InputArray ForwardLooperTracks/tracks
  set OutputArray eflowTracks
}



####################
# Energy flow merger
####################

module Merger EFlowMerger {
# add InputArray InputArray
  add InputArray EFlowTrackMerger/eflowTracks
  add InputArray Calorimeter/eflowPhotons
  add InputArray TimeOfFlightNeutralHadron/eflowNeutralHadrons
  set OutputArray eflow
}


####################
# Tower merger
####################

module Merger TowerMerger {
# add InputArray InputArray
  add InputArray Calorimeter/towers
  add InputArray MuonFilter/muons
  set OutputArray towers
}


###################
# Photon efficiency
###################

module Efficiency PhotonEfficiency {
  set InputArray Calorimeter/eflowPhotons
  set OutputArray photons

  # set EfficiencyFormula {efficiency formula as a function of eta and pt}
  # efficiency formula for photons
  set EfficiencyFormula {
        (energy < 2.0)                                        * (0.000)+
        (energy >= 2.0) * (abs(eta) <= 0.88)                  * (0.99) +
        (energy >= 2.0) * (abs(eta) >0.88 && abs(eta) <= 3.0) * (0.99) +
        (abs(eta) > 3.0)                                      * (0.000)
  }
}

##################
# Photon isolation
##################

module Isolation PhotonIsolation {
  set CandidateInputArray PhotonEfficiency/photons
  set IsolationInputArray EFlowMerger/eflow

  set OutputArray photons

  set DeltaRMax 0.5

  set PTMin 0.5

  set PTRatioMax 9999.
}

#################
# Electron filter
#################

module PdgCodeFilter ElectronFilter {
  set InputArray EFlowTrackMerger/eflowTracks
  set OutputArray electrons
  set Invert true
  add PdgCode {11}
  add PdgCode {-11}
}

#################
# Muon filter
#################

module PdgCodeFilter MuonFilter {
  set InputArray EFlowTrackMerger/eflowTracks
  set OutputArray muons
  set Invert true
  add PdgCode {13}
  add PdgCode {-13}
}


#####################
# Electron efficiency
#####################

module Efficiency ElectronEfficiency {
  set InputArray ElectronFilter/electrons
  set OutputArray electrons

  # set EfficiencyFormula {efficiency formula as a function of eta and pt}

  # efficiency formula for electrons
  set EfficiencyFormula {
        (energy < 2.0)                                         * (0.000)+
        (energy >= 2.0) * (abs(eta) <= 0.88)                   * (0.99) +
        (energy >= 2.0) * (abs(eta) >0.88 && abs(eta) <= 3.0)  * (0.99) +
        (abs(eta) > 3.0)                                       * (0.000)
  }
}

####################
# Electron isolation
####################

module Isolation ElectronIsolation {
  set CandidateInputArray ElectronEfficiency/electrons
  set IsolationInputArray EFlowMerger/eflow

  set OutputArray electrons

  set DeltaRMax 0.5

  set PTMin 0.5

  set PTRatioMax 9999
}

#################
# Muon efficiency
#################

module Efficiency MuonEfficiency {
  set InputArray MuonFilter/muons
  set OutputArray muons

  # set EfficiencyFormula {efficiency as a function of eta and pt}

  # efficiency formula for muons
  set EfficiencyFormula {
        (energy < 2.0)                                         * (0.000)+
        (energy >= 2.0) * (abs(eta) <= 0.88)                   * (0.99) +
        (energy >= 2.0) * (abs(eta) >0.88 && abs(eta) <= 3.0)  * (0.99) +
        (abs(eta) > 3.0)                                       * (0.000)
  }
}

################
# Muon isolation
################

module Isolation MuonIsolation {
  set CandidateInputArray MuonEfficiency/muons
  set IsolationInputArray EFlowMerger/eflow

  set OutputArray muons

  set DeltaRMax 0.5

  set PTMin 0.5

  set PTRatioMax 9999.
}

###################
# Missing ET merger
###################

module Merger MissingET {
# add InputArray InputArray
  add InputArray EFlowMerger/eflow
  set MomentumOutputArray momentum
}


#####################
# Neutrino Filter
#####################

module PdgCodeFilter NeutrinoFilter {

  set InputArray Delphes/stableParticles
  set OutputArray filteredParticles

  set PTMin 0.0

  add PdgCode {12}
  add PdgCode {14}
  add PdgCode {16}
  add PdgCode {-12}
  add PdgCode {-14}
  add PdgCode {-16}
}

###################################
# Gen Jet finder Durham exclusive
###################################

module FastJetFinder GenJetFinderDurhamN2 {

  set InputArray NeutrinoFilter/filteredParticles
  set OutputArray jets

  # algorithm: 11 ee-durham kT algorithm
  # ref: https://indico.cern.ch/event/1173562/contributions/4929025/attachments/2470068/4237859/2022-06-FCC-jets.pdf
  # to run exclusive njet mode set NJets to int
  # to run exclusive dcut mode set DCut to float
  # if DCut > 0 will run in dcut mode

  set JetAlgorithm 11
  set ExclusiveClustering true
  set NJets 2
  # set DCut 10.0
}

################################
# Jet finder Durham exclusive
################################

module FastJetFinder FastJetFinderDurhamN2 {
#  set InputArray Calorimeter/towers
  set InputArray EFlowMerger/eflow

  set OutputArray jets

  # algorithm: 11 ee-durham kT algorithm
  # ref: https://indico.cern.ch/event/1173562/contributions/4929025/attachments/2470068/4237859/2022-06-FCC-jets.pdf
  # to run exclusive njet mode set NJets to int
  # to run exclusive dcut mode set DCut to float
  # if DCut > 0 will run in dcut mode

  set JetAlgorithm 11
  set ExclusiveClustering true
  set NJets 2
  # set DCut 10.0

}




#####################
# MC truth jet finder
#####################

module FastJetFinder GenJetFinder {
  set InputArray NeutrinoFilter/filteredParticles
  set OutputArray jets

  set JetAlgorithm 10
  set ParameterR 1.5
  set ParameterP -1.0
  set JetPTMin 1.0

}


#########################
# Gen Missing ET merger
########################

module Merger GenMissingET {
# add InputArray InputArray
  add InputArray NeutrinoFilter/filteredParticles
  set MomentumOutputArray momentum
}

############
# Jet finder
############

module FastJetFinder FastJetFinder {
#  set InputArray Calorimeter/towers
  set InputArray EFlowMerger/eflow

  set OutputArray jets

  # algorithm: 1 CDFJetClu, 2 MidPoint, 3 SIScone, 4 kt, 5 Cambridge/Aachen, 6 antikt
  set JetAlgorithm 10
  set ParameterR 1.5
  set ParameterP -1.0
  set JetPTMin 1.0


}

##################
# Jet Energy Scale
##################

module EnergyScale JetEnergyScale {
  set InputArray FastJetFinder/jets
  set OutputArray jets

  # scale formula for jets
  set ScaleFormula {1.00}
}

########################
# Jet Flavor Association
########################

module JetFlavorAssociation JetFlavorAssociation {

  set PartonInputArray Delphes/partons
  set ParticleInputArray Delphes/allParticles
  set ParticleLHEFInputArray Delphes/allParticlesLHEF
  set JetInputArray JetEnergyScale/jets

  set DeltaR 0.5
  set PartonPTMin 1.0
  set PartonEtaMax 3.0
}

###########
# b-tagging
###########

module BTagging BTagging {
  set JetInputArray JetEnergyScale/jets

  set BitNumber 0

  # add EfficiencyFormula {abs(PDG code)} {efficiency formula as a function of eta and pt}

  # default efficiency formula (misidentification rate)
  add EfficiencyFormula {0} {0.005}

  # efficiency formula for c-jets (misidentification rate)
  add EfficiencyFormula {4} {0.01}

  # efficiency formula for b-jets
  add EfficiencyFormula {5} {0.85}
}

###########
# c-tagging
###########

module BTagging CTagging {
  set JetInputArray JetEnergyScale/jets

  set BitNumber 1

  # add EfficiencyFormula {abs(PDG code)} {efficiency formula as a function of eta and pt}

  # default efficiency formula (misidentification rate)
  add EfficiencyFormula {0} {0.01}

  # efficiency formula for c-jets (misidentification rate)
  add EfficiencyFormula {5} {0.05}

  # efficiency formula for b-jets
  add EfficiencyFormula {4} {0.80}
}


#############
# tau-tagging
#############

module TauTagging TauTagging {
  set ParticleInputArray Delphes/allParticles
  set PartonInputArray Delphes/partons
  set JetInputArray JetEnergyScale/jets

  set DeltaR 0.5
  set TauPTMin 1.0
  set TauEtaMax 3.0

  # default efficiency formula (misidentification rate)
  add EfficiencyFormula {0} {0.01}
  # efficiency formula for tau-jets
  add EfficiencyFormula {15} {0.85}
}



##################
# ROOT tree writer
##################

# Tracks, towers and eflow objects are not stored by default in the output.
# If needed (for jet constituent or other studies), uncomment the relevant
# "add Branch ..." lines.

module TreeWriter TreeWriter {
    # add Branch InputArray BranchName BranchClass

    add Branch Delphes/allParticles Particle GenParticle
    add Branch TruthVertexFinder/vertices GenVertex Vertex

    add Branch EFlowTrackMerger/eflowTracks EFlowTrack Track
    add Branch TrackSmearing/tracks Track Track
    add Branch Calorimeter/eflowPhotons EFlowPhoton Tower
    add Branch TimeOfFlightNeutralHadron/eflowNeutralHadrons EFlowNeutralHadron Tower

    add Branch EFlowMerger/eflow ParticleFlowCandidate ParticleFlowCandidate
    add Branch Calorimeter/towers Tower Tower

    add Branch ElectronEfficiency/electrons Electron Electron
    add Branch MuonEfficiency/muons Muon Muon
    add Branch PhotonEfficiency/photons Photon Photon

    add Branch JetEnergyScale/jets Jet Jet
    add Branch MissingET/momentum MissingET MissingET

    add Branch GenJetFinder/jets GenJet Jet
    add Branch GenMissingET/momentum GenMissingET MissingET

    add Branch GenJetFinderDurhamN2/jets GenJetDurhamN2 Jet
    add Branch FastJetFinderDurhamN2/jets JetDurhamN2 Jet

    # add Info InfoName InfoValue
    add Info Bz $B
}
