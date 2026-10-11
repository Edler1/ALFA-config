#
# Copyright (c) 2020-2024 Key4hep-Project.
#
# This file is part of Key4hep.
# See https://key4hep.github.io/key4hep-doc/ for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
from Gaudi.Configuration import INFO, DEBUG
from k4FWCore import ApplicationMgr, IOSvc
from Configurables import EventDataSvc, DDPlanarDigi, GeoSvc, UniqueIDGenSvc,  RootHistSvc, Gaudi__Histograming__Sink__Root as RootHistoSink
import os

id_service = UniqueIDGenSvc("UniqueIDGenSvc")

geoservice = GeoSvc("GeoSvc")
# geoservice.detectors = [os.environ["K4GEO"]+"/FCCee/ALFA/compact/ALFA_o1_v00/ALFA_o1_v00.xml"]
geoservice.detectors = ["/home/edi/Software/ALFA-config/FCCee/ALFA/compact/ALFA_o1_v00/ALFA_o1_v00.xml"]
geoservice.OutputLevel = INFO
geoservice.EnableGeant4Geo = False

otbar_digi = DDPlanarDigi("OTBardigitizer")
otbar_digi.SubDetectorName = "OTBar"    
otbar_digi.IsStrip = False
#Smearing resolution in mm applied along the two local directions on eachs ensor plane. 
otbar_digi.ResolutionU = [0.003]   # mm
otbar_digi.ResolutionV = [0.003]
otbar_digi.SimTrackHitCollectionName = ["OTBarCollection"] #input collection name; change it according to the root file 
#output container storing the relation btw each digi hit and its corresponsding sim hit so as to trace back which MC hit produced the digi hit 
otbar_digi.SimTrkHitRelCollection = ["OTBarHitRelations"]
otbar_digi.TrackerHitCollectionName = ["OTBarDigiHits"] # container for new digi hits after smearing 
otbar_digi.ForceHitsOntoSurface = True

vtx_digi = DDPlanarDigi("VTXdigitizer")
vtx_digi.SubDetectorName = "VTX"    
vtx_digi.IsStrip = False
#Smearing resolution in mm applied along the two local directions on eachs ensor plane. 
vtx_digi.ResolutionU = [0.003]   # mm
vtx_digi.ResolutionV = [0.003]
vtx_digi.SimTrackHitCollectionName = ["VTXCollection"] #input collection name; change it according to the root file 
#output container storing the relation btw each digi hit and its corresponsding sim hit so as to trace back which MC hit produced the digi hit 
vtx_digi.SimTrkHitRelCollection = ["VTXHitRelations"]
vtx_digi.TrackerHitCollectionName = ["VTXDigiHits"] # container for new digi hits after smearing 
vtx_digi.ForceHitsOntoSurface = True

iosvc = IOSvc()
iosvc.Input = "alfaTrackerSimulation.root"
iosvc.Output = "alfaTrackerDigitization.root"

#Histogramming services
hps = RootHistSvc("HistogramPersistencySvc")
root_hist_svc = RootHistoSink("RootHistoSink") #writes histos diretcly to a root file : ddplanardigi_hist.root
root_hist_svc.FileName = "ddplanardigi_hist.root"

#TopAlg : algo executed once per event
ApplicationMgr(TopAlg=[otbar_digi, vtx_digi],
               EvtSel="NONE",
               EvtMax=-1,
               ExtSvc=[EventDataSvc("EventDataSvc"), root_hist_svc],
               OutputLevel=INFO,
               )
