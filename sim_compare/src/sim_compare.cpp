#include <iostream>
#include <podio/Reader.h>
#include <podio/ROOTReader.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <TFile.h>
#include <TH1D.h>
#include <Math/Vector3D.h>
#include <Math/Point3D.h>
#include <TMatrixD.h>
#include <TMatrixDEigen.h>
#include <format>
#include "DD4hep/DD4hepUnits.h"


// #include <memory>
#include <filesystem>

constexpr float TOLERANCE = 1E-6;


bool auditHits(const auto& sourceHits, const auto& referenceHits) {

    if (sourceHits.size() != referenceHits.size()) return false;
    // If no hits in event, they trivially match
    if (sourceHits.size() == 0) return true;


    for (std::size_t i{0}; i < sourceHits.size(); ++i) {
        const auto& src = sourceHits[i];
        const auto& ref = referenceHits[i];

        if (src.getCellID() != ref.getCellID()) return false;

        // Cache 3D vectors to avoid repeated member function calls
        const auto& p1 = src.getPosition();
        const auto& p2 = ref.getPosition();
        if (std::abs(p1[0] - p2[0]) > TOLERANCE || 
            std::abs(p1[1] - p2[1]) > TOLERANCE || 
            std::abs(p1[2] - p2[2]) > TOLERANCE) return false;

        const auto& m1 = src.getMomentum();
        const auto& m2 = ref.getMomentum();
        if (std::abs(m1[0] - m2[0]) > TOLERANCE || 
            std::abs(m1[1] - m2[1]) > TOLERANCE || 
            std::abs(m1[2] - m2[2]) > TOLERANCE) return false;

        if (std::abs(src.getEDep() - ref.getEDep()) > TOLERANCE) return false;
    }
    return true;
}


void printBanner(std::string message, std::string colorCode = "0") {
    message = "<<" +  message + ">>";
    if (message.size() > 60) {
        std::cout << message << std::endl;
        return;
    }
    auto nPadding = 30 - message.size() / 2;   // std::size_t
    std::cout << "\033[" + colorCode + "m" << std::string(nPadding, ':') + message + std::string(nPadding + !(message.size() % 2), ':') << "\033[0m" << std::endl;
}



int main(int argc, char** argv) {



    // sourceFile -> file which we wish to compare
    // referenceFile -> file whose simulation we compare against

    // Trivially parse input
    if (argc != 3 ||
            std::filesystem::path(argv[1]).extension() != ".root" ||
            std::filesystem::path(argv[2]).extension() != ".root") {
        std::cerr << "Usage: simCompare <file1.root> <file2.root>\n";
        return EXIT_FAILURE;
    }

    auto sourceReader = podio::makeReader(argv[1]);
    auto referenceReader = podio::makeReader(argv[2]);


    printBanner("Starting <<SimTrackerHits>> comparison", "38;5;33");


    if (sourceReader.getEvents() != referenceReader.getEvents()) std::cerr << "<<Events do not match for source and reference files!>>\n";


    bool pEvent = false;
    for (size_t i = 0; i < sourceReader.getEvents(); ++i) {

        auto sourceEvent = sourceReader.readEvent(i);
        auto referenceEvent = referenceReader.readEvent(i);
        // const auto& mcParticles = event.get<edm4hep::MCParticleCollection>("MCParticles");

        const auto& sourceSimTrackerHits = sourceEvent.get<edm4hep::SimTrackerHitCollection>("OTBarCollection");
        const auto& referenceSimTrackerHits = referenceEvent.get<edm4hep::SimTrackerHitCollection>("OTBarCollection");

        // std::cout << "--processing event " << std::to_string(i) << "--" << std::endl;
        // std::cout << sourceSimTrackerHits << std::endl;
        // std::cout << referenceSimTrackerHits << std::endl;
            

        if (!auditHits(sourceSimTrackerHits, referenceSimTrackerHits)) throw std::runtime_error("<<Audit failed!>>");

        // Print CellID of first event for reference
        if (!pEvent && sourceSimTrackerHits.size() > 0) {
        // if (sourceSimTrackerHits.size() > 0) {

            const auto& sourceHit = sourceSimTrackerHits[0];
            const auto& referenceHit = referenceSimTrackerHits[0];

            std::cout << "Event " << i << " (example output):" << std::endl;
            std::cout << "------------------" << std::endl;
            std::cout << "CellID -> " << sourceHit.getCellID() << " vs " << referenceHit.getCellID() << std::endl;
            std::cout << "Position -> (" << sourceHit.getPosition() << ") vs (" << referenceHit.getPosition() << ")" << std::endl;
            std::cout << "------------------" << std::endl;
            pEvent = true;


        }
            

        // std::cout << "event leaded, simTrackerHits -> " << sourceSimTrackerHits << std::endl;
    }
    printBanner("Finished comparison", "32");

    return EXIT_SUCCESS;

}

