//
//  main.cpp
//  cgtest
//
//  Created by plewis on 7/15/13.
//  Copyright (c) 2013 plewis. All rights reserved.
//

#include <iostream>
#include <GLUT/GLUT.h>
#include <Cg/cgGL.h>
#include <Cg/cgGL.h>
#include <unistd.h>
#include <math.h>

void display(void);
void reshape(int, int);
void OnIdle(void);
void mouseMove(int, int);
void mouseAction(int, int, int, int);
void keyAction(unsigned char, int, int);
void keyUp(unsigned char key, int x, int y);
CGprogram julia;
CGprogram newton;
CGprogram mandel;
CGprogram ship;
CGprogram gen;
CGprogram pass;
float delta = 0.0;
float _w, _h;
int winSze = 500;
float speed = 0.002;
float mouseX = 1.0;
float mouseY = 1.0;
bool mouse_l_down = false;
bool mouse_r_down = false;
float zoom = 2.0;
float centerX = 0.0;
float centerY = 0.0;
float lastZoom = zoom;
float lastCenterX = centerX;
float lastCenterY = centerY;
float lastMouseX = mouseX;
float lastMouseY = mouseY;
float startDragX, startDragY;
float cursorX, cursorY;
bool zoomIn = false;
bool zoomOut = false;
bool panL = false;
bool panR = false;
bool panU = false;
bool panD = false;
bool mouseL = false;
bool mouseR = false;
bool mouseU = false;
bool mouseD = false;

float var1 = 0.0;
float var2 = 0.0;
float var3 = 0.0;

bool var1up = false;
bool var2up = false;
bool var3up = false;
bool var1down = false;
bool var2down = false;
bool var3down = false;

bool hunting = false;

bool h1, h2, h3 = false;

bool fadeStart = true;

int curScene1, curScene2;
int numScenes;

CGprofile f_prof, v_prof;

struct Position{
	float px;
	float py;
	float z;
	float v1;
};

void fillPoint(Position *p, char *s)
{
    int i = 0;
    //Ignore point number
    while(s[i++] != ' ');
	p->px = atof(s + i);
    while(s[i++] != ' ');
	p->py = atof(s + i);
    while(s[i++] != ' ');
	p->z = atof(s + i);
    while(s[i++] != ' ');
	p->v1 = atof(s + i);
}

Position posList[25];
int posIndex = 0;
int posLen = 0;
float ptTime = 3.0;

FILE *pts;
char fileName[11];
char progName[10];

CGcontext context;

//a sanity check - two high-zoom points shouldn't be adjacent unless they're very close
bool isWellOrdered(Position *pl)
{
    Position pos, posN;
    for(int i = 0; i < posLen - 1; i++){
        pos = pl[i];
        posN = pl[i+1];
        if( pos.z + posN.z < 0.02 && (abs(pos.px - posN.px) > 0.1 || abs(pos.py - posN.py) > 0.1))
            return false;
    }
    return true;
}

void newScenes(int sc1, int sc2)
{
    sprintf(fileName, "cg_%d_%d.txt", sc1, sc2);
    fileName[10] = '\0';
    
    if((pts = fopen(fileName, "r")) == NULL){
        printf("Couldn't open file: %s\n", fileName);
        exit(EXIT_FAILURE);
    }
    char *ln;
    size_t sz;
	posLen = 0;
    while((ln = fgetln(pts, &sz)) != NULL)
    {
        fillPoint(&(posList[posLen]), ln);
        posLen++;
    }
    fclose(pts);
	
	sprintf(progName, "cg_%d_%d.cg", sc1, sc2);
	cgGLUnbindProgram(f_prof);
	cgGLEnableProfile(f_prof);
	gen = cgCreateProgramFromFile(context, CG_SOURCE, progName, f_prof, "main", NULL);
	cgGLLoadProgram(gen);
	cgGLBindProgram(gen);
	cgGLEnableProfile(f_prof);
}

