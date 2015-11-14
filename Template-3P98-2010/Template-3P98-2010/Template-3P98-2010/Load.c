/* An example of how to read an image (img.tif) from file using freeimage and then
display that image using openGL's drawPixelCommand. Also allow the image to be saved
to backup.tif with freeimage and a simple thresholding filter to be applied to the image.
Conversion by Lee Rozema.
Added triangle draw routine, fixed memory leak and improved performance by Robert Flack (2008)
*/

#include <stdlib.h>
#include <math.h>


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


//random point linked list
struct node{
    int x, y, drawn;
	struct node *next;
};
typedef struct node node;

//random point linked list
struct edge{
    node *start, *end;
	struct edge *next;
};

typedef struct edge edge;

//triangle linked list
struct triangle{
    node *a, *b, *c;
	struct triangle *next;
};
typedef struct triangle triangle;

//the global structure
typedef struct {
	pixel *data;
	int w, h, pointCount, numClusters, triangleCount;
	struct node *headCluster;
	struct node *tailCluster;
	struct node *tailNode;
	struct node *headNode;
	struct edge *tailEdge;
	struct edge *headEdge;
	struct triangle *headTriangle;
	struct triangle *tailTriangle;
	int mouse;
} glob;
glob global;


void AddCluster(node *add)
{
	struct node *newPoint; 
	newPoint = (node*)(malloc( sizeof(struct node) ));  
	newPoint->next = 0;
	newPoint->x = add->x;
	newPoint->y = add->y;
	newPoint->drawn = 0;

	if(!global.headCluster)
	{
		global.headCluster = newPoint;
		global.tailCluster = global.headCluster;
		global.numClusters++;
	}
	else
	{
		global.tailCluster->next = newPoint; 
		global.tailCluster = newPoint;
		global.numClusters++;
	}

}

void RemoveCluster(node *clust)
{
	struct node *current = global.headCluster;

	//if its the head
	if(current->x == clust->x &&
				current->y == clust->y)
	{
		global.headCluster = global.headCluster->next;
		global.numClusters--;
		free(current);
		return;
	}

	if(clust)
	{
		while(current)
		{
			if(current->next->x == clust->x &&
				current->next->y == clust->y)
			{
				global.numClusters--;
				current->next = current->next->next;
				free(clust);
				return;
			}
			current = current-> next;
		}
	}
}

int PointExists(int x, int y)
{
	node* head = global.headNode;
	while(head)
	{
		if(x == head->x && y == head->y){
			return 1;
		}
		head = head->next;

	}

	return 0;
}

void AddPoint(int x, int y)
{
	struct node *newPoint; 
	newPoint = (node*)(malloc( sizeof(struct node) ));
	newPoint->next = 0;
	newPoint->x = x;
	newPoint->y = y;
	newPoint->drawn = 0;

	if(PointExists(x,y) == 1) return;

	if(!global.headNode)
	{
		global.headNode = newPoint;
		global.tailNode = global.headNode;
		global.pointCount++;
	}
	else
	{
		global.tailNode->next = newPoint; 
		global.tailNode = newPoint;
		global.pointCount++;
	}
}

node* RemovePoint(node *dest)
{
	struct node *current = global.headNode;

	//if its the head
	if(current->x == dest->x &&
				current->y == dest->y)
	{
		global.headNode = global.headNode->next;
		global.pointCount--;
		free(current);
		return;
	}

	if(dest)
	{
		while(current)
		{
			if(current->next->x == dest->x &&
				current->next->y == dest->y)
			{
				global.pointCount--;
				current->next = current->next->next;
				free(dest);
				return current->next;
			}
			current = current-> next;
		}
	}
	else
		return NULL;

	if(!dest) return NULL;
}


void Draw_Triangle(triangle* t)
{
	glColor3ub(rand()%256,rand()%256,rand()%256); 
    glPointSize(4.0);
	glBegin(GL_LINE_LOOP);	
	glVertex2d(t->a->x, t->a->y);
	glVertex2d(t->b->x, t->b->y);
	glVertex2d(t->c->x, t->c->y);
	glEnd();	
	glFlush();	
	glBegin(GL_POINTS);	
	glVertex2d((t->a->x + t->b->x + t->c->x)/3, (t->a->y + t->b->y + t->c->y)/3);
	glEnd();	
	glFlush();
}

