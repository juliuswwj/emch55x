
all: ch55x.cpython-34m.so

run: ch55x.cpython-34m.so
	./ch55x.py ../ch552/main.ihx

ch55x.cpython-34m.so: ch55x.py build.py emu8051.h core.c opcodes.c disasm.c
	python3 build.py

prepair:
	sudo apt-get install libffi-dev python3-pip
	sudo pip3 install cffi

clean:
	-rm *.o *.so ch55x.c