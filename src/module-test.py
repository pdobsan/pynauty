#!/usr/bin/env python3

import sys
import importlib

if len(sys.argv) < 2:
    print(f'Usage: {sys.argv[0]} module-name [message]')
    sys.exit(1)
else:
    module = sys.argv[1]

try:
    importlib.import_module(module)
except ImportError:
    print()
    print(f'Importing module "{module}" failed.')
    print('The following paths were searched:')
    for x in sys.path:
        print(' ', x)
    print()
    if len(sys.argv) > 2:
        print(sys.argv[2])
    else:
        print(f'You might need to install module "{module}".')
    print()
    sys.exit(1)
