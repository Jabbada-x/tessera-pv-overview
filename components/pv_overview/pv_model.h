#pragma once
#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace esphome::pv_overview::model {
enum class Kind { POWER, ENERGY, PERCENT };
enum class Status { OK, MISSING, UNAVAILABLE, UNIT, INVALID, STALE };
struct Reading {
  double value = 0;
  uint32_t updated = 0;
  Status status = Status::MISSING;
};

inline Reading parse(const char *state, const char *unit, Kind kind, uint32_t updated) {
  Reading r;
  r.updated = updated;
  if (!state || !*state) return r;
  if (!std::strcmp(state, "unknown") || !std::strcmp(state, "unavailable")) {
    r.status = Status::UNAVAILABLE;
    return r;
  }
  char *end = nullptr;
  errno = 0;
  double n = std::strtod(state, &end);
  const bool parsed = end && end != state;
  while (end && (*end == ' ' || *end == '\t')) ++end;
  if (!parsed || !end || *end || errno == ERANGE || !std::isfinite(n)) {
    r.status = Status::INVALID;
    return r;
  }
  unit = unit ? unit : "";
  double scale = 1;
  bool valid_unit = false;
  if (kind == Kind::POWER) {
    valid_unit = !std::strcmp(unit, "W") || !std::strcmp(unit, "kW");
    if (!std::strcmp(unit, "kW")) scale = 1000;
  } else if (kind == Kind::ENERGY) {
    valid_unit = !std::strcmp(unit, "Wh") || !std::strcmp(unit, "kWh");
    if (!std::strcmp(unit, "Wh")) scale = 0.001;
  } else valid_unit = !std::strcmp(unit, "%");
  if (!valid_unit) { r.status = Status::UNIT; return r; }
  n *= scale;
  if (!std::isfinite(n) || (kind == Kind::PERCENT && (n < 0 || n > 100)) ||
      (kind == Kind::ENERGY && n < 0)) {
    r.status = Status::INVALID;
    return r;
  }
  r.value = n;
  r.status = Status::OK;
  return r;
}

inline Status status(const Reading &r, uint32_t now, uint32_t max_age) {
  if (r.status == Status::OK && max_age && now && r.updated && now > r.updated && now - r.updated > max_age)
    return Status::STALE;
  return r.status;
}

// Inputs are in watts, with import positive and export negative.
inline Reading home_balance(Reading solar, Reading grid, uint32_t now, uint32_t max_age) {
  for (Reading input : {solar, grid}) {
    input.status = status(input, now, max_age);
    if (input.status != Status::OK) return input;
  }
  const double watts = solar.value + grid.value;
  if (!std::isfinite(watts) || watts < 0) return {0, 0, Status::INVALID};
  return {watts, std::min(solar.updated, grid.updated), Status::OK};
}

// A Schmitt trigger prevents direction labels flickering near zero. Values themselves are never rounded to zero here.
inline int direction(double watts, int previous, double deadband) {
  if (previous > 0 && watts > deadband / 2) return 1;
  if (previous < 0 && watts < -deadband / 2) return -1;
  return watts > deadband ? 1 : watts < -deadband ? -1 : 0;
}

inline std::string format(double value, Kind kind) {
  char text[64];
  if (kind == Kind::PERCENT) std::snprintf(text, sizeof(text), "%.0f %%", value);
  else if (kind == Kind::ENERGY) std::snprintf(text, sizeof(text), "%.2f kWh", value);
  else if (std::fabs(value) >= 1000) std::snprintf(text, sizeof(text), "%.2f kW", std::fabs(value) / 1000);
  else std::snprintf(text, sizeof(text), "%.0f W", std::fabs(value));
  return text;
}

struct Layout { int columns = 1; int count = 1; };
// Prefer all requested readings; use additional columns only when each cell is wide enough.
inline Layout layout(int width, int height, int count, int cell_width, int cell_height, int gap) {
  Layout best;
  for (int cols = 1; cols <= 3; ++cols) {
    if (cols > count || (cols > 1 && (width - (cols - 1) * gap) / cols < cell_width)) continue;
    int rows = std::max(1, (height + gap) / std::max(1, cell_height + gap));
    int visible = std::min(count, rows * cols);
    if (visible > best.count || (visible == best.count && cols <= visible)) best = {cols, visible};
  }
  return best;
}
}  // namespace esphome::pv_overview::model
