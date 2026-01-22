"""CORSIKA file I/O utilities."""

import panama as pn
from pathlib import Path
from typing import List, Tuple
import pandas as pd


def find_dat_files(output_dir: Path, pattern: str = "DAT*") -> List[Path]:
    """Find all CORSIKA DAT files in output directory.
    
    Supports both flat structure (all DAT files in output_dir directly)
    and subdirectory structure (DAT files in dated subdirectories).
    
    Parameters
    ----------
    output_dir : Path
        Directory containing CORSIKA outputs
    pattern : str
        Glob pattern for DAT files (default: "DAT*")
        
    Returns
    -------
    List[Path]
        Sorted list of DAT file paths (excluding .long and .log files)
    """
    dat_files = []
    
    # First, try flat structure - DAT files directly in output_dir
    for dat_file in output_dir.glob(pattern):
        if dat_file.is_file() and not dat_file.name.endswith(('.long', '.log', '.txt')):
            dat_files.append(dat_file)
    
    # If no files found in flat structure, try subdirectory structure
    if not dat_files:
        for run_dir in output_dir.glob("*"):
            if run_dir.is_dir():
                for dat_file in run_dir.glob(pattern):
                    if dat_file.is_file() and not dat_file.name.endswith(('.long', '.log', '.txt')):
                        dat_files.append(dat_file)
    
    return sorted(dat_files)


def read_corsika_data(
    dat_files: List[Path],
    mother_columns: bool = False,
    additional_columns: bool = True
) -> Tuple[pd.DataFrame, pd.DataFrame, pd.DataFrame]:
    """Read CORSIKA DAT files using PANAMA.
    
    Parameters
    ----------
    dat_files : List[Path]
        List of DAT files to read
    mother_columns : bool
        Whether to include mother particle information
    additional_columns : bool
        Whether to add additional calculated columns
        
    Returns
    -------
    run_header : pd.DataFrame
        Run header information
    event_header : pd.DataFrame
        Event header information
    particles : pd.DataFrame
        Particle data
    """
    if not dat_files:
        raise ValueError("No DAT files provided")
    
    # Convert to strings for PANAMA
    dat_files_str = [str(f) for f in dat_files]
    
    # Read with PANAMA
    run_header, event_header, particles = pn.read_DAT(
        files=dat_files_str,
        mother_columns=mother_columns,
        additional_columns=additional_columns
    )
    
    return run_header, event_header, particles


def save_processed_data(
    run_header: pd.DataFrame,
    event_header: pd.DataFrame,
    particles: pd.DataFrame,
    output_dir: Path,
    compression: str = 'snappy'
) -> None:
    """Save processed CORSIKA data to Parquet files.
    
    Parameters
    ----------
    run_header : pd.DataFrame
        Run header DataFrame
    event_header : pd.DataFrame
        Event header DataFrame
    particles : pd.DataFrame
        Particles DataFrame
    output_dir : Path
        Directory to save files
    compression : str
        Compression algorithm (snappy, gzip, etc.)
    """
    output_dir.mkdir(parents=True, exist_ok=True)
    
    particles.to_parquet(output_dir / "particles.parquet", compression=compression)
    event_header.to_parquet(output_dir / "event_header.parquet", compression=compression)
    run_header.to_parquet(output_dir / "run_header.parquet", compression=compression)


def load_processed_data(input_dir: Path) -> Tuple[pd.DataFrame, pd.DataFrame, pd.DataFrame]:
    """Load processed CORSIKA data from Parquet files.
    
    Parameters
    ----------
    input_dir : Path
        Directory containing processed Parquet files
        
    Returns
    -------
    run_header : pd.DataFrame
        Run header DataFrame
    event_header : pd.DataFrame
        Event header DataFrame
    particles : pd.DataFrame
        Particles DataFrame
    """
    particles = pd.read_parquet(input_dir / "particles.parquet")
    event_header = pd.read_parquet(input_dir / "event_header.parquet")
    run_header = pd.read_parquet(input_dir / "run_header.parquet")
    
    return run_header, event_header, particles
