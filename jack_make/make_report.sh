#!/bin/bash

cargo test -- -Z unstable-options --format json > out

python test_report.py out