int main( int argc, char **argv )
{    
    //Initialize Window
    glutInit( &argc, argv );
    glutInitWindowSize(winSze, winSze);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow("Fractal Shader");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMotionFunc(mouseMove);
    glutMouseFunc(mouseAction);
    glutIdleFunc(OnIdle);
    glutKeyboardFunc(keyAction);
    glutKeyboardUpFunc(keyUp);
    glClearColor(0, 0, 0, 1);
    context = cgCreateContext();
    if(context == NULL){
        printf("Failed to create CGcontext\n");
        return 1;
    }
	
	//Profile set-up
	f_prof = cgGLGetLatestProfile(CG_GL_FRAGMENT); 
	v_prof = cgGLGetLatestProfile(CG_GL_VERTEX);
    cgGLEnableProfile(f_prof);
    cgGLEnableProfile(v_prof);
	/*
	printf("%d, %d\n", f_prof, v_prof);
    julia = cgCreateProgramFromFile(context, CG_SOURCE, "Julia.cg", f_prof, "main", NULL);
    newton = cgCreateProgramFromFile(context, CG_SOURCE, "Newton.cg", f_prof, "main", NULL);
    mandel = cgCreateProgramFromFile(context, CG_SOURCE, "Mandel.cg", f_prof, "main", NULL);
    ship = cgCreateProgramFromFile(context, CG_SOURCE, "Ship.cg", f_prof, "main", NULL);
    pass = cgCreateProgramFromFile(context, CG_SOURCE, "Pass.cg", v_prof, "main", NULL);
	gen = cgCreateProgramFromFile(context, CG_SOURCE, "GenIter.cg", f_prof, "main", NULL);
    printf("%s\n", cgGetErrorString(cgGetError()));
    cgGLLoadProgram(julia);
    cgGLLoadProgram(newton);
    cgGLLoadProgram(mandel);
    cgGLLoadProgram(ship);
	cgGLLoadProgram(gen);
    cgGLLoadProgram(pass);
    //printf("%s\n", cgGetErrorString(cgGetError()));
    cgGLEnableProfile(f_prof);
    cgGLEnableProfile(v_prof);
    printf("%s\n", cgGetErrorString(cgGetError()));
	*/
	
	//Use throughput vertex shader
    chdir("/Users/peter/Desktop/CGDemo");
    pass = cgCreateProgramFromFile(context, CG_SOURCE, "Pass.cg", v_prof, "main", NULL);
	cgGLLoadProgram(pass);
    cgGLBindProgram(pass);
    cgGLSetParameter1f(cgGetNamedParameter(pass, "startX"), -1);
    cgGLSetParameter1f(cgGetNamedParameter(pass, "endX"), 1);
    cgGLSetParameter1f(cgGetNamedParameter(pass, "startY"), -1);
    cgGLSetParameter1f(cgGetNamedParameter(pass, "endY"), 1);
   
	 //Pick scenes
	curScene1 = 0;
	curScene2 = 1;
	numScenes = 6;
	newScenes(curScene1, curScene2);
    
    //Randomize point list
    /*
    srand(time(NULL));
    //printf("%d\n", rand() % posLen);
    
    int pos1, pos2;
    Position p;
    for(int i = 0; i < 100; i++){
        pos1 = rand() % posLen;
        pos2 = rand() % posLen;
        p = posList[pos1];
        posList[pos1] = posList[pos2];
        posList[pos2] = p;
    }
    while(!isWellOrdered(posList)){
        pos1 = rand() % posLen;
        pos2 = rand() % posLen;
        p = posList[pos1];
        posList[pos1] = posList[pos2];
        posList[pos2] = p;
    }
	*/
	
	//cgGLBindProgram(gen);
	
    if(posLen > 0){
        Position pos = posList[0];
        centerX = pos.px;
        centerY = pos.py;
        zoom = pos.z;
        var1 = pos.v1;
    }
    
    glutMainLoop();
    return 0;
}

void huntPrint()
{
	printf("Position: %f, %f\n", centerX, centerY);
	printf("Zoom: %f\n", zoom);
	printf("Var1: %f\n", var1);
    fprintf(pts, "%d: %f %f %f %f\n", ++posLen, centerX, centerY, zoom, var1);
}

