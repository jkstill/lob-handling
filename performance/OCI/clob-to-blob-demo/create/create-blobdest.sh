#!/usr/bin/env bash

# always returns the location of the current script
scriptHome=$(dirname -- "$( readlink -f -- "$0"; )")

cd $scriptHome || { echo "cd $scriptHome failed"; exit 1; }

source ../demo-setenv.sh

sqlplus -s /nolog <<EOF

	connect $dest_oracle_user/$dest_oracle_password@$dest_oracle_ezName

	@create-blobdest.sql

	exit

EOF

