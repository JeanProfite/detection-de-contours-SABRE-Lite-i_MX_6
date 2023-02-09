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

#include "highgui.h"
#include "cv.h"
#include <unistd.h>
#define SEUIL 100

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
	int height,width,step,channels;				// Taille de l'image
	uchar *Data_in;						// pointeur des données Image_IN
	uchar *Data_out;					// pointeur des données Image_OUT
	uchar *Data_sobelX;					// pointeur des données Image_SOBELx
	uchar *Data_sobelY;					// pointeur des données Image_SOBELy
	uchar *Data_sobel;					// pointeur des données Image_SOBEL
	uchar *Data_median;					// pointeur des données Image_MEDIAN
	int i,j,k,t,c,y;					// indices

//conversion RGB en niveau de gris
void CalculNiveauGris(int step_gray){
	    for(int i=0;i<height;i++) 
		    for(int j=0;j<width;j++)
		    { Data_out[i*step_gray+j]=0.114*Data_in[i*step+j*channels+0]+ 0.587*Data_in[i*step+j*channels+1] + 0.299*Data_in[i*step+j*channels+2];}
}

//CALCUL MEDIAN-------------------------------------------------------------------------------------------
void CalculMedian(int step_gray){
int matrice[9];

for (int i=0; i<height; i++){ 
		    for(int t=0; t<width; t++){
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
				matrice[8]= Data_out[(i+1)*step_gray+t+1];

				//Tri du tableau
				for (int y=0; y<8; y++){
					for(int j=i+1; j<9; j++){
						if (matrice[y]>matrice[j]){
							c = matrice[y];
							matrice[y]=matrice[j];
							matrice[j]=c;
						}
		    			}
				}
				Data_median[i*step_gray+t]=matrice[4];
			}
		}
	}
}

//CALCUL SOBEL--------------------------------------------------------------------------------------------
void CalculSobel(int step_gray){
		int resultX[height*width];
		int resultY[height*width];
		int resultSOBEL[height*width];

	    for (int i=0; i<height; i++){ 
		    for(int t=0; t<width; t++){
			    if (i==0 || t==0 || i==height-1 || t== width-1){Data_sobelX[i*step_gray+t]=0;}
			    else{
				    resultX[i*step_gray+t]=
					     Data_median[(i-1)*step_gray+t-1]*-1
					    +Data_median[(i-1)*step_gray+t  ]*0
					    +Data_median[(i-1)*step_gray+t+1]*1
					    +Data_median[  i  *step_gray+t-1]*-2
					    +Data_median[  i  *step_gray+t  ]*0
					    +Data_median[  i  *step_gray+t+1]*2
					    +Data_median[(i+1)*step_gray+t-1]*-1
					    +Data_median[(i+1)*step_gray+t  ]*0
					    +Data_median[(i+1)*step_gray+t+1]*1;
				    
					if (resultX[i*step_gray+t] >= SEUIL) {Data_sobelX[i*step_gray+t]=255;}
					else if (resultX[i*step_gray+t] < SEUIL) {Data_sobelX[i*step_gray+t]=0;}
			    }
		    }
	    }

	    for (int i=0; i<height; i++){ 
		    for(int t=0; t<width; t++){
			    if (i==0 || t==0 || i==height-1 || t==width-1){Data_sobelY[i*step_gray+t]=0;}
			    else{
				    resultY[i*step_gray+t]=
					     Data_median[(i-1)*step_gray+t-1]*-1
					    +Data_median[(i-1)*step_gray+t  ]*-2
					    +Data_median[(i-1)*step_gray+t+1]*-1
					    +Data_median[  i  *step_gray+t-1]*0
					    +Data_median[  i  *step_gray+t  ]*0
					    +Data_median[  i  *step_gray+t+1]*0
					    +Data_median[(i+1)*step_gray+t-1]*1
					    +Data_median[(i+1)*step_gray+t  ]*2
					    +Data_median[(i+1)*step_gray+t+1]*1;
					
					if (resultY[i*step_gray+t] >= SEUIL) {Data_sobelY[i*step_gray+t]=255;}
					else if (resultY[i*step_gray+t] < SEUIL) {Data_sobelY[i*step_gray+t]=0;}

			    }
		    }
	    }

	    for (int i=0; i<height; i++){ 
		    for(int t=0; t<width; t++){
			    if (i==0 || t==0 || i==height-1 || t==width-1){Data_sobel[i*step_gray+t]=0;}
			    else{
				    resultSOBEL[i*step_gray+t]=
					    sqrt(resultX[i*step_gray+t]*resultX[i*step_gray+t]
						+resultY[i*step_gray+t]*resultY[i*step_gray+t]);

					if (resultSOBEL[i*step_gray+t] >= SEUIL) {Data_sobel[i*step_gray+t]=255;}
					else if (resultSOBEL[i*step_gray+t] < SEUIL) {Data_sobel[i*step_gray+t]=0;}
			    }
		    }
	    }
}
//---------------------------------------------------------------------------------------------------------

