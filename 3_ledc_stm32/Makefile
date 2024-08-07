objs 	:= start.o main.o
ld   	:=arm-linux-gnueabihf-ld
gcc  	:=arm-linux-gnueabihf-gcc
objcopy :=arm-linux-gnueabihf-objcopy
objdump :=arm-linux-gnueabihf-objdump

ledc.bin : $(objs)
	$(ld) -Timx6u.lds -o ledc.elf $^
	$(objcopy) -O binary -S ledc.elf $@
	$(objdump) -D -m arm ledc.elf > ledc.dis

%.o : %.s
	$(gcc) -Wall -nostdlib -c -O2 -o $@ $<
	
%.o : %.c
	$(gcc) -Wall -nostdlib -c -O2 -o $@ $<

clean:
	rm -rf *.o ledc.bin ledc.elf ledc.dis