# OPN-2500 & OPN-6000 C-Development kit
This is the official SDK from Opticon Sensors Europe BV for the Companion Scanners OPN-2500 and OPN-6000 to create and customize applications and load them onto the OPN-2500 and OPN-6000

The github pages of this SDK can be found here: [opn2500_opn6000_dev_kit](https://opticonosedevelopment.github.io/opn2500_opn6000_dev_kit/)

The prerequisites for using this SDK are:
- Latest SDK files from this repository
- `SEGGER Embedded Studio for ARM` can download from here: [Segger IDE](https://www.segger.com/downloads/embedded-studio/)
- `Appload` firmware upload application for Windows [Appload](https://www.opticon.com/support/Appload/Appload%20Setup.exe)
- Opticon USB Drivers can download from here: [USB Drivers Installer](https://www.opticon.com/support/Drivers/USB%20Drivers%20Installer.exe)

The SEGGER Embedded studio can be licensed for free when using devices with a Nordic CPU like the OPN-6000 / OPN-2500
For more info see: [Get License for Nordic Semiconductor Devices](https://wiki.segger.com/Get_a_License_for_Nordic_Semiconductor_Devices)
It can be useful to attach a nRF52 DK board to you PC, see the license is automatically recognized, although not it's not necessary to build applications

# Getting started

- Download and extract this complete repository into a working folder
- Download and install the latest Opticon USB drivers
- Download and install the latest Appload version 
- Install the SEGGER Embedded Studio for ARM

# SDK Files

This SDK contains the following components:

## /firmware

- **/bootloader**
  - `FAMVxxxx.HEX` -> Bootloader of the OPN-6000
  - `FANVxxxx.HEX` -> Bootloader of the OPN-2500

- **/library**
  - `OPN2500_FBNVxxxx.a` -> OPN-2500 Operating System library file
  - `OPN6000_FBMVxxxx.a` -> OPN-6000 Operating System library file

## /include

- `lib.h` -> OS library include file
- `lib_legacy.h` -> Legacy OS library with legacy lower-case function names, file system, and defines (for porting existing applications)
- `Debug.h` -> Used for debugging
- `/FileSystem` -> Header files for the [ChaN's FatFs Generic file system](http://elm-chan.org/fsw/ff/FileSystem) 
- `(other OS related header files)`

## /source
- `version.c` -> Places the version of the application at a fixed location in memory, so it can be retrieved in Appload

## /mdk

- `OPN2500` / `OPN6000` / `nRF528xx` specific header files

## /nrfutil

- `nrfutil.exe` -> Nordic tool for signing and packing bootloader, softdevice, OS, and application

## /softdevice

- `/MBR` -> Master boot record (starts the bootloader)
- `/S113` -> Compiled softdevice S113 (Bluetooth BLE wireless protocol stack for nRF52840 Nordic CPU)
- `/Common` -> Softdevice header files

## /toolchain

- `/CMSIS` -> Provides software components for retargeting I/O operations in standard C run-time libraries

## /documentation

- `OPN-6000 User Manual.pdf` -> Latest version is always available at [OPN-6000 User Manual](https://www.opticon.com/support/OPN-6000/OPN-6000%20User%20Manual.pdf)
- `OPN-2500 User Manual.pdf` -> Latest version is always available at [OPN-2500 User Manual](https://www.opticon.com/support/OPN-2500/OPN-2500%20User%20Manual.pdf)


# Applications

In the folder `Projects/Applications`, 2 applications can be found that are installed on the OPN-2500 / OPN-6000 by default. They provide a good starting point when only small customizations are required:

## BatchApplication
    -> Batch application to demonstrate using the OPN as batch device and communication using USB-MSD and OseComm

## BluetoothAppl
    -> Bluetooth application to demonstrate Bluetooth BLE Serial, HID and OptiConnect as well as USB-HID, USB-CDC and USB-VCP and memorizing

## Snippets
    -> Source files used by both applications for storing, updating and exporting bar code data in the file system

- Open the projects file: `BatchApplication\Projects\ses\Batch_Appl.emProject` or `BluetoothAppl\Projects\ses\Bluetooth_Appl.emProject`

- After building, the resulting .bin file can be found in the root folder of the applications and loaded using Appload

- The User Manuals of the OPN-2500 and OPN-6000 in the SDK provide more information on how to use these applications

# Examples

In the folder `Projects/Examples`, you'll find small example applications for all basic peripheral functions of the OPN-2500 / OPN-6000 
    
- Open the projects file: `OPN_Examples.emProject` in Segger IDE

- In the `Project explorer`, right click the source files and use the `Exclude from build` option to select which sample application to build.

- After building, the resulting `.bin-file` can be found in the `/Output folder and loaded using Appload

# Loading of Firmware

- See `/documentation/How to load Firmware.pdf` for more information on how to load the resulting .bin files on to the OPN-2500 / OPN-6000

# Trouble Shooting

By default, the OPN will have the USB-VCP COM-port enabled, which makes it possible to load firmware using Appload over USB. 
In application where this COM-port is not enabled (i.e. USB-MSD) or when the application has crashed, use this method to enter firmware update mode:

- Press and hold both the trigger and function key for at least 15 seconds. 
- After the companion scanner LED turns white, release the function first to enter firmware update mode.
- If successful, the LED of your companion scanner should now be blinking red. 

You should now be able to load new firmware using Appload or OptiConnect. 
To exit firmware update mode, press both keys for 3 seconds (or wait 30 seconds).

# OptiConnect

OptiConnect is a free application developed by Opticon to configure and manage the OPN series of barcode scanners and your bar code data that is available on all platforms.

### Features
- Easy pairing and connection with Bluetooth
- Collect barcode data
- Easy configuration
- Easy export of barcode data
- Easy integration with custom apps (Intents; API)
- Wireless device firmware update (DFU)
- Barcode validation

To connect your OPN to the OptiConnect app:
1. Download and install the OptiConnect app from your app store or go to [OptiConnect](https://opticonnect.opticon.com) (available for iOS, Android, Mac OS and Windows).
2. Make sure the OPN-6000 is set to BLE mode by scanning the label in the quick start guide.
3. In OptiConnect, go to ‘Pair Scanner,’ and select the discovered ‘OPN-6000’ or 'OPN-25000' to connect

A Flutter SDK is available for mobile app developers to use the OptiConnect functionality in your own Flutter application:
[opticonnect_sdk_flutter](https://github.com/OpticonOSEDevelopment/opticonnect_sdk_flutter)

An Android SDK is available for mobile app developers to use the OptiConnect functionality in your own Android application:
[opticonnect_sdk_android](https://github.com/OpticonOSEDevelopment/opticonnect_sdk_android)

# OptiConfigure

Opticon barcode readers can be customized using configuration barcodes. All available configurations for each device can be found here: 

[OptiConfigure for OPN-6000](https://opticonfigure.opticon.com/configure?scanner=OPN6000)
[OptiConfigure for OPN-2500](https://opticonfigure.opticon.com/configure?scanner=OPN2500)

The SystemSetting()-function in the SDK makes it possible to use these configurations in your own application.

<code>int SystemSetting( const char *option_string );</code>

Note that 3-letter menu codes should be prefixed with the character '['. Likewise, 4-letter menu codes should be prefixed with the character ']'. 

### Example: 
<code>SystemSetting("[BCM");  // BCM: Enable all 1D codes excl. add-on</code>

# Porting existing applications

When porting applications of the OPN-200x, PX-20 and RS-3000 please note the following differences

## New Features in OPN-6000 / OPN-2500

- **BLE OptiConnect service** (BLE and USB)
- **Bluetooth Device Firmware Update (DFU)** (via OptiConnect)
- **Bluetooth BLE HID supported** (replaces Bluetooth Classic HID)
- **Both USB-VCP (default) and USB-CDC driver are supported**
- **Bluetooth, Batch, and OPN2001 application merged into a single default application**
- **New defaults (default firmware):**
  - **OPND**: OseComm data collector mode (previously Batch application + ‘C01’)
  - **BPC**: Switch to USB-CDC (Serial scanner mode)
  - **BQZ**: Switch to USB-CDC (OPN2001 mode)
  - **U2**: OPN-2001 default

## Removed / Missing Features from OPN-6000

- **Bluetooth Classic (HID/SPP) is not supported**
- **Scanning of Bluetooth address labels to connect is removed** (BLE is always slave)
- **Setting of Pin code not supported**
- **NetO protocol is no longer supported** (OseComm is recommended as alternative)

## Changed Default Behaviour (Bluetooth Application)

- **Bluetooth BLE HID is set as factory default**
- **Discoverability** Device becomes discoverable when pressing the trigger key
- **Memorizing** (when not connected) is enabled by default
- **Memorizing option**: 'Trigger to send' is enabled in BLE HID and USB-HID mode
- **Keyboard toggle on iOS** is now enabled by default
- **'Trigger to connect(able)’ uses function key**, not the trigger key
- **OPN6000 automatically switches into OPN2001 mode** after receiving an OPN2001 interrogate command
- **Manual reset** by pressing both keys for 15 seconds results in white LED. Release the function key to enter bootloader mode.
- **Device can enter sleep mode in all connection statuses** (except when connected to USB), greatly improving battery life when connected to Bluetooth.
- **Function key can be used to remove barcodes** when connected to OptiConnect (and when memorizing)
- **Default Bluetooth local name is OPN6000_(serial)** instead of OPN6000_(MAC Address), because mobile platforms often hide the Bluetooth address nowadays for privacy reasons.

## File System / Database Functions

- **The Bluetooth, Batch, and OPN2001 application are merged** into a single default application and store their data in the same format.
- **Switch between all applications without losing data.**
- **Barcode data is stored in two files**:
  - `SCANNED.DAT`: barcode data
  - `SCANNED.IDX`: quantity, serial, date, & time
- **The batch application generates the CSV file** with the configured formatting as soon as the USB-cable is connected instead of while scanning.
- **USB-MSD file system is now 'read/write'** instead of 'read/only' (file system corruption by Windows is resolved).
- **The OPN-6000/OPN-2500 uses ChaN's FatFS filesystem**, which uses functions like `f_open` and `f_close` (from `ff.h` and `FileSystem.h`).
- **For backwards compatibility**, `stdio.h` functions like `fopen` and `fclose` are supported using a wrapper.
- **Low-level I/O functions** `open`, `close`, `read`, `write` are no longer supported.

## Source code
- If your application is derived from the OPN-2006 Batch or Bluetooth application, it is recommended use the new Batch/Bluetooth application as starting point to add your customizations

## BLE Serial

BLE Serial (default **EBLE**) is the BLE alternative to the **Bluetooth 2.0 SSP profile**.  

It requires a custom BLE application on the host device with the following UUIDs:

- **Service UUID**: `46409be5-6967-4557-8e70-784e1e55263b`  
- **Read UUID**: `720330f4-1db7-4fd7-ae5a-87e5bd942880`  
- **Write UUID**: `708346f9-2a8f-4df6-aba8-b94625aede49`  

The **Read** & **Write** characteristics work like a serial interface.

- **Barcode data** is received on the **Read** characteristic.  
- **Configurations** can be made using the **Write** characteristic.  

### Command Format

A configuration command is formatted as:

<ESC><Command string><CR>

- `<ESC>` → ASCII escape character (`0x1B`)  
- `<Command string>` → Configuration option with its parameters (as found on OptiConfigure or the menu book)  
- `<CR>` → ASCII carriage return (`0x0D`)  

### Configuration Options

All available options for each device can be found here:

- [OptiConfigure for OPN-6000](https://opticonfigure.opticon.com/configure?scanner=OPN6000)  
- [OptiConfigure for OPN-2500](https://opticonfigure.opticon.com/configure?scanner=OPN2500) 

#### Example 1 — Set Code 39 prefix

`<ESC>`M41B`<CR>`

- Configures the ASCII control code `<STX>` as the prefix for Code 39.  
- In hexadecimal: 1B 4D 34 31 42 0D

#### Example 2 — 3-character command

`<ESC>`[BCC`<CR>`

- Each **3-character command** should be preceded with the `[` character (`0x5B`).  
- Enables **Data Matrix**.

#### Example 3 — 4-character command

`<ESC>`]DIAU`<CR>`

- Each **4-character command** should be preceded with the `]` character (`0x5D`).  
- Disables **auto-connect**.

### Storing Settings

Some options (e.g., defaults) are **not immediately active**.  
To store changes permanently in non-volatile memory, send: 

`<ESC>`Z2`<CR>`

### Built-in Commands

For feedback the following options are available (i.e `<ESC>`B`<CR>`)
- `B` → Sound a **good read** beep  
- `E` → Sound an **error** beep  
- `L` → Switch on **good read LED**  
- `N` → Switch on **bad read LED**  
- `O` → Switch on **both LEDs**  

