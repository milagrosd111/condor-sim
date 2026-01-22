"""
Simulation configuration bridge for the CONDOR package.

Imports the authoritative CORSIKA simulation settings so callers can use:
    from condor.simulation import OBSERVATORY, ENERGY_RANGES
"""

from .config import (
    PROJECT_ROOT,
    CORSIKA_DIR,
    CORSIKA_EXE,
    EPOS_DIR,
    OUTPUT_BASE,
    OBSERVATORY,
    MAGNETIC_FIELD,
    PRIMARY_PARTICLES,
    CORSIKA_PARTICLE_IDS,
    ENERGY_RANGES,
    ZENITH_RANGES,
    AZIMUTH_RANGE,
    N_SHOWERS_DEFAULT,
    CORSIKA_PARAMS,
    PANAMA_CONFIG,
    as_dict,
)

__all__ = [
    "PROJECT_ROOT",
    "CORSIKA_DIR",
    "CORSIKA_EXE",
    "EPOS_DIR",
    "OUTPUT_BASE",
    "OBSERVATORY",
    "MAGNETIC_FIELD",
    "PRIMARY_PARTICLES",
    "CORSIKA_PARTICLE_IDS",
    "ENERGY_RANGES",
    "ZENITH_RANGES",
    "AZIMUTH_RANGE",
    "N_SHOWERS_DEFAULT",
    "CORSIKA_PARAMS",
    "PANAMA_CONFIG",
    "as_dict",
]
