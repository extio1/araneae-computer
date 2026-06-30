#!/bin/bash

set -e

CWD=$(dirname $0)
source ${CWD}/source.sh

BINARY_PATH=$1

${MANAGER} --verbose -il -w -s DebugBinary definitionFile ${DEFINITION_FILE} archName ${ARCH_NAME} binaryFileToRun ${BINARY_PATH} codeRamBankName code ipRegStorageName IP finishMnemonicName hlt
