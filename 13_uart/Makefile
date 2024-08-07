CROSS_COMPILE ?= arm-linux-gnueabihf-
TARGET        ?= print

CC            :=$(CROSS_COMPILE)gcc
LD            :=$(CROSS_COMPILE)ld
OBJCOPY       :=$(CROSS_COMPILE)objcopy
OBJDUMP       :=$(CROSS_COMPILE)objdump

LIBPATH		  := -lgcc -L /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/lib/gcc/arm-linux-gnueabihf/4.9.4

INCLUDIRS     := imx6ul \
				 bsp/clk \
				 bsp/led \
				 bsp/beep \
				 bsp/key \
				 bsp/gpio \
				 bsp/int \
				 bsp/exti \
				 bsp/epittimer \
				 bsp/keyfilter \
				 bsp/uart \
				 stdio/include \
				 bsp/delay 

SRCDIRS       := project \
				 bsp/clk \
				 bsp/led \
				 bsp/beep \
				 bsp/key \
				 bsp/gpio \
				 bsp/int \
				 bsp/exti \
				 bsp/epittimer \
				 bsp/keyfilter \
				 bsp/uart \
				 stdio/lib \
				 bsp/delay 

INCLUDE       :=  $(patsubst %, -I %, $(INCLUDIRS))

SFILES         := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.s))
CFILES         := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

SFILENDIR      := $(notdir $(SFILES))
CFILENDIR      := $(notdir $(CFILES))

SOBJS       :=  $(patsubst %, obj/%, $(SFILENDIR:.s=.o))
COBJS       :=  $(patsubst %, obj/%, $(CFILENDIR:.c=.o))

OBJS        := $(SOBJS)$(COBJS)

VPATH       := $(SRCDIRS)

.PHONY:clean

$(TARGET).bin : $(OBJS)
	$(LD) -Timx6u.lds -o $(TARGET).elf $^ $(LIBPATH)
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

$(SOBJS) : obj/%.o : %.s
	$(CC) -Wall -nostdlib -fno-builtin -c -O2 $(INCLUDE) -o $@ $<

$(COBJS) : obj/%.o : %.c
	$(CC) -Wall -Wa,-mimplicit-it=thumb -nostdlib -fno-builtin -c -O2 $(INCLUDE) -o $@ $<

clean:
	rm -rf $(SOBJS)$(COBJS) $(TARGET).bin $(TARGET).elf $(TARGET).dis


