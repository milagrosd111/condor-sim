# CONDOR Project Overview & Architecture

**Project Name**: CONDOR Simulation Framework  
**Full Name**: Cosmic Ray Observatory Network for Directional Detection  
**Status**: Week 1 - Architecture & Framework Setup ✅  
**Last Updated**: January 16, 2026

---

## Mission Statement

CONDOR is a comprehensive **air shower simulation and detector response framework** for a proposed ultra-large cosmic ray observatory in the Andes. The project combines:

1. **CORSIKA**: Simulates cosmic ray air showers in the atmosphere
2. **Geant4**: Simulates detector response and particle interactions  
3. **Analysis Tools**: Reconstructs particle properties and validates physics

The framework generates synthetic datasets to understand detector performance, optimize array geometry, and prepare for future observational campaigns.

**Scientific Goals**:
- Study cosmic ray composition (10¹⁶ - 10²⁰ eV)
- Detect TeV gamma ray sources via air shower reconstruction
- Investigate ultra-high-energy cosmic ray (UHECR) properties
- Validate detector concepts before hardware deployment

---

## Project Structure Overview

```
condor-sim/
├── documentation/                 # Complete technical specifications
├── config/                        # Configuration & templates
├── corsika/                       # CORSIKA 7.8050 (installed ✅)
├── geant4/                        # Geant4 detector simulation
├── integration/                   # CORSIKA → Geant4 pipeline
├── analysis/                      # Data analysis & validation
└── src/                           # Python package (condor module)
```

**Key Achievement**: Complete Week 1 foundation with:
- ✅ CORSIKA 7.8050 operational
- ✅ 41 test cosmic ray showers generated
- ✅ Python I/O utilities complete
- ✅ All technical documentation finished
- ✅ Geant4 framework initialized
- [x] Data loading pipeline (PANAMA integration)
- [x] Detector geometry model (Python, matches CONDOR specs)
- [x] Initial visualizations & analysis
---

## Data Flow Pipeline

### Complete Simulation Chain

```
Cosmic Ray Air Shower (CORSIKA)
         ↓
Secondary particles @ 5,300 m
         ↓
Detector Geometry (6,340 bars)
         ↓
Particle Transport (Geant4)
         ↓
Scintillation & Photon Generation
         ↓
Hit Detection & Digitization
         ↓
Reconstruction Algorithms
         ↓
Physics Analysis & Validation
```

---

## Week 1: Foundation & Framework Setup ✅

### Completed Deliverables

| Component | Status | Details |
|-----------|--------|---------|
| **CORSIKA 7.8050** | ✅ Complete | Installed with EPOS + UrQMD |
| **Test Data Generation** | ✅ Complete | 41 shower events generated |
| **Python I/O Library** | ✅ Complete | CORSIKA file reader (PANAMA interface) |
| **Configuration Management** | ✅ Complete | Site parameters, input templates |
| **Technical Documentation** | ✅ Complete | All specs, guides, and references |
| **Geant4 Framework** | ✅ Started | CMakeLists, basic structure initialized |

### Generated Test Datasets

```
corsika/outputs/
├── DAT000001 - DAT000041     # 41 test shower events
├── *.long files              # Longitudinal shower profiles
└── *.log files               # Simulation logs & parameters
```

**Validation Status**: All files successfully parsed by Python reader ✅

### Key Achievements

1. **CORSIKA Operational**: Executable tested with multiple configurations
2. **Data Pipeline**: Successfully reads CORSIKA binary format
3. **Documentation Complete**: All technical specifications documented
4. **Framework Foundation**: Python package structure in place

---

## Week 2-4: Implementation Plan 📋

### Week 2: Detector Simulation
- Geant4 geometry construction (6,340 bars)
- PMT response modeling
- Hit digitization & timing

### Week 3: Physics Validation
- End-to-end pipeline testing
- Reconstruction algorithm implementation
- Performance benchmarking

### Week 4: Production & Analysis
- Large-scale simulation runs
- Systematic studies
- Publication-quality results

---

## Key Technical Parameters

### Observatory Site (Fixed)

| Parameter | Value | Purpose |
|---|---|---|
| **Latitude** | 25.0°S | Location specification |
| **Longitude** | 66.5°W | Location specification |
| **Altitude** | 5,300 m | Shower observation level |
| **B-field (H)** | 22.5 μT | Charged particle tracking |
| **B-field (V)** | -13.0 μT | Particle bending |
| **Atmosphere** | US-STANDARD | Shower development |

