# Changelog

## 0.5.0 - 2026-11-22
- Add compact horizontal and stacked energy flows for smaller 2×2 and 2×3 automatic tiles.

## 0.4.1 - 2026-11-22
- Account for charging and discharging when calculating household consumption from a configured battery power sensor.

## 0.4.0 - 2026-11-22
- Display daily yield directly under solar output when space allows.
- Show battery charge and power beside a horizontal charge indicator on a tinted, outlined card in both themes.

## 0.3.0 - 2026-11-20
- The control-room view groups battery charge and power below the central house. The battery panel fits its contents and leaves more space below the icon.

## 0.2.0 - 2026-11-20
- Large automatic tiles show directional energy flows between solar, home, grid and optional battery.
- Leave the home sensor empty to calculate solar power plus signed grid power. Existing home sensor selections take precedence.

## 0.1.0 - 2026-11-20
- First beta: responsive PV overview with six selectable Home Assistant sensors.
- Choose the tile size, hide unused sensors and invert grid or battery signs.
- Missing, invalid and unavailable readings are not shown as zero.
