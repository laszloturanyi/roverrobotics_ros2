#pragma once
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "openrover_core_msgs/msg/raw_motor_command.hpp"
#include "openrover_core_msgs/msg/raw_data.hpp"
#include "openrover_core_msgs/msg/raw_command.hpp"
#include "rclcpp/time.hpp"
#include "diagnostic_msgs/msg/diagnostic_array.hpp"
#include "diagnostic_msgs/msg/key_value.hpp"
#include <data.hpp>
#include "timestamped.hpp"

namespace openrover
{
/// This node supervises a Connection node and translates between low-level commands and high-level commands.
class Rover : public rclcpp::Node
{
public:
  Rover();

protected:
  std::unordered_map<uint8_t, std::unique_ptr<Timestamped<std::array<uint8_t, 2>>>> most_recent_data;

  /// Speed (m/s) this rover will attain running its motors at full power forward.
  double top_speed_linear;
  /// Rotational speed (rad/s) this rover will attain pushing its motors at full efford in opposite directions
  double top_speed_angular;

  /// If both motor encoders have traveled n increments, this is how far the rover has traveled.
  double meters_per_encoder_unit;
  /// If the left motor encoder has traveled n increments more than the right motor encoder,
  /// this value times n is how far the rover has rotated
  double radians_per_delta_encoder_unit;

  template <typename T>
  T get_parameter_checked(std::string name, std::function<bool(T)> predicate, T fallback)
  {
    T value;
    bool has_param = get_parameter<T>(name, value);
    if (!has_param)
    {
      RCLCPP_WARN(get_logger(), "Parameter %s was not defined. Using %s", name.c_str(),
                  std::to_string(fallback).c_str());
      return fallback;
    }
    if (!predicate(value))
    {
      RCLCPP_WARN(get_logger(), "Parameter %s=%s was not valid. Using %s", name.c_str(), std::to_string(value).c_str(),
                  std::to_string(fallback).c_str());
      return fallback;
    }
    return value;
  }

  /// Callback for velocity commands
  void on_velocity(geometry_msgs::msg::Twist::SharedPtr);
  /// Callback for new raw data received
  void on_raw_data(openrover_core_msgs::msg::RawData::SharedPtr data);

  rclcpp::TimerBase::SharedPtr tmr_diagnostics;
  void update_diagnostics();
  std::shared_ptr<std::vector<diagnostic_msgs::msg::KeyValue>> pending_diagnostics;
  rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr pub_diagnostics;

  rclcpp::TimerBase::SharedPtr tmr_odometry;
  void update_odometry();

  std::unique_ptr<Timestamped<data::LeftMotorEncoderState>> encoder_left_published_state;
  std::unique_ptr<Timestamped<data::RightMotorEncoderState>> encoder_right_published_state;

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_obs_vel;

  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_cmd_vel;
  /// Subscription for raw data coming from the rover
  rclcpp::Subscription<openrover_core_msgs::msg::RawData>::SharedPtr sub_raw_data;
  /// Publisher for efforts going to the rover
  rclcpp::Publisher<openrover_core_msgs::msg::RawMotorCommand>::SharedPtr pub_motor_efforts;
  rclcpp::Publisher<openrover_core_msgs::msg::RawCommand>::SharedPtr pub_rover_command;

  template <typename T>
  std::unique_ptr<Timestamped<T>> get_recent()
  {
    try
    {
      auto raw = *this->most_recent_data.at(T::which());
      return std::make_unique<Timestamped<T>>(raw.nanoseconds, T(raw.state));
    }
    catch (std::out_of_range&)
    {
      return nullptr;
    }
  }
};
}  // namespace openrover