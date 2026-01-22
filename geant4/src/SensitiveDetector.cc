#include "SensitiveDetector.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4RunManager.hh"

SensitiveDetector::SensitiveDetector(const G4String& name, EventAction* eventAction)
    : G4VSensitiveDetector(name),
      fEventAction(eventAction)
{
    collectionName.insert("PMTHitCollection");
}

SensitiveDetector::~SensitiveDetector()
{
}

G4bool SensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    // This SD is attached to PMT volumes and detects optical photons
    G4Track* track = step->GetTrack();
    
    // Only process optical photons
    if (track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) {
        return false;
    }

    // Get photon properties
    G4double photonEnergy = track->GetKineticEnergy();
    G4double wavelength = (1240.0 * eV * nm) / photonEnergy;  // Convert energy to wavelength (nm)

    // Calculate quantum efficiency at this wavelength
    G4double qe = GetQuantumEfficiency(wavelength / nm);

    // Apply stochastic detection
    G4bool detected = DetectPhoton(qe);
    
    // Kill the photon (absorbed by PMT either way)
    track->SetTrackStatus(fStopAndKill);
    
    if (detected) {
        // Record optical photon detection
        G4int pmt_id = step->GetPreStepPoint()->GetTouchableHandle()->GetCopyNumber();
        G4double detection_time = track->GetGlobalTime() / ns;
        
        // Record to EventAction (PMT ID corresponds to bar ID in 1:1 geometry)
        fEventAction->RecordOpticalPhoton(pmt_id, detection_time);
        
        return true;
    }
    
    return false;
}

G4double SensitiveDetector::GetQuantumEfficiency(G4double wavelength_nm)
{
    // Hamamatsu 14160-6015PS SiPM quantum efficiency curve
    // Peak: 32% @ 420 nm (improved from 25% for older PMT design)
    // Gaussian approximation with FWHM ~ 50 nm (sigma ~ 21 nm)
    
    G4double sigma = 21.0;  // nm
    G4double center = 420.0;  // nm
    G4double peak_qe = 0.32;  // 32% at peak
    
    G4double qe = peak_qe * exp(-0.5 * pow((wavelength_nm - center) / sigma, 2));
    
    // Ensure QE is in valid range
    if (qe < 0.0) qe = 0.0;
    if (qe > 1.0) qe = 1.0;
    
    return qe;
}

G4bool SensitiveDetector::DetectPhoton(G4double quantum_efficiency)
{
    // Stochastic photon detection based on QE
    return (G4UniformRand() < quantum_efficiency);
}

G4double SensitiveDetector::CalculateOpticalPhotons(G4double energy_deposited_mev)
{
    // This function is kept for reference but not used in optical photon mode
    // In optical mode, Geant4 generates photons automatically via scintillation process
    
    // Photon yield from scintillation: ~10,000 photons/MeV for PVT
    G4double photons = energy_deposited_mev * PHOTON_YIELD;
    
    // Birks quenching (simplified model)
    G4double quenching_factor = 1.0 / (1.0 + 0.0051 * energy_deposited_mev);
    photons *= quenching_factor;
    
    return photons;
}
