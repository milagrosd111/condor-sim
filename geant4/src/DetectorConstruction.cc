#include "DetectorConstruction.hh"
#include "SensitiveDetector.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4UserLimits.hh"
#include "G4VisAttributes.hh"
#include "G4Color.hh"
#include "G4RotationMatrix.hh"
#include "G4SystemOfUnits.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"

DetectorConstruction::DetectorConstruction()
    : G4VUserDetectorConstruction(),
      fWorldLV(nullptr),
      fBarLV(nullptr),
      fFiberLV(nullptr),
      fPMTLV(nullptr)
{
}

DetectorConstruction::~DetectorConstruction()
{
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    DefineMaterials();
    DefineOpticalProperties();

    // World volume (large air envelope)
    G4double world_size = 15000000.0;  // mm (15 km for air shower)
    
    G4Box* worldS = new G4Box("World", world_size/2, world_size/2, world_size/2);
    
    G4Material* air = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
    fWorldLV = new G4LogicalVolume(worldS, air, "World");
    
    G4VPhysicalVolume* worldPV = new G4PVPlacement(
        nullptr,                  // rotation
        G4ThreeVector(0, 0, 0),   // position
        fWorldLV,                 // logical volume
        "World",                  // name
        nullptr,                  // mother volume
        false,                    // no boolean operation
        0,                        // copy number
        true);                    // overlap check

    // Define array geometry (6,340 bars in clusters with WLS fibers and PMTs)
    DefineArrayGeometry();

    return worldPV;
}

void DetectorConstruction::DefineMaterials()
{
    // Get standard materials
    G4NistManager* nist = G4NistManager::Instance();
    
    // Air (for world and shower)
    nist->FindOrBuildMaterial("G4_AIR");
    
    // PVT scintillator (approximated as polystyrene)
    // Real PVT: C_n H_{2n}, density ~1.023 g/cm³
    nist->FindOrBuildMaterial("G4_POLYSTYRENE");
    
    // PMMA for WLS fiber (Y11 Kuraray)
    nist->FindOrBuildMaterial("G4_PLEXIGLASS");
    
    // Silicon for PMT photocathode
    nist->FindOrBuildMaterial("G4_Si");
}

