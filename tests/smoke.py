import sys
from pathlib import Path
import subprocess

RED, GREEN, NC = '\033[0;31m', '\033[0;32m', '\033[0m'
root = Path(__file__).parent.parent
build_dir = Path(sys.argv[1])
corpus_dir = root / 'tests/corpus'
exe_suffix = sys.argv[2] if len(sys.argv) > 2 else ''

arg0 = build_dir / ('bort'+exe_suffix)
for file in build_dir.rglob("*"):
    if file.name == 'bort'+exe_suffix:
        arg0 = file
        break

for file in corpus_dir.glob('*.c'):
    print(arg0)
    result = subprocess.run(
        [arg0, '--dump-ast', '--emit-ir', '-o', '-', file])
    print(f"{GREEN+'OK' if result.returncode == 0 else RED + 'FAIL'} {file}{NC}\n")
