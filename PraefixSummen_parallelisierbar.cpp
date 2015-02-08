// Test.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	int feld[256];
	int x[256];		// array of prefix sums to be alculated
	int n = 256;
	int k = 8;
	int i, d;

	// initialisieren
	for (i=0 ; i<n ; i++)
		feld[i] = 2;

	// kopieren
	for (i=0 ; i<n ; i++)
		x[i] = feld[i];

	// Up-Sweep
	for (d=0 ; d<k ; d++) 
		for (i=(1<<(d+1))-1 ; i<n ; i+=1<<(d+1)) // diese Schleife kann parallelisiert werden
           x[i] = x[i] + x[i-(1<<d)];

	// Down-Sweep
	x[n-1]=0;
	for (d=k-1 ; d>=0 ; d--) 
		for (i=(1<<(d+1))-1 ; i<n ; i+=1<<(d+1)) { // diese Schleife kann parallelisiert werden
			int tmp = x[i];
			x[i] += x[i-(1<<d)];
			x[i-(1<<d)] = tmp;
     }

	// jetzt enthält das Feld x die Präfix-Summen

	return 0;
}

