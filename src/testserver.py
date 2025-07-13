import socket
import time
import threading

def send_market_data(client_socket):
    try:
        while True:
            # Use actual SOH character (\x01) instead of literal ^A
            message = "8=FIX.4.4\x0135=W\x0152=20240101-12:30:45\x0155=AAPL\x01268=2\x01269=0\x01270=150.25\x01271=100\x01269=1\x01270=150.30\x01271=200\x0110=123\x01"
            client_socket.send(message.encode())
            time.sleep(1)
    except Exception as e:
        print(f"Client disconnected: {e}")
    finally:
        client_socket.close()

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(('127.0.0.1', 9000))
server.listen(1)

print("Server listening on 127.0.0.1:9000")

try:
    while True:
        client, addr = server.accept()
        print(f"Client connected from {addr}")
        threading.Thread(target=send_market_data, args=(client,)).start()
except KeyboardInterrupt:
    print("Server shutting down...")
finally:
    server.close()