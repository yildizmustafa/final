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

// Global degiskenler haar sýnýflandýrýcýlar xml dosyalarý proje ana dizinine eklendi
String yuzCascade_xml = "haarcascade_frontalface_alt.xml";
String gozCascade_xml = "haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier yuzCascade;
CascadeClassifier gozCascade;

//pencere adý
string pencere = "Bitirme Ödevi|Sürücü Uyku Tespiti";

RNG rng(12345);

/************************************************************************************************
Bir sayac deðiþkeni oluþturuldu.
Yüz ve göz tanýma yapýlabilmesi için gerekli olan sýnýflandýrýcýlar tanýmlandý.
Webcam'dan görüntü alýnmaya baþlandý ve alýnan her bir frame, frame deðiþkenine aktarýldý. Video akýþý saðlandý.
Gözler kapalý olduðunda sayaç dizisi 1'er artar. Ve kullanýcýya mesaj gönderilir.
***************************************************************************************************/

int main(int argc, const char** argv)
{
	//VideoCapture* capture;
	//VideoCapture capture;
	Mat frame;

	namedWindow(pencere, CV_WINDOW_AUTOSIZE);

	//resizeWindow(pencere, 1280, 960);
	


	//Cascade sýnýflandýrýcýlarý yüklendi
	if (!yuzCascade.load(yuzCascade_xml))
	{
		printf("Yüz xml dosyasý yüklenemedi");
		return -1;
	};

	if (!gozCascade.load(gozCascade_xml))
	{
		printf("Göz xml dosyasý yüklenemedi");
		return -1;
	};

	//Kameradan gelen görüntüler teker teker frame deðiþkenine aktarýlýr

	VideoCapture capture(0);
	
	if (true)
	{
		const int epocs = 5;//5 snde bir tekrardan  etmesi için 
		int artis = -1;
		int i;
		int sayac[epocs]; //sayac kaç saniyede 1 tekrardan saymaya baþlayacak
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

			//sayac dizisi 5 deðerine ulaþýrsa kendini sýfýrlayacak.
			if (artis == epocs)
			{
				artis = 0;

				for (i = 0; i < epocs; i++)
				{
					sayac[i] = 0;
				}
			}

			//sýnýflandýrýyý uygulama
			if (!frame.empty())
			{
				uykuTespit(frame, artis, sayac);
			}
			else
			{
				printf(" Görüntü Alýnamýyor!");
				break;
			}

			if (artis == 0)//gözün algýlanamama durumu
			{
				if (sayac[artis] == 1)
				{
					kapali = kapali + 1;
					printf("\Goz Kapali Gecen Sure = %d\n", kapali);
				}
			}

			else if (artis > 0)//gözðn kapalý olmasý durumu
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

				else if (sayac[artis] == 0)//gözün açýk olma durumu
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
uykuTespit fonksiyonunda gelen framelerden yüz ve gözleri algýlar. Algýlanan gözleri bir daire
içerisine alýr. Gözler açýk olarak algýlanýrsa sayaç deðiþkenine 0 atanýr. Gözlerin kapalý olduðu
tespit edilirse sayaç dizisinin indexsine 1 atanýr.
**************************************************************************************/

void uykuTespit(Mat frame, int artis, int sayac[])
{
	std::vector<Rect> faces;
	Mat frame_gray;

	cvtColor(frame, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);


	//gelen frameden yüz tespiti
	yuzCascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

	for (size_t i = 0; i < faces.size(); i++)
	{
		Mat yuzroi = frame_gray(faces[i]);
		std::vector<Rect> eyes;

		//Bulunan yüz içerisinden gözlerin tespiti
		gozCascade.detectMultiScale(yuzroi, eyes, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

		
		for (size_t j = 0; j < eyes.size(); j++)
		{
			Point center(faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5);
			int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);
			circle(frame, center, radius, Scalar(255, 0, 0), 4, 8, 0);
		}

		/*
		size() 0dan büyükse gözler açýktýr. sayaç dizisi 0dýr.
		gözler kapalý ise sayac dizisinin ilgili indexi 1 olarak güncellenir.
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