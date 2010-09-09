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
            "stage/libraries/i686-win32/lib/debug/glod.lib"
        cp "lib/release/glod.dll" \
            "stage/libraries/i686-win32/lib/debug/glod.dll"
    ;;
#        "darwin")
#			libdir="$top/stage/libraries/universal-darwin/"
#            mkdir -p "$libdir"/lib_{debug,release}
#			make -C lib
#			make -C tests
#			tests/release/llconvexdecompositionstubtest
#			cp "lib/debug/libllconvexdecompositionstub.a" \
#				"$libdir/lib_debug/libllconvexdecomposition.a"
#			cp "lib/release/libllconvexdecompositionstub.a" \
#				"$libdir/lib_release/libllconvexdecomposition.a"
#		;;
#        "linux")
#			libdir="$top/stage/libraries/i686-linux/"
#            mkdir -p "$libdir"/lib_{debug,release}_client
#			make -C lib
#			make -C tests
#			tests/release/llconvexdecompositionstubtest
#			cp "lib/debug/libllconvexdecomposition.a" \
#				"$libdir/lib_debug_client/libllconvexdecomposition.a"
#			cp "lib/release/libllconvexdecomposition.a" \
#				"$libdir/lib_release_client/libllconvexdecomposition.a"
#			cp "lib/debug_stub/libllconvexdecompositionstub.a" \
#				"$libdir/lib_debug_client/libllconvexdecompositionstub.a"
#			cp "lib/release_stub/libllconvexdecompositionstub.a" \
#				"$libdir/lib_release_client/libllconvexdecompositionstub.a"
#        ;;
esac
mkdir -p "stage/libraries/include/glod"
cp "include/glod.h" "stage/libraries/include/glod/glod.h"
mkdir -p stage/LICENSES
cp LICENSE stage/LICENSES/GLOD.txt

pass

