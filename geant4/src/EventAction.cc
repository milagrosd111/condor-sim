#include "EventAction.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include <fstream>
#include <iomanip>

EventAction::EventAction()
    : G4UserEventAction(),
      fEventID(0)
{
}

EventAction::~EventAction()
{
}

void EventAction::BeginOfEventAction(const G4Event* event)
{
    fEventID = event->GetEventID();
    fHits.clear();
    fOpticalPhotonCounts.clear();
    
    if (fEventID % 10 == 0) {
        G4cout << "--- Event " << fEventID << " ---" << G4endl;
    }
}

void EventAction::EndOfEventAction(const G4Event* event)
{
    if (fHits.size() > 0) {
        G4cout << "Event " << fEventID << ": " << fHits.size() << " bars hit" << G4endl;
        WriteHitsToFile();
    } else {
        // Write empty event to file for tracking
        std::ofstream outfile;
        outfile.open("detector_hits.txt", std::ios::app);
        outfile << "EVENT " << fEventID << std::endl;
        outfile << "NHITS 0" << std::endl;
        outfile << std::endl;
        outfile.close();
    }
}

void EventAction::RecordHit(const DetectorHit& hit)
{
    fHits.push_back(hit);
}

void EventAction::RecordOpticalPhoton(G4int bar_id, G4double time_ns)
{
    // Increment optical photon count for this bar
    fOpticalPhotonCounts[bar_id]++;
}

void EventAction::WriteHitsToFile()
{
    // Aggregate hits by bar_id and merge optical photon counts
    std::map<G4int, DetectorHit> aggregated;
    
    // First, aggregate energy deposition hits
    for (const auto& hit : fHits) {
        if (aggregated.find(hit.bar_id) == aggregated.end()) {
            aggregated[hit.bar_id] = hit;
        } else {
            // Merge multiple hits in same bar
            aggregated[hit.bar_id].energy_mev += hit.energy_mev;
            aggregated[hit.bar_id].adc_value += hit.adc_value;
            aggregated[hit.bar_id].n_photons += hit.n_photons;
            if (hit.time_ns < aggregated[hit.bar_id].time_ns) {
                aggregated[hit.bar_id].time_ns = hit.time_ns;  // Keep earliest time
            }
        }
    }
    
    // Add optical photon counts - create entries even for bars with no energy hits
    for (const auto& photon_pair : fOpticalPhotonCounts) {
        G4int bar_id = photon_pair.first;
        G4int photon_count = photon_pair.second;
        
        if (aggregated.find(bar_id) != aggregated.end()) {
            // Bar already has energy hit, just add photon count
            aggregated[bar_id].n_photons += photon_count;
        } else {
            // Bar has optical photons but no energy hit - create entry
            DetectorHit optical_hit;
            optical_hit.bar_id = bar_id;
            optical_hit.cluster_id = bar_id / 4;
            optical_hit.energy_mev = 0.0;  // No direct energy deposition
            optical_hit.adc_value = 0.0;
            optical_hit.time_ns = 0.0;  // Could track first photon time if needed
            optical_hit.n_photons = photon_count;
            optical_hit.x = 0.0;
            optical_hit.y = 0.0;
            optical_hit.z = 0.0;
            aggregated[bar_id] = optical_hit;
        }
    }
    
    // Write to file
    std::ofstream outfile;
    outfile.open("detector_hits.txt", std::ios::app);
    
    outfile << "EVENT " << fEventID << std::endl;
    outfile << "NHITS " << aggregated.size() << std::endl;
    
    for (const auto& pair : aggregated) {
        const auto& hit = pair.second;
        outfile << "HIT "
                << hit.bar_id << " "
                << hit.cluster_id << " "
                << std::fixed << std::setprecision(2)
                << hit.energy_mev << " "
                << std::fixed << std::setprecision(2)
                << hit.adc_value << " "
                << hit.time_ns << " "
                << hit.n_photons << std::endl;
    }
    outfile << std::endl;
    outfile.close();
}
