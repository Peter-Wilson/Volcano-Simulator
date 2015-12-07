#if !defined(Linux)
  #include <windows.h>           //Not Linux must be windows
#else
	#include <stdlib.h>
#endif
#include <GL/gl.h>
#include <stdio.h>
#include <glut.h>
#include <math.h>
#include <FreeImage.h>


#define X 0
#define Y 1
#define Z 2

//the x,y,z coordinate struct used for positioning
struct coordinates{
	float x, y, z;
};
typedef struct coordinates coordinates;


//The rgb struct
struct colour{
	GLubyte r, g, b;
};
typedef struct colour colour;

//a linked list of particles
struct particle{
	coordinates *position, *directionVector, *directionRotation, *rotation;
	float speed;
	int size, age;
	float velocity;
	colour *c;
	int isCamera, type;
	struct particle *next;
};
typedef struct particle particle;

//the global variables
struct glob {
   particle *headParticle;
   float angle[3];
   int axis;
   float gravity, airResistance, groundResistance;
   int particleCount;
   int particleSize;
   int planeBounds;
   int fallingCutoff;
   int ageCutoff;
   int steamOfPixels;
   int timeBetweenShots, timer;
   GLuint groundTexture;
   GLuint lavaTexture;
   GLuint volcanoTexture;
   GLuint lavaTexture2;
   GLuint lavaTexture3;
   GLuint index;
   int spray, multiball, randomSpeed, randomSpin, backfaceCulling, displayVersion, lightingVersion;  
   coordinates *volcano; 
};

struct glob global= {NULL,{0.0,0.0,0.0},Y,0.001,0.00001,0.1,0, 5, 180, -300, 7000, 0, 500, 500, 0, 1, 2, 3, 4};

//create a new coordinate
coordinates* newCoordinate(float x, float y, float z)
{
	struct coordinates *newCoordinate; 
	newCoordinate = (coordinates*)(malloc( sizeof(struct coordinates) ));  
	newCoordinate->x = x;
	newCoordinate->y = y;
	newCoordinate->z = z;
}

//resets the camera
void resetCamera()
{
	global.angle[X] = 0.0;
	global.angle[Y] = 0.0;
	global.angle[Z] = 0.0;
	glPopMatrix();
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective(30.0, 1.5, 0.1, 1000.0);
	gluLookAt(0.0, 500.0, 600, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0);
}

