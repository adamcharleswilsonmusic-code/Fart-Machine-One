#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/fmo build/gen
python3 tools/import_samples.py --placeholder --output build/gen/fmo_test_bank.h
CC="${CC:-cc}"
$CC -O1 -g -fsanitize=address,undefined -ffp-contract=off -Wall -Wextra -Werror -DOM_HOST -DFMO_SAMPLE_BANK_HEADER=\"fmo_test_bank.h\" -Ibuild/gen -Ifirmware/src/dsp tests/fmo/dsp_test.c firmware/src/dsp/omni.c -o build/fmo/dsp_test
build/fmo/dsp_test
python3 tests/fmo/import_test.py
FMO_TEST_BANK=1 sh host/build_host.sh
build/host/omni_host tests/fmo/play.omni build/fmo
$CC -O1 -g -fsanitize=address,undefined -ffp-contract=off -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -DOM_HOST -DFMO_SAMPLE_BANK_HEADER=\"fmo_test_bank.h\" -Ibuild/gen -Ifirmware/src -Ifirmware/src/dsp tests/fmo/ui_test.c firmware/src/dsp/omni.c -lm -o build/fmo/ui_test
build/fmo/ui_test
$CC -O1 -g -fsanitize=address,undefined -ffp-contract=off -Wall -Wextra -Werror -DOM_HOST -Ifirmware/src/dsp tests/fmo/sequencer_test.c firmware/src/dsp/omni.c -o build/fmo/sequencer_test
build/fmo/sequencer_test
$CC -O1 -g -fsanitize=address,undefined -ffp-contract=off -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -DOM_HOST -Ibuild/gen -Ifirmware/src -Ifirmware/src/dsp tests/fmo/sequencer_ui_test.c firmware/src/dsp/omni.c -lm -o build/fmo/sequencer_ui_test
build/fmo/sequencer_ui_test
for name in encoder uac trs; do
  $CC -O2 -w -Ifirmware/src -Ifirmware/hal "tests/host/${name}_test.c" -lm -o "build/fmo/${name}_test"
  "build/fmo/${name}_test"
done
$CC -O2 tests/storage_test.c -o build/fmo/storage_test
build/fmo/storage_test
if [ -f build/fart-machine-one.fwsc ]; then
  $CC tests/ota_test.c -o build/fmo/ota_test
  build/fmo/ota_test build/fart-machine-one.fwsc
fi
