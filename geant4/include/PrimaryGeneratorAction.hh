#ifndef PRIMARYGENERATORACTION_HH
#define PRIMARYGENERATORACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include <vector>
#include <string>

struct CorsikaEvent {
    G4int event_id;
    G4String particle_name;
    G4double energy_gev;
    G4double zenith_rad;
    G4double azimuth_rad;
    G4double altitude_cm;
    G4double px_gev;
    G4double py_gev;
    G4double pz_gev;
};

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event* anEvent);

    void SetParticleType(G4String type) { fParticleType = type; }
    void SetEnergy(G4double energy) { fEnergy = energy; }
    void SetZenithAngle(G4double theta) { fZenithAngle = theta; }
    void SetAzimuthAngle(G4double phi) { fAzimuthAngle = phi; }
    
    // CORSIKA file input
    void LoadCorsikaFile(const G4String& filename);
    G4bool UseCorsika() const { return fUseCorsika; }

  private:
    G4ParticleGun* fParticleGun;
    G4String fParticleType;
    G4double fEnergy;
    G4double fZenithAngle;
    G4double fAzimuthAngle;
    
    // CORSIKA input
    std::vector<CorsikaEvent> fCorsikaEvents;
    G4int fCurrentEventIndex;
    G4bool fUseCorsika;
};

#endif
