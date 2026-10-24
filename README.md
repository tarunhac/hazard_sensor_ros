# Hazard Sensor ROS

ESP32 environmental hazard sensing integrated with ROS 2 Humble for an
industrial inspection robot.

## Architecture

ESP32 sensors -> USB serial -> Raspberry Pi 4B running ROS 2 Humble
-> ROS 2 sensor topics -> threshold monitoring and alerts.

## Project Structure

- `firmware/esp32_sensor_node/`: ESP32 firmware
- `ros2_ws/src/`: ROS 2 packages
- `config/`: sensor and alert configuration
- `docs/`: wiring, setup, and testing documentation

## Development

Develop and test on Ubuntu 22.04 with ROS 2 Humble.
Deploy the verified code to the Raspberry Pi 4B.
