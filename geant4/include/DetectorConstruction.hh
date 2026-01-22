#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction();
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct();
    virtual void ConstructSDandField();
    
    // Accessor for bar logical volume (for SD registration)
    G4LogicalVolume* GetBarLV() const { return fBarLV; }
    // Accessor for PMT logical volume (for SD registration)
    G4LogicalVolume* GetPMTLV() const { return fPMTLV; }

  private:
    void DefineArrayGeometry();
    void DefineMaterials();
    void DefineOpticalProperties();

    G4LogicalVolume*  fWorldLV;
    G4LogicalVolume*  fBarLV;
    G4LogicalVolume*  fFiberLV;
    G4LogicalVolume*  fPMTLV;
    
    static constexpr G4int N_CLUSTERS = 1585;
    static constexpr G4int BARS_PER_CLUSTER = 4;
    
    // Bar dimensions (meters -> mm for Geant4)
    static constexpr G4double BAR_LENGTH = 2500.0;  // mm (2.5 m)
    static constexpr G4double BAR_WIDTH = 250.0;    // mm (0.25 m)
    static constexpr G4double BAR_HEIGHT = 50.0;    // mm (0.05 m)
    
    // WLS fiber dimensions
    static constexpr G4double FIBER_RADIUS = 1.0;   // mm (2mm diameter fiber)
    static constexpr G4double FIBER_LENGTH = BAR_LENGTH - 10.0;  // mm (slightly shorter to avoid overlap)
    
    // PMT/SiPM dimensions (Hamamatsu 14160-6015PS equivalent)
    static constexpr G4double PMT_WIDTH = 6.0;      // mm (6mm square SiPM)
    static constexpr G4double PMT_HEIGHT = 6.0;     // mm
    static constexpr G4double PMT_DEPTH = 1.0;      // mm (thin photocathode)
    
    // Cluster spacing (meters)
    static constexpr G4double CLUSTER_SPACING_X = 40000.0;  // mm (~40 m)
    static constexpr G4double CLUSTER_SPACING_Y = 40000.0;  // mm (~40 m)
    
    // Observation level (altitude, in mm)
    static constexpr G4double OBSERVATION_LEVEL = 5300000.0;  // mm (5300 m at Cerro Toco)
};

#endif
