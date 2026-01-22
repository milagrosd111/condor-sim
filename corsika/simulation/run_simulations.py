#!/usr/bin/env python
"""
CONDOR Observatory - CORSIKA Simulation Runner

Comprehensive script to run CORSIKA air shower simulations for the CONDOR Observatory.
Supports both custom single/batch runs and production-scale simulations from the paper.

USAGE MODES:
============

1. SINGLE ZENITH ANGLE (custom):
   python run_simulations.py --primary gamma --zenith 0 --n-showers 100
   
2. ZENITH ANGLE RANGE (custom):
   python run_simulations.py --primary gamma --zenith-range 0 60 10 --n-showers 100
   
3. MULTIPLE PRIMARIES (custom):
   python run_simulations.py --all --n-showers 100
   
4. PRODUCTION BATCH (paper specification):
   python run_simulations.py --production
   
   Runs full production matching arXiv:2503.08850:
   - Primaries: gamma, proton
   - Energy: 20 GeV to 800 GeV
   - Zenith: 0° to 60° in 2° steps
   - Showers: 1,000 per configuration
   - Total: 62 configurations x 1,000 = 62,000 showers

FEATURES:
=========
- Sequential numbering of output files (DAT000001, DAT000002, ...)
- Direct CORSIKA execution (no parallelization wrapper)
- Full CORSIKA output captured in .log files
- Flat output directory structure (corsika/outputs/)

OUTPUT FILES:
=============
For each simulation:
  - DATxxxxxx:      Binary shower data
  - DATxxxxxx.long: Longitudinal energy deposit profile
  - DATxxxxxx.log:  Complete CORSIKA output, processing log, and input card

"""

import argparse
import sys
from pathlib import Path
from datetime import datetime
import subprocess
import shutil

# Add current directory to path for imports
sys.path.insert(0, str(Path(__file__).parent))

from config import (
    CORSIKA_EXE, EPOS_DIR, OUTPUT_BASE, PRIMARY_PARTICLES,
    CORSIKA_PARTICLE_IDS, ENERGY_RANGES, ZENITH_RANGES, AZIMUTH_RANGE,
    OBSERVATORY, MAGNETIC_FIELD, CORSIKA_PARAMS,
    PANAMA_CONFIG, N_SHOWERS_DEFAULT, PROJECT_ROOT
)




def generate_input_card(primary, zenith_min, zenith_max, n_showers, output_dir, run_number=1):
    """
    Generate CORSIKA input card for direct execution.
    
    CRITICAL: 
    - Use ABSOLUTE paths for EPOS
    - fname commands MUST come BEFORE input command
    - pathnx must have trailing slash
    """
    energy = ENERGY_RANGES[primary]
    corsika_id = CORSIKA_PARTICLE_IDS[primary]
    
    # Generate fixed values for this run
    card = f"""RUNNR   {run_number}
EVTNR   1
NSHOW   {n_showers}
PRMPAR  {corsika_id}
ESLOPE  {CORSIKA_PARAMS['eslope']}
ERANGE  {energy['min']:.1f}  {energy['max']:.1f}
THETAP  {zenith_min:.1f}  {zenith_max:.1f}
PHIP    {AZIMUTH_RANGE['min']:.1f}  {AZIMUTH_RANGE['max']:.1f}
SEED    {run_number * 2 + 1}  0  0
SEED    {run_number * 2 + 2}  0  0
OBSLEV  {OBSERVATORY['observation_level']}
MAGNET  {MAGNETIC_FIELD['bx']}  {MAGNETIC_FIELD['bz']}
HADFLG  {' '.join(map(str, CORSIKA_PARAMS['hadflg']))}
ECUTS   {' '.join(map(str, CORSIKA_PARAMS['ecuts']))}
MUADDI  T
MUMULT  T
ELMFLG  T   T
STEPFC  1.0
DIRECT  {output_dir}/
EPOPAR fname pathnx {EPOS_DIR}/
EPOPAR fname inics {EPOS_DIR}/epos.inics
EPOPAR fname iniev {EPOS_DIR}/epos.iniev
EPOPAR fname initl {EPOS_DIR}/epos.initl
EPOPAR fname inirj {EPOS_DIR}/epos.inirj
EPOPAR fname hpf {EPOS_DIR}/urqmd34/tables.dat
EPOPAR fname check none
EPOPAR fname histo none
EPOPAR fname data none
EPOPAR fname copy none
EPOPAR input {EPOS_DIR}/epos.param
LONGI   {CORSIKA_PARAMS['longi']}
ECTMAP  {CORSIKA_PARAMS['ectmap']}
MAXPRT  {CORSIKA_PARAMS['maxprt']}
USER    condor
DEBUG   {CORSIKA_PARAMS['debug']}
EXIT
"""
    return card


