#!/bin/sh

ROOT_PATH=`dirname "$0"`
PARTY_PATH="${ROOT_PATH}/3rdparty"

LIBUV_PATH="${PARTY_PATH}/lib/pkgconfig/libuv.pc"
#LIBUV_PATH="${PARTY_PATH}/3rdparty/lib/pkgconfig"

rm -r *
cmake -DCMAKE_BUILD_TYPE=Debug -DLIBUV_PATH="${LIBUV_PATH}" "${ROOT_PATH}"