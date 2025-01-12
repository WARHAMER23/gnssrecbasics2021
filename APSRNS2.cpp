#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>  // loading libraries
#include <cmath>
#include <time.h>
#include <ctime> // ��������� ������������ ���������� �������
#include <iostream>
#include <fstream>
#include <windows.h>

#define sF1       pow(2,-5)
#define sF2       pow(2,-43)
#define sF3       pow(2,-31)
#define sF4       pow(2,-29)
#define sF5       pow(2,-33)
#define sF6       pow(2,-19)
#define sF7       pow(2,4)
#define sF8       pow(2,-55)    //  Scale factor     
constexpr auto sc = 180;   //Semi circles;
constexpr auto pi = 3.1415326535898;


struct Ephemeris {   
	double   Crs;
	double   Dn;
	double   M0;
	double   Cuc;
	double   e;
	double   Cus;
	double   sqrtA;
	uint32_t toe;
	double   Cic;
	double   Omega0;
	double   Cis;
	double   i0;
	double   Crc;
	double   omega;
	double   OmegaDot;
	double   iDot;
	double   Tgd;
	uint32_t toc;
	double   af2;
	double   af1;
	double   af0;
	uint32_t WN;
	uint16_t IODC;
	uint8_t  URA;
	uint8_t  Health;
	uint16_t IODE2;
	uint16_t IODE3;
	bool     codeL2;
	bool     L2P;
	uint32_t slot;
};
const int32_t subFrameLength = 300;
struct SF1_3 {
	uint32_t slot;
	char sf1[subFrameLength + 1];
	char sf2[subFrameLength + 1];
	char sf3[subFrameLength + 1];
};

struct Coord
{
	double x;
	double y;
	double z;
};
void printEmp(Ephemeris* ep);
int32_t file2subFrames(SF1_3* sf, FILE* fid, uint8_t svNum);
int32_t subFrames2Eph(Ephemeris* ep, SF1_3* subframes);
void save(Ephemeris* ep, FILE* fod);
void Coordinate_calculation(Ephemeris* ep, uint32_t t, Coord *Position);

int main(void)
{
	
	uint32_t begin_time = clock();
	uint8_t svNum = 16;
	FILE* fid = fopen("in.txt", "r");
	FILE* fod = fopen("out.txt", "w");
	FILE* fopenfile = fopen("coords.txt", "r");    // ���������� ��������� �� ������� 
	if (fid != nullptr) {
		SF1_3 subframes;
		if (!file2subFrames(&subframes, fid, svNum)) {

			Ephemeris *ep = (Ephemeris*)calloc(1, sizeof(Ephemeris));
			if (!subFrames2Eph(ep, &subframes)) {
				printEmp(ep);

			}
			else {
				printf(" Cannot decode subframes\n ");
			}
			fclose(fid);
			if (fod) {
				save(ep, fod);
			}
			else
			{
				printf(" Cannot open out.txt ");
			}
			fclose(fod);

			uint32_t start = 86400 + 18 + 1; // 1 iteration
			uint32_t stop = start + 86399;
			Coord Position;

			double** Massive_coordinates = new double*[3];

			for (int i = 0; i < 3; i++)
			{
				Massive_coordinates[i] = new double[stop - start];
			}

			double** Massive_coordinates_matlab = new double*[3];

			for (int i = 0; i < 3; i++)
			{
				Massive_coordinates_matlab[i] = new double[stop - start];
			}

			for (int t = start; t < stop; t++)
			{
				Coordinate_calculation(ep, t, &Position);
				Massive_coordinates[0][t - start] = Position.x;
				Massive_coordinates[1][t - start] = Position.y;
				Massive_coordinates[2][t - start] = Position.z;
			}

			std::ifstream file("coords.txt");
			double sec;
			if (!file.is_open())
				std::cout << "Can't open" << std::endl;
			else {
				for (int t = 0; t < stop - start; t++)
				{
					file >> sec >> Massive_coordinates_matlab[0][t] >> Massive_coordinates_matlab[1][t] >> Massive_coordinates_matlab[2][t];
				}
				file.close();
			}

			double max_delta = 0;
			for (int i = 0; i < 3; i++)
			{
				for (int k = 0; k < stop - start; k++)
				{
					if (abs(Massive_coordinates[i][k] - Massive_coordinates_matlab[i][k]) > max_delta)
					{
						max_delta = abs(Massive_coordinates[i][k] - Massive_coordinates_matlab[i][k]);
					}
				}
			}

			delete[] * Massive_coordinates;
			delete[] Massive_coordinates;
			delete[] * Massive_coordinates_matlab;
			delete[] Massive_coordinates_matlab;

			uint32_t end_time = clock();
			uint32_t calculation_time = end_time - begin_time;

			
			printf("\tmax_delta = %f \t[m] \n", max_delta);
			printf("\tLoop required = %f \t[sec] \n", double(calculation_time)/1000);
			free(ep);
		}
		else {
			printf(" Subframes not found\n ");
		}
	}
	else {
		printf(" Cannot open in.txt ");
	}


	return 0;
}
uint32_t str2uint(char *sf, int32_t start, int32_t stop) {
	uint32_t ans = 0;
	for (int i = start; i < stop; i++) {
		bool bit = (sf[i - 1] == '1');
		ans = ans | (bit << (stop - i - 1));

	}
	return ans;




}

