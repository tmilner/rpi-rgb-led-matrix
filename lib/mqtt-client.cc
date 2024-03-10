#include "mqtt-client.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include <atomic>

using namespace std;

const auto TIMEOUT = std::chrono::seconds(10);

MQTTClient::MQTTClient(string server_address, string client_id, std::string mqtt_user_name, std::string mqtt_password, vector<string> topics) : cli{mqtt::async_client(server_address, client_id)}, topics{topics}
{
    auto connOpts = mqtt::connect_options_builder()
		.clean_session(true)
        .user_name(mqtt_user_name)
        .password(mqtt_password)
		.finalize();


    try {
        cli.start_consuming();
	    
        cout << "Connecting to MQTT Server at " << server_address << flush;
        auto tok = cli.connect(connOpts);

        auto rsp = tok->get_connect_response();

//May need to fix this, it will assume topics never change. 
        if(!rsp.is_session_present()) {
            for (string topic: topics)
            {
                cli.subscribe(topic, QOS)->wait();
            }
        }
		cout << "Connected to MQTT and subscribed." << endl;
    } catch (const mqtt::exception& exc) {
        cerr << exc << endl;
        throw exc;
    }
}

MQTTClient::~MQTTClient()
{
    if (this->cli.is_connected()) {
		cout << "\nShutting down and disconnecting from the MQTT server..." << flush;
        for (string topic: topics)
        {
            cli.unsubscribe(topic)->wait();
        }
		this->cli.stop_consuming();
		this->cli.disconnect()->wait();
		cout << "OK" << endl;
	}
}
mqtt::message::const_ptr_t MQTTClient::consume_message()
{
    return cli.consume_message();
}

void MQTTClient::publish_message(mqtt::message_ptr message)
{
    cli.publish(message);
}