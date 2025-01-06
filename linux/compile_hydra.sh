#!/bin/bash
parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
echo "Current Directory : $parent_path"
cd "$parent_path"

echo "Hydra+ compilation script for gcc and Linux platforms."
echo "This configuration links statically the WolfSSL library."
echo "The ODBC and MariaDB support needs the shared libraries."
echo "The script expects the  libwolfssl.a and the libreproc.a to be precompiled."
echo "Last change : 2024-07-11 14:35"
echo "Nikos Mourgis deus-ex.gr 2024."
echo "Live Long and Prosper."

echo "Create directories -> "
mkdir hydra+/obj
echo  " -> hydra+/obj"
mkdir "hydra+/bin"
echo  "-> hydra+/obj"

read -p "Download build-essential (gcc and libraries) ? [Y/n]" -n 1 -r
if [[ $REPLY =~ ^[Yy]$ ]]
then
 apt install build-essential
fi
echo ""
read -p "Download ODBC driver and libraries ? [Y/n]" -n 1 -r
if [[ $REPLY =~ ^[Yy]$ ]]
then
 apt install unixodbc-dev
fi
echo ""
read -p "Download MariaDB client libraries ? [Y/n]" -n 1 -r
if [[ $REPLY =~ ^[Yy]$ ]]
then
 apt install libmariadb3 libmariadb-dev
fi
echo ""
read -p "Start Compilation? [Y/n]" -n 1 -r
if [[ $REPLY =~ ^[Yy]$ ]]
then
echo ""
echo "gcc -> Compile Threads support..."
gcc  -w -m64 -O3 -m64 -Iincludes -Iincludes/thirdparty -Iincludes/thirdparty/mariadb/includes -c includes/thirdparty/cthreads/cthreads.c -o hydra+/obj/cthreads.o

echo "gcc -> Compile base64 support..."
gcc  -w -m64 -O3 -m64 -Iincludes/thirdparty/base64 -c includes/thirdparty/base64/buffer.c -o hydra+/obj/buffer.o
gcc  -w -m64 -O3 -m64 -Iincludes/thirdparty/base64 -c includes/thirdparty/base64/decode.c -o hydra+/obj/decode.o
gcc  -w -m64 -O3 -m64 -Iincludes/thirdparty/base64 -c includes/thirdparty/base64/encode.c -o hydra+/obj/encode.o

echo "gcc -> Compile cuba zip support..."
gcc  -w -m64 -O3 -m64 -Iincludes/thirdparty/zipcuba -c includes/thirdparty/zipcuba/zip.c -o hydra+/obj/zip.o

echo "gcc -> Compile Hydra..."
gcc  -w -m64 -O3 -m64 -Iincludes -Iincludes/thirdparty -Iincludes/thirdparty/mariadb/includes -c hydra+/hydra.c -o hydra+/obj/main.o

echo "gcc -> Linking..."
gcc  -o hydra+/bin/hydra+ hydra+/obj/buffer.o hydra+/obj/decode.o hydra+/obj/encode.o hydra+/obj/cthreads.o hydra+/obj/zip.o hydra+/obj/main.o  -m64 -lpthread -ldl -lodbc -lm -lmariadb -s -m64  includes/thirdparty/reproc/libreproc.a includes/thirdparty/wolfssl/libwolfssl.a

fi

echo "Operation completed."