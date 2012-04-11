//============================================================================
// Name        : TauP_pspo.cpp
// Author      : Makoto Sadahiro
// Version     :
// Copyright   : Texas Advanced Computing Center
// Description : TauP domain volume to VTK format converter in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <limits>
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <cmath>

using namespace std;

/* constant for version, debug and etc */
// current version number;  change it here
const string TauP_pspo_version= "alpha 0.1";
const string TauP_pspo_build_date= __DATE__;
const string TauP_pspo_build_time= __TIME__;

unsigned int dim_ps = 241;
unsigned int dim_t = 500;
unsigned int dim_po = 131;
float min_s = numeric_limits<float>::max();
float max_s = -numeric_limits<float>::max();

int main(int argc, char *argv[]) {
	cout << "TauP_pspo:" << TauP_pspo_version << endl
			<< "     build " << TauP_pspo_build_date << " " << TauP_pspo_build_time << endl;

	string file_in;
	string argv_1= argv[1];
	if(("-help"==argv_1)||("--help"==argv_1)){
    cerr << "usage: fe2ras srcDataFile" << endl;
    return EXIT_FAILURE;
  }
	else{
		file_in = argv_1;
	}
	cout << "input data file name: " << file_in << endl;

	float min_cut = atof(argv[2]);
	float max_cut = atof(argv[3]);

	// open file and read to an array
	unsigned int dim_volume = dim_t * dim_ps * dim_po;
	float* fvol = new float[dim_volume];
	cout << "generated an array of " << dim_volume << "cells." << endl;

	FILE* pFile;
	pFile = fopen(file_in.c_str(), "r");

	size_t result = 0;// = fread(fvol, sizeof(float), dim_volume, pFile);

	for(unsigned int z = 0; z < dim_po; ++z){
		for(unsigned int x = 0; x < dim_ps; ++x){
			for(unsigned int y = 0; y < dim_t; ++y){
				fread(fvol + (dim_po -1 - z) * dim_ps * dim_t + y * dim_ps + x, sizeof(float), 1, pFile);
				result++;
			}
		}
	}

	cout << "finished reading " << result << " elements of size " << sizeof(float) << "." << endl;

	if(dim_volume != result){
		cout << "the element count does not match.  Exiting..." << endl;
		fclose(pFile);
		delete [] fvol;
    return EXIT_FAILURE;
	}
	fclose(pFile);

	// check on min and max
	//	for(unsigned int i = 0; i < dim_volume; ++i){
	//		if(fvol[i] < min_s){
	//			min_s = fvol[i];
	//		}
	//		if(max_s < fvol[i]){
	//			max_s = fvol[i];
	//		}
	//	}
	//	cout << "[min .. max] = [" << min_s << " .. " << max_s << "]" << endl;

	min_s = min_cut;
	max_s = max_cut;

	// cropping the range in the volume
	for(unsigned int i = 0; i < dim_volume; ++i){
		if(fvol[i] <= min_s){
			fvol[i] = -numeric_limits<float>::max() * 0.5;
		}
		if(max_s <= fvol[i]){
			fvol[i] = numeric_limits<float>::max() * 0.5;
		}
	}

	// scaling values to take advantage of float range
	float abs_max = max(abs(min_s), abs(max_s));
	float scaler = ((numeric_limits<float>::max()) * 0.9) / abs_max;
	cout << "abs_max:" << abs_max
			<< "  scaler:" << scaler
			<< "  maxfloat:" << numeric_limits<float>::max()
			<< endl;
//	for(unsigned int i = 0; i < dim_volume; i++){
//		if(fvol[i] <= min_s){}
//		else if(max_s <= fvol[i]){}
//		else{
//			fvol[i] *= scaler;
//			if(fvol[i]!=fvol[i]){
//				cout << "nan" << endl;
//			}
//		}
//	}

	// generate vtk file

	ofstream streamOut;
	streamOut.open((file_in + "_" + argv[2] + "_" + argv[3] + ".vtk").c_str());

	streamOut << "# vtk DataFile Version 2.0" << endl
			<< "TauP__t_ps_po" << endl
	    << "ASCII" << endl
	    << "DATASET RECTILINEAR_GRID" << endl
	    << "DIMENSIONS " << dim_ps << " " << dim_t << " " << dim_po << endl << endl;

	streamOut << "X_COORDINATES " << dim_ps << " float" << endl;
	for(unsigned int i = 0; i < dim_ps; i++){
		streamOut << i;
		if(i != (dim_ps - 1)){
			streamOut << " ";
		}else{
			streamOut << endl;
		}
	}

	streamOut << "Y_COORDINATES " << dim_t << " float" << endl;
	for(unsigned int i = 0; i < dim_t; i++){
		streamOut << i;
		if(i != (dim_t - 1)){
			streamOut << " ";
		}else{
			streamOut << endl;
		}
	}

	streamOut << "Z_COORDINATES " << dim_po << " float" << endl;
	for(unsigned int i = 0; i < dim_po; i++){
		streamOut << i;
		if(i != (dim_po - 1)){
			streamOut << " ";
		}else{
			streamOut << endl;
		}
	}

	streamOut << "POINT_DATA " << dim_volume << endl;

  // single scaler
  streamOut << "SCALARS myVar float 1" << endl
  		<< "LOOKUP_TABLE default" << endl;
  for(unsigned int i = 0; i < dim_volume; i++){
  	streamOut << fvol[i] << endl;
  }

  streamOut.close();

	// chean up all and exit
	delete [] fvol;
	cout << "exiting TauP_pspo." << endl;
	return 0;
}
