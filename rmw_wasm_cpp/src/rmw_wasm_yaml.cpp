#include <string>

#include "rmw_wasm_cpp/rmw_wasm_yaml.hpp"
#include "rmw_wasm_cpp/rmw_types.hpp"

#include "dynmsg/message_reading.hpp"
#include "dynmsg/msg_parser.hpp"
#include "dynmsg/yaml_utils.hpp"

#include "rcutils/logging_macros.h"

#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"

namespace rmw_wasm_cpp
{
    namespace conversion
    {

        inline std::string yaml_to_string(const YAML::Node & yaml)
        {
            return dynmsg::yaml_to_string(yaml, false, false);
        }

        namespace c
        {
            std::string msg_to_yaml(
                const rosidl_typesupport_introspection_c__MessageMembers * members,
                const void * msg)
            {
                RCUTILS_LOG_DEBUG_NAMED("wasm_wasm", "trace conversion::c::msg_to_yaml()");

                RosMessage ros_msg{ };
                ros_msg.type_info = members;
                ros_msg.data = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(msg));
                YAML::Node yaml = dynmsg::c::message_to_yaml(ros_msg);
                const auto & yaml_str = yaml_to_string(yaml);
                RCUTILS_LOG_DEBUG_NAMED("MSG", "C: %s", yaml_str.c_str());
                return yaml_str;
            }

            bool yaml_to_msg(
                const rosidl_typesupport_introspection_c__MessageMembers * members,
                const std::string & yaml,
                void * ros_message,
                rcutils_allocator_t * allocator)
            {
                RCUTILS_LOG_DEBUG_NAMED("wasm_wasm", "trace conversion::c::yaml_to_msg()");

                RosMessage ros_msg = dynmsg::c::yaml_and_typeinfo_to_rosmsg(members, yaml, allocator);
                if (!ros_msg.data && !ros_msg.type_info) {
                    return false;
                }
                memcpy(ros_message, ros_msg.data, members->size_of_);
                // TODO: deallocate message
                // ros_message_destroy_with_allocator(&ros_msg, allocator);
                return true;
            }
            
        } // namespace c

        namespace cpp
        {
            std::string msg_to_yaml(
                const rosidl_typesupport_introspection_cpp::MessageMembers * members,
                const void * msg)
            {
                RCUTILS_LOG_DEBUG_NAMED("wasm_wasm", "trace conversion::cpp::msg_to_yaml()");

                RosMessage_Cpp ros_msg{ };
                ros_msg.type_info = members;
                ros_msg.data = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(msg));
                YAML::Node yaml = dynmsg::cpp::message_to_yaml(ros_msg);
                const auto & yaml_str = yaml_to_string(yaml);
                RCUTILS_LOG_DEBUG_NAMED("MSG", "C++: %s", yaml_str.c_str());
                return yaml_str;
            }

            bool yaml_to_msg(
                const rosidl_typesupport_introspection_cpp::MessageMembers * members,
                const std::string & yaml,
                void * ros_message)
            {
                RCUTILS_LOG_DEBUG_NAMED("wasm_wasm", "trace conversion::cpp::yaml_to_msg()");

                dynmsg::cpp::yaml_and_typeinfo_to_rosmsg(members, yaml, ros_message);
                return true;
            }

        } // namespace cpp

    } // namespace conversion

    std::string msg_to_yaml(
        const rmw_wasm_pub_t * publisher, 
        const void * msg)
    {
        RCUTILS_LOG_DEBUG_NAMED("wasm_wasm", "trace msg_to_yaml()");

        const rosidl_message_type_support_t * ts = nullptr;
        ts = get_message_typesupport_handle(
            &publisher->type_support,
            rosidl_typesupport_introspection_c__identifier);
        if (ts) {
            auto members = static_cast<
                const rosidl_typesupport_introspection_c__MessageMembers *>(
                ts->data);
            return conversion::c::msg_to_yaml(members, msg);
        }
        rcutils_error_string_t error_c = rcutils_get_error_string();
        rcutils_reset_error();

        ts = get_message_typesupport_handle(
            &publisher->type_support,
            rosidl_typesupport_introspection_cpp::typesupport_identifier);
        if (ts) {
            auto members = static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
            ts->data);
            return conversion::cpp::msg_to_yaml(members, msg);
        }
        rcutils_error_string_t error_cpp = rcutils_get_error_string();
        rcutils_reset_error();

        throw std::runtime_error(
                "msg_to_yaml unable to find type support:\n"
                "    C: " + std::string(error_c.str) + "\n"
                "  C++: " + std::string(error_cpp.str));
    }

    bool yaml_to_msg(
        const rmw_wasm_sub_t * subscription,
        const std::string & yaml,
        void * ros_message,
        rcutils_allocator_t * allocator)
    {
        RCUTILS_LOG_DEBUG_NAMED("wasm_wasm", "trace yaml_to_msg()");

        const rosidl_message_type_support_t * ts = nullptr;
        ts = get_message_typesupport_handle(
            &subscription->type_support,
            rosidl_typesupport_introspection_c__identifier);
        if (ts) {
            auto members = static_cast<const rosidl_typesupport_introspection_c__MessageMembers *>(
            ts->data);
            return conversion::c::yaml_to_msg(members, yaml, ros_message, allocator);
        }
        rcutils_error_string_t error_c = rcutils_get_error_string();
        rcutils_reset_error();

        ts = get_message_typesupport_handle(
            &subscription->type_support,
            rosidl_typesupport_introspection_cpp::typesupport_identifier);
        if (ts) {
            auto members = static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
            ts->data);
            return conversion::cpp::yaml_to_msg(members, yaml, ros_message);
        }
        rcutils_error_string_t error_cpp = rcutils_get_error_string();
        rcutils_reset_error();

        throw std::runtime_error(
                "msg_to_yaml unable to find type support:\n"
                "    C: " + std::string(error_c.str) + "\n"
                "  C++: " + std::string(error_cpp.str));
    }

} // namespace rmw_wasm_cpp