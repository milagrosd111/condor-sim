"""
Configuration for CONDOR Observatory CORSIKA Simulations

Observatory Details:
- Location: Cerro Toco, Atacama Desert, Chile (-23.863°S, -68.269°W)
- Altitude: 5,300 m a.s.l.
- Magnetic Field: H=25.5 μT, Z=-31.2 μT (Atacama region)
"""

from pathlib import Path

# ============================================================================
# PATHS
# ============================================================================
PROJECT_ROOT = Path(__file__).parent.parent.parent
CORSIKA_DIR = PROJECT_ROOT / 'corsika' / 'corsika-78050'
CORSIKA_EXE = CORSIKA_DIR / 'run' / 'corsika78050Linux_EPOS_urqmd'
EPOS_DIR = CORSIKA_DIR / 'epos'
OUTPUT_BASE = PROJECT_ROOT / 'corsika' / 'outputs'

# ============================================================================
# OBSERVATORY PARAMETERS
# ============================================================================
OBSERVATORY = {
    'name': 'CONDOR',
    'latitude': -23.863,      # degrees (Cerro Toco, Atacama)
    'longitude': -68.269,     # degrees (Cerro Toco, Atacama)
    'altitude': 5300,         # meters (Cerro Toco elevation)
    'observation_level': 530000.0,  # cm (5300 m)
}

# Magnetic field (Cerro Toco, Atacama - IGRF-13, 2025)
MAGNETIC_FIELD = {
    'bx': 25.5,    # μT (horizontal component, North)
    'bz': -31.2,   # μT (vertical component, downward)
}

# ============================================================================
# PRIMARY PARTICLES
# ============================================================================
# PDG IDs for PANAMA (not CORSIKA IDs)
PRIMARY_PARTICLES = {
    'gamma': 22,     # PDG ID for photon/gamma
    'proton': 2212,  # PDG ID for proton
}

# CORSIKA particle IDs (for CORSIKA input card PRMPAR field)
CORSIKA_PARTICLE_IDS = {
    'gamma': 1,      # CORSIKA ID for photon/gamma
    'proton': 14,    # CORSIKA ID for proton
}

# ============================================================================
# SIMULATION PARAMETERS
# ============================================================================

# Energy ranges (GeV) for different primaries
ENERGY_RANGES = {
    'gamma': {
        'min': 20.0,
        'max': 800.0,
    },
    'proton': {
        'min': 20.0,
        'max': 800.0,
    }
}

# Zenith angle ranges (degrees)
# CONDOR: 0° (vertical) to 60° (maximum for good reconstruction)
ZENITH_RANGES = {
    'min': 0.0,
    'max': 60.0,
}

# Azimuth angle range (degrees) - full coverage
AZIMUTH_RANGE = {
    'min': -180.0,
    'max': 180.0,
}

# Number of showers per simulation
N_SHOWERS_DEFAULT = 100

# ============================================================================
# CORSIKA PARAMETERS
# ============================================================================
CORSIKA_PARAMS = {
    'eslope': -2.0,          # Energy spectrum slope (E^-2)
    'ecuts': [0.3, 0.3, 0.003, 0.003],  # Energy cuts (GeV): hadron, muon, electron, photon
    'hadflg': [0, 0, 0, 0, 0, 2],        # Hadronic interaction flags (EPOS+UrQMD)
    'longi': 'T  20.  F  T',             # Longitudinal distribution
    'ectmap': 1.E5,                       # Gamma factor cut
    'maxprt': 1,                          # Max printed events
    'debug': 'F  6  F  1000000',         # Debug flags
}

# ============================================================================
# PANAMA SETTINGS
# ============================================================================
PANAMA_CONFIG = {
    'n_jobs': 4,         # Number of parallel jobs (adjust based on CPU cores)
    'seed_base': 12345,  # Base random seed
}
