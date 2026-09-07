#include "GeoUtils.h"
#include <TGeoShape.h>
#include <TGeoTube.h>
#include <TGeoShapeAssembly.h>

// PseudoShape constructor
PseudoShape::PseudoShape(TGeoShape* shape) {
    // : fType(typeid(*shape).name()) {
    // : fType(TGeoShape->IsA()->GetName()) {

    if (!shape) {throw std::invalid_argument("<<Shape is NULL>>");}
    fType = typeid(*shape).name();

    if (typeid(*shape) == typeid(TGeoTubeSeg)) {

        auto* tubeSeg = static_cast<const TGeoTubeSeg*>(shape);
        fParams = {tubeSeg->GetPhi1(), tubeSeg->GetPhi2(), tubeSeg->GetRmin(), tubeSeg->GetRmax(), tubeSeg->GetDz()};

    } else if (typeid(*shape) == typeid(TGeoBBox)) { 

        auto* bBox = static_cast<const TGeoBBox*>(shape);
        fParams = {bBox->GetDX(), bBox->GetDY(), bBox->GetDZ()};

    } else if (typeid(*shape) == typeid(TGeoShapeAssembly)) { 

        // Note here we are only comparing the bounding box, not the actual shapes within the assembly
        auto* shapeAssembly = static_cast<const TGeoShapeAssembly*>(shape);
        fParams = {shapeAssembly->GetDX(), shapeAssembly->GetDY(), shapeAssembly->GetDZ()};

    } else {

        throw std::invalid_argument("Unknown shape <<" + static_cast<std::string>(typeid(*shape).name()) + ">>");

    }
}
    


// PseudoVolume constructor
// PseudoVolume::PseudoVolume(const char* name, std::vector<PseudoNode*> nodes, const PseudoVolume* mother, const TGeoShape* shape, const TGeoMedium* medium)
//     : fName{name}, fNodes{nodes}, fMother{mother} {
PseudoVolume::PseudoVolume(const char* name, std::vector<PseudoNode*> nodes, TGeoShape* shape, TGeoMedium* medium)
    : fName{name}, fNodes{nodes}, fShape{shape} {

        // if (!shape) {throw std::invalid_argument("<<Shape is NULL>>");}
        // fShape = PseudoShape(shape);

        if (!medium) {throw std::invalid_argument("<<Medium is NULL>>");}
        fMedium = medium->GetName();

}

// PseudoNode contructor
PseudoNode::PseudoNode(const char* name, PseudoVolume* volume, PseudoVolume* mother)
    : fName{name}, fVolume{volume}, fMother{mother} {}



// PseudoManager contructor
PseudoManager::PseudoManager(TGeoVolume* topLevelVolume) {

        if (!topLevelVolume) {throw std::invalid_argument("<<Top level volume is NULL>>");}
        auto* name = topLevelVolume->GetName();
        // auto nodes = std::vector<std::unique_ptr<PseudoNode>>{};
        auto nodes = std::vector<PseudoNode*>{};

        // auto* geoNodes = topLevelVolume->GetNodes();
        // for(auto* geoObject : *geoNodes){
        //     auto* geoNode = static_cast<TGeoNode*>(geoObject);
        //     PseudoNode
        // }
        //     topLevelVolume->GetNodes(); // Wrong type!! Need to read out nodes!! 
        

        auto* shape = topLevelVolume->GetShape(); 
        auto* medium = topLevelVolume->GetMedium(); 
        fTopLevelVolume = PseudoVolume(name, nodes, shape, medium);
        fName = name;
        // fNodes = std::vector<PseudoNodes>{};
        // fVolumes = std::vector<PseudoVolumes>{};

}

// Note that node => the TGeoNode this PseudoNode replicates, mother => volume within which the node exists, volume => volume to which the transformation matrix pertains
void PseudoManager::SpawnNode(TGeoNode* node, PseudoVolume* mother, PseudoVolume* volume) {

    // Create PseudoNode 
    fNodes.emplace_back(std::make_unique<PseudoNode>(
                node->GetName(),
                volume,
                mother
                ));
    
    // Add it to mother's node vector
    mother->fNodes.push_back(fNodes.back().get());

}


// Note that volume => the TGeoVolume this PseudoVolume replicates, node => node that points to this volume
// No attempt is made to "reuse" volumes, each node gets a "fresh" copy of the volume (to be revised if mem bloats)
void PseudoManager::SpawnVolume(TGeoVolume* volume, PseudoNode* node) {

    // Create PseudoVolume
    fVolumes.emplace_back(std::make_unique<PseudoVolume>(
                volume->GetName(),
                std::vector<PseudoNode*>{},
                volume->GetShape(),
                volume->GetMedium()
                ));

    // Link it to its owning node
    node->fVolume = fVolumes.back().get();
    
    
}