void DetectorConstruction::DefineOpticalProperties()
{
    G4NistManager* nist = G4NistManager::Instance();
    G4Material* scintillator = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
    G4Material* wls_fiber = nist->FindOrBuildMaterial("G4_PLEXIGLASS");
    G4Material* air = nist->FindOrBuildMaterial("G4_AIR");
    
    // Photon energy range for optical properties (1.5 eV to 6.0 eV)
    const G4int nEntries = 50;
    G4double photonEnergy[nEntries];
    for (G4int i = 0; i < nEntries; i++) {
        photonEnergy[i] = (1.5 + i * (6.0 - 1.5) / (nEntries - 1)) * eV;
    }
    
    // ============================================================
    // PVT Scintillator Optical Properties (BC-408 equivalent)
    // ============================================================
    
    // Refractive index of PVT (~1.58)
    G4double scintRIndex[nEntries];
    for (G4int i = 0; i < nEntries; i++) {
        scintRIndex[i] = 1.58;
    }
    
    // Absorption length (~4 m for high-quality PVT)
    G4double scintAbsLength[nEntries];
    for (G4int i = 0; i < nEntries; i++) {
        scintAbsLength[i] = 4000.0 * mm;  // 4 meters
    }
    
    // Scintillation emission spectrum (peak at 425 nm, ~2.92 eV)
    G4double scintEmission[nEntries];
    for (G4int i = 0; i < nEntries; i++) {
        G4double energy = photonEnergy[i];
        G4double peakEnergy = 2.92 * eV;  // 425 nm
        G4double sigma = 0.15 * eV;
        scintEmission[i] = exp(-pow(energy - peakEnergy, 2) / (2.0 * sigma * sigma));
    }
    
    // Create material properties table for scintillator
    auto* scintMPT = new G4MaterialPropertiesTable();
    scintMPT->AddProperty("RINDEX", photonEnergy, scintRIndex, nEntries);
    scintMPT->AddProperty("ABSLENGTH", photonEnergy, scintAbsLength, nEntries);
    scintMPT->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy, scintEmission, nEntries);
    scintMPT->AddProperty("SCINTILLATIONCOMPONENT2", photonEnergy, scintEmission, nEntries);
    
    // Scintillation parameters
    scintMPT->AddConstProperty("SCINTILLATIONYIELD", 10000.0 / MeV);  // 10,000 photons/MeV
    scintMPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
    scintMPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 2.4 * ns);  // Fast component
    scintMPT->AddConstProperty("SCINTILLATIONTIMECONSTANT2", 2.4 * ns);  // Slow component (same for PVT)
    scintMPT->AddConstProperty("SCINTILLATIONYIELD1", 1.0);  // 100% fast
    scintMPT->AddConstProperty("SCINTILLATIONYIELD2", 0.0);  // 0% slow
    
    scintillator->SetMaterialPropertiesTable(scintMPT);
    
    // ============================================================
    // WLS Fiber Optical Properties (Kuraray Y11)
    // ============================================================
    
    // Refractive index of PMMA core (~1.59)
    G4double fiberRIndex[nEntries];
    for (G4int i = 0; i < nEntries; i++) {
        fiberRIndex[i] = 1.59;
    }
    
    // WLS absorption spectrum (absorbs blue, ~400-450 nm)
    G4double wlsAbsLength[nEntries];
    for (G4int i = 0; i < nEntries; i++) {
        G4double energy = photonEnergy[i];
        G4double wavelength = (1240.0 / (energy / eV));  // nm
        
        // Strong absorption in blue region (400-450 nm)
        if (wavelength > 380.0 && wavelength < 470.0) {
            wlsAbsLength[i] = 1.0 * mm;  // Strong absorption
        } else {
            wlsAbsLength[i] = 10000.0 * mm;  // Transparent elsewhere
        }
    }
    
    // WLS emission spectrum (emits green, ~490 nm peak)
    G4double wlsEmission[nEntries];
    for (G4int i = 0; i < nEntries; i++) {
        G4double energy = photonEnergy[i];
        G4double peakEnergy = 2.53 * eV;  // 490 nm
        G4double sigma = 0.12 * eV;
        wlsEmission[i] = exp(-pow(energy - peakEnergy, 2) / (2.0 * sigma * sigma));
    }
    
    // Create material properties table for WLS fiber
    auto* fiberMPT = new G4MaterialPropertiesTable();
    fiberMPT->AddProperty("RINDEX", photonEnergy, fiberRIndex, nEntries);
    fiberMPT->AddProperty("WLSABSLENGTH", photonEnergy, wlsAbsLength, nEntries);
    fiberMPT->AddProperty("WLSCOMPONENT", photonEnergy, wlsEmission, nEntries);
    fiberMPT->AddConstProperty("WLSTIMECONSTANT", 8.5 * ns);  // Y11 decay time
    fiberMPT->AddConstProperty("WLSMEANNUMBERPHOTONS", 0.87);  // Quantum efficiency
    
    wls_fiber->SetMaterialPropertiesTable(fiberMPT);
    
    // ============================================================
    // Air Optical Properties (for completeness)
    // ============================================================
    
    G4double airRIndex[nEntries];
    for (G4int i = 0; i < nEntries; i++) {
        airRIndex[i] = 1.0;
    }
    
    auto* airMPT = new G4MaterialPropertiesTable();
    airMPT->AddProperty("RINDEX", photonEnergy, airRIndex, nEntries);
    air->SetMaterialPropertiesTable(airMPT);
    
    G4cout << "Optical properties defined for scintillator and WLS fiber" << G4endl;
}