uint32_t str2uint1(char *sf, int32_t start, int32_t stop, int32_t start1, int32_t stop1) {
	uint32_t ans = 0;
	for (int i = start; i < stop; i++) {
		bool bit = (sf[i - 1] == '1');
		ans = (ans | bit) << 1;

	}
	for (int i = start1; i < stop1; i++) {
		bool bit = (sf[i - 1] == '1');
		ans = ans | bit;
		if (i < stop1 - 1) {
			ans = ans << 1;
		}
	}
	return ans;
}
//uint32_t str2uint2(char *sf, int32_t start, int32_t stop, char *cf, int32_t start1, int32_t stop1) {
//	uint32_t ans = 0;
//	for (int i = start; i < stop; i++) {
//		bool bit = (sf[i - 1] == '1');
//		ans = ans | (bit << (stop - i - 1));
//
//	}
//	for (int i = start1; i < stop1; i++) {
//		bool bit = (cf[i - 1] == '1');
//		ans = ans | (bit << (stop1 - i - 1));
//
//	}
//	return ans;
//}

int32_t twoCompl2int(uint32_t ans, int numBit) {       // twos-complement func
	int32_t m = 0xFFFFFFFF;
	if ((numBit < 32) && bool((1 << numBit - 1) & ans))
	{
		ans |= m << numBit;
		return -(~(ans - 1));
	}
	if (numBit == 32 && bool((1 << 31) & ans)) {
		return -(~(ans - 1));
	}
	return ans;
}

int32_t subFrames2Eph(Ephemeris* ep, SF1_3* subframes) {
	double deg2rad = pi / sc;
	ep->slot = subframes->slot;
	ep->WN = str2uint(subframes->sf1, 61, 71);
	ep->Crs = twoCompl2int(str2uint(subframes->sf2, 69, 85), 16) * sF1;
	ep->Cuc = twoCompl2int(str2uint(subframes->sf2, 151, 167), 16)*sF4;
	ep->toe = str2uint(subframes->sf2, 271, 287)*sF7;
	ep->toc = twoCompl2int(str2uint(subframes->sf1, 219, 235), 16) * sF7;
	ep->IODC = str2uint1(subframes->sf1, 83, 85, 211, 219);
	ep->URA = str2uint(subframes->sf1, 73, 75);
	ep->Health = str2uint(subframes->sf1, 73, 79);
	ep->IODE2 = str2uint(subframes->sf2, 61, 69);
	ep->IODE3 = str2uint(subframes->sf3, 271, 279);
	//ep->IODE = twoCompl2int(str2uint2(subframes->sf2, 61, 69, subframes->sf3, 271, 279), 8);
	ep->codeL2 = bool(subframes->sf1[91]);
	ep->L2P = bool(subframes->sf1[91]);
	ep->e = str2uint1(subframes->sf2, 167, 175, 181, 205) * sF5;
	ep->af1 = twoCompl2int(str2uint(subframes->sf1, 249, 265), 16) * sF2;
	ep->af2 = twoCompl2int(str2uint(subframes->sf1, 241, 249), 8) * sF8;
	ep->af0 = twoCompl2int(str2uint(subframes->sf1, 271, 293), 22) * sF3;
	ep->Dn = twoCompl2int(str2uint(subframes->sf2, 91, 107), 16) * sF2 * sc * deg2rad;
	ep->M0 = twoCompl2int(str2uint1(subframes->sf2, 107, 115, 121, 145), 32) * sF3 * sc * deg2rad;
	ep->Cus = twoCompl2int(str2uint(subframes->sf2, 211, 227), 16) * sF4;
	ep->sqrtA = str2uint1(subframes->sf2, 227, 235, 241, 265) * sF6;
	ep->Cic = twoCompl2int(str2uint(subframes->sf3, 61, 77), 16) * sF4;
	ep->Omega0 = twoCompl2int(str2uint1(subframes->sf3, 77, 85, 91, 115), 32) * sF3 * sc * deg2rad;
	ep->Cis = twoCompl2int(str2uint(subframes->sf3, 121, 137), 16) * sF4;
	ep->i0 = twoCompl2int(str2uint1(subframes->sf3, 137, 145, 151, 175), 32) * sF3 * sc * deg2rad;
	ep->Crc = twoCompl2int(str2uint(subframes->sf3, 181, 197), 16) * sF1;
	ep->omega = twoCompl2int(str2uint1(subframes->sf3, 197, 205, 211, 235), 32) * sF3 * sc * deg2rad;
	ep->OmegaDot = twoCompl2int(str2uint(subframes->sf3, 241, 265), 24) * sF2 * sc * deg2rad;
	ep->iDot = twoCompl2int(str2uint(subframes->sf3, 279, 293), 14) * sF2 * sc * deg2rad;
	ep->Tgd = twoCompl2int(str2uint(subframes->sf1, 197, 205), 8) * sF3 * deg2rad;
	return 0;


}

