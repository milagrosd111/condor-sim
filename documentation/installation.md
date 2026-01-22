# Installation Guide

Complete setup instructions for the CONDOR Simulation Framework.

---

## Prerequisites

- **OS**: Linux (Ubuntu 20.04+ recommended) or macOS
- **Python**: 3.10 or higher
- **Compiler**: GCC/Clang with C++17 support
- **CMake**: 3.16 or higher
- **Conda**: Anaconda or Miniconda for environment management

---

## Step 1: Clone & Setup Repository

```bash
# Navigate to SAPHIR directory
cd /path/to/Documents/SAPHIR

# Repository already exists, update if needed
cd condor-sim
git pull origin main
```

---

## Step 2: Create Conda Environment

```bash
# Create environment with Python 3.10
conda create -n saphir python=3.10 -y

# Activate environment
conda activate saphir
```

---

## Step 3: Install Python Dependencies

```bash
cd /path/to/condor-sim

# Install from requirements.txt
pip install -r requirements.txt
```

### Dependencies Overview
- **panama**: CORSIKA data reading (air shower analysis)
- **pandas**: Data manipulation and analysis
- **numpy**: Numerical computing
- **matplotlib**: Data visualization
- **jupyter**: Interactive notebooks
- **scipy**: Scientific computing

---

## Step 4: Build Geant4 (Optional)

Geant4 is already pre-built in the project. If you need to rebuild:

```bash
cd geant4

# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Compile (takes ~10-15 minutes)
make -j4
```

The compiled executable will be at `geant4/build/condorSim`.

---

## Step 5: Verify Installation

```bash
# Check CORSIKA outputs exist
ls corsika/outputs/gammas/DAT* | wc -l    # Should show 30
ls corsika/outputs/protons/DAT* | wc -l   # Should show 30

# Launch a test notebook
cd notebooks/cosmic_rays
jupyter notebook corsika_simulation.ipynb
```

---

## Troubleshooting

### Python Module Import Errors
```bash
# Add src/ to Python path
export PYTHONPATH="/path/to/condor-sim/src:$PYTHONPATH"

# Or install as development package
cd /path/to/condor-sim
pip install -e .
```

### Jupyter Kernel Issues
```bash
# Install IPython kernel in conda environment
conda activate saphir
python -m ipykernel install --user --name saphir --display-name "Python (saphir)"
```

### Geant4 Build Failures
- Ensure CMake ≥ 3.16: `cmake --version`
- Ensure GCC ≥ 9.0: `gcc --version`
- Try: `rm -rf geant4/build && mkdir geant4/build && cd geant4/build && cmake .. && make -j4`

### CORSIKA Data Not Found
```bash
# Verify path resolution
python3 -c "from src.condor.paths import get_project_root; print(get_project_root())"
# Should print: /path/to/condor-sim
```

---

## Development Environment

For code development and testing:

```bash
# Install development dependencies
pip install -r requirements-dev.txt

# This includes: pytest, black, flake8, mypy
```

---

## Next Steps

1. Read [README.md](../README.md) for project overview
2. Check [QUICKREF.md](QUICKREF.md) for common commands
3. Review [project_overview.md](project_overview.md) for physics context
4. Start with [corsika_simulation.ipynb](../notebooks/cosmic_rays/corsika_simulation.ipynb)
