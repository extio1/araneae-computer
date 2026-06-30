#!/bin/bash

set -e

CWD=$(dirname $0)
source ${CWD}/source.sh

BINARY_PATH=$1

${MANAGER} -il -s ExecuteBinaryWithInteractiveInput stdinRegStName s_in stdoutRegStName s_out definitionFile ${DEFINITION_FILE} archName ${ARCH_NAME} binaryFileToRun ${BINARY_PATH} codeRamBankName code ipRegStorageName IP finishMnemonicName hlt

# ${MANAGER} --verbose -il -s ExecuteBinaryWithInteractiveInput stdinRegStName s_in stdoutRegStName s_out definitionFile ${DEFINITION_FILE} archName ${ARCH_NAME} binaryFileToRun out.ptptb codeRamBankName code ipRegStorageName IP finishMnemonicName hlt