# CONDOR Simulation Framework

**Cosmic Ray Observatory Network for Directional Detection**

A comprehensive air shower simulation and detector response framework for cosmic ray observatory design and analysis.

---

## Quick Start

### Prerequisites
- Python 3.10+
- CORSIKA 7.8050 (installed locally under `corsika/corsika-78050`, not committed)
- Geant4 11.3.2 (build artifacts in `geant4/build` are local-only)
- conda environment: `saphir`

### Installation
```bash
# Install Python dependencies
conda activate saphir
pip install -r requirements.txt

# Build Geant4 (if needed)
cd geant4/build && cmake .. && make -j4 && cd ../..
```

### Running Simulations

**CORSIKA Air Shower Simulation** (Already complete)
```bash
# 30 gamma-ray simulations (DAT000001-000030)
# 30 proton simulations (DAT000031-000060)
# Located in: corsika/outputs/{gammas,protons}/
```

**Geant4 Detector Simulations**
```bash
# Run single simulation
bash geant4/scripts/run_single_simulation.sh gammas 000001

# Run all 60 simulations (batch mode)
bash geant4/scripts/run_batch_simulation.sh

# Run 14 simulations in parallel
bash geant4/scripts/run_parallel.sh 4
```

**Analysis Notebooks**
```bash
# CORSIKA data analysis & visualization
notebooks/cosmic_rays/corsika_simulation.ipynb

# Detector array simulation results
notebooks/detection/detector_array_simulation.ipynb

# Geant4 detector response analysis
notebooks/detection/geant4_simulation.ipynb
```

---

## CORSIKA (local-only, not included)

- CORSIKA is licensed separately and not stored in this repository.
- To build locally, follow [corsika/SETUP.md](corsika/SETUP.md) and install under `corsika/corsika-78050/` (gitignored).

---

## Project Structure

```
condor-sim/
├── README.md                              # This file
├── requirements.txt                       # Python dependencies
├── .gitignore                            # Git ignore rules
│
├── documentation/                         # Technical documentation
│   ├── project_overview.md               # Project mission & architecture
│   ├── installation.md                   # Installation & setup guide
│   ├── corsika_setup.md                  # CORSIKA configuration details
│   ├── geant4_setup.md                   # Geant4 build & physics setup
│   ├── detector_specs.md                 # CONDOR detector specifications
│   ├── quickref.md                       # Quick reference guide
│   └── references.bib                    # Bibliography
│
├── config/                                # Configuration files
│   ├── condor_site.conf                  # Observatory site parameters
│   ├── input_card_template.txt           # CORSIKA steering template
│   └── particle_codes.txt                # CORSIKA particle ID mappings
│
├── corsika/                               # CORSIKA simulation framework
│   ├── corsika-78050/ (local)            # CORSIKA 7.8050 source & build (gitignored)
│   ├── simulation/                       # CORSIKA run scripts
│   └── outputs/ (generated)              # CORSIKA DAT outputs (gitignored)
│       ├── gammas/                       # 30 gamma-ray simulations
│       └── protons/                      # 30 proton simulations
│
├── geant4/                                # Geant4 detector simulation
│   ├── CMakeLists.txt                    # CMake build configuration
│   ├── build/ (local)                    # Compiled executables & build files (gitignored)
│   ├── include/                          # C++ header files
│   ├── src/                              # C++ source code
│   ├── macros/                           # Geant4 macro files
│   ├── scripts/                          # Simulation runner scripts
│   │   ├── run_single_simulation.sh      # Run one DAT file
│   │   ├── run_batch_simulation.sh       # Run all 60 simulations
│   │   └── run_parallel.sh               # Run in parallel
│   └── outputs/                          # Detector response results
│
├── src/                                   # Python source code
│   ├── corsika_to_geant4.py              # CORSIKA → Geant4 converter
│   └── condor/                           # Main Python package
│       ├── __init__.py
│       ├── paths.py                      # Path utilities
│       ├── io/                           # Input/Output modules
│       │   ├── corsika.py               # CORSIKA file reading
│       │   └── __init__.py
│       └── simulation/                   # Simulation modules
│           ├── config.py                 # Configuration management
│           └── __init__.py
│
└── notebooks/                             # Jupyter analysis notebooks
   ├── cosmic_rays/
   │   ├── corsika_simulation.ipynb      # CORSIKA data analysis
   │   └── results/ (generated)          # Generated plots & data (gitignored)
   ├── detection/
   │   ├── detector_array_simulation.ipynb  # Detector array response
   │   ├── geant4_simulation.ipynb       # Geant4 results analysis
   │   └── results/ (generated)          # Generated plots & data (gitignored)
   └── results/ (generated)              # Shared results directory (gitignored)
```

