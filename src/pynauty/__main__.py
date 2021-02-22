import sys
from . import _nauty_version, _nauty_dir, __version__

meta = {
    'nauty-version'   : _nauty_version,
    'nauty-dir'       : _nauty_dir,
    'pynauty-version' : __version__,
    }

if len(sys.argv) <= 1:
    print(f"Usage: python -m pynauty "
            f"( nauty-version | nauty-dir | pynauty-version )")
    exit(1)

x = meta.get(sys.argv[1])
if x : print(x)
