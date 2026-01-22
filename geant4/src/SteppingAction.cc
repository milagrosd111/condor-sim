#include "SteppingAction.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <cmath>

SteppingAction::SteppingAction(EventAction* eventAction)
    : G4UserSteppingAction(),
      fEventAction(eventAction)
{
}

SteppingAction::~SteppingAction()
{
}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    // Parametric mode: record energy deposition as hits regardless of geometry
    // This is faster than full optical photon tracking and sufficient for shower studies
    
    // Skip optical photons
    if (step->GetTrack()->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
        return;
    }

    // Get energy deposited by charged particles
    G4double edep = step->GetTotalEnergyDeposit();
    
    if (edep <= 0) {
        return;  // No energy deposited
    }

    // For parametric mode, assign energy to a pseudo-random bar
    // based on particle position and track direction
    G4ThreeVector pos = step->GetPreStepPoint()->GetPosition();
    G4int bar_id = (G4int)(fabs(pos.x() + pos.y()) / 1000.0) % 6340;  // Pseudo-random bar assignment
    if (bar_id < 0) bar_id = 0;
    if (bar_id >= 6340) bar_id = 6339;
    
    G4double time_ns = step->GetPreStepPoint()->GetGlobalTime() / ns;
    
    // Calculate parametric response
    G4double energy_mev = edep / MeV;
    G4int n_photoelectrons = CalculatePhotoelectrons(energy_mev);
    G4double adc = n_photoelectrons * ADC_PER_PE;
    
    if (adc > ADC_SATURATION) {
        adc = ADC_SATURATION;
    }
    
    // Add small electronic noise
    G4double noise = G4RandGauss::shoot(0.0, 3.0);
    adc += noise;
    if (adc < 0) adc = 0;

    // Record hit
    DetectorHit hit;
    hit.bar_id = bar_id;
    hit.cluster_id = bar_id / 4;
    hit.energy_mev = energy_mev;
    hit.adc_value = adc;
    hit.time_ns = time_ns;
    hit.n_photons = n_photoelectrons;
    hit.x = pos.x();
    hit.y = pos.y();
    hit.z = pos.z();

    fEventAction->RecordHit(hit);
}

G4double SteppingAction::CalculateADC(G4double energy_deposited)
{
    // Legacy function for parametric mode
    G4double energy_mev = energy_deposited / MeV;
    G4int photoelectrons = CalculatePhotoelectrons(energy_mev);
    G4double adc = photoelectrons * ADC_PER_PE;
    if (adc > ADC_SATURATION) {
        adc = ADC_SATURATION;
    }
    G4double noise = G4RandGauss::shoot(0.0, 3.0);
    adc += noise;
    return adc;
}

G4int SteppingAction::CalculatePhotoelectrons(G4double energy_mev)
{
    // Expected photoelectron yield (for comparison with optical simulation)
    // 10,000 photons/MeV × 90% WLS × 32% SiPM PDE ≈ 2,880 PE/MeV theoretical
    // With coupling losses: ~12.8 PE/MeV practical
    
    G4double photoelectrons = energy_mev * PE_PER_MEV;
    
    // Apply Poisson fluctuation
    G4int n_pe = (G4int)(photoelectrons + G4RandGauss::shoot(0.0, std::sqrt(photoelectrons)));
    
    if (n_pe < 0) {
        n_pe = 0;
    }
    
    return n_pe;
}
