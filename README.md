# PV overview

> Development beta for Tessera firmware 0.60.0 (plugin API 0.24). Manifest checks and native model tests are provided; a complete ESPHome build, the official visual bench and physical display tests have not been completed.

A responsive tile for solar power, grid import/export, home consumption, battery power, battery charge and daily solar yield. Select Home Assistant sensors separately for each tile. No cloud service or Home Assistant control actions are used.

## Set up

In Tessera, open **Plugins → Add with a link** and paste the URL of this GitHub repository. Add the plugin to a display, build its firmware and select the sensors in Layout. The following steps describe local installation.

1. Extract the archive. Copy the complete `pv_overview` folder to `/config/tessera-plugins/pv_overview/` in Home Assistant, beside the `esphome` folder. The manifest must be `/config/tessera-plugins/pv_overview/tessera-plugin.yaml`, without a second nested folder.
2. Open Tessera > Plugins. The local plugin appears as **Test**. Add it to one screen first and build its firmware. This is the first real firmware compilation test of this beta; if it fails, keep the complete build log.
3. In that screen's Layout, add **PV overview** from Plugins. Select solar and signed net grid power sensors. Choose the optional sensors you have.
4. Resize the tile in Layout, or choose the whole page. Layout and sensor changes do not require another firmware build. Add the plugin to the other displays after the first screen has passed its test.
5. If positive grid power means export in your installation, enable **Invert grid sign**. If positive battery power means discharge, enable **Invert battery sign**.

The plugin is compiled into the display firmware. It cannot be installed as a normal Lovelace card or by uploading the ZIP directly to a display. Code changes require **Build again** in the screen's Plugins tab.

## Sensor mapping

| Input | Expected measurement | Supported units | Normalized positive sign |
|---|---|---|---|
| Solar | Combined AC solar output | W, kW | Generation |
| Grid | Signed net grid power | W, kW | Import |
| Home | Actual household consumption, optional | W, kW | Consumption |
| Battery | Signed charging/discharging power, optional | W, kW | Charging |
| Charge | Battery state of charge, optional | % | 0–100 |
| Today | Solar energy since midnight, optional | Wh, kWh | Yield |

The selected units must be in Home Assistant's `unit_of_measurement` attribute. Unsupported or missing units show **Unit?**. `unknown`, `unavailable`, non-numeric values and out-of-range charge levels are not converted to zero. The daily counter must reset daily in Home Assistant; this plugin does not reset or integrate it.

For two solar inverters choose an existing sum sensor, or create a Home Assistant helper first. For two batteries, use a suitable combined power sensor and an energy-capacity-weighted charge sensor if you want one system total. Separate grid import/export sensors need a signed net-power helper. A single battery sensor must represent net charge/discharge, not just one direction. With the home sensor empty, home power is solar AC power plus signed grid power. Export counts negatively. This balance includes no separate battery correction. With AC storage, select a measured home sensor if you need appliance consumption. Missing or stale inputs and negative balances display their status.

## Size and appearance

The same plugin runs on all Tessera boards; there are no hard-coded 4-, 7- or 10.1-inch layouts. Large automatic tiles place the house between solar and grid, with a battery branch when configured. Live arrows indicate import/export and charging/discharging. Smaller tiles and Compact mode use numeric cells. Selected optional inputs are included as room permits. Order: solar, grid, home, battery power, charge, today. Very small tiles show only solar; increasing the tile reveals more readings. Optional inputs left empty use no cell. The full-page option and sizes 1x1 through 9x9 are offered only where they fit Tessera's grid.

Start with a full page on a 4-inch screen. Try a medium-to-large tile on 7-inch and 10.1-inch screens and adjust in Layout. **Compact** limits the view to the first four readings; **Automatic** allows all six. A footer, when there is enough room, tells you when readings are omitted. Light/dark colors and fonts follow Tessera. The app editor shows a placeholder; the custom appearance is drawn by firmware on the display.

Grid/battery values retain their normalized +/- sign even if a caption must be shortened. Direction labels use a configurable 10 W threshold by default and half of it as the return-to-neutral threshold, preventing flicker. The numerical measurement remains unchanged. Thus a value near zero can have a neutral caption.

## Reading age

Age checking is off by default. Set **Maximum reading age** only when all selected sensors regularly refresh their `updated` timestamps. Otherwise a correct, unchanged value (especially charge or a night-time energy counter) could be marked old. When enabled, stale readings show **Old data**, not a current-looking number. A missing clock or timestamp cannot be age-checked. The Home Assistant footer names the data source, not a guaranteed connection status; watch Tessera's connection indicator for connectivity.

## Validation and development

See `VALIDATION.md`. The source is based on Tessera's official template. Plugin API minimum is 0.15; only APIs present in firmware 0.60.0/API 0.24 are used. The schema was checked against release `screens-v0.4.97`, which ships firmware 0.60.0. No firmware upgrade is assumed.

The archive contains deterministic synthetic preview fixtures, not recordings of your installation. Flash/memory budgets are conservative estimates pending a real build. This beta is for local testing, not a verified store release.
