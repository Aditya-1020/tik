# Test UDP server
import socket
import time

# CHUNK_SIZE = 1024
# filepath = ''
client_ip = 'localhost'
client_port = 9000

udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"Sending UDP data to {client_ip}:{client_port}")

"""
# File sending
with open(filepath, 'rb') as f:
    while chunk := f.read(CHUNK_SIZE):
        udp_socket.sendto(chunk, (client_ip, client_port))
        time.sleep(0.01)

udp_socket.sendto(b'EOF', (client_ip, client_port))
udp_socket.close()
print("File sent")
"""

try:
    while True:
        message = "8=FIX.4.2\x019=123\x0135=W\x0155=EUR/USD\x01268=2\x01269=0\x01270=1.1234\x01271=100000\x01269=1\x01270=1.1236\x01271=120000\x0110=168\x01"
        udp_socket.sendto(message.encode(), (client_ip, client_port))
        print(f"Sent: {message}")
        time.sleep(1)
except KeyboardInterrupt:
    print("Stopped")
finally:
    udp_socket.close()