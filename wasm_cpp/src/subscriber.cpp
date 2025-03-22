#include <string>
#include <optional>

#include "rcutils/logging_macros.h"

#include <emscripten/emscripten.h>
#include <emscripten/val.h>

#include "wasm_cpp/subscriber.hpp"
#include "wasm_cpp/participant.hpp"
#include "wasm_cpp/modes.hpp"

namespace wasm_cpp
{
    Subscriber::Subscriber(const std::string & topic_name)
        : Participant(topic_name, "subscriber")
    {
        RCUTILS_LOG_DEBUG_NAMED("wasm_cpp", "trace Subscriber::Subscriber()");
        
        get_global_context()->register_subscriber(this);
    }

    

    Subscriber::Subscriber(const std::string & topic_name, const std::string & msg_type, const std::string & msg_namespace)
        : Participant(topic_name, "subscriber")
    {
        RCUTILS_LOG_DEBUG_NAMED("wasm_cpp", "trace Subscriber::Subscriber()");
        if (roslibjs_enable()){
            std::string module_name = msg_namespace.substr(0, msg_namespace.find("__"));
            std::string full_interface_name = msg_type;
            if (module_name.length() > 0)
                full_interface_name = module_name + "/" + full_interface_name;

            m_roslib_handle = get_global_context()->get_roslib_js().create_subscriber(
                topic_name,
                full_interface_name,
                [=] (const std::string &msg) {
                    push_message(msg);
                });
        }
        else{
            get_global_context()->register_subscriber(this);
        }

    }

    Subscriber::~Subscriber()
    {
        RCUTILS_LOG_DEBUG_NAMED("wasm_cpp", "trace Subscriber::~Subscriber()");

        if(roslibjs_enable()){
            get_global_context()->get_roslib_js().destroy_subscriber(m_roslib_handle);
        }
        else{
            get_global_context()->unregister_subscriber(this);
        }
    }

    std::string Subscriber::get_message()
    {
        RCUTILS_LOG_DEBUG_NAMED("wasm_cpp", "trace Subscriber::get_message()");

        std::string message;
        std::string topic_name{ get_name() };

        emscripten::val js_retrieve = emscripten::val::module_property("retrieveMessage");
        emscripten::val js_response = js_retrieve(topic_name).await();

        try {
            auto js_message = js_response.as<std::string>();
            if (!js_message.empty()) {
                message = js_message;
            } 
        }
        catch (...) {
            RCUTILS_LOG_ERROR_NAMED("wasm_cpp", "Unable to convert js message.");
        }

        return message;
    }

} // namespace wasm_cpp
