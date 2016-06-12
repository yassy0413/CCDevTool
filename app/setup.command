#!/bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "${DIR}"

npm install


wget https://github.com/joshuagatcke/HTML-KickStart/archive/master.zip
unzip master.zip
mv HTML-KickStart-master kickstart
rm master.zip

wget http://dl.nwjs.io/v0.15.1/nwjs-v0.15.1-win-x64.zip
unzip nwjs-v0.15.1-win-x64.zip
mv nwjs-v0.15.1-win-x64 nwjs-win-x64
rm nwjs-v0.15.1-win-x64.zip

wget http://dl.nwjs.io/v0.15.1/nwjs-v0.15.1-osx-x64.zip
unzip nwjs-v0.15.1-osx-x64.zip
mv nwjs-v0.15.1-osx-x64 nwjs-osx-x64
rm nwjs-v0.15.1-osx-x64.zip
