# Validation — 0.2.0

Checked on 2026-11-20. This is an implementation beta, not a hardware-verified release.

## Passed

- Official `tessera-plugins/tools/check.py` against the unmodified manifest validator from Tessera release `screens-v0.4.97` (firmware 0.60.0, plugin API 0.24): `pv_overview: ok`, `needs: ok`. The checker notes the missing optional store image.
- C++17 native tests for the shared production model: strict number parsing, missing/unavailable/invalid states, W/kW and Wh/kWh conversion, percentage bounds, unsupported units, age checking, direction hysteresis, formatting, and more than 60,000 combinations of layout dimensions and requested reading counts.
- Native model compiled with `-Wall -Wextra -Werror -fsanitize=address,undefined`; execution passed with leak detection disabled because the sandbox uses ptrace. This does not compile the display adapter or ESPHome firmware.
- Python component source passes `py_compile`.

## Not completed

- Full ESPHome firmware compilation and linking, including the C++ tile adapter against LVGL.
- The official render bench and image inspection for light/dark on 4-, 7-, 10.1-inch screens. Invoking `tools/preview.py --strict` stops because `git` is missing. Shell DNS/network access also prevents fetching its build dependencies.
- Physical display tests, sensor delivery from the user's Home Assistant, and OTA installation.
- Measured flash/RAM budgets and store thumbnail. Manifest values are estimates.

## Repeat native tests

From this plugin directory:

```sh
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_model.cpp -o /tmp/pv_model_test
/tmp/pv_model_test
```

If LeakSanitizer is blocked by a ptrace sandbox, run the test with `ASAN_OPTIONS=detect_leaks=0` instead. Address and undefined-behavior checking remain enabled.

## Complete official validation

Keep the official repositories beside the plugin folder. To validate the firmware version requested rather than the latest branch:

```sh
git clone https://github.com/MaxGramser/tessera-plugins ../tessera-plugins
git clone --branch screens-v0.4.97 --depth 1 https://github.com/MaxGramser/homeassistant_espscreen ../tessera-core-060
python3 -m pip install pyyaml
TESSERA_MANIFEST=../tessera-core-060/screen_manager/app/plugin_manifest.py TESSERA_BOARDS=../tessera-core-060/screen_manager/app/boards.json python3 ../tessera-plugins/tools/check.py .
TESSERA_TREE=../tessera-core-060 python3 ../tessera-plugins/tools/preview.py --setup
TESSERA_TREE=../tessera-core-060 python3 ../tessera-plugins/tools/preview.py . --strict
TESSERA_TREE=../tessera-core-060 python3 ../tessera-plugins/tools/preview.py . --screen 1024x600@7 --strict
```

If the current preview harness requires a newer core, use the corresponding older plugins-tool revision or run the build from the installed Tessera 0.60.0 app instead. Do not silently treat a newer firmware build as a test of 0.60.0. Inspect each smallest and largest image in both themes; also test full-page portrait. After the official bench passes, generate the store image with its documented `store_image.py` workflow.

The preview values are explicitly synthetic. They exercise a consistent AC balance (1,200 W solar, 600 W household, 400 W charge and 200 W export). The first fixture leaves the house sensor empty, so its calculated house value is 1,000 W. The compact fixture uses the measured 600 W appliance load.

## Physical acceptance

1. Install locally on one screen; retain the build log if compilation fails.
2. Check selected readings against Home Assistant including actual units and positive/negative grid/battery cases.
3. Confirm optional channels disappear when unselected, and unavailable channels never become zero.
4. Try small, medium and full-page sizes, portrait/landscape where used, and both themes.
5. Test connection loss and recovery. Tessera's connection indicator is authoritative; the plugin footer only names the data source.
6. Repeat on the user's 4-, 7- and 10.1-inch displays before broader use.

## References

- https://github.com/MaxGramser/tessera-plugins/blob/main/docs/MAKING_A_PLUGIN.md
- https://github.com/MaxGramser/tessera-plugins/blob/main/docs/MANIFEST.md
- https://github.com/MaxGramser/tessera-plugins/blob/main/docs/FIRMWARE_API.md
- https://github.com/MaxGramser/homeassistant_espscreen/blob/screens-v0.4.97/components/smart_display/plugin_api.h
- https://github.com/MaxGramser/homeassistant_espscreen/releases/tag/screens-v0.4.97
