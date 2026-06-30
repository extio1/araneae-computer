#!/bin/bash
SCRIPT_PATH=$(dirname "$(realpath "${BASH_SOURCE[0]}")")
BUILD_DIR=$SCRIPT_PATH/../../../build/compiler
MANAGER="mono /home/extio1/Education/itmo2/RemoteTasks/Portable.RemoteTasks.Manager.exe -ul 506765 -up 48ac0c89-93e9-4b7c-a394-6d311086f371"
DEFINITION_FILE="$BUILD_DIR/tools/araneaeVM/ARARCH.target.pdsl"
echo $DEFINITION_FILE
ARCH_NAME="ARARCH"
PATH=$SCRIPT_PATH:$PATH