int32_t file2subFrames(SF1_3* sf, FILE* fid, uint8_t svNum) {
	int32_t sth1, sth2, sth3, sth4, sth5;
	char str_0R[8];
	char str_GPSL1CA[12];
	char str_reh[8];
	char str[1000];
	uint32_t svStr;
	uint32_t slot;
	int32_t subFrameNum;

	uint32_t slot_SF1 = 0;
	uint32_t slot_SF2 = 0;
	uint32_t slot_SF3 = 0;
	int32_t readres = 0;

	while (readres != EOF)
	{
		svStr = 0;
		readres = fscanf(fid, "%d %d %d %s %s %s %u\t %u %d %d %d %s", &sth1, &sth2, &sth3, str_0R, str_GPSL1CA, str_reh, &svStr, &slot, &sth4, &sth5, &subFrameNum, str);
		if ((svStr == svNum) && (slot >= (604800 / 6))) {
			if (subFrameNum == 1) {
				slot_SF1 = slot;
				strncpy(sf->sf1, str, sizeof(sf->sf1));
			}
			else if (subFrameNum == 2) {
				slot_SF2 = slot;
				strncpy(sf->sf2, str, sizeof(sf->sf2));
			}
			else if (subFrameNum == 3) {
				slot_SF3 = slot;
				strncpy(sf->sf3, str, sizeof(sf->sf3));
			}
			if ((slot_SF1 + 1 == slot_SF2) && (slot_SF2 + 1 == slot_SF3)) {
				sf->slot = slot_SF1;
				return 0;
			}
		}






	}
	return 1;
}

void Coordinate_calculation(Ephemeris* ep, uint32_t t, Coord *Position)   // ���������� ��������� ��
{
	double Omega_e_dot = 7.2921151467e-5;
	double mu = 3.986005e14;
	double A = pow(ep->sqrtA, 2);
	double n_0 = sqrt(mu / pow(A, 3));
	int32_t t_k = t - ep->toe;

	if (t_k > 302400)
	{
		t_k = t_k - 604800;
	}
	else if (t_k < -302400)
	{
		t_k = t_k + 604800;
	}

	double n = n_0 + ep->Dn;

	double M_k = ep->M0 + n * t_k;
	double E_0 = M_k; // �������������� �������� 
	double E_k = 0;
	int k = 0;

	while (1)
	{
		E_k = E_0;
		E_0 = E_0 + (M_k - E_0 + ep->e * sin(E_0)) / (1 - ep->e * cos(E_0));

		if (abs(E_0 - E_k) < 1e-8){
			break;
	}
		k = k + 1;
	}

	double nu = atan2((sqrt(1 - pow(ep->e, 2)) * sin(E_k) / (1 - ep->e * cos(E_k))), ((cos(E_k) - ep->e) / (1 - ep->e * cos(E_k))));
	double Phi = nu + ep->omega;

	double delta_u = ep->Cus * sin(2 * Phi) + ep->Cuc * cos(2 * Phi);
	double delta_r = ep->Crs * sin(2 * Phi) + ep->Crc * cos(2 * Phi);
	double delta_i = ep->Cis * sin(2 * Phi) + ep->Cic * cos(2 * Phi);
	double u_k = Phi + delta_u;
	double r = pow(ep->sqrtA, 2) * (1 - ep->e * cos(E_k)) + delta_r;
	double i_corr = ep->i0 + delta_i + ep->iDot * t_k;

	double x_shtr = r * cos(u_k);
	double y_shtr = r * sin(u_k);

	double Omega_corr = ep->Omega0 + (ep->OmegaDot - Omega_e_dot) * t_k - Omega_e_dot * ep->toe;

	double x_k = x_shtr * cos(Omega_corr) - y_shtr * cos(i_corr) * sin(Omega_corr);
	double y_k = x_shtr * sin(Omega_corr) + y_shtr * cos(i_corr) * cos(Omega_corr);
	double z_k = y_shtr * sin(i_corr);

	Position->x = x_k;
	Position->y = y_k;
	Position->z = z_k;
}

