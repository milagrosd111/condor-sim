# CORSIKA Air Shower Simulation Setup Guide

**Project**: CONDOR Observatory Cosmic Ray Simulations  
**Version**: 1.0  
**Last Updated**: January 16, 2026

---

## Overview

CORSIKA 7.8050 is a Monte Carlo program that simulates extensive air showers (EAS) produced when cosmic rays interact with the Earth's atmosphere. For the CONDOR project, CORSIKA generates secondary particle distributions at 5,300 m altitude, which are then injected into Geant4 for detector simulation.

**Output**: Binary DAT files containing particle trajectory data (position, energy, particle type) readable by PANAMA.

---

## Installation Status

### Current Setup ✅
- **Location**: `corsika/corsika-78050/`
- **Version**: 7.8050 (configured with EPOS + UrQMD hadronic models)
- **Executable**: `corsika/corsika-78050/run/corsika78050Linux_EPOS_urqmd`
- **Status**: Fully installed and operational

### Key Components
```
corsika-78050/
├── src/                    # CORSIKA source code
├── include/                # Header files
├── lib/                    # Compiled libraries
├── run/
│   └── corsika78050Linux_EPOS_urqmd  # Main executable ✅
├── epos/                   # EPOS hadronic model data
└── config/                 # Configuration files
```

### Dependencies (Pre-installed)
- **Fortran Compiler**: gfortran (or equivalent)
- **C/C++ Compiler**: gcc/g++
- **Make**: GNU Make
- **EPOS**: Hadronic interaction model (included)
- **UrQMD**: Resonance decay model (included)

---

## Configuration

### Observatory Parameters (`corsika/simulation/config.py`)

All CORSIKA simulations use parameters configured in `config.py`:

```python
OBSERVATORY = {
    'name': 'CONDOR',
    'latitude': -25.0,              # degrees (southern hemisphere)
    'longitude': -66.5,             # degrees (western hemisphere)
    'altitude': 5300,               # meters a.s.l.
    'observation_level': 530000.0,  # cm (for CORSIKA input)
}

MAGNETIC_FIELD = {
    'bx': 22.5,     # μT (horizontal, North)
    'bz': -13.0,    # μT (vertical, downward)
}
```

### Simulation Parameters

**Primary Particles**:
- Gamma rays (CORSIKA ID: 1)
- Protons (CORSIKA ID: 14)
- Extensible to iron nuclei (CORSIKA ID: 5626) if needed

**Energy Range**:
```python
ENERGY_RANGES = {
    'gamma': {'min': 20.0, 'max': 800.0},    # GeV
    'proton': {'min': 20.0, 'max': 800.0},   # GeV
}
```

**Zenith Angles** (Θ = angle from vertical):
```python
ZENITH_RANGES = {
    'min': 0.0,     # Vertical incidence
    'max': 60.0,    # Maximum for good reconstruction
}
```

**Azimuth Angle**: Full coverage (-180° to +180°, uniformly distributed)

### CORSIKA-Specific Parameters

| Parameter | Value | Meaning |
|-----------|-------|---------|
| **ESLOPE** | -2.0 | Energy spectrum index (E⁻² power law) |
| **ECUTS** | [0.3, 0.3, 0.003, 0.003] | Energy cut-offs (GeV): hadron, muon, electron, photon |
| **HADFLG** | [0,0,0,0,0,2] | Hadronic interaction flags (EPOS+UrQMD enabled) |
| **LONGI** | 'T  20.  F  T' | Longitudinal shower profile calculation |
| **ECTMAP** | 1.E5 | Gamma factor cut for muon tracking |
| **Particle Output** | All secondary particles | Enables detailed tracking |

---

## Running Simulations

### 1. Single Test Run

```bash
cd /path/to/condor-sim

# Activate conda environment
conda activate saphir

# Run single simulation
python corsika/simulation/run_simulations.py \
    --primary gamma \
    --zenith 0 \
    --n-showers 10
```

**Output**:
- `corsika/outputs/DATxxxxxx` - Binary particle data
- `corsika/outputs/DATxxxxxx.long` - Longitudinal shower profile
- `corsika/outputs/DATxxxxxx.log` - Simulation log and parameters

### 2. Zenith Angle Scan

```bash
# Scan from 0° to 30° in 10° steps
python corsika/simulation/run_simulations.py \
    --primary gamma \
    --zenith-range 0 30 10 \
    --n-showers 100
```

**Output**: Multiple DAT files (one per zenith angle)

### 3. Multiple Primaries

```bash
# Run both gamma and proton simulations
python corsika/simulation/run_simulations.py \
    --all \
    --n-showers 100
```

**Output**: Separate DAT files for gamma and proton primaries

### 4. Production Batch (Full Specification)

