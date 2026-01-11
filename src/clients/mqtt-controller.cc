#include "clients/mqtt-controller.h"

#include <json/json.h>

#include <algorithm>
#include <cctype>
#include <sstream>

namespace {
std::string toLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return value;
}

std::string trim(const std::string &value) {
  size_t start = value.find_first_not_of(" \t\n\r");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = value.find_last_not_of(" \t\n\r");
  return value.substr(start, end - start + 1);
}

} // namespace

MqttController::MqttController(MQTTClient *client, ScreenState *state,
                               ScrollingLineScreen *scrolling_screen,
                               MqttControllerConfig config)
    : client(client), state(state), scrolling_screen(scrolling_screen),
      config(std::move(config)) {}

std::vector<std::string> MqttController::command_topics() const {
  std::vector<std::string> topics = {
      commandTopic("power"),
      commandTopic("brightness"),
      commandTopic("speed"),
      commandTopic("line1/options"),
      commandTopic("line2/options"),
      commandTopic("line1/rotate_seconds"),
      commandTopic("line2/rotate_seconds"),
      commandTopic("near_end_chars"),
      commandTopic("min_display_seconds"),
  };

  if (!config.light_command_topic.empty()) {
    topics.push_back(config.light_command_topic);
  }
  if (!config.light_brightness_command_topic.empty()) {
    topics.push_back(config.light_brightness_command_topic);
  }

  return topics;
}

void MqttController::publishStartupState() {
  publishLightState(state->screen_on);
  publishBrightnessState(state->current_brightness);
  publishSpeedState(state->speed);
  publishLineOptionsState();
  publishRotateState();
  publishPacingState();
}

void MqttController::publishDiscovery() {
  if (!config.enable_discovery || config.discovery_prefix.empty() ||
      config.device_id.empty()) {
    return;
  }

  const std::string base = config.discovery_prefix;
  const std::string device = config.device_id;

  Json::Value device_info;
  device_info["identifiers"].append(device);
  device_info["name"] = config.device_name;
  device_info["model"] = "RGB LED Matrix";
  device_info["manufacturer"] = "Custom";

  Json::Value light_config;
  light_config["name"] = config.device_name + " Power";
  light_config["unique_id"] = device + "_light";
  light_config["brightness"] = true;
  light_config["brightness_scale"] = 100;
  light_config["payload_on"] = "ON";
  light_config["payload_off"] = "OFF";
  light_config["state_topic"] = stateTopic("power");
  light_config["command_topic"] = commandTopic("power");
  light_config["brightness_state_topic"] = stateTopic("brightness");
  light_config["brightness_command_topic"] = commandTopic("brightness");
  light_config["device"] = device_info;

  Json::StreamWriterBuilder writer;
  const std::string light_payload = Json::writeString(writer, light_config);
  mqtt::message_ptr msg =
      mqtt::make_message(base + "/light/" + device + "/config", light_payload);
  msg->set_qos(1);
  msg->set_retained(true);
  client->publish_message(msg);
}

void MqttController::run(std::atomic<bool> *running) {
  while (running->load()) {
    try {
      auto message = client->consume_message();
      if (!message) {
        return;
      }
      handleMessage(message->get_topic(), message->to_string());
    } catch (const mqtt::exception &exc) {
      std::cerr << "Exception on MQTT handler thread" << exc << std::endl;
    } catch (const std::exception &exc) {
      std::cerr << "Exception on MQTT handler thread" << exc.what()
                << std::endl;
    }
  }
}

void MqttController::handleMessage(const std::string &topic,
                                   const std::string &payload) {
  if (topic == config.light_command_topic || topic == commandTopic("power")) {
    bool on = false;
    if (parseBool(payload, on)) {
      {
        std::lock_guard<std::mutex> lock(state->mutex);
        state->screen_on = on;
      }
      publishLightState(on);
    }
    return;
  }

  if (topic == config.light_brightness_command_topic ||
      topic == commandTopic("brightness")) {
    int brightness = 0;
    if (parseInt(payload, brightness)) {
      brightness = std::clamp(brightness, 0, 100);
      {
        std::lock_guard<std::mutex> lock(state->mutex);
        state->current_brightness = brightness;
      }
      publishBrightnessState(brightness);
    }
    return;
  }

  if (topic == commandTopic("speed")) {
    float speed = 0.0f;
    if (parseFloat(payload, speed)) {
      speed = std::clamp(speed, 0.0f, 10.0f);
      {
        std::lock_guard<std::mutex> lock(state->mutex);
        state->speed = speed;
      }
      publishSpeedState(speed);
    }
    return;
  }

  if (topic == commandTopic("line1/options")) {
    std::vector<LineType> options;
    if (parseLineOptions(payload, options)) {
      scrolling_screen->setLine1Options(std::move(options));
      publishLineOptionsState();
    }
    return;
  }

  if (topic == commandTopic("line2/options")) {
    std::vector<LineType> options;
    if (parseLineOptions(payload, options)) {
      scrolling_screen->setLine2Options(std::move(options));
      publishLineOptionsState();
    }
    return;
  }

  if (topic == commandTopic("line1/rotate_seconds")) {
    int value = 0;
    if (parseInt(payload, value)) {
      scrolling_screen->setLineRotateIntervals(
          std::chrono::seconds(value), scrolling_screen->line2RotateSeconds());
      publishRotateState();
    }
    return;
  }

  if (topic == commandTopic("line2/rotate_seconds")) {
    int value = 0;
    if (parseInt(payload, value)) {
      scrolling_screen->setLineRotateIntervals(
          scrolling_screen->line1RotateSeconds(), std::chrono::seconds(value));
      publishRotateState();
    }
    return;
  }

  if (topic == commandTopic("near_end_chars")) {
    int value = 0;
    if (parseInt(payload, value)) {
      scrolling_screen->setLinePacing(value,
                                      scrolling_screen->minDisplaySeconds());
      publishPacingState();
    }
    return;
  }

  if (topic == commandTopic("min_display_seconds")) {
    int value = 0;
    if (parseInt(payload, value)) {
      scrolling_screen->setLinePacing(scrolling_screen->nearEndChars(),
                                      std::chrono::seconds(value));
      publishPacingState();
    }
    return;
  }
}

