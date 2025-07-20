# Debug Test Server - Sends market data AND receives orders
import socket
import time
import threading
import random

# Configuration
MARKET_DATA_PORT = 9000  # Port to send market data to C++ app
ORDER_RECEIVE_PORT = 9001  # Port to receive orders from C++ app
client_ip = '127.0.0.1'

def parse_fix_message(message):
    """Parse FIX message and extract key fields"""
    try:
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
    
    tag_names = {
        '8': 'BeginString', '9': 'BodyLength', '35': 'MsgType',
        '49': 'SenderCompID', '56': 'TargetCompID', '52': 'SendingTime',
        '11': 'ClOrdID', '55': 'Symbol', '54': 'Side', '38': 'OrderQty',
        '40': 'OrdType', '44': 'Price', '59': 'TimeInForce', '10': 'CheckSum'
    }
    
    side_map = {'1': 'BUY', '2': 'SELL'}
    
    for tag, value in fields.items():
        name = tag_names.get(tag, f'Tag{tag}')
        if tag == '54':  # Side
            value = side_map.get(value, value)
        order_info.append(f"{name}({tag}): {value}")
    
    return order_info

def send_market_data():
    """Send market data that should trigger orders"""
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        print(f"Market data server started on {client_ip}:{MARKET_DATA_PORT}")
        
        # Start with base prices
        base_bid = 1.1230
        base_ask = 1.1240
        
        while True:
            # Realistic price movements
            bid_movement = random.uniform(-0.0005, 0.0005)
            ask_movement = random.uniform(-0.0005, 0.0005)
            
            current_bid = base_bid + bid_movement
            current_ask = base_ask + ask_movement
            
            # Ensure spread is maintained
            if current_ask <= current_bid:
                current_ask = current_bid + 0.0001
            
            bid_qty = random.randint(80000, 120000)
            ask_qty = random.randint(80000, 120000)
            
            message = f"8=FIX.4.2\x019=123\x0135=W\x0155=EUR/USD\x01268=2\x01269=0\x01270={current_bid:.5f}\x01271={bid_qty}\x01269=1\x01270={current_ask:.5f}\x01271={ask_qty}\x0110=168\x01"
            
            udp_socket.sendto(message.encode(), (client_ip, MARKET_DATA_PORT))
            
            # Occasionally trigger strategy with low ask
            if random.random() < 0.3:  # 30% chance
                trigger_ask = 1.1220  # Below target of 1.1235
                trigger_message = f"8=FIX.4.2\x019=123\x0135=W\x0155=EUR/USD\x01268=2\x01269=0\x01270={current_bid:.5f}\x01271={bid_qty}\x01269=1\x01270={trigger_ask:.5f}\x01271={ask_qty}\x0110=168\x01"
                udp_socket.sendto(trigger_message.encode(), (client_ip, MARKET_DATA_PORT))
            
            time.sleep(0.1)  # 10 updates per second
            
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(f"Market data error: {e}")
    finally:
        udp_socket.close()

def receive_orders():
    """Listen for orders from C++ application"""
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        udp_socket.settimeout(1.0)  # 1 second timeout for non-blocking receive
        
        udp_socket.bind((client_ip, ORDER_RECEIVE_PORT))
        print(f"Order receiver listening on {client_ip}:{ORDER_RECEIVE_PORT}")
        
        order_count = 0
        
        while True:
            try:
                data, addr = udp_socket.recvfrom(1024)
                order_count += 1
                message = data.decode('utf-8', errors='ignore')
                
                # Parse FIX message
                fields = parse_fix_message(message)
                
                if fields:
                    symbol = fields.get('55', 'Unknown')
                    side = 'BUY' if fields.get('54') == '1' else 'SELL' if fields.get('54') == '2' else fields.get('54', 'Unknown')
                    quantity = fields.get('38', 'Unknown')
                    price = fields.get('44', 'Unknown')
                    order_id = fields.get('11', 'Unknown')
                    
                    print(f"ORDER #{order_count}: {side} {quantity} {symbol} @ {price} (ID: {order_id})")
                
            except socket.timeout:
                continue
                
    except KeyboardInterrupt:
        print(f"\nReceived {order_count} orders total")
    except Exception as e:
        print(f"Order receiver error: {e}")
    finally:
        udp_socket.close()

def main():
    print("=== TRADING GATEWAY TEST SERVER ===")
    
    # Start market data sender in background thread
    market_thread = threading.Thread(target=send_market_data, daemon=True)
    market_thread.start()
    
    # Give it a moment to start
    time.sleep(1)
    
    # Run order receiver in main thread
    receive_orders()

if __name__ == "__main__":
    main()