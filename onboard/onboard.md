# onboard.ino
Code for the onboard computer. The primary purpose of this code is to log sensor data over the serial, and to receive pin state commands from the serial.

## Output
This code outputs log messages to the serial as new data becomes available. The format is as follows:
```
LOG<timestamp>,[valve1,valve2,...valve9],[]
```

  --------------
  Time: ######
  Sensor 0 Temperature/Pressure (Nitrous Inlet)
  Sensor 1 Temperature/Pressure (IPA Inlet)
  Sensor 2 Temperature/Pressure (Nitrous Tank)
  Sensor 3 Temperature/Pressure (IPA Tank)
  Sensor 4 Temperature/Pressure (Nitrogen Inlet)

Command Mode Format:

“PDW A X Y Z….”
PDW = PinDigitalWrite to set a relay
A = 1 or 0
X, Y, Z, …. = relays you want to set

1 = N2O Pressurant Line (normally closed)
2 = IPA Pressurant Line (normally closed)
3 = N2O Run Valve (normally open)
4 = IPA Run Valve (normally open)
5 = N2O Vent Valve (normally open)
6 = IPA Vent Valve (normally open)
7 = Pneumatics Line Valve (normally closed) -- ACTUALLY NORMALLY OPEN
8 = Pneumatics Line Vent Valve (normally open)
9 = Purge Valve (normally closed)

Useful Commands:
- Setup (closing normally open valves) = PDW 1 3 4 5 6 8
- Pressurizing Tanks = PDW 1 1 2
- Stop Pressurizing Tanks = PDW 0 1 2
- Activating Pressurant Line = PDW 1 7
- Closing/venting Pressurant Line = PDW 0 7 8
- Opening Run lines = PDW 0 3 4
- Closing Run Lines = PDW 1 3 4
- Opening all vents = PDW 0 5 6 8
- Final Depress = PDW 1 1 2 7 —> PDW 0 5 6 8
- Reverting back to off state = PDW 0 1 2 3 4 5 6 7 8 9

Close all the vents and run valves = PDW 1 3 4 5 6 8