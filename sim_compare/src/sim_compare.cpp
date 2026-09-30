#include <iostream>
#include <podio/Reader.h>
#include <podio/ROOTReader.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/TrackerHitPlaneCollection.h>
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


std::vector<std::pair<std::string, std::string>> queryCollections(podio::Reader& sourceReader, podio::Reader& referenceReader, const std::vector<std::string>& userCollections = {}) {

    // Check files are non-empty and event number matches
    if (sourceReader.getEvents() && sourceReader.getEvents() != referenceReader.getEvents()) throw std::runtime_error("<<Events do not match for source and reference files!>>");

    // Query first event for reading collections
    podio::Frame sourceEvent = sourceReader.readEvent(0);
    podio::Frame referenceEvent = referenceReader.readEvent(0);

    std::vector<std::pair<std::string, std::string>> collections{};

    const std::vector<std::string> nameCollections = (userCollections.empty()) ? sourceEvent.getAvailableCollections() : userCollections;
    for (const auto& collection : nameCollections) {

        const podio::CollectionBase* sourceCollectionBase = sourceEvent.get(collection);
        if (!sourceCollectionBase) throw std::runtime_error("<<Collection \"" + collection + "\" is NULL>>");

        const podio::CollectionBase* referenceCollectionBase = referenceEvent.get(collection);
        if (!referenceCollectionBase) throw std::runtime_error("<<Collection \"" + collection + "\" is NULL>>");

        const std::string_view type = sourceCollectionBase->getTypeName();
        if (referenceCollectionBase->getTypeName() != type) throw std::runtime_error("<source and reference collections \"" + collection +"\" type do not match!>>");

        collections.emplace_back(collection, type);
    }

    return collections;

}


// Hit comparison for SimTrackerHits
bool compareHit(const edm4hep::SimTrackerHit& sourceHit, const edm4hep::SimTrackerHit& referenceHit) {

    if (sourceHit.getCellID() != referenceHit.getCellID()) return false;

    const edm4hep::Vector3d& p1 = sourceHit.getPosition();
    const edm4hep::Vector3d& p2 = referenceHit.getPosition();
    if (std::abs(p1[0] - p2[0]) > TOLERANCE || 
            std::abs(p1[1] - p2[1]) > TOLERANCE || 
            std::abs(p1[2] - p2[2]) > TOLERANCE) return false;

    const edm4hep::Vector3f& m1 = sourceHit.getMomentum();
    const edm4hep::Vector3f& m2 = referenceHit.getMomentum();
    if (std::abs(m1[0] - m2[0]) > TOLERANCE || 
            std::abs(m1[1] - m2[1]) > TOLERANCE || 
            std::abs(m1[2] - m2[2]) > TOLERANCE) return false;

    if (std::abs(sourceHit.getEDep() - referenceHit.getEDep()) > TOLERANCE) return false;
    return true;
}


// Hit comparison for TrackerHitPlaneCollection
bool compareHit(const edm4hep::TrackerHitPlane& sourceHit, const edm4hep::TrackerHitPlane& referenceHit) {

    if (sourceHit.getCellID() != referenceHit.getCellID()) return false;

    const edm4hep::Vector3d& p1 = sourceHit.getPosition();
    const edm4hep::Vector3d& p2 = referenceHit.getPosition();
    if (std::abs(p1[0] - p2[0]) > TOLERANCE || 
            std::abs(p1[1] - p2[1]) > TOLERANCE || 
            std::abs(p1[2] - p2[2]) > TOLERANCE) return false;

    const edm4hep::Vector2f& u1 = sourceHit.getU();
    const edm4hep::Vector2f& u2 = referenceHit.getU();
    if (std::abs(u1[0] - u2[0]) > TOLERANCE || 
            std::abs(u1[1] - u2[1]) > TOLERANCE) return false;

    const edm4hep::Vector2f& v1 = sourceHit.getV();
    const edm4hep::Vector2f& v2 = referenceHit.getV();
    if (std::abs(v1[0] - v2[0]) > TOLERANCE || 
            std::abs(v1[1] - v2[1]) > TOLERANCE) return false;

    if (std::abs(sourceHit.getEDep() - referenceHit.getEDep()) > TOLERANCE) return false;
    return true;
}

template <typename T>
bool auditHits(const T& sourceHits, const T& referenceHits) {

    if (sourceHits.size() != referenceHits.size()) return false;
    // If no hits in event, they trivially match
    if (sourceHits.size() == 0) return true;


    for (std::size_t i{0}; i < sourceHits.size(); ++i) {
        if (!compareHit(sourceHits[i], referenceHits[i])) return false;
    }

    return true;
}



