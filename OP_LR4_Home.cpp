#include <iostream>
#include <cstring>
#include <cmath>
#include <fstream>
#include <math.h>

using namespace std;
double str_to_int(string help);

typedef struct
{
	int8_t id[2];            // ������ �� ����� 'B' � 'M' //�� ����� ���� ��� ��� �� � ����� ���������� ����� ������� � ������� ���� ������, ��� ���� ���� ��� � ������� ������, ���� ����� ����������� � ���� �������� �� �������� ��� � ���������� �������
	int32_t filesize;        // ����� ����� � ������                                   NEED
	int16_t reserved[2] = { 0,0 };     // 0, 0
	int32_t headersize = 54L;      // 54L ��� 24-����� ���������
	int32_t infoSize = 40L;        // 40L ��� 24-����� ���������
	int32_t width;           // ������ ���������� � �������                            NEED
	int32_t hight;           // ������ ���������� � �������                            NEED
	int16_t biPlanes = 1;        // 1 (��� 24-����� ���������)
	int16_t bits = 24;            // 24 (��� 24-����� ���������)
	int32_t biCompression = 0L;   // 0L
	int32_t biSizeImage = 0L;     // ����� ��������� � 0L ��� ��������� ��� �������� (��� ������)
	int32_t biXPelsPerMeter = 0L; // ������������� ������� ������ �� ����, ����� 0L //�� ��� ���� �� ���� ����� � "������", ������ �� ��� ��������
	int32_t biYPelsPerMeter = 0L; // �� ����, �� �����
	int32_t biClrUsed = 0L;       // ��� ������������ ���������, ����� ��������� 0L
	int32_t biClrImportant = 0L;  // �� ����
} BMPHEAD;
typedef struct
{
	int8_t redComponent;      //00 00 ff 00
	int8_t greenComponent;    //00 ff 00 00    00 00 00 ff - alpha// �� ��� �������� � ����������� �������
	int8_t blueComponent;     //ff 00 00 00
} PIXELDATA;
typedef struct
{
	PIXELDATA fir;
	PIXELDATA sec;
	PIXELDATA thi;
	PIXELDATA fou;
	PIXELDATA fif;
}NEIGHBORHOOD;

class Picture
{
public:
	Picture(string inFile, string outFile, double augment);
	void resize_picture();//������� �������� �����������, ������ ��� ����������� ������
private:/*���� ������� �������:
		1)���������� ����� � �����
		1.5)������� � �������
		2)������� ������ � ��������� ������ � ������ �����  ---------------------������
		2.1) ����� ��������� ��������� ��� ����� ��������� NULL, ��� �� ��� ������ ��� ��������� + ���� ����� ������ "������" ����� -----------------������
		3)���� ������������. � �� ������� (�� ������ ������, �� ����� ������� � ������� ���� �� ������). ���� ���������� ��� ��� ������� ����� ------------------������
		4)������� ��� ������ ����� ����� � ����� ����. ����� ��������� ���� � ���� ������ ����� �� �������� �����
		
	*/
	BMPHEAD bmpFile;
	int new_sizeY; //= static_cast<int>(bmpFile.hight * augment)
	int new_sizeX; //= static_cast<int>(bmpFile.width * augment)
	PIXELDATA** color_table ;//�� �����, � ���� �� ������ �������� �������� ������� ��� ������, �� ��� ������� �������������, ���� ������ �� �������
	PIXELDATA** initial_table ;//�� ���������� ����� � ���������, ���� ����� ������� � ������������ �� �
	double augment; //���������� ���������
	string inFile, outFile; //����� �����
	int get32Bit(char arr[]); 
	int getDataFromFile();
	void bicubic_interpolation(); //������� ������������ ��� ������ ������ �������
	void streatch(); //��������� ������ ������� � ���� ����������
	
	PIXELDATA full_NULL(); //�������� 1 ������ ��� ��� ������ (���� �����) ��������� ����
	void full_NULL_mas(); //�������� ���� 2� ����� �������� � ��������� ����
	bool is_empty_(PIXELDATA pix); //�������� �� ����� ������ ������� 
	bool exist(int i, int j); //�������� ���������� ����������� ��� ���������� �� ������
	PIXELDATA calculate_color(int i, int j); //���� ����  ��� ������ ������
	PIXELDATA sum_pix(PIXELDATA pix1, PIXELDATA pix2, int a); //���� ������� ������� ���� ������, �� 2�� ���� ������� � ����
	PIXELDATA div_pix(PIXELDATA pix, int8_t count); //������ �������� ������� ������ �� ������� ����� (��� ����������� ���������� �������� �������)
};

Picture::Picture(string inFile, string outFile, double augment)
{
	this->augment = augment; //� ������ ���� ����������
	this->inFile = inFile;
	this->outFile = outFile;
}

