#!/usr/bin/env bash

# always returns the location of the current script
scriptHome=$(dirname -- "$( readlink -f -- "$0"; )")

cd $scriptHome || { echo "cd $scriptHome failed"; exit 1; }

source ./setenv.sh

create/create-blobdest.sh

cd jkstill.dump

time sqlldr jkstill/grok@$ezName parfile=blobdest.par readsize=10000000 bindsize=10000000