void keyAction(unsigned char key, int x, int y)
{
    
    switch (key) {
        case ';':
            zoomIn = true;
            break;
            
        case '/':
            zoomOut = true;
            break;
        
        case 'a':
            panL = true;
            break;
            
        case 's':
            panD = true;
            break;
            
        case 'd':
            panR = true;
            break;
            
        case 'w':
            panU = true;
            break;
        
        case 'j':
            mouseL = true;
            break;
            
        case 'k':
            mouseD = true;
            break;
            
        case 'l':
            mouseR = true;
            break;
            
        case 'i':
            mouseU = true;
            break;
			
		case '1':
			var1up = true;
			break;
			
		case '2':
			var1down = true;
			break;
			
		case '9':
			var2up = true;
			break;
			
		case '0':
			var2down = true;
			break;
			
		case ' ':
            if(hunting)
                huntPrint();
			break;
            
        case 'h':
            h1 = true;
            break;
            
        case 'u':
            h2 = h1;
            break;
        
        case 'n':
            h3 = h2;
            break;
            
        case 't':
            if(!hunting && h3){
                printf("Point hunting activated!\n");
                pts = fopen(fileName, "a");
                hunting = true;
            }
            break;
			
		case '.':
			posIndex++;
			break;
			
		case ',':
			posIndex > 0 ? posIndex-- : 1;
            
        default:
            break;
    }
}
           
void keyUp(unsigned char key, int x, int y)
{
    switch (key) {
        case ';':
            zoomIn = false;
            break;
            
        case '/':
            zoomOut = false;
            break;
            
        case 'a':
            panL = false;
            break;
            
        case 's':
            panD = false;
            break;
            
        case 'd':
            panR = false;
            break;
            
        case 'w':
            panU = false;
            break;
            
        case 'j':
            mouseL = false;
            break;
            
        case 'k':
            mouseD = false;
            break;
            
        case 'l':
            mouseR = false;
            break;
            
        case 'i':
            mouseU = false;
            break;
			
		case '1':
			var1up = false;
			break;
			
		case '2':
			var1down = false;
			break;
			
		case '9':
			var2up = false;
			break;
			
		case '0':
			var2down = false;
			break;
            
        default:
            break;
    }
}

void updateMouse(int x, int y, float *nx, float *ny)
{
    *nx = (float)x * 2.0 / (float)winSze - 1.0;
    *ny = -1*((float)y * 2.0 / (float)winSze - 1.0);
}

void mouseAction(int button, int state, int x, int y)
{
    if(button == GLUT_RIGHT_BUTTON){
        if(state == GLUT_DOWN){
            mouse_r_down = true;
            updateMouse(x, y, &cursorX, &cursorY);
            startDragX = cursorX;
            startDragY = cursorY;

        }
        else if(state == GLUT_UP){
            mouse_r_down = false;
            lastZoom = zoom;
            lastCenterX = centerX;
            lastCenterY = centerY;
        }
    }
    else if(button == GLUT_LEFT_BUTTON){
        if(state == GLUT_DOWN){
            mouse_l_down = true;
			updateMouse(x, y, &cursorX, &cursorY);
			startDragX = cursorX;
			startDragY = cursorY;
        }
        else if(state == GLUT_UP){
            mouse_l_down = false;
            lastCenterX = centerX;
            lastCenterY = centerY;
			lastMouseX = mouseX;
			lastMouseY = mouseY;
        }
    }
}

void mouseMove(int x, int y)
{
    if(mouse_l_down && !mouse_r_down){
        updateMouse(x, y, &cursorX, &cursorY);
		mouseX = lastMouseX - (zoom/5.0) * (cursorX - startDragX);
		mouseY = lastMouseY - (zoom/5.0) * (cursorY - startDragY);
		printf("%f, %f\n", mouseX, mouseY);
    }
    if(mouse_r_down){
        updateMouse(x, y, &cursorX, &cursorY);
        centerX = lastCenterX - pow(zoom, 0.1) * (cursorX - startDragX);
        centerY = lastCenterY - pow(zoom, 0.1) * (cursorY - startDragY);
        //printf("%f, %f\n", centerX, centerY);
    }
}

