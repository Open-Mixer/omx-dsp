#!/bin/sh
# Scratch probe driver. Run inside the omx-rtsan image (fedora 44, clang 22.1.8):
#   podman run --rm -v $PWD:/src:Z -v <omx-contract 3.19 checkout>/include:/contract:ro,Z localhost/omx-rtsan:2 sh /src/rtsan/run.sh
set -u
cd /src
python3 -I rtsan/annotate.py >/dev/null
export OMX_CONTRACT_INC=/contract
export RTSAN_OPTIONS="halt_on_error=false:abort_on_error=false:suppress_equal_stacks=true"
export ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer
RTFLAGS="-fsanitize=realtime -g -fno-omit-frame-pointer -Wno-function-effects -Wno-error"
rm -rf build; mkdir -p rtsan/out
for t in ${TARGETS:-fx-oracles mix-golden analysis-oracles engine-kernels-golden suite consumer}; do
  make -k -j8 $t CC=clang CFLAGS="-Wall -O1 $RTFLAGS" FASTMATH=off > rtsan/out/$t.log 2>&1
  echo "$t exit=$? rtsan-reports=$(grep -c 'ERROR: RealtimeSanitizer' rtsan/out/$t.log)"
done
