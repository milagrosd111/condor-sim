#!/usr/bin/env python3
"""
Convert CORSIKA event headers to Geant4 primary particle input format.

CORSIKA primary cosmic rays are injected into Geant4 to simulate their
propagation through the detector.
"""

import panama
import pandas as pd
import numpy as np
from pathlib import Path
import argparse
import math

# CORSIKA particle ID to Geant4 particle mapping
# NOTE: CORSIKA uses its own particle ID system (not PDG), defined in CORSIKA 7.8050
CORSIKA_TO_G4 = {
    1: "gamma",         # Photon (gamma ray)
    2: "e+",            # Positron
    3: "e-",            # Electron
    4: "mu+",           # Antimuon
    5: "mu-",           # Muon
    6: "pi0",           # Pion0
    7: "pi+",           # Pion+
    8: "pi-",           # Pion-
    9: "K+",            # Kaon+
    10: "K0",           # Kaon0
    11: "K-",           # Kaon-
    12: "neutron",      # Neutron
    13: "anti-proton",  # Anti-proton (rare)
    14: "proton",       # Proton (primary cosmic ray)
    # Higher energy hadrons are rare as primaries, but included for completeness
    15: "lambda0",      # Lambda
    16: "Sigma+",       # Sigma+
    17: "Sigma0",       # Sigma0
    18: "Sigma-",       # Sigma-
    19: "Xi0",          # Xi0
    20: "Xi-",          # Xi-
    21: "Omega-",       # Omega-
}


def corsika_id_to_geant4(corsika_id):
    """Convert CORSIKA particle ID to Geant4 particle name."""
    corsika_id_int = int(corsika_id)
    if corsika_id_int in CORSIKA_TO_G4:
        return CORSIKA_TO_G4[corsika_id_int]
    return None  # Unsupported particle


def extract_primary_particles(corsika_files, max_events=None, output_file=None):
    """
    Extract primary cosmic ray parameters from CORSIKA DAT files.
    
    Parameters
    ----------
    corsika_files : str or list
        Path or list of CORSIKA DAT files
    max_events : int, optional
        Maximum number of events to read
    output_file : str, optional
        Output CSV file. If None, returns DataFrame
        
    Returns
    -------
    DataFrame with columns: event_id, particle_name, energy_gev, zenith_rad, 
                           azimuth_rad, altitude_cm, px_gev, py_gev, pz_gev
    """
    
    # Read CORSIKA files
    run_header, event_header, particles = panama.read_DAT(
        files=corsika_files,
        max_events=max_events,
        drop_non_particles=True,
        drop_mothers=True
    )
    
    # Extract primary particle information
    primaries = []
    
    # Group by event and take first row (primary cosmic ray)
    # event_header is indexed by (run_number, event_number)
    seen_events = set()
    
    for idx, row in event_header.iterrows():
        # idx is a tuple (run_id, event_id) or just event_id
        if isinstance(idx, tuple):
            run_id, event_id = idx
        else:
            event_id = idx
        
        # Skip duplicate events (keep only first occurrence)
        if event_id in seen_events:
            continue
        seen_events.add(event_id)
        
        # Extract parameters
        corsika_id = int(row['particle_id'])
        particle_name = corsika_id_to_geant4(corsika_id)
        
        if particle_name is None:
            print(f"Warning: Unsupported CORSIKA particle ID {corsika_id} in event {event_id}")
            continue
        
        energy_gev = float(row['total_energy'])
        zenith_rad = float(row.get('zenith', 0.0))
        azimuth_rad = float(row.get('azimuth', 0.0))
        # Particles start at observation level (Cerro Toco site, 5300 m)
        altitude_cm = 530000.0  # 5300 m
        
        # Calculate momentum components
        # In CORSIKA: momentum_z = -total_energy * cos(zenith), momentum_xy = total_energy * sin(zenith)
        momentum_mag = energy_gev
        px_gev = momentum_mag * math.sin(zenith_rad) * math.cos(azimuth_rad)
        py_gev = momentum_mag * math.sin(zenith_rad) * math.sin(azimuth_rad)
        pz_gev = -momentum_mag * math.cos(zenith_rad)  # Downward
        
        primaries.append({
            'event_id': int(event_id),
            'particle_name': particle_name,
            'energy_gev': energy_gev,
            'zenith_rad': zenith_rad,
            'azimuth_rad': azimuth_rad,
            'altitude_cm': altitude_cm,
            'px_gev': px_gev,
            'py_gev': py_gev,
            'pz_gev': pz_gev,
        })
    
    df = pd.DataFrame(primaries)
    
    if output_file:
        df.to_csv(output_file, index=False)
        print(f"Wrote {len(df)} primary particles to {output_file}")
    
    return df


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Convert CORSIKA files to Geant4 input')
    parser.add_argument('corsika_files', nargs='+', help='CORSIKA DAT file(s) or glob pattern')
    parser.add_argument('-o', '--output', help='Output CSV file', default='corsika_primaries.csv')
    parser.add_argument('-n', '--max-events', type=int, help='Maximum events to read')
    
    args = parser.parse_args()
    
    # Handle glob patterns and filter for valid DAT files
    from pathlib import Path
    files_to_process = []
    for pattern in args.corsika_files:
        if '*' in pattern or '?' in pattern:
            # Glob expansion
            matching_files = sorted(Path('.').glob(pattern))
            # Filter to only include files that match DAT format (no extension or just DAT)
            for f in matching_files:
                if f.suffix == '' or f.name.startswith('DAT'):  # Main DAT files have no suffix
                    files_to_process.append(f)
        else:
            # Direct file path
            files_to_process.append(pattern)
    
    if not files_to_process:
        print(f"Error: No valid CORSIKA DAT files found matching {args.corsika_files}")
        exit(1)
    
    # Filter out .log and .long files
    files_to_process = [f for f in files_to_process if not (str(f).endswith('.log') or str(f).endswith('.long'))]
    
    if not files_to_process:
        print(f"Error: No valid CORSIKA DAT files found (all matches were .log or .long files)")
        exit(1)
    
    df = extract_primary_particles(files_to_process, max_events=args.max_events, 
                                    output_file=args.output)
    print(f"\nRead {len(df)} primary cosmic rays")
    if len(df) > 0:
        print(f"\nFirst 5 events:")
        print(df.head())