void printEmp(Ephemeris* ep)
{
	printf("LNAV Ephemeris (slot = %u) =  \n", ep->slot);
	printf("\tCrs     = %e                \n", ep->Crs);
	printf("\tDn      = %e \t[deg/s]      \n", ep->Dn);
	printf("\tM0      = %f \t[deg]        \n", ep->M0);
	printf("\tCuc     = %e                \n", ep->Cuc);
	printf("\te       = %e                \n", ep->e);
	printf("\tCus     = %e                \n", ep->Cus);
	printf("\tsqrtA   = %e                \n", ep->sqrtA);
	printf("\ttoe     = %u                \n", ep->toe);
	printf("\tCic     = %e                \n", ep->Cic);
	printf("\tOmega0  = %f \t[deg]        \n", ep->Omega0);
	printf("\tCis     = %e                \n", ep->Cis);
	printf("\ti0      = %f \t[deg]        \n", ep->i0);
	printf("\tCrc     = %e               \n", ep->Crc);
	printf("\tomega   = %f \t[deg]        \n", ep->omega);
	printf("\tOmegaDot= %e \t[deg/s]      \n", ep->OmegaDot);
	printf("\tiDot    = %e \t[deg/s]      \n", ep->iDot);
	printf("\tTgd     = %e                \n", ep->Tgd);
	printf("\ttoc     = %u                \n", ep->toc);
	printf("\taf2     = %e                \n", ep->af2);
	printf("\taf1     = %e                \n", ep->af1);
	printf("\taf0     = %e                \n", ep->af0);
	printf("\tWN      = %u                \n", ep->WN);
	printf("\tIODC    = %u                \n", ep->IODC);
	printf("\tURA     = %u                \n", ep->URA);
	printf("\tHealth  = %u                \n", ep->Health);
	printf("\tIODE2   = %u                \n", ep->IODE2);
	printf("\tIODE3   = %u                \n", ep->IODE3);
	printf("\tcodeL2  = %u                \n", ep->codeL2);
	printf("\tL2P     = %u                \n", ep->L2P);
}

void save(Ephemeris* ep, FILE* fod)
{
	fprintf(fod,"LNAV Ephemeris (slot = %u) =  \n", ep->slot);
	fprintf(fod,"\tCrs     = %e                \n", ep->Crs);
	fprintf(fod,"\tDn      = %e \t[deg/s]      \n", ep->Dn);
	fprintf(fod,"\tM0      = %f \t[deg]        \n", ep->M0);
	fprintf(fod,"\tCuc     = %e                \n", ep->Cuc);
	fprintf(fod,"\te       = %e                \n", ep->e);
	fprintf(fod, "\tCus     = %e                \n", ep->Cus);
	fprintf(fod, "\tsqrtA   = %e                \n", ep->sqrtA);
	fprintf(fod, "\ttoe     = %u                \n", ep->toe);
	fprintf(fod, "\tCic     = %e                \n", ep->Cic);
	fprintf(fod, "\tOmega0  = %f \t[deg]        \n", ep->Omega0);
	fprintf(fod, "\tCis     = %e                \n", ep->Cis);
	fprintf(fod, "\ti0      = %f \t[deg]        \n", ep->i0);
	fprintf(fod, "\tCrc     = %e               \n", ep->Crc);
	fprintf(fod, "\tomega   = %f \t[deg]        \n", ep->omega);
	fprintf(fod, "\tOmegaDot= %e \t[deg/s]      \n", ep->OmegaDot);
	fprintf(fod, "\tiDot    = %e \t[deg/s]      \n", ep->iDot);
	fprintf(fod, "\tTgd     = %e                \n", ep->Tgd);
	fprintf(fod, "\ttoc     = %u                \n", ep->toc);
	fprintf(fod, "\taf2     = %e                \n", ep->af2);
	fprintf(fod, "\taf1     = %e                \n", ep->af1);
	fprintf(fod, "\taf0     = %e                \n", ep->af0);
	fprintf(fod, "\tWN      = %u                \n", ep->WN);
	fprintf(fod, "\tIODC    = %u                \n", ep->IODC);
	fprintf(fod, "\tURA     = %u                \n", ep->URA);
	fprintf(fod, "\tHealth  = %u                \n", ep->Health);
	fprintf(fod, "\tIODE2   = %u                \n", ep->IODE2);
	fprintf(fod, "\tIODE3   = %u                \n", ep->IODE3);
	fprintf(fod, "\tcodeL2  = %u                \n", ep->codeL2);
	fprintf(fod, "\tL2P     = %u                \n", ep->L2P);
}