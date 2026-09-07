#ifndef GEOUTILS_H
#define GEOUTILS_H


#include <string>
#include <vector>
#include <memory>

#include <TGeoNode.h>
#include <TGeoVolume.h>


// ToDo
// -1. Try to write skeleton for PseudoManager --> IN PROGRESS
//      a. Should own all PseudoNodes and PseudoVolumes
//      b. Should "start" somewhere (world volume)
//      c. Should write contructor for PseudoManager
// 0. Try to compile headers and see if they behave
// 1. Write overloaded constructors for PseudoShape acting on TGeoBox and TGeoTube 
// 2. Write constructors (and destructors) for PseudoNode
// 3. Write constructors (and destructors) for PseudoVolume
// 4. Test minimally the written classes (can do so with basic calls, do not need recursion


// Forward declaration since PseudoNodes reference PseudoVolumes
class PseudoNode;

struct PseudoShape{
    std::string fType{};        // Shape type (e.g. T...)
    std::vector<double> fParams{};        // Shape params (e.g. x, r...)
    PseudoShape(TGeoShape* shape);    // TGeoShape must be checked against its true shape inside the constructor implementation in for branches
    PseudoShape() {};   
};

// Minimal representation of a TGeoNode
class PseudoVolume{
    public:

        std::string fName{};
        std::vector<PseudoNode*> fNodes{};           // Nodes within this Volume
        // PseudoVolume* fMother{nullptr};           // Mother Volume (this link should be made by nodes
        // PseudoShape fShape{};                       // Shape
        PseudoShape fShape;                       // Shape
        std::string fMedium{};                       // (tracking) Medium/Material
        // std::size_t fNumber{};                  // Volume serial number (in list of volumes) = not needed?


        // Ignore constructor for now... Should implement a == operator or isEqual operation to compare PseudoNodes
        // this could also check volumes being referenced? But might need to do this cleverlTrackingy if we wish to not repeatedly check the same volume (only an issue if slow...)
        // PseudoVolume(const char* name, std::vector<PseudoNode*> nodes, const PseudoVolume* mother, const TGeoShape* shape, const TGeoMedium* medium);   // This constructor will have some logic since it must call the PseudoShape constructor on TGeoShape, and must extract the name of the medium, as well as volume serial number?
        PseudoVolume(const char* name, std::vector<PseudoNode*> nodes, TGeoShape* shape, TGeoMedium* medium);   // This constructor will have some logic since it must call the PseudoShape constructor on TGeoShape, and must extract the name of the medium, as well as volume serial number?
        PseudoVolume() {};   // This constructor will have some logic since it must call the PseudoShape constructor on TGeoShape, and must extract the name of the medium, as well as volume serial number?

        std::vector<PseudoNode*> GetNodes() const {return fNodes;}

};


// Minimal representation of a TGeoNode
class PseudoNode{
    public:

        std::string fName{};
        PseudoVolume* fVolume{nullptr};           // Volume associated with Node
        PseudoVolume* fMother{nullptr};           // Mother Volume (is this necessary?)


        // // Ignore constructor for now... Should implement a == operator or isEqual operation to compare PseudoNodes
        // // this could also check volumes being referenced? But might need to do this cleverly if we wish to not repeatedly check the same volume (only an issue if slow...)
        // Vector(double x, double y, double z);
        // // + should add both vectors
        // Vector operator+(const Vector& v) const;
        PseudoNode(const char* name, PseudoVolume* volume, PseudoVolume* mother);   // Trivial implementation lives in .cpp


        PseudoVolume* GetVolume() const {return fVolume;}
        std::vector<PseudoNode*> GetNodes() const {return fVolume->GetNodes();}



};


// Minimal representation of a TGeoManager
class PseudoManager{
    public:

        std::string fName{"Geometry Manager"};
        PseudoVolume fTopLevelVolume;             // Should be set by constructor (which itself should require a "top level volume" as a TGeoVolume*
        std::vector<std::unique_ptr<PseudoNode>> fNodes{};           // Nodes spawned by the manager
        std::vector<std::unique_ptr<PseudoVolume>> fVolumes{};           // Volumes spawned by the manager

        // Ignore constructor for now... Should implement a == operator or isEqual operation to compare PseudoNodes
        // this could also check volumes being referenced? But might need to do this cleverlTrackingy if we wish to not repeatedly check the same volume (only an issue if slow...)
        PseudoManager(TGeoVolume* topLevelVolume);
        // PseudoManager(const char* name, const TGeoShape* shape, const TGeoMedium* med = nullptr);


        // void SpawnNode(PseudoNode*, TGeoNode*);         // Strips info from relevant TGeoNode and passes it to PseudoNode constructor
        void SpawnNode(TGeoNode* node, PseudoVolume* mother, PseudoVolume* volume = nullptr);   // Strips info from relevant TGeoNode(just the name) and passes it to PseudoNode constructor
        void SpawnVolume(TGeoVolume* volume, PseudoNode* node);  // Strips info from relevant TGeoVolume and passes it to PseudoVolume constructor
                                           // NOTE: The above function calls cannot fill the mother-daughter pointers, hence when calling Spawn*, its mother Node should be passed 
                                           // so that a. its fMother pointer can be filled and b. the mother's fNodes vector can receive a pointer to the new node

        // For now we do not support removing nodes or volumes (should not be needed)

};



#endif
