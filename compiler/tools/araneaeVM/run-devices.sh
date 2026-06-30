#!/bin/bash
# Run a binary with device I/O (SimplePipe stdio, PIC, Clock).
# Usage: run-devices.sh <binary.ptptb>

set -e

CWD=$(dirname $0)
source ${CWD}/source.sh

BINARY_PATH=$1
DEVICES_FILE="${CWD}/devices.xml"

${MANAGER} --verbose -ip -w -s ExecuteBinaryWithIo \
  devices.xml ${DEVICES_FILE} \
  definitionFile ${DEFINITION_FILE} \
  archName ${ARCH_NAME} \
  binaryFileToRun ${BINARY_PATH} \
  codeRamBankName code \
  ipRegStorageName IP \
  finishMnemonicName hlt
