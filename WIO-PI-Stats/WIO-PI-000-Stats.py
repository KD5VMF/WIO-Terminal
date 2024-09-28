import smbus2
import time
import psutil
import socket
import os

# I2C setup
I2C_BUS = 1  # The I2C bus on the Raspberry Pi (typically 1 for models after Rev 2)
I2C_ADDRESS = 0x08  # Address of the Wio Terminal

bus = smbus2.SMBus(I2C_BUS)

# Function to send data to Wio Terminal
def send_data(metric_name, value):
    data_str = f"{metric_name}:{value}\n"
    data_bytes = data_str.encode('utf-8')
    # Split data into manageable chunks (max 32 bytes for I2C)
    chunks = [data_bytes[i:i + 30] for i in range(0, len(data_bytes), 30)]
    for chunk in chunks:
        try:
            print(f"Sending data: {chunk.decode('utf-8')}")  # Debug: Print the chunk being sent
            bus.write_i2c_block_data(I2C_ADDRESS, 0, list(chunk))
            time.sleep(0.2)  # Short delay to ensure data is received properly
        except OSError as e:
            print(f"Error sending data to Wio Terminal: {e}")

# Function to get system metrics
def get_metrics():
    # Get RAM metrics
    ram = psutil.virtual_memory()
    ram_total = round(ram.total / (1024 ** 2), 2)  # Convert bytes to MB
    ram_free = round(ram.available / (1024 ** 2), 2)  # Convert bytes to MB

    # Send RAM metrics
    send_data("RAM_TOTAL", ram_total)
    send_data("RAM_FREE", ram_free)

    # Get CPU usage
    cpu_usage = psutil.cpu_percent(interval=1, percpu=False)  # Total CPU usage in %
    send_data("CPU_USAGE", cpu_usage)

    # Get CPU temperature (Linux specific)
    try:
        with open("/sys/class/thermal/thermal_zone0/temp", "r") as f:
            cpu_temp = round(int(f.read()) / 1000, 1)  # Convert to degrees Celsius
    except FileNotFoundError:
        cpu_temp = "N/A"  # CPU temperature might not be available on all devices

    send_data("CPU_TEMP", cpu_temp)

    # Get storage metrics
    disk = psutil.disk_usage('/')
    storage_total = round(disk.total / (1024 ** 3), 2)  # Convert bytes to GB
    storage_free = round(disk.free / (1024 ** 3), 2)  # Convert bytes to GB

    send_data("STORAGE_TOTAL", storage_total)
    send_data("STORAGE_FREE", storage_free)

    # Get network status and IP address
    net_interfaces = psutil.net_if_addrs()
    ip_address = "Disconnected"
    if 'wlan0' in net_interfaces or 'eth0' in net_interfaces:
        for interface in ['wlan0', 'eth0']:
            if interface in net_interfaces:
                for addr in net_interfaces[interface]:
                    if addr.family == socket.AF_INET:
                        ip_address = addr.address

    send_data("IP_ADDR", ip_address)

    # Get user and computer name
    user_name = os.getlogin()
    computer_name = socket.gethostname()

    send_data("USER_NAME", user_name)
    send_data("COMPUTER_NAME", computer_name)

    # Get network status
    network_status = "Connected" if ip_address != "Disconnected" else "Disconnected"
    send_data("NETWORK", network_status)

if __name__ == "__main__":
    try:
        while True:
            get_metrics()
            time.sleep(5)  # Update every 5 seconds
    except KeyboardInterrupt:
        print("Stopping metric collection.")
    finally:
        bus.close()
