//
// RK_Sabre.cpp
//
// affichage sur l'ecran de l'hote, sur sabre faire :
// export DISPLAY=192.168.1.1:0.0
// sur l'hote dans un terminal faire un xhost +
// sur sabre test avec  xeyes  par exemple doit s'afficher sur ecran de l'hote
//
// Compilation : 
// g++ `pkg-config opencv --cflags` RK_Sabre.cpp -o RK_Projet `pkg-config opencv --libs`
//
// pour trouver le /dev de la WebCam faire :
//   sudo rm /dev/video*
// brancher la WebCam
//   ls -lt /dev/web_cam0 
//   lrwxrwxrwx 1 root root 6 Jan 23 15:44 /dev/web_cam0 -> video0
//
// adapter la ligne :
//      capture = cvCreateFileCapture("/dev/web_cam0");
//  ou  capture = cvCreateCameraCapture( 0 );
//

/*
 * Code written by Vinz (GeckoGeek.fr) 14/03/2010
 * http://www.geckogeek.fr/lire-le-flux-dune-webcam-camera-video-avec-opencv.html
 * And modified by Rostom Kachouri; 16/02/2016
 * IN5E23
 */
 
#include <stdio.h>
#include <stdlib.h>
// OpenCV header
#include <bits/stdc++.h>
using namespace std;

#include "highgui.h"
#include "cv.h"
#define SEUIL 120

/*int intComparator ( const void * first, const void * second ) {
    int firstInt = * (const int *) first;
    int secondInt = * (const int *) second;
    return firstInt - secondInt;
}*/

#include <stdio.h>

int median(int t[], int size, int p) {
	int i, j, k, v;

	// Phase I.
	// on trie les p premieres valeurs
	for (i = 1; i < p; i++) {
		v = t[i];
		j = i;
		k = i - 1;
		while (k >= 0 && t[k] > v) {
			t[j] = t[k];
			j = k;
			k = k - 1;
		}
		t[j] = v;
	}

	// Phase II.
	// on parcourt le reste du tableau faisant en sorte
	// d'avoir les p plus petites valeurs du tableau triées
	// et dans les p premières cellules
	for (i = p; i < size; i++) {
		v = t[i];
		j = i;
		k = p - 1; // différence avec partie I.
		while (k >= 0 && t[k] > v) {
			t[j] = t[k];
			j = k;
			k = k - 1;
		}
		t[j] = v;
	}

	// La p-ème valeur se trouve à la bonne place.
	return t[p - 1];
}

double racineCarre(double n, float l)
{
    // En supposant que le carré de n n'est que n.
    double x = n;

    // L'estimation fermée sera stockée dans la racine.
    double root;
	
    int count=0;
    while (count<20) {
	count++;

        // Calcul du x le plus proche
        root = 0.5 * (x + (n / x));

        // Vérification de la proximité
        if (abs(root - x) < l)
            break;

        // Actualisation de root
        x = root;
    }
    return root;
}


