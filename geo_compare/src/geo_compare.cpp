#include <iostream>
#include <memory>
#include <filesystem>
#include <TFile.h>
#include <TGeoManager.h>

#include "GeoUtils.h"


bool compareNodes(const TGeoNode* node, const PseudoNode* pseudoNode) {

    // Define PseudoNode from given TGeoNode
    PseudoNode rootNode{node};
    PseudoVolume rootVolume{node->GetVolume()};
    rootNode.fVolume = &rootVolume;

    // Use == operators to determine their equality
    return *pseudoNode==rootNode;

}

// Traversal function that builds a pseudoGeometry out of the TGeo Geometry
void copyGeometry(const TGeoNode* node, PseudoVolume* motherPseudoVolume, PseudoManager* geometryManager) {

    // Define the pseudoNode and its pseudoVolume 
    auto* pseudoNode = geometryManager->SpawnNode(node, motherPseudoVolume);
    auto* pseudoVolume = pseudoNode->fVolume;

    // Deepest node reached, no children
    if (!node->GetNodes()) return;
    
    // Loop over chlidren nodes for each node
    for (auto* child : *(node->GetNodes())) {
        copyGeometry(static_cast<const TGeoNode*>(child), pseudoVolume, geometryManager);
    }

}

PseudoNode* findNodePartner(const TGeoNode* node, std::vector<PseudoNode*>& pseudoNeighbours) {

    PseudoNode* neighbour{nullptr};

    // Search from the end so that erasing a match is O(1)
    for (auto it = pseudoNeighbours.rbegin(); it != pseudoNeighbours.rend(); ++it) {
        if (compareNodes(node, *it)) {
            neighbour = *it;
            pseudoNeighbours.erase((it+1).base());    // base() returns the iterator to 1 after the logical element reverse_iterator (it) is looking at (rbegin ~ end-1)
            break;
        }
    }

    if (!neighbour) throw std::runtime_error("No node found for <<" + static_cast<std::string>(node->GetName()) + ">>. Geometries do not match!");

    return neighbour;

}

// Traversal function that builds compares the TGeo Geometry to pseudoGeometry 
void auditGeometry(const TGeoNode* node, PseudoVolume* motherPseudoVolume, PseudoManager* geometryManager) {

    // Compare equality of Top Node and Top PseudoNode 
    if (!motherPseudoVolume && !compareNodes(node, geometryManager->fTopLevelNode)) throw std::runtime_error("<<Top level nodes are not equivalent>>");

    // Find pseudoNode that matches the TGeoNode among the neighbouring PseudoNodes (unless at top node, i.e. no motherPseudoVolume)
    auto* pseudoNode = motherPseudoVolume ? findNodePartner(node, motherPseudoVolume->fNodesRemaining) : geometryManager->fTopLevelNode;
    auto* pseudoVolume = pseudoNode->fVolume;

    // Deepest node reached, no children
    if (!node->GetNodes()) return;

    for (auto* child : *(node->GetNodes())) {
        auditGeometry(static_cast<const TGeoNode*>(child), pseudoVolume, geometryManager);
    }

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
     
    // sourceFile -> file from which we build our geometry
    // referenceFile -> file whose geometry we compare against

    // Trivially parse input
    if (argc != 3 ||
            std::filesystem::path(argv[1]).extension() != ".root" ||
            std::filesystem::path(argv[2]).extension() != ".root") {
        std::cerr << "Usage: geoCompare <file1.root> <file2.root>\n";
        return EXIT_FAILURE;
    }
    

    std::unique_ptr<TFile> inputFile(TFile::Open(argv[1]));
    auto* inputGeoManager = static_cast<TGeoManager*>(inputFile->Get("default"));
    auto* inputTopNode = inputGeoManager->GetTopNode();


    // PseudoGeometry manager (will hold "copy" of input geometry)
    PseudoManager geometryManager{};

    printBanner("Building Geometry", "38;5;33");

    copyGeometry(inputTopNode, nullptr, &geometryManager);
    geometryManager.SyncNodesRemaining();

    printBanner("Finished building Geometry", "32");
    std::cout << "There are " << geometryManager.fNodes.size() << " nodes" << std::endl;
    std::cout << "There are " << geometryManager.fVolumes.size() << " volumes" << std::endl;
    std::cout << "TGeo -> There are " << inputGeoManager->GetNNodes() << " nodes" << std::endl;

    // Careful: this replaces input file's TGeoManager!
    std::unique_ptr<TFile> referenceFile(TFile::Open(argv[2]));
    auto* referenceGeoManager = static_cast<TGeoManager*>(referenceFile->Get("default"));
    auto* referenceTopNode = referenceGeoManager->GetTopNode();
    
    printBanner("Auditing Geometry", "38;5;33");

    // Auditing the same geo we just built. Show work trivially. Should repeat for copied file.
    auditGeometry(referenceTopNode, nullptr, &geometryManager);

    printBanner("Finished auditing Geometry", "32");

    return EXIT_SUCCESS;
}




