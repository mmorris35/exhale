#!/usr/bin/env python3
"""
Generate wiring diagrams for CO2 Fan Controller using Schemdraw
"""

import schemdraw
import schemdraw.elements as elm


def create_i2c_diagram():
    """I2C connection detail"""
    d = schemdraw.Drawing()
    d.config(unit=3, fontsize=12)

    # Title
    d += elm.Label().at((0, 5)).label('I2C Connection - ESP32 to SCD-40', fontsize=14, halign='left')

    # VCC rail
    d += elm.Line().at((0, 4)).right().length(10).color('red')
    d += elm.Label().at((0, 4.3)).label('3.3V', fontsize=10, halign='left', color='red')

    # GND rail
    d += elm.Line().at((0, 0)).right().length(10).color('black')
    d += elm.Label().at((0, -0.3)).label('GND', fontsize=10, halign='left')

    # SDA line
    d += elm.Line().at((2, 2.5)).right().length(6).color('blue')
    d += elm.Label().at((5, 2.8)).label('SDA (GPIO13)', fontsize=10, color='blue')
    d += elm.Dot().at((2, 2.5)).color('blue')
    d += elm.Dot().at((8, 2.5)).color('blue')

    # SCL line
    d += elm.Line().at((2, 1.5)).right().length(6).color('orange')
    d += elm.Label().at((5, 1.2)).label('SCL (GPIO16)', fontsize=10, color='orange')
    d += elm.Dot().at((2, 1.5)).color('orange')
    d += elm.Dot().at((8, 1.5)).color('orange')

    # ESP32 side connections (vertical lines)
    d += elm.Line().at((2, 4)).down().length(1.5).color('red')
    d += elm.Dot().at((2, 4)).color('red')
    d += elm.Line().at((2, 0)).up().length(1.5).color('black')
    d += elm.Dot().at((2, 0)).color('black')

    # SCD-40 side connections
    d += elm.Line().at((8, 4)).down().length(1.5).color('red')
    d += elm.Dot().at((8, 4)).color('red')
    d += elm.Line().at((8, 0)).up().length(1.5).color('black')
    d += elm.Dot().at((8, 0)).color('black')

    # Labels for boards
    d += elm.Label().at((2, -0.8)).label('ESP32-POE-ISO', fontsize=11)
    d += elm.Label().at((8, -0.8)).label('SCD-40', fontsize=11)

    d.save('docs/images/i2c-wiring.svg')
    return d


def create_relay_diagram():
    """Relay wiring diagram"""
    d = schemdraw.Drawing()
    d.config(unit=3, fontsize=12)

    # Title
    d += elm.Label().at((0, 6)).label('Relay Wiring - GPIO4 to 110V Fan', fontsize=14, halign='left')

    # ESP32 GPIO4 output
    d += elm.Dot().at((1, 4)).label('GPIO4', loc='left', fontsize=10)
    d += elm.Line().at((1, 4)).right().length(2)

    # Relay coil (represented as inductor/coil)
    d += elm.Inductor2(loops=3).at((3, 4)).right().label('Relay\nCoil', loc='bottom', fontsize=10)

    # To ground through transistor (simplified)
    d += elm.Line().at((5, 4)).right().length(1)
    d += elm.Line().at((6, 4)).down().length(2)
    d += elm.Ground().at((6, 2))

    # 3.3V to relay VCC
    d += elm.Dot().at((1, 5)).label('3.3V', loc='left', fontsize=10, color='red')
    d += elm.Line().at((1, 5)).right().length(3).color('red')
    d += elm.Line().at((4, 5)).down().length(1).color('red')

    # AC Side - separated
    d += elm.Label().at((0, 0)).label('110V AC Side', fontsize=12, halign='left')

    # Hot from breaker
    d += elm.Dot().at((1, -1)).label('HOT\n(Black)', loc='left', fontsize=9)
    d += elm.Line().at((1, -1)).right().length(2)

    # Relay contact (switch)
    d += elm.Switch().at((3, -1)).right().label('NO', loc='top', fontsize=9)

    # To fan
    d += elm.Line().at((5, -1)).right().length(2)
    d += elm.Motor().at((7, -1)).right().label('Fan', loc='right', fontsize=10)

    # Neutral (direct)
    d += elm.Dot().at((1, -2.5)).label('NEUTRAL\n(White)', loc='left', fontsize=9)
    d += elm.Line().at((1, -2.5)).right().length(7.5)
    d += elm.Line().at((8.5, -2.5)).up().length(1)

    d.save('docs/images/relay-wiring.svg')
    return d