void DetectorConstruction::DefineArrayGeometry()
{
    G4Material* scint = G4NistManager::Instance()->FindOrBuildMaterial("G4_POLYSTYRENE");
    G4Material* fiber_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_PLEXIGLASS");
    G4Material* pmt_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_Si");

    // ============================================================
    // Define a single scintillator bar
    // ============================================================
    G4Box* barS = new G4Box("Bar",
        BAR_LENGTH / 2,
        BAR_WIDTH / 2,
        BAR_HEIGHT / 2);

    fBarLV = new G4LogicalVolume(barS, scint, "BarLV");

    // Set visualization attributes for bar
    G4VisAttributes* barVis = new G4VisAttributes(G4Color(0.0, 0.5, 1.0, 0.3));  // Semi-transparent blue
    barVis->SetForceSolid(true);
    fBarLV->SetVisAttributes(barVis);

    // ============================================================
    // Define WLS fiber (runs along center of bar)
    // ============================================================
    G4Tubs* fiberS = new G4Tubs("Fiber",
        0.0,                    // Inner radius
        FIBER_RADIUS,          // Outer radius (1 mm)
        FIBER_LENGTH / 2,      // Half-length
        0.0,                   // Start angle
        360.0 * deg);          // Span angle

    fFiberLV = new G4LogicalVolume(fiberS, fiber_mat, "FiberLV");

    // Set visualization attributes for fiber
    G4VisAttributes* fiberVis = new G4VisAttributes(G4Color(0.0, 1.0, 0.0, 0.5));  // Green
    fiberVis->SetForceSolid(true);
    fFiberLV->SetVisAttributes(fiberVis);

    // ============================================================
    // Define PMT/SiPM at fiber end
    // ============================================================
    G4Box* pmtS = new G4Box("PMT",
        PMT_WIDTH / 2,
        PMT_HEIGHT / 2,
        PMT_DEPTH / 2);

    fPMTLV = new G4LogicalVolume(pmtS, pmt_mat, "PMTLV");

    // Set visualization attributes for PMT
    G4VisAttributes* pmtVis = new G4VisAttributes(G4Color(1.0, 0.0, 0.0, 0.8));  // Red
    pmtVis->SetForceSolid(true);
    fPMTLV->SetVisAttributes(pmtVis);

    // ============================================================
    // Place fiber inside each bar (once, reused for all bars)
    // Fiber runs along X-axis for E-W bars, will be rotated for N-S bars
    // ============================================================
    // The fiber is placed at the center of the bar (0, 0, 0 in bar coordinates)
    // For E-W bars (long axis = X), fiber also along X
    new G4PVPlacement(
        nullptr,                          // No rotation (fiber along X)
        G4ThreeVector(0, 0, 0),          // Center of bar
        fFiberLV,
        "Fiber",
        fBarLV,                          // Mother volume = bar
        false,
        0,
        true);

    // ============================================================
    // Create 1,585 clusters of 4 bars each
    // Each bar has a PMT at one end
    // ============================================================
    
    // Rotation matrices for stereo configuration
    // E-W bars (long axis along X): no rotation
    G4RotationMatrix* rotEW = nullptr;
    
    // N-S bars (long axis along Y): rotate 90° around Z
    G4RotationMatrix* rotNS = new G4RotationMatrix();
    rotNS->rotateZ(90.*deg);

    // Rotation for PMT at fiber ends
    G4RotationMatrix* pmtRotX = new G4RotationMatrix();
    pmtRotX->rotateY(90.*deg);  // PMT facing along X-axis
    
    G4RotationMatrix* pmtRotY = new G4RotationMatrix();
    pmtRotY->rotateX(90.*deg);  // PMT facing along Y-axis

    // Simple 2D grid layout
    G4int nx = 40;  // ~40 clusters in X
    G4int ny = 40;  // ~40 clusters in Y
    
    G4int cluster_id = 0;
    G4int bar_counter = 0;
    
    // Bar separation within cluster (mm)
    G4double bar_sep = 600.0;  // mm
    G4double z_offset = 30.0;  // mm vertical offset between layers

    for (G4int ix = 0; ix < nx && cluster_id < N_CLUSTERS; ix++) {
        for (G4int iy = 0; iy < ny && cluster_id < N_CLUSTERS; iy++) {
            
            // Cluster center position
            G4double cluster_x = -nx * CLUSTER_SPACING_X / 2 + ix * CLUSTER_SPACING_X;
            G4double cluster_y = -ny * CLUSTER_SPACING_Y / 2 + iy * CLUSTER_SPACING_Y;
            G4double cluster_z = OBSERVATION_LEVEL;

            // ============================================================
            // Bottom layer: 2 E-W bars (long axis along X, measure Y)
            // ============================================================
            
            // Bar 0: E-W, South position
            G4ThreeVector bar0_pos(cluster_x, cluster_y - bar_sep/2, cluster_z);
            new G4PVPlacement(
                rotEW,
                bar0_pos,
                fBarLV, "Bar",
                fWorldLV, false, bar_counter, false);
            
            // PMT at east end of bar 0
            G4ThreeVector pmt0_pos = bar0_pos + G4ThreeVector(BAR_LENGTH/2 + PMT_DEPTH/2, 0, 0);
            new G4PVPlacement(
                pmtRotX,
                pmt0_pos,
                fPMTLV, "PMT",
                fWorldLV, false, bar_counter, false);
            bar_counter++;

            // Bar 1: E-W, North position
            G4ThreeVector bar1_pos(cluster_x, cluster_y + bar_sep/2, cluster_z);
            new G4PVPlacement(
                rotEW,
                bar1_pos,
                fBarLV, "Bar",
                fWorldLV, false, bar_counter, false);
            
            // PMT at east end of bar 1
            G4ThreeVector pmt1_pos = bar1_pos + G4ThreeVector(BAR_LENGTH/2 + PMT_DEPTH/2, 0, 0);
            new G4PVPlacement(
                pmtRotX,
                pmt1_pos,
                fPMTLV, "PMT",
                fWorldLV, false, bar_counter, false);
            bar_counter++;

            // ============================================================
            // Top layer: 2 N-S bars (long axis along Y, measure X)
            // ============================================================
            
            // Bar 2: N-S, West position
            G4ThreeVector bar2_pos(cluster_x - bar_sep/2, cluster_y, cluster_z + z_offset);
            new G4PVPlacement(
                rotNS,
                bar2_pos,
                fBarLV, "Bar",
                fWorldLV, false, bar_counter, false);
            
            // PMT at north end of bar 2
            G4ThreeVector pmt2_pos = bar2_pos + G4ThreeVector(0, BAR_LENGTH/2 + PMT_DEPTH/2, 0);
            new G4PVPlacement(
                pmtRotY,
                pmt2_pos,
                fPMTLV, "PMT",
                fWorldLV, false, bar_counter, false);
            bar_counter++;

            // Bar 3: N-S, East position
            G4ThreeVector bar3_pos(cluster_x + bar_sep/2, cluster_y, cluster_z + z_offset);
            new G4PVPlacement(
                rotNS,
                bar3_pos,
                fBarLV, "Bar",
                fWorldLV, false, bar_counter, false);
            
            // PMT at north end of bar 3
            G4ThreeVector pmt3_pos = bar3_pos + G4ThreeVector(0, BAR_LENGTH/2 + PMT_DEPTH/2, 0);
            new G4PVPlacement(
                pmtRotY,
                pmt3_pos,
                fPMTLV, "PMT",
                fWorldLV, false, bar_counter, false);
            bar_counter++;

            cluster_id++;
        }
    }

    G4cout << "Created " << cluster_id << " clusters with " << bar_counter 
           << " total bars and PMTs" << G4endl;
}

void DetectorConstruction::ConstructSDandField()
{
    // Sensitive detector registration will be done in main when EventAction is available
    // PMTs will be registered as sensitive detectors (not bars)
    G4cout << "DetectorConstruction::ConstructSDandField - PMTs ready for SD registration" << G4endl;
}
