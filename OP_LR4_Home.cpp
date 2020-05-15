#include <iostream>
#include <cstring>
#include <cmath>

using namespace std;
double str_to_int(string help);

typedef struct
{
	int8_t id[2];            // Завжди дві літери 'B' і 'M' //ще мають бути дані про біт з якого починається запис кольорів і довжина двох хедерів, нам наче лише дані з першого потрібні, тому можна перескочити а потім копірнути всі початкові дані з невеликими замінами
	int32_t filesize;        // Розмір файла в байтах                                   NEED
	int16_t reserved[2] = { 0,0 };     // 0, 0
	int32_t headersize = 54L;      // 54L для 24-бітних зображень
	int32_t infoSize = 40L;        // 40L для 24-бітних зображень
	int32_t width;           // ширина зображення в пікселях                            NEED
	int32_t hight;           // висота зображення в пікселях                            NEED
	int16_t biPlanes = 1;        // 1 (для 24-бітних зображень)
	int16_t bits = 24;            // 24 (для 24-бітних зображень)
	int32_t biCompression = 0L;   // 0L
	int32_t biSizeImage = 0L;     // Можна поставити в 0L для зображень без компрессії (наш варіант)
	int32_t biXPelsPerMeter = 0L; // Рекомендована кількість пікселів на метр, можна 0L //ще там десь має бути розмір в "метрах", моживо він теж потрібний
	int32_t biYPelsPerMeter = 0L; // Те саме, по висоті
	int32_t biClrUsed = 0L;       // Для індексованих зображень, можна поставити 0L
	int32_t biClrImportant = 0L;  // Те саме
} BMPHEAD;
typedef struct
{
	int8_t redComponent;      //00 00 ff 00
	int8_t greenComponent;    //00 ff 00 00    00 00 00 ff - alpha// це вже записано в зворотньому порядку
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
	Picture(string inFile,string outFile,double augment);
	void resize_picture();//Функція доступна користувачу, просто для ініціалізації роботи
private:/*сюди потрібно функції: 
		1)зчитування даних з файлу 
		1.5)Сеттери і Геттери
		2)перепис масиву з кольорами пікселів в більший розмір  ---------------------Готово
		2.1) Можна попереднь заповнити цей масив кольорами NULL, щоб не біло траблів при зчитуванні + буде змога знайти "порожні" пікселі -----------------Готово
		3)сама інтерполяція. Я за бікубічну (чи взагалі білінійну, бо фільт Ланцоша я посеред ночі не врубаю). Вона виконується над всім масивом даних ------------------Готово
		4)Функція для запису даних назад в новий файл. Шапка змінюється лише в місці розміру файла та довжинах сторін
		5)https://www.cambridgeincolour.com/ru/tutorials-ru/image-interpolation.htm + https://www.youtube.com/watch?v=Y8EBRn4Bf70 (тут дуже добре він розказа що за чим йде тому нам бде легко знайти впотрібні дані)
	*/
	BMPHEAD bmpFile;
	int new_sizeY; //= static_cast<int>(bmpFile.hight * augment)
	int old_sizeY; //= bmpFile.hight
	int new_sizeX; //= static_cast<int>(bmpFile.width * augment)
	int old_sizeX; //= bmpFile.width
	PIXELDATA** color_table = new PIXELDATA* [new_sizeY];//це масив, в який ми будемо зберігати значення кольору для пікселів, він вже повністю ініціалізований, його чіпати не потрібно
	PIXELDATA** initial_table = new PIXELDATA * [old_sizeY];//це початковий масив з кольорами, його треба зчитати і ініціалізувати по Х
	double augment; //коефіціент збільшення
	string inFile, outFile; //імена файлів
	void bicubic_interpolation(); //бікубічна інтерполяція для всього масиву кольорів
	void streatch(); //створення масиву кольорів і його розширення
	PIXELDATA full_NULL(); //Заповнює 1 піксель для всіх каналів (окрім альфа) значенням НУУЛ
	void full_NULL_mas(); //Заповнює весь 2д масив пікселями зі значенням НУУЛ
	bool is_empty_(PIXELDATA pix); //Перевіряє чи даний піксель порожній 
	bool exist(int i, int j); //перевіряє доступність коефіцієнтів для переміщення по масиву
	PIXELDATA calculate_color(int i, int j); //рахує колір  для одного пікселя
	PIXELDATA sum_pix(PIXELDATA pix1, PIXELDATA pix2, int a); //сума значень кольорів двох пікселів, де 2ий може братися а разів
	PIXELDATA div_pix(PIXELDATA pix, int8_t count); //частка значення кольору пікселя та якогось числа (для знаходження середнього значення кольору)
};
Picture::Picture(string inFile, string outFile, double augment) 
{
	this->augment = augment; //в скільки разів збільшувати
	this->inFile = inFile;
	this->outFile = outFile;
}
void Picture::resize_picture()
{
	//тут зчитування данних, сеттери для хеддерів та масиву кольорів
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
				color_table[i][j] = calculate_color(i,j); 
			}
		}
	}
}
PIXELDATA Picture::calculate_color(int i, int j)
{
	NEIGHBORHOOD near;
	near.fir = near.sec = near.thi = near.fou = near.fif = full_NULL();
	int8_t count = 0, parts=0;
	for (int b = -1; b < 2; b += 2)
	{
		if (exist(i, j + b)) { count++; near.fir = sum_pix(near.fir, color_table[i][j + b],1); }
		if (exist(i + b, j)) { count++; near.fir = sum_pix(near.fir, color_table[i + b][j],1); }
	}
	if (count != 0) { near.sec = div_pix(near.fir, count); }
	count = 0;
	for (int a = -1; a < 2; a += 2)
	{
		for (int b = -1; b < 2; b += 2)
		{
			if (exist(i + a, j + b)) { count++; near.sec = sum_pix(near.sec,color_table[i + a][j + b],1); }
		}
	}
	if (count != 0) { near.sec = div_pix(near.sec, count); }
	count = 0;
	for (int b = -2; b < 3; b += 4)
	{
		if (exist(i, j + b)) { count++; near.thi = sum_pix(near.thi, color_table[i][j + b],1); }
		if (exist(i + b, j)) { count++; near.thi = sum_pix(near.thi, color_table[i + b][j],1); }
	}
	if (count != 0) { near.thi = div_pix(near.thi, count); }
	count = 0;
	for (int a = -2; a < 3; a += 4)
	{
		for (int b = -2; b < 3; b += 4)
		{
			if (exist(i + a, j + b)) { count++; near.fif = sum_pix(near.fif, color_table[i + a][j + b],1); }
		}
	}
	if (count != 0) { near.fif = div_pix(near.fif, count); }
	count = 0;
	for (int a = -2; a < 3; a += 4)
	{
		for (int b = -1; b < 2; b += 2)
		{
			if (exist(i + a, j + b)) { count++; near.fif = sum_pix(near.fif, color_table[i + a][j + b],1); }
			if (exist(i + b, j + a)) { count++; near.fif = sum_pix(near.fif, color_table[i + b][j + a],1); }
		}
	}
	if (count != 0) { near.fou = div_pix(near.fou, count); }
	count = 0;
	if (!is_empty_(near.fir))
	{color_table[i][j] = sum_pix(color_table[i][j], near.fir, 5); parts = parts + 5;}
	if (!is_empty_(near.sec))
	{color_table[i][j] = sum_pix(color_table[i][j], near.sec, 4); parts = parts + 4;}
	if (!is_empty_(near.thi))
	{color_table[i][j] = sum_pix(color_table[i][j], near.thi, 3); parts = parts + 3;}
	if (!is_empty_(near.fou))
	{color_table[i][j] = sum_pix(color_table[i][j], near.fou, 2); parts = parts + 2;}
	if (!is_empty_(near.fif))
	{color_table[i][j] = sum_pix(color_table[i][j], near.fif, 1); parts = parts + 1;}
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
	pix3.blueComponent=pix1.blueComponent +a* pix2.blueComponent;
	pix3.greenComponent= pix1.greenComponent +a* pix2.greenComponent;
	pix3.redComponent=pix1.redComponent +a* pix2.redComponent;
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
	for (int a = 0; a < old_sizeX; a++)
	{
		color_table[a]=new PIXELDATA [new_sizeX];
	}
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
	double a = 0, numer=10, tail=1, power=1;
	for (int i = 0; i < help.size(); i++)
	{
		if (help[i] != '.' && help[i] != ',') { a = a * numer + (help[i] - 48) * pow(tail, power); power++; }
		else { numer = 1; tail = 0.1; power = 1; }
	}
	return a;
}

int main(int argc, char* argv[])//запускать з консолі і пишеш щось типу !! OP_LR4_Home.exe(назві проекту) data.txt data3.txt 234,567(число збільшення, можна і через точку)!!
{
	double plusultra = 1;//в скільки разів збільшувати
	string inFile, outFile, help;//назви вхідного та вихідного файлів, допоміжний рядок для зчитування збільшення
	if (argc > 1)
	{
		inFile = argv[1]; cout << inFile << endl;
		outFile = argv[2]; cout << outFile << endl;
		help = argv[3]; plusultra = str_to_int(help); cout << plusultra << endl;
	}
	Picture my_pic(inFile, outFile, plusultra);
	my_pic.resize_picture();
}