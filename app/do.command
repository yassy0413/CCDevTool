#!/bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "${DIR}"

./nwjs-osx-x64/nwjs.app/Contents/MacOS/nwjs ./
