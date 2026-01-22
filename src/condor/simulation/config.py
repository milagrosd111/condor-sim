"""
Shared simulation configuration for the CONDOR package.

This module re-exports the authoritative settings defined in
`corsika/simulation/config.py` so downstream code can import from
`condor.simulation` without duplicating values. Keeping a single source of
truth avoids drift between the CORSIKA runner and higher-level analysis
layers.
"""

from __future__ import annotations

import importlib.util
from pathlib import Path
from types import ModuleType


def _load_corsika_config() -> ModuleType:
    """Dynamically load the CORSIKA simulation config module."""

    cfg_path = Path(__file__).resolve().parents[2] / "corsika" / "simulation" / "config.py"
    spec = importlib.util.spec_from_file_location("corsika_sim_config", cfg_path)
    if spec is None or spec.loader is None:
        raise ImportError(f"Could not load CORSIKA config from {cfg_path}")

    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


_corsika = _load_corsika_config()

# Paths
PROJECT_ROOT = _corsika.PROJECT_ROOT
CORSIKA_DIR = _corsika.CORSIKA_DIR
CORSIKA_EXE = _corsika.CORSIKA_EXE
EPOS_DIR = _corsika.EPOS_DIR
OUTPUT_BASE = _corsika.OUTPUT_BASE

# Observatory and physics parameters
OBSERVATORY = _corsika.OBSERVATORY
MAGNETIC_FIELD = _corsika.MAGNETIC_FIELD
PRIMARY_PARTICLES = _corsika.PRIMARY_PARTICLES
CORSIKA_PARTICLE_IDS = _corsika.CORSIKA_PARTICLE_IDS
ENERGY_RANGES = _corsika.ENERGY_RANGES
ZENITH_RANGES = _corsika.ZENITH_RANGES
AZIMUTH_RANGE = _corsika.AZIMUTH_RANGE
N_SHOWERS_DEFAULT = _corsika.N_SHOWERS_DEFAULT
CORSIKA_PARAMS = _corsika.CORSIKA_PARAMS
PANAMA_CONFIG = _corsika.PANAMA_CONFIG


def as_dict() -> dict:
    """Return a consolidated dict of configuration values."""

    return {
        "project_root": PROJECT_ROOT,
        "corsika_dir": CORSIKA_DIR,
        "corsika_exe": CORSIKA_EXE,
        "epos_dir": EPOS_DIR,
        "output_base": OUTPUT_BASE,
        "observatory": OBSERVATORY,
        "magnetic_field": MAGNETIC_FIELD,
        "primary_particles": PRIMARY_PARTICLES,
        "corsika_particle_ids": CORSIKA_PARTICLE_IDS,
        "energy_ranges": ENERGY_RANGES,
        "zenith_ranges": ZENITH_RANGES,
        "azimuth_range": AZIMUTH_RANGE,
        "n_showers_default": N_SHOWERS_DEFAULT,
        "corsika_params": CORSIKA_PARAMS,
        "panama_config": PANAMA_CONFIG,
    }


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
