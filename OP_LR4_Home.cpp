#include <iostream>
#include <cstring>
#include <cmath>

using namespace std;
double str_to_int(string help);

int main(int argc, char* argv[])//запускать з консолі і пишеш щось типу !! OP_LR4_Home.exe(назві проекту) data.txt data3.txt 234,567(число збільшення, можна і через точку)!!
{
	double plusultra = 1;//в скільки разів збільшувати
	string inFile, outFile, help;//назви вхідного та вихідного файлів, допоміжний рядок для зчитування збільшення
	if(argc>1)
	{	inFile = argv[1]; cout << inFile << endl;
		outFile = argv[2]; cout << outFile << endl; 
		help = argv[3]; plusultra = str_to_int(help); cout << plusultra << endl;
	}
}
class Picture
{
public:
	Picture(string inFile,string outFile,double augment);
	void resize_picture();//Функція доступна користувачу, просто для ініціалізації роботи
private:/*сюди потрібно функції: 
		1)зчитування даних з файлу 
		2)перепис масиву з кольорами пікселів в більший розмір (хоча це можна робити одразу при записі (просто домножати індекс на збільшення і брати цілу частину))
		2.1) Можна попереднь заповнити цей масив кольорами NULL, щоб не біло траблів при зчитуванні + буде змога знайти "порожні" пікселі
		3)сама інтерполяція. Я за бікубічну (чи взагалі білінійну, бо фільт Ланцоша я посеред ночі не врубаю). Вона виконується над всім масивом даних
		4)Функція для запису даних назад в новий файл. Шапка змінюється лише в місці розміру файла та довжинах сторін
		5)https://www.cambridgeincolour.com/ru/tutorials-ru/image-interpolation.htm + https://www.youtube.com/watch?v=Y8EBRn4Bf70 (тут дуже добре він розказа що за чим йде тому нам бде легко знайти впотрібні дані)
	*/
	BMPHEAD bmpFile;
	int new_size; //= bmpFile.hight * augment
	PIXELDATA** color_table = new PIXELDATA* [new_size];//це масив, в який ми будемо зберігати значення кольору для пікселів і розмір його буде не зовсім в задану кількість раз більший (бо не ціла розміність не є можливою)
	double augment;
	string inFile, outFile;
};
Picture::Picture(string inFile, string outFile, double augment) 
{
	this->augment = augment; //в скільки разів збільшувати
	this->inFile = inFile;
	this->outFile = outFile;
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
	int8_t id[2];            // Завжди дві літери 'B' і 'M' //ще мають бути дані про біт з якого починається запис кольорів і довжина двох хедерів, нам наче лише дані з першого потрібні, тому можна перескочити а потім копірнути всі початкові дані з невеликими замінами
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
	int32_t biXPelsPerMeter=0L; // Рекомендована кількість пікселів на метр, можна 0L //ще там десь має бути розмір в "метрах", моживо він теж потрібний
	int32_t biYPelsPerMeter=0L; // Те саме, по висоті
	int32_t biClrUsed=0L;       // Для індексованих зображень, можна поставити 0L
	int32_t biClrImportant=0L;  // Те саме
} BMPHEAD;
typedef struct
{
	int8_t redComponent;      //00 00 ff 00
	int8_t greenComponent;    //00 ff 00 00    00 00 00 ff - alpha// це вже записано в зворотньому порядку
	int8_t blueComponent;     //ff 00 00 00
} PIXELDATA;