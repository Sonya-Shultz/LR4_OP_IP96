#include <iostream>
#include <cstring>
#include <cmath>
#include <fstream>
#include <math.h>

using namespace std;
double str_to_int(string help);

typedef struct
{
	uint32_t sizeFile;        // Розмір файла в байтах                                   NEED
	int32_t old_sizeY;		  //высота пикселей старая
	int32_t  old_sizeX;		 //ширина пикселей старая 
	int32_t imageSize;		//размер картинки
	int32_t horizResolution; //шырина резолюции в файле
	int32_t vertResolution; //высота резолюции в файле
	int32_t dataOfMap;
} BMPHEAD;

typedef struct
{
	uint8_t redComponent;      //00 00 ff 00
	uint8_t greenComponent;    //00 ff 00 00    00 00 00 ff - alpha// це вже записано в зворотньому порядку
	uint8_t blueComponent;     //ff 00 00 00
	uint8_t full; //1 for full, 0 for empty
} PIXELDATA;
typedef struct
{
	PIXELDATA fir;
	PIXELDATA sec;
	PIXELDATA thi;
	PIXELDATA fou;
	PIXELDATA fif;
}NEIGHBORHOOD;
typedef struct
{
	int32_t x;
	int32_t y;
}COORD;

class Picture
{
public:
	Picture(string inFile, string outFile, double argument);
	void resize_picture();//Функція доступна користувачу, просто для ініціалізації роботи
private:/*сюди потрібно функції:
		1)зчитування даних з файлу
		1.5)Сеттери і Геттери
		2)перепис масиву з кольорами пікселів в більший розмір  ---------------------Готово
		2.1) Можна попереднь заповнити цей масив кольорами NULL, щоб не біло траблів при зчитуванні + буде змога знайти "порожні" пікселі -----------------Готово
		3)сама інтерполяція. Я за бікубічну (чи взагалі білінійну, бо фільт Ланцоша я посеред ночі не врубаю). Вона виконується над всім масивом даних ------------------Готово
		4)Функція для запису даних назад в новий файл. Шапка змінюється лише в місці розміру файла та довжинах сторін
		
	*/
	BMPHEAD bmpFile;
	
	int32_t new_sizeY; // высота пикселей новая
	int32_t  new_sizeX; //ширина пикселей новая 
	
	
	
	PIXELDATA** color_table;//це масив, в який ми будемо зберігати значення кольору для пікселів, він вже повністю ініціалізований, його чіпати не потрібно
	PIXELDATA** initial_table;//це початковий масив з кольорами, його треба зчитати і ініціалізувати по Х
	void addBArr();
	double argument; //коефіціент збільшення
	string inFile, outFile; //імена файлів
	int32_t get32Bit(char arr[]);
	uint8_t get8BitM(char arr[]);
	void getDataFromFile();
	void bicubic_interpolation(); //бікубічна інтерполяція для всього масиву кольорів
	void streatch(); //створення масиву кольорів і його розширення
	void count_new_fsize();
	PIXELDATA full_NULL(); //Заповнює 1 піксель для всіх каналів (окрім альфа) значенням НУУЛ
	void full_NULL_mas(); //Заповнює весь 2д масив пікселями зі значенням НУУЛ
	bool is_empty_(PIXELDATA pix); //Перевіряє чи даний піксель порожній 
	bool exist(uint32_t i, uint32_t j); //перевіряє доступність коефіцієнтів для переміщення по масиву
	void calculate_color(uint32_t i, uint32_t j); //рахує колір  для одного пікселя
	PIXELDATA sum_pix(PIXELDATA pix1, PIXELDATA pix2, uint8_t a); //сума значень кольорів двох пікселів, де 2ий може братися а разів
	PIXELDATA div_pix(PIXELDATA pix, uint8_t count); //частка значення кольору пікселя та якогось числа (для знаходження середнього значення кольору)
	void fillDataInFile();
	void LineInterpol(int32_t i, int32_t j);
	void find_value(int32_t i, int32_t j, COORD* coord);
};

