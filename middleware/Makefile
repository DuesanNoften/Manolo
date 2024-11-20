obj-m := Prueba.o  # Kernel module object
obj-y := libprueba.a    # Static library to be included in the kernel module

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
    
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

