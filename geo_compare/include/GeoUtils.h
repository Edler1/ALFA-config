#ifndef GEOUTILS_H
#define GEOUTILS_H


#include <string>
#include <vector>
#include <memory>

#include <TGeoNode.h>
#include <TGeoVolume.h>
#include <TGeoShape.h>
#include <TGeoMatrix.h>


constexpr double GEOMETRY_TOLERANCE = 1E-6;
constexpr double PLACEMENT_TOLERANCE = 1E-6;

class PseudoNode;

struct PseudoShape{
    std::string fType{};        // Shape type (e.g. TGeoTubeSeg...)
    std::vector<double> fParams{};        // Shape params (e.g. x, r...)
    PseudoShape(const TGeoShape* shape);    // TGeoShape must be checked against its true shape inside the constructor implementation in for branches
    bool operator==(const PseudoShape& rhs) const;
};

struct PseudoMatrix{
    std::vector<double> fTranslation{};
    std::vector<double> fRotation{};
    std::vector<double> fScale{};
    PseudoMatrix(const TGeoMatrix* matrix); // Populates above matrices using TGeoMatrix accessors
    bool operator==(const PseudoMatrix& rhs) const;
};

// Minimal representation of a TGeoVolume
class PseudoVolume{
    public:
        std::string fName{};
        std::vector<PseudoNode*> fNodes{};           // Nodes within this Volume
        std::vector<PseudoNode*> fNodesRemaining{};  // Nodes within this Volume, used for bookkeeping during recursive calls (avoiding comparison to same node twice)
        PseudoShape fShape;                       // Shape
        std::string fMaterial{};                       // Material
        // std::size_t fNumber{};                  // Volume serial number (in list of volumes) = not needed?

        PseudoVolume(const TGeoVolume* volume);   
        bool operator==(const PseudoVolume& rhs) const;
};


// Minimal representation of a TGeoNode
class PseudoNode{
    public:
        std::string fName{};
        PseudoVolume* fVolume{nullptr};           // Volume associated with Node
        PseudoVolume* fMother{nullptr};           // Mother Volume (unused for now)
        PseudoMatrix fMatrix;                     // Placement matrix associated with fVolume

        PseudoNode(const TGeoNode* node);  
        bool operator==(const PseudoNode& rhs) const;
};


// Minimal representation of a TGeoManager
class PseudoManager{
    public:
        std::string fName{"Geometry Manager"};
        PseudoNode* fTopLevelNode{nullptr};             
        PseudoVolume* fTopLevelVolume{nullptr};             
        std::vector<std::unique_ptr<PseudoNode>> fNodes{};           // Nodes spawned by the manager
        std::vector<std::unique_ptr<PseudoVolume>> fVolumes{};           // Volumes spawned by the manager

        PseudoManager() = default;

        PseudoNode* SpawnNode(const TGeoNode* node, PseudoVolume* mother);   // Strips info from relevant TGeoNode(just the name) and passes it to PseudoNode constructor
        // void SpawnVolume(TGeoVolume* volume, PseudoNode* node);  // Strips info from relevant TGeoVolume and passes it to PseudoVolume constructor (now done by above)

        void SyncNodesRemaining();  // Copies content from fNodes to fNodesRemaining for each volume

        // For now we do not support removing nodes or volumes (should not be needed)
};



#endif