Picture::Picture(string inFile, string outFile, double argument)
{
	this->argument = argument; //в скільки разів збільшувати
	this->inFile = inFile;
	this->outFile = outFile;
}

int32_t Picture::get32Bit(char arr[]) {
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

uint8_t Picture::get8BitM(char arr[]) {
	int helpArr[1][2];
	uint8_t a = 0;
	int i = 0;
	if ((int)arr[i] < 0) {
		helpArr[i][0] = (256 + (int)arr[i]) / 16;
		helpArr[i][1] = (256 + (int)arr[i]) % 16;
	}
	else {
		helpArr[i][0] = ((int)arr[i]) / 16;
		helpArr[i][1] = ((int)arr[i]) % 16;
	}

	int powMy = 0;
	a = a + helpArr[i][1] * pow(16, powMy) + helpArr[i][0] * (pow(16, powMy + 1));
	return a;
}

void Picture::getDataFromFile(){
	ifstream fin(inFile, ios::binary);
	cout << 1 << endl;
	if (!fin.is_open()) { cout << "In thes folder isn't such file to take data! "; }
	
	else {
		char arr[8];

		//BMP header

		// 0 bit, size - 2
		fin.read(arr, 2);
		if (arr[0] != 'B' || arr[1] != 'M') { cout << "It's not a BMP! "; return; }
		
		// 2 bit, size 4
		fin.read(arr, 4);
		bmpFile.sizeFile = get32Bit(arr); //размер файла байт
		// выводится ниже и дописывается в файл outfin.write((char*)&sizeFile, 4);

		// 6 bit 
		fin.read(arr, 4);
		
		//10 bit
		fin.read(arr, 4);
		bmpFile.dataOfMap = get32Bit(arr); // расположение байта мапа
		
		// DIB header
		//14 bit
		fin.read(arr, 4);
		int32_t dibHeaderSize = get32Bit(arr); // размер DIB headra
		
		//18 bit
		fin.read(arr, 4);
		bmpFile.old_sizeX = get32Bit(arr); // ширина пикселей
		//int32_t a = (bmpFile.old_sizeX * 3);
		//while (a % 4 != 0) { a++; }
		new_sizeX = static_cast<int32_t>(bmpFile.old_sizeX * argument);
		
		//22 bit
		fin.read(arr, 4);
		bmpFile.old_sizeY = get32Bit(arr); // высота пикселей
		new_sizeY = static_cast<int32_t>(bmpFile.old_sizeY* argument);
				
		//26 bit
		fin.read(arr, 8);
		
		//34 bit
		fin.read(arr, 4);
		bmpFile.imageSize = get32Bit(arr); // размер картинки
		bmpFile.imageSize = static_cast<int32_t>(bmpFile.imageSize * pow(argument, 2));
		
		
		//38 bit
		fin.read(arr, 4);
		bmpFile.horizResolution = get32Bit(arr); // ширина резолюции в метрах
		bmpFile.horizResolution = static_cast<int32_t>(bmpFile.horizResolution * argument);
		
		//42 bit size 4
		fin.read(arr, 4);
		bmpFile.vertResolution = get32Bit(arr); // высота резолюции в метрах
		bmpFile.vertResolution = static_cast<int32_t>(bmpFile.vertResolution * argument);
	
		//DIB color profile
		char* tempArr = new char[bmpFile.dataOfMap - 46];
		int32_t tempSize = bmpFile.dataOfMap - 46;

		// read to map element
		fin.read(tempArr, tempSize);
		
		char a[1];
		initial_table = new PIXELDATA * [bmpFile.old_sizeY];
		addBArr();
		count_new_fsize();

		for (int32_t i = 0; i < bmpFile.old_sizeY; i++) //bmpFile.old_sizeY
		{
			for (int32_t j = 0; j < bmpFile.old_sizeX; j++) {
					fin.read(a, 1);
					initial_table[i][j].blueComponent= get8BitM(a);

					fin.read(a, 1);
					initial_table[i][j].greenComponent = get8BitM(a);

					fin.read(a, 1);
					initial_table[i][j].redComponent = get8BitM(a);

					initial_table[i][j].full = 1;
			}
			int32_t q = 0;
			char co[1];
			co[0] = static_cast<char>(NULL);
			while ((bmpFile.old_sizeX * 3 + q) % 4 != 0)
			{
				q++;
				fin.read((char*)&co, 1);
			}
			
		}

		delete[] tempArr;
		fin.close();
	}
	return;
}

void Picture::count_new_fsize()
{
	int32_t a = (bmpFile.old_sizeX * 3);
	while (a % 4 != 0) { a++; }
	cout << bmpFile.sizeFile << endl << bmpFile.imageSize << endl;
	bmpFile.imageSize = bmpFile.old_sizeY*3*(a);
	bmpFile.imageSize = bmpFile.imageSize / 3 + 2;
	bmpFile.sizeFile = bmpFile.imageSize + bmpFile.dataOfMap;
	cout << bmpFile.sizeFile << endl<<bmpFile.imageSize<<endl;
}

void Picture::fillDataInFile() {
	ifstream fin(inFile, ios::binary);
	ofstream outfin(outFile, ios::binary);
	cout << 3 << endl;

	if (!fin.is_open() || !outfin.is_open()) { cout << "In thes folder isn't such file to take data!"; }
	else {
		char arr[12];
		// 0 bit
		fin.read(arr, 2);
		outfin.write(arr, 2);
		//2 bit
		fin.read(arr, 4);
		outfin.write((char*)&bmpFile.sizeFile, 4);
		// 6 bit
		fin.read(arr, 12);
		outfin.write(arr, 12);
		//18 bit
		fin.read(arr, 8);
		outfin.write((char*)&new_sizeX, 4);
		outfin.write((char*)&new_sizeY, 4);
		// 26 bit
		fin.read(arr, 8);
		outfin.write(arr, 8);
		//34 bit
		fin.read(arr, 4);
		outfin.write((char*)&bmpFile.imageSize, 4);
		// 38 bit
		fin.read(arr, 8);
		outfin.write((char*)&bmpFile.horizResolution, 4);
		outfin.write((char*)&bmpFile.vertResolution, 4);

		//46 bit
		char* tempArr = new char[bmpFile.dataOfMap - 46];
		int tempSize = bmpFile.dataOfMap - 46;
		// read to map element
		fin.read(tempArr, tempSize);
		outfin.write(tempArr, tempSize);
		delete[] tempArr;
		char colourArr[3];
		for (int32_t i = 0; i < new_sizeY; i++) {
			for (int32_t j = 0; j < new_sizeX; j++) {

				colourArr[0] = color_table[i][j].blueComponent;
				colourArr[1] = color_table[i][j].greenComponent;
				colourArr[2] = color_table[i][j].redComponent;
				uint32_t ans = (static_cast<uint8_t>(static_cast<uint8_t>(colourArr[0]))) |(static_cast<uint8_t>(static_cast<uint8_t>(colourArr[1])) << 8) | (static_cast<uint8_t>(static_cast<uint8_t>(colourArr[2])) << 16);// | static_cast<uint8_t>(static_cast<uint8_t>(colourArr[3] << 24));
				outfin.write((char*)&ans, 3);
			}
			int32_t q = 0;
			char co[1];
			co[0] = static_cast<char>(NULL);
			while ((new_sizeX * 3 + q) % 4 != 0)
			{
				q++;
				outfin.write((char*)&co, 1);
			}
		}
		/*uint32_t sum = bmpFile.dataOfMap;
		for (int32_t i = 0; i < bmpFile.old_sizeY; i++) {
			for (int32_t j = 0; j < bmpFile.old_sizeX; j++) {

				colourArr[0] = initial_table[i][j].blueComponent;
				colourArr[1] = initial_table[i][j].greenComponent;
				colourArr[2] = initial_table[i][j].redComponent;
				uint32_t ans = (static_cast<uint8_t>(static_cast<uint8_t>(colourArr[0]))) |(static_cast<uint8_t>(static_cast<uint8_t>(colourArr[1])) << 8) | (static_cast<uint8_t>(static_cast<uint8_t>(colourArr[2])) << 16);// | static_cast<uint8_t>(static_cast<uint8_t>(colourArr[3] << 24));
				
				outfin.write((char*)&ans, 3);
			}
			int32_t q = 0;
			char co[1];
			co[0]=static_cast<char>(NULL);
			while ((bmpFile.old_sizeX * 3 + q) % 4 != 0) 
			{ q++;
			outfin.write((char*)&co, 1);}
		}*/
	}
	outfin.close();
}

void Picture::resize_picture()
{
	getDataFromFile(); //считывание данных с файла
	bicubic_interpolation(); // интерполфция картинки
	fillDataInFile(); //запись в файл
}

void Picture::addBArr() {
	for (int32_t a = 0; a < bmpFile.old_sizeY; a++)
	{
		initial_table[a] = new PIXELDATA[bmpFile.old_sizeX];
	}
}

void Picture::bicubic_interpolation()
{
	cout << 2 << endl;
	//Picture::full_NULL_mas();

	Picture::streatch();
	for (uint32_t i = 0; i < new_sizeY; i++)
	{
		for (uint32_t j = 0; j < new_sizeX; j++)
		{
			if (color_table[i][j].full==0)
			{
				LineInterpol(i,j);
			}
		}
	}
}
void Picture::LineInterpol(int32_t i, int32_t j)
{
	COORD* coord = new COORD[4];
	find_value(i, j, coord);
	/*for (int i = 0; i < 4; i++)
	{
		cout << coord[i].x << "|" << coord[i].y << " ";
	}
	cout << endl;*/

	uint8_t R1 = 0, R2 = 0, R11=0, R22 =0, P1=0, P2=0, count1=0, count2=0;
	/*R1 = ((coord[0].x - i) / (coord[0].x - coord[2].x))*color_table[coord[0].x][coord[0].y].redComponent;
	R2 = ((i-coord[2 ].x) / (coord[0].x - coord[2].x)) * color_table[coord[2 ].x][coord[2 ].y].redComponent;
	R11 = ((coord[1].x - i) / (coord[1].x - coord[3].x)) * color_table[coord[1].x][coord[1].y].redComponent;
	R22 = ((i - coord[3].x) / (coord[1].x - coord[3].x)) * color_table[coord[3].x][coord[3].y].redComponent;
	P2 = ((-coord[0].y + j) / (-coord[0].y + coord[1].y)) * (R11 + R22);
	P1 = ((-j + coord[1].y) / (-coord[0].y + coord[1].y)) * (R1+R2);
	color_table[i][j].redComponent = (P1 + P2);

	R1 = ((coord[0].x - i) / (coord[0].x - coord[2].x)) * color_table[coord[0].x][coord[0].y].greenComponent;
	R2 = ((i - coord[2].x) / (coord[0].x - coord[2].x)) * color_table[coord[2].x][coord[2].y].greenComponent;
	R11 = ((coord[1].x - i) / (coord[1].x - coord[3].x)) * color_table[coord[1].x][coord[1].y].greenComponent;
	R22 = ((i - coord[3].x) / (coord[1].x - coord[3].x)) * color_table[coord[3].x][coord[3].y].greenComponent;
	P2 = ((-coord[0].y + i) / (-coord[0].y + coord[1].y)) * (R11 + R22);
	P1 = ((-i + coord[1].y) / (-coord[0].y + coord[1].y)) * (R1 + R2);
	color_table[i][j].greenComponent = (P1 + P2);

	R1 = (double)((coord[0].x - i) / (coord[0].x - coord[2].x)) * color_table[coord[0].x][coord[0].y].blueComponent;
	R2 = (double)((i - coord[2].x) / (coord[0].x - coord[2].x)) * color_table[coord[2].x][coord[2].y].blueComponent;
	R11 = (double)((coord[1].x - i) / (coord[1].x - coord[3].x)) * color_table[coord[1].x][coord[1].y].blueComponent;
	R22 = (double)((i - coord[3].x) / (coord[1].x - coord[3].x)) * color_table[coord[3].x][coord[3].y].blueComponent;
	P2 = (double)((-coord[0].y + j) / (-coord[0].y + coord[1].y)) * (R11 + R22);
	P1 = (double)((-j + coord[1].y) / (-coord[0].y + coord[1].y)) * (R1 + R2);
	color_table[i][j].blueComponent = (P1 + P2);*/

	R1 = color_table[coord[1].x][coord[1].y].blueComponent + 
	(i - coord[1].x) * ( color_table[coord[3].x][coord[3].y].blueComponent- color_table[coord[1].x][coord[1].y].blueComponent) /
	(coord[3].x- coord[1].x);
	R2 = color_table[coord[0].x][coord[0].y].blueComponent +
		(i - coord[0].x) * (color_table[coord[2].x][coord[2].y].blueComponent- color_table[coord[0].x][coord[0].y].blueComponent) /
		(coord[2].x - coord[0].x);
	P1 = R1 + (j - coord[1].y) * (R2 - R1) / (coord[0].y - coord[1].y);
	color_table[i][j].blueComponent = P1;

	R1 = color_table[coord[1].x][coord[1].y].redComponent +
		(i - coord[1].x) * (color_table[coord[3].x][coord[3].y].redComponent - color_table[coord[1].x][coord[1].y].redComponent) /
		(coord[3].x - coord[1].x);
	R2 = color_table[coord[0].x][coord[0].y].redComponent +
		(i - coord[0].x) * (color_table[coord[2].x][coord[2].y].redComponent - color_table[coord[0].x][coord[0].y].redComponent) /
		(coord[2].x - coord[0].x);
	P1 = R1 + (j - coord[1].y) * (R2 - R1) / (coord[0].y - coord[1].y);
	color_table[i][j].redComponent = P1;

	R1 = color_table[coord[1].x][coord[1].y].greenComponent +
		(i - coord[1].x) * (color_table[coord[3].x][coord[3].y].greenComponent - color_table[coord[1].x][coord[1].y].greenComponent) /
		(coord[3].x - coord[1].x);
	R2 = color_table[coord[0].x][coord[0].y].greenComponent +
		(i - coord[0].x) * (color_table[coord[2].x][coord[2].y].greenComponent - color_table[coord[0].x][coord[0].y].greenComponent) /
		(coord[2].x - coord[0].x);
	P1 = R1 + (j - coord[1].y) * (R2 - R1) / (coord[0].y - coord[1].y);
	color_table[i][j].greenComponent = P1;

	delete[] coord;
}
void Picture::find_value(int32_t i, int32_t j, COORD* coord)
{
	coord[0].x = ((int32_t)(i / argument)) * argument;
	coord[0].y = ((int32_t)(j / argument)+1) * argument;
	coord[1].x = ((int32_t)(i / argument)) * argument;
	coord[1].y = ((int32_t)(j / argument)) * argument;
	coord[2].x = ((int32_t)(i / argument)+1) * argument;
	coord[2].y = ((int32_t)(j / argument)+1) * argument;
	coord[3].x = ((int32_t)(i / argument)+1) * argument;
	coord[3].y = ((int32_t)(j / argument)) * argument;
	for (int i = 0; i < 5; i++)
	{
		if (coord[i].x > new_sizeX - 1)
		{
			coord[i].x = ((int32_t)((new_sizeX - 1) / argument) - 1) * argument;
			//cout << coord[i].x << " ";
		}
		if (coord[i].y > new_sizeY - 1)
		{
			coord[i].y = ((int32_t)((new_sizeY - 1) / argument) - 1) * argument;
		}
	}
}
/*void Picture::find_value(int32_t i, int32_t j, COORD* coord)
{
	int32_t count = 3, count1 = 1, count2=0;
	COORD coord1; 
	while (true)
	{
		coord1.x = i - count1; coord1.y = j - count1;
		for (int32_t a = 0; a < count; a++)
		{
			if (exist(coord1.y + a, coord1.x) ) {
				coord[count2].x = coord1.x;  coord[count2].y = coord1.y + a;  count2++; if (count2 == 8)return;
			}
		}
		for (int32_t a = 0; a < count-1; a++)
		{
			if (exist(coord1.y, coord1.x + a+1)) { 
				coord[count2].x = coord1.x + a+1;  coord[count2].y = coord1.y; count2++; if (count2 == 8)return;
			}
		}
		coord1.x = i + count1; coord1.y = j + count1;
		for (int32_t a = 0; a < count-2; a++)
		{
			if (exist(coord1.y - a, coord1.x)) { 
				coord[count2].x = coord1.x;  coord[count2].y = coord1.y - a;  count2++; if (count2 == 8)return;
			}
		}
		for (int32_t a = 0; a < count-2; a++)
		{
			if (exist(coord1.y, coord1.x +1 - a)) { 
				coord[count2].x = coord1.x - a+1;  coord[count2].y = coord1.y;  count2++; if (count2 == 8)return;
			}
		}
		count += 2; count1 +=1;
	}
}*/
/*void Picture::LineInterpol(int32_t i, int32_t j)
{
	int32_t count = 0, red = 0, blue=0, grean=0, up=100;
	for (int32_t a = -1; a < 2; a+=2) {
		for (int32_t b = -1; b < 2; b+=2)
		{
			if (exist(i + a, j + b) && color_table[i + a][j + b].full == 1) { up = 100; }
			else { up = 25; }
			if (count != 0 && exist(i + a, j + b)) 
			{
				red += up*static_cast<int32_t>(color_table[i + a][j + b].redComponent);
				blue += up * static_cast<int32_t>(color_table[i + a][j + b].blueComponent);
				grean += up * static_cast<int32_t>(color_table[i + a][j + b].greenComponent);
				count= up*1+count;
			}
			if (count == 0 && exist(i + a, j + b)) 
			{
				red = up * static_cast<int32_t>(color_table[i + a][j + b].redComponent);
				blue = up * static_cast<int32_t>(color_table[i + a][j + b].blueComponent);
				grean = up * static_cast<int32_t>(color_table[i + a][j + b].greenComponent);
				count = up * 1 + count;
			}
		}
	}
	for (int32_t q = -1; q < 2; q+=2)
	{
		if (exist(i, j + q) && color_table[i][j + q].full == 1) { up = 50; }
		else { up = 5; }
		if (count != 0 && exist(i, j + q))
		{
			red += up * static_cast<int32_t>(color_table[i][j + q].redComponent);
			blue += up * static_cast<int32_t>(color_table[i][j + q].blueComponent);
			grean += up * static_cast<int32_t>(color_table[i][j + q].greenComponent);
			count = up * 1 + count;
		}
		if (count == 0 && exist(i, j + q))
		{
			red = up * static_cast<int32_t>(color_table[i][j + q].redComponent);
			blue = up * static_cast<int32_t>(color_table[i][j + q].blueComponent);
			grean = up * static_cast<int32_t>(color_table[i][j + q].greenComponent);
			count = up * 1 + count;
		}
		if (exist(i+q, j) && color_table[i+q][j].full == 1) { up = 50; }
		else { up = 5; }
		if (count != 0 && exist(i + q, j))
		{
			red += up * static_cast<int32_t>(color_table[i + q][j].redComponent);
			blue += up * static_cast<int32_t>(color_table[i + q][j].blueComponent);
			grean += up * static_cast<int32_t>(color_table[i + q][j].greenComponent);
			count = up * 1 + count;
		}
		if (count == 0 && exist(i+q, j)) 
		{
			red = up * static_cast<int32_t>(color_table[i + q][j].redComponent);
			blue = up * static_cast<int32_t>(color_table[i + q][j].blueComponent);
			grean = up * static_cast<int32_t>(color_table[i + q][j].greenComponent);
			count = up * 1 + count;
		}
	}
	red = red / count; blue = blue / count; grean = grean / count;
	color_table[i][j].blueComponent = static_cast<int8_t>(blue);
	color_table[i][j].greenComponent = static_cast<int8_t>(grean);
	color_table[i][j].redComponent= static_cast<int8_t>(red);
	color_table[i][j].full = 2;
}*/
void Picture::calculate_color(uint32_t i, uint32_t j)
{
	NEIGHBORHOOD near;
	near.fir = near.sec = near.thi = near.fou = near.fif = full_NULL();
	uint8_t count = 0, parts = 0, help = 0;
	for (int32_t b = -1; b < 2; b += 2)
	{
		if (count == 0) {
			if (exist(i, j + b)) { count++; near.fir = color_table[i][j + b]; near.fir.full = 1; }
			if (exist(i + b, j)) { count++; near.fir = color_table[i + b][j]; near.fir.full = 1; }
		}
		else {
			if (exist(i, j + b)) { count++; near.fir = sum_pix(near.fir, color_table[i][j + b], 1); near.fir.full = 1; }
			if (exist(i + b, j)) { count++; near.fir = sum_pix(near.fir, color_table[i + b][j], 1); near.fir.full = 1; }
		}
	}
	if (count != 0) { near.fir = div_pix(near.fir, count); }
	count = 0;
	for (int32_t a = -1; a < 2; a += 2)
	{
		for (int32_t b = -1; b < 2; b += 2)
		{
			if (count == 0) {
				if (exist(i + a, j + b)) { count++; near.sec = color_table[i + a][j + b]; }
			}
			else {
				if (exist(i + a, j + b)) { count++; near.sec = sum_pix(near.sec, color_table[i + a][j + b], 1); near.sec.full = 1; }
			}
		}
	}
	if (count != 0) { near.sec = div_pix(near.sec, count); }
	count = 0;
	for (int32_t b = -2; b < 3; b += 4)
	{
		if (count == 0) {
			if (exist(i, j + b)) { count++; near.thi = color_table[i][j + b]; near.thi.full = 1; }
			if (exist(i + b, j)) { count++; near.thi = color_table[i + b][j]; near.thi.full = 1; }
		}
		else {
			if (exist(i, j + b)) { count++; near.thi = sum_pix(near.thi, color_table[i][j + b], 1); near.thi.full = 1; }
			if (exist(i + b, j)) { count++; near.thi = sum_pix(near.thi, color_table[i + b][j], 1); near.thi.full = 1; }
		}
	}
	if (count != 0) { near.thi = div_pix(near.thi, count); }
	count = 0;
	for (int32_t a = -2; a < 3; a += 4)
	{
		for (int32_t b = -2; b < 3; b += 4)
		{
			if (count == 0) {
				if (exist(i + a, j + b)) {
					count++; near.fif = color_table[i + a][j + b]; near.fif.full = 1;
				}
			}
			else {
				if (exist(i + a, j + b)) { count++; near.fif = sum_pix(near.fif, color_table[i + a][j + b], 1); near.fif.full = 1; }
			}
		}
	}
	if (count != 0) { near.fif = div_pix(near.fif, count); }
	count = 0;
	for (int32_t a = -2; a < 3; a += 4)
	{
		for (int32_t b = -1; b < 2; b += 2)
		{
			if (count == 0) {
				if (exist(i + a, j + b)) { count++; near.fou = color_table[i + a][j + b]; near.fou.full = 1; }
				if (exist(i + b, j + a)) { count++; near.fou = color_table[i + b][j + a]; near.fou.full = 1; }
			}
			else {
				if (exist(i + a, j + b)) { count++; near.fou = sum_pix(near.fou, color_table[i + a][j + b], 1); near.fou.full = 1; }
				if (exist(i + b, j + a)) { count++; near.fou = sum_pix(near.fou, color_table[i + b][j + a], 1); near.fou.full = 1; }
			}
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
	//cout << (int)color_table[i][j].greenComponent << endl;
//	cout << static_cast<int>(color_table[i][j].blueComponent) <<endl;
	//color_table[i][j] = div_pix(color_table[i][j], parts);
	color_table[i][j].full = 1;
}
PIXELDATA Picture::div_pix(PIXELDATA pix, uint8_t count)
{
	pix.blueComponent = pix.blueComponent / count;
	pix.greenComponent =  pix.greenComponent / count;
	pix.redComponent = pix.redComponent / count;
	return pix;
}
PIXELDATA Picture::sum_pix(PIXELDATA pix1, PIXELDATA pix2, uint8_t a)
{
	PIXELDATA pix3;
	pix3.blueComponent = static_cast<int8_t>(pix1.blueComponent + a * pix2.blueComponent);
	pix3.greenComponent = static_cast<int8_t>( pix1.greenComponent + a * pix2.greenComponent);
	pix3.redComponent = static_cast<int8_t> (pix1.redComponent + a * pix2.redComponent);
	return pix3;
}

bool Picture::is_empty_(PIXELDATA pix)
{
	if (pix.full==0) { return true; }
	else { return false; }
}

bool Picture::exist(uint32_t i, uint32_t j)
{
	if (i >= 0 && j >= 0 && i < new_sizeY && j < new_sizeX && color_table[i][j].full == 1) { return true; }
	else { return false; }
}

PIXELDATA Picture::full_NULL()
{
	PIXELDATA pix;
	pix.redComponent = NULL;
	pix.greenComponent = NULL;
	pix.blueComponent = NULL;
	pix.full = 0;
	return pix;
}

void Picture::full_NULL_mas()
{
	for (int32_t i = 0; i < new_sizeY; i++)
	{
		for (int32_t j = 0; j < new_sizeX; j++)
		{
			color_table[i][j] = full_NULL();
		}
	}
}

void Picture::streatch()
{
	color_table = new PIXELDATA * [new_sizeY];

	for (int32_t a = 0; a < new_sizeY; a++)
	{
		color_table[a] = new PIXELDATA[new_sizeX];
	}
	 full_NULL_mas();
	for (int32_t i = 0; i < bmpFile.old_sizeY; i++)
	{
		for (int32_t j = 0; j < bmpFile.old_sizeX; j++)
		{
			color_table[static_cast<int32_t>(i * argument)][static_cast<int32_t>(j * argument)] = initial_table[i][j];
			color_table[static_cast<int32_t>(i * argument)][static_cast<int32_t>(j * argument)].full = 1;
		}
	}
}

double str_to_int(string help)
{
	double a = 0, numer = 10, tail = 1, power = 1;
	for (int i = 0; i < help.size(); i++)
	{
		if (help[i] != '.' && help[i] != ',') { a = a * numer + (static_cast<int8_t>(help[i]) - 48) * pow(tail, power); power++; }
		else { numer = 1; tail = 0.1; power = 1; }
	}
	return a;
}

int main(int argc, char* argv[])//запускать з консолі і пишеш щось типу !! OP_LR4_Home.exe(назві проекту) data.txt data3.txt 234,567(число збільшення, можна і через точку)!!
{
	setlocale(LC_ALL, "Russian");
	double plusultra = 1;//в скільки разів збільшувати
	string inFile, outFile, help;//назви вхідного та вихідного файлів, допоміжний рядок для зчитування збільшення
	if (argc > 1)
	{
		inFile = argv[1]; cout << inFile << endl;
		outFile = argv[2]; cout << outFile << endl;
		help = argv[3];
		plusultra = str_to_int(help); cout << plusultra << endl;
	}
	Picture my_pic("bmp.bmp", "bmp1.bmp", 5);
	my_pic.resize_picture();
}