#!/bin/bash

set -o errexit
set -o pipefail
set -x

#DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
#DIR=~asztalos/GbenchMacro

#LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/$DIR/lib"

$TEST_TOOL_PATH "$@"
