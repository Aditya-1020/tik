# Tik: Low Latency Trading Gateway
Simple High performance, low latency system implemented in C++ written for learning purposes. Processes Data in realtime via FIX protocol.

## Features
- Lock Free queues
- Memory Pools to eliminate allocations
- CPU pinning with dedicated cores for market and order processing
- Zero copy using in place parsing and string views
- State machine for FIX parsing
- Compile time optimizations

### Performance results
- Average tik-to-trade: 10-15 microseconds
- Peak Performance: 2.8μs minimum latency
- Message Processing: 400+ messages per 30-second test with ~98% execution rate
- Parser efficiency: Sub-2μs (averaging around 1.4μs)

### Build

**Tree**
```sh
├── config.json
├── include/
│   ├── fast_float/  (9 headers)
│   ├── json.hpp
│   ├── ordermanager.h
│   ├── orders.h
│   ├── parser.h
│   └── receive.h
├── Makefile
├── README.md
└── src/
    ├── (main, ordermanager, parser, orders, receive).cpp
    └── testserver.py
```
**Prerequisites**

```sh
# Ubuntu/Debian
sudo apt-get install build-essential nlohmann-json3-dev libboost-dev
```

**Compilation**
```sh
make clean && make
```
### Configuration
Edit `config.json` for trading parameters (for now its only one config variable)
```json
{
    "symbol": "EUR/USD",
    "target_buy_price": 1.1235,
    "order_quantity": 1500
}
```
Compile and run
```sh
./TIK
```

Test with Market data
```sh
python3 testserver.py
```

#### Data Flow
```md
Market Data (UDP) → Parser → Order Book → Strategy → Order Execution
```


### Planned goals
- Implement SBE protocol
- Multi-symbol support