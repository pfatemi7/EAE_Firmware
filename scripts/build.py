#!/usr/bin/env python3
"""
EAE Firmware Build Script
Configures and builds the project using vcpkg + CMake
Cross-platform build script
"""

import os
import sys
import subprocess
import platform
from pathlib import Path

def find_vcpkg():
    """Find vcpkg installation"""
    # Check environment variable first
    vcpkg_root = os.environ.get('VCPKG_ROOT')
    if vcpkg_root and os.path.exists(vcpkg_root):
        return vcpkg_root
    
    # Check common locations
    home = Path.home()
    common_paths = [
        home / '.vcpkg',
        Path.cwd() / 'vcpkg',
        Path.cwd() / '..' / 'vcpkg'
    ]
    
    for path in common_paths:
        if path.exists():
            return str(path)
    
    return None

def get_triplet():
    """Get appropriate vcpkg triplet for current platform"""
    system = platform.system().lower()
    if system == 'windows':
        return 'x64-windows-static'
    elif system == 'linux':
        return 'x64-linux-static'
    else:
        return 'x64-osx-static'

def run_command(cmd, cwd=None):
    """Run a command and return success status"""
    try:
        print(f"Running: {' '.join(cmd)}")
        result = subprocess.run(cmd, cwd=cwd, check=True, capture_output=True, text=True)
        if result.stdout:
            print(result.stdout)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error running command: {e}")
        if e.stderr:
            print(f"Error output: {e.stderr}")
        return False

def main():
    print("=== EAE Firmware Build Script ===")
    
    # Find vcpkg
    vcpkg_root = find_vcpkg()
    if not vcpkg_root:
        print("Error: vcpkg not found.")
        print("install vcpkg")
        print("  git clone https://github.com/Microsoft/vcpkg.git ~/.vcpkg")
        print("  cd ~/.vcpkg && ./bootstrap-vcpkg.sh")
        sys.exit(1)
    
    print(f"Using vcpkg at: {vcpkg_root}")
    
    # Verify vcpkg installation
    vcpkg_cmake = os.path.join(vcpkg_root, 'scripts', 'buildsystems', 'vcpkg.cmake')
    if not os.path.exists(vcpkg_cmake):
        print(f"Error: vcpkg.cmake not found at {vcpkg_cmake}")
        print("Please ensure vcpkg is properly installed.")
        sys.exit(1)
    
    # Create build directory
    print("Creating build directory...")
    build_dir = Path('build')
    build_dir.mkdir(exist_ok=True)
    
    # Get triplet
    triplet = get_triplet()
    print(f"Using triplet: {triplet}")
    
    # Configure with CMake
    print("Configuring with CMake + vcpkg...")
    cmake_cmd = [
        'cmake', '..',
        f'-DCMAKE_TOOLCHAIN_FILE={vcpkg_cmake}',
        f'-DVCPKG_TARGET_TRIPLET={triplet}',
        '-DCMAKE_BUILD_TYPE=Release'
    ]
    
    if not run_command(cmake_cmd, cwd=build_dir):
        print("CMake configuration failed!")
        sys.exit(1)
    
    # Build the project
    print("Building project...")
    build_cmd = ['cmake', '--build', '.', '--config', 'Release']
    
    if not run_command(build_cmd, cwd=build_dir):
        print("Build failed!")
        sys.exit(1)
    
    # Run tests
    print("Running unit tests...")
    test_cmd = ['ctest', '--output-on-failure', '--build-config', 'Release']
    
    if not run_command(test_cmd, cwd=build_dir):
        print("Tests failed!")
        sys.exit(1)
    
    print("=== Build Complete ===")
    
    # Determine executable path
    if platform.system().lower() == 'windows':
        exe_path = build_dir / 'Release' / 'eae_firmware.exe'
    else:
        exe_path = build_dir / 'eae_firmware'
    
    print(f"Executable: {exe_path}")
    print("Run with: python scripts/run.py [args]")

if __name__ == '__main__':
    main()
