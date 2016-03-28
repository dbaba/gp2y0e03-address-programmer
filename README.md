gp2y0e03-address-programmer
===

A tool for modifying [GP2Y0E03](http://www.sharp-world.com/products/device/lineup/data/pdf/datasheet/gp2y0e03_e.pdf) I2C address with E-Fuse running on mbed device.

# Schematic

    STM32F401RE Nucleo        GP2Y0E03
    +----------------+        +------+
    |           GND ------------ GND |
    |                |        |      |
    |      SDA(D14) -------+---- SDA |
    |                |     |  |      |
    |                |  <R4K7>|      |
    |                |     |  |      |
    |           3V3 -------+---- VIN |
    |                |     |  |      |
    |                |     +---- VDD |
    |                |     |  |      |
    |                |     +---- GPIO|
    |                |     |  |      |
    |                |  <R4K7>|      |
    |                |     |  |      |
    |      SCL(D15) -------+---- SCL |
    |                |        |      |
    |            D7 ------------ Vpp |
    |                |        |      |
    +----------------+        +------+

 * R4K7 ... 4.7K Ohm pull-up resistor
 * Vout(A) is never used for I2C

# How to setup (w/ ST Nucleo F401RE)

    yt target st-nucleo-f401re-gcc
    yt install

# How to flash

Copy `build/st-nucleo-f401re-gcc/source/gp2y0e03-address-programmer.bin` to the mbed folder or use st-flash as shown below.

    st-flash write build/st-nucleo-f401re-gcc/source/gp2y0e03-address-programmer.bin 0x8000000

# How to modify address

## Edit the address data

Open [app.cpp](source/app.cpp) and edit the following line to modify the address.

    #define NEW_ADDRESS_DATA 0x01

Note that `NEW_ADDRESS_DATA` is NOT a slave address. Choose the `Data` column value of the following table.

| Slave ID | 7-bit Address |   Data   |
|----------|---------------|----------|
| 0x00     | 0x00          | **0x00** |
| 0x10     | 0x08          | **0x01** |
| 0x20     | 0x10          | **0x02** |
| 0x30     | 0x18          | **0x03** |
| 0x40     | 0x20          | **0x04** |
| 0x50     | 0x28          | **0x05** |
| 0x60     | 0x30          | **0x06** |
| 0x70     | 0x38          | **0x07** |
| 0x80     | 0x40 DEFAULT! |   0x08   |
| 0x90     | 0x48          | **0x09** |
| 0xA0     | 0x50          | **0x0A** |
| 0xB0     | 0x58          | **0x0B** |
| 0xC0     | 0x60          | **0x0C** |
| 0xD0     | 0x68          | **0x0D** |
| 0xE0     | 0x70          | **0x0E** |
| 0xF0     | 0x78          | **0x0F** |

Please do NOT choose `0x08` as the corresponding Salve ID is the default value.

The Slave ID is an I2C address for data writing. See Table.21 List of Slave ID in the [application note](http://www.sharp-world.com/products/device/lineup/data/pdf/datasheet/gp2y0e02_03_appl_e.pdf).

## Create a binary and flash it

See the description above.

## Open Terminal and connect the board

Push 'Reset' button after connecting it in order to display the prompt message.

## Connect the sensor

See the schematic above.

## Push button to start

Now ready for performing E-Fuse programmer. Push the blue button to continue.

# Output Example

You can see the following output by connecting with a terminal app with baud rate 115200.

    ** Welcome to Address Programmer!
    ** I'll update the GP2Y0E03 I2C address from [0x80](Write) to [0x10](Write)
    ** You cannot revert the change once the address is updated
    ** Push Button1 to continue!
    +++++++++++++++++++++++++++++++++++
        L E T ' S   S T A R T ! ! !
    +++++++++++++++++++++++++++++++++++
    stage 1
    stage 2
    stage 3
    stage 4
    stage 5
    stage 6
    stage 7
    stage 8
    stage 9
    Check 0x27[4:0] => 0
    e_fuse_stage9():result => 0 (0=success)
    ===================================
               F I N I S H
    ===================================

# CAVEAT

The 'stage 10' functions like `e_fuse_stage10_1_1()` are never tested as I haven't failed to modify addresses yet.
