#include "GeoUtils.h"
#include <TGeoShape.h>
#include <TGeoTube.h>
#include <TGeoCone.h>
#include <TGeoPgon.h>
#include <TGeoArb8.h>
#include <TGeoShapeAssembly.h>
#include <TGeoCompositeShape.h>

#include <algorithm>

// PseudoShape constructor
PseudoShape::PseudoShape(const TGeoShape* shape) {

    if (!shape) throw std::invalid_argument("<<Shape is NULL>>");
    fType = typeid(*shape).name();

    if (typeid(*shape) == typeid(TGeoTube)) {

        auto* tube = static_cast<const TGeoTube*>(shape);
        fParams = {tube->GetRmin(), tube->GetRmax(), tube->GetDz()};

    } else if (typeid(*shape) == typeid(TGeoTubeSeg)) { 

        auto* tubeSeg = static_cast<const TGeoTubeSeg*>(shape);
        fParams = {tubeSeg->GetPhi1(), tubeSeg->GetPhi2(), tubeSeg->GetRmin(), tubeSeg->GetRmax(), tubeSeg->GetDz()};

    } else if (typeid(*shape) == typeid(TGeoConeSeg)) { 

        auto* coneSeg = static_cast<const TGeoConeSeg*>(shape);
        fParams = {coneSeg->GetDz(), coneSeg->GetRmin1(), coneSeg->GetRmax1(), coneSeg->GetRmin2(), coneSeg->GetRmax2(), coneSeg->GetPhi1(), coneSeg->GetPhi2()};

    } else if (typeid(*shape) == typeid(TGeoBBox)) { 

        auto* bBox = static_cast<const TGeoBBox*>(shape);
        fParams = {bBox->GetDX(), bBox->GetDY(), bBox->GetDZ()};

    } else if (typeid(*shape) == typeid(TGeoPgon)) { 

        auto* pgon = static_cast<const TGeoPgon*>(shape);
        fParams = {pgon->GetPhi1(), pgon->GetDphi(), static_cast<double>(pgon->GetNz())};
        for (int i{0}; i < pgon->GetNz(); ++i){
            fParams.push_back(pgon->GetZ(i));
            fParams.push_back(pgon->GetRmin(i));
            fParams.push_back(pgon->GetRmax(i));
        }

    } else if (typeid(*shape) == typeid(TGeoArb8)) { 

        auto* arb8 = static_cast<const TGeoArb8*>(shape);
        fParams = {arb8->GetDz()};
        auto* arb8Vertices = const_cast<TGeoArb8*>(arb8)->GetVertices();
        for (size_t i{0}; i<8; ++i) {
            fParams.push_back(arb8Vertices[2 * i]);
            fParams.push_back(arb8Vertices[2 * i + 1]);
        }

    } else if (typeid(*shape) == typeid(TGeoShapeAssembly)) { 

        // Note here we are only comparing the bounding box, not the actual shapes within the assembly
        auto* shapeAssembly = static_cast<const TGeoShapeAssembly*>(shape);
        fParams = {shapeAssembly->GetDX(), shapeAssembly->GetDY(), shapeAssembly->GetDZ()};

    } else if (typeid(*shape) == typeid(TGeoCompositeShape)) { 

        // Note here we are only comparing the bounding box, as above
        auto* compositeShape = static_cast<const TGeoShapeAssembly*>(shape);
        fParams = {compositeShape->GetDX(), compositeShape->GetDY(), compositeShape->GetDZ()};

    } else {

        throw std::invalid_argument("Unknown shape <<" + static_cast<std::string>(typeid(*shape).name()) + ">>");

    }
}

bool PseudoShape::operator==(const PseudoShape& rhs) const {

    // Check for equality in basic fields
    if (fType != rhs.fType) return false;

    if (fParams.size() != rhs.fParams.size()) return false;
    for (std::size_t i{0}; i < fParams.size(); ++i) {
        if (std::abs(fParams[i] - rhs.fParams[i]) > GEOMETRY_TOLERANCE) return false;
    }

    return true;
}