int Picture::get32Bit(char arr[]) {
	int helpArr[4][2];
	uint32_t a = 0;
	for (int i = 0; i < 4; i++) {
		if ((int)arr[i] < 0) {
			helpArr[i][0] = (256 + (int)arr[i]) / 16;
			helpArr[i][1] = (256 + (int)arr[i]) % 16;
			i++;
		}
		helpArr[i][0] = ((int)arr[i]) / 16;
		helpArr[i][1] = ((int)arr[i]) % 16;
	}

	int powMy = 0;
	for (int i = 0; i < 4; i++) {
		a = a + helpArr[i][1] * pow(16, powMy) + helpArr[i][0] * (pow(16, powMy + 1));
		powMy = powMy + 2;
	}
	return a;
}

int Picture::getDataFromFile(){
	ifstream fin("inFile.bmp", ios::binary);
	ofstream outfin("outFlile.bmp", ios::binary);
	if (!fin.is_open) { cout << "In thes folder isn't such file to take data!"; }
	if (!outfin.is_open) { cout << "In thes folder isn't such file!"; }
	else {
		char arr[8];

		//BMP header

		// 0 bit, size - 2
		fin.read(arr, 2);
		if (arr[0] != 'B' || arr[1] != 'M') { cout << "It's not a BMP! "; return -1; }
		outfin.write(arr, 2);

		// 2 bit, size 4
		fin.read(arr, 4);
		int32_t sizeFile = get32Bit(arr);//������ ����� ����
		outfin.write((char*)&sizeFile, 4);

		// 6 bit 
		fin.read(arr, 4);
		outfin.write(arr, 4);
		//10 bit
		fin.read(arr, 4);
		const int32_t dataOfMap = get32Bit(arr); // ������������ ����� ����
		outfin.write(arr, 4);

		// DIB header
		//14 bit
		fin.read(arr, 4);
		const int32_t dibHeaderSize = get32Bit(arr); // ������ DIB headra
		outfin.write(arr, 4);
		//18 bit
		fin.read(arr, 4);
		const int32_t pixelWidth = get32Bit(arr); // ������ ��������
		outfin.write(arr, 4);
		//22 bit
		fin.read(arr, 4);
		bmpFile.hight = get32Bit(arr); // ������ ��������
		outfin.write(arr, 4);
		//26 bit
		fin.read(arr, 8);
		outfin.write(arr, 8);
		//34 bit
		fin.read(arr, 4);
		const int32_t imageSize = get32Bit(arr); // ������ ��������
		outfin.write(arr, 4);
		//38 bit
		fin.read(arr, 4);
		const int32_t horizResolution = get32Bit(arr); // ������ ��������� � ������
		outfin.write(arr, 4);
		//42 bit size 4
		fin.read(arr, 4);
		const int32_t vertResolution = get32Bit(arr); // ������ ��������� � ������
		outfin.write(arr, 4);

		//DIB color profile
		char* tempArr = new char[dataOfMap - 46];
		int tempSize = dataOfMap - 46;
		cout << tempSize;
		// read to map element
		fin.read(tempArr, tempSize);
		outfin.write(tempArr, tempSize);
		delete[] tempArr;
		fin.close();
	}
	return 0;
}

void Picture::resize_picture()
{
	//��� ���������� ������, ������� ��� ������� �� ������ �������
	bicubic_interpolation();
}

