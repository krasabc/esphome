#pragma once

#include "../tion_o2.h"
#include "../../tion/tion_climate_component.h"

namespace esphome {
namespace tion {

using dentra::tion_o2::tiono2_state_t;

class TionO2Climate : public TionClimateComponent<TionO2Api> {
 public:
  explicit TionO2Climate(TionO2Api *api, TionVPortType vport_type) : TionClimateComponent(api, vport_type) {}

  void dump_config() override;

  void on_ready() {
    this->api_->request_connect();
    TionClimateComponent::on_ready();
  }

  void update_state(const tiono2_state_t &state) override;
  void dump_state(const tiono2_state_t &state) const;

  TionGatePosition get_gate_position() const override { return TionGatePosition::NONE; }

  void reset_filter() { this->api_->reset_filter(this->state_); }

  void control_climate_state(climate::ClimateMode mode, uint8_t fan_speed, float target_temperature,
                             TionGatePosition gate_position) override;

  void control_buzzer_state(bool state) {}

 protected:
  class TionO2Call : public TionCall<TionO2Climate> {
   public:
    explicit TionO2Call(TionO2Climate *parent) : TionCall(parent) {}
    void perform();
  };

  TionO2Call make_tion_call() { return TionO2Call(this); }
};

}  // namespace tion
}  // namespace esphome