### CORSIKA Configuration (Fixed)

| Parameter | Value | Meaning |
|---|---|---|
| **Primary Types** | γ, p | Gamma rays, protons |
| **Energy Range** | 20-800 GeV | Test to production scale |
| **Zenith** | 0-60° | Vertical to inclined |
| **ESLOPE** | -2.0 | E⁻² power law |
| **Hadronic Model** | EPOS + UrQMD | Modern interaction physics |

### Detector Array (Fixed)

| Parameter | Value | Notes |
|---|---|---|
| **Total Bars** | 6,340 | 1,585 clusters × 4 bars |
| **Coverage Area** | ~24 km² | Efficient for simulation |
| **Cluster Spacing** | 30-50 m | Detector spacing |
| **Time Resolution** | 10 ns | 100 MHz digitization |
| **Energy Range** | 0-120 MeV | Per bar ADC window |

---

## Software Stack

### Installed & Operational ✅

- **CORSIKA 7.8050**: Full installation with EPOS + UrQMD
- **Python 3.x**: Data processing & analysis
- **Pandas/NumPy**: Data manipulation
- **Matplotlib**: Visualization

### To Be Installed

- **Geant4 11.0+**: Detector simulation
- **CMake 3.20+**: Build system
- **ROOT 6.20+**: Data storage (optional)

---

## Directory Organization

```
condor-sim/
├── README.md                              # Quick start guide
├── WHAT_WE_HAVE.md                        # Detailed status
│
├── documentation/ ✅ COMPLETE
│   ├── project_overview.md                # This file
│   ├── detector_specs.md                  # Hardware specs
│   ├── corsika_setup.md                   # CORSIKA guide
│   ├── geant4_setup.md                    # Geant4 guide
│   └── references.bib                     # Citations
│
├── config/ ✅ COMPLETE
│   ├── condor_site.conf
│   ├── input_card_template.txt
│   └── particle_codes.txt
│
├── corsika/ ✅ OPERATIONAL
│   ├── corsika-78050/                     # Installed source
│   │   └── run/corsika78050Linux_EPOS_urqmd  # Executable
│   ├── simulation/
│   │   ├── run_simulations.py
│   │   └── config.py
│   └── outputs/
│       └── DAT000001-DAT000041 + logs
│
├── geant4/ 🔄 IN PROGRESS
│   ├── CMakeLists.txt
│   ├── src/
│   ├── include/
│   ├── macros/
│   └── build/
│
├── integration/
│   └── detector_simulation.ipynb
│
├── analysis/
│   ├── corsika/
│   │   └── condor_analysis.ipynb
│   └── geant4/
│
└── src/condor/
    ├── io/corsika.py                      # File reader
    ├── simulation/
    └── analysis/
```

---

## Success Criteria - Week 1 ✅

### Must Haves
- ✅ CORSIKA fully operational
- ✅ Test data generated (≥40 showers)
- ✅ Python reader working
- ✅ All documentation complete
- ✅ Project structure organized

### Nice to Haves
- ✅ Geant4 framework initialized
- ✅ Integration pipeline sketched
- ✅ Analysis notebooks created

---

## References & Documentation

### Getting Started
- [../README.md](../README.md) - Project overview & quick start guide
- [INSTALLATION.md](INSTALLATION.md) - Step-by-step setup instructions
- [QUICKREF.md](QUICKREF.md) - Quick reference for common commands

### Development & Project Management
- [CHANGELOG.md](CHANGELOG.md) - Development history & milestones
- [CONTRIBUTING.md](CONTRIBUTING.md) - Development workflow & code standards
- [PROJECT_REVIEW.md](PROJECT_REVIEW.md) - Structure assessment & metrics

### Technical Specifications
- [project_overview.md](project_overview.md) - Mission & architecture (this file)
- [detector_specs.md](detector_specs.md) - Full hardware specifications
- [corsika_setup.md](corsika_setup.md) - CORSIKA configuration & usage
- [geant4_setup.md](geant4_setup.md) - Geant4 build & physics setup
- [references.bib](references.bib) - Scientific citations & bibliography

### Configuration Files
- `config/condor_site.conf` - Site-specific parameters
- `corsika/simulation/config.py` - Python configuration

### External Resources
- **CORSIKA Manual**: https://www.iap.kit.edu/corsika/
- **Geant4 Guide**: https://geant4-userdoc.web.cern.ch/
- **CONDOR Paper**: arXiv:2503.08850 (2025)