void Picture::bicubic_interpolation()
{
	Picture::full_NULL_mas();
	Picture::streatch();
	PIXELDATA NULL_PIX = full_NULL();
	for (int i = 0; i < new_sizeY; i++)
	{
		for (int j = 0; j < new_sizeX; j++)
		{
			if (is_empty_(color_table[i][j]))
			{
				color_table[i][j] = calculate_color(i, j);
			}
		}
	}
}
PIXELDATA Picture::calculate_color(int i, int j)
{
	NEIGHBORHOOD near;
	near.fir = near.sec = near.thi = near.fou = near.fif = full_NULL();
	int8_t count = 0, parts = 0;
	for (int b = -1; b < 2; b += 2)
	{
		if (exist(i, j + b)) { count++; near.fir = sum_pix(near.fir, color_table[i][j + b], 1); }
		if (exist(i + b, j)) { count++; near.fir = sum_pix(near.fir, color_table[i + b][j], 1); }
	}
	if (count != 0) { near.sec = div_pix(near.fir, count); }
	count = 0;
	for (int a = -1; a < 2; a += 2)
	{
		for (int b = -1; b < 2; b += 2)
		{
			if (exist(i + a, j + b)) { count++; near.sec = sum_pix(near.sec, color_table[i + a][j + b], 1); }
		}
	}
	if (count != 0) { near.sec = div_pix(near.sec, count); }
	count = 0;
	for (int b = -2; b < 3; b += 4)
	{
		if (exist(i, j + b)) { count++; near.thi = sum_pix(near.thi, color_table[i][j + b], 1); }
		if (exist(i + b, j)) { count++; near.thi = sum_pix(near.thi, color_table[i + b][j], 1); }
	}
	if (count != 0) { near.thi = div_pix(near.thi, count); }
	count = 0;
	for (int a = -2; a < 3; a += 4)
	{
		for (int b = -2; b < 3; b += 4)
		{
			if (exist(i + a, j + b)) { count++; near.fif = sum_pix(near.fif, color_table[i + a][j + b], 1); }
		}
	}
	if (count != 0) { near.fif = div_pix(near.fif, count); }
	count = 0;
	for (int a = -2; a < 3; a += 4)
	{
		for (int b = -1; b < 2; b += 2)
		{
			if (exist(i + a, j + b)) { count++; near.fif = sum_pix(near.fif, color_table[i + a][j + b], 1); }
			if (exist(i + b, j + a)) { count++; near.fif = sum_pix(near.fif, color_table[i + b][j + a], 1); }
		}
	}
	if (count != 0) { near.fou = div_pix(near.fou, count); }
	count = 0;
	if (!is_empty_(near.fir))
	{
		color_table[i][j] = sum_pix(color_table[i][j], near.fir, 5); parts = parts + 5;
	}
	if (!is_empty_(near.sec))
	{
		color_table[i][j] = sum_pix(color_table[i][j], near.sec, 4); parts = parts + 4;
	}
	if (!is_empty_(near.thi))
	{
		color_table[i][j] = sum_pix(color_table[i][j], near.thi, 3); parts = parts + 3;
	}
	if (!is_empty_(near.fou))
	{
		color_table[i][j] = sum_pix(color_table[i][j], near.fou, 2); parts = parts + 2;
	}
	if (!is_empty_(near.fif))
	{
		color_table[i][j] = sum_pix(color_table[i][j], near.fif, 1); parts = parts + 1;
	}
	color_table[i][j] = div_pix(color_table[i][j], parts);
	return color_table[i][j];
}
PIXELDATA Picture::div_pix(PIXELDATA pix, int8_t count)
{
	pix.blueComponent = pix.blueComponent / count;
	pix.greenComponent = pix.greenComponent / count;
	pix.redComponent = pix.redComponent / count;
	return pix;
}
PIXELDATA Picture::sum_pix(PIXELDATA pix1, PIXELDATA pix2, int a)
{
	PIXELDATA pix3;
	pix3.blueComponent = pix1.blueComponent + a * pix2.blueComponent;
	pix3.greenComponent = pix1.greenComponent + a * pix2.greenComponent;
	pix3.redComponent = pix1.redComponent + a * pix2.redComponent;
	return pix3;
}

bool Picture::is_empty_(PIXELDATA pix)
{
	PIXELDATA NULL_PIX = full_NULL();
	if (pix.blueComponent == NULL && pix.greenComponent == NULL && pix.redComponent == NULL) { return true; }
	else { return false; }
}

bool Picture::exist(int i, int j)
{
	if (i >= 0 && j >= 0 && i < new_sizeY && j < new_sizeX && !is_empty_(color_table[i][j])) { return true; }
	else { return false; }
}

PIXELDATA Picture::full_NULL()
{
	PIXELDATA pix;
	pix.redComponent = NULL;
	pix.greenComponent = NULL;
	pix.blueComponent = NULL;
	return pix;
}

void Picture::full_NULL_mas()
{
	for (int i = 0; i < new_sizeY; i++)
	{
		for (int j = 0; j < new_sizeX; j++)
		{
			color_table[i][j] = full_NULL();
		}
	}
}

void Picture::streatch()
{
	color_table = new PIXELDATA * [new_sizeY];
	for (int a = 0; a < old_sizeX; a++)
	{
		color_table[a] = new PIXELDATA[new_sizeX];
	}
	full_NULL_mas();
	for (int i = 0; i < old_sizeY; i++)
	{
		for (int j = 0; j < old_sizeX; j++)
		{
			color_table[static_cast<int>(i * augment)][static_cast<int>(j * augment)] = initial_table[i][j];
		}
	}
}

double str_to_int(string help)
{
	double a = 0, numer = 10, tail = 1, power = 1;
	for (int i = 0; i < help.size(); i++)
	{
		if (help[i] != '.' && help[i] != ',') { a = a * numer + (help[i] - 48) * pow(tail, power); power++; }
		else { numer = 1; tail = 0.1; power = 1; }
	}
	return a;
}

int main(int argc, char* argv[])//��������� � ������ � ����� ���� ���� !! OP_LR4_Home.exe(���� �������) data.txt data3.txt 234,567(����� ���������, ����� � ����� �����)!!
{
	double plusultra = 1;//� ������ ���� ����������
	string inFile, outFile, help;//����� �������� �� ��������� �����, ��������� ����� ��� ���������� ���������
	if (argc > 1)
	{
		inFile = argv[1]; cout << inFile << endl;
		outFile = argv[2]; cout << outFile << endl;
		help = argv[3]; plusultra = str_to_int(help); cout << plusultra << endl;
	}
	Picture my_pic(inFile, outFile, plusultra);
	my_pic.resize_picture();
}