#!/usr/bin/env bash

# always returns the location of the current script
scriptHome=$(dirname -- "$( readlink -f -- "$0"; )")

cd $scriptHome || { echo "cd $scriptHome failed"; exit 1; }

source ./setenv.sh

sqlplus -s /nolog <<EOF

	connect jkstill/grok@${ezName}

	@create-blobdest.sql

	exit

EOF

