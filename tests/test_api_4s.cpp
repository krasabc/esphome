#include "esphome/components/climate/climate_mode.h"

#include "../components/tion-api/tion-api-4s-internal.h"
#include "../components/tion_4s/climate/tion_4s_climate.h"
#include "../components/tion_4s_uart/tion_4s_uart.h"

#include "test_api.h"
#include "test_vport.h"

DEFINE_TAG;

using namespace esphome;
using namespace esphome::tion;
using namespace dentra::tion;
using namespace dentra::tion_4s;

using Tion4sBleVPortApiTest = TionVPortApi<Tion4sBleIOTest::frame_spec_type, TionApi4s>;
// using Tion4sUartVPortApiTest = TionVPortApi<Tion4sUartIOTest::frame_spec_type, TionApi4s>;

class Tion4sUartVPortApiTest : public TionVPortApi<Tion4sUartIOTest::frame_spec_type, TionApi4s> {
 public:
  Tion4sUartVPortApiTest(Tion4sUartVPort *vport) : TionVPortApi<Tion4sUartIOTest::frame_spec_type, TionApi4s>(vport) {
    this->set_writer(
        TionApi4s::writer_type::create<Tion4sUartVPortApiTest, &Tion4sUartVPortApiTest::test_write_>(*this));

    this->state_.max_fan_speed = 6;
    this->state_.fan_speed = 1;
    this->state_.counters.work_time = 0xFFFF;
  }

 protected:
  tion4s_state_t state_{};
  bool test_write_(uint16_t type, const void *data, size_t size) {
    struct req_t {
      uint32_t request_id;
      tion4s_state_set_t data;
    } __attribute__((__packed__));

    if (type == FRAME_TYPE_STATE_SET && size == sizeof(req_t)) {
      const auto *req = static_cast<const req_t *>(data);

      this->state_.flags.power_state = req->data.power_state;
      this->state_.flags.heater_mode = req->data.heater_mode;
      if (req->data.fan_speed > 0 && req->data.fan_speed <= this->state_.max_fan_speed) {
        this->state_.fan_speed = req->data.fan_speed;
      }
      this->state_.target_temperature = req->data.target_temperature;
      this->state_.gate_position = req->data.gate_position;
      // add others

      this->on_state(this->state_, req->request_id);
    }
    return this->write_frame_(type, data, size);
  }
};

class Tion4sBleVPortTest : public Tion4sBleVPort {
 public:
  Tion4sBleVPortTest(Tion4sBleIOTest *io) : Tion4sBleVPort(io) {}
  // uint16_t get_state_type() const { return this->state_type_; }
};

class Tion4sTest : public Tion4sClimate {
 public:
  Tion4sTest(TionApi4s *api, esphome::tion::TionVPortType vport_type) : Tion4sClimate(api, vport_type) {
    this->state_.counters.work_time = 0xFFFF;
  }
  tion4s_state_t &state() { return this->state_; };

  void update_preset_service(std::string preset_str, std::string mode_str, int fan_speed, int target_temperature,
                             std::string gate_position_str) {
    this->update_preset_service_(preset_str, mode_str, fan_speed, target_temperature, gate_position_str);
  }

  const esphome::tion::TionPreset &get_preset(esphome::climate::ClimatePreset index) const {
    return this->presets_[index];
  }

  void for_each_preset(const std::function<void(esphome::climate::ClimatePreset index)> &fn) const {
    this->for_each_preset_(fn);
  }
};

/*
bool test_api_4s_x() {
  bool res = true;
  TestTionLtBleProtocol p;
  for (auto data : test_4s_data) {
    Api4sTest t4s(&p);
    // p.set_api(&t4s);
    for (auto d : data.frames) {
      p.read_data(cloak::from_hex(d));
    }
    test_check(res, t4s.received_struct_, data.await_struct);
  }
  return res;
}
*/

bool test_api_4s() {
  bool res = true;

  esphome::ble_client::BLEClient client;
  client.set_address(0x112233445566);
  client.connect();

  Tion4sBleIOTest io(&client);
  Tion4sBleVPortTest vport(&io);
  Tion4sBleVPortApiTest api(&vport);
  // Tion4sTest comp(&api);

  io.node_state = esphome::esp32_ble_tracker::ClientState::ESTABLISHED;
  // vport.set_persistent_connection(true);

  // cloak::setup_and_loop({&vport, &comp});

  api.request_state();
  res &= cloak::check_data("request_state", io, "80.0C.00.3A.AD 32.32 01.00.00.00 64.F7");

  api.request_dev_info();
  res &= cloak::check_data("request_dev_info", io, "80.0C.00.3A.AD 32.33 01.00.00.00 CE.A6");

  api.request_timer(0, 1);
  res &= cloak::check_data("request_timer", io, "80.11.00.3A.AD 32.34 01.00.00.00 01.00.00.00.00 DB.D5");

  api.request_timers(1);
  res &= cloak::check_data("request_timers", io,
                           "80.11.00.3A.AD.32.34.01.00.00.00.01.00.00.00.00.DB.D5"
                           "80.11.00.3A.AD.32.34.01.00.00.00.01.00.00.00.01.CB.F4"
                           "80.11.00.3A.AD.32.34.01.00.00.00.01.00.00.00.02.FB.97"
                           "80.11.00.3A.AD.32.34.01.00.00.00.01.00.00.00.03.EB.B6"
                           "80.11.00.3A.AD.32.34.01.00.00.00.01.00.00.00.04.9B.51"
                           "80.11.00.3A.AD.32.34.01.00.00.00.01.00.00.00.05.8B.70"
                           "80.11.00.3A.AD.32.34.01.00.00.00.01.00.00.00.06.BB.13"
                           "80.11.00.3A.AD.32.34.01.00.00.00.01.00.00.00.07.AB.32"
                           "80.11.00.3A.AD.32.34.01.00.00.00.01.00.00.00.08.5A.DD"
                           "80.11.00.3A.AD.32.34.01.00.00.00.01.00.00.00.09.4A.FC"
                           "80.11.00.3A.AD.32.34.01.00.00.00.01.00.00.00.0A.7A.9F"
                           "80.11.00.3A.AD.32.34.01.00.00.00.01.00.00.00.0B.6A.BE");

  return res;
}