def get_next_run_number():
    """Find the highest existing DAT file number and return the next available number."""
    output_dir = OUTPUT_BASE
    if not output_dir.exists():
        return 1
    
    # Find all DAT files matching pattern DATxxxxxx (exactly 6 digits) in all subdirectories
    import re
    dat_pattern = re.compile(r'^DAT(\d{6})$')
    
    max_number = 0
    for file in output_dir.rglob('DAT*'):  # Recursive glob
        if file.is_file() and dat_pattern.match(file.name):
            match = dat_pattern.match(file.name)
            if match:
                num = int(match.group(1))
                max_number = max(max_number, num)
    
    return max_number + 1


def run_simulation(primary, zenith_min, zenith_max, n_showers, run_number, dry_run=False):
    """Run a single CORSIKA simulation directly (without PANAMA parallelization)."""
    
    # Organize outputs by particle type: gammas/ or protons/
    particle_dir = f"{primary}s" 
    output_dir = OUTPUT_BASE / particle_dir
    
    # Ensure output directory exists
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Generate input card
    input_card = generate_input_card(
        primary=primary,
        zenith_min=zenith_min,
        zenith_max=zenith_max,
        n_showers=n_showers,
        output_dir=output_dir,
        run_number=run_number
    )
    
    log_file = output_dir / f'DAT{run_number:06d}.log'
    
    print("="*80)
    print(f"Simulation Configuration:")
    print(f"  Primary:        {primary} (PDG {PRIMARY_PARTICLES[primary]}, CORSIKA {CORSIKA_PARTICLE_IDS[primary]})")
    print(f"  Energy:         {ENERGY_RANGES[primary]['min']}-{ENERGY_RANGES[primary]['max']} GeV")
    print(f"  Zenith:         {zenith_min}° - {zenith_max}°")
    print(f"  Showers:        {n_showers}")
    print(f"  RUNNR:          {run_number}")
    print(f"  Output dir:     {output_dir}")
    print(f"  Expected files: DAT{run_number:06d}, DAT{run_number:06d}.long, DAT{run_number:06d}.log")
    print("="*80)
    
    if dry_run:
        print("\n✓ Dry run - input card will be in log file")
        return True
    
    # Run CORSIKA directly
    print(f"\nStarting CORSIKA simulation...")
    
    try:
        import subprocess
        
        # Run CORSIKA with input from file, capture output to log
        with open(log_file, 'w') as log:
            log.write(f"CONDOR Observatory CORSIKA Simulation\n")
            log.write(f"{'='*60}\n")
            log.write(f"Start time:     {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            log.write(f"Primary:        {primary} (CORSIKA {CORSIKA_PARTICLE_IDS[primary]})\n")
            log.write(f"Energy:         {ENERGY_RANGES[primary]['min']}-{ENERGY_RANGES[primary]['max']} GeV\n")
            log.write(f"Zenith:         {zenith_min}° - {zenith_max}°\n")
            log.write(f"Showers:        {n_showers}\n")
            log.write(f"RUNNR:          {run_number}\n")
            log.write(f"{'='*60}\n\n")
            log.flush()
            
            # Run CORSIKA
            process = subprocess.Popen(
                [str(CORSIKA_EXE)],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                cwd=str(CORSIKA_EXE.parent),  # Run from CORSIKA's run/ directory
                text=True
            )
            
            # Send input card and capture output
            stdout, _ = process.communicate(input=input_card)
            
            # Write CORSIKA output to log
            log.write(stdout)
            
            # Write completion
            log.write(f"\n{'='*60}\n")
            log.write(f"End time:       {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            log.write(f"Exit code:      {process.returncode}\n")
            
            if process.returncode == 0:
                log.write(f"Status:         SUCCESS ✓\n")
            else:
                log.write(f"Status:         FAILED\n")
            log.write(f"{'='*60}\n")
        
        if process.returncode != 0:
            print(f"\n✗ CORSIKA failed with exit code {process.returncode}")
            print(f"  Check log: {log_file}")
            return False
        
        print(f"\n✓ Simulation completed successfully!")
        
        # Verify output files
        dat_file = output_dir / f'DAT{run_number:06d}'
        long_file = output_dir / f'DAT{run_number:06d}.long'
        
        if dat_file.exists() and long_file.exists():
            dat_size = dat_file.stat().st_size / 1024 / 1024
            long_size = long_file.stat().st_size / 1024 / 1024
            print(f"\nOutput files:")
            print(f"  {dat_file.name}:      {dat_size:.2f} MB")
            print(f"  {long_file.name}: {long_size:.2f} MB")
            print(f"  {log_file.name}:      log file created")
            return True
        else:
            print(f"\n✗ Warning: Expected output files not found")
            if not dat_file.exists():
                print(f"  Missing: {dat_file.name}")
            if not long_file.exists():
                print(f"  Missing: {long_file.name}")
            return False
            
    except Exception as e:
        # Log error to file
        with open(log_file, 'a') as log:
            log.write(f"\n{'='*60}\n")
            log.write(f"End time:       {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            log.write(f"Status:         FAILED\n")
            log.write(f"Error:          {e}\n")
            log.write(f"{'='*60}\n")
            import traceback
            log.write("\nTraceback:\n")
            traceback.print_exc(file=log)
        
        print(f"\nERROR: Simulation failed")
        print(f"  {e}")
        import traceback
        traceback.print_exc()
        return False


def parse_arguments():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description='Run CORSIKA simulations for CONDOR Observatory',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    
    # Production mode (new)
    parser.add_argument(
        '--production',
        action='store_true',
        help='Run full production batch (arXiv:2503.08850): 62 configurations x 1,000 showers'
    )
    
    # Primary particle
    parser.add_argument(
        '--primary',
        choices=['gamma', 'proton'],
        default='gamma',
        help='Primary particle type (default: gamma) [ignored if --production]'
    )
    
    parser.add_argument(
        '--all',
        action='store_true',
        help='Run all primaries with default zenith range [ignored if --production]'
    )
    
    # Zenith angle
    parser.add_argument(
        '--zenith',
        type=float,
        default=0.0,
        help='Single zenith angle (degrees, default: 0) [ignored if --production or --zenith-range]'
    )
    
    parser.add_argument(
        '--zenith-range',
        nargs=3,
        type=float,
        metavar=('MIN', 'MAX', 'STEP'),
        help='Zenith angle range: min max step (degrees) [ignored if --production]'
    )
    
    # Number of showers
    parser.add_argument(
        '--n-showers',
        type=int,
        default=N_SHOWERS_DEFAULT,
        help=f'Number of showers per simulation (default: {N_SHOWERS_DEFAULT}) [ignored if --production]'
    )
    
    # Dry run
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Show what would be run without executing'
    )
    
    args = parser.parse_args()
    
    # Validation
    if args.production:
        # Production mode: ignore other options
        if args.zenith_range or args.zenith != 0.0 or args.all or args.primary != 'gamma':
            print("NOTE: --production mode overrides all other options (--primary, --zenith, --zenith-range, --all)")
    else:
        # Custom mode: validate options
        if args.zenith_range and args.zenith != 0.0:
            parser.error("Cannot specify both --zenith and --zenith-range")
    
    return args




def run_production_batch(dry_run=False):
    """Run full production batch matching paper specification (arXiv:2503.08850)."""
    
    # Production parameters
    PRIMARIES = ['gamma', 'proton']
    ZENITH_MIN = 0
    ZENITH_MAX = 60
    ZENITH_STEP = 2
    SHOWERS_PER_CONFIG = 1000
    
    print("\n" + "="*80)
    print("CONDOR OBSERVATORY - PRODUCTION SIMULATION BATCH")
    print("="*80)
    print()
    print("Paper reference: arXiv:2503.08850")
    print("'CONDOR: A ground-based gamma-ray observatory at 5300m altitude'")
    print()
    print("SIMULATION SPECIFICATION:")
    print(f"  Primaries:           {', '.join(PRIMARIES)}")
    print(f"  Energy range:        20 GeV to 800 GeV")
    print(f"  Zenith angles:       {ZENITH_MIN}° to {ZENITH_MAX}° in {ZENITH_STEP}° steps")
    print(f"  Showers per config:  {SHOWERS_PER_CONFIG:,}")
    print()
    
    # Calculate total simulations
    zenith_angles = list(range(ZENITH_MIN, ZENITH_MAX + 1, ZENITH_STEP))
    n_zenith = len(zenith_angles)
    n_configs = len(PRIMARIES) * n_zenith
    total_showers = n_configs * SHOWERS_PER_CONFIG
    
    print("SUMMARY:")
    print(f"  Zenith angles:       {n_zenith} ({zenith_angles})")
    print(f"  Total configurations: {n_configs} ({len(PRIMARIES)} primaries × {n_zenith} zenith angles)")
    print(f"  Total showers:       {total_showers:,}")
    print()
    print("OUTPUT:")
    print(f"  Location: corsika/outputs/")
    print(f"  Files: DAT000001, DAT000001.long, DAT000001.log, ... (sequential numbering)")
    print()
    print("RUNTIME ESTIMATE:")
    print("  ~2-4 hours on a 4-core system (actual time depends on CPU speed)")
    print("  Progress will be printed as each simulation completes.")
    print()
    print("="*80)
    print()
    
    # Find the next available run number based on existing DAT files
    start_runnr = get_next_run_number()
    if start_runnr > 1:
        print(f"NOTE: Found existing DAT files. Continuing from DAT{start_runnr:06d}")
        print()
    
    # Run simulations
    start_time = datetime.now()
    completed = 0
    failed = 0
    
    for i, primary in enumerate(PRIMARIES):
        for zenith in zenith_angles:
            # Convert single zenith to range (single point)
            config_num = i * n_zenith + (zenith - ZENITH_MIN) // ZENITH_STEP + 1
            total_configs = n_configs
            runnr = start_runnr + config_num - 1
            
            print(f"[{config_num:2d}/{total_configs}] {primary:8s} zenith {zenith:5.1f}° (DAT{runnr:06d})...", end=' ', flush=True)
            
            try:
                success = run_simulation(
                    primary=primary,
                    zenith_min=float(zenith),
                    zenith_max=float(zenith),
                    n_showers=SHOWERS_PER_CONFIG,
                    run_number=runnr,
                    dry_run=dry_run
                )
                if success:
                    completed += 1
                    print("✓")
                else:
                    failed += 1
                    print("✗")
            except Exception as e:
                failed += 1
                print(f"✗ Error: {e}")
    
    # Summary
    elapsed = datetime.now() - start_time
    hours = elapsed.total_seconds() / 3600
    
    print()
    print("="*80)
    print("PRODUCTION BATCH COMPLETE")
    print("="*80)
    print(f"Completed:  {completed}/{n_configs} configurations")
    print(f"Failed:     {failed}/{n_configs} configurations")
    print(f"Runtime:    {hours:.1f} hours ({elapsed})")
    print()
    
    if completed == n_configs:
        print("✓ All simulations completed successfully!")
        print()
        return 0
    else:
        print(f"⚠ {failed} simulation(s) failed. Review errors above and retry if needed.")
        return 1


def main():
    """Main execution."""
    args = parse_arguments()
    
    # Check if production mode
    if args.production:
        return run_production_batch(dry_run=args.dry_run)
    
    print("\n" + "="*80)
    print("CONDOR Observatory - CORSIKA Simulation Runner")
    print("="*80)
    print(f"CORSIKA:        {CORSIKA_EXE}")
    print(f"EPOS:           {EPOS_DIR}")
    print(f"Output base:    {OUTPUT_BASE}")
    print("="*80 + "\n")
    
    # Determine primaries
    if args.all:
        primaries = list(PRIMARY_PARTICLES.keys())
    else:
        primaries = [args.primary]
    
    # Determine zenith angles
    if args.all:
        zenith_angles = [(0, 10), (10, 20), (20, 30), (30, 40), (40, 50), (50, 60)]
    elif args.zenith_range:
        zmin, zmax, zstep = args.zenith_range
        zenith_angles = [(z, min(z + zstep, zmax)) 
                        for z in range(int(zmin), int(zmax), int(zstep))]
    else:
        # Single zenith (default or specified)
        z = args.zenith
        zenith_angles = [(z, z)]
    
    # Run simulations
    total = len(primaries) * len(zenith_angles)
    success_count = 0
    batch_output_dirs = []
    
    print(f"Planning {total} simulation(s):\n")
    for primary in primaries:
        for zmin, zmax in zenith_angles:
            print(f"  - {primary:8s}  zenith {zmin:5.1f}° - {zmax:5.1f}°")
    
    # No confirmation prompt to keep CLI non-interactive
    print("\n")
    
    # Find the next available run number based on existing DAT files
    start_runnr = get_next_run_number()
    if start_runnr > 1:
        print(f"NOTE: Found existing DAT files. Starting from DAT{start_runnr:06d}\n")
    
    # Use simple sequential numbering for RUNNR
    for i, (primary, (zmin, zmax)) in enumerate(
        [(p, z) for p in primaries for z in zenith_angles], 1
    ):
        runnr = start_runnr + i - 1
        print(f"\n[{i}/{total}] Running: {primary}, zenith {zmin}°-{zmax}°, RUNNR {runnr:06d}")
        success = run_simulation(
            primary=primary,
            zenith_min=zmin,
            zenith_max=zmax,
            n_showers=args.n_showers,
            run_number=runnr,
            dry_run=args.dry_run
        )
        if success:
            success_count += 1
            # Track the specific output directory for this run
            # Match the latest directory created for this primary/zenith in OUTPUT_BASE
            # Pattern matches our naming: YYYYMMDD_HHMMSS_mmm_primary_zAA-BB
            pattern = f"{datetime.now().strftime('%Y%m%d_')}*_{primary}_z{int(zmin):02d}-{int(zmax):02d}"
            matches = sorted([d for d in OUTPUT_BASE.glob(pattern) if d.is_dir()])
            if matches:
                batch_output_dirs.append(matches[-1])
    
    # Summary
    print("\n" + "="*80)
    print("BATCH SIMULATION SUMMARY")
    print("="*80)
    print(f"Total simulations:  {total}")
    print(f"Successful:         {success_count}")
    print(f"Failed:             {total - success_count}")
    print("="*80 + "\n")
    
    if success_count == total:
        print("All simulations completed successfully!")
    elif success_count > 0:
        print("Some simulations failed")
    else:
        print("All simulations failed")




if __name__ == '__main__':
    main()