void DrawAllTriangles()
{
	triangle* head = global.headTriangle;
	glutPostRedisplay();
	while(head)
	{
		Draw_Triangle(head);
		head = head->next;
	}

}

int TriangleAlreadyExists(node* a, node* b, node* c)
{
	triangle* head = global.headTriangle;

	while(head)
	{
		if(head->a == a && head->b == b && head->c == c)
			return 1;
		head = head->next;
	}
	return 0;
}

int ValidTriangle(node* a, node* b, node* c)
{
	if(a == b || b == c || c == a)
		0;

	if(TriangleAlreadyExists(a,b,c))
		0;
}

int AddTriangle(node* a, node* b, node* c)
{
	struct triangle *newTriangle; 
	newTriangle = (node*)(malloc( sizeof(struct triangle) ));  
	newTriangle->a = a;
	newTriangle->b = b;
	newTriangle->c = c;
	newTriangle->next = 0;

	

	if(!global.headTriangle)
	{
		global.headTriangle = newTriangle;
		global.tailTriangle = global.headTriangle;
		global.triangleCount++;
	}
	else
	{
		global.tailTriangle->next = newTriangle; 
		global.tailTriangle = newTriangle;
		global.triangleCount++;
	}
	Draw_Triangle(newTriangle);
	return 1;
}

node* RemoveTriangle(triangle *dest)
{
	struct triangle *current = global.headTriangle;

	//if its the head
	if(current->a == dest->a &&
				current->b == dest->b &&
				current->c == dest->c)
	{
		global.headTriangle = global.headTriangle->next;
		global.triangleCount--;
		free(current);
		return;
	}

	if(dest)
	{
		while(current)
		{
			if(current->next->a == dest->a &&
				current->next->b == dest->b &&
				current->next->c == dest->c)
			{
				global.triangleCount--;
				current->next = current->next->next;
				free(dest);
				return current->next;
			}
			current = current-> next;
		}
	}
	else
		return NULL;

	if(!dest) return NULL;
}

edge* AddEdge(node *start, node* end)
{
	struct edge *newEdge; 
	newEdge = (edge*)(malloc( sizeof(struct edge) ));  
	newEdge->next = 0;
	newEdge->start = start;
	newEdge->end = end;

	if(start->x == end->x && start->y == end->y) return NULL;

	if(!global.headEdge)
	{
		global.headEdge = newEdge;
		global.tailEdge = global.headEdge;
	}
	else
	{
		global.tailEdge->next = newEdge; 
		global.tailEdge = newEdge;
	}

	return newEdge;
}

void RemoveEdge(edge *e)
{
	edge* current = global.headEdge;
	//if its the head
	if(current->start == e->start &&
				current->end == e->end)
	{
		global.headEdge = global.headEdge->next;
		free(current);
		return;
	}
	//else iterate to find it
	if(e)
	{
		while(current)
		{
			if(current->next->start == e->start &&
				current->next->end == e->end)
			{
				current->next = current->next->next;
				free(e);
				return;
			}
			current = current-> next;
		}
	}
}

int InCluster(node *point, node*mid)
{
	node* current = global.headCluster;
	int currentDistance = pow(((double)(point->x - mid->x)),2)+pow(((double)(point->y - mid->y)),2);

	while(current)
	{
		if(currentDistance > pow(((double)(point->x - current->x)),2)+pow(((double)(point->y - current->y)),2))
			return -1;
		current = current->next;
	}

	return 1;
}

node* selectNPoint(int n)
{
	node* current = global.headNode;
	int i;
	for(i = 0; i < n; i++)
		current = current->next;

	return current;
}


void GenerateNPoints(int n)
{
	int i;
	for(i = 0; i < n; i++)
	{
		AddPoint((rand() % global.w), (rand() % global.h));
	}
}

void CopyPoints()
{
	node* points = global.headNode;
	int x, y;
	
	glColor3f(1,1,1); 
    glPointSize(3.0);
	glBegin(GL_POINTS);
	
	while(points)
	{
		glVertex2f(points->x , points->y);
		points = points->next;
	}
	
	glEnd();

	glFlush();
}