//Creates all the objects in the display list
void DisplayListInit(){
	//initializes the sizes
	int p[][3] = {{global.particleSize,global.particleSize,global.particleSize}, 
						{global.particleSize,-global.particleSize,global.particleSize}, 
						{-global.particleSize,-global.particleSize,global.particleSize}, 
						{-global.particleSize,global.particleSize,global.particleSize},
						 {global.particleSize,global.particleSize,-global.particleSize}, 
						{global.particleSize,-global.particleSize,-global.particleSize}, 
						{-global.particleSize,-global.particleSize,-global.particleSize}, 
						{-global.particleSize,global.particleSize,-global.particleSize}};
	int p2[][3] = {{global.particleSize+2,global.particleSize+2,global.particleSize+2}, 
						{global.particleSize+2,-(global.particleSize+2),global.particleSize+2}, 
						{-(global.particleSize+2),-(global.particleSize+2),global.particleSize+2}, 
						{-(global.particleSize+2),global.particleSize+2,global.particleSize+2},
						 {global.particleSize+2,global.particleSize+2,-(global.particleSize+2)}, 
						{global.particleSize+2,-(global.particleSize+2),-(global.particleSize+2)}, 
						{-(global.particleSize+2),-(global.particleSize+2),-(global.particleSize+2)}, 
						{-(global.particleSize+2),global.particleSize+2,-(global.particleSize+2)}};
	int p3[][3] = {{global.particleSize-2,global.particleSize-2,global.particleSize-2}, 
						{global.particleSize-2,-(global.particleSize-2),global.particleSize-2}, 
						{-(global.particleSize-2),-(global.particleSize-2),global.particleSize-2}, 
						{-(global.particleSize-2),global.particleSize-2,global.particleSize-2},
						 {global.particleSize-2,global.particleSize-2,-(global.particleSize-2)}, 
						{global.particleSize-2,-(global.particleSize-2),-(global.particleSize-2)}, 
						{-(global.particleSize-2),-(global.particleSize-2),-(global.particleSize-2)}, 
						{-(global.particleSize-2),global.particleSize-2,-(global.particleSize-2)}};

		  int e[][4] = {{0,3,2,1},{3,7,6,2},{7,4,5,6},{4,0,1,5}, {0,4,7,3},{1,2,6,5}};  
		  int i;
		  

		  //for particles
		  GLfloat mat_ambient[] = {0.2, 0.2, 0.2, 1.0};
		  GLfloat mat_diffuse[] = {0.8, 0.8, 0.8, 1.0};
		  GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
		  GLfloat mat_shininess[] = { 77.0 };
		  float norm[][3] = {{0,0,1.0},{-1.0,0,0},{0,0,-1.0},{1.0,0,0},{0,1.0,0},{0,-1.0,0}};

		  //for plane
		  GLfloat mat_ambient2[] = {0.2, 0.2, 0.2, 1.0};
		  GLfloat mat_diffuse2[] = {0.8, 0.8, 0.8, 1.0};
		  GLfloat mat_specular2[] = {1.0, 1.0, 1.0, 1.0};
		  GLfloat mat_shininess2[] = { 77.0 };
		  float norm2[][3] = {{0,1.0,0}};

		  //for volcano
		  GLfloat mat_ambient3[] = {0.2, 0.2, 0.2, 1.0};
		  GLfloat mat_diffuse3[] = {0.8, 0.8, 0.8, 1.0};
		  GLfloat mat_specular3[] = {1.0, 1.0, 1.0, 1.0};
		  GLfloat mat_shininess3[] = { 77.0 };
		  float norm3[][3] = {{0,1/sqrt(5.0),2/sqrt(5.0)},{(-2/sqrt(5.0))/2,1/sqrt(5.0),(2/sqrt(5.0))/2}, 
							{-2/sqrt(5.0),1/sqrt(5.0),0},{(-2/sqrt(5.0))/2,1/sqrt(5.0),(-2/sqrt(5.0))/2},
							{0,1/sqrt(5.0),-2/sqrt(5.0)},{(2/sqrt(5.0))/2,1/sqrt(5.0),(-2/sqrt(5.0))/2},
							{2/sqrt(5.0),1/sqrt(5.0),0}, {(2/sqrt(5.0))/2,1/sqrt(5.0),(2/sqrt(5.0))/2},};

	global.index = glGenLists(9);
	glLineWidth((GLfloat)10);
	glPointSize(1);

	//create the particles
	glNewList(global.index, GL_COMPILE);   // compile the first one	  

		  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

		  glBindTexture(GL_TEXTURE_2D, global.lavaTexture);
		  for (i=0; i < 6; ++i) {
			 glBegin(GL_QUADS);
				glNormal3fv(norm[i]);glTexCoord2f(0.0f, 1.0f);glVertex3iv(p[e[i][0]]);
				glNormal3fv(norm[i]);glTexCoord2f(1.0f, 1.0f);glVertex3iv(p[e[i][1]]);
				glNormal3fv(norm[i]);glTexCoord2f(1.0f, 0.0f);glVertex3iv(p[e[i][2]]);
				glNormal3fv(norm[i]);glTexCoord2f(0.0f, 0.0f);glVertex3iv(p[e[i][3]]);
			 glEnd();
		  }
	glEndList();

	//create the plane
	glNewList(global.index+1, GL_COMPILE);   	  

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient2);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse2);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular2);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess2);

	glBindTexture(GL_TEXTURE_2D, global.groundTexture);
		glBegin( GL_QUADS );
				glNormal3fv(norm2[0]);glTexCoord2f(0.0f, 1.0f);glVertex3i(-global.planeBounds, 0, global.planeBounds);
				glNormal3fv(norm2[0]);glTexCoord2f(1.0f, 1.0f);glVertex3i(global.planeBounds, 0 , global.planeBounds);
				glNormal3fv(norm2[0]);glTexCoord2f(1.0f, 0.0f);glVertex3i(global.planeBounds, 0, -global.planeBounds);
				glNormal3fv(norm2[0]);glTexCoord2f(0.0f, 0.0f);glVertex3i(-global.planeBounds, 0, -global.planeBounds);	
		glEnd();
	glEndList();

	//create the volcano
	glNewList(global.index+2, GL_COMPILE);  
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient3);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse3);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular3);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess3);

	
		glBindTexture(GL_TEXTURE_2D, global.volcanoTexture);
		glBegin( GL_TRIANGLES );
		glNormal3fv(norm3[1]);glTexCoord2f(1.0f, 0.0f);glVertex3f( -50.0f, 0.0f, 50.0f );
		glNormal3fv(norm3[7]);glTexCoord2f(0.0f, 0.0f);glVertex3f( 50.0f, 0.0f, 50.0f);		
		glNormal3fv(norm3[0]);glTexCoord2f(0.0f, 1.0f);glVertex3f( 0.0f, 100.f, 0.0f );

	
		
		glNormal3fv(norm3[3]);glTexCoord2f(1.0f, 0.0f);glVertex3f( -50.0f, 0.0f, -50.0f);
		glNormal3fv(norm3[1]);glTexCoord2f(0.0f, 0.0f);glVertex3f( -50.0f, 0.0f, 50.0f);
		glNormal3fv(norm3[2]);glTexCoord2f(0.0f, 1.0f);glVertex3f( 0.0f, 100.0f, 0.0f);
	
		
		glNormal3fv(norm3[5]);glTexCoord2f(1.0f, 0.0f);glVertex3f( 50.0f, 0.0f, -50.0f);
		glNormal3fv(norm3[3]);glTexCoord2f(0.0f, 0.0f);glVertex3f( -50.0f, 0.0f, -50.0f);
		glNormal3fv(norm3[4]);glTexCoord2f(0.0f, 1.0f);glVertex3f( 0.0f, 100.0f, 0.0f);
	
		
		glNormal3fv(norm3[7]);glTexCoord2f(1.0f, 0.0f);glVertex3f( 50.0f, 0.0f, 50.0f);
		glNormal3fv(norm3[5]);glTexCoord2f(0.0f, 0.0f);glVertex3f( 50.0f, 0.0f, -50.0f);
		glNormal3fv(norm3[6]);glTexCoord2f(0.0f, 1.0f);glVertex3f( 0.0f, 100.0f, 0.0f);
	
		glEnd();
	glEndList();

	//create the particle version 2
	glNewList(global.index+3, GL_COMPILE);   // compile the first one	  

		  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

		  glBindTexture(GL_TEXTURE_2D, global.lavaTexture2);
		  for (i=0; i < 6; ++i) {
			 glBegin(GL_QUADS);
				glNormal3fv(norm[i]);glTexCoord2f(0.0f, 1.0f);glVertex3iv(p2[e[i][0]]);
				glNormal3fv(norm[i]);glTexCoord2f(1.0f, 1.0f);glVertex3iv(p2[e[i][1]]);
				glNormal3fv(norm[i]);glTexCoord2f(1.0f, 0.0f);glVertex3iv(p2[e[i][2]]);
				glNormal3fv(norm[i]);glTexCoord2f(0.0f, 0.0f);glVertex3iv(p2[e[i][3]]);
			 glEnd();
		  }
	glEndList();

	//create the particle version 3
	glNewList(global.index+4, GL_COMPILE);   // compile the first one	  

		  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

		  glBindTexture(GL_TEXTURE_2D, global.lavaTexture3);
		  for (i=0; i < 6; ++i) {
			 glBegin(GL_QUADS);
				glNormal3fv(norm[i]);glTexCoord2f(0.0f, 1.0f);glVertex3iv(p3[e[i][0]]);
				glNormal3fv(norm[i]);glTexCoord2f(1.0f, 1.0f);glVertex3iv(p3[e[i][1]]);
				glNormal3fv(norm[i]);glTexCoord2f(1.0f, 0.0f);glVertex3iv(p3[e[i][2]]);
				glNormal3fv(norm[i]);glTexCoord2f(0.0f, 0.0f);glVertex3iv(p3[e[i][3]]);
			 glEnd();
		  }
	glEndList();

	//Wireframe plane
	glNewList(global.index+5, GL_COMPILE);   	  
		glBegin( GL_LINES );
				glVertex3i(-global.planeBounds, 0, global.planeBounds);
				glVertex3i(global.planeBounds, 0 , global.planeBounds);
				glVertex3i(global.planeBounds, 0 , global.planeBounds);
				glVertex3i(global.planeBounds, 0, -global.planeBounds);
				glVertex3i(global.planeBounds, 0, -global.planeBounds);
				glVertex3i(-global.planeBounds, 0, -global.planeBounds);
				glVertex3i(-global.planeBounds, 0, -global.planeBounds);	
				glVertex3i(-global.planeBounds, 0, global.planeBounds);
		glEnd();
	glEndList();

	//wireframe volcano
	glNewList(global.index+6, GL_COMPILE); 
		//glColor3ui(255,255,255);
		glBegin( GL_LINES );
			glVertex3f( 0.0f, 100.f, 0.0f );
			glVertex3f( -50.0f, 0.0f, 50.0f );
			glVertex3f( -50.0f, 0.0f, 50.0f );
			glVertex3f( 50.0f, 0.0f, 50.0f);
			glVertex3f( 50.0f, 0.0f, 50.0f);
			glVertex3f( 0.0f, 100.0f, 0.0f);
			glVertex3f( 0.0f, 100.0f, 0.0f);
			glVertex3f( -50.0f, 0.0f, -50.0f);
			glVertex3f( -50.0f, 0.0f, -50.0f);
			glVertex3f( -50.0f, 0.0f, 50.0f);
			glVertex3f( -50.0f, 0.0f, 50.0f);
			glVertex3f( 0.0f, 100.0f, 0.0f);
			glVertex3f( 0.0f, 100.0f, 0.0f);
			glVertex3f( 50.0f, 0.0f, -50.0f);
			glVertex3f( 50.0f, 0.0f, -50.0f);
			glVertex3f( -50.0f, 0.0f, -50.0f);
			glVertex3f( -50.0f, 0.0f, -50.0f);
			glVertex3f( 0.0f, 100.0f, 0.0f);
			glVertex3f( 0.0f, 100.0f, 0.0f);
			glVertex3f( 50.0f, 0.0f, 50.0f);
			glVertex3f( 50.0f, 0.0f, 50.0f);
			glVertex3f( 50.0f, 0.0f, -50.0f);
		glEnd();
	glEndList();

	//wireframe particle
	glNewList(global.index+7, GL_COMPILE);   // compile the first one
		  for (i=0; i < 6; ++i) {
			//glColor3ui(255,255,255);
			 glBegin(GL_LINES);
				glVertex3iv(p2[e[i][0]]);
				glVertex3iv(p2[e[i][1]]);
				glVertex3iv(p2[e[i][1]]);
				glVertex3iv(p2[e[i][2]]);
				glVertex3iv(p2[e[i][2]]);
				glVertex3iv(p2[e[i][3]]);
				glVertex3iv(p2[e[i][0]]);
			 glEnd();
		  }
	glEndList();

	//dot particle
	glNewList(global.index+8, GL_COMPILE);   // compile the first one
			//glColor3ui(255,255,255);
			glPointSize(10.0);
			 glBegin(GL_POINTS);
				glVertex3i(0,0,0);
			 glEnd();
	glEndList();
}

