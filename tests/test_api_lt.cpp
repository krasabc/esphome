#include "../components/tion_lt/climate/tion_lt_climate.h"

#include "test_api.h"
#include "test_vport.h"

DEFINE_TAG;

using namespace dentra::tion;

using TionLtBleVPortApiTest = esphome::tion::TionVPortApi<TionLtBleIOTest::frame_spec_type, dentra::tion::TionLtApi>;

class TionLtBleVPortTest : public esphome::tion::TionLtBleVPort {
 public:
  TionLtBleVPortTest(TionLtBleIOTest *io) : esphome::tion::TionLtBleVPort(io) {}
  // uint16_t get_state_type() const { return this->state_type_; }
};

class TionLtTest : public esphome::tion::TionLtClimate {
 public:
  TionLtTest(dentra::tion::TionLtApi *api, esphome::tion::TionVPortType vport_type)
      : esphome::tion::TionLtClimate(api, vport_type) {}
};

bool test_api_lt() {
  bool res = true;

  esphome::ble_client::BLEClient client;
  client.set_address(0x112233445566);
  client.connect();

  TionLtBleIOTest io(&client);
  TionLtBleVPortTest vport(&io);
  TionLtBleVPortApiTest api(&vport);
  // TionLtTest comp(&api);

  io.node_state = esphome::esp32_ble_tracker::ClientState::ESTABLISHED;
  // vport.set_persistent_connection(true);

  cloak::setup_and_loop({&vport});

  api.request_state();
  vport.call_loop();
  res &= cloak::check_data("request_state", io, "80.0C.00.3A.AD.32.12.01.00.00.00.6C.43");

  api.request_dev_info();
  vport.call_loop();
  res &= cloak::check_data("request_dev_info", io, "80.0C.00.3A.AD.09.40.01.00.00.00.D1.DC");

  tionlt_state_t st{};
  st.flags.power_state = true;
  st.flags.heater_state = true;
  st.fan_speed = 2;
  st.counters.work_time = 0xFFFFFFFF;
  st.counters.filter_time = 0xEEEEEEEE;

  api.write_state(st, 1);
  vport.call_loop();
  res &= cloak::check_data("write_state", io,
                           "00.1E.00.3A.AD."
                           "30.12."
                           "01.00.00.00."
                           "01.00.00.00."
                           "11.00.00.00.02.C0.00.00.00.00.00.00.00.00.00."
                           "9B.AB");

  return res;
}

REGISTER_TEST(test_api_lt);
