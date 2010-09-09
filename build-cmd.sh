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

# load autbuild provided shell functions and variables
set +x
eval "$("$AUTOBUILD" source_environment)"
set -x

top="$(pwd)"
case "$AUTOBUILD_PLATFORM" in
    "windows")
        build_sln "glodlib.sln" "Debug"
        build_sln "glodlib.sln" "Release"
		mkdir -p stage/libraries/i686-win32/lib/{debug,release}
        cp "lib/debug/glod.lib" \
            "stage/libraries/i686-win32/lib/debug/glod.lib"
        cp "lib/debug/glod.dll" \
            "stage/libraries/i686-win32/lib/debug/glod.dll"
        cp "src/api/debug/glod.pdb" \
            "stage/libraries/i686-win32/lib/debug/glod.pdb"
        cp "lib/release/glod.lib" \
            "stage/libraries/i686-win32/lib/release/glod.lib"
        cp "lib/release/glod.dll" \
            "stage/libraries/i686-win32/lib/release/glod.dll"
    ;;
        "darwin")
			libdir="$top/stage/libraries/universal-darwin/"
            mkdir -p "$libdir"/lib_{debug,release}
			make -C src clean
			make -C src debug
			cp "lib/libGLOD.so" \
				"$libdir/lib_debug/libglod.so"
			make -C src clean
			make -C src release
			cp "lib/release/libGLOD.so" \
				"$libdir/lib_release/libglod.so"
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
mkdir -p "stage/libraries/include/glod"
cp "include/glod.h" "stage/libraries/include/glod/glod.h"
mkdir -p stage/LICENSES
cp LICENSE stage/LICENSES/GLOD.txt

pass

