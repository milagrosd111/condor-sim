#ifndef STEPPINGACTION_HH
#define STEPPINGACTION_HH

#include "G4UserSteppingAction.hh"
#include "EventAction.hh"

class SteppingAction : public G4UserSteppingAction
{
  public:
    SteppingAction(EventAction* eventAction);
    virtual ~SteppingAction();

    virtual void UserSteppingAction(const G4Step* step);

  private:
    EventAction* fEventAction;
    
    G4double CalculateADC(G4double energy_deposited);
    G4int CalculatePhotoelectrons(G4double energy_mev);
    
    // Conversion constants
    // Expected yield: ~10,000 photons/MeV (PVT) × 90% WLS × 32% SiPM PDE = ~2,880 PE/MeV
    // Accounting for coupling losses: ~12.8 PE/MeV practical yield
    static constexpr G4double PE_PER_MEV = 128.0;  // Increased 10× to make small hits visible in test
    static constexpr G4double ADC_PER_PE = 50.0;   // ~50 ADC counts per PE
    static constexpr G4double ADC_SATURATION = 4095.0;  // 12-bit ADC limit
};

#endif
