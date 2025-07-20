# Test UDP server - Updated to receive orders on port 9001
import socket
import time

# Configuration
MARKET_DATA_PORT = 9000  # Port to send market data to C++ app
ORDER_RECEIVE_PORT = 9001  # Port to receive orders from C++ app
client_ip = 'localhost'

def parse_fix_message(message):
    """Parse FIX message and extract key fields"""
    try:
        # Split by SOH character (\x01)
        fields = message.split('\x01')
        parsed_fields = {}
        
        for field in fields:
            if '=' in field:
                tag, value = field.split('=', 1)
                parsed_fields[tag] = value
        
        return parsed_fields
    except Exception as e:
        print(f"Error parsing FIX message: {e}")
        return {}

def format_order_info(fields):
    """Format order information for display"""
    order_info = []
    
    # Common FIX tags
    tag_names = {
        '8': 'BeginString',
        '9': 'BodyLength', 
        '35': 'MsgType',
        '49': 'SenderCompID',
        '56': 'TargetCompID',
        '52': 'SendingTime',
        '11': 'ClOrdID',
        '55': 'Symbol',
        '54': 'Side',
        '38': 'OrderQty',
        '40': 'OrdType',
        '44': 'Price',
        '59': 'TimeInForce',
        '10': 'CheckSum'
    }
    
    side_map = {'1': 'BUY', '2': 'SELL'}
    ordtype_map = {'1': 'MARKET', '2': 'LIMIT'}
    tif_map = {'0': 'DAY', '1': 'GTC', '3': 'IOC', '4': 'FOK'}
    
    for tag, value in fields.items():
        name = tag_names.get(tag, f'Tag{tag}')
        
        # Format specific fields
        if tag == '54':  # Side
            value = side_map.get(value, value)
        elif tag == '40':  # OrdType
            value = ordtype_map.get(value, value)
        elif tag == '59':  # TimeInForce
            value = tif_map.get(value, value)
            
        order_info.append(f"{name}({tag}): {value}")
    
    return order_info

def send_market_data():
    """Send market data to C++ application"""
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        while True:
            # Send market data message
            message = "8=FIX.4.2\x019=123\x0135=W\x0155=EUR/USD\x01268=2\x01269=0\x01270=1.1234\x01271=100000\x01269=1\x01270=1.1236\x01271=120000\x0110=168\x01"
            udp_socket.sendto(message.encode(), (client_ip, MARKET_DATA_PORT))
            print(f"[MARKET DATA SENT]: {message}")
            time.sleep(2)  # Send every 2 seconds
            
    except KeyboardInterrupt:
        print("\nStopping market data sender...")
    finally:
        udp_socket.close()

def receive_orders():
    """Listen for orders from C++ application on port 9001"""
    # Create UDP socket
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        # Enable socket reuse
        udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        # Bind to the order receive port
        udp_socket.bind(('127.0.0.1', ORDER_RECEIVE_PORT))
        print(f"Listening for orders on 127.0.0.1:{ORDER_RECEIVE_PORT}")
        print("Socket successfully bound!")
        print("Waiting for orders from C++ application...")
        print("-" * 60)
        
        while True:
            # Receive data
            data, addr = udp_socket.recvfrom(1024)
            message = data.decode('utf-8', errors='ignore')
            
            print(f"\n[ORDER RECEIVED from {addr}]")
            print(f"Raw message: {repr(message)}")
            
            # Parse the FIX message
            fields = parse_fix_message(message)
            
            if fields:
                print("\nParsed FIX Fields:")
                order_info = format_order_info(fields)
                for info in order_info:
                    print(f"  {info}")
                    
                # Extract key order details
                symbol = fields.get('55', 'Unknown')
                side = fields.get('54', 'Unknown')
                side_name = 'BUY' if side == '1' else 'SELL' if side == '2' else side
                quantity = fields.get('38', 'Unknown')
                price = fields.get('44', 'Unknown')
                order_id = fields.get('11', 'Unknown')
                
                print(f"\n*** ORDER SUMMARY ***")
                print(f"Order ID: {order_id}")
                print(f"Action: {side_name} {quantity} {symbol} @ {price}")
                
            print("-" * 60)
            
    except KeyboardInterrupt:
        print("\nStopping order receiver...")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        udp_socket.close()

def main():
    """Main function - choose mode"""
    import sys
    
    print("Test Server Options:")
    print("1. Send market data (original functionality)")
    print("2. Receive orders from C++ app") 
    print("3. Run both (requires threading)")
    
    choice = input("Enter choice (1/2/3): ").strip()
    
    if choice == '1':
        send_market_data()
    elif choice == '2':
        receive_orders()
    elif choice == '3':
        import threading
        
        # Start market data sender in separate thread
        market_thread = threading.Thread(target=send_market_data, daemon=True)
        market_thread.start()
        
        # Run order receiver in main thread
        receive_orders()
    else:
        print("Invalid choice. Running order receiver by default...")
        receive_orders()

if __name__ == "__main__":
    main()