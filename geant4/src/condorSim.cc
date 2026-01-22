#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "QBBC.hh"
#include "G4OpticalPhysics.hh"
#include "G4OpticalParameters.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4SDManager.hh"
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "SensitiveDetector.hh"
#include "G4SystemOfUnits.hh"

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

int main(int argc, char** argv)
{
    // Parse command-line arguments
    G4bool use_ui = false;
    G4int n_events = 10;
    G4String particle_type = "proton";
    G4double energy = 1.0 * GeV;
    G4double zenith_angle = 0.0 * deg;  // Vertical by default for testing
    G4String corsika_input = "";

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--ui") == 0) {
            use_ui = true;
        } else if (strcmp(argv[i], "--events") == 0 && i + 1 < argc) {
            n_events = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--particle") == 0 && i + 1 < argc) {
            particle_type = argv[++i];
        } else if (strcmp(argv[i], "--energy") == 0 && i + 1 < argc) {
            energy = atof(argv[++i]) * GeV;
        } else if (strcmp(argv[i], "--zenith") == 0 && i + 1 < argc) {
            zenith_angle = atof(argv[++i]) * deg;
        } else if (strcmp(argv[i], "--corsika") == 0 && i + 1 < argc) {
            corsika_input = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            std::cout << "Usage: condorSim [options]\n"
                      << "  --ui                 Enable interactive UI\n"
                      << "  --events N           Number of events to simulate (default: 10)\n"
                      << "  --particle TYPE      Particle type: proton, gamma, e-, pi+ (default: proton)\n"
                      << "  --energy E           Particle energy in GeV (default: 1.0)\n"
                      << "  --zenith ANGLE       Zenith angle in degrees (default: 45.0)\n"
                      << "  --corsika FILE       CORSIKA input file (DAT prefix)\n"
                      << "  --help               Show this message\n";
            return 0;
        }
    }

    // Construct run manager
    auto* runManager = new G4RunManager;

    // Set detector construction
    auto* detector = new DetectorConstruction();
    runManager->SetUserInitialization(detector);

    // Set physics list with optical physics
    auto* physicsList = new QBBC;
    physicsList->SetVerboseLevel(0);
    
    // Register optical physics for scintillation, WLS, absorption and transport
    auto* opticalPhysics = new G4OpticalPhysics();
    physicsList->RegisterPhysics(opticalPhysics);

    // Configure optical processes explicitly to ensure scintillation is active
    auto* opticalParams = G4OpticalParameters::Instance();
    opticalParams->SetProcessActivation("Scintillation", true);
    opticalParams->SetProcessActivation("Cerenkov", false);
    opticalParams->SetProcessActivation("WLS", true);
    opticalParams->SetProcessActivation("Absorption", true);
    opticalParams->SetProcessActivation("Rayleigh", false);  // Disable Rayleigh for speed
    opticalParams->SetProcessActivation("MieHG", false);
    opticalParams->SetProcessActivation("Boundary", true);
    opticalParams->SetScintTrackSecondariesFirst(true);
    opticalParams->SetScintTrackInfo(false);
    opticalParams->SetScintStackPhotons(true);
    
    // Limit photon sampling to speed up high-multiplicity events
    opticalParams->SetScintYieldFactor(0.5);  // Reduce photons per MeV by 50% to avoid timeout
    
    runManager->SetUserInitialization(physicsList);

    // Set primary generator action
    auto* primaryGenerator = new PrimaryGeneratorAction();
    primaryGenerator->SetParticleType(particle_type);
    primaryGenerator->SetEnergy(energy);
    primaryGenerator->SetZenithAngle(zenith_angle);
    
    // Try to load CORSIKA file if it exists
    const G4String corsika_file = "corsika_primaries.csv";
    std::ifstream check_file(corsika_file);
    if (check_file.good()) {
        check_file.close();
        primaryGenerator->LoadCorsikaFile(corsika_file);
    }
    
    runManager->SetUserAction(primaryGenerator);

    // Set event action
    auto* eventAction = new EventAction();
    runManager->SetUserAction(eventAction);

    // Set stepping action
    auto* steppingAction = new SteppingAction(eventAction);
    runManager->SetUserAction(steppingAction);

    // Initialize G4 kernel
    runManager->Initialize();

    // Register sensitive detector after initialization (when geometry is built)
    // Attach to PMT volumes (where photons are detected) instead of bars
    auto* sdManager = G4SDManager::GetSDMpointer();
    auto* pmtSD = new SensitiveDetector("PMTSD", eventAction);
    sdManager->AddNewDetector(pmtSD);
    if (detector->GetPMTLV()) {
        detector->GetPMTLV()->SetSensitiveDetector(pmtSD);
        G4cout << "Sensitive detector attached to PMT volumes" << G4endl;
    }

    // Get UI manager
    auto* UImanager = G4UImanager::GetUIpointer();

    if (use_ui) {
        // Interactive mode with visualization
        auto* visManager = new G4VisExecutive;
        visManager->Initialize();

        auto* ui = new G4UIExecutive(argc, argv);
        UImanager->ApplyCommand("/control/execute macros/vis.mac");
        ui->SessionStart();
        delete ui;
        delete visManager;
    } else {
        // Batch mode
        G4String command = "/run/beamOn " + std::to_string(n_events);
        UImanager->ApplyCommand(command);
    }

    // Job termination
    delete runManager;
    return 0;
}
