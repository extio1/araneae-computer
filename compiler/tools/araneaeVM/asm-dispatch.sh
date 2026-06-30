#!/bin/bash

set -e

CWD=$(dirname $0)
source ${CWD}/source.sh

ASM_LISTING=$*

${MANAGER} --verbose -w -s Assemble -w asmListing "$ASM_LISTING",${SCRIPT_PATH}/../../test/e2e/builtins.S definitionFile ${DEFINITION_FILE} archName ${ARCH_NAME}


# ${MANAGER} -s Assemble asmListing ./output/out/main.ae/main.ae.s,../kernel/lib/io.ae.s,../kernel/lib/configure-vm.ae.s definitionFile ${DEFINITION_FILE} archName ${ARCH_NAME}
# ${MANAGER} -g dc3cfb6d-27b3-4bb2-9856-f90411acac44 -r out.ptptb > out.ptptb 
# ${MANAGER} --verbose -ip -w -s ExecuteBinaryWithIo devices.xml ../compiler/tools/araneaeVM/devices.xml definitionFile ${DEFINITION_FILE} archName ${ARCH_NAME} binaryFileToRun out.ptptb codeRamBankName code ipRegStorageName IP finishMnemonicName hlt

# так работает вывод
# ${MANAGER} --verbose -ip -w -s ExecuteBinaryWithIo devices.xml ../compiler/tools/araneaeVM/devices.xml  definitionFile ${DEFINITION_FILE} archName ${ARCH_NAME} binaryFileToRun out.ptptb codeRamBankName code ipRegStorageName IP finishMnemonicName hlt