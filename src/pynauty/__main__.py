import sys
from . import _nauty_version, _nauty_tarfile, _nauty_checksum, _nauty_url, _nauty_dir, __version__

meta = {
    'nauty-version'   : _nauty_version,
    'nauty-tarfile'   : _nauty_tarfile,
    'nauty-checksum'  : _nauty_checksum,
    'nauty-url'       : _nauty_url,
    'nauty-dir'       : _nauty_dir,
    'pynauty-version' : __version__,
    }

if len(sys.argv) <= 1:
    print(f"Usage: python -m pynauty "
            f"( nauty-version | nauty-tarfile | nauty-checksum | nauty-url | nauty-dir | pynauty-version )")
    exit(1)

x = meta.get(sys.argv[1])
if x : print(x)
