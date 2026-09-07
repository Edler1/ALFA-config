#include <iostream>
#include <memory>
#include <TFile.h>
#include <TGeoManager.h>

#include "GeoUtils.h"





// Initial debugging traversal function
void copyGeometry(TGeoNode* node){
    
    std::cout << " :::::::::::::: starting node traversal ::::::::::::::" << std::endl; 


    // Here we can define the nodes and extract relevant properties
    
    // Here we could already link the node to its parent (which must already be defined, since we start recursion on the top level this is a given)
    // We must all fill the fMother, so we can find the "top level volume"
    // Top level volume is created by the constructor of the geometry manager, and nodes are made to point to it if their mother is the world volume (in TGeo)
    // this check should be made when fMother is filled? or is it in the "normal" logic branch


    if (!node) return;
    // Deepest node reached, no children
    if (!node->GetNodes() || node->GetNodes()->GetEntriesFast() == 0) return;
    

    for (auto* child : *(node->GetNodes())){

        auto* childNode = static_cast<TGeoNode*>(child);
        std::cout << "childNode -> " << childNode->GetName() << std::endl;
        
        // Define node we wish to descend into further
        copyGeometry(childNode);

    }



}

std::vector<TGeoNode*> constructChildrenVector(TObjArray* children){
    
    // Start by defining vector
    std::vector<TGeoNode*> childrenVector;

    // No need to check for children (nullptr), since check already done before calling constructChildrenVector in traverseNode

    // Find number of children
    const std::size_t nChildren = children->GetEntriesFast();
    childrenVector.reserve(nChildren);


    // Reverse children to prevent shifting vector if first element is popped (i.e. if nodes in reference and test files have same ordering)
    for (std::size_t i{nChildren}; i; --i){
        childrenVector.push_back(static_cast<TGeoNode*>(children->UncheckedAt(i-1)));
    }

    return childrenVector;

}

bool compareNodes(TGeoNode* node_a, TGeoNode* node_b){
    return true;
}

TGeoNode* findNodePartner(TGeoNode* node_a, std::vector<TGeoNode*>& neighbours_b){

    TGeoNode* neighbour{nullptr};

    // Here we must iterate over the nodes in neighbours_b and return the matching node (and delete it from the vector)
    for (auto it = neighbours_b.begin(); it != neighbours_b.end(); ++it){
        if (compareNodes(node_a, *it)){
            neighbour = *it;
            neighbours_b.erase(it);
            std::cout << "--Matching node [" << node_a->GetName() << "] = [" << node_a->GetName() << "]--" << std::endl;
            break;
        }
    }

    return neighbour;

}

// Actual traversal function
void traverseNode(TGeoNode* node_a, std::vector<TGeoNode*>& neighbours_b){
    

    // std::cout << " :::::::::::::: starting traversal ::::::::::::::" << std::endl; 
    
    // Check should be per-node, and could thus live here?
    // Careful with above though, since it stops on nodes with no children! Those need to be checked too!
    // Moved it below check, does that work?

    auto* node_b = findNodePartner(node_a, neighbours_b);
    if (!node_b){
        std::cout << "No partner found in test geometry!" << std::endl;
        // Should halt execution here...
    }

    if (!node_a) return;
    // Deepest node reached, no children
    // if (!node_a->GetNodes() || node_a->GetNodes()->GetEntriesFast() == 0) return;
    if (!node_a->GetNodes()) return;
    
    // auto* nodeVolume = node->GetVolume();

    // for (auto* child : *(node->GetNodes())){

    // auto* nodes = node_a->GetNodes();
    // const int count = nodes->GetEntriesFast(); // Or GetEntries()

    // Constructing vector to hold the children of b
    auto childrenVector_b = constructChildrenVector(node_b->GetNodes());
    for (auto* child : *(node_a->GetNodes())){

        auto* childNode = static_cast<TGeoNode*>(child);
        std::cout << "childNode -> " << childNode->GetName() << std::endl;

        // auto* child_b = findNodePartner(child, childrenVector_b); 
        // auto grandChildrenVector_b = constructChildrenVector(child_b->GetNodes());
        
        // Define node we wish to descend into further
        traverseNode(childNode, childrenVector_b);

    }


}