void initScreen()
{
	int x,y;
	global.w = 1000; 
	global.h = 800;
	global.pointCount = 0;
	global.numClusters = 0;
	global.data = (pixel *)malloc((global.h)*(global.w)*sizeof(pixel));
	for(x = 0; x < global.w; x++)
	{
		for(y = 0; y < global.h; y++)
		{
			global.data[(x)+(y*global.w)].r = 0;
			global.data[(x)+((y)*global.w)].g = 0;
			global.data[(x)+((y)*global.w)].b = 0;
		}
	}
}

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
	CopyPoints();
	glFlush();
}//display_image()

void Quick_Hull(node* head, edge* line)
{
	node* point = head;
	node* clusters;
	node* max = NULL;
	int maxDistance = -1;
	int d, minD, current;
	while(point)
	{
		if(point->drawn == 0)
		{
			if((point->x == line->start->x && point->y == line->start->y) 
				|| (point->x == line->end->x && point->y == line->end->y))
			{
				point = point->next;
				continue;
			}

			if(line->start == line->end)
				d = LengthOfLine(line->start, point);
			else
				d = Distance(line, point);
			
			if(d > maxDistance)
			{
				if((d == 0) && (LengthOfLine(line->start, point) + 
					LengthOfLine(point, line->end)) != LengthOfLine(line->start, line->end))
				{
					point = point->next;
					continue;
				}
				max = point;
				maxDistance = d;
			}
		}

		point = point->next;
	}

	if(maxDistance >= 0)
	{
		edge* one = AddEdge(line->start,max);
		edge* two = AddEdge(max,line->end);
		RemoveEdge(line);

		Quick_Hull(head, one);
		Quick_Hull(head, two);
	}
}



node* MinPoint(node *mid)
{
	node* current = mid;
	node* min = NULL;
	while(current)
	{
		if((!min || current->y < min->y) && current->drawn == 0)
			min = current;

		current = current->next;
	}
	return min;
}

node* MaxPoint(node *mid)
{
	node* current = mid;
	node* max = NULL;
	while(current)
	{
		if((!max || current->y > max->y) && current->drawn == 0)
			max = current;

		current = current->next;
	}
	return max;
}


void Draw_Edges()
{
	edge* next;
    glPointSize(4.0);  
	while(global.headEdge)
	{
		glBegin(GL_LINES);	
		glVertex2d(global.headEdge->start->x, global.headEdge->start->y);
		glVertex2d(global.headEdge->end->x, global.headEdge->end->y);
		glEnd();

		//remove it from the next hull
		global.headEdge->start->drawn++;
		global.headEdge->end->drawn++;	

		RemoveEdge(global.headEdge);
	
	}
	glFlush();	
	printf("Point Count: %d\n",global.pointCount);
}

int PointInHull(node* point)
{
	edge* lines = global.headEdge;
	if(point->x == lines->start->x && point->y == lines->start->y)
		return 1;

	while(lines)
	{
		if(point->x == lines->end->x && point->y == lines->end->y)
		return 1;

		lines = lines->next;
	}

	return 0;
}

node* findCenterOfGravity(edge* start)
{
	//find the bounding square
	int maxX, maxY, minX, minY, leastDistance, d, inHull;	
	node* point;
	node* tempPoint;
	node* closestPoint;
	node* points = global.headNode;
	edge* lines = start;

	maxX = NULL;
	maxY = NULL;
	minX = NULL;
	minY = NULL;
	tempPoint = (node*)(malloc( sizeof(struct node) )); 
	leastDistance = NULL;

	//go through each point and find the min/max values
	while(lines)
	{
		point = lines->start;
		if(minX == NULL || minX > point->x) minX = point->x;
		if(minY == NULL || minY > point->y) minY = point->y;
		if(maxX == NULL || maxX < point->x) maxX = point->x;
		if(maxY == NULL || maxY < point->y) maxY = point->y;
		lines = lines->next;
	}

	//find the center of the bounding square
	tempPoint->x = minX+((maxX-minX)/2);
	tempPoint->y = minY+((maxY-minY)/2);

	//return closest point not on the hull
	while(points)
	{
		if(PointInHull(points))
		{			
			points = points->next;
			continue;
		}
		d = LengthOfLine(points,tempPoint);
		if(leastDistance == NULL || d < leastDistance)
		{
			leastDistance = d;
			closestPoint = points;
		}
		points = points->next;
	}

	free(tempPoint);

	return closestPoint;

}


