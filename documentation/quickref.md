# Quick Reference Guide

Fast lookup for common tasks and commands. For detailed information, see the full documentation files.

---

## Quick Start

```bash
# Activate environment
conda activate saphir

# Navigate to project
cd /path/to/condor-sim

# Check installation
ls corsika/outputs/gammas/DAT* | wc -l    # Should be 30
ls corsika/outputs/protons/DAT* | wc -l   # Should be 30
```

---

## CORSIKA Data

### Check Simulation Status
```bash
# Count available DAT files
ls corsika/outputs/gammas/DAT* | wc -l    # Should be 30
ls corsika/outputs/protons/DAT* | wc -l   # Should be 30

# List gamma shower files
ls -lh corsika/outputs/gammas/DAT*

# List proton shower files
ls -lh corsika/outputs/protons/DAT*

# Show file sizes
du -sh corsika/outputs/*
```

### Run CORSIKA Analysis
```bash
# Navigate to notebook
cd notebooks/cosmic_rays

# Launch Jupyter
jupyter notebook corsika_simulation.ipynb

# Run analysis (execute cells 1-8 for full pipeline):
# 1. Imports & setup
# 2. Path configuration
# 3. Diagnostic path check
# 4. Data loading
# 5. Particle composition analysis
# 6. Energy distributions
# 7. Muon content comparison
# 8. Summary plots
```

### Regenerate CORSIKA Simulations (if needed)
```bash
# Go to CORSIKA simulation directory
cd corsika/simulation

# Run simulations
python3 run_simulations.py

# Duration: ~1 hour for 60 simulations
# Output: DAT files in corsika/outputs/{gammas,protons}/
```

---

## Geant4 Detector Simulation

### Run Single Simulation
```bash
# Format: bash geant4/scripts/run_single_simulation.sh <particle_type> <dat_number>

# Example: Simulate first gamma shower
bash geant4/scripts/run_single_simulation.sh gammas 000001

# Example: Simulate first proton shower
bash geant4/scripts/run_single_simulation.sh protons 000031

# Valid DAT numbers:
#   Gammas:  000001-000030
#   Protons: 000031-000060 (internally converted)
```

### Run All 60 Simulations (Batch)
```bash
# Start batch processing (runs sequentially, ~4-6 hours)
bash geant4/scripts/run_batch_simulation.sh

# Monitor progress in another terminal
tail -f geant4/batch.log

# Expected output: 60 detector_hits.txt files
# Location: geant4/outputs/batch_TIMESTAMP/
```

### Run Parallel Simulations
```bash
# Run 14 sample simulations in parallel (7 gammas + 7 protons)
bash geant4/scripts/run_parallel.sh

# Uses half available CPU cores by default
# Monitor: ps aux | grep condorSim

# Samples: DAT001, 005, 010, 015, 020, 025, 030 for each type
```

### Monitor Geant4 Runs
```bash
# Check batch log
tail -f geant4/batch.log

# Check for errors
grep -i "error" geant4/batch.log

# Count completed runs
ls geant4/outputs/ | wc -l

# Check detector output file
head geant4/outputs/detector_hits.txt
```

---

## Data Loading in Python

### Load CORSIKA Data
```python
import panama as pn
import pandas as pd
from pathlib import Path

# Load single DAT file
dat = pn.read_DAT("corsika/outputs/gammas/DAT000001")

# Inspect structure
print(dat.columns)      # Column names
print(dat.shape)        # (num_particles, num_columns)
print(dat.head())       # First 5 rows

# Get particle types
particle_types = dat.particle.unique()
print(f"Particles: {particle_types}")

# Filter by particle type
gammas = dat[dat.particle == 1]        # Photons
electrons = dat[dat.particle == 3]     # Electrons
muons = dat[dat.particle == 5]         # Muons
protons = dat[dat.particle == 14]      # Protons
```

### Load All CORSIKA Data
```python
import panama as pn
from pathlib import Path

project_root = Path.cwd().parent.parent  # Adjust based on notebook location
corsika_base = project_root / "corsika" / "outputs"

# Load all gamma showers
gamma_dir = corsika_base / "gammas"
gamma_files = sorted(gamma_dir.glob("DAT*"))
gamma_data = [pn.read_DAT(f) for f in gamma_files]

# Load all proton showers
proton_dir = corsika_base / "protons"
proton_files = sorted(proton_dir.glob("DAT*"))
proton_data = [pn.read_DAT(f) for f in proton_files]

# Combine into single dataframe
all_gamma = pd.concat(gamma_data, ignore_index=True)
all_proton = pd.concat(proton_data, ignore_index=True)
```

---

## Path Utilities

### Get Project Root
```python
from pathlib import Path

# From any notebook:
project_root = Path.cwd().parent.parent

# From source code:
from src.condor.paths import get_project_root
project_root = get_project_root()

# Verify resolution
print(f"Project root: {project_root}")
print(f"CORSIKA data: {project_root / 'corsika' / 'outputs'}")
print(f"Geant4 build: {project_root / 'geant4' / 'build'}")
```

