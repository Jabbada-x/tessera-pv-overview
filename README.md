# PV overview

Track solar generation, grid exchange and home consumption alongside your battery’s charging or discharging power and state of charge. Each tile uses the Home Assistant sensors you choose.

## Set up

1. In Tessera, open **Plugins**, choose **PV overview** in the store and add it to your screen.
2. Build and install the screen firmware. Wait for the screen to reconnect.
3. Open the screen’s **Layout**, place **PV overview** from **Plugins** and choose a tile size.
4. Select the two required sensors: total AC solar power and signed net grid power. Add measured home consumption, signed battery power, battery charge or today’s solar yield if available.
5. Positive grid power means import and positive battery power means charging. Enable **Invert grid sign** if your sensor reports export as positive, or **Invert battery sign** if it reports discharge as positive.

You can change sensors and tile size in Layout at any time. After a plugin code update, use **Build again** on the screen’s Plugins tab.

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

For two solar inverters choose an existing sum sensor, or create a Home Assistant helper first. For two batteries, use a suitable combined power sensor and an energy-capacity-weighted charge sensor if you want one system total. Separate grid import/export sensors need a signed net-power helper. A single battery sensor must represent net charge/discharge, not just one direction. With the home sensor empty, consumption is solar + grid import − grid export − battery charging + battery discharging. Use PV generation before storage flows and preferably AC-side battery power. Without a battery power sensor the storage term is omitted. A selected home sensor takes precedence. Missing or stale inputs and negative balances display their status.

## Size and appearance

Automatic can use a compact horizontal flow on 2×2 tiles and place solar and grid above the house on taller 2×3 tiles. The fit depends on usable pixels, fonts and configured sensors.

The central home dial connects to a compact battery panel sized for its readings. Select battery power and charge to show both. A horizontal battery gauge sits beside charge and power on a tinted, outlined card in both themes. Daily yield appears beneath solar power when the tile has enough height.

The same plugin runs on all Tessera boards; there are no hard-coded 4-, 7- or 10.1-inch layouts. Large automatic tiles place the house between solar and grid, with a battery branch when configured. Live arrows indicate import/export and charging/discharging. Tiles too small for either diagram and Compact mode use numeric cells. Selected optional inputs are included as room permits. Order: solar, grid, home, battery power, charge, today. Very small tiles show only solar; increasing the tile reveals more readings. Optional inputs left empty use no cell. The full-page option and sizes 1x1 through 9x9 are offered only where they fit Tessera's grid.

Start with a full page on a 4-inch screen. Try a medium-to-large tile on 7-inch and 10.1-inch screens and adjust in Layout. **Compact** limits the view to the first four readings; **Automatic** allows all six. A footer, when there is enough room, tells you when readings are omitted. Light/dark colors and fonts follow Tessera. The app editor shows a placeholder; the custom appearance is drawn by firmware on the display.

Grid/battery values retain their normalized +/- sign even if a caption must be shortened. Direction labels use a configurable 10 W threshold by default and half of it as the return-to-neutral threshold, preventing flicker. The numerical measurement remains unchanged. Thus a value near zero can have a neutral caption.

## Reading age

Age checking is off by default. Set **Maximum reading age** only when all selected sensors regularly refresh their `updated` timestamps. Otherwise a correct, unchanged value (especially charge or a night-time energy counter) could be marked old. When enabled, stale readings show **Old data**, not a current-looking number. A missing clock or timestamp cannot be age-checked. The Home Assistant footer names the data source, not a guaranteed connection status; watch Tessera's connection indicator for connectivity.

## Tested displays

The maintainer successfully tested the tile on Guition 4-inch, 7-inch and 10-inch displays running Tessera 0.62.0. See [validation notes](VALIDATION.md) for additional checks.
