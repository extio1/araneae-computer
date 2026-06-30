#!/bin/bash

set -e

CWD=$(dirname $0)
source ${CWD}/source.sh

ASM_LISTING=$1

${MANAGER} --verbose -w -s ValidateArchDefSyntax -w definitionFile ${DEFINITION_FILE} archName ${ARCH_NAME}