//initializes the lighting
void myLightInit() {
   GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};
   GLfloat diffuse[] = {0.8, 0.8, 0.8, 1.0};
   GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};
   GLfloat position[] = {1.0, 1.0, 1.0, 0.0};
   GLfloat lmodel_ambient[] = {0.2, 0.2, 0.2, 1.0};
   GLfloat local_view[] = {0.0};

   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
   glLightfv(GL_LIGHT0, GL_POSITION, position);
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
   glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

   //default to flat lighting
   glShadeModel(GL_FLAT);

   glEnable(GL_LIGHTING);   /* turns on lighting */
   glEnable(GL_LIGHT0);     /* turns on light 0  */
   /* glEnable(GL_NORMALIZE); not necessary here. */
}

//Create the texture given a texture file and the id
void createTexture(char *filename, GLuint id)
{
	int i,j;
	int tile;
	FIBITMAP *pImage;
	FIBITMAP* bitmap;
	int nWidth;
	int nHeight;
	//Create the texture id
	glBindTexture(GL_TEXTURE_2D, id);

	//load the image
   bitmap = FreeImage_Load(
        FreeImage_GetFileType(filename, 0),
        filename, BMP_DEFAULT);

	pImage = FreeImage_ConvertTo32Bits(bitmap);
	nWidth = FreeImage_GetWidth(pImage);
	nHeight = FreeImage_GetHeight(pImage);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, nWidth, nHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(pImage));
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, nWidth, nHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(pImage));
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, nWidth, nHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(pImage));
	
	glEnable(GL_TEXTURE_2D);	
	FreeImage_Unload(pImage);
}

