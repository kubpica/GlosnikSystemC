#include "stdafx.h"
#include <systemc.h>
#include <conio.h>

//#include "modulSprzetowy.cpp"

using namespace std;

SC_MODULE(cpu1)
{
	sc_inout<sc_uint<10>> switches;
	sc_inout<sc_uint<8>> hexes[6];
	sc_fifo_out<int> output;
	sc_inout<sc_uint<10>> diodes;
	sc_out<sc_uint<8>> outToHardware;

	int glosnosc = 0;
	int state = 0;
	int substate = 0; //status kanalu (lewy/prawy/zaden)
	unsigned int nieaktywneSw = 0; //maska sw wywo³uj¹cych alarm

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
		wait(50, SC_MS);
	}

	void zgasWszystkieDiody(float time) {
		int i = 0;
		for (i = 0; i < 10; i++) {
			setLed(i, 0, time);
		}
	}

	void setHex(int hex, unsigned char znak) {
		//unsigned int znak2 = znak;
		hexes[hex].write(znak);
	}

	unsigned char getHexMask(int i) {
		switch (i) {
		case 0:
			return ZERO;
		case 1:
			return ONE;
		case 2:
			return TWO;
		case 3:
			return THREE;
		case 4:
			return FOUR;
		case 5:
			return FIVE;
		case 6:
			return SIX;
		case 7:
			return SEVEN;
		case 8:
			return EIGHT;
		case 9:
			return NINE;
		}
		return 0;
	}

	unsigned int getSwMask(int i) {
		switch (i) {
		case 0:
			return SW0;
		case 1:
			return SW1;
		case 2:
			return SW2;
		case 3:
			return SW3;
		case 4:
			return SW4;
		case 5:
			return SW5;
		case 6:
			return SW6;
		case 7:
			return SW7;
		case 8:
			return SW8;
		case 9:
			return SW9;
		}
	}

	void kanal(int sw, unsigned int swstate) {
		switch (substate) {
		case 0: //¿aden
			if (swstate & getSwMask(sw)) {
				substate = 1; //lewy
				setLed(sw, 1, 1);
				setHex(1, HEXL);
				printf("Wybrano kanal lewy!\n");
			}
			else if (swstate & getSwMask(sw + 1)) {
				substate = 2; //prawy
				setHex(1, HEXP);
				setLed(sw + 1, 1, 1);
				printf("Wybrano kanal prawy!\n");
			}
			break;
		case 1: //kanal lewy
			if (!(swstate & getSwMask(sw))) {
				substate = 0;
				setLed(sw, 0, 2);
				setHex(1, SEGG);
				printf("Wylaczono tryb kanalu lewego!\n");
			}
			else if (swstate & getSwMask(sw + 1)) {
				nieaktywneSw = nieaktywneSw | getSwMask(sw + 1);
				przejdzDoStanu(6); //alarm
			}
			break;
		case 2: //kanal prawy
			if (!(swstate & getSwMask(sw + 1))) {
				substate = 0;
				setLed(sw + 1, 0, 2);
				setHex(1, SEGG);
				printf("Wylaczono tryb kanalu prawego!\n");
			}
			else if (swstate & getSwMask(sw)) {
				nieaktywneSw = nieaktywneSw | getSwMask(sw);
				przejdzDoStanu(6); //alarm
			}
			break;
		}
	}

	int sw9 = 0;
	int sw8 = 0;
	void glosnik(unsigned int swstate) {
		switch (sw9) {
		case 0:
			if (swstate & SW9) {
				sw9 = 1;
				setLed(9, 1, 1);
				printf("Przelaczono na dodawanie!\n");
			}
			break;
		case 1:
			if (!(swstate & SW9)) {
				sw9 = 0;
				setLed(9, 0, 2);
				printf("Przelaczono na odejmowanie!\n");
			}
			break;
		}

		switch (sw8) {
		case 0:
			if (swstate & SW8) {
				sw8 = 1;

				if (sw9)
					glosnosc += 1;
				else
					glosnosc -= 1;

				printf("Nowa glosnosc: %d \n", glosnosc);

				if (glosnosc >= 10 || glosnosc < 0) {
					nieaktywneSw = nieaktywneSw | SW8;
					przejdzDoStanu(6); //alarm
				}
				else {
					setHex(5, getHexMask(glosnosc));
					setLed(8, 1, 1);
				}
			}
			break;
		case 1:
			if (!(swstate & SW8)) {
				sw8 = 0;
				setLed(8, 0, 2);
			}
			break;
		}
	}

	void przejdzDoStanu(int stan) {
		//wyjscie z aktualnego stanu
		switch (state) {
		case 0: //idle
			break;
		case 1: //przedni
			setLed(0, 0, 2);
			break;
		case 2: //tylne
			setLed(1, 0, 2);
			break;
		case 3: //centralny
			outToHardware.write(0);

			//setLed(2, 0, 2);
			break;
		case 4: //niskotonowy
			output.write(0);

			//setLed(3, 0, 2);
			break;
		case 5: //wszystko
			setLed(4, 0, 2);
			break;
		case 6: //alarm
			zgasWszystkieDiody(2);
			setLed(9, 0, 2);
			setHex(0, ZERO);
			setHex(1, 0);
			//setHex(2, 0);
			//setHex(3, 0);
			outToHardware.write(0);
			output.write(0);
			setHex(4, 0);
			setHex(5, 0);
			break;
		}

		//przejscie do stanu docelowego
		switch (stan) {
		case 0: //idle
			state = 0;
			nieaktywneSw = SW9 | SW8;
			zgasWszystkieDiody(2);
			setHex(0, ZERO);
			setHex(1, 0);
			setHex(2, 0);
			setHex(3, 0);
			setHex(4, 0);
			setHex(5, 0);
			printf("Przelaczono na tryb 0 - idle!\n");
			break;
		case 1: //przedni
			state = 1;
			setLed(0, 1, 1);
			nieaktywneSw = SW3 | SW4;
			setHex(0, ONE);
			setHex(1, SEGG);
			setHex(5, getHexMask(glosnosc));
			printf("Przelaczono na tryb 1 - przednie!\n");
			break;
		case 2: //tylne
			state = 2;
			setLed(1, 1, 1);
			nieaktywneSw = SW0 | SW4;
			setHex(0, TWO);
			setHex(1, SEGG);
			setHex(5, getHexMask(glosnosc));
			printf("Przelaczono na tryb 2 - tylne!\n");
			break;
		case 3: //centralny
			outToHardware.write(3);







			state = 3;
			//setLed(2, 1, 1);
			nieaktywneSw = SW0 | SW1 | SW3 | SW4;
			setHex(0, 0);
			setHex(5, getHexMask(glosnosc));
			printf("Przelaczono na tryb 3 - centralny!\n");
			break;
		case 4: //niskotonowy
			output.write(4);








			state = 4;
			//setLed(3, 1, 1);
			nieaktywneSw = SW0 | SW1 | SW2 | SW4;
			setHex(0, 0);
			setHex(5, getHexMask(glosnosc));
			printf("Przelaczono na tryb 4 - niskotonowy!\n");
			break;
		case 5: //wszystko
			state = 5;
			setLed(4, 1, 1);
			nieaktywneSw = SW0 | SW1 | SW2 | SW3;
			setHex(0, FIVE);
			setHex(5, getHexMask(glosnosc));
			printf("Przelaczono na tryb 5 - wszystko!\n");
			break;
		case 6: //alarm
			state = 6;
			zgasWszystkieDiody(2);
			setLed(9, 1, 3);
			//nieaktywneSw = SW0 | SW1 | SW2 | SW3 | SW4 | SW8 | SW9;
			printf("Alarm!\n");
			setHex(0, HEXE);
			setHex(1, HEXE);
			//setHex(2, HEXE);
			//setHex(3, HEXE);
			outToHardware.write('E');
			output.write('E');
			setHex(4, HEXE);
			setHex(5, HEXE);
			break;
		}
		wait(500, SC_NS);
	}

	bool wyswietl = true;

	void task1()
	{
		//vector<int> stack;
		int i = 0;
		while (1)
		{
			wait(500, SC_NS);

			if (wyswietl) {
				//wyswietl = false;
				wyswietlStan();
			}

			if (_kbhit())
			{
				wyswietlStan();

				wait(25, SC_NS);
				cout << "(Entered an integer) sw: ";
				cin >> i;
				if (i >= 0 && i <= 9)
				{
					wait(500, SC_NS);
					sc_uint<32> sw = switches.read();
					if (i <= 9 && i >= 0) {
						int a = 1;
						while (i--) {
							a = a << 1;
						}
						if (sw & a)
							sw = sw & ~a;
						else
							sw = sw | a;
					}
					//wyswietl = true;
					switches.write(sw);
					wait(500, SC_NS);
				}
				else
				{
					cout << "Blad - nie ma takiego slidera" << endl;
				}
			}

			wait(500, SC_NS);
			sc_uint<32> swstate = switches.read();
			if (state == 0) {
				if (swstate & SW0) {
					przejdzDoStanu(1); //przedni
				}
				else if (swstate & SW1) {
					przejdzDoStanu(2); //tylni
				}
				else if (swstate & SW2) {
					przejdzDoStanu(3); //centralny
				}
				else if (swstate & SW3) {
					przejdzDoStanu(4); //niskotonowy
				}
				else if (swstate & SW4) {
					przejdzDoStanu(5); //wszystko
				}
				else if (swstate & nieaktywneSw) {
					przejdzDoStanu(6); //alarm
				}
			}

			wait(500, SC_NS);
			//switches.write(sw_state);
			wait(100, SC_MS);
		}
	}

	void task2() {
		while (1) {
			sc_uint<32> swstate = switches.read();

			if (state == 1) {
				if (!(swstate & SW0)) {
					przejdzDoStanu(0); //idle
				}
				else if (swstate & nieaktywneSw) {
					przejdzDoStanu(6); //alarm
				}
				else {
					kanal(1, swstate);
					glosnik(swstate);
				}
			}

			wait(10, SC_MS);
		}
	}

	void task3() {
		while (1) {
			sc_uint<32> swstate = switches.read();

			if (state == 2) {
				if (!(swstate & SW1)) {
					przejdzDoStanu(0); //idle
				}
				else if (swstate & nieaktywneSw) {
					przejdzDoStanu(6); //alarm
				}
				else {
					kanal(2, swstate);
					glosnik(swstate);
				}
			}

			wait(10, SC_MS);
		}
	}

	void task4() {
		while (1) {
			sc_uint<32> swstate = switches.read();

			if (state == 3) {
				if (!(swstate & SW2)) {
					przejdzDoStanu(0); //idle
				}
				else if (swstate & nieaktywneSw) {
					przejdzDoStanu(6); //alarm
				}
				else {
					glosnik(swstate);
				}
			}

			wait(10, SC_MS);
		}
	}

	void task5() {
		while (1) {
			sc_uint<32> swstate = switches.read();

			if(state==4){
				if(!(swstate & SW3)){
					przejdzDoStanu(0); //idle
				}else if(swstate & nieaktywneSw){
					przejdzDoStanu(6); //alarm
				}else{
					glosnik(swstate);
				}
			}

			wait(10, SC_MS);
		}
	}

	void task6() {
		while (1) {
			sc_uint<32> swstate = switches.read();

			if (state == 5) {
				if (!(swstate & SW4)) {
					przejdzDoStanu(0); //idle
				}
				else if (swstate & nieaktywneSw) {
					przejdzDoStanu(6); //alarm
				}
				else {
					glosnik(swstate);
				}
			}
			else if (state == 6) {
				if (!(swstate & nieaktywneSw)) {
					sw8 = 0;
					sw9 = 0;
					substate = 0;
					glosnosc = 0;

					//wyjœcie z alarmu:
					if (swstate & SW0) {
						przejdzDoStanu(1); //przedni
					}
					else if (swstate & SW1) {
						przejdzDoStanu(2); //tylni
					}
					else if (swstate & SW2) {
						przejdzDoStanu(3); //centralny
					}
					else if (swstate & SW3) {
						przejdzDoStanu(4); //niskotonowy
					}
					else if (swstate & SW4) {
						przejdzDoStanu(5); //wszystko
					}
					else {
						przejdzDoStanu(0); //idle
					}
				}
			}

			wait(200, SC_MS);

		}
	}

	unsigned char lastHex[6];
	unsigned int lastLed = 0;
	unsigned int lastSw = 0;

	void wyswietlStan() {
		bool czyToSamo = true;
		unsigned char hex[6];
		unsigned int led = 0;
		unsigned int sw = 0;

		sw = switches.read();
		if (sw != lastSw)
			czyToSamo = false;
		led = diodes.read();
		if (led != lastLed)
			czyToSamo = false;
		hex[0] = hexes[0].read();
		if (hex[0] != lastHex[0])
			czyToSamo = false;
		hex[1] = hexes[1].read();
		if (hex[1] != lastHex[1])
			czyToSamo = false;
		hex[2] = hexes[2].read();
		if (hex[2] != lastHex[2])
			czyToSamo = false;
		hex[3] = hexes[3].read();
		if (hex[3] != lastHex[3])
			czyToSamo = false;
		hex[4] = hexes[4].read();
		if (hex[4] != lastHex[4])
			czyToSamo = false;
		hex[5] = hexes[5].read();
		if (hex[5] != lastHex[5])
			czyToSamo = false;

		if (czyToSamo)
			return;

		unsigned int a = 1;
		int i = 0;            
		for (i = 5; i >= 0; i--) {
			printf(" "); (hex[i] & a) ? printf("_") : printf(" "); printf(" ");
		}
		printf("\n");
		unsigned int b = 0x00002;
		unsigned int f = 0x00020;
		unsigned int g = 0x00040;
		i = 0;
		for (i = 5; i >= 0; i--) {
			(hex[i] & f) ? printf("|") : printf(" ");
			(hex[i] & g) ? printf("_") : printf(" ");
			(hex[i] & b) ? printf("|") : printf(" ");
		}
		printf("\n");
		unsigned int e = 0x00010;
		unsigned int d = 0x00008;
		unsigned int c = 0x00004;
		i = 0;
		for (i = 5; i >= 0; i--) {
			(hex[i] & e) ? printf("|") : printf(" ");
			(hex[i] & d) ? printf("_") : printf(" ");
			(hex[i] & c) ? printf("|") : printf(" ");
		}
		printf("\n");
		a = 0x00000200;
		while (a) {
			(led & a) ? printf("* ") : printf("- ");
			a = a >> 1;
		}
		printf("\n");
		a = 0x00000200;
		while (a) {
			(sw & a) ? printf("1 ") : printf("0 ");
			a = a >> 1;
		}
		printf("\n");

		lastSw = sw;
		lastLed = led;
		lastHex[0] = hex[0];
		lastHex[1] = hex[1];
		lastHex[2] = hex[2];
		lastHex[3] = hex[3];
		lastHex[4] = hex[4];
		lastHex[5] = hex[5];
	}

	void postConstruct() {
		for (int i = 0; i < 6; i++)
		hexes[i].write((sc_uint<8>) ' ');
	}


	SC_CTOR(cpu1)
	{


		SC_THREAD(task1);

		SC_THREAD(task2);
		SC_THREAD(task3);
		SC_THREAD(task4);
		SC_THREAD(task5);
		SC_THREAD(task6);


		
	}
};