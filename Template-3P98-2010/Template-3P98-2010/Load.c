/* An example of how to read an image (img.tif) from file using freeimage and then
display that image using openGL's drawPixelCommand. Also allow the image to be saved
to backup.tif with freeimage and a simple thresholding filter to be applied to the image.
Conversion by Lee Rozema.
Added triangle draw routine, fixed memory leak and improved performance by Robert Flack (2008)
*/

#include <stdlib.h>


#ifdef __unix__
	#include <GL/freeglut.h>
#elif defined(_WIN32) || defined(WIN32)
	#include <freeglut.h>
#endif


#include <FreeImage.h>
#include <stdio.h>
#include <malloc.h>


//the pixel structure
typedef struct {
	GLubyte r, g, b;
} pixel;

//the global structure
typedef struct {
	pixel *data;
	int w, h;
} glob;
glob global;

//read image
pixel *read_img(char *name, int *width, int *height) {
	FIBITMAP *image;
	int i,j,pnum;
	RGBQUAD aPixel;
	pixel *data;

	if((image = FreeImage_Load(FIF_TIFF, name, 0)) == NULL) {
		return NULL;
	}      
	*width = FreeImage_GetWidth(image);
	*height = FreeImage_GetHeight(image);

	//data = (pixel *)malloc((*height)*(*width)*sizeof(pixel *));
	data = (pixel *)malloc((*height)*(*width)*sizeof(pixel));
	pnum=0;
	for(i = 0 ; i < (*height) ; i++) {
		for(j = 0 ; j < (*width) ; j++) {
			FreeImage_GetPixelColor(image, j, i, &aPixel);
			data[pnum].r = (aPixel.rgbRed);
			data[pnum].g = (aPixel.rgbGreen);
			data[pnum++].b = (aPixel.rgbBlue);
		}
	}
	FreeImage_Unload(image);
	return data;
}//read_img

//write_img
void write_img(char *name, pixel *data, int width, int height) {
	FIBITMAP *image;
	RGBQUAD aPixel;
	int i,j;

	image = FreeImage_Allocate(width, height, 24, 0, 0, 0);
	if(!image) {
		perror("FreeImage_Allocate");
		return;
	}
	for(i = 0 ; i < height ; i++) {
		for(j = 0 ; j < width ; j++) {
			aPixel.rgbRed = data[i*width+j].r;
			aPixel.rgbGreen = data[i*width+j].g;
			aPixel.rgbBlue = data[i*width+j].b;

			FreeImage_SetPixelColor(image, j, i, &aPixel);
		}
	}
	if(!FreeImage_Save(FIF_TIFF, image, name, 0)) {
		perror("FreeImage_Save");
	}
	FreeImage_Unload(image);
}//write_img


/*draw the image - it is already in the format openGL requires for glDrawPixels*/
void display_image(void)
{
	glDrawPixels(global.w, global.h, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)global.data);	
	glFlush();
}//display_image()

// Read the screen image back to the data buffer after drawing to it
void draw_triangle(void)
{
	glDrawPixels(global.w, global.h, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)global.data);
	glBegin(GL_TRIANGLES);
	glColor3f(1.0,0,0);
	glVertex2i(rand()%global.w,rand()%global.h);
	glColor3f(0,1.0,0);
	glVertex2i(rand()%global.w,rand()%global.h);
	glColor3f(0,0,1.0);
	glVertex2i(rand()%global.w,rand()%global.h);
	glEnd();
	glFlush();
	glReadPixels(0,0,global.w,global.h,GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)global.data);
}

/* A simple thresholding filter.
*/
void MyFilter(pixel* Im, int myIm_Width, int myIm_Height){
	int x,y;

	for (x=0; x < myIm_Width; x++)
		for (y=0; y < myIm_Height; y++){
			if (Im[x+y*myIm_Width].b > 128)
				Im[x+y*myIm_Width].b = 255;
			else 
				Im[x+y*myIm_Width].b = 0;

			if (Im[x+y*myIm_Width].g > 128)
				Im[x+y*myIm_Width].g = 255;
			else 
				Im[x+y*myIm_Width].g = 0;

			if (Im[x+y*myIm_Width].r > 128)
				Im[x+y*myIm_Width].r = 255;
			else 
				Im[x+y*myIm_Width].r = 0;
		}

	glutPostRedisplay();	// Tell glut that the image has been updated and needs to be redrawn
}//My_Filter


/*glut keyboard function*/
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 0x1B:
	case'q':
	case 'Q':
		free(global.data);
		exit(0);
		break;
	case's':
	case'S':
		printf("SAVING IMAGE: backup.tif\n");
		write_img("backup.tif", global.data, global.w, global.h);
		break;
	case 't':
	case 'T':
		draw_triangle();
		break;
	case'f':
	case'F':
		MyFilter(global.data,global.w,global.h);
		break;
	}
}//keyboard


int main(int argc, char** argv)
{
	global.data = read_img("img.tif", &global.w, &global.h);
	if (global.data==NULL)
	{
		printf("Error loading image file img.tif\n");
		return 1;
	}
	printf("Q:quit\nF:filter\nT:triangle\nS:save\n");
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGB|GLUT_SINGLE);
	
	glutInitWindowSize(global.w,global.h);
	glutCreateWindow("SIMPLE DISPLAY");
	glShadeModel(GL_SMOOTH);
	glutDisplayFunc(display_image);
	glutKeyboardFunc(keyboard);
	glMatrixMode(GL_PROJECTION);
	glOrtho(0,global.w,0,global.h,0,1);
	
	glutMainLoop();

	return 0;
}
