#!/usr/bin/env python
"""
Quick validation test: run a small subset of the production spec to verify setup.

This script runs:
  - 1 gamma and 1 proton primary
  - 3 zenith angles (0°, 30°, 60°)
  - 100 showers each (instead of 1000)

Total: 6 configurations × 100 showers = 600 showers (~5-10 minutes)

Use this to verify everything is working before running the full production batch.
"""

import subprocess
import sys
from pathlib import Path
from datetime import datetime

def run_validation():
    """Run validation subset matching production spec."""
    
    print("="*80)
    print("CONDOR OBSERVATORY - VALIDATION TEST")
    print("="*80)
    print()
    print("This test validates your setup using a subset of the production spec:")
    print("  - 2 primaries (gamma, proton)")
    print("  - 3 zenith angles (0°, 30°, 60°)")
    print("  - 100 showers each (reduced from 1000 for speed)")
    print()
    print("Expected runtime: 5-10 minutes")
    print()
    print("="*80)
    print()
    
    configs = [
        ('gamma', 0.0),
        ('gamma', 30.0),
        ('gamma', 60.0),
        ('proton', 0.0),
        ('proton', 30.0),
        ('proton', 60.0),
    ]
    
    start_time = datetime.now()
    completed = 0
    failed = 0
    
    for i, (primary, zenith) in enumerate(configs, 1):
        print(f"[{i}/6] {primary:8s} zenith {zenith:5.1f}°...", end='', flush=True)
        
        cmd = [
            sys.executable,
            'run_simulations.py',
            '--primary', primary,
            '--zenith', str(zenith),
            '--n-showers', '100',
        ]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
            if result.returncode == 0:
                completed += 1
                print(" ✓")
            else:
                failed += 1
                print(f" ✗ (return code {result.returncode})")
        except subprocess.TimeoutExpired:
            failed += 1
            print(" ✗ (timeout)")
        except Exception as e:
            failed += 1
            print(f" ✗ ({e})")
    
    elapsed = datetime.now() - start_time
    minutes = elapsed.total_seconds() / 60
    
    print()
    print("="*80)
    print(f"Validation: {completed}/6 passed ({minutes:.1f} minutes)")
    print("="*80)
    print()
    
    if completed == 6:
        print("✓ Validation successful! Your setup is ready for production.")
        print()
        print("To run the full production batch:")
        print("  python run_simulations.py --production")
        print()
        return 0
    else:
        print(f"⚠ {failed} test(s) failed. Review errors and check your setup.")
        print()
        print("Troubleshooting:")
        print("  - Check CORSIKA paths in corsika/simulation/config.py")
        print("  - Verify EPOS data files exist in corsika/corsika-78050/epos/")
        print("  - Ensure conda environment 'saphir' is active")
        print()
        return 1


if __name__ == '__main__':
    sys.exit(run_validation())