void myTextureInit()
{
	//Create the ground texture
	createTexture("ground_texture_2.png", global.groundTexture);

	//Create the volcano texture
	createTexture("volcano_texture.png", global.volcanoTexture);

	//Create the lava texture
	createTexture("texture_of_lava.png", global.lavaTexture);

	//Create the lava texture
	createTexture("texture_of_lava_2.png", global.lavaTexture2);

	//Create the lava texture
	createTexture("texture_of_lava_3.png", global.lavaTexture3);

}

//remove the specified particle from the particle linked list
particle* RemoveParticle(particle *particle)
{
	struct particle *current = global.headParticle;

	//if its the head
	if(particle == global.headParticle)
	{
		global.headParticle = global.headParticle->next;
		global.particleCount--;
		free(particle);
		return global.headParticle;
	}
	
	if(particle)
	{
		while(current->next)
		{
			if(current->next == particle)
			{
				global.particleCount--;
				current->next = current->next->next;
				free(particle);
				return current->next;
			}
			current = current-> next;
		}
	}
	return NULL;
}


//create a new colour
colour* newColour(GLubyte r, GLubyte g, GLubyte b)
{
	struct colour *newColour; 
	newColour = (colour*)(malloc( sizeof(struct colour) ));  
	newColour->r = r;
	newColour->g = g;
	newColour->b = b;
}

