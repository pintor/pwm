# See LICENSE file for copyright and license information

all:
	g++ -std=c++11  pwm.cpp util.cpp -lX11 -o pwm

clean:
	rm pwm

install:
	cp pwm /usr/local/bin/

.PHONY: all clean