### Common Paths
```python
from pathlib import Path

project_root = Path.cwd().parent.parent

# Data paths
corsika_gammas = project_root / "corsika" / "outputs" / "gammas"
corsika_protons = project_root / "corsika" / "outputs" / "protons"
geant4_outputs = project_root / "geant4" / "outputs"

# Config paths
config_dir = project_root / "config"
particle_codes = config_dir / "particle_codes.txt"
corsika_template = config_dir / "input_card_template.txt"
site_config = config_dir / "condor_site.conf"

# Source code
condor_package = project_root / "src" / "condor"
```

---

## Jupyter Notebook Tips

### Launch Notebook
```bash
# From project root
jupyter notebook

# From specific directory
cd notebooks/cosmic_rays
jupyter notebook corsika_simulation.ipynb
```

### Reload Modules
```python
# In notebook cell, reload modified modules
%load_ext autoreload
%autoreload 2

# Now imports will reflect code changes without restart
from src.condor import paths  # Will auto-reload on change
```

### Clear Notebook Output
```python
# Clear all output and restart kernel
# Menu: Kernel → Restart & Clear Output

# Or from terminal
jupyter nbconvert --to notebook --execute --inplace notebook.ipynb
```

---

## Debugging

### Check CORSIKA File Integrity
```python
import panama as pn

# Load and verify
dat = pn.read_DAT("corsika/outputs/gammas/DAT000001")

# Check particle type (should be 1 for gammas)
primary_particle = dat[dat.level == 0].particle.iloc[0]
print(f"Primary particle type: {primary_particle}")
assert primary_particle == 1, "Wrong particle type!"

# Count particle types
print(dat.particle.value_counts().sort_index())

# Check energy ranges
print(f"Min energy: {dat.energy.min():.2e} GeV")
print(f"Max energy: {dat.energy.max():.2e} GeV")
```

### Verify Geant4 Output
```bash
# Check if detector output exists
ls geant4/outputs/batch_*/gammas/DAT*/detector_hits.txt | head

# Inspect first detector output
head -n 20 geant4/outputs/batch_2026-01-21-14-30-45/gammas/DAT000001/detector_hits.txt

# Count total hits
wc -l geant4/outputs/batch_*/gammas/DAT*/detector_hits.txt | tail -1
```

### Environment Check
```bash
# Check conda environment
conda activate saphir
python --version

# Check installed packages
pip list | grep -E "panama|pandas|numpy|jupyter"

# Check Geant4 build
file geant4/build/condorSim
./geant4/build/condorSim --help  # If executable
```

### File System Check
```bash
# Verify project structure
tree -d -L 2 -I '__pycache__'

# Check disk usage
du -sh ./*

# Verify symlinks
ls -la | grep '\->'
```

---

## Common Issues & Fixes

### "Module not found" error
```bash
# Add src to Python path
export PYTHONPATH="/path/to/condor-sim/src:$PYTHONPATH"

# Or install as editable package
pip install -e /path/to/condor-sim/
```

### Jupyter kernel not found
```bash
# Install kernel for saphir environment
conda activate saphir
python -m ipykernel install --user --name saphir

# Restart Jupyter and select "Python (saphir)" kernel
```

### CORSIKA file not found
```bash
# Verify files exist
ls -la corsika/outputs/gammas/DAT000001
ls -la corsika/outputs/protons/DAT000031

# Check permissions
stat corsika/outputs/gammas/DAT000001

# If missing, regenerate:
cd corsika/simulation && python3 run_simulations.py
```

### Geant4 build issues
```bash
# Clean rebuild
rm -rf geant4/build
mkdir geant4/build && cd geant4/build
cmake ..
make -j4

# Check build artifacts
ls geant4/build/condorSim
file geant4/build/condorSim
```

---

## Documentation Index

| Document | Purpose | Location |
|----------|---------|----------|
| README | Project overview & quick start | `README.md` |
| INSTALLATION | Setup guide | `documentation/INSTALLATION.md` |
| CHANGELOG | Development history | `documentation/CHANGELOG.md` |
| CONTRIBUTING | Dev guidelines | `documentation/CONTRIBUTING.md` |
| PROJECT_REVIEW | Structure assessment | `documentation/PROJECT_REVIEW.md` |
| project_overview | Physics & architecture | `documentation/project_overview.md` |
| corsika_setup | CORSIKA configuration | `documentation/corsika_setup.md` |
| geant4_setup | Geant4 build details | `documentation/geant4_setup.md` |
| detector_specs | Detector specifications | `documentation/detector_specs.md` |
| references | Physics bibliography | `documentation/references.bib` |

---

## Useful Commands Summary

```bash
# Activate environment
conda activate saphir

# Navigate to project
cd /path/to/condor-sim

# Check CORSIKA data
ls corsika/outputs/gammas/DAT* | wc -l

# Launch Jupyter
jupyter notebook notebooks/cosmic_rays/corsika_simulation.ipynb

# Run Geant4 batch simulation
bash geant4/scripts/run_batch_simulation.sh

# Monitor progress
tail -f geant4/batch.log

# Check Python path
python -c "from src.condor.paths import get_project_root; print(get_project_root())"

# View requirements
cat requirements.txt
```

---

*Last updated: 2026-01-21*
