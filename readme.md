# SERDES

This is a proof of concept (POC) intended to show the ability to serialize data over an existing I2S bus that is transporting audio.  The POC consists of two [MIMXRT685-AUD-EVK](https://www.nxp.com/design/development-boards/i-mx-evaluation-and-development-boards/i-mx-rt600-audio-evaluation-kit:MIMXRT685-AUD-EVK) boards - one acting as the master which is the audio source and the other acting as the slave which will receive the I2S stream and serialize or deserialize any data that is not on a designated audio channel (hence the name SERDES for serializer-deserializer).

## Proof of Concept Layout

A generalized depcition of the POC layout is shown in the image below.  In this image, the two boards are connected by a single I2S bus for receive and transmit.  The slave is connect to the onboard codec ([Cirrus Logic CS42448](https://www.cirrus.com/products/cs42448/)) which serves as the audio source for the proof of concept.  When triggered, the master board starts communication with the codec (i.e. receiving input audio from the codec) and starts the clocks (bit clock and frame sync) for the bridge communication between the master and the other devices on the line (the amps and the slave device).

![POC Layout](docs/POC_Layout.png)

Aside from the slave board, the system also consists of one or two [MAX98388](https://www.analog.com/en/products/max98388.html) amps from Analog Devices.  These amps can be configured to receive data in time domain multiplexed (TDM) format on a particular channel.  In addition, the amps can be configured to return IV data similarly in TDM format on particular channels.  Data is inserted on the configured channel and all other channels are held in a high impedance state allowing other devices on the line to insert data on other channels.

Although also depicted is a microphone acting as a source of PCM data, this will not be part of the proof of concept.  It is provided simply to demonstrate other possible sources of data on the return line of the master.

In addition to the codec on the master, the audio EVK provides other built in peripherals including switches and LEDs.  In this POC, the system will respond to a button presson on the slave by inserting data into the receive stream on the master.  When the master receives the data from the switch press, it will respond with a change in an LED state.

## RT685 Capabilities

## Functions

Blue LED indicates that the board is acting as a slave and is on at boot.

Green LED is lit when the master is transmitting on the bridge.

Pressing switch two on the master will start I2S transmission.

Pressing switch two on the slave will load a request for lighting up the LED on the master

# Resources

NXP. (2022, Febuary 17). *I2S Bus Specification v3*, https://www.nxp.com/docs/en/user-manual/UM11732.pdf.
NXP. (2020, Febuary 28). *AN12749: I2S Transmit and Receive on RT500 HiFi4*, https://style.nxp.com/docs/en/application-note/AN12749.pdf.