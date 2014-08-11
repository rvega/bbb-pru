#BBB-PRU

This is all for figuring out how to program the PRU units in the Beagle Bone Black.

1. How to compile and run assembly code for the PRU? 
2. How to communicate between a host application (linux app) and the code running in the PRU? Interrupt based, please.
3. How to use GPIO and ADCs from the PRU code? 
4. Other hardware we might need, timers maybe? 
5. How to do all of this stuff in C instead of assembly?  

## 1. Install driver and assembler.

    cd vendors/am335x_pru_package/
    make
    make install


## Links:

https://delicious.com/rvega/beaglebone

https://github.com/beagleboard/am335x_pru_package

http://www.embeddedrelated.com/showarticle/586.php

http://www.embeddedrelated.com/showarticle/603.php

https://github.com/texane/pru_sdk

https://github.com/BeaglePilot/PRUSS-C   <-- Nice lirary!

## License

GPLv3
