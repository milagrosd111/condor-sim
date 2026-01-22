#ifndef SENSITIVEDETECTOR_HH
#define SENSITIVEDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4VPhysicalVolume.hh"
#include "EventAction.hh"

class SensitiveDetector : public G4VSensitiveDetector
{
  public:
    SensitiveDetector(const G4String& name, EventAction* eventAction);
    virtual ~SensitiveDetector();

    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);

  private:
    EventAction* fEventAction;
    
    G4double CalculateOpticalPhotons(G4double energy_deposited);
    G4double GetQuantumEfficiency(G4double wavelength);
    G4bool DetectPhoton(G4double qe);
    
    // Optical constants
    static constexpr G4double PHOTON_YIELD = 10000.0;  // photons/MeV in PVT scintillator
    static constexpr G4double WAVELENGTH_PEAK = 420.0; // nm (PVT emission peak)
    static constexpr G4double QE_PEAK = 0.32;          // 32% PDE @ peak (Hamamatsu SiPM, was 25% for PMT)
    static constexpr G4double WLS_EFFICIENCY = 0.90;   // 90% transmission (air-coupled fiber, was 50% for direct coupling)
};

#endif