def create_full_system():
    """Full system block diagram"""
    d = schemdraw.Drawing()
    d.config(unit=2.5, fontsize=11)

    # Title
    d += elm.Label().at((0, 8)).label('CO2 Fan Controller - System Diagram', fontsize=14, halign='left')

    # PoE Input
    d += elm.Dot().at((0, 5)).label('PoE\n(Cat6)', loc='left', fontsize=10)
    d += elm.Line().at((0, 5)).right().length(2)

    # ESP32 Box - draw as 4 lines forming rectangle
    # Box from (2,3.5) to (6,6.5)
    d += elm.Line().at((2, 3.5)).right().length(4)  # bottom
    d += elm.Line().at((6, 3.5)).up().length(3)      # right
    d += elm.Line().at((6, 6.5)).left().length(4)   # top
    d += elm.Line().at((2, 6.5)).down().length(3)   # left
    d += elm.Label().at((4, 5.5)).label('ESP32-POE-ISO', fontsize=11)
    d += elm.Label().at((4, 4.5)).label('GPIO13: SDA\nGPIO16: SCL', fontsize=8)
    d += elm.Label().at((4, 6)).label('GPIO4: Relay', fontsize=8)

    # I2C to sensor
    d += elm.Line().at((2, 4.5)).left().length(1).color('blue')
    d += elm.Label().at((0.5, 4.5)).label('I2C', fontsize=9, color='blue')

    # SCD-40 Box
    d += elm.Line().at((-1, 3.5)).right().length(2)   # bottom
    d += elm.Line().at((1, 3.5)).up().length(2)       # right
    d += elm.Line().at((1, 5.5)).left().length(2)    # top
    d += elm.Line().at((-1, 5.5)).down().length(2)   # left
    d += elm.Label().at((0, 4.8)).label('SCD-40', fontsize=10)
    d += elm.Label().at((0, 4.2)).label('CO2 Sensor', fontsize=8)

    # GPIO4 to Relay
    d += elm.Line().at((6, 5.5)).right().length(2)

    # Relay Box
    d += elm.Line().at((8, 4.75)).right().length(2)   # bottom
    d += elm.Line().at((10, 4.75)).up().length(1.5)   # right
    d += elm.Line().at((10, 6.25)).left().length(2)  # top
    d += elm.Line().at((8, 6.25)).down().length(1.5) # left
    d += elm.Label().at((9, 5.5)).label('Relay\n10A', fontsize=10)

    # To Fan
    d += elm.Line().at((10, 5.5)).right().length(1.5).color('orange')
    d += elm.Motor().at((12, 5.5)).label('110V\nFan', loc='right', fontsize=10)

    # Legend
    d += elm.Label().at((0, 1.5)).label('Legend:', fontsize=10, halign='left')
    d += elm.Line().at((0, 1)).right().length(1).color('blue')
    d += elm.Label().at((1.2, 1)).label('I2C Bus', fontsize=9, halign='left', color='blue')
    d += elm.Line().at((4, 1)).right().length(1).color('orange')
    d += elm.Label().at((5.2, 1)).label('110V AC', fontsize=9, halign='left', color='orange')

    d.save('docs/images/system-diagram.svg')
    return d


if __name__ == '__main__':
    import os
    os.makedirs('docs/images', exist_ok=True)

    print("Generating wiring diagrams...")

    try:
        create_i2c_diagram()
        print("  Created: docs/images/i2c-wiring.svg")
    except Exception as e:
        print(f"  Error creating I2C diagram: {e}")

    try:
        create_relay_diagram()
        print("  Created: docs/images/relay-wiring.svg")
    except Exception as e:
        print(f"  Error creating relay diagram: {e}")

    try:
        create_full_system()
        print("  Created: docs/images/system-diagram.svg")
    except Exception as e:
        print(f"  Error creating system diagram: {e}")

    print("\nDone! SVG files are in docs/images/")
    print("GitHub renders SVG natively in README files.")
