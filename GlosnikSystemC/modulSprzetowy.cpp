#include "stdafx.h"
#include <systemc.h>

SC_MODULE(hardware)
{
	sc_in<sc_uint<8>> input;
	sc_out<sc_uint<8>> hex;
	sc_out<sc_uint<10>> diodes;

	void setHex(unsigned char znak) {
		hex.write(znak);
	}

	unsigned int getLedMask(int i) {
		switch (i) {
		case 0:
			return LED0;
		case 1:
			return LED1;
		case 2:
			return LED2;
		case 3:
			return LED3;
		case 4:
			return LED4;
		case 5:
			return LED5;
		case 6:
			return LED6;
		case 7:
			return LED7;
		case 8:
			return LED8;
		case 9:
			return LED9;
		}
		return 0;
	}

	void setLed(unsigned int dioda, int stan, float time) {
		unsigned int mask = getLedMask(dioda);
		sc_uint<32> led = diodes.read();
		if (stan) {
			led = led | mask;
		}
		else {
			led = led & ~mask;
		}
		diodes.write(led);
	}

	sc_uint<8> in;

	void run()
	{
		while (1) {
			in = input.read();
			if (in == 'E') {
				setHex(HEXE);
			}
			else if (in == 3) {
				wait(50, SC_MS);
				setHex(THREE);
				setLed(2, 1, 0);
			}
			else if (in == 0) {
				wait(50, SC_MS);
				setHex(0);
				setLed(2, 0, 0);
			}
			wait(50, SC_MS);
		}
	}

	SC_CTOR(hardware)
	{
		SC_THREAD(run);
	}
};