# ESP32 Serial Protocol

## Transport

- Transport: USB serial
- Baud rate: 115200
- Encoding: UTF-8
- Framing: one JSON object per line
- Sample interval: approximately 1000 ms

## Startup message

{"type":"status","message":"ESP32_READY"}

## Sensor message

Each sensor message contains:

- `type`: `sensor_data`
- `schema_version`: protocol version, initially 1
- `seq`: monotonically increasing sample sequence
- `uptime_ms`: ESP32 uptime in milliseconds
- `temperature_c`: temperature in degrees Celsius
- `humidity_pct`: relative humidity percentage
- `pressure_hpa`: atmospheric pressure in hPa
- `gas_raw`: raw MQ-2 ADC reading
- `light_raw`: raw LDR ADC reading
- `distance_cm`: ultrasonic distance in centimetres
- `status`: validity flags for each sensor

A reading that is unavailable or invalid must be represented as JSON `null`.
The corresponding sensor validity flag must be `false`.

## Simulation mode

The firmware initially uses `SIMULATION_MODE 1`.
These values are generated for software testing and do not represent
actual environmental measurements.

Before physical deployment, change `SIMULATION_MODE` to `0` and
install the Adafruit BME280 and Adafruit Unified Sensor libraries.

## Important limitations

- MQ-2 output is a raw sensor value, not a calibrated gas concentration.
- The LDR output is a raw ADC value; its interpretation depends on wiring.
- HC-SR04 measurements outside the supported range are marked invalid.
- BME280 measurements are marked invalid if sensor initialization fails.
- Simulation mode must not be used for real safety monitoring.
