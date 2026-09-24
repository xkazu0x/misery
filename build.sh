#!/usr/bin/env bash
set -eu
cd "$(dirname "$0")"

# --- Unpack Arguments --------------------------------------------------------
for arg in "$@"; do declare $arg='1'; done

# --- Compile/Link Definitions ------------------------------------------------
cc_common="-I../src/ -Wall -Wextra -Wno-missing-braces -Wno-override-init -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wno-switch -Wno-cpp"
cc_debug="-g -O0 -DBUILD_DEBUG=1 ${cc_common}"
cc_release="-g -O2 -DBUILD_DEBUG=0 ${cc_common}"
cc_link="-lm"

# --- External Libraries ------------------------------------------------------
if [[ -x "$(command -v pkg-config)" ]]; then
  cc_os_gfx="$(pkg-config --cflags --libs x11 xcursor)"
else
  cc_os_gfx="-lX11 -lXcursor"
fi

# --- Choose Compile/Link -----------------------------------------------------
if   [[ "${clang:-0}" == "1" ]]; then compiler="${CC:-clang}"; echo "[clang compile]"; 
elif [[ "${gcc:-1}"   == "1" ]]; then compiler="${CC:-gcc}";   echo "[gcc compile]";   
fi
if   [[ "${release:-0}" == "1" ]]; then compile="$compiler $cc_release"; echo "[release mode]"; 
elif [[ "${debug:-1}"   == "1" ]]; then compile="$compiler $cc_debug";   echo "[debug mode]";   
fi

# --- Prep Directories --------------------------------------------------------
mkdir -p build

# --- Build -------------------------------------------------------------------
cd build
$compile ../src/scratch_main.c $cc_link $cc_os_gfx -o scratch
if [[ -v run ]]; then ./scratch; fi
cd ..
