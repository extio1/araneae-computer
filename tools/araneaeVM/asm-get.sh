#!/bin/bash

set -e

CWD=$(dirname $0)
source ${CWD}/source.sh

ID=$1

${MANAGER} -g ${ID} -r out.ptptb > ./out.ptptb
