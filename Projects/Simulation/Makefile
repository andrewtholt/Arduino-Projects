
TARGETS=modbus calc

all:	$(TARGETS)

modbus:	modbus.c
	gcc -g modbus.c -o modbus

calc:	calc.c
	gcc -g calc.c -o calc

clean:
	rm -f $(TARGETS) cscope.out
