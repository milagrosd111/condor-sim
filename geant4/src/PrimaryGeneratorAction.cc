#include "PrimaryGeneratorAction.hh"
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction(),
      fParticleGun(nullptr),
      fParticleType("gamma"),
      fEnergy(100.0 * GeV),
      fZenithAngle(0.0),
      fAzimuthAngle(0.0),
      fCurrentEventIndex(0),
      fUseCorsika(false)
{
    G4int n_particle = 1;
    fParticleGun = new G4ParticleGun(n_particle);

    // Set default particle (gamma)
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
    fParticleGun->SetParticleDefinition(particle);
    fParticleGun->SetParticleEnergy(fEnergy);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

void PrimaryGeneratorAction::LoadCorsikaFile(const G4String& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        G4cerr << "Warning: Could not open CORSIKA file " << filename << G4endl;
        fUseCorsika = false;
        return;
    }
    
    fCorsikaEvents.clear();
    std::string line;
    
    // Skip header
    std::getline(file, line);
    
    // Read events
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        // Parse CSV line (comma-separated values)
        std::istringstream iss(line);
        CorsikaEvent evt;
        std::string token;
        
        // Read event_id
        if (!std::getline(iss, token, ',')) continue;
        evt.event_id = std::stoi(token);
        
        // Read particle_name
        if (!std::getline(iss, token, ',')) continue;
        evt.particle_name = token;
        
        // Read energy_gev
        if (!std::getline(iss, token, ',')) continue;
        evt.energy_gev = std::stod(token);
        
        // Read zenith_rad
        if (!std::getline(iss, token, ',')) continue;
        evt.zenith_rad = std::stod(token);
        
        // Read azimuth_rad
        if (!std::getline(iss, token, ',')) continue;
        evt.azimuth_rad = std::stod(token);
        
        // Read altitude_cm
        if (!std::getline(iss, token, ',')) continue;
        evt.altitude_cm = std::stod(token);
        
        // Read px_gev
        if (!std::getline(iss, token, ',')) continue;
        evt.px_gev = std::stod(token);
        
        // Read py_gev
        if (!std::getline(iss, token, ',')) continue;
        evt.py_gev = std::stod(token);
        
        // Read pz_gev
        if (!std::getline(iss, token, ',')) continue;
        evt.pz_gev = std::stod(token);
        
        fCorsikaEvents.push_back(evt);
    }
    
    file.close();
    fUseCorsika = (fCorsikaEvents.size() > 0);
    fCurrentEventIndex = 0;
    
    G4cout << "Loaded " << fCorsikaEvents.size() << " CORSIKA primary events" << G4endl;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    // Choose between CORSIKA input and generated particles
    if (fUseCorsika && fCurrentEventIndex < (G4int)fCorsikaEvents.size()) {
        // Use CORSIKA event
        const CorsikaEvent& evt = fCorsikaEvents[fCurrentEventIndex];
        fCurrentEventIndex++;
        
        // Get particle definition
        G4ParticleDefinition* particle = 
            G4ParticleTable::GetParticleTable()->FindParticle(evt.particle_name);
        
        if (!particle) {
            G4cerr << "Warning: Unknown particle " << evt.particle_name << G4endl;
            return;
        }
        
        fParticleGun->SetParticleDefinition(particle);
        fParticleGun->SetParticleEnergy(evt.energy_gev * GeV);
        
        // Set direction from momentum components
        G4double pmag = std::sqrt(evt.px_gev*evt.px_gev + evt.py_gev*evt.py_gev + evt.pz_gev*evt.pz_gev);
        if (pmag > 0) {
            G4ThreeVector dir(evt.px_gev / pmag, evt.py_gev / pmag, evt.pz_gev / pmag);
            fParticleGun->SetParticleMomentumDirection(dir.unit());
        }
        
        // Starting altitude from CORSIKA (convert cm to mm)
        // CORSIKA altitude is already at observation level (5300 m), NOT site altitude + observation level
        G4double z0 = evt.altitude_cm * 10.0 + 1000000.0;  // cm to mm, +1 km above detector
        
        // Spawn particles within array footprint (rough position to ensure bar hits)
        // Array spans ~1600 km × 1600 km (40 clusters × 40 km each)
        // Use shower event ID to distribute across array
        G4double array_span = 1600000000.0;  // mm (1600 km total)
        G4double x0 = -array_span/2.0 + (evt.event_id % 40) * (array_span / 40.0);
        G4double y0 = -array_span/2.0 + ((evt.event_id / 40) % 40) * (array_span / 40.0);
        
        fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, z0));
        
    } else {
        // Generate random particle (fallback or standalone mode)
        // Set particle type
        G4ParticleDefinition* particle = nullptr;
        if (fParticleType == "gamma") {
            particle = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
        } else if (fParticleType == "proton") {
            particle = G4ParticleTable::GetParticleTable()->FindParticle("proton");
        } else if (fParticleType == "e-") {
            particle = G4ParticleTable::GetParticleTable()->FindParticle("e-");
        } else {
            particle = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
        }

        fParticleGun->SetParticleDefinition(particle);
        fParticleGun->SetParticleEnergy(fEnergy);

        // Set direction from zenith and azimuth angles
        G4double theta = fZenithAngle;    // polar angle from vertical
        G4double phi = fAzimuthAngle;     // azimuthal angle
        
        G4double px = std::sin(theta) * std::cos(phi);
        G4double py = std::sin(theta) * std::sin(phi);
        G4double pz = -std::cos(theta);  // Downward (negative Z)
        
        fParticleGun->SetParticleMomentumDirection(G4ThreeVector(px, py, pz));

        // Random impact position in X-Y plane above detector
        // For testing: spawn particles above central cluster within bar coverage area
        // Central cluster bars span ±300mm in both X and Y
        G4double x0 = G4UniformRand() * 600.0 - 300.0;  // ±300mm
        G4double y0 = G4UniformRand() * 600.0 - 300.0;  // ±300mm
        G4double z0 = 5613000.0 + 1000.0;  // 1m above detector
        
        fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, z0));
    }

    fParticleGun->GeneratePrimaryVertex(anEvent);
}
