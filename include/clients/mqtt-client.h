#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <string>
#include <chrono>
#include <vector>
#include "mqtt/async_client.h"

const int QOS = 1;

class MQTTClient
{
public:
    MQTTClient(std::string server_address, std::string client_id, std::string mqtt_user_name, std::string mqtt_password, std::vector<std::string> topics);
    ~MQTTClient();
    mqtt::message::const_ptr_t consume_message();
    void publish_message(mqtt::message_ptr message);
    void update_topics(const std::vector<std::string> &new_topics);
    void stop_consuming();
private:
    std::vector<std::string> topics;
    mqtt::async_client cli;
};

#endif /*MQTT_CLIENT_H*/
