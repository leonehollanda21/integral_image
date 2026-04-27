#!/usr/bin/env python3
"""
Wrapper to run the validation script located at python/validate.py.
Use this file so `python3 validate.py` from the repository root runs the proper script.
"""
import os
import sys
import runpy

HERE = os.path.dirname(__file__)
SCRIPT = os.path.join(HERE, "python", "validate.py")

if not os.path.exists(SCRIPT):
    print(f"Erro: nao encontrei {SCRIPT}", file=sys.stderr)
    sys.exit(1)

# Run the script in this interpreter, preserving stdout/stderr
runpy.run_path(SCRIPT, run_name="__main__")