void updateCamera()
{
	Position pos = posList[posIndex];
    Position posNext = posList[posIndex + 1];
	float prog = delta / ptTime;
    if(curScene1 == 3)
		prog = 0.5*(1 - cos(M_PI*prog));
    //prog = 0.5*(cbrt(2.0*prog - 1.0) + 1.0);
	//prog = pow(prog, 1.0 / (38.0*zoom));
    float progN = 1.0 - prog;
    centerX = prog * posNext.px + progN * pos.px;
    centerY = prog * posNext.py + progN * pos.py;
    var1 = prog * posNext.v1 + progN * pos.v1;
    
    zoom = prog * posNext.z + progN * pos.z;
    
    if(prog >= 0.999)
    {
		printf("Point %d Reached\n", posIndex + 2);
        //printf("Point Reached: %f %f %f %f\n", posNext.px, posNext.py, posNext.z, posNext.v1);
        delta = 0.0;
        posIndex += 1;
    }
    
    //Suppose we try a different approach, based more on consistent steady movement
    
//    float disX, disY, disZ, disV;
//    disX = posNext.px - centerX;
//    disY = posNext.py - centerY;
//    disV = posNext.v1 - var1;
//    disZ = posNext.z / zoom;
//
//    if(abs(disX) > zoom)
//        centerX += 0.1*zoom * disX / (abs(disX));
//    if(abs(disY) > zoom)
//        centerY += 0.1*zoom * disY / (abs(disY));
//    if(abs(1 - disZ) > 0.001)
//        zoom += 0.1*zoom * disZ / (abs(disZ));
//    if(abs(disV) > zoom)
//        var1 += zoom * disV / (abs(disV));
//    
//    if(abs(disX) + abs(disY) + abs(disV) + abs(1 - disZ) < 0.001)
//    {
//        printf("Point Reached: %f %f %f %f\n", posNext.px, posNext.py, posNext.z, posNext.v1);
//        delta = 0.0;
//        posIndex += 1;
//    }
    
}

void OnIdle(void)
{
    if(fadeStart){
		delta += speed/20;
		if(centerX < 0.29)
			fadeStart = false;
	}
	else delta += speed;
	var2 += 2*speed;
    if(zoomIn){
        zoom *= 0.99;
    }
    else if(zoomOut){
        zoom *= 1.02;
    }
    if(panL){
        centerX -= zoom/80;
    }
    else if(panR){
        centerX += zoom/80;
    }
    if(panU){
        centerY += zoom/80;
    }
    else if(panD){
        centerY -= zoom/80;
    }
    if(mouseL){
        mouseX -= zoom/80;
    }
    else if(mouseR){
        mouseX += zoom/80;
    }
    if(mouseU){
        mouseY += zoom/80;
    }
    else if(mouseD){
        mouseY -= zoom/80;
    }
	if(var1up){
		var1 += 0.001;
	}
	else if(var1down){
		var1 -= 0.001;
	}
	if(var2up){
		var2 += 0.001;
	}
	else if(var2down){
		var2 -= 0.001;
	}
    //sleep(0.1);
	
	if(!hunting){
		if(posIndex + 1 < posLen)
			updateCamera();
		else if(curScene2 < numScenes){
			delta = 0.0;
			posIndex = 0;
			var1 = 0.0;
			curScene1++;
			curScene2++;
			printf("Changing to scenes %d, %d\n", curScene1, curScene2);
			newScenes(curScene1, curScene2);
		}
	}
	
    glutPostRedisplay();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);    
