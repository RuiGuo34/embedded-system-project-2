CC := $(CROSS_COMPILE)gcc
WIRINGPI_PATH := /home/rui/RPdev/wiringPi_armhf/

all: w1 w2 w3

w1:
	$(CC) -o main_section2_w1 main_section2.c assignment1.c assignment2.c governor.c scheduler.o 1/workload.o 1/deadlines.c -lwiringPi -lpthread -I$(WIRINGPI_PATH) -L$(WIRINGPI_PATH)

w2:
	$(CC) -o main_section2_w2 main_section2.c assignment1.c assignment2.c governor.c scheduler.o 2/workload.o 2/deadlines.c -lwiringPi -lpthread -I$(WIRINGPI_PATH) -L$(WIRINGPI_PATH)

w3:
	$(CC) -o main_section2_w3 main_section2.c assignment1.c assignment2.c governor.c scheduler.o 3/workload.o 3/deadlines.c -lwiringPi -lpthread -I$(WIRINGPI_PATH) -L$(WIRINGPI_PATH)

clean:
	rm -rf main_section2_w1 main_section2_w2 main_section2_w3
