
# Raspberry Pi System Metrics Display on Wio Terminal

## Project Overview

This project aims to display real-time system metrics from a Raspberry Pi on a Wio Terminal. The metrics include RAM usage, CPU temperature, network information, and more. Communication between the Pi and the Wio Terminal occurs over the I2C protocol, with the Wio Terminal acting as a slave device that displays the data on its screen.

## Requirements

- Raspberry Pi (with I2C enabled)
- Wio Terminal (running Arduino code to act as an I2C slave)
- Python 3 installed on the Raspberry Pi
- Arduino IDE for uploading code to the Wio Terminal

## Setting Up the Raspberry Pi

### Install Dependencies

Ensure Python 3 is installed on your Raspberry Pi. You can also install the necessary Python libraries using the following commands:

```sh
sudo apt update
sudo apt install python3-pip
pip3 install smbus2 psutil
```

### Python Script

The Python script gathers system metrics and sends them to the Wio Terminal via I2C. Save the Python script as `send_metrics.py` in your home directory or in a folder like `~/myapps/`.

### Run the Python Script on Boot

To run the script automatically on boot, create a systemd service.

#### Create a Systemd Service File

1. Create a new file using:

    ```sh
    sudo nano /etc/systemd/system/send_metrics.service
    ```

2. Add the following content to the file:

    ```ini
    [Unit]
    Description=Send System Metrics to Wio Terminal
    After=multi-user.target

    [Service]
    Type=idle
    User=pi  # Replace 'pi' with your username if different
    ExecStart=/usr/bin/python3 /home/pi/myapps/send_metrics.py  # Adjust path if needed

    [Install]
    WantedBy=multi-user.target
    ```

#### Enable the Service

Reload the systemd configuration and enable the service:

```sh
sudo systemctl daemon-reload
sudo systemctl enable send_metrics.service
```

#### Start the Service

Start the service immediately:

```sh
sudo systemctl start send_metrics.service
```

Check the status to confirm it's running without issues:

```sh
sudo systemctl status send_metrics.service
```

## Setting Up the Wio Terminal

### Install Arduino IDE

Install the Arduino IDE on your computer if you haven't already. Make sure to add the necessary boards for Seeed Wio Terminal by including the Seeed boards URL in the Board Manager preferences.

### Upload the Arduino Code

1. Open the provided Arduino code in the IDE.
2. Select the Wio Terminal board from the Tools menu.
3. Connect the Wio Terminal to your computer and upload the code.
4. The code will make the Wio Terminal listen as an I2C slave and update the LCD with system metrics received from the Raspberry Pi.

## Connecting the Raspberry Pi and Wio Terminal

### Wiring for I2C

Connect the Raspberry Pi GPIO pins to the Wio Terminal’s I2C pins:

- **SCL (Serial Clock Line)**: Connect Raspberry Pi GPIO 3 (SCL) to Wio Terminal SCL.
- **SDA (Serial Data Line)**: Connect Raspberry Pi GPIO 2 (SDA) to Wio Terminal SDA.
- **Ground (GND)**: Ensure both devices share a common ground.

### I2C Configuration on Raspberry Pi

Enable I2C on the Raspberry Pi:

```sh
sudo raspi-config
```

- Go to **Interfacing Options** > **I2C** and enable it.

To verify that the Wio Terminal is connected properly, run:

```sh
sudo i2cdetect -y 1
```

You should see the I2C address (typically `0x08` for this project) listed on the bus.

## Debugging and Verifying

### Arduino Serial Monitor

Open the Serial Monitor from the Arduino IDE to observe the data received by the Wio Terminal. This will help you verify that the data is coming in correctly from the Raspberry Pi.

### Python Debug Output

The Python script prints each metric as it sends data. You can run the script manually to see the output and confirm what’s being transmitted:

```sh
python3 /home/pi/myapps/send_metrics.py
```

### I2C Communication Check

Use `i2cdetect` on the Raspberry Pi to ensure that the Wio Terminal is recognized and the correct I2C address is shown:

```sh
sudo i2cdetect -y 1
```

## Final Notes

- **Data Display**: The Wio Terminal dynamically updates the display based on the metrics received from the Raspberry Pi. Metrics such as RAM, CPU, and storage are continuously updated.
- **Performance**: To reduce flickering and maintain performance, only portions of the display are updated when their corresponding metrics change.

This guide allows you to replicate the setup and understand how to configure both the Raspberry Pi and Wio Terminal for real-time metric display. Feel free to modify the scripts and settings according to your requirements.

## Troubleshooting

- **No Data on Wio Terminal Display**: Ensure the I2C wiring is correct and that the I2C slave address (`0x08`) is detected on the Pi using `i2cdetect`.
- **Service Not Running**: If the Python script service does not start, use `sudo systemctl status send_metrics.service` to debug the issue.
- **Check Serial Output**: Use the Arduino Serial Monitor to verify that the Wio Terminal is receiving data correctly.
