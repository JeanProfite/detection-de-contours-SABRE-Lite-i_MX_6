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
	 
    // Capture vidéo
    CvCapture *capture;
	
	// variables images
	int height,width,step,channels;		// Taille de l'image
	uchar *Data_in;						// pointeur des données Image_IN
	uchar *Data_out;					// pointeur des données Image_OUT
	uchar *Data_sobelX;					// pointeur des données Image_SOBELx
	uchar *Data_sobelY;					// pointeur des données Image_SOBELy
	uchar *Data_sobel;					// pointeur des données Image_SOBEL
	int i,j,k,t;							// indices
 
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
    cvNamedWindow("Image_IN_Window", CV_WINDOW_AUTOSIZE);   // Image_IN
    cvNamedWindow("Image_OUT_Window", CV_WINDOW_AUTOSIZE); 	// Image_OUT
    cvNamedWindow("Image_SOBELX_Window", CV_WINDOW_AUTOSIZE); 	// Image_SOBELX
    cvNamedWindow("Image_SOBELY_Window", CV_WINDOW_AUTOSIZE); 	// Image_SOBELY
    cvNamedWindow("Image_SOBEL_Window", CV_WINDOW_AUTOSIZE); 	// Image_SOBEL

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
    int step_gray = Image_OUT->widthStep/sizeof(uchar);

    // Boucle tant que l'utilisateur n'appuie pas sur la touche q (ou Q)
    while(ESC_keyboard != 'q' && ESC_keyboard != 'Q') {

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

	    //conversion RGB en niveau de gris
	    for(i=0;i<height;i++) 
		    for(j=0;j<width;j++)
		    { Data_out[i*step_gray+j]=0.114*Data_in[i*step+j*channels+0]+ 0.587*Data_in[i*step+j*channels+1] + 0.299*Data_in[i*step+j*channels+2];}

	    //CALCUL SOBEL--------------------------------------------------------------------------------------------
	    for (i=0; i<height; i++){ 
		    for(t=0; t<width; t++){
			    if (i==0 || t==0 || i==height-1 || t== width-1){Data_sobelX[i*step_gray+t]=0;}
			    else{
				    Data_sobelX[i*step_gray+t]=
					     Data_out[(i-1)*step_gray+t-1]*-1
					    +Data_out[(i-1)*step_gray+t  ]*0
					    +Data_out[(i-1)*step_gray+t+1]*1
					    +Data_out[  i  *step_gray+t-1]*-2
					    +Data_out[  i  *step_gray+t  ]*0
					    +Data_out[  i  *step_gray+t+1]*2
					    +Data_out[(i+1)*step_gray+t-1]*-1
					    +Data_out[(i+1)*step_gray+t  ]*0
					    +Data_out[(i+1)*step_gray+t+1]*1;
			    }
		    }
	    }

	    for (i=0; i<height; i++){ 
		    for(t=0; t<width; t++){
			    if (i==0 || t==0 || i==height-1 || t==width-1){Data_sobelY[i*step_gray+t]=0;}
			    else{
				    Data_sobelY[i*step_gray+t]=
					     Data_out[(i-1)*step_gray+t-1]*-1
					    +Data_out[(i-1)*step_gray+t  ]*-2
					    +Data_out[(i-1)*step_gray+t+1]*-1
					    +Data_out[  i  *step_gray+t-1]*0
					    +Data_out[  i  *step_gray+t  ]*0
					    +Data_out[  i  *step_gray+t+1]*0
					    +Data_out[(i+1)*step_gray+t-1]*1
					    +Data_out[(i+1)*step_gray+t  ]*2
					    +Data_out[(i+1)*step_gray+t+1]*1;
			    }
		    }
	    }

	    for (i=0; i<height; i++){ 
		    for(t=0; t<width; t++){
			    if (i==0 || t==0 || i==height-1 || t==width-1){Data_sobel[i*step_gray+t]=0;}
			    else{
				    Data_sobel[i*step_gray+t]=
					    sqrt(Data_sobelX[i*step_gray+t]*Data_sobelX[i*step_gray+t]
						+Data_sobelY[i*step_gray+t]*Data_sobelY[i*step_gray+t]);
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
	    // On affiche l'Image_SOBEL
	    cvShowImage( "Image_SOBELY_Window", Image_SOBELY);
	    // On affiche l'Image_SOBEL 
	    cvShowImage( "Image_SOBEL_Window", Image_SOBEL);

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

    return 0;

}
