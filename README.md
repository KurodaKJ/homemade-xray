# homemade-xray

## How to run the project

To run the PC Admin, you will need to run:
```bash
make run
```

To run the test, you will need to run:
```bash
make test
```

To upload the code to Arduino, you will need to run:
```bash
pio run --target upload --upload-port /dev/ttyUSB[PORT_NUMBER]
```

To monitor the serial (e.g. for debugging), you will need to run:
```bash
pio device monitor --port [PORT] --baud [BAUD_RATE]
```
