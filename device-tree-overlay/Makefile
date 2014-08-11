
PRU-DTO-00A0.dtbo: build

build: PRU-DTO.dts
	dtc -O dtb -o PRU-DTO-00A0.dtbo -b 0 -@ PRU-DTO.dts 

.PHONY: clean
clean:
	rm PRU-DTO-00A0.dtbo

.PHONY: install
install: PRU-DTO-00A0.dtbo
	cp PRU-DTO-00A0.dtbo /lib/firmware

.PHONY: uninstall
uninstall: 
	rm /lib/firmware/PRU-DTO-00A0.dtbo

.PHONY: load
load:
	echo PRU-DTO > /sys/devices/bone_capemgr.9/slots

.PHONY: unload
unload:
	echo -`cat /sys/devices/bone_capemgr.9/slots | grep PRU-DTO | sed 's/:.*//' | sed 's/ //'` > /sys/devices/bone_capemgr.9/slots