PseudoMatrix::PseudoMatrix(const TGeoMatrix* matrix) 
    : fTranslation(matrix->GetTranslation(), matrix->GetTranslation()+3), 
    fRotation(matrix->GetRotationMatrix(), matrix->GetRotationMatrix()+9), 
    fScale(matrix->GetScale(), matrix->GetScale()+3) {}

bool PseudoMatrix::operator==(const PseudoMatrix& rhs) const {

    // Check for equality in translation
    for (std::size_t i{0}; i < fTranslation.size(); ++i) {
        if (std::abs(fTranslation[i] - rhs.fTranslation[i]) > PLACEMENT_TOLERANCE) return false;
    }
    
    // Check for equality in rotation
    for (std::size_t i{0}; i < fRotation.size(); ++i) {
        if (std::abs(fRotation[i] - rhs.fRotation[i]) > PLACEMENT_TOLERANCE) return false;
    }
    
    // Check for equality in scaling
    for (std::size_t i{0}; i < fScale.size(); ++i) {
        if (std::abs(fScale[i] - rhs.fScale[i]) > PLACEMENT_TOLERANCE) return false;
    }

    return true;
}

// PseudoVolume constructor
PseudoVolume::PseudoVolume(const TGeoVolume* volume)
    : fName{volume->GetName()}, fNodes{}, fShape{PseudoShape(volume->GetShape())}, fMaterial{volume->GetMaterial()->GetName()} {}

bool PseudoVolume::operator==(const PseudoVolume& rhs) const {

    // Check for equality in basic fields
    if (fName != rhs.fName) return false;
    if (fMaterial != rhs.fMaterial) return false;
    
    // Check for equality in PseudoShape
    if (!(fShape == rhs.fShape)) return false;

    return true;
}
    
// PseudoNode constructor
PseudoNode::PseudoNode(const TGeoNode* node)
    : fName{node->GetName()}, fVolume{nullptr}, fMother{nullptr},  fMatrix{node->GetMatrix()} {}

bool PseudoNode::operator==(const PseudoNode& rhs) const {

    // Check for equality in basic fields
    if (fName != rhs.fName) return false;
    
    // Check for equality in PseudoVolume
    if (!(*fVolume == *rhs.fVolume)) return false;

    // Check for equality in PseudoMatrix
    if (!(fMatrix == rhs.fMatrix)) return false;
    
    return true;
}
    
// Note that node => the TGeoNode this PseudoNode replicates, mother => volume within which the node exists, volume => volume to which the transformation matrix pertains
PseudoNode* PseudoManager::SpawnNode(const TGeoNode* node, PseudoVolume* mother) {

    // Create PseudoNode 
    fNodes.emplace_back(std::make_unique<PseudoNode>(node));
    auto* pseudoNode = fNodes.back().get();

    // Create PseudoVolume 
    fVolumes.emplace_back(std::make_unique<PseudoVolume>(node->GetVolume()));
    auto* pseudoVolume = fVolumes.back().get();
    
    // Link PseudoVolume to its node
    pseudoNode->fVolume = pseudoVolume;

    // Add it to mother's node vector, mother is NULL for TopNode
    if (mother) {
        pseudoNode->fMother = mother;
        mother->fNodes.push_back(pseudoNode);
    } else {
        fTopLevelNode = pseudoNode;
        fTopLevelVolume = pseudoVolume;
    }
    return pseudoNode;
}

void PseudoManager::SyncNodesRemaining() {
    for (auto& volume : fVolumes) {
        volume->fNodesRemaining = volume->fNodes;
        // Reversing order for findNodePartner iteration in geo_compare.cpp
        std::reverse(volume->fNodesRemaining.begin(), volume->fNodesRemaining.end());
    }
}






