#pragma once

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <cstdarg>

#define println(__FMT__, ...)       std::printf(__FMT__ "\n", __VA_ARGS__);
#define print_info(__FMT__, ...)    println("[ INFO ][" __FUNCTION__ "]: " __FMT__, __VA_ARGS__);
#define print_warning(__FMT__, ...) println("[ WARNING ][" __FUNCTION__ "]: " __FMT__, __VA_ARGS__);
#define print_error(__FMT__, ...)   println("[ ERROR ][" __FUNCTION__ "]: " __FMT__, __VA_ARGS__);

// For usage of time, like 16ms (instead of chrono cast)
using namespace std::literals;

class ccpp final
{
public:
    // The effect data; id, viewer, etc
    struct effect_t {
        std::string id, viewer;
    };

    enum class status_t : std::uint8_t {
        success     = 0x00,
        failure     = 0x01, // Immediately requests a refund on the backend
        unavailable = 0x02, // Permanently makes the effect unusable until the session is restarted; probably shouldn't use this
        retry       = 0x03, // Tells the backend to keep trying the effect (up to 5 mins then refund)
        paused      = 0x06, // Pauses a timed effect
        resume      = 0x07, // Resumes a paused timed effect
        finished    = 0x08, // Stops a timed effect

        visible        = 0x80, // The effect should be show in the menu
        not_visible    = 0x81, // The effect should be hidden in the menu
        selectable     = 0x82, // The effect should be selectable in the menu
        not_selectable = 0x83, // The effect should be unselectable in the menu
    };

    // initialize the ccpplib before doing anything else! Use threaded if you do not have access to a main loop
    bool initialize_ex(const char *ip, std::uint16_t port, bool threaded);
    // initialize the ccpplib before doing anything else! Use threaded if you do not have access to a main loop
    bool initialize(bool threaded = false);
    // Non-Blocking update, used to roll out own implementation within main loop; do not call if initialize is called with true!
    void update();
    // Registers a callback to the desired CrowdControl effect code
    void register_trigger(const std::string &code, std::function<ccpp::status_t()> callback);
    // Sets the thread cooldown; not mutexed! call before initalize if threaded update
    void set_cooldown(std::chrono::milliseconds ms) { this->thread_cooldown = ms; }

    // Determines if initialize was called and socket was opened
    bool ready = false;
    // Tells the library to shutdown and cleanup
    bool shutdown = false;
    // The CrowdControl effect codes that are sent to the net socket and their callback
    std::unordered_map<std::string, std::function<ccpp::status_t()>> triggers;
    // Typical string VA
    static std::string va(const char *fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        char result[512]{};
        std::vsprintf(result, fmt, va);
        return std::string(result);
    }

private:
    // Threaded update; required to be threaded due to blocking TCP
    void threaded_update();
    // Tells ccpp to check if an effect has been sent
    void check_socket();
    // The ms sleep timing; default 16ms
    std::chrono::milliseconds thread_cooldown = 16ms;
};