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
        message = "TEST"
        udp_socket.sendto(message.encode(), (client_ip, client_port))
        print(f"Sent: {message}")
        time.sleep(1)
except KeyboardInterrupt:
    print("Stopped")
finally:
    udp_socket.close()