#include "../components/pv_overview/pv_model.h"
#include <cassert>
#include <iostream>
using namespace esphome::pv_overview::model;
int main() {
  const auto solar = parse("0.8", "kW", Kind::POWER, 100);
  auto balance = [&](const char *grid, uint32_t now = 100, unsigned age = 0) {
    return home_balance(solar, parse(grid, "W", Kind::POWER, 110), now, age);
  };
  assert(balance("150").value == 950);
  assert(balance("-150").value == 650);
  assert(balance("-800").status == Status::OK && balance("-800").value == 0);
  assert(balance("-801").status == Status::INVALID);
  assert(balance("unavailable").status == Status::UNAVAILABLE);
  assert(balance("150", 116, 15).status == Status::STALE);
  assert(home_balance(parse("", "W", Kind::POWER, 0), solar, 100, 0).status == Status::MISSING);
  assert(parse("1.5", "kW", Kind::POWER, 0).value == 1500);
  assert(parse("-500", "W", Kind::POWER, 0).value == -500);
  assert(parse("5400", "Wh", Kind::ENERGY, 0).value == 5.4);
  assert(parse("0", "W", Kind::POWER, 0).status == Status::OK);
  for (auto s : {"nan", "inf", "1e999", "12junk", " ", "--"})
    assert(parse(s, "W", Kind::POWER, 0).status == Status::INVALID);
  for (auto s : {"unknown", "unavailable"})
    assert(parse(s, "W", Kind::POWER, 0).status == Status::UNAVAILABLE);
  assert(parse("", "W", Kind::POWER, 0).status == Status::MISSING);
  assert(parse("42", "kWh", Kind::POWER, 0).status == Status::UNIT);
  assert(parse("42", "", Kind::POWER, 0).status == Status::UNIT);
  assert(parse("101", "%", Kind::PERCENT, 0).status == Status::INVALID);
  assert(parse("-1", "%", Kind::PERCENT, 0).status == Status::INVALID);
  assert(parse("-1", "kWh", Kind::ENERGY, 0).status == Status::INVALID);
  assert(parse("100", "%", Kind::PERCENT, 0).status == Status::OK);
  auto r = parse("400", "W", Kind::POWER, 1000);
  assert(status(r, 1100, 60) == Status::STALE);
  assert(status(r, 1060, 60) == Status::OK);
  assert(status(r, 2000, 0) == Status::OK);
  assert(status(r, 900, 60) == Status::OK);
  assert(status(r, 0, 60) == Status::OK);
  assert(direction(11, 0, 10) == 1);
  assert(direction(7, 1, 10) == 1);
  assert(direction(4, 1, 10) == 0);
  assert(direction(-11, 1, 10) == -1);
  assert(direction(-7, -1, 10) == -1);
  assert(direction(0, -1, 0) == 0);
  assert(format(1500, Kind::POWER) == "1.50 kW");
  assert(format(-350, Kind::POWER) == "350 W");
  assert(format(50, Kind::PERCENT) == "50 %");
  assert(layout(120, 85, 6, 132, 56, 8).count == 1);
  assert(layout(440, 360, 6, 132, 56, 8).count == 6);
  assert(layout(950, 500, 6, 132, 56, 8).count == 6);
  for (int w = 80; w <= 1400; w += 7) for (int h = 60; h <= 850; h += 11) for (int n = 2; n <= 6; ++n) {
    auto l = layout(w, h, n, 132, 56, 8);
    assert(l.count >= 1 && l.count <= n);
    assert(l.columns >= 1 && l.columns <= 3 && l.columns <= l.count);
    int rows = (l.count + l.columns - 1) / l.columns;
    assert((w - (l.columns - 1) * 8) / l.columns > 0);
    assert((h - (rows - 1) * 8) / rows >= 56);
  }
  std::cout << "PV model: parsing, units, states, signs, age and layout tests passed\n";
}