int SharedPoints(triangle* t, triangle* t2)
{
	int count = 0;
	if(t->a == t2->a || t->a == t2->b || t->a == t2->c) count++;
	if(t->b == t2->a || t->b == t2->b || t->b == t2->c) count++;
	if(t->c == t2->a || t->c == t2->b || t->c == t2->c) count++;
	return count;
}

int OppositeSign(int a, int b)
{
	if((a > 0 && b < 0) || (a < 0 && b > 0))
		return 1;
	else 
		return 0;
}

int Cleanup(triangle* tri)
{
	triangle* tempA;
	triangle* tempB;
	triangle* t = tri;
	triangle* iterator = tri;
	node* a = NULL;
	node* b = NULL;
	node* c = NULL;
	node* d = NULL;
	int changes = 0;
	int flag = 0;
	
	while(t)
	{
		printf("trying new triangle");
		iterator = global.headTriangle;
		while(iterator != NULL)
		{
			//check if has shared edges
			if(SharedPoints(t,iterator) == 2)
			{
				a = NULL;
				b = NULL;
				c = NULL;
				d = NULL;

				//determine the shared edges
				if(t->a == iterator->a || t->a == iterator->b || t->a == iterator->c){
					if(a == NULL) a = t->a;
					else if(b == NULL) b = t->a;
				}
				if(t->b == iterator->a || t->b == iterator->b || t->b == iterator->c){
					if(a == NULL) a = t->b;
					else if(b == NULL) b = t->b;
				}
				if(t->c == iterator->a || t->c == iterator->b || t->c == iterator->c){
					if(a == NULL) a = t->c;
					else if(b == NULL) b = t->c;
				}

				//determine ther edges
				if(t->a != a && t->a != b) c = t->a;
				if(t->b != a && t->b != b) c = t->b;
				if(t->c != a && t->c != b) c = t->c;

				
				if(iterator->a != a && iterator->a != b) d = iterator->a;
				if(iterator->b != a && iterator->b != b) d = iterator->b;
				if(iterator->c != a && iterator->c != b) d = iterator->c;

				//determine if new line is better
				if(LengthOfLine(c,d) < LengthOfLine(a,b))
				{
					if(OppositeSign(Distance2(c,d,a),Distance2(c,d,b)))
					{
						//if new line is better triangle and add new
						if(ValidTriangle(a,c,d) && ValidTriangle(b,c,d))
						{
							AddTriangle(a,c,d);
							AddTriangle(b,c,d);
							tempA = iterator->next;
							tempB = iterator->next;
							RemoveTriangle(iterator);
							RemoveTriangle(t);
							t = tempB;
							flag++;
							changes++;
							break;
						}
					}
				}			

				

			}

			iterator = iterator->next;
		}
		if(flag == 1)
		{
			flag = 0;
			continue;
		}
		t = t->next;
	}
	return changes;
}

int pointInTriangle(triangle* t, node* p)
{
	int a,b,c;

	a = Distance2(t->a,t->b,p);
	b = Distance2(t->b,t->c,p);
	c = Distance2(t->c,t->a,p);

	if((a == 0 && (LengthOfLine(t->a, p) + LengthOfLine(p, t->b)) == LengthOfLine(t->a, t->b)) ||
		(b == 0 && (LengthOfLine(t->b, p) + LengthOfLine(p, t->c)) == LengthOfLine(t->b, t->c)) ||
		(c == 0 && (LengthOfLine(t->c, p) + LengthOfLine(p, t->a)) == LengthOfLine(t->c, t->a)))
	{
		return 2;
	}

	//if inside
	if(a < 0 && b < 0 && c < 0)
		return 1;

	//if inside but done in wrong order
	if(a > 0 && b > 0 && c > 0)
		return 1;

	//outside
	return 0;
}


