obj-m := arduino_driver.o
obj-y := libarduino_driver.a

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
    
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

