// bitirme_guncelhali.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <fstream>


using namespace std;
using namespace cv;

void uykuTespit(Mat frame, int artis, int sayac[]);

// Global degiskenler haar s�n�fland�r�c�lar xml dosyalar� proje ana dizinine eklendi
String yuzCascade_xml = "haarcascade_frontalface_alt.xml";
String gozCascade_xml = "haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier yuzCascade;
CascadeClassifier gozCascade;

//pencere ad�
string pencere = "Bitirme �devi|S�r�c� Uyku Tespiti";

RNG rng(12345);

/************************************************************************************************
Bir sayac de�i�keni olu�turuldu.
Y�z ve g�z tan�ma yap�labilmesi i�in gerekli olan s�n�fland�r�c�lar tan�mland�.
Webcam'dan g�r�nt� al�nmaya ba�land� ve al�nan her bir frame, frame de�i�kenine aktar�ld�. Video ak��� sa�land�.
G�zler kapal� oldu�unda saya� dizisi 1'er artar. Ve kullan�c�ya mesaj g�nderilir.
***************************************************************************************************/

int main(int argc, const char** argv)
{
	//VideoCapture* capture;
	//VideoCapture capture;
	Mat frame;

	namedWindow(pencere, CV_WINDOW_AUTOSIZE);

	//resizeWindow(pencere, 1280, 960);
	


	//Cascade s�n�fland�r�c�lar� y�klendi
	if (!yuzCascade.load(yuzCascade_xml))
	{
		printf("Y�z xml dosyas� y�klenemedi");
		return -1;
	};

	if (!gozCascade.load(gozCascade_xml))
	{
		printf("G�z xml dosyas� y�klenemedi");
		return -1;
	};

	//Kameradan gelen g�r�nt�ler teker teker frame de�i�kenine aktar�l�r

	VideoCapture capture(0);
	
	if (true)
	{
		const int epocs = 5;//5 snde bir tekrardan  etmesi i�in 
		int artis = -1;
		int i;
		int sayac[epocs]; //sayac ka� saniyede 1 tekrardan saymaya ba�layacak
		int kapali = 0;//

		//
		for (i = 0; i < epocs; i++)
		{
			sayac[i] = 0;
		}

		while (true)
		{
			bool videooku = capture.read(frame);

			artis++;

			//sayac dizisi 5 de�erine ula��rsa kendini s�f�rlayacak.
			if (artis == epocs)
			{
				artis = 0;

				for (i = 0; i < epocs; i++)
				{
					sayac[i] = 0;
				}
			}

			//s�n�fland�r�y� uygulama
			if (!frame.empty())
			{
				uykuTespit(frame, artis, sayac);
			}
			else
			{
				printf(" G�r�nt� Al�nam�yor!");
				break;
			}

			if (artis == 0)//g�z�n alg�lanamama durumu
			{
				if (sayac[artis] == 1)
				{
					kapali = kapali + 1;
					printf("\Goz Kapali Gecen Sure = %d\n", kapali);
				}
			}

			else if (artis > 0)//g�z�n kapal� olmas� durumu
			{
				if (sayac[artis] == 1)
				{
					if ((sayac[artis - 1] == 1) || (sayac[artis - 1] == 0))
					{
						kapali = kapali + 1;
						printf("\Goz Kapali Gecen Sure = %d\n", kapali);
						if (kapali >= 1)
						{
							kapali = 0;
							printf("Uyku Tespit edildi\n");
							cout << "\7";//alarm ver
						}
					}
				}

				else if (sayac[artis] == 0)//g�z�n a��k olma durumu
				{
					if (kapali < 5)
					{
						kapali = 0;
						printf("\Gozler acik\n", kapali);
						
					}
					else if (kapali >= 5)
					{
						kapali = 0;
						printf("yola odaklanin\n");
					}
				}
			}
			int c = waitKey(10);
		}
	}
	
	return 0;
}

/***********************************************************************************
uykuTespit fonksiyonunda gelen framelerden y�z ve g�zleri alg�lar. Alg�lanan g�zleri bir daire
i�erisine al�r. G�zler a��k olarak alg�lan�rsa saya� de�i�kenine 0 atan�r. G�zlerin kapal� oldu�u
tespit edilirse saya� dizisinin indexsine 1 atan�r.
**************************************************************************************/

void uykuTespit(Mat frame, int artis, int sayac[])
{
	std::vector<Rect> faces;
	Mat frame_gray;

	cvtColor(frame, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);


	//gelen frameden y�z tespiti
	yuzCascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

	for (size_t i = 0; i < faces.size(); i++)
	{
		Mat yuzroi = frame_gray(faces[i]);
		std::vector<Rect> eyes;

		//Bulunan y�z i�erisinden g�zlerin tespiti
		gozCascade.detectMultiScale(yuzroi, eyes, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

		
		for (size_t j = 0; j < eyes.size(); j++)
		{
			Point center(faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5);
			int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);
			circle(frame, center, radius, Scalar(255, 0, 0), 4, 8, 0);
		}

		/*
		size() 0dan b�y�kse g�zler a��kt�r. saya� dizisi 0d�r.
		g�zler kapal� ise sayac dizisinin ilgili indexi 1 olarak g�ncellenir.
		*/
		if (eyes.size() > 0)
		{
			sayac[artis] = 0;
		}

		else
		{
			sayac[artis] = 1;
		}
	}


	imshow(pencere, frame);
}