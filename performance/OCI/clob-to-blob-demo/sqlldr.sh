#!/usr/bin/env bash

# always returns the location of the current script
scriptHome=$(dirname -- "$( readlink -f -- "$0"; )")

cd $scriptHome || { echo "cd $scriptHome failed"; exit 1; }

create/create-blobdest.sh

cd dump

time sqlldr $dest_oracle_user/$dest_oracle_password@$dest_oracle_ezName parfile=blobdest.par readsize=20000000 bindsize=20000000


