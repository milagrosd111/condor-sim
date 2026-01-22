"""Centralized path management for CONDOR simulation project."""

from pathlib import Path
import os

# Project root is 2 levels up from this file
PROJECT_ROOT = Path(__file__).parent.parent.parent.resolve()

# Main directories
DATA_DIR = PROJECT_ROOT / "data"
RESULTS_DIR = PROJECT_ROOT / "results"
CONFIG_DIR = PROJECT_ROOT / "config"

# CORSIKA-specific paths
CORSIKA_RAW_DIR = DATA_DIR / "raw" / "corsika"
CORSIKA_PROCESSED_DIR = DATA_DIR / "processed" / "corsika"
CORSIKA_RESULTS_DIR = RESULTS_DIR / "corsika"

# Legacy paths (for backward compatibility)
LEGACY_CORSIKA_OUTPUTS = PROJECT_ROOT / "corsika" / "outputs"
LEGACY_ANALYSIS_DATA = PROJECT_ROOT / "analysis" / "corsika" / "data" / "processed"
LEGACY_ANALYSIS_RESULTS = PROJECT_ROOT / "analysis" / "corsika" / "results"

# Geant4-specific paths
GEANT4_RAW_DIR = DATA_DIR / "raw" / "geant4"
GEANT4_PROCESSED_DIR = DATA_DIR / "processed" / "geant4"
GEANT4_RESULTS_DIR = RESULTS_DIR / "geant4"


def ensure_directories():
    """Create necessary directories if they don't exist."""
    dirs_to_create = [
        CORSIKA_RAW_DIR,
        CORSIKA_PROCESSED_DIR,
        CORSIKA_RESULTS_DIR / "figures",
        CORSIKA_RESULTS_DIR / "tables",
        GEANT4_RAW_DIR,
        GEANT4_PROCESSED_DIR,
        GEANT4_RESULTS_DIR / "figures",
        GEANT4_RESULTS_DIR / "tables",
    ]
    
    for directory in dirs_to_create:
        directory.mkdir(parents=True, exist_ok=True)


def get_corsika_output_dir(use_legacy=True):
    """Get CORSIKA output directory.
    
    Parameters
    ----------
    use_legacy : bool
        If True, use legacy path (corsika/outputs/), otherwise use new structure.
    
    Returns
    -------
    Path
        Path to CORSIKA output directory
    """
    if use_legacy and LEGACY_CORSIKA_OUTPUTS.exists():
        return LEGACY_CORSIKA_OUTPUTS
    return CORSIKA_RAW_DIR


def get_processed_dir(use_legacy=True):
    """Get processed data directory.
    
    Parameters
    ----------
    use_legacy : bool
        If True, use legacy path, otherwise use new structure.
    
    Returns
    -------
    Path
        Path to processed data directory
    """
    if use_legacy and LEGACY_ANALYSIS_DATA.exists():
        return LEGACY_ANALYSIS_DATA
    return CORSIKA_PROCESSED_DIR


def get_results_dir(use_legacy=True):
    """Get results directory.
    
    Parameters
    ----------
    use_legacy : bool
        If True, use legacy path, otherwise use new structure.
    
    Returns
    -------
    Path
        Path to results directory
    """
    if use_legacy and LEGACY_ANALYSIS_RESULTS.exists():
        return LEGACY_ANALYSIS_RESULTS
    return CORSIKA_RESULTS_DIR
