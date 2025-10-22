#!/usr/bin/env python3
"""
EAE Firmware Run Script
Launches the firmware simulator with command line arguments
Cross-platform run script
"""

import os
import sys
import subprocess
import platform
import argparse
from pathlib import Path

def find_executable():
    """Find the built executable"""
    build_dir = Path('build')
    
    if platform.system().lower() == 'windows':
        exe_path = build_dir / 'Release' / 'eae_firmware.exe'
    else:
        exe_path = build_dir / 'eae_firmware'
    
    if not exe_path.exists():
        return None
    
    return exe_path

def run_simulator(setpoint, critical, quiet=False):
    """Run the firmware simulator"""
    exe_path = find_executable()
    
    if not exe_path:
        print("Error: Executable not found.")
        print("Please run 'python scripts/build.py' first.")
        sys.exit(1)
    
    print("=== EAE Firmware Run Script ===")
    print(f"Starting EAE Firmware Simulator...")
    print(f"Setpoint: {setpoint}°C")
    print(f"Critical: {critical}°C")
    print()
    
    # Build command
    cmd = [str(exe_path), f'--set={setpoint}', f'--crit={critical}']
    
    if quiet:
        cmd.append('--quiet')
    
    try:
        # Run the executable
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Simulator exited with error code: {e.returncode}")
        sys.exit(e.returncode)
    except KeyboardInterrupt:
        print("\nSimulation interrupted by user.")
        sys.exit(0)

def main():
    parser = argparse.ArgumentParser(
        description='EAE Firmware Simulator',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python scripts/run.py                    # Use defaults (55°C, 90°C)
  python scripts/run.py --set 60 --crit 95 # Custom temperatures
  python scripts/run.py --quiet           # Reduced output
        """
    )
    
    parser.add_argument(
        '--set', 
        type=float, 
        default=55.0,
        help='Set temperature setpoint (default: 55.0°C)'
    )
    
    parser.add_argument(
        '--crit', 
        type=float, 
        default=90.0,
        help='Set critical temperature (default: 90.0°C)'
    )
    
    parser.add_argument(
        '--quiet', 
        action='store_true',
        help='Reduce output verbosity'
    )
    
    args = parser.parse_args()
    
    # Validate arguments
    if args.set <= 0:
        print("Error: Setpoint must be positive")
        sys.exit(1)
    
    if args.crit <= 0:
        print("Error: Critical temperature must be positive")
        sys.exit(1)
    
    if args.set >= args.crit:
        print("Warning: Setpoint should be less than critical temperature")
    
    run_simulator(args.set, args.crit, args.quiet)

if __name__ == '__main__':
    main()