void Trisect(triangle* t)
{
	//findPointInBetween
	int minX, minY, maxX, maxY, minDistance, d;
	node* point;
	triangle* a,b,c;
	node* tempPoint;
	node* points = global.headNode;	
	minDistance = NULL;
	point = NULL;
	tempPoint = (node*)(malloc( sizeof(struct node) )); 
	minX = Min(t->a->x, t->b->x, t->c->x);
	minY = Min(t->a->y, t->b->y, t->c->y);
	maxX = Max(t->a->x, t->b->x, t->c->x);
	maxY = Max(t->a->y, t->b->y, t->c->y);
	tempPoint->x = (t->a->x + t->b->x + t->c->x)/3;
	tempPoint->y = (t->a->y + t->b->y + t->c->y)/3;

	while(points)
	{
		if(points->x > minX && points->y > minY && points->x < maxX && points->y < maxY &&
			points != t->a && points != t->b && points != t->c)
		{
			if(pointInTriangle(t, points) >= 1)
			{
				d = LengthOfLine(points,tempPoint);
				if(minDistance == NULL || d < minDistance)
				{
					minDistance = d;
					point = points;
				}
			}
		}
		points = points->next;
	}

	free(tempPoint);

	//If point is inbetween, delete triangle, add new and recurse
	if(point)
	{
		if(pointInTriangle(t,point) == 2)
		{
			//point on edge of triangle
			if(Distance2(t->a,t->b,point) == 0)
			{
				if(AddTriangle(t->a, point, t->c))
					Trisect(global.tailTriangle);
				
				if(AddTriangle(t->b, point, t->c))	
					Trisect(global.tailTriangle);
				
				RemoveTriangle(t);
			}
			else if(Distance2(t->a,t->c,point) == 0)
			{
				if(AddTriangle(t->a, point, t->b))
					Trisect(global.tailTriangle);
				if(AddTriangle(t->b, point, t->c))	
					Trisect(global.tailTriangle);
				RemoveTriangle(t);
			}
			else if(Distance2(t->b,t->c,point) == 0)
			{
				if(AddTriangle(t->b, point, t->a))	
					Trisect(global.tailTriangle);
				if(AddTriangle(t->c, point, t->a))
					Trisect(global.tailTriangle);
				RemoveTriangle(t);
			}
		}
		else
		{
			//point in middle of triangle
			if(AddTriangle(t->a,t->b,point))		
				Trisect(global.tailTriangle);
			if(AddTriangle(t->b,t->c,point))		
				Trisect(global.tailTriangle);
			if(AddTriangle(t->c,t->a,point))		
				Trisect(global.tailTriangle);
			RemoveTriangle(t);
		}
	}
	//If not, resurse back up
}

void Triangulate(node *i)
{
	node* center;
	Convex_Hull(i,1);
	center = findCenterOfGravity(global.headEdge);

	while(global.headEdge)
	{
		AddTriangle(global.headEdge->start, global.headEdge->end, center);
		Trisect(global.tailTriangle);		
		RemoveEdge(global.headEdge);
	}
	while(1==1)
	{
		if(Cleanup(global.headTriangle) == 0)
			break;
	}
	CopyPoints();
}


int Convex_Hull(node *i, int trisection)
{
	while(1 == 1)
	{
		edge* bottomEdge;
		node* min = MinPoint(i);
		node* max = MaxPoint(i);

		if(min && max)
		{
			edge *top = AddEdge(min,max);
			edge *bottom = AddEdge(max,min);
		
			Quick_Hull(i, top);	
			bottomEdge = global.tailEdge;
			Quick_Hull(i, bottom);
		}

		if(!global.headEdge || trisection == 1)
			return;
		Draw_Edges();
	}
}

int Min(int a, int b, int c)
{
	int min = a;
	if(b < min) min = b;
	if(c < min) min = c;
	return min;
}

int Max(int a, int b, int c)
{
	int max = a;
	if(b > max) max = b;
	if(c > max) max = c;
	return max;
}





int Cluster(int n)
{
	node *current;
	node *newPoint;
	node *list = NULL;
	node *points;
	while(global.numClusters < n)
	{
		AddCluster(selectNPoint(rand() % global.pointCount));
	}

	current = global.headCluster;
	while(current)
	{
		points = global.headNode;
		while(points)
		{
			if(InCluster(points,current) == 1 && points->drawn == 0)
			{ 
				newPoint = (node*)(malloc( sizeof(struct node) ));  
				newPoint->next = 0;
				newPoint->x = points->x;
				newPoint->y = points->y;
				newPoint->drawn = 0;

				if(!list)
					list = newPoint;
				else
					newPoint->next = list;
					list = newPoint;

					points->drawn = 1;
			}

			points = points->next;

		}
		glColor3ub(rand()%256,rand()%256,rand()%256); 

		Convex_Hull(list, 0);
		while(list)
		{
			points = list;
			list = list->next;
			free(points);
		}
		current = current->next;
	}
}

