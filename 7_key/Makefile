CROSS_COMPILE ?= arm-linux-gnueabihf-
TARGET        ?= key

CC            :=$(CROSS_COMPILE)gcc
LD            :=$(CROSS_COMPILE)ld
OBJCOPY       :=$(CROSS_COMPILE)objcopy
OBJDUMP       :=$(CROSS_COMPILE)objdump

INCLUDIRS     := imx6ul \
				 bsp/clk \
				 bsp/led \
				 bsp/beep \
				 bsp/key \
				 bsp/gpio \
				 bsp/delay 

SRCDIRS       := project \
				 bsp/clk \
				 bsp/led \
				 bsp/beep \
				 bsp/key \
				 bsp/gpio \
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
	$(LD) -Timx6u.lds -o $(TARGET).elf $^
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

$(SOBJS) : obj/%.o : %.s
	$(CC) -Wall -nostdlib -c -O2 $(INCLUDE) -o $@ $<

$(COBJS) : obj/%.o : %.c
	$(CC) -Wall -nostdlib -c -O2 $(INCLUDE) -o $@ $<

clean:
	rm -rf $(SOBJS)$(COBJS) $(TARGET).bin $(TARGET).elf $(TARGET).dis


