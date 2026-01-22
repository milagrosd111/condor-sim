#ifndef EVENTACTION_HH
#define EVENTACTION_HH

#include "G4UserEventAction.hh"
#include "G4Types.hh"
#include <vector>
#include <map>

struct DetectorHit
{
    G4int bar_id;
    G4int cluster_id;
  G4double energy_mev;   // energy deposited in bar (MeV)
  G4double adc_value;    // parametric ADC counts from PE estimate
  G4double time_ns;
  G4int n_photons;       // parametric photoelectrons (not optical count)
    G4double x, y, z;  // Position in mm
};

class EventAction : public G4UserEventAction
{
  public:
    EventAction();
    virtual ~EventAction();

    virtual void BeginOfEventAction(const G4Event* event);
    virtual void EndOfEventAction(const G4Event* event);

    void RecordHit(const DetectorHit& hit);
    void RecordOpticalPhoton(G4int bar_id, G4double time_ns);
    const std::vector<DetectorHit>& GetHits() const { return fHits; }

  private:
    void WriteHitsToFile();
    
    std::vector<DetectorHit> fHits;
    std::map<G4int, G4int> fOpticalPhotonCounts;  // bar_id -> detected photon count
    G4int fEventID;
};

#endif
