#include <chrono>
#include <memory>
#include <random>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "std_msgs/msg/float64.hpp"

using namespace std::chrono_literals;
using CallbackReturn =
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class LifecycleSensor : public rclcpp_lifecycle::LifecycleNode
{
public:
  LifecycleSensor()
  : rclcpp_lifecycle::LifecycleNode("lifecycle_sensor"),
    generator_(std::random_device{}()),
    distribution_(0.0, 100.0)
  {
  }

protected:
  CallbackReturn on_configure(const rclcpp_lifecycle::State &)
  {
    publisher_ = this->create_publisher<std_msgs::msg::Float64>(
      "/sensor_data", 10);

    RCLCPP_INFO(get_logger(), "Sensor configured");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State &)
  {
    publisher_->on_activate();

    timer_ = this->create_wall_timer(
      500ms,
      std::bind(&LifecycleSensor::publish_data, this));

    RCLCPP_INFO(get_logger(), "Sensor activated");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &)
  {
    publisher_->on_deactivate();
    timer_.reset();

    RCLCPP_INFO(get_logger(), "Sensor deactivated");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &)
  {
    publisher_.reset();
    timer_.reset();

    RCLCPP_INFO(get_logger(), "Sensor cleaned up");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &)
  {
    RCLCPP_INFO(get_logger(), "Sensor shutting down");
    return CallbackReturn::SUCCESS;
  }

private:
  void publish_data()
  {
    if (!publisher_->is_activated()) {
      return;
    }

    auto msg = std_msgs::msg::Float64();
    msg.data = distribution_(generator_);

    RCLCPP_INFO(get_logger(),
                "Publishing sensor data: %.2f",
                msg.data);

    publisher_->publish(msg);
  }

  rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::Float64>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;

  std::mt19937 generator_;
  std::uniform_real_distribution<double> distribution_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<LifecycleSensor>();
  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}
