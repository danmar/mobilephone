
# mobilephone

Project to build a simple mobile phone (GSM). The mobile phone will handle
voice calls.

I will use off-the-shelf electronic components.

The software development and electronic components will be sponsored by
Evidente.

## Progress

### Step 1 : GSM chip

I bought a SM5100B GSM chip, a simple evaluation board for this chip, and a
GSM antenna. The chip understands AT commands.

I bought a normal prepaid comviq sim card. This has 3G and data but I won't
use that.

![alt text](http://github.com/danmar/mobilephone/raw/master/images/1-mini.png "Chip")

### Step 2 : Setup

I connected everything and tried to setup the chip for the GSM card. It was
quite straight forward.

By using AT commands from my PC I can send/receive SMS and dial/answer voice
calls (however I have no audio yet).

### Step 3 : Audio

Now I plan to connect microphone and speaker. The datasheet for SM5100B has
reference designs that I will try to use.

According to the datasheet, the speaker can be connected directly to output
pins on the chip.

I made this connection:
  GSM chip speaker output => voltage divider => microphone input on my PC

![alt text](http://github.com/danmar/mobilephone/raw/master/images/3a-mini.png "Connection")

Oscilloscope view when I whizzle in the other phone

![alt text](http://github.com/danmar/mobilephone/raw/master/images/3b.png "Oscilloscope view")

Audio recording when I say test, test. It's very faint.

http://github.com/danmar/mobilephone/raw/master/images/3.wma

### Step 4 : Connecting the microcontroller to the GSM chip

![alt text](http://github.com/danmar/mobilephone/raw/master/images/4-mini.png "MBED")
