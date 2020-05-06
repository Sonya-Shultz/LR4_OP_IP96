#include <iostream>
#include <cstring>
#include <cmath>

using namespace std;
double str_to_int(string help);

int main(int argc, char* argv[])//запускать з консолі і пишеш щось типу !! OP_LR4_Home.exe(назві проекту) data.txt data3.txt 234,567(число збільшення, можна і через точку)!!
{
	double plusultra = 1;
	string inFile, outFile, help;
	if(argc>1)
	{	inFile = argv[1]; cout << inFile << endl;
		outFile = argv[2]; cout << outFile << endl; 
		help = argv[3]; plusultra = str_to_int(help); cout << plusultra << endl;
	}
}
double str_to_int(string help) 
{
	double a = 0, numer=10, tail=1, power=1;
	for (int i = 0; i < help.size(); i++)
	{
		if (help[i] != '.' && help[i] != ',') { a = a * numer + (help[i] - 48) * pow(tail, power); power++; }
		else { numer = 1; tail = 0.1; power = 1; }
	}
	return a;
}

typedef struct
{
	int8_t id[2];            // Завжди дві літери 'B' і 'M'
	int32_t filesize;        // Розмір файла в байтах                                   NEED
	int16_t reserved[2];     // 0, 0
	int32_t headersize=54L;      // 54L для 24-бітних зображень
	int32_t infoSize=40L;        // 40L для 24-бітних зображень
	int32_t width;           // ширина зображення в пікселях                            NEED
	int32_t hight;           // висота зображення в пікселях                            NEED
	int16_t biPlanes=1;        // 1 (для 24-бітних зображень)
	int16_t bits=24;            // 24 (для 24-бітних зображень)
	int32_t biCompression=0L;   // 0L
	int32_t biSizeImage=0L;     // Можна поставити в 0L для зображень без компрессії (наш варіант)
	int32_t biXPelsPerMeter=0L; // Рекомендована кількість пікселів на метр, можна 0L
	int32_t biYPelsPerMeter=0L; // Те саме, по висоті
	int32_t biClrUsed=0L;       // Для індексованих зображень, можна поставити 0L
	int32_t biClrImportant=0L;  // Те саме
} BMPHEAD;
typedef struct
{
	int8_t redComponent;      //00 00 ff 00
	int8_t greenComponent;    //00 ff 00 00    00 00 00 ff - alpha це вже записано в зворотньому порядку
	int8_t blueComponent;     //ff 00 00 00
} PIXELDATA;