bool test_presets() {
  bool res = true;

  esphome::uart::UARTComponent uart;
  Tion4sUartIOTest io(&uart);
  Tion4sUartVPort vport(&io);
  Tion4sUartVPortApiTest api(&vport);
  Tion4sTest comp(&api, vport.get_type());

  cloak::setup_and_loop({&vport, &comp});

  auto call = comp.make_call();

  call.set_preset(esphome::climate::ClimatePreset::CLIMATE_PRESET_HOME);
  comp.control(call);

  call.set_preset(esphome::climate::ClimatePreset::CLIMATE_PRESET_NONE);
  comp.control(call);

  // проверяем, что не упали в неинициализированное состояние
  res &= cloak::check_data("target_temperature > 0", comp.target_temperature > 0, true);

  comp.control(comp.make_call().set_fan_mode(std::string("3")));

  call.set_preset(esphome::climate::ClimatePreset::CLIMATE_PRESET_BOOST);
  comp.control(call);

  res &= cloak::check_data("fan_speed == 6", comp.get_fan_speed(), 6);

  call.set_preset(esphome::climate::ClimatePreset::CLIMATE_PRESET_NONE);
  comp.control(call);

  res &= cloak::check_data("fan_speed == 3", comp.get_fan_speed(), 3);

  return res;
}

bool test_heat_cool() {
  bool res = true;

  esphome::uart::UARTComponent uart;
  Tion4sUartIOTest io(&uart);
  Tion4sUartVPort vport(&io);
  Tion4sUartVPortApiTest api(&vport);
  Tion4sTest comp(&api, vport.get_type());

  cloak::setup_and_loop({&vport, &comp});

  for (auto mode : {climate::ClimateMode::CLIMATE_MODE_FAN_ONLY, climate::ClimateMode::CLIMATE_MODE_HEAT}) {
    auto call = comp.make_call();

    call.set_mode(mode);
    comp.control(call);

    call.set_mode(esphome::climate::ClimateMode::CLIMATE_MODE_OFF);
    comp.control(call);

    call.set_mode(esphome::climate::ClimateMode::CLIMATE_MODE_HEAT_COOL);
    comp.control(call);

    auto str = std::string("climate mode ") + LOG_STR_ARG(climate::climate_mode_to_string(mode));
    res &= cloak::check_data(str, comp.mode == mode, true);
  }

  return res;
}

bool test_preset_update() {
  bool res = true;

  esphome::uart::UARTComponent uart;
  Tion4sUartIOTest io(&uart);
  Tion4sUartVPort vport(&io);
  Tion4sUartVPortApiTest api(&vport);
  Tion4sTest comp(&api, vport.get_type());

  cloak::setup_and_loop({&vport, &comp});

  comp.for_each_preset([&comp](auto index) { comp.update_preset(index, esphome::climate::CLIMATE_MODE_OFF); });

  comp.update_preset_service("eco", "fan_only", 1, 11, "outdoor");
  auto preset = comp.get_preset(esphome::climate::CLIMATE_PRESET_ECO);
  res &= cloak::check_data("preset.fan_speed==1", preset.fan_speed, 1);
  res &= cloak::check_data("preset.target_temperature==11", preset.target_temperature, 11);
  res &= cloak::check_data("preset.mode==fan_only", preset.mode, esphome::climate::CLIMATE_MODE_FAN_ONLY);

  res &= cloak::check_data("preset.gate_position==outdoor", preset.gate_position,
                           esphome::tion::TION_CLIMATE_GATE_POSITION_OUTDOOR);

  comp.update_preset_service("eco", "fan_only", 1, 11, "indoor");
  preset = comp.get_preset(esphome::climate::CLIMATE_PRESET_ECO);
  res &= cloak::check_data("preset.gate_position==indoor", preset.gate_position,
                           esphome::tion::TION_CLIMATE_GATE_POSITION_INDOOR);

  comp.update_preset_service("eco", "fan_only", 1, 11, "mixed");
  preset = comp.get_preset(esphome::climate::CLIMATE_PRESET_ECO);
  res &= cloak::check_data("preset.gate_position==mixed", preset.gate_position,
                           esphome::tion::TION_CLIMATE_GATE_POSITION_MIXED);

  comp.update_preset_service("eco", "off", 1, 11, "mixed");
  preset = comp.get_preset(esphome::climate::CLIMATE_PRESET_ECO);
  res &= cloak::check_data("preset.mode==off", preset.mode, esphome::climate::CLIMATE_MODE_OFF);

  return res;
}

REGISTER_TEST(test_api_4s);
REGISTER_TEST(test_presets);
REGISTER_TEST(test_heat_cool);
REGISTER_TEST(test_preset_update);
