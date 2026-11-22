#include "pv_overview.h"
#include "pv_model.h"
#include <array>

namespace esphome::pv_overview {
using tessera::Font;
using tessera::Tone;
namespace ui = tessera::ui;
using model::Kind;
using model::Status;
static constexpr const char *KEYS[] = {"solar", "grid", "home", "battery", "soc", "today"};
static constexpr Kind KINDS[] = {Kind::POWER, Kind::POWER, Kind::POWER, Kind::POWER, Kind::PERCENT, Kind::ENERGY};

class OverviewTile : public tessera::Tile {
 public:
  explicit OverviewTile(const tessera::Plugin *plugin) : plugin_(plugin) {}
  void create(const tessera::TileContext &c) override {
    width_ = c.width;
    height_ = c.height;
    grid_invert_ = c.options["grid_invert"] | false;
    battery_invert_ = c.options["battery_invert"] | false;
    deadband_ = c.options["deadband"] | 10.0;
    max_age_ = static_cast<uint32_t>(c.options["stale_minutes"] | 0) * 60;
    bool compact = std::strcmp(c.options["layout"] | "auto", "compact") == 0;
    for (int i = 0; i < 6; ++i) {
      configured_[i] = *(c.options[KEYS[i]] | "") != '\0';
      if (i < 3 || configured_[i]) order_[wanted_++] = i;
    }
    const int gap = std::max(1, ui::px(8));
    const int bh = ui::line_height(Font::BODY);
    const int icon_h = ui::line_height(Font::ICON_SMALL);
    const int arrow_space = std::max(icon_h, ui::text_width(ui::icon(0xF0054), Font::ICON_SMALL)) + gap;
    int card_min_w = std::max(ui::px(88), ui::text_width(plugin_->text("export"), Font::BODY) + gap);
    if (configured_[5]) card_min_w = std::max(card_min_w, ui::text_width(std::string(plugin_->text("today")) + " 99.99 kWh", Font::BODY));
    const int card_min_h = icon_h + bh + ui::line_height(Font::TITLE) + gap;
    const bool second_row = configured_[3] || configured_[4];
    const int row_count = second_row ? 2 : 1;
    flow_ = !compact && width_ >= 3 * card_min_w + 2 * arrow_space &&
            height_ >= row_count * card_min_h + (second_row ? arrow_space + 3 * gap : 0) + (configured_[5] ? bh + gap : 0) &&
            (width_ - 2 * arrow_space) / 3 >= card_min_h + gap;
    if (flow_) {
      create_flow(c.parent, arrow_space);
      return;
    }
    const bool group_day = configured_[5] && (!compact || wanted_ <= 4) && height_ >= 2 * bh + ui::line_height(Font::TITLE) + gap;
    if (group_day) --wanted_;  // The last channel shares the solar cell.
    const int min_cell_h = bh + ui::line_height(Font::TITLE) + gap + (group_day ? bh : 0);
    int footer_h = height_ >= min_cell_h * 2 + bh + gap ? bh + gap : 0;
    const int room = std::max(1, height_ - footer_h);
    const int min_cell_w = std::max(ui::px(132), ui::text_width(plugin_->text("export"), Font::BODY));
    const auto plan = model::layout(width_, room, compact ? std::min(wanted_, group_day ? 3 : 4) : wanted_, min_cell_w, min_cell_h, gap);
    visible_ = plan.count;
    const int rows = (visible_ + plan.columns - 1) / plan.columns;
    cell_w_ = std::max(1, (width_ - (plan.columns - 1) * gap) / plan.columns);
    cell_h_ = std::max(1, (room - (rows - 1) * gap) / rows);
    for (int j = 0; j < visible_; ++j) {
      int i = order_[j];
      x_[i] = (j % plan.columns) * (cell_w_ + gap);
      y_[i] = (j / plan.columns) * (cell_h_ + gap);
      caption_[i] = ui::label(c.parent, Font::BODY, theme::MUTED);
      value_[i] = ui::label(c.parent, Font::VALUE, theme::INK);
      lv_obj_set_width(caption_[i], cell_w_);
      lv_obj_set_width(value_[i], cell_w_);
    }
    if (group_day) create_day(c.parent);
    if (footer_h) {
      footer_ = ui::label(c.parent, Font::BODY, theme::MUTED);
      lv_obj_set_pos(footer_, 0, height_ - bh);
      lv_obj_set_width(footer_, width_);
    }
  }
  void on_state(JsonObjectConst data) override {
    for (int i = 0; i < 6; ++i) {
      JsonObjectConst e = data["entities"][KEYS[i]].as<JsonObjectConst>();
      readings_[i] = model::parse(e["state"] | "", e["attributes"]["unit_of_measurement"] | "", KINDS[i], e["updated"] | 0u);
      if (configured_[i] && readings_[i].status == Status::MISSING) readings_[i].status = Status::UNAVAILABLE;
      if ((i == 0 || i == 2) && readings_[i].status == Status::OK && readings_[i].value < 0) readings_[i].status = Status::INVALID;
      if ((i == 1 && grid_invert_) || (i == 3 && battery_invert_)) readings_[i].value *= -1;
      if (i == 1 || i == 3) direction_[i] = readings_[i].status == Status::OK ? model::direction(readings_[i].value, direction_[i], deadband_) : 0;
    }
  }
  void on_tick(uint32_t epoch) override { paint(epoch); }
  void on_theme() override {
    for (int i = 0; i < 6; ++i) if (caption_[i]) ui::set_color(caption_[i], theme::MUTED);
    if (pack_) style_pack();
    if (flow_) lv_obj_set_style_border_color(panels_[2], ui::tone_color(Tone::GOOD, theme::INK), 0);
    if (footer_) ui::set_color(footer_, theme::MUTED);
    paint(tessera::epoch());
  }
 private:
  void create_flow(lv_obj_t *parent, int lane) {
    const bool has_pack = configured_[3] || configured_[4];
    const int gap = std::max(1, ui::px(8));
    const int body = ui::line_height(Font::BODY);
    const int title = ui::line_height(Font::TITLE);
    const int icon = ui::line_height(Font::ICON_SMALL);
    const int pack_h = std::max(icon, (configured_[3] ? body : 0) + (configured_[4] ? title : 0) +
                                       (configured_[3] && configured_[4] ? gap : 0)) + 2 * gap;
    const int top_h = height_ - (has_pack ? pack_h + lane : 0);
    const int diameter = std::min(top_h, (width_ - 2 * lane) / 3);
    const int cy = top_h / 2;
    const int cx = width_ / 2;
    cell_w_ = (width_ - diameter - 2 * lane) / 2;
    cell_h_ = top_h;
    visible_ = wanted_;
    auto *hub = panels_[2] = ui::block(parent, theme::RAISED);
    lv_obj_set_pos(hub, cx - diameter / 2, cy - diameter / 2);
    lv_obj_set_size(hub, diameter, diameter);
    lv_obj_set_style_radius(hub, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(hub, std::max(1, ui::px(2)), 0);
    lv_obj_set_style_border_color(hub, ui::tone_color(Tone::GOOD, theme::INK), 0);
    const int centers[] = {cell_w_ / 2, width_ - cell_w_ / 2, cx};
    const uint32_t symbols[] = {0xF0599, 0xF0D3E, 0xF02DC};
    for (int i = 0; i < 3; ++i) {
      int w = i == 2 ? diameter * 4 / 5 : cell_w_;
      flow_width_[i] = w;
      x_[i] = centers[i] - w / 2;
      icons_[i] = ui::label(parent, Font::ICON_SMALL, theme::ACCENT);
      ui::set_text(icons_[i], ui::icon(symbols[i]));
      caption_[i] = ui::label(parent, Font::BODY, theme::MUTED);
      value_[i] = ui::label(parent, Font::TITLE, theme::INK);
      for (auto *part : {icons_[i], caption_[i], value_[i]}) {
        lv_obj_set_width(part, w);
        lv_obj_set_style_text_align(part, LV_TEXT_ALIGN_CENTER, 0);
      }
    }
    const int hub_left = cx - diameter / 2;
    add_link(parent, 0, hub_left - lane / 2, cy, lane, false);
    add_link(parent, 1, hub_left + diameter + lane / 2, cy, lane, false);
    if (has_pack) {
      const int symbol_w = 2 * icon;
      const int text_space = std::max(1, width_ - symbol_w - 6 * gap);
      int text_w = configured_[4] ? ui::text_width("100 %", Font::TITLE) : 0;
      if (configured_[3]) {
        for (auto key : {"charging", "discharging", "standby", "battery"})
          text_w = std::max(text_w, ui::text_width(std::string(plugin_->text(key)) + " · -99.99 kW", Font::BODY));
      }
      text_w = std::min(text_w, text_space);
      const int pack_w = symbol_w + text_w + 6 * gap;
      const int bx = (width_ - pack_w) / 2, by = top_h + lane;
      const int right = bx + symbol_w + 4 * gap;
      pack_ = ui::block(parent, theme::RAISED);
      lv_obj_set_pos(pack_, bx, by);
      lv_obj_set_size(pack_, pack_w, pack_h);
      lv_obj_set_style_radius(pack_, gap, 0);
      const int stroke = std::max(1, ui::px(2));
      lv_obj_set_style_border_width(pack_, stroke, 0);
      const int sx = bx + 2 * gap, sy = by + (pack_h - icon) / 2;
      const int shell_w = symbol_w - 2 * stroke;
      shell_ = ui::block(parent, theme::RAISED);
      lv_obj_set_pos(shell_, sx, sy);
      lv_obj_set_size(shell_, shell_w, icon);
      lv_obj_set_style_radius(shell_, stroke * 2, 0);
      lv_obj_set_style_border_width(shell_, stroke, 0);
      cap_ = ui::block(parent, theme::BATTERY);
      lv_obj_set_pos(cap_, sx + shell_w, sy + icon / 3);
      lv_obj_set_size(cap_, 2 * stroke, std::max(1, icon / 3));
      charge_bar_ = ui::block(parent, theme::BATTERY);
      charge_width_ = std::max(1, shell_w - 4 * stroke);
      lv_obj_set_pos(charge_bar_, sx + 2 * stroke, sy + 2 * stroke);
      lv_obj_set_size(charge_bar_, charge_width_, std::max(1, icon - 4 * stroke));
      lv_obj_add_flag(charge_bar_, LV_OBJ_FLAG_HIDDEN);
      style_pack();
      if (configured_[4]) {
        value_[4] = ui::label(parent, Font::TITLE, theme::INK);
        lv_obj_set_pos(value_[4], right, by + (configured_[3] ? gap : (pack_h - title) / 2));
        lv_obj_set_width(value_[4], text_w);
        flow_width_[4] = text_w;
      }
      if (configured_[3]) {
        value_[3] = ui::label(parent, Font::BODY, theme::INK);
        lv_obj_set_pos(value_[3], right, by + (configured_[4] ? title + 2 * gap : (pack_h - body) / 2));
        lv_obj_set_width(value_[3], text_w);
        flow_width_[3] = text_w;
        const int start = cy - diameter / 2 + diameter;
        add_link(parent, 3, cx, (start + by) / 2, by - start, true);
      }
    }
    if (configured_[5]) create_day(parent);
    hub_diameter_ = diameter;
  }

  void create_day(lv_obj_t *parent) {
    day_ = ui::label(parent, Font::BODY, theme::MUTED);
    lv_obj_set_width(day_, cell_w_);
    if (flow_) lv_obj_set_style_text_align(day_, LV_TEXT_ALIGN_CENTER, 0);
  }

  void style_pack() {
    const auto accent = ui::tone_color(Tone::GOOD, theme::BATTERY);
    const auto tint = lv_color_mix(accent, ui::color(theme::CARD), LV_OPA_20);
    const auto edge = lv_color_mix(accent, ui::color(theme::INK), LV_OPA_50);
    for (auto *part : {pack_, shell_}) {
      lv_obj_set_style_bg_color(part, tint, 0);
      lv_obj_set_style_border_color(part, edge, 0);
    }
    lv_obj_set_style_bg_color(cap_, edge, 0);
    lv_obj_set_style_bg_color(charge_bar_, accent, 0);
  }

  void add_link(lv_obj_t *parent, int i, int x, int y, int lane, bool vertical) {
    auto *line = tracks_[i] = ui::block(parent, theme::LINE);
    const int stroke = std::max(1, ui::px(2));
    const int w = vertical ? stroke : lane, h = vertical ? lane : stroke;
    lv_obj_set_size(line, w, h);
    lv_obj_set_pos(line, x - w / 2, y - h / 2);
    auto *arrow = arrows_[i] = ui::label(parent, Font::ICON_SMALL, theme::ACCENT);
    lv_obj_set_width(arrow, lane);
    lv_obj_set_style_text_align(arrow, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(arrow, x - lane / 2, y - ui::line_height(Font::ICON_SMALL) / 2);
  }

  void paint_link(int i, Status state, Tone tone) {
    if (!arrows_[i]) return;
    int direction = i == 0 ? (readings_[i].value > 0 ? 1 : 0) : direction_[i];
    if (state != Status::OK) direction = 0;
    uint32_t glyph = 0xF0054;  // solar to house
    if (i == 1) glyph = direction > 0 ? 0xF004D : 0xF0054;
    if (i == 3) glyph = direction > 0 ? 0xF0045 : 0xF005D;
    ui::set_text(arrows_[i], direction ? ui::icon(glyph) : "");
    if (i < 2 && direction) {
      const auto role = i == 0 ? theme::ENERGY_SOLAR : theme::ACCENT;
      ui::set_color(arrows_[i], role);
      ui::set_fill(tracks_[i], Tone::NORMAL, role);
      return;
    }
    ui::set_tone(arrows_[i], tone, theme::INK);
    ui::set_fill(tracks_[i], direction ? tone : Tone::NORMAL, theme::LINE);
  }
  const char *caption_key(int i, Status status) const {
    if (status != Status::OK) return KEYS[i];
    if (i == 1) return direction_[i] > 0 ? "import" : direction_[i] < 0 ? "export" : "grid";
    if (i == 3) return direction_[i] > 0 ? "charging" : direction_[i] < 0 ? "discharging" : "battery";
    return KEYS[i];
  }
  void paint(uint32_t epoch) {
    if (!configured_[2]) {
      const auto *battery = configured_[3] ? &readings_[3] : nullptr;
      readings_[2] = model::home_balance(readings_[0], readings_[1], epoch, max_age_, battery);
    }
    if (charge_bar_) {
      if (model::status(readings_[4], epoch, max_age_) == Status::OK && readings_[4].value > 0) {
        lv_obj_set_width(charge_bar_, std::max(1, static_cast<int>(charge_width_ * readings_[4].value / 100)));
        lv_obj_clear_flag(charge_bar_, LV_OBJ_FLAG_HIDDEN);
      } else lv_obj_add_flag(charge_bar_, LV_OBJ_FLAG_HIDDEN);
    }
    bool problem = false;
    for (int j = 0; j < visible_ + (!flow_ && day_ ? 1 : 0); ++j) {
      int i = j == visible_ ? 5 : order_[j];
      Status state = model::status(readings_[i], epoch, max_age_);
      std::string label = plugin_->text(caption_key(i, state));
      if (ui::text_width(label, Font::BODY) > cell_w_) label = plugin_->text(KEYS[i]);
      std::string value;
      Tone tone = Tone::NORMAL;
      if (state == Status::OK) {
        value = model::format(readings_[i].value, KINDS[i]);
        if ((i == 1 || i == 3) && readings_[i].value != 0) value = (readings_[i].value < 0 ? "-" : "+") + value;
        if (std::strcmp(plugin_->text("decimal"), ",") == 0) std::replace(value.begin(), value.end(), '.', ',');
        if (i == 0) tone = Tone::GOOD;
        if (i == 1) tone = direction_[i] > 0 ? Tone::BUSY : direction_[i] < 0 ? Tone::GOOD : Tone::NORMAL;
        if (i == 3) tone = direction_[i] > 0 ? Tone::GOOD : direction_[i] < 0 ? Tone::BUSY : Tone::NORMAL;
        if (i == 4) tone = readings_[i].value <= 20 ? Tone::BUSY : Tone::GOOD;
      } else {
        problem = true;
        const char *key = state == Status::MISSING ? "choose" : state == Status::UNIT ? "unit_error" : state == Status::STALE ? "stale" : state == Status::INVALID ? "invalid" : "offline";
        value = plugin_->text(key);
        tone = Tone::BUSY;
      }
      if (i == 5 && day_) {
        if (state == Status::OK) {
          const auto unit = value.find(" kWh");
          if (unit != std::string::npos && unit && value[unit - 1] == '0') value.erase(unit - 1, 1);
        }
        std::string line = label + " " + value;
        ui::set_text(day_, ui::text_width(line, Font::BODY) <= cell_w_ ? line : value);
        ui::set_color(day_, theme::MUTED);
        continue;
      }
      if (flow_ && i >= 3) {
        if (i == 3) {
          if (state == Status::OK && direction_[3] == 0) label = plugin_->text("standby");
          if (state == Status::OK && direction_[3] && !value.empty() && (value[0] == '+' || value[0] == '-'))
            value.erase(0, 1);
          std::string line = label + " · " + value;
          ui::set_text(value_[3], ui::text_width(line, Font::BODY) <= flow_width_[3] ? line : value);
          ui::set_color(value_[3], theme::INK);
          paint_link(3, state, tone);
          continue;
        }
        Font font = ui::text_width(value, Font::TITLE) <= flow_width_[i] ? Font::TITLE : Font::BODY;
        ui::set_font(value_[i], font);
        ui::set_text(value_[i], value);
        ui::set_tone(value_[i], tone, theme::INK);
        continue;
      }
      const int bh = ui::line_height(Font::BODY);
      const int icon_h = flow_ ? ui::line_height(Font::ICON_SMALL) + ui::px(2) : 0;
      const int room_w = flow_ ? flow_width_[i] : cell_w_;
      const int day_h = i == 0 && day_ ? bh + ui::px(2) : 0;
      const int room_h = flow_ && i == 2 ? hub_diameter_ - ui::px(6) : cell_h_ - day_h;
      const bool show_caption = room_h >= bh * 2;
      const int available_h = show_caption ? room_h - bh - ui::px(2) - icon_h : room_h;
      Font face = Font::BODY;
      for (Font candidate : {Font::VALUE, Font::HEADLINE, Font::TITLE, Font::BODY}) {
        if (ui::text_width(value, candidate) <= room_w && ui::line_height(candidate) <= available_h) { face = candidate; break; }
      }
      if (ui::text_width(value, face) > room_w) value = "--";
      ui::set_font(value_[i], face);
      ui::set_text(value_[i], value);
      ui::set_tone(value_[i], tone, theme::INK);
      const int total = icon_h + ui::line_height(face) + (show_caption ? bh + ui::px(2) : 0);
      const int top = y_[i] + std::max(0, (cell_h_ - total - day_h) / 2);
      if (flow_) {
        if (panels_[i]) ui::set_fill(panels_[i], Tone::NORMAL, theme::RAISED);
        if (i == 2) ui::set_tone(icons_[i], Tone::GOOD, theme::INK);
        else ui::set_color(icons_[i], i == 0 ? theme::ENERGY_SOLAR : theme::ACCENT);
        if (state == Status::OK) ui::set_color(value_[i], theme::INK);
        lv_obj_set_pos(icons_[i], x_[i], top);
        paint_link(i, state, tone);
      }
      if (day_h) lv_obj_set_pos(day_, x_[0], top + total + ui::px(2));
      lv_obj_set_pos(caption_[i], x_[i], top + icon_h);
      ui::set_text(caption_[i], show_caption ? label : "");
      lv_obj_set_pos(value_[i], x_[i], top + icon_h + (show_caption ? bh + ui::px(2) : 0));
    }
    if (footer_) {
      std::string text;
      if (visible_ < wanted_) text = std::to_string(visible_) + "/" + std::to_string(wanted_) + " " + plugin_->text("enlarge");
      else text = plugin_->text(problem ? "check_sensors" : "live");
      ui::set_text(footer_, text);
    }
  }
  const tessera::Plugin *plugin_;
  std::array<model::Reading, 6> readings_{};
  std::array<bool, 6> configured_{};
  std::array<int, 6> direction_{}, order_{}, x_{}, y_{};
  std::array<lv_obj_t *, 6> caption_{}, value_{};
  std::array<lv_obj_t *, 6> panels_{}, icons_{}, tracks_{}, arrows_{};
  lv_obj_t *pack_ = nullptr, *shell_ = nullptr, *cap_ = nullptr, *charge_bar_ = nullptr, *day_ = nullptr;
  int charge_width_ = 0;
  int hub_diameter_ = 0;
  std::array<int, 6> flow_width_{};
  bool flow_ = false;
  lv_obj_t *footer_ = nullptr;
  int width_ = 0, height_ = 0, cell_w_ = 0, cell_h_ = 0, wanted_ = 0, visible_ = 0;
  uint32_t max_age_ = 0;
  double deadband_ = 10;
  bool grid_invert_ = false, battery_invert_ = false;
};
void PvOverview::setup() { add_tile("overview", [this]() { return new OverviewTile(this); }); }
}  // namespace esphome::pv_overview
