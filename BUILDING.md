# Build Fart Machine One

Prebuilt firmware users can follow [INSTALL.md](INSTALL.md) without compiling.
These source-build steps target macOS or Linux with a C compiler, Git and Python 3.
Docker is required unless using native Linux x86-64. Docker must be running.

## Dependencies

Run from this repository's root:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install -r requirements-build.txt
sh tools/get_toolchain.sh "$PWD/tooling/jieli"
git clone --branch AC79NN_SDK_V1.2.1_2023-12-13 --depth 1 https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK tooling/ac79-sdk
export JIELI_TOOLCHAIN="$PWD/tooling/jieli/toolchain"
export AC79_SDK="$PWD/tooling/ac79-sdk"
export PYTHON="$PWD/.venv/bin/python"
export X0X_FONTSET=terminus
export JIELI_DOCKER_IMAGE='debian:bookworm-slim@sha256:7c7b2c966bc9ee8cedfeef67e0e279108992c77681fa595db4a9d65c06ccc587'
```

The toolchain downloader uses a mutable official endpoint. The tested version
was `jieli-linux-toolchains-20260730.1`; see [docs/build-versions.json](docs/build-versions.json)
for the compiler checksum and SDK commit. A newer download may produce different
results. The build checks the three packaged SDK file hashes against its reference.

## Build and check

```sh
python tools/import_samples.py samples/originals --names samples/display_names.json
sh tests/run_tests.sh
./build.sh
```

Output: `build/fart-machine-one.fwsc` (installable package), `build/omni.bin`
(application), and `build/loader/ota.bin` (update loader). Builds and tests do not
contact the FM-1. The default package identity is `FM-1_701`.
The build checks entry placement, RAM/app capacity and low-level code boundaries.
The test suite covers sample import, playback, effects, panel/MIDI controls,
sequencer timing, overdubbing, stop/clear, encoders, USB audio, TRS MIDI and storage.
When a firmware package exists, it also checks OTA handling against that package.

## Optional browser preview

Install Emscripten SDK 4.0.23 and source its `emsdk_env.sh`, then:

```sh
sh web/emu/build.sh
node tests/host/emu_test.mjs build/emu/omni.wasm
node tests/fmo/sequencer_emu_test.mjs build/emu/omni.wasm
python3 -m http.server 8791 --bind 127.0.0.1 --directory build/emu
```

Open http://127.0.0.1:8791/ and switch on. The preview runs the app's C playback
and screen code. It does not simulate USB flashing or the physical audio circuitry.
Native/browser performance does not measure the physical FM-1's CPU load.