int main(){

printf("Version 0\n"); 

char ESC_keyboard;
	// intialisation de la variable ESC_keyboard
	ESC_keyboard = cvWaitKey(5);

    // Ouvrir le flux vidéo
    //capture = cvCreateFileCapture("/path/to/your/video/test.avi"); // chemin pour un fichier
    //capture = cvCreateFileCapture("/dev/v4l/by-id/usb-046d_HD_Pro_Webcam_C920_*");
    //capture = cvCreateFileCapture("/dev/v4l/by-path/platform-ci_hdrc.1-usb-0\:1.1\:1.0-video-index0");
    //capture = cvCreateFileCapture("/dev/web_cam0");
    capture = cvCreateCameraCapture( CV_CAP_ANY );
    //capture = cvCreateCameraCapture( 4 );

    // Vérifier si l'ouverture du flux est ok
    if (!capture) {

	    printf("Ouverture du flux vidéo impossible !\n");
	    return 1;

    }

    // Définition des fenêtres
    cvNamedWindow("Image_IN_Window", CV_WINDOW_AUTOSIZE);       // Image_IN
    cvNamedWindow("Image_OUT_Window", CV_WINDOW_AUTOSIZE); 	// Image_OUT
    cvNamedWindow("Image_SOBELX_Window", CV_WINDOW_AUTOSIZE); 	// Image_SOBELX
    cvNamedWindow("Image_SOBELY_Window", CV_WINDOW_AUTOSIZE); 	// Image_SOBELY
    cvNamedWindow("Image_SOBEL_Window", CV_WINDOW_AUTOSIZE); 	// Image_SOBEL
    cvNamedWindow("Image_MEDIAN_Window", CV_WINDOW_AUTOSIZE); 	// Image_MEDIAN

    // Positionnement des fenêtres
    cvMoveWindow("Image_IN_Window", 0,0);
    cvMoveWindow("Image_OUT_Window", 0,500);
    cvMoveWindow("Image_SOBEL_Window", 975,500);
    cvMoveWindow("Image_SOBELX_Window", 650,0);
    cvMoveWindow("Image_SOBELY_Window", 1300,0 );

    // Première acquisition
    Image_IN = cvQueryFrame(capture); 

    // Création de l'image de sortie
    Image_OUT = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1); 
    Image_SOBELX = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1); //Création image SobelX
    Image_SOBELY = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1); //Création image SobelY
    Image_SOBEL = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1); //Création image Sobel
    Image_MEDIAN = cvCreateImage(cvSize(Image_IN->width,Image_IN->height),  IPL_DEPTH_8U, 1); //Création image Sobel	
    int step_gray = Image_OUT->widthStep/sizeof(uchar);


    for(int frame = 0; frame < 150; frame++){

	    // On récupère une Image_IN
	    Image_IN = cvQueryFrame(capture);
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

	    
		CalculNiveauGris(step_gray);
		CalculMedian(step_gray);
		CalculSobel(step_gray);


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