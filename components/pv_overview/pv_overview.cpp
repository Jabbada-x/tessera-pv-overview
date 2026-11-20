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
      if (i < 2 || configured_[i]) order_[wanted_++] = i;
    }
    const int gap = std::max(1, ui::px(8));
    const int bh = ui::line_height(Font::BODY);
    const int min_cell_h = bh + ui::line_height(Font::TITLE) + gap;
    int footer_h = height_ >= min_cell_h * 2 + bh + gap ? bh + gap : 0;
    const int room = std::max(1, height_ - footer_h);
    const int min_cell_w = std::max(ui::px(132), ui::text_width(plugin_->text("export"), Font::BODY));
    const auto plan = model::layout(width_, room, compact ? std::min(wanted_, 4) : wanted_, min_cell_w, min_cell_h, gap);
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
    if (footer_) ui::set_color(footer_, theme::MUTED);
    paint(tessera::epoch());
  }
 private:
  const char *caption_key(int i, Status status) const {
    if (status != Status::OK) return KEYS[i];
    if (i == 1) return direction_[i] > 0 ? "import" : direction_[i] < 0 ? "export" : "grid";
    if (i == 3) return direction_[i] > 0 ? "charging" : direction_[i] < 0 ? "discharging" : "battery";
    return KEYS[i];
  }
  void paint(uint32_t epoch) {
    bool problem = false;
    for (int j = 0; j < visible_; ++j) {
      int i = order_[j];
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
      const int bh = ui::line_height(Font::BODY);
      const bool show_caption = cell_h_ >= bh * 2;
      const int available_h = show_caption ? cell_h_ - bh - ui::px(2) : cell_h_;
      Font face = Font::BODY;
      for (Font candidate : {Font::VALUE, Font::HEADLINE, Font::TITLE, Font::BODY}) {
        if (ui::text_width(value, candidate) <= cell_w_ && ui::line_height(candidate) <= available_h) { face = candidate; break; }
      }
      if (ui::text_width(value, face) > cell_w_) value = "--";
      ui::set_font(value_[i], face);
      ui::set_text(value_[i], value);
      ui::set_tone(value_[i], tone, theme::INK);
      const int total = ui::line_height(face) + (show_caption ? bh + ui::px(2) : 0);
      const int top = y_[i] + std::max(0, (cell_h_ - total) / 2);
      lv_obj_set_pos(caption_[i], x_[i], top);
      ui::set_text(caption_[i], show_caption ? label : "");
      lv_obj_set_pos(value_[i], x_[i], top + (show_caption ? bh + ui::px(2) : 0));
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
  lv_obj_t *footer_ = nullptr;
  int width_ = 0, height_ = 0, cell_w_ = 0, cell_h_ = 0, wanted_ = 0, visible_ = 0;
  uint32_t max_age_ = 0;
  double deadband_ = 10;
  bool grid_invert_ = false, battery_invert_ = false;
};
void PvOverview::setup() { add_tile("overview", [this]() { return new OverviewTile(this); }); }
}  // namespace esphome::pv_overview