---

## Data Pipeline

### Input Data
- **CORSIKA DAT files**: Binary air shower particle data
  - Located: `corsika/outputs/{gammas,protons}/DAT000001-000060`
  - Format: CORSIKA binary (read by PANAMA library)
  - Content: 30+ particle types per event, energy/momentum/position info

### Processing Steps

1. **CORSIKA Simulation** ✅ Complete
   - 30 gamma-ray showers (primary energy 20-800 GeV)
   - 30 proton showers (primary energy 20-800 GeV)
   - Ground level (5300 m altitude)
   - All particles E > 3 MeV tracked

2. **Data Analysis** ✅ Complete
   - Jupyter notebook: `notebooks/cosmic_rays/corsika_simulation.ipynb`
   - Particle composition analysis
   - Energy/momentum distributions
   - Shower comparison (gamma vs proton)
   - Output: plots saved to `notebooks/cosmic_rays/results/`

3. **Detector Array Simulation** ✅ Complete
   - Jupyter notebook: `notebooks/detection/detector_array_simulation.ipynb`
   - Maps CORSIKA particles to detector units
   - Computes hit rates and energy deposition
   - Output: detector response statistics

4. **Geant4 Detector Simulation** ⏳ In Progress
   - Optical physics simulation for water Cherenkov detector
   - Photon production and propagation
   - Runner scripts: `geant4/scripts/run_*.sh`
   - Output: `geant4/outputs/*/detector_hits.txt`

5. **Final Analysis** ⏳ Pending
   - Jupyter notebook: `notebooks/detection/geant4_simulation.ipynb`
   - Detector performance metrics
   - Gamma-ray discrimination power

### Output Data
- **CORSIKA**: Binary DAT files (~2-3 MB each, 60 total)
- **Analysis**: PNG plots, CSV summaries, HDF5 datasets
- **Geant4**: Detector hit files, simulation logs
- **Results**: Stored in `notebooks/*/results/` for each notebook

---

## Key Features

### Physics Simulation
- **CORSIKA 7.8050**: Full air shower simulation
  - Hadronic model: EPOS-LHC (E>30 GeV) + URQMD (E<30 GeV)
  - EM model: EGS4 (detailed) + NKG (approximation)
  - Magnetic field: IGRF-13 model (Chile location)
  - Output: Particle-level data at ground level

- **Geant4 11.3.2**: Detector response simulation
  - Optical physics: Cherenkov light production
  - Water medium: n=1.33, Cherenkov threshold ≈ 0.71c
  - Photon tracking and detection efficiency
  - Output: Hit patterns and energy deposition

### Detector Geometry
- **CONDOR Observatory**: Water Cherenkov detector array
  - Central array: 10×10 units (8.1 m × 8.7 m each)
  - Peripheral: 20 units (veto/tagging region)
  - Total: 120 units × 52 bars = 6,240 detector bars
  - See `documentation/detector_specs.md` for details

### Analysis Tools
- **PANAMA**: Python library for CORSIKA file parsing
- **Pandas/NumPy**: Data analysis and statistics
- **Matplotlib**: Plotting and visualization
- **Jupyter**: Interactive analysis notebooks

---

## Documentation

All project documentation is in the `documentation/` folder:

- [documentation/project_overview.md](documentation/project_overview.md) - Mission & architecture overview
- [documentation/installation.md](documentation/installation.md) - Installation & setup guide
- [documentation/quickref.md](documentation/quickref.md) - Quick reference for common commands
- [documentation/corsika_setup.md](documentation/corsika_setup.md) - CORSIKA configuration details
- [documentation/geant4_setup.md](documentation/geant4_setup.md) - Geant4 build & physics setup
- [documentation/detector_specs.md](documentation/detector_specs.md) - CONDOR detector specifications
- [documentation/references.bib](documentation/references.bib) - Scientific citations

---

## Configuration

### CORSIKA Settings
See [documentation/corsika_setup.md](documentation/corsika_setup.md) for full parameters:
- **Primary particles**: Protons (PRMPAR=14), Gamma rays (PRMPAR=1)
- **Energy range**: 20-800 GeV (power-law spectrum E⁻²)
- **Site**: Cerro Toco, Chile (5300 m altitude)
- **Interaction models**: EPOS-LHC + URQMD (hadronic), EGS4 (EM)
- **Output**: DAT files with full particle details at ground level

