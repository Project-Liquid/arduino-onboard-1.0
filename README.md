# arduino-onboard
Code for the onboard arduino. Allows the arduino to talk to the computer by serial using a set of utf-8 string messages. You can read these messages on the computer using the arduino ide's serial monitor tool, or programmatically, using eg. the python `serial` library.

We also have code for broadcasting these serial messages over a websockets connection (see [websockets-serial](https://github.com/Project-Liquid/websockets-serial)), allowing for a browser-based user interface and (conceivably, in the far future) authenticated network access to the command and telemetry interfaces.