```bash
# Full production matching arXiv:2503.08850 specifications
python corsika/simulation/run_simulations.py --production
```

**Configuration**:
- Primaries: gamma, proton
- Energy: 20-800 GeV (logarithmic sampling)
- Zenith: 0-60° in 2° steps
- Showers per config: 1,000
- **Total**: 62 configurations × 1,000 = 62,000 showers

**Duration**: ~1-2 weeks on single CPU (can be parallelized)

**Disk Space**: ~50-100 GB for all outputs

---

## Output Files

### DAT Files (Binary Particle Data)

**Format**: CORSIKA binary format (readable by PANAMA)

**Contents**: For each shower event:
1. **Run Header**: Simulation parameters (energy, zenith, primary type)
2. **Event Headers**: Per-event information (event ID, weight)
3. **Particle Blocks**: Secondary particles at observation level
   - Particle ID (CORSIKA convention)
   - 3D momentum vector (GeV/c)
   - 3D position at 5,300 m (cm)
   - Time (ns)
   - Additional tracking info

**Typical Size**: 0.5-2 MB per 100 showers (depends on primary energy)

### LONG Files (Longitudinal Profiles)

**Format**: ASCII text

**Contents**: Energy deposition vs. atmospheric depth (g/cm²)
- Cumulative shower development
- Used for validation and analysis
- Optional but useful for debugging

### LOG Files (Simulation Records)

**Format**: ASCII text

**Contents**:
- Complete CORSIKA input card used
- Simulation execution log
- Processing time and statistics
- Memory usage information
- Error messages (if any)

---

## Data Processing Pipeline

### Step 1: File Discovery

```python
from condor.io import find_dat_files

dat_files = find_dat_files('corsika/outputs')
print(f"Found {len(dat_files)} DAT files")
```

**Smart Detection**:
- Finds both flat and nested directory structures
- Filters out `.long` and `.log` files automatically
- Returns sorted list of DAT file paths

### Step 2: Data Loading

```python
from condor.io import read_corsika_data

run_header, event_header, particles = read_corsika_data(dat_files)

print(f"Events: {len(event_header)}")
print(f"Particles: {len(particles)}")
print(f"Columns: {particles.columns.tolist()}")
```

**Output DataFrames**:
- `run_header`: Run-level parameters (primary type, energy, etc.)
- `event_header`: Event-level info (event ID, impact position, etc.)
- `particles`: Individual particle data (type, energy, position, direction)

### Step 3: Data Validation

**Check particle distributions**:
```python
import matplotlib.pyplot as plt

# Energy distribution
particles['energy'].hist(bins=50)
plt.xlabel('Energy (GeV)')
plt.ylabel('Count')
plt.title('Cosmic Ray Shower Particle Energies')
plt.show()

# Spatial distribution
plt.scatter(particles['x'], particles['y'], s=1, alpha=0.5)
plt.xlabel('X (cm)')
plt.ylabel('Y (cm)')
plt.title('Impact Positions at 5,300 m')
plt.show()
```

### Step 4: Data Export

```python
from condor.io import save_processed_data

save_processed_data(
    run_header, 
    event_header, 
    particles,
    output_dir='data/processed',
    compression='snappy'
)
```

**Output Formats**:
- Parquet files (columnar, highly compressible)
- CSV export (human-readable, larger files)
- HDF5 (legacy, compatibility reasons)

---

## Analysis Tools

### Notebook-Based Analysis

**Location**: `analysis/corsika/condor_analysis.ipynb`

**Contents**:
1. Load multiple DAT files
2. Combine and validate data
3. Generate summary statistics
4. Plot distributions
5. Characterize shower properties

