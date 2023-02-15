# atmega328_vnh2sp30
### Basic program to control the VNH2SP30 motor driver with an ATMEGA328 through serial
Part of the demo uses code from [small_printf](https://www.menie.org/georges/embedded/small_printf_source_code.html), distributed under the [GNU LGPL](https://www.gnu.org/licenses/lgpl-3.0.en.html). <br>
<b>The current demo doesn't use the current sensing feature of the driver.</b> <br>
### Pin configuration :
| ATMEGA328 | VNH2SP30 |
| --------- | -------- |
| PB1       | PWM      |
| PD2       | INA      |
| PD4       | INB      |
| PD6       | ENA      |

<i>Note: this demo assumes that the ATMEGA328 is running at 16MHz, that the baud rate is 9600</i>