//Fire a new particle into the scene
void FireParticle(coordinates *position, coordinates *direction, float speed, int size, coordinates *rotation, int isCamera, int type){
	//initialize the particle
	struct particle *newParticle; 
	newParticle = (particle*)(malloc( sizeof(struct particle) ));  
	newParticle->next = global.headParticle;
	newParticle->position = position;
	newParticle->velocity = direction->y;
	newParticle->directionVector = direction;
	if(global.randomSpin == 1)
		newParticle->directionRotation = newCoordinate(rand()%5,rand()%5, rand()%5);
	else 
		newParticle->directionRotation = newCoordinate(0,0,0);
	newParticle->rotation = newCoordinate(0,0,0);
	if(global.randomSpeed == 1)	{
		newParticle->speed = (rand()%4)+1;
	}
	else{
		newParticle->speed = speed;
	}
	newParticle->size = size;
	newParticle->age = 0;
	newParticle->isCamera = isCamera;
	newParticle->c = newColour(125,125,125);
	newParticle->type = type;
	//add the particle to the list
	global.headParticle = newParticle;
	global.particleCount++;
	PlaySound(L"bounce.wav", NULL, SND_ASYNC|SND_FILENAME); //make a firing sound
}

//reset the board
void reset()
{
	while(global.headParticle)
	{
		RemoveParticle(global.headParticle);
	}
	resetCamera();
	global.volcano = newCoordinate(0.0,0.0,0.0);
	global.spray = 0;
	global.multiball = 0;
	global.randomSpeed = 0;
	global.randomSpin = 0;
	global.backfaceCulling = 0;
	global.displayVersion = 0;
	global.lightingVersion = 0;
	glEnable(GL_LIGHTING);   /* turns on lighting */
	glEnable(GL_LIGHT0);
}

