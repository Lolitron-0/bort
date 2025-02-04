import sys
from pathlib import Path
import subprocess

RED, GREEN, NC = '\033[0;31m', '\033[0;32m', '\033[0m'
root = Path(__file__).parent.parent
build_dir = sys.argv[1]
corpus_dir = root / 'tests/corpus'
exe_suffix = sys.argv[2] if len(sys.argv) > 2 else ''

for file in corpus_dir.glob('*.c'):
    result = subprocess.run(
        [Path(build_dir) / ('bort'+exe_suffix), '--dump-ast', '--emit-ir', file])
    print(f"{GREEN+'OK' if result.returncode == 0 else RED + 'FAIL'} {file}{NC}\n")
