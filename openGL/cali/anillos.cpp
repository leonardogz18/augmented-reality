#include "calibrator.h"

int main(int, char**)
{
	Calibrator cali;
	int th, max, min;	float mult;
	th = 250;	max = 160;	min=35;	mult=1.6;
	cali.init("Calibrar_Prueba.wmv",th, max, min, mult);
	//th = 250;	max = 150;	min=25;	mult=2.;
	//cali.init("Prueba.wmv",th, max, min, mult);
	//cali.draw_axis();
	//cali.gen_intrinsic_data("data");
	//cali.read_intrinsic_data("data");
	cali.loop();
    
    return 0;
}