//check if the particle collides with the ground
int Collision(particle* particle)
{
	return ((particle->directionVector->y + particle->position->y - (particle->size)) < 0 && abs(particle->position->x) < global.planeBounds && abs(particle->position->z) < global.planeBounds);
}

//move and draw the particle
void calculateMatrix(particle *particle){
	int random;
	//loop through the particles
	while(particle)
	{
		//if the particle is too old or fell below the cutoff point, remove if
		  if(particle->age >= global.ageCutoff || particle->position->y < global.fallingCutoff)
		  {
			  if(particle->isCamera == 1)
			  {
				  resetCamera();
				  }
			  particle = RemoveParticle(particle);
				continue;
		  }

			//update the position
			particle->position->x += particle->directionVector->x*particle->speed;
			particle->position->z += particle->directionVector->z*particle->speed;

			//update the rotation
			particle->rotation->x = (int)(particle->rotation->x+particle->directionRotation->x)%360;
			particle->rotation->y = (int)(particle->rotation->y+particle->directionRotation->y)%360;
			particle->rotation->z = (int)(particle->rotation->z+particle->directionRotation->z)%360;
	  
			//check if it hit the ground
			if(Collision(particle))
			{
				//bounce
				if(particle->velocity/2 > 0.1)
				{
					particle->velocity /= 2;
					particle->directionVector->y = particle->velocity;
					particle->position->y += particle->directionVector->y;
				}
				//if velocity is 0, stop the particle
				else
				{
					particle->velocity = 0;
					particle->directionVector->y = 0;
					particle->position->y = particle->size;
					if(particle->isCamera == 1){
						resetCamera();
					}
					particle = RemoveParticle(particle);
					continue;
				}
				particle->speed -= global.groundResistance;
			}
			//otherwise, slow down the particle due to air resistance
			else
			{
				particle->position->y += particle->directionVector->y*particle->speed;
				particle->speed -= global.airResistance;
			}

			//check if the particle stopped
			if(particle->speed < 0)
				particle->speed = 0;
		  
		  //push the new matrix for this particle
		  glPushMatrix();
		  if(particle->isCamera == 1)
		  {
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				gluPerspective(30.0, 1.5, 0.1, 1000.0);
				gluLookAt(particle->position->x, 
					particle->position->y, 
					particle->position->z, 
				  1.0, 1.0, 1.0, 0.0, 1.0, 0.0);
				glPopMatrix();
		  }
		  
		  glMatrixMode(GL_MODELVIEW);

		  //move the particle to the save position
		  glTranslatef(particle->position->x, particle->position->y, particle->position->z);

		  //rotate the particle to the new rotation
		  glRotatef(particle->rotation->x, 1.0, 0.0, 0.0); 
		  glRotatef(particle->rotation->y, 0.0, 1.0, 0.0); 
		  glRotatef(particle->rotation->z, 0.0, 0.0, 1.0);
		  
		  if(global.displayVersion == 0)
		  {
			  switch(particle->type)
			  {
				  case 0:
					  //draw from the call list
					glCallList(global.index);
					break;
				  case 1:
					  //draw from the call list
					glCallList(global.index+3);
					break;
				  case 2:
					  //draw from the call list
					glCallList(global.index+4);
					break;
			  }
		  }
		  //draw the wireframe version
		  else if (global.displayVersion == 1)
		  {
			  glCallList(global.index+7);
		  }
		  //draw the dot version
		  else
		  {
			  glCallList(global.index+8);
		  }
	  
		  //update post move variables
		  particle->directionVector->y -= global.gravity;
		  particle->age++;

		  //pop that new matrix
		  glPopMatrix();
		  particle = particle->next;
	}
}

