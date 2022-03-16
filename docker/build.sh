#!/bin/sh

DIR=`dirname $0`
. $DIR/env.sh

if [ "$1" = "local" ]
then
  docker build --compress --force-rm -f docker/Dockerfile -t $NAME .
else
  docker buildx build --platform linux/arm64,linux/amd64 --compress --force-rm -f docker/Dockerfile --push -t sptrakesh/$NAME:$VERSION -t sptrakesh/$NAME:latest .
  docker pull sptrakesh/$NAME:latest
fi
