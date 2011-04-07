#!/bin/sh

# turn on verbose debugging output for parabuild logs.
set -x
# make errors fatal
set -e

if [ -z "$AUTOBUILD" ] ; then 
    fail
fi

if [ "$OSTYPE" = "cygwin" ] ; then
    export AUTOBUILD="$(cygpath -u $AUTOBUILD)"
fi

# run build from root of checkout
cd "$(dirname "$0")"

# load autbuild provided shell functions and variables
set +x
eval "$("$AUTOBUILD" source_environment)"
set -x

top="$(pwd)"
case "$AUTOBUILD_PLATFORM" in
    "windows")
        build_sln "glodlib.sln" "Debug|Default"
        build_sln "glodlib.sln" "Release|Default"
		mkdir -p stage/lib/{debug,release}
        cp "lib/debug/glod.lib" \
            "stage/lib/debug/glod.lib"
        cp "lib/debug/glod.dll" \
            "stage/lib/debug/glod.dll"
        cp "src/api/debug/glod.pdb" \
            "stage/lib/debug/glod.pdb"
        cp "lib/release/glod.lib" \
            "stage/lib/release/glod.lib"
        cp "lib/release/glod.dll" \
            "stage/lib/release/glod.dll"
    ;;
        "darwin")
			libdir="$top/stage/lib/"
            mkdir -p "$libdir"/{debug,release}
			make -C src clean
			make -C src debug
			cp "lib/libGLOD.so" \
				"$libdir/debug/libglod.so"
			make -C src clean
			make -C src release
			cp "lib/release/libGLOD.so" \
				"$libdir/release/libglod.so"
		;;
        "linux")
			libdir="$top/stage/libraries/i686-linux/"
            mkdir -p "$libdir"/lib_{debug,release}_client
			make -C src clean
			make -C src debug
			cp "lib/libGLOD.so" \
				"$libdir/lib_debug_client/libglod.so"
			make -C src clean
			make -C src release
			cp "lib/libGLOD.so" \
				"$libdir/lib_release_client/libglod.so"
        ;;
esac
mkdir -p "stage/include/glod"
cp "include/glod.h" "stage/include/glod/glod.h"
mkdir -p stage/LICENSES
cp LICENSE stage/LICENSES/GLOD.txt

pass