//draw the plane
void drawPlane(){
	if(global.displayVersion > 0)
		glCallList(global.index+5); //draw the wireframe version
	else
		glCallList(global.index+1); //draw the textured version
}

//draw the pyramid
void drawPyramid(){
	glPushMatrix();
	glTranslatef(global.volcano->x, global.volcano->y, global.volcano->z); //move the volcano to it's spot
	if(global.displayVersion > 0)
		glCallList(global.index+6); //draw the wireframe version
	else
		glCallList(global.index+2); //draw the textured version
	glPopMatrix();
}

//draw the board
void drawBoard(void) {

	float angle = (((float)(rand()%10)/10.0)*0.2)-0.1;
	float angle2 = (((float)(rand()%10)/10.0)*0.2)-0.1;

	//slowly rotate around the board
    glRotatef(global.angle[X], 1.0, 0.0, 0.0);
	glRotatef(global.angle[Y], 0.0, 1.0, 0.0);
	glRotatef(global.angle[Z], 0.0, 0.0, 1.0); 

	//shoot a new particle if stream is on
	if(global.steamOfPixels == 1)
	{
		//determine the spray
		if(global.spray == 1)
		{
			angle = (((float)(rand()%10)/10.0)*0.8)-0.1;
			angle2 = (((float)(rand()%10)/10.0)*0.8)-0.1;
		}

		if(global.timer <= 0)
		{
			int value = 0;
		    if(global.multiball == 1)
			  value = (rand()%3);
			global.timer = global.timeBetweenShots;			
			FireParticle(newCoordinate(global.volcano->x,100,global.volcano->z), newCoordinate(angle,0.5,angle2), 1, 5, newCoordinate(5,1,0), 0, value);
		}
		else
			global.timer--;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw the environment	
	drawPlane();
	drawPyramid();

	//draw the particles
	calculateMatrix(global.headParticle);

	glutSwapBuffers();
	glFlush();
}

//Give the users a command list
void userintro() {  
   printf("\nWelcome to PETE'S VOLCANO SIMULATOR 3000\n");   
   printf("=========================================\n");
   printf("                COMMANDS                  \n");
   printf("=========================================\n");
   printf("Shooting Modes:\n\n");
   printf("\tSpace bar = turn on constant stream of particles\n");
   printf("\tF (press) = Shoot one particle\n");
   printf("\tF (hold) = Rapid Fire\n");
   printf("\tE = Spray Mode\n");
   printf("\tM = Multiparticle Mode\n");
   printf("\tN = Random Spin\n");   
   printf("\tP = shoot the camera out of the volcano\n");
   printf("\tG = Random Speed\n\n");
   
   printf("Movement controls:\n\n");
   printf("\tW A S D = Move the volcano around\n");
   printf("\tx, y, z = rotate about x, y, or z axis\n");
   printf("\tLeft mouse = rotate left faster\n");
   printf("\tRight mouse = rotate right faster\n\n");

   printf("Display settings:\n\n");
   printf("\tV = toggle display version (dot, wireframe, textured)\n");
   printf("\tB = Toggle Backface Culling\n");
   printf("\tL = Toggle Gouraud Shading (To the volcano)\n");
   printf("R = reset\n");
   printf("q = quit\n");
}

//respond to key events
void keyboard(unsigned char key, int x, int y) {
	float angle = (((float)(rand()%10)/10.0)*0.2)-0.1;
	float angle2 = (((float)(rand()%10)/10.0)*0.2)-0.1;
	int value = 0;	
	if(global.spray == 1)
	{
		angle = (((float)(rand()%10)/10.0)*0.8)-0.1;
		angle2 = (((float)(rand()%10)/10.0)*0.8)-0.1;
	}


   switch (key){
      case ' ':
		  if(global.steamOfPixels == 0)
			  global.steamOfPixels = 1;
		  else
			  global.steamOfPixels = 0;
	 break;

	 case 'x':
      case 'X':
	 global.axis = X;
	 break;

      case 'y':
      case 'Y':
	 global.axis = Y;
	 break;

      case 'z':
      case 'Z':
	 global.axis = Z;
	 break;

      case 'f':
      case 'F':
		  if(global.multiball == 1)
			value = (rand()%3);
		FireParticle(newCoordinate(global.volcano->x,100,global.volcano->z), newCoordinate(angle,0.5,angle2), 1, 5, newCoordinate(5,1,0), 0, value);
	 break;

	  case 'p':
	  case 'P':
		   if(global.multiball == 1)
				value = (rand()%3);
		   FireParticle(newCoordinate(global.volcano->x,100,global.volcano->z), newCoordinate(angle,0.5,angle2), 1, 5, newCoordinate(5,1,0),1, value);
	 break;

      case 'r':
      case 'R':
	 reset();
	 break;

	 case 'g':
	  case 'G':
		  global.randomSpeed = (global.randomSpeed == 0)?1:0;
	  break;

	  case 'e':
	  case 'E':
		  global.spray = (global.spray == 0)?1:0;
	  break;

	  case 'n':
	  case 'N':
		  global.randomSpin = (global.randomSpin == 0)?1:0;
	  break;	  

	  case 'l':
	  case 'L':
		  global.lightingVersion = (global.lightingVersion == 0)?1:0;
		  if(global.lightingVersion > 0) {
			  glShadeModel(GL_SMOOTH); 
		  }
		  else {
			  glShadeModel(GL_FLAT); 
		  }

	  break;
	  
	  case 'v':
	  case 'V':
		  global.displayVersion = (global.displayVersion != 2)?global.displayVersion+1:0;
		  if(global.displayVersion > 0)  {
			  glDisable(GL_LIGHTING);   /* turns on lighting */
			  glDisable(GL_LIGHT0);
		  }
		  else {
			  glEnable(GL_LIGHTING);   /* turns on lighting */
			  glEnable(GL_LIGHT0);
		  }
	  break;

	  
	  case 'm':
	  case 'M':
		  global.multiball = (global.multiball == 0)?1:0;
	  break;

	  case 'd':
	  case 'D':
		  if((global.volcano->x + 50)+5 < global.planeBounds)
			  global.volcano->x+=5;
	  break;

	  case 's':
	  case 'S':
		  if((global.volcano->z + 50)+5 < global.planeBounds)
			  global.volcano->z+=5;
	  break;

	  case 'a':
	  case 'A':
		  if((global.volcano->x - 50)-5 > -(global.planeBounds))
			  global.volcano->x -=5;
	  break;

	  case 'W':
	  case 'w':
		  if((global.volcano->z - 50)-5 > -(global.planeBounds))
			  global.volcano->z-=5;
	  break;

	  case 'b':
	  case 'B':
		  global.backfaceCulling = (global.backfaceCulling == 0)?1:0;
		  if(global.backfaceCulling == 1)  {			  
			  glDisable(GL_CULL_FACE);
		  }
		  else {
			  glFrontFace(GL_CCW); 
			  glCullFace(GL_BACK);
			  glEnable(GL_CULL_FACE);
		  }
	  break;

      case 0x1B:
      case 'q':
      case 'Q':
      exit(0);
	 break;
   }
}

//respond to mouse events
void mouse(int btn, int state, int x, int y) {
   int value = 0;
   if (state == GLUT_DOWN) {
	   if (btn==GLUT_LEFT_BUTTON) {
  	  global.angle[global.axis] = global.angle[global.axis] + 0.2;
      }
      else if (btn == GLUT_RIGHT_BUTTON) {
	  global.angle[global.axis] = global.angle[global.axis] - 0.2;
      }
   }
}

//initialize the board
main(int argc, char **argv) {

  userintro();
  glutInit(&argc, argv);
  glutInitWindowSize(500, 500);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("Volcano City");
  glutMouseFunc(mouse);
  glutKeyboardFunc(keyboard);
  glutDisplayFunc(drawBoard);
  glutIdleFunc(drawBoard); 

  global.volcano = newCoordinate(0.0,0.0,0.0);

  resetCamera();

  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  
  glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	DisplayListInit();
  myLightInit();
  myTextureInit();

  glFrontFace(GL_CCW); 
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

  glutMainLoop();
}

