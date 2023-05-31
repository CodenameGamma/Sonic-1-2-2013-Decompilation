#include "ccpp.h"
#include "ActiveSocket.h"

#include "nlohmann/json.hpp"

// The tcp socket
static CActiveSocket sock{};

bool ccpp::initialize_ex(const char *ip, std::uint16_t port, bool threaded)
{
    print_info("Attempting to connect to CrowdControl...");
    if (sock.Initialize()) {
        print_info("Attempting to connect to CrowdControl...");
        if (sock.Open(ip, port)) {
            this->ready = true;
            print_info("Connected to CrowdControl!");

            if (threaded) {
                this->threaded_update();
            }
            else {
                if (!sock.SetNonblocking()) {
                    print_error("Failed to set socket to NonBlocking!");
                }
            }

            return true;
        }
        else {
            print_error("Unable to connect to CrowdControl! Is CrowdControl open and ready?");
            shutdown = true;
            return false;
        }
    }
}

bool ccpp::initialize(bool threaded) 
{
    
    return ccpp::initialize_ex("127.0.0.1", 58430, threaded); }

void ccpp::check_socket()
{
    // Recieved packet from tcp socket
    std::vector<std::uint8_t> buffer{};
    if (sock.Receive(1)) {
        if (sock.GetData() == 0)
            return;
        // Recieve the data in chunks of 1 byte
        buffer.emplace_back((std::uint8_t)sock.GetData()[0]);
        // Until null term
        while (sock.Receive(1) && (std::uint8_t)sock.GetData()[0] != 0x00) {
            buffer.emplace_back((std::uint8_t)sock.GetData()[0]);
        }
        buffer.emplace_back((std::uint8_t)0x00);

        // Create new C array with size of the dynamic array
        std::uint8_t *data = (std::uint8_t *)std::malloc(buffer.size());
        // Copy the data into the new C array
        for (auto i = 0; i < buffer.size(); ++i) {
            data[i] = buffer[i];
        }
        buffer.clear();

        // Convert the json packet to c++ json
        try {
            auto json = nlohmann::json::parse(data);

            // Convert CrowdControl effect code in json to std::string
            std::string code = json["code"].dump();
            // Remove the double quotes that exist in the json code
            code.erase(std::remove(code.begin(), code.end(), '\"'), code.end());

            // continued...
            std::string viewer = json["viewer"].dump();
            viewer.erase(std::remove(viewer.begin(), viewer.end(), '\"'), viewer.end());

            // this is sent as an int
            std::string id = json["id"].dump();

            // Check if effect has been registered
            if (this->triggers[code] != 0) {
                ccpp::effect_t data;
                data.viewer = viewer.c_str();

                // If it has been registered, then run the callback
                auto status_code = this->triggers[code]();
                // Send the backend the status code with the id
                std::string status = va("{\"id\":%s,\"status\":%i}", id.c_str(), (int)status_code);

                // Send...
                if (!sock.Send((const std::uint8_t *)status.c_str(), status.size() + 1)) {
                    print_warning("Did not send effect status ");
                }
            }
            else {
                print_warning("The code \"%s\" has not been registered!", code.c_str());
            }
        }
        // Obligatory catch when using json...
        catch (std::exception e) {
            print_error("%s", e.what());
        }
    }
}

void ccpp::update()
{
    if (!this->shutdown) {
        if (!this->ready) {
            print_error("ccpp is not ready! ccpp::initalize has not been ran yet!");
            return;
        }

        this->check_socket();
    }
}

void ccpp::threaded_update()
{
    std::thread([this]() {
        while (!this->shutdown) {
            this->update();

            std::this_thread::sleep_for(this->thread_cooldown);
        }
    }).detach();
}

void ccpp::register_trigger(const std::string &key, std::function<ccpp::status_t()> callback) { this->triggers.insert({ key, callback }); }