void printBanner(std::string message, std::string colorCode = "0") {
    message = "<<" +  message + ">>";
    if (message.size() > 60) {
        std::cout << message << std::endl;
        return;
    }
    std::size_t nPadding = 30 - message.size() / 2;   
    std::cout << "\033[" + colorCode + "m" << std::string(nPadding, ':') + message + std::string(nPadding + !(message.size() % 2), ':') << "\033[0m" << std::endl;
}



int main(int argc, char** argv) {



    // sourceFile -> file which we wish to compare
    // referenceFile -> file whose simulation we compare against
    //
    // Collection names can be specified for SimTrackerHitCollection using "-collection <collectionName>"

    std::vector<std::string> userCollections{}; // Collections to be compared between the two sim files. "OTBarCollection" as ALFA default

    
    // Trivially parse input
    if (argc < 3 || 
            argc % 2 == 0 ||
            std::filesystem::path(argv[1]).extension() != ".root" ||
            std::filesystem::path(argv[2]).extension() != ".root") {
        std::cerr << "Usage: simCompare <file1.root> <file2.root>\n";
            std::cerr << "(optionally) Collections specified: simCompare <file1.root> <file2.root> -collection <collectionName1> -collection <collectionName2>\n";
        return EXIT_FAILURE;
    }
    for (int i{3}; i < argc; i+=2) {
        if (std::string_view(argv[i]) != "-collection") {
            std::cerr << "Usage: simCompare <file1.root> <file2.root>\n";
            std::cerr << "(optionally) Collections specified: simCompare <file1.root> <file2.root> -collection <collectionName1> -collection <collectionName2>\n";
            return EXIT_FAILURE;
        } else {
            userCollections.push_back(argv[i + 1]);
        }
    }
    if (userCollections.empty()) {
        // Maybe make default behaviour simply check all collections? use pEvent and getAvailableCollections
        std::cerr << "No collections specified, defaulting to  all collections.\n";
    }
    



    auto sourceReader = podio::makeReader(argv[1]);
    [[maybe_unused]] const std::size_t nEventsSource = sourceReader.getEvents(); // Necesssary for forcing init of reader (bug)

    auto referenceReader = podio::makeReader(argv[2]);
    [[maybe_unused]] const std::size_t nEventsReference = referenceReader.getEvents();






    std::vector<std::pair<std::string, std::string>> collections = queryCollections(sourceReader, referenceReader, userCollections);





    printBanner("Starting <<SimTrackerHits>> comparison", "38;5;33");


    bool pEvent = false;
    for (size_t i = 0; i < sourceReader.getEvents(); ++i) {



        const podio::Frame sourceEvent = sourceReader.readEvent(i);
        const podio::Frame referenceEvent = referenceReader.readEvent(i);
        // const auto& mcParticles = event.get<edm4hep::MCParticleCollection>("MCParticles");


        


        


        // for (const auto& [collection, type] : collections) {
        for (auto it = collections.begin(); it != collections.end();) {

            const std::string& collection = it->first;
            const std::string& type = it->second;


            const podio::CollectionBase* sourceCollectionBase = sourceEvent.get(collection);
            const podio::CollectionBase* referenceCollectionBase = referenceEvent.get(collection);

            


            if (type == "edm4hep::SimTrackerHitCollection") {
                if (!auditHits(static_cast<const edm4hep::SimTrackerHitCollection&>(*sourceCollectionBase), static_cast<const edm4hep::SimTrackerHitCollection&>(*referenceCollectionBase))) throw std::runtime_error("<<Audit failed!>>");
                ++it;
            } else if (type == "edm4hep::TrackerHitPlaneCollection") {
                if (!auditHits(static_cast<const edm4hep::TrackerHitPlaneCollection&>(*sourceCollectionBase), static_cast<const edm4hep::TrackerHitPlaneCollection&>(*referenceCollectionBase))) throw std::runtime_error("<<Audit failed!>>");
                ++it;
            } else {
                std::cerr << "Collection \"" << collection << "\" of type <" << type << "> is skipped (type not implemented).\n";
                collections.erase(it);
            }



            // Print CellID of first event for reference
            if (!pEvent && sourceCollectionBase->size() > 0 && type == "edm4hep::SimTrackerHitCollection") {

                const auto& sourceHit = static_cast<const edm4hep::SimTrackerHitCollection&>(*sourceCollectionBase)[0];
                const auto& referenceHit = static_cast<const edm4hep::SimTrackerHitCollection&>(*referenceCollectionBase)[0];

                std::cout << "Event " << i << " (example output):" << std::endl;
                std::cout << "------------------" << std::endl;
                std::cout << "CellID -> " << sourceHit.getCellID() << " vs " << referenceHit.getCellID() << std::endl;
                std::cout << "Position -> (" << sourceHit.getPosition() << ") vs (" << referenceHit.getPosition() << ")" << std::endl;
                std::cout << "------------------" << std::endl;
                pEvent = true;


            }

        }
            

    }
    printBanner("Finished comparison", "32");

    return EXIT_SUCCESS;

}