**Key Metrics**:
- Shower size (total # particles)
- Energy distribution by type
- Lateral spread (RMS distance from core)
- Particle composition (e/γ/μ ratios)

### Integration with Detector Simulation

**Location**: `integration/detector_simulation.ipynb`

**Features**:
1. Load CORSIKA particles from DAT files
2. Define CONDOR detector geometry (6,340 bars)
3. Map particles to detector positions
4. Calculate hit counts and energy deposition
5. Generate detector response plots

**Output**:
- Detector array visualization
- Hit distribution heatmaps
- Energy deposition profiles
- Particle type analysis

---

## Common Use Cases

### Use Case 1: Validate Installation

```bash
# Quick test with small shower count
python corsika/simulation/run_simulations.py \
    --primary gamma \
    --zenith 0 \
    --n-showers 5
```

**Expected**: ~1 minute, creates `DATxxxxxx` and `.long` files

### Use Case 2: Generate Test Dataset

```bash
# ~30 showers for rapid prototyping
python corsika/simulation/run_simulations.py \
    --primary gamma \
    --zenith-range 0 20 10 \
    --n-showers 10
```

**Expected**: ~10 minutes, ~50 MB output

### Use Case 3: Production Run

```bash
# Full week-long production batch
nohup python corsika/simulation/run_simulations.py --production > corsika_run.log 2>&1 &
```

**Expected**: 1-2 weeks, ~100 GB output, 62,000 showers

### Use Case 4: Specific Physics Study

```bash
# Study energy dependence of shower size
python corsika/simulation/run_simulations.py \
    --primary proton \
    --zenith 0 \
    --n-showers 100
```

**For different energies**: Edit `config.py` ENERGY_RANGES or add command-line options

---

## Troubleshooting

### Issue: "corsika78050Linux_EPOS_urqmd: command not found"

**Cause**: Installation incomplete or wrong path

**Solution**:
```bash
# Check installation
ls -l corsika/corsika-78050/run/corsika78050Linux_EPOS_urqmd

# If missing, rebuild CORSIKA
cd corsika/corsika-78050
./configure
make
```

### Issue: "EPOS initialization failed"

**Cause**: Missing or corrupted EPOS data files

**Solution**:
```bash
# Verify EPOS directory
ls corsika/corsika-78050/epos/

# Check for required files
ls corsika/corsika-78050/epos/epos.inics
ls corsika/corsika-78050/epos/epos.iniev
```

### Issue: "File does not start with b'RUNH'"

**Cause**: DAT file corrupted or not a CORSIKA file

**Solution**:
```bash
# Check file header
xxd corsika/outputs/DAT000001 | head

# Remove corrupted files
rm corsika/outputs/DAT00000X*
# Re-run simulation
```

### Issue: Out of Disk Space

**Cause**: Too many CORSIKA outputs generated

**Solution**:
```bash
# Check disk usage
du -sh corsika/outputs/

# Archive old results
tar -czf corsika_outputs_2026-01-16.tar.gz corsika/outputs/
rm -rf corsika/outputs/*

# Resume production
```

---

## Performance Optimization

### Parallel Execution

**Current**: Sequential (one shower at a time)

**Future**: Parallelization possible using:
```bash
# Run multiple simulations in parallel (4 processes)
for i in {1..4}; do
    python corsika/simulation/run_simulations.py \
        --primary gamma \
        --zenith $(expr $i \* 15) \
        --n-showers 250 &
done
wait
```

### Memory & CPU

- **RAM**: ~500 MB per CORSIKA process
- **CPU**: ~10-30 seconds per 100-shower batch (depends on energy)
- **I/O**: DAT files are sequential, minimize parallel disk access

### Disk Space Strategy

| Stage | Size | Management |
|-------|------|------------|
| Input files | Negligible | Already in repo |
| CORSIKA outputs (raw) | ~1-2 GB per 10k showers | Archive after processing |
| Processed data (Parquet) | ~20-30% of raw | Main working copy |
| Analysis results | ~1-10 GB | Keep on fast disk |

---

## References

### Internal Documentation
- `corsika/simulation/run_simulations.py` - Complete implementation
- `corsika/simulation/config.py` - All configuration parameters
- `src/condor/io/corsika.py` - Python interface
- `analysis/corsika/condor_analysis.ipynb` - Example analysis

### External Resources
- **CORSIKA Manual**: https://www.iap.kit.edu/corsika/
- **CORSIKA Input Cards**: https://www.iap.kit.edu/corsika/physics/list_of_parameters
- **PANAMA Library**: https://panama.readthedocs.io/
- **EPOS Model**: https://www.desy.de/~epos/
- **UrQMD**: https://urqmd.org/

### Related Papers
- CONDOR Design: arXiv:2503.08850 (2025)
- CORSIKA Physics: https://doi.org/10.5281/zenodo.1291010

---

## FAQ

**Q: How many showers do I need for good statistics?**  
A: For typical plots, 100-1,000 showers sufficient. Full production uses 62,000 for parameter space coverage.

**Q: Can I run CORSIKA on Windows/Mac?**  
A: CORSIKA is Linux-only. Use WSL (Windows) or native Linux for Mac.

**Q: How do I change primary particle type?**  
A: Edit `CORSIKA_PARTICLE_IDS` in `config.py` or pass `--primary gamma/proton` to script.

**Q: What's the maximum zenith angle?**  
A: Physically ~90°, but CONDOR uses 0-60° for reconstruction quality.

**Q: Can I add more primary particles (iron, etc.)?**  
A: Yes - add to `PRIMARY_PARTICLES` and `CORSIKA_PARTICLE_IDS` in `config.py`.

---

**For Questions**: Refer to `PROJECT_STATUS.md` or contact project lead.

**Last Updated**: January 16, 2026  
**Status**: ✅ Complete (Week 1)