### Geant4 Settings
See [documentation/geant4_setup.md](documentation/geant4_setup.md) for full parameters:
- **Physics list**: Optical physics with EM interactions
- **Detector**: Water Cherenkov with scintillation bars
- **Geometry**: Matches CONDOR specifications exactly
- **Output**: Detector hit positions and photoelectron counts

### Site Parameters
See `config/condor_site.conf`:
- Latitude: 23.863°S
- Longitude: 68.269°W
- Altitude: 5300 m a.s.l.
- Magnetic field: 25.5 μT (north), -31.2 μT (vertical)

---

## Usage Examples

### Example 1: Run a Single Geant4 Simulation
```bash
# Simulate detector response to first gamma-ray shower
bash geant4/scripts/run_single_simulation.sh gammas 000001

# Output: geant4/outputs/single_gammas_DAT000001_TIMESTAMP/
#   ├── corsika_primaries.csv      (particle input)
#   ├── detector_hits.txt           (simulation results)
#   └── run.log                     (execution log)
```

### Example 2: Run All Simulations in Background
```bash
# Start batch processing (60 simulations)
nohup bash geant4/scripts/run_batch_simulation.sh > geant4/batch.log 2>&1 &

# Monitor progress
tail -f geant4/batch.log

# Results: geant4/outputs/batch_TIMESTAMP/
#   ├── gammas/
#   │   ├── DAT000001/detector_hits.txt
#   │   └── ...
#   ├── protons/
#   │   ├── DAT000031/detector_hits.txt
#   │   └── ...
#   └── batch_summary.json
```

### Example 3: Run Subset in Parallel
```bash
# Run 14 simulations (7 gamma + 7 proton) with 4 parallel jobs
bash geant4/scripts/run_parallel.sh 4

# Results: geant4/outputs/single_*_TIMESTAMP/
```

### Example 4: Analyze Results in Jupyter
```bash
# Start Jupyter server
jupyter notebook notebooks/detection/geant4_simulation.ipynb

# Load results and generate plots
# (Notebook handles all analysis)
```

---

## Development & Contributing

### Code Organization
- **Python package**: `src/condor/` - Core simulation infrastructure
- **Scripts**: `geant4/scripts/` - Automation and batch processing
- **Notebooks**: `notebooks/*/` - Analysis and visualization
- **Configuration**: `config/` - Settings and parameters

### Adding New Features
1. Update documentation in `documentation/`
2. Add Python code to appropriate `src/condor/` subpackage
3. Create test/demo notebook if adding major feature
4. Update this README with new capabilities

### Dependencies
See `requirements.txt` for Python package versions:
- panama: CORSIKA file I/O
- pandas: Data manipulation
- numpy: Numerical computations
- matplotlib: Plotting
- jupyter: Interactive notebooks
- particle: Particle physics utilities

---

## Known Issues & Limitations

1. **CORSIKA output structure**: Recently changed from theta_XX subdirectories to flat DAT files
   - All scripts updated to use new structure
   - See `geant4/scripts/` for current versions

2. **Geant4 compilation**: Requires proper build configuration
   - Use: `cd geant4/build && cmake .. && make -j4`
   - Consult `documentation/geant4_setup.md` if issues arise

3. **Large data files**: CORSIKA outputs not tracked in git
   - Files in `corsika/outputs/` are generated, not committed
   - Regenerate if needed using CORSIKA

4. **Jupyter path dependencies**: Notebooks assume specific working directory
   - Always run from notebook directory or update paths
   - Path calculation: `project_root = Path.cwd().parent.parent`

---

## Performance Notes

### Timing Estimates
- **CORSIKA per simulation**: ~5 minutes (30 showers for each particle type)
- **Geant4 per simulation**: ~2-5 minutes (1000 events)
- **Batch (60 simulations)**: ~4-6 hours total
- **Parallel (14 samples)**: ~30 minutes with 4 cores

### Resource Requirements
- **Disk**: ~500 MB for CORSIKA outputs + results
- **Memory**: 4-8 GB recommended for Jupyter + analysis
- **CPU**: 4+ cores recommended for parallel processing

---

## References

- CORSIKA 7.8050: https://www.iap.kit.edu/corsika/
- Geant4 11.3.2: https://geant4.web.cern.ch/
- PANAMA: https://github.com/The-Ludwig/PANAMA
- CONDOR specs: See `documentation/detector_specs.md`
- Physics refs: See `documentation/references.bib`

---

## Contact & Support

For questions or issues:
1. Check documentation in `documentation/`
2. Review notebook examples in `notebooks/*/`
3. Check logs in relevant output directories
4. Consult project overview in `documentation/project_overview.md`

---

**Last Updated**: January 21, 2026  
**Status**: Core simulations complete, Geant4 detector analysis in progress