void MqttController::publishLightState(bool on) {
  const std::string payload = on ? "ON" : "OFF";
  mqtt::message_ptr msg = mqtt::make_message(stateTopic("power"), payload);
  msg->set_qos(1);
  msg->set_retained(true);
  client->publish_message(msg);

  if (!config.light_state_topic.empty()) {
    mqtt::message_ptr legacy =
        mqtt::make_message(config.light_state_topic, payload);
    legacy->set_qos(1);
    legacy->set_retained(true);
    client->publish_message(legacy);
  }
}

void MqttController::publishBrightnessState(int brightness) {
  mqtt::message_ptr msg =
      mqtt::make_message(stateTopic("brightness"), std::to_string(brightness));
  msg->set_qos(1);
  msg->set_retained(true);
  client->publish_message(msg);

  if (!config.light_brightness_state_topic.empty()) {
    mqtt::message_ptr legacy = mqtt::make_message(
        config.light_brightness_state_topic, std::to_string(brightness));
    legacy->set_qos(1);
    legacy->set_retained(true);
    client->publish_message(legacy);
  }
}

void MqttController::publishSpeedState(float speed) {
  mqtt::message_ptr msg =
      mqtt::make_message(stateTopic("speed"), std::to_string(speed));
  msg->set_qos(1);
  msg->set_retained(true);
  client->publish_message(msg);
}

void MqttController::publishLineOptionsState() {
  auto formatOptions = [](const std::vector<LineType> &options) {
    Json::Value array(Json::arrayValue);
    for (LineType type : options) {
      array.append(LineTypeToString(type));
    }
    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, array);
  };

  mqtt::message_ptr line1 =
      mqtt::make_message(stateTopic("line1/options"),
                         formatOptions(scrolling_screen->line1Options()));
  line1->set_qos(1);
  line1->set_retained(true);
  client->publish_message(line1);

  mqtt::message_ptr line2 =
      mqtt::make_message(stateTopic("line2/options"),
                         formatOptions(scrolling_screen->line2Options()));
  line2->set_qos(1);
  line2->set_retained(true);
  client->publish_message(line2);
}

void MqttController::publishRotateState() {
  mqtt::message_ptr line1 = mqtt::make_message(
      stateTopic("line1/rotate_seconds"),
      std::to_string(scrolling_screen->line1RotateSeconds().count()));
  line1->set_qos(1);
  line1->set_retained(true);
  client->publish_message(line1);

  mqtt::message_ptr line2 = mqtt::make_message(
      stateTopic("line2/rotate_seconds"),
      std::to_string(scrolling_screen->line2RotateSeconds().count()));
  line2->set_qos(1);
  line2->set_retained(true);
  client->publish_message(line2);
}

void MqttController::publishPacingState() {
  mqtt::message_ptr near_end =
      mqtt::make_message(stateTopic("near_end_chars"),
                         std::to_string(scrolling_screen->nearEndChars()));
  near_end->set_qos(1);
  near_end->set_retained(true);
  client->publish_message(near_end);

  mqtt::message_ptr min_display = mqtt::make_message(
      stateTopic("min_display_seconds"),
      std::to_string(scrolling_screen->minDisplaySeconds().count()));
  min_display->set_qos(1);
  min_display->set_retained(true);
  client->publish_message(min_display);
}

bool MqttController::parseBool(const std::string &payload, bool &value) const {
  const std::string lower = toLower(trim(payload));
  if (lower == "on" || lower == "1" || lower == "true") {
    value = true;
    return true;
  }
  if (lower == "off" || lower == "0" || lower == "false") {
    value = false;
    return true;
  }
  return false;
}

bool MqttController::parseInt(const std::string &payload, int &value) const {
  try {
    value = std::stoi(trim(payload));
    return true;
  } catch (...) {
    return false;
  }
}

bool MqttController::parseFloat(const std::string &payload,
                                float &value) const {
  try {
    value = std::stof(trim(payload));
    return true;
  } catch (...) {
    return false;
  }
}

bool MqttController::parseLineOptions(const std::string &payload,
                                      std::vector<LineType> &options) const {
  const std::string trimmed = trim(payload);
  if (trimmed.empty()) {
    return false;
  }

  std::vector<std::string> tokens;
  if (!trimmed.empty() && trimmed.front() == '[') {
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(trimmed, root) || !root.isArray()) {
      return false;
    }
    for (const auto &value : root) {
      if (value.isString()) {
        tokens.push_back(value.asString());
      }
    }
  } else {
    std::stringstream ss(trimmed);
    std::string item;
    while (std::getline(ss, item, ',')) {
      tokens.push_back(item);
    }
  }

  options.clear();
  for (const auto &token : tokens) {
    LineType type = LineType::CurrentTime;
    if (TryParseLineType(token, type)) {
      options.push_back(type);
    }
  }

  return !options.empty();
}

std::string MqttController::stateTopic(const std::string &suffix) const {
  if (config.base_topic.empty()) {
    return suffix;
  }
  return config.base_topic + "/state/" + suffix;
}

std::string MqttController::commandTopic(const std::string &suffix) const {
  if (config.base_topic.empty()) {
    return suffix;
  }
  return config.base_topic + "/set/" + suffix;
}
