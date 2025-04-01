# **ReadMe**

## **Part 1: Smart Thermostat System**

### **Overview**
This embedded application implements a smart thermostat prototype using a TI CC3220S LaunchPad. It integrates multiple hardware peripherals (I2C, UART, GPIO, Timer) and is designed to monitor and regulate temperature based on a user-defined setpoint. The system demonstrates multitasking and interrupt-driven behavior through a task scheduler running on a timer-based architecture.

### **Core Components**
- **Temperature Monitoring**: The TMP006 sensor communicates via I2C to read ambient temperature values.
- **User Interaction**: Two GPIO buttons are configured with falling-edge interrupts to adjust the setpoint (Button 0 increases, Button 1 decreases).
- **Heater Control**: An LED simulates the heater status—ON if temperature is below the setpoint.
- **UART Output**: Periodically transmits `<temperature, setpoint, heater_status, uptime>` via UART to simulate cloud communication.
- **Task Scheduling**:
  - Button check: every 200 ms
  - Temperature read: every 500 ms
  - Data transmission: every 1000 ms

### **Execution Flow**
The timer fires every **100 ms**, incrementing a global counter. Various tasks execute depending on the counter's modulus:
```Courier New
if (timerCounter % 2 == 0) { check buttons }
if (timerCounter % 5 == 0) { read temperature and control heater }
if (timerCounter % 10 == 0) { transmit data over UART }
```

### **Design Considerations**
- Efficient multitasking through a software scheduler.
- Immediate responsiveness via GPIO interrupts.
- Compatibility with multiple TMP sensor variants.
- UART-based simulation of IoT/cloud communication.

---

## **Part 2: Morse Code “SOS” / “OK” Blinker**

### **Overview**
This is a synchronous state machine implementation to blink LEDs in Morse code patterns for "SOS" (`...---...`) and "OK" (`---.-`). It utilizes timer-driven execution and GPIO interrupts for real-time state transitions.

### **System Behavior**
- **Red LED**: Indicates a DOT (500 ms ON).
- **Green LED**: Indicates a DASH (1500 ms ON).
- **OFF State**: Managed by inter-character (500 ms) and inter-word (3500 ms) pauses.
- **Message Toggle**: Button press toggles between “SOS” and “OK” sequences.

### **State Machine Structure**
```Courier New
States:
  DOT         → LED Red ON (500ms)
  DASH        → LED Green ON (1500ms)
  INTER_CHAR  → LEDs OFF (500ms)
  INTER_WORD  → LEDs OFF (3500ms)
```

Transitions are driven by a **500ms** periodic timer callback (`Timer_CONTINUOUS_CALLBACK`). The current message is processed character-by-character using a `currentIndex` pointer. Upon button press, a flag toggles the current message during the INTER_WORD state.

### **TimerCallback Logic Summary**
```Courier New
switch (state):
  DOT        → Red ON → wait → OFF → INTER_CHAR
  DASH       → Green ON → wait → OFF → INTER_CHAR
  INTER_CHAR → wait → next DOT or DASH or INTER_WORD
  INTER_WORD → wait → restart or switch message
```

### **GPIO Interaction**
The button input is configured using `GPIO_CFG_IN_INT_FALLING`, which triggers `gpioButtonFxn0()` to set a toggle flag. The flag is checked in the `INTER_WORD` state to avoid interrupting a message mid-sequence.

---

## **Final Remarks**
Both programs showcase foundational concepts in embedded systems:
- Real-time event handling via timers and interrupts.
- State machine implementation and timing precision.
- Peripheral integration (I2C, GPIO, UART).
- Responsiveness and scheduling in resource-constrained environments.

These modules form the basis for more sophisticated IoT-capable devices and provide a solid framework for learning low-level systems design on embedded platforms like TI's CC3220S.
