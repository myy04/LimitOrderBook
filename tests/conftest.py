import os
import sys

# Make the C++-backed Python package (src/python/LimitOrderBook) importable
# without installing it, regardless of where pytest is invoked from.
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "src", "python"))