int Distance(edge* e, node* pointC)
{
	int ABx = e->end->x - e->start->x;
    int ABy = e->end->y - e->start->y;
    int num = ABx * (e->start->y - pointC->y) - ABy * (e->start->x - pointC->x);
    return num;
}

int Distance2(node* pointA, node* pointB, node* pointC)
{
	int ABx = pointB->x - pointA->x;
    int ABy = pointB->y - pointA->y;
    int num = ABx * (pointA->y - pointC->y) - ABy * (pointA->x - pointC->x);
    return num;
}

int LengthOfLine(node* pointA, node* pointB)
{
	return sqrt(pow(pointA->x - pointB->x, 2) + pow(pointA->y - pointB->y,2));
}

void Grid(int n)
{
	int i, j;

	for(i = 0; i < (int)global.w/n; i++)
	{
		for(j = 0; j < (int)(global.h/n); j++)
		{
			AddPoint(i*n, j*n);
		}
	}
}




/*glut keyboard function*/
void keyboard(unsigned char key, int x, int y)
{
	int n;
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
		case'r':
		case'R':
			printf("\n\n\nEnter the number of vectors you would like:");
			scanf("%d", &n);
			GenerateNPoints(n);
			CopyPoints();
			break;
		case 'c':
		case 'C':
			glColor3f(0.0,0.4,0.1); 
			Convex_Hull(global.headNode, 0);
			break;			
		case 'h':
		case 'H': 
			printf("\n\n\nEnter the number of clusers you would like:");
			scanf("%d", &n);
			Cluster(n);
			break;			
		case 'g':
		case 'G': 
			Grid(20);
			CopyPoints();
			break;					
		case 'm':
		case 'M': 
			if(global.mouse == 1)
				global.mouse = 0;
			else
				global.mouse = 1;
			break;				
		case 't':
		case 'T': 
			Triangulate(global.headNode);
			//DrawAllTriangles();
			break;

	}
}//keyboard

void menuItems(int num)
{
	switch(num)
	{
		case 0:
			GenerateNPoints(100);
			CopyPoints();
			break;
		case 1:
			GenerateNPoints(200);
			CopyPoints();
			break;
		case 2:
			GenerateNPoints(500);
			CopyPoints();
			break;
		case 3:
			GenerateNPoints(1000);
			CopyPoints();
			break;
		case 4:
			glColor3f(0.0,0.4,0.1); 
			Convex_Hull(global.headNode,0);
			break;
	}
}




void createMenu()
{ 
	int menu_id = glutCreateMenu(menuItems);
    glutAddMenuEntry("Add 100 random points", 0);
    glutAddMenuEntry("Add 200 random points", 1);
    glutAddMenuEntry("Add 500 random points", 2);
    glutAddMenuEntry("Add 1000 random points", 3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

}

void AddNewVertex(int button, int state, int x, int y)
{
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		printf("new vertex at x=%d and y=%d\n",x,y);
		if(global.mouse == 1)
		{
			AddPoint(x,(global.h-y));
		}
	}
}

int main(int argc, char** argv)
{
	initScreen();
	global.mouse = 0;
	if (global.data==NULL)
	{
		printf("Error loading image file img.tif\n");
		return 1;
	}
	printf("Q:quit\nS:save\nR:Random Point\nC:Convex Hull");
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGB|GLUT_SINGLE);
	
	glutInitWindowSize(global.w,global.h);
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glutCreateWindow("SIMPLE DISPLAY");
	glShadeModel(GL_SMOOTH);
	glutMouseFunc(AddNewVertex);
	glutDisplayFunc(display_image);
	glutKeyboardFunc(keyboard);
	glMatrixMode(GL_PROJECTION);
	glOrtho(0,global.w,0,global.h,0,1);
	createMenu();
	
	glutMainLoop();

	return 0;
}
