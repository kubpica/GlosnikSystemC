#include "stdafx.h"
#include "systemc.h"
#include "Procesor1.cpp"
#include "Procesor2.cpp"
#include "modulSprzetowy.cpp"

SC_MODULE(queue)
{
	sc_fifo<int> Que;

	SC_CTOR(queue)
	{
		sc_fifo<int> Que(6);
	}
};

// sc_main in top level function like in C++ main
int sc_main(int argc, char* argv[]) {
	sc_signal<sc_uint<10>, SC_MANY_WRITERS> diodes;
	sc_signal<sc_uint<10>, SC_MANY_WRITERS> switches;
	sc_signal<sc_uint<8>, SC_MANY_WRITERS> hex[6];
	sc_signal<sc_uint<8>, SC_MANY_WRITERS> cpuTohardware;

	queue kolejka("queue");

	cpu1 procesor("cpu1");
	procesor.output(kolejka.Que);
	procesor.diodes(diodes);
	procesor.switches(switches);
	procesor.outToHardware(cpuTohardware);

	for (int i = 0; i < 6; i++)
		procesor.hexes[i](hex[i]);

	cpu2 procesor2("cpu2");
	procesor2.diodes(diodes);
	procesor2.hex(hex[3]);
	procesor2.input(kolejka.Que);

	hardware modulSprzetowy("hardware");
	modulSprzetowy.diodes(diodes);
	modulSprzetowy.hex(hex[2]);
	modulSprzetowy.input(cpuTohardware);

	sc_start();

	cout << endl << endl;
	return(0);
}