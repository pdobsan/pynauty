import sys
from . import _nauty_dir, _nauty_url

if len(sys.argv) <= 1:
    print("Usage: python -m pynauty ( nauty-dir | nauty-url )")
    exit(1)
if sys.argv[1] == "nauty-dir":
    print(_nauty_dir)
elif sys.argv[1] == "nauty-url":
    print(_nauty_url)