int main()
{ 
    // Touche clavier pour quitter
    char ESC_keyboard;
	// intialisation de la variable ESC_keyboard
	ESC_keyboard = cvWaitKey(5);

    // Images
     IplImage *Image_IN;
     IplImage *Image_OUT;
     IplImage *Image_SOBELX; //image SOBELx
     IplImage *Image_SOBELY; //image SOBELy
     IplImage *Image_SOBEL; //image SOBEL
     IplImage *Image_MEDIAN; //image MEDIAN
	 
    // Capture vidéo
    CvCapture *capture;
	
	// variables images
	int height,width,step,channels;		// Taille de l'image
	uchar *Data_in;						// pointeur des données Image_IN
	uchar *Data_out;					// pointeur des données Image_OUT
	uchar *Data_sobelX;					// pointeur des données Image_SOBELx
	uchar *Data_sobelY;					// pointeur des données Image_SOBELy
	uchar *Data_sobel;					// pointeur des données Image_SOBEL
	uchar *Data_median;					// pointeur des données Image_MEDIAN
	int i,j,k,t,c,y;							// indices
	int matrice[9];
	double resultX, resultY, result;
	int resultSOBEL;
	float precision = 1;
 
    // Ouvrir le flux vidéo
    capture = cvCreateFileCapture("/home/linaro/Projet_sabre/notre_dame.png"); // chemin pour un fichier
    //capture = cvCreateFileCapture("/dev/v4l/by-id/usb-046d_HD_Pro_Webcam_C920_*");
    //capture = cvCreateFileCapture("/dev/v4l/by-path/platform-ci_hdrc.1-usb-0\:1.1\:1.0-video-index0");
    //capture = cvCreateFileCapture("/dev/web_cam0");
    //capture = cvCreateCameraCapture( CV_CAP_ANY );
    //capture = cvCreateCameraCapture( 4 );

    // Vérifier si l'ouverture du flux est ok
    if (!capture) {

	    printf("Ouverture du flux vidéo impossible !\n");
	    return 1;

    }

    // Définition des fenêtres
    cvNamedWindow("Image_IN_Window", CV_WINDOW_AUTOSIZE);   // Image_IN
    cvNamedWindow("Image_OUT_Window", CV_WINDOW_AUTOSIZE); 	// Image_OUT
    cvNamedWindow("Image_SOBELX_Window", CV_WINDOW_AUTOSIZE); 	// Image_SOBELX
    cvNamedWindow("Image_SOBELY_Window", CV_WINDOW_AUTOSIZE); 	// Image_SOBELY
    cvNamedWindow("Image_SOBEL_Window", CV_WINDOW_AUTOSIZE); 	// Image_SOBEL
    cvNamedWindow("Image_MEDIAN_Window", CV_WINDOW_AUTOSIZE); 	// Image_MEDIAN

    // Positionnement des fenêtres
    cvMoveWindow("Image_IN_Window", 0,0);
    cvMoveWindow("Image_OUT_Window", 0,500);
    cvMoveWindow("Image_SOBEL_Window", 1300,500);
    cvMoveWindow("Image_SOBELX_Window", 650,0);
    cvMoveWindow("Image_SOBELY_Window", 1300,0 );
    cvMoveWindow("Image_MEDIAN_Window", 650,500 );



    // Première acquisition
    Image_IN = cvQueryFrame(capture); 

    // Création de l'image de sortie
    Image_OUT = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1); 
    Image_SOBELX = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1); //Création image SobelX
    Image_SOBELY = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1); //Création image SobelY
    Image_SOBEL = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1); //Création image Sobel
    Image_MEDIAN = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1); //Création image Sobel	
    int step_gray = Image_OUT->widthStep/sizeof(uchar);

    // Boucle tant que l'utilisateur n'appuie pas sur la touche q (ou Q)
    while(ESC_keyboard != 'q' && ESC_keyboard != 'Q') {

	    // On récupère une Image_IN
	    //Image_IN = cvQueryFrame(capture);
	    // Dimension
	    height    = Image_IN->height;
	    width     = Image_IN->width;
	    // distance entre les deux premiers pixels de lignes successives
	    step      = Image_IN->widthStep;
	    channels  = Image_IN->nChannels;
	    // initialisation des pointeurs de donnée
	    Data_in      = (uchar *)Image_IN->imageData; 
	    Data_out = (uchar *) Image_OUT->imageData;
	    Data_sobelX = (uchar *) Image_SOBELX->imageData;
	    Data_sobelY = (uchar *) Image_SOBELY->imageData;
	    Data_sobel = (uchar *) Image_SOBEL->imageData;
	    Data_median = (uchar *) Image_MEDIAN->imageData;

	    //conversion RGB en niveau de gris
	    for(i=0;i<height;i++) 
		    for(j=0;j<width;j++)
		    { Data_out[i*step_gray+j]=0.114*Data_in[i*step+j*channels+0]+ 0.587*Data_in[i*step+j*channels+1] + 0.299*Data_in[i*step+j*channels+2];}

	    //CALCUL MEDIAN------------------------------------------------------------------------------------------- // On doit opti en parallélisant
	for (i=0; i<height; i++){ 
		    for(t=0; t<width; t++){
			 if (i==0 || t==0 || i==height-1 || t== width-1){Data_median[i*step_gray+t]=0;}
			    else{
				// On rempli un tableau avec la valeur de chacun des 9 pixels
				matrice[0]= Data_out[(i-1)*step_gray+t-1];
				matrice[1]= Data_out[(i-1)*step_gray+ t ];
				matrice[2]= Data_out[(i-1)*step_gray+t+1];
				matrice[3]= Data_out[  i  *step_gray+t-1];
				matrice[4]= Data_out[  i  *step_gray+ t ];
				matrice[5]= Data_out[  i  *step_gray+t+1];
				matrice[6]= Data_out[(i+1)*step_gray+t-1];
				matrice[7]= Data_out[(i+1)*step_gray+ t ];
				matrice[9]= Data_out[(i+1)*step_gray+t+1];

				//Tri du tableau

				Data_median[i*step_gray+t]=median(matrice, 9, 5);
				//qsort( matrice, 9, sizeof(int), intComparator ); // A opti avec le choix du tri du tableau 

				/*for (y=0; y<8; y++){
					for(j=i+1; j<9; j++){
						if (matrice[y]>matrice[j]){
							c = matrice[y];
							matrice[y]=matrice[j];
							matrice[j]=c;
						}
		    			}
				}*/
				//Data_median[i*step_gray+t]=matrice[4];
			}
		}
	}

	    //CALCUL SOBEL-------------------------------------------------------------------------------------------- //Il faudra opti en faissant en parallèle et en se souvenant des cases qu'on utilise plusieurs fois
	    for (i=0; i<height; i++){ 
		    for(t=0; t<width; t++){
			    if (i==0 || t==0 || i==height-1 || t== width-1){Data_sobel[i*step_gray+t]=0;} // IL faudra optimiser les effets de bord
			    else{
				     resultX=
					     Data_median[(i-1)*step_gray+t-1]
					    +Data_median[  i  *step_gray+t-1]*2
					    +Data_median[(i+1)*step_gray+t-1]
					    -(Data_median[(i-1)*step_gray+t+1]
					    +Data_median[  i  *step_gray+t+1]*2
					    +Data_median[(i+1)*step_gray+t+1]);
				    
					if (resultX >= SEUIL) {Data_sobelX[i*step_gray+t]=255;}
					else {Data_sobelX[i*step_gray+t]=0;}
			   
				    resultY=
					     Data_median[(i-1)*step_gray+t-1]
					    -Data_median[(i+1)*step_gray+t-1]
					    +(Data_median[(i-1)*step_gray+ t ]
					    +Data_median[(i+1)*step_gray+ t ]*-1)*2
					    +Data_median[(i-1)*step_gray+t+1]
					    -Data_median[(i+1)*step_gray+t+1];
					
					if (resultY >= SEUIL) {Data_sobelY[i*step_gray+t]=255;}
					else {Data_sobelY[i*step_gray+t]=0;}

				    result = resultX*resultX+resultY*resultY;

				    resultSOBEL=
					    racineCarre(result, precision); //Il faudra optimiser le calcul de la racine carré

					if (resultSOBEL >= SEUIL) {Data_sobel[i*step_gray+t]=255;}
					else {Data_sobel[i*step_gray+t]=0;}
			    }
		    }
	    }
	    //---------------------------------------------------------------------------------------------------------

	    // On affiche l'Image_IN
	    cvShowImage( "Image_IN_Window", Image_IN);
	    // On affiche l'Image_OUT
	    cvShowImage( "Image_OUT_Window", Image_OUT);
	    // On affiche l'Image_SOBELX
	    cvShowImage( "Image_SOBELX_Window", Image_SOBELX);
	    // On affiche l'Image_SOBELY
	    cvShowImage( "Image_SOBELY_Window", Image_SOBELY);
	    // On affiche l'Image_SOBEL 
	    cvShowImage( "Image_SOBEL_Window", Image_SOBEL);
	    // On affiche l'Image_MEDIAN 
	    cvShowImage( "Image_MEDIAN_Window", Image_MEDIAN);

	    // On attend 5ms
	    ESC_keyboard = cvWaitKey(5);

    }

    // Fermeture de l'acquisition Vidéo
    cvReleaseCapture(&capture);

    // Fermeture des fenêtres d'affichage
    cvDestroyWindow("Image_IN_Window");
    cvDestroyWindow("Image_OUT_Window");
    cvDestroyWindow("Image_SOBELX_Window");  //Destruction image SOBELX
    cvDestroyWindow("Image_SOBELY_Window");  //Destruction image SOBELY
    cvDestroyWindow("Image_SOBEL_Window");  //Destruction image SOBEL
    cvDestroyWindow("Image_MEDIAN_Window");  //Destruction image MEDIAN

    return 0;

}
