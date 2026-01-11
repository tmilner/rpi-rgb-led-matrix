#ifndef MQTT_CONTROLLER_H
#define MQTT_CONTROLLER_H

#include "clients/mqtt-client.h"
#include "screens/screen_state.h"
#include "screens/scrolling-line-screen.h"

#include <atomic>
#include <string>
#include <vector>

struct MqttControllerConfig {
  std::string base_topic;
  std::string discovery_prefix;
  std::string device_id;
  std::string device_name;
  bool enable_discovery = true;

  std::string light_state_topic;
  std::string light_command_topic;
  std::string light_brightness_state_topic;
  std::string light_brightness_command_topic;
};

class MqttController {
public:
  MqttController(MQTTClient *client, ScreenState *state,
                 ScrollingLineScreen *scrolling_screen,
                 MqttControllerConfig config);

  std::vector<std::string> command_topics() const;
  void publishStartupState();
  void publishDiscovery();
  void run(std::atomic<bool> *running);

private:
  void handleMessage(const std::string &topic, const std::string &payload);
  void publishLightState(bool on);
  void publishBrightnessState(int brightness);
  void publishSpeedState(float speed);
  void publishLineOptionsState();
  void publishRotateState();
  void publishPacingState();

  bool parseBool(const std::string &payload, bool &value) const;
  bool parseInt(const std::string &payload, int &value) const;
  bool parseFloat(const std::string &payload, float &value) const;
  bool parseLineOptions(const std::string &payload,
                        std::vector<LineType> &options) const;

  std::string stateTopic(const std::string &suffix) const;
  std::string commandTopic(const std::string &suffix) const;

  MQTTClient *client;
  ScreenState *state;
  ScrollingLineScreen *scrolling_screen;
  MqttControllerConfig config;
};

#endif /*MQTT_CONTROLLER_H*/