// void compareNodeEquivalence(TGeoNode* a_child, TGeoNode* b_children){
//
//     if (!node) return;
//     // Deepest node reached, no children
//     if (!node->GetNodes() || node->GetNodes()->GetEntriesFast() == 0) return;
//
//     std::cout << " :::::::::::::: starting traversal ::::::::::::::" << std::endl; 
//
//     // auto* nodeVolume = node->GetVolume();
//
    // auto* nodes = node_a->GetNodes();
    // const int count = nodes->GetEntriesFast(); // Or GetEntries()
                                               //
//     // Fill b's node vector in reverse, so matching entries can be popped
//     for (size_t i = count - 1; i; --i) {
//         auto* child = static_cast<TGeoNode*>(nodes->UncheckedAt(i));
//
//         auto* childNode = static_cast<TGeoNode*>(child);
//         std::cout << "childNode -> " << childNode->GetName() << std::endl;
//
//         // Define node we wish to descend into further
//         traverseNode(childNode);
//
//     }
//
//
// }


int main(int argc, char** argv){


    // ToDo:
    // 0. Should check execution actually does what we want
    //  a. Does the reversing of vector work, and does it match the first node consistently?
    // 1. Guard against passing wrong arguments?
    // 2. Should be some way to pass flags? Do we care?
    // 3. Guard loading of objects against nulls
    // 4. Can aggressively enforce consts

    // Hardcoded filepath + names
    // constexpr auto fpath = "~/Software/official/k4geo/";
    std::string fpath = "~/Software/official/k4geo/";
    std::string f1 = fpath + "ALFA_debug.root";
    std::string f2 = fpath + "ALFA_debug_copy.root";

    // Start by reading .root geometry file
    // auto fileName = argv[1];
    // std::unique_ptr<TFile> geoFile(TFile::Open(argv[1]));
    
    std::unique_ptr<TFile> geoFile_1(TFile::Open(f1.c_str()));
    // std::unique_ptr<TFile> geoFile_2(TFile::Open(f2.c_str()));
    
    // Attach TGeoManager
    auto* geoManager_1 = static_cast<TGeoManager*>(geoFile_1->Get("default"));
    
    std::cout << "manager 1: " << geoManager_1 << '\n';
    std::cout << "world 1:   " << geoManager_1->GetTopVolume() << '\n';

    // auto* geoManager_2 = static_cast<TGeoManager*>(geoFile_2->Get("default"));

    // std::cout << "manager 1: " << geoManager_1 << '\n';
    // std::cout << "world 1:   " << geoManager_1->GetTopVolume() << '\n';


    // std::cout << "manager 2: " << geoManager_2 << '\n';
    // std::cout << "world 2:   " << geoManager_2->GetTopVolume() << '\n';

    // Find top (world) volume
    auto* worldVolume_1 = geoManager_1->GetTopVolume();
    // auto* worldVolume_2 = geoManager_2->GetTopVolume();

    worldVolume_1->GetNodes()->Dump();

    auto* nodes = worldVolume_1->GetNodes();
    auto* firstNode = nodes->At(0);
    auto* firstNode_1 = static_cast<TGeoNode*>(firstNode);

    // Select first node, need to cast since GetNodes returns a TObjArray
    // auto* firstNode_1 = static_cast<TGeoNode*>(worldVolume_1->GetNodes()->At(0));
    // auto* firstNode_2 = static_cast<TGeoNode*>(worldVolume_2->GetNodes()->At(0));

    // // Select its volume
    // auto* firstNode_volume = firstNode->GetVolume();

    // Descend further into first layer
    auto* layerNode_1 = static_cast<TGeoNode*>(firstNode_1->GetNodes()->At(0));
    // auto* layerNode_2 = static_cast<TGeoNode*>(firstNode_2->GetNodes()->At(0));

    // worldVolume->ls();
    std::cout << " :::::::::::::: first node ::::::::::::::" << std::endl; 
    // layerNode->Dump();

    // auto layerNode_2Vector = constructChildrenVector(layerNode_2->GetNodes());
    // traverseNode(layerNode_1, layerNode_2Vector);


    // Testing PseudoShape
    auto* testShape = layerNode_1->GetVolume()->GetShape();
    std::cout << " :::::::::::::: Dumping Shape ::::::::::::::" << std::endl; 
    testShape->Dump();
    auto testPseudoShape = PseudoShape(testShape);
    std::cout << " :::::::::::::: Dumping (Pseudo)Shape ::::::::::::::" << std::endl; 
    std::cout<<testPseudoShape.fType<<std::endl;
    for (auto param : testPseudoShape.fParams){
        std::cout<<param<<std::endl;
    }

    // Testing PseudoVolume
    auto* testVolume = layerNode_1->GetVolume();
    std::cout << " :::::::::::::: Dumping Volume ::::::::::::::" << std::endl; 
    testVolume->Dump();
    auto testPseudoVolume = PseudoVolume(testVolume->GetName(), std::vector<PseudoNode*>{}, testVolume->GetShape(), testVolume->GetMedium());
    std::cout << " :::::::::::::: Dumping (Pseudo)Volume ::::::::::::::" << std::endl; 
    std::cout<<testPseudoVolume.fName<<std::endl;
    for (auto param : testPseudoVolume.fNodes){
        std::cout<<param<<std::endl;
    }
    std::cout<<testPseudoVolume.fMedium<<std::endl;
    std::cout<<testPseudoVolume.fShape.fType<<std::endl;
    for (auto param : testPseudoVolume.fShape.fParams){
        std::cout<<param<<std::endl;
    }
    
    // Testing PseudoNode
    auto* testNode = static_cast<TGeoNode*>(layerNode_1->GetVolume()->GetNodes()->At(0));
    std::cout << " :::::::::::::: Dumping Node ::::::::::::::" << std::endl; 
    testNode->Dump();
    auto testPseudoNode = PseudoNode(testNode->GetName(), &testPseudoVolume, nullptr); 
    std::cout << " :::::::::::::: Dumping (Pseudo)Node ::::::::::::::" << std::endl; 
    std::cout<<testPseudoNode.fName<<std::endl;
    for (auto param : testPseudoNode.fVolume->fNodes){
        std::cout<<param<<std::endl;
    }
    std::cout<<testPseudoNode.fVolume->fMedium<<std::endl;
    std::cout<<testPseudoNode.fVolume->fShape.fType<<std::endl;
    for (auto param : testPseudoNode.fVolume->fShape.fParams){
        std::cout<<param<<std::endl;
    }

    // Testing PseudoManager
    auto* testTopVolume = geoManager_1->GetTopVolume();
    std::cout << " :::::::::::::: Dumping Top Volume ::::::::::::::" << std::endl; 
    testTopVolume->Dump();
    testTopVolume->GetShape()->Dump();

    auto testPseudoManager = PseudoManager(geoManager_1->GetTopVolume());
    std::cout << " :::::::::::::: Dumping (Pseudo)Manager ::::::::::::::" << std::endl; 
    std::cout<<testPseudoManager.fName<<std::endl;
    std::cout<<testPseudoManager.fTopLevelVolume.fShape.fType<<std::endl;
    for (auto param : testPseudoManager.fTopLevelVolume.fShape.fParams){
        std::cout<<param<<std::endl;
    }

    std::cout << " ::::::::::::::<<Starting Traversal>>::::::::::::::" << std::endl; 

    copyGeometry(layerNode_1);

    // std::cout << "script skeleton" << std::endl;
    // std::cout << geoManager_1->GetVisLevel() << std::endl;
    
    return EXIT_SUCCESS;
}



