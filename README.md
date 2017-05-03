# drmfree-printer-firmware
Firmware for Arduino that controls the drmfree-printer motors

## Usage
### Additional software
This firmware can be controlled by [drmfree-printer-software](https://github.com/97nomad/drmfree-printer-software).

### Table of commands
```
  X..   - move along X coordinate
  Y..   - move along Y coordinate
  S     - stop all engines
  I     - information about position
  C     - set current position as zero coordinate
  G     - some ordinary g-codes
  N..   - set minimum pwm level to X engine
  M..   - set maximum pwm level to X engine
  J..   - set minimum pwm level to Y engine
  K..   - set minimum pwm level to Y engine
  D     - enable debug mode
```

## Some video demonstration
[![link to Youtube](https://img.youtube.com/vi/kIhLO65ZZnQ/0.jpg)](https://www.youtube.com/watch?v=kIhLO65ZZnQ)
