# QuickAir / CannaBreezy
Arduino compatible Hard- and Software to get climate & air, to grow plants, done.

<br>

CannaBreezy is made to measure, monitor and regulate all elementary air values.
All of the following values having their own ports on the µC to do action if the value is higher or lower than your configuration allows. High/low values, timings and time-outs are free configurable.

- Temperature
  - Exhaust
  - Intake
  - Circulation
  - Heat & Cool
- Humidity
  - Dry & Humidify
- CO2
  - Add
- Dewing Point
  - Dry / Heat

**This project is actually under heavy development! Just the functionality as "reader" is useable right now!** 


**CannaBreezy** is functioning without the need of any network connections. All configuration can be made via an onBoard running HMI with support for VT-100 compatible Terminals. So, Linux, macOS and since a short while even Windows, having everything on board to manage and view **CannaBreezy**.
Untested right now, but terminal-apps for mobile phones will be able to controls CannaBreezy, too - as long they can manage VT-100 sequences!

- once set, it will reboot after power losses safe back into the right state without the need of manual actions. Even if CannaBreezy is in ModBUS-Slave-Mode and the master is dead - CannaBreezy will run your predefined emergency settings until the master is back.

- All water specific sensors/modules from <a href="https://atlas-scientific.com/"> Atlas-Scientific</a> are supported.

- On a 328-Arduino, **CannaBreezy** can manage up to 10 of those Atlas sensors/modules. One 4-point level-sensor is on board of a 328.

<br>

**CannaBreezy** has two siblings...
- <a href="https://github.com/PitWD/QuickTimer"> CannaClocky</a> to get any timings, to grow plants, done. As you can read in another repository from me: <a href="https://github.com/CannaParts/LetsGrowSmart/blob/main/FastVegaFlowerLowPower.md">To grow Cannabis based on 24h cycles is wasted time and energy.</a> To do such timings - you will need CannaClocky.

- <a href="https://github.com/PitWD/QuickWater"> CannaWatery</a> to measure and regulate pH, EC, °C, Redox, O2 and level of your water. *All water specific sensors/modules from <a href="https://atlas-scientific.com/"> Atlas-Scientific</a> are supported.* 

All three together are the base to have a very solid control over your grow. Their ability to function as ModBUS slaves makes it possible to let (multiple of) them act in a bigger context under control of more complex controllers, HMIs and 3rd party products.

<br>

The **loop screen** during runtime. Visualization of all probes and their averaged value.  
The states "TooLow", "Low", "OK", "High", "TooHigh" get visualized in different colors. You're absolutely free to define of what "(Too)High" and "(Too)Low" is, so that it will fit best to your needs. The last line symbolizes the action-ports  & action-values for "(Too)Low", "(Too)High".  
![Loop Screen](/images/Auswahl_001.png)

<br>

The *primary* **main settings menu**. This menu gives you access on generic settings and installed probes/modules.  
You can individually setup the color and style of the menu-key and the dimmed text style, to have a nice appearance fitting to your terminal/desktop style. The screenshots are made with an "solarized terminal" just the green is a little greener than from the original solarized green.  
![Primary Main Screen](/images/Auswahl_002.png)
Key color changed:
![Primary Main Screen](/images/Auswahl_003.png)
Dim Color changed:
![Primary Main Screen](/images/Auswahl_004.png)

<br>

The *secondary* **main settings menu**. All probes/modules are selected.
![Secondary Main Screen](/images/Auswahl_005.png)

<br>

The **probes/modules menu**. One of two humidity probes is selected.
![Probes Screen 1 selected](/images/Auswahl_006.png)
With both probes selected.
![Probes Screen 2 selected](/images/Auswahl_007.png)

<br>

The **calibration menu**. Two humidity probes are selected.
![Calibration Screen](/images/Auswahl_008.png)
During the calibration for a 1-Point calibration.
![Calibration Screen Dry](/images/Auswahl_009.png)

<br>

The **Set Values Menu**. Here you define the values for "(Too)Low" and "(Too)High".  
*On a 328 you can define up to three independent settings*
![Set Timings Menu](/images/Auswahl_010.png)

<br>

The **Set Timings & Steps Menu**. Here you define how the action-times are if a state is for at least the delay-time "TooLow", "Low", "High" or "TooHigh". You also define the ventilation-speed mapping here.   
*On a 328 you can define up to three independent settings*
![Set Timings Menu](/images/Auswahl_011.png)

<br>

I can't declare this project actually as that useable like I do for <a href="https://github.com/PitWD/QuickTimer"> CannaClocky</a> or the <a href="https://github.com/PitWD/QuickWater"> CannaWatery</a>. The communication-part with the Atlas-Scientific modules and visualization of the read values is 100% usable - some prototypes are working perfect since a while. Just the actions and manual-mode isn't fully implemented/functioning yet...  
Some planned extensions could break the actual structure of the internal EPROM - this will break the user made settings. There is actually no safe "fuse" to prevent that re-flashed µC's with a then broken EPROM will boot in a usable state.