//	cgGLSetParameter1f(cgGetNamedParameter(julia, "zoom"), zoom);
//	cgGLSetParameter2f(cgGetNamedParameter(julia, "zc"), centerX, centerY);
//    cgGLSetParameter2f(cgGetNamedParameter(julia, "c"), mouseX, mouseY);
//    cgGLSetParameter2f(cgGetNamedParameter(ship, "c"), mouseX, mouseY);
//    cgGLSetParameter2f(cgGetNamedParameter(ship, "zc"), centerX, centerY);
//    cgGLSetParameter1f(cgGetNamedParameter(ship, "delta"), 5*delta);
//    cgGLSetParameter1f(cgGetNamedParameter(ship, "zoom"), zoom);
//    cgGLSetParameter1f(cgGetNamedParameter(newton, "zoom"), zoom);
//    cgGLSetParameter2f(cgGetNamedParameter(newton, "f"), mouseX, mouseY);
//    cgGLSetParameter2f(cgGetNamedParameter(newton, "zc"), centerX, centerY);
//	cgGLSetParameter1f(cgGetNamedParameter(mandel, "zoom"), zoom);
//	cgGLSetParameter2f(cgGetNamedParameter(mandel, "zc"), centerX, centerY);
//	cgGLSetParameter1f(cgGetNamedParameter(gen, "zoom"), zoom);
//	cgGLSetParameter2f(cgGetNamedParameter(gen, "zc"), centerX, centerY);
//	cgGLSetParameter1f(cgGetNamedParameter(gen, "var1"), var1);
//	cgGLSetParameter1f(cgGetNamedParameter(gen, "var2"), var2);
	/*
    //General shader viewer
    cgGLBindProgram(newton);
    glBegin(GL_QUADS);
    glVertex2f(-1, -1);
    glVertex2f(1, -1);
    glVertex2f(1, 1);
    glVertex2f(-1, 1);
    glEnd();
	cgGLDisableProgramProfiles(newton);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD_EXT);
	cgGLBindProgram(gen);
    glBegin(GL_QUADS);
    glVertex2f(-1, -1);
    glVertex2f(1, -1);
    glVertex2f(1, 1);
    glVertex2f(-1, 1);
    glEnd();
	glDisable(GL_BLEND);
    */
	
	//DEMO
	cgGLSetParameter1f(cgGetNamedParameter(gen, "zoom"), zoom);
	cgGLSetParameter2f(cgGetNamedParameter(gen, "zc"), centerX, centerY);
	cgGLSetParameter1f(cgGetNamedParameter(gen, "var1"), var1);
	cgGLSetParameter1f(cgGetNamedParameter(gen, "var2"), var2);
    glBegin(GL_QUADS);
    glVertex2f(-1, -1);
    glVertex2f(1, -1);
    glVertex2f(1, 1);
    glVertex2f(-1, 1);
    glEnd();
	
    /*
    //Mandelbrot viewer with mini Julia set
    cgGLBindProgram(julia);
    glBegin(GL_QUADS);
    glVertex2f(-1, -1);
    glVertex2f(1, -1);
    glVertex2f(1, 1);
    glVertex2f(-1, 1);
    glEnd();
    cgGLSetParameter1f(cgGetNamedParameter(pass, "startX"), 0.25);
    cgGLSetParameter1f(cgGetNamedParameter(pass, "endX"), 1);
    cgGLSetParameter1f(cgGetNamedParameter(pass, "startY"), -0.25);
    cgGLSetParameter1f(cgGetNamedParameter(pass, "endY"), -1);
    cgGLBindProgram(julia);
    glBegin(GL_QUADS);
    glVertex2f(0.25, -0.25);
    glVertex2f(1, -0.25);
    glVertex2f(1, -1);
    glVertex2f(0.25, -1);
    glEnd();
    */
    /*
    //Julia and Newton quads
    cgGLBindProgram(julia);
    glBegin(GL_QUADS);
    glVertex2f(-1, -1);
    glVertex2f(-1, 0);
    glVertex2f(0, 0);
    glVertex2f(0, -1);
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glVertex2f(1, 1);
    glVertex2f(0, 1);
    glEnd();
    cgGLBindProgram(newton);
    glBegin(GL_QUADS);
    glVertex2f(-1, 0);
    glVertex2f(0, 0);
    glVertex2f(0, 1);
    glVertex2f(-1, 1);
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glVertex2f(1, -1);
    glVertex2f(0, -1);
    glEnd();
    */
    /*
    //Interactive Newton
    cgGLBindProgram(newton);
    cgGLSetParameter1f(cgGetNamedParameter(newton, "delta"), 0.0);
    cgGLSetParameter1f(cgGetNamedParameter(newton, "fx"), mouseX);
    cgGLSetParameter1f(cgGetNamedParameter(newton, "fy"), mouseY);
    glBegin(GL_QUADS);
    glVertex2f(-1, -1);
    glVertex2f(1, -1);
    glVertex2f(1, 1);
    glVertex2f(-1, 1);
    glEnd();
    */
    glutSwapBuffers();
}

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
}

