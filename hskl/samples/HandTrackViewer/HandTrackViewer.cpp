// The aim of this sample is to show a well-documented example of how to use our tracking with minimal code
#include "hsklu.h" // Tracking library API + 3D depth sensor support
#include <gl/freeglut.h> // For rendering output (OpenGL/FreeGLUT)
#include <cstdarg>
#include <cstdio>
#include <math.h>
#include "..\RtMIDI\RtMidi.h"
#include "winsock2.h"
#include <stdlib.h>
#pragma comment(lib,"ws2_32.lib")
#include <iostream>
#include <windows.h>
#include <process.h>

using namespace std;

hskl::Tracker	g_tracking;								// Tracking library context
float			g_handWidth  = 0.0683f;					// Width of hand, assume 8 cm to start
float			g_handLength = 0.1498f;					// Length of hand, assume 19 cm to start
const char *	g_profileString = "left hand";			// Description of which hand is being tracked

int				g_winWidth, g_winHeight;				// Width and height of the rendering window
const float		g_nearZ = 0.01f, g_farZ = 2.0f;			// Near and far clip distances for rendering
int				g_time0, g_time1, g_numFrames, g_fps;	// Timing control variables
bool			g_layers[4] = {true,false,false,false};	// Rendering layers

RtMidiOut *midiout = new RtMidiOut();
std::vector<unsigned char> message;
void initializeData();
void checkLocation();
void checkPosition();
void checkFinger(int region);
void midiTable(int num,int region);
void getData(void* p);
void mideOutPut(int btnNum,int level);
int	fingerNum,btnNum;
bool musicControl[3][7];
// monitor the status of returning functions
int flag;     
int bytesRecv = SOCKET_ERROR;
char sendbuf[32] = "Server: Sending Data.";
char recvbuf[32] = "";
SOCKET m_socket;
//music Temp
bool MusicTemp;
int NowRegion,NowNum;
char *Data1 = new char[99];

void UpdateTracking()
{
	g_tracking.Update();
	if(++g_numFrames == 4) { g_time1 = glutGet(GLUT_ELAPSED_TIME); g_fps = 1000 * g_numFrames / (g_time1-g_time0); g_time0 = g_time1; g_numFrames = 0; }
	glutPostRedisplay();
}

void OnKeyboard(unsigned char ch, int x, int y)
{
	// If user hit escape, clean up properly and quit
	if(ch == 27)
	{
		exit(0);
	}

	// Otherwise let user resize the hand based on two scaling factors
	switch(ch)
	{
	case 'i': initializeData(); break;
	case 'w': g_handLength *= 1.02f; break;
	case 'a': g_handWidth  /= 1.02f; break;
	case 's': g_handLength /= 1.02f; break;
	case 'd': g_handWidth  *= 1.02f; break;
	case 'r': g_tracking.SetModelType(HSKL_MODEL_RIGHT_HAND); g_profileString = "right hand"; break;
	case 'l': g_tracking.SetModelType(HSKL_MODEL_LEFT_HAND ); g_profileString = "left hand";  break;
	case 't': g_tracking.SetModelType(HSKL_MODEL_TWO_HAND  ); g_profileString = "both hands"; break;
	case '1': case '2': case '3': case '4': g_layers[ch-'1'] = !g_layers[ch-'1']; break;
	}
	g_tracking.SetHandMeasurements(g_handWidth, g_handLength);
}

// Utility function to write a printf style string on the screen
void DrawString(int x, int y, const char * format, ...)
{
	glRasterPos2i(x,y);
	va_list args; char buffer[1024];
	va_start(args, format);
	int n = vsnprintf(buffer, sizeof(buffer), format, args);
	for(int i=0; i<n; ++i) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[i]);
	va_end(args);
}
void drawLine()
{	
	
	glEnable( GL_DEPTH_TEST );
	double x1=0.05;
	glColor4f(1,0,0,0.8f);
	glBegin(GL_LINES);
		glVertex3f(0.4,0,0.4);
		glVertex3f(-0.4,0,0.4);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(0,0.4,0.4);
		glVertex3f(0,-0.4,0.4);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(x1,0.4,0.4);
		glVertex3f(x1,-0.4,0.4);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(x1*2,0.4,0.4);
		glVertex3f(x1*2,-0.4,0.4);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(x1*3,0.4,0.4);
		glVertex3f(x1*3,-0.4,0.4);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(x1*4,0.4,0.4);
		glVertex3f(x1*4,-0.4,0.4);
	glEnd();
	
	
	//checkPosition();
	checkLocation();

	

}
void initializeData()
{
	int bytesSent;
	
	bool check=true;
	int count=0;
// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR )
	printf("Error at WSAStartup()\n");

	// Create a socket.
	
	m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if ( m_socket == INVALID_SOCKET ) {
		printf( "Error at socket(): %ld\n", WSAGetLastError() );
		WSACleanup();
	}

	// Bind the socket.
	sockaddr_in service;

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	service.sin_port = htons( 9999 );

	if ( bind( m_socket, (SOCKADDR*) &service, sizeof(service) ) == SOCKET_ERROR ) {
		printf( "bind() failed.\n" );
		closesocket(m_socket);
	}

	// Listen on the socket.
	if ( listen( m_socket, 1 ) == SOCKET_ERROR )
		printf( "Error listening on socket.\n");

		// Accept connections.
		SOCKET AcceptSocket;

		printf( "Waiting for a client to connect...\n" );
		while (1) {
		AcceptSocket = SOCKET_ERROR;
		while ( AcceptSocket == SOCKET_ERROR ) {
		AcceptSocket = accept( m_socket, NULL, NULL );
		}
		printf( "Client Connected.\n");
		m_socket = AcceptSocket; 
		break;
	}
	_beginthread(getData, 0, (void*)Data1 );
}
void getData(void* p)
{
	while(true)
	{
		bytesRecv = recv( m_socket, recvbuf, 1, 0 );
		if(recvbuf[0]==1)	
			printf("open\n");
		else
			printf("close\n");
	}
	
}
void mideOutPut(int btnNum,int level)
{
	int musicNum=0;
	switch(btnNum)
	{
		case 1:
			musicNum=60;
			break;
		case 2:
			musicNum=62;
			break;
		case 3:
			musicNum=64;
			break;
		case 4:
			musicNum=65;
			break;
		case 5:
			musicNum=67;
			break;
		case 6:
			musicNum=69;
			break;
		case 7:
			musicNum=71;
			break;
	}
		if(level ==2)
			musicNum+=12;
		else if(level ==0)
			musicNum-=12;
		
		message.push_back (144);
		message.push_back (musicNum);
		message.push_back (90);
		midiout->sendMessage( &message );
		message.clear();
}
/*
void checkPosition()
{
	double posi_y[]={-0.1,-0.07,-0.04,-0.01,0.02,0.05,0.08,0.11,0.14,0.17,0.2,0.23};
	int finger_pos[]={4,7,10,13,16};
	for(int j=0;j<5;j++)
	{
		if(g_tracking.GetPosition(finger_pos[j]).z>0.38 && g_tracking.GetPosition(finger_pos[j]).z<0.42 )
		if(g_tracking.GetPosition(finger_pos[j]).x>-0.0125 && g_tracking.GetPosition(finger_pos[j]).x< 0.0125)
		{
			for(int i=0;i<11;i++)
			{
				if(g_tracking.GetPosition(finger_pos[j]).y>(posi_y[i]-0.0125)&& g_tracking.GetPosition(finger_pos[j]).y< posi_y[i+1]-0.0125)
				{
					btnNum=i;
					fingerNum=j;
					/*if(musicControl[i]&&temp)
					{
						mideOutPut(i);

						musicControl[i]=false;
						temp=false;
						//printf("close btn%d\n",i);
					}
					//mideOutPut(i);
				}	
			}
		}
		
		if(g_tracking.GetPosition(finger_pos[fingerNum]).z<0.38 || g_tracking.GetPosition(finger_pos[fingerNum]).z>0.42 &&
			g_tracking.GetPosition(finger_pos[fingerNum]).x<-0.0125 || g_tracking.GetPosition(finger_pos[fingerNum]).x> 0.0125 && 
			g_tracking.GetPosition(finger_pos[fingerNum]).y<(posi_y[btnNum]-0.0125)|| g_tracking.GetPosition(finger_pos[fingerNum]).y > posi_y[btnNum+1]-0.0125)
		{
			//musicControl[btnNum]=true;
			//printf("open btn%d\n",btnNum);
		}
		
				
	}
					
}*/
void drawRegion(int num)
{
	double tempX=0,tempX1=0;
	double tempY=0,tempY1=0.4;

	if(num%4==0){tempX=0.2;tempX1=0.15;}
	else if(num%4==1){tempX=0.15;tempX1=0.1;}
	else if(num%4==2){tempX=0.1;tempX1=0.05;}
	else{tempX=0.05;tempX1=0;}
	
	if(num<=3){tempY-=0.4;tempY1-=0.4;}

	glColor4f(1,1,0,0.8f);
	glBegin(GL_POLYGON);
		glVertex3f(tempX1,tempY,0.4);
		glVertex3f(tempX,tempY,0.4);
		glVertex3f(tempX,tempY1,0.4);
		glVertex3f(tempX1,tempY1,0.4);
	glEnd();
}
void checkLocation()
{
	double LineX[]={0.2,0.15,0.1,0.05,0};
	int NumOfRegion=0;
	glColor4f(1,1,0,0.8f);
	double a=-0.03;

	for(int i=0;i<4;i++)
	{
		if(g_tracking.GetPosition(1).x < LineX[i] && g_tracking.GetPosition(1).x > LineX[i+1])
		{
			if(g_tracking.GetPosition(1).y>0)
			{
				checkFinger(i+4);
				drawRegion(i+4);
			}
			else
			{
				checkFinger(i);
				drawRegion(i);
			}
		}
	}
	
	//return NumOfRegion;
}
void checkFinger(int region)
{
	//printf("region=%d \n",region);
	int fingerNum[]={4,7,10,13};
	double temp=0,max=0;
	int num=-1;
	for(int i=0;i<4;i++)
	{
		temp+=g_tracking.GetPosition(fingerNum[i]).z;
	}
	temp/=4;
	//max=g_tracking.GetPosition(fingerNum[0]).z-temp;
	for(int i=0;i<4;i++)
	{
		if(g_tracking.GetPosition(fingerNum[i]).z-temp>max)
		{
			max=g_tracking.GetPosition(fingerNum[i]).z-temp;
			num=i;
		}
		//printf("num=%d loca=%f de=%f\n",i,g_tracking.GetPosition(fingerNum[i]).z,g_tracking.GetPosition(fingerNum[i]).z-temp);
	}
	if(max>0.02)
	{
		//printf("num=%d region=%d \n",num,region);
		//if(musicControl[region][num])
		
			printf("num=%d region=%d \n",num,region);
			if(musicControl[region][num])
			{
				midiTable(num,region);
			}
			NowNum=num;
			NowRegion=region;
			
			musicControl[region][num]=false;
		
			//printf("%d num=%d region=%d at the same way\n",musicControl[region][num],num,region);
		
	}
	else
	{
		//printf("region=%d nowNum=%dnot in range\n",NowRegion,NowNum);
		musicControl[NowRegion][NowNum]=true;
	}
		

	//if(NowRegion!=region || NowNum!=num)
		//musicControl[NowRegion][NowNum]=true;
}

void midiTable(int num,int region)
{
	switch(region)
	{
		case 0:
			switch(num)
			{
				case 0:mideOutPut(2,1);break;
				case 1:mideOutPut(1,1);break;
				case 2:mideOutPut(7,0);break;
				case 3:mideOutPut(6,0);break;
			}
			break;
		case 1:
			switch(num)
			{
				case 0:mideOutPut(6,1);break;
				case 1:mideOutPut(5,1);break;
				case 2:mideOutPut(4,1);break;
				case 3:mideOutPut(3,1);break;
			}
			break;
		case 2:
			switch(num)
			{
				case 0:mideOutPut(3,2);break;
				case 1:mideOutPut(2,2);break;
				case 2:mideOutPut(1,2);break;
				case 3:mideOutPut(7,1);break;
			}
			break;
		case 3:
			switch(num)
			{
				case 0:mideOutPut(7,2);break;
				case 1:mideOutPut(6,2);break;
				case 2:mideOutPut(5,2);break;
				case 3:mideOutPut(4,2);break;
			}
			break;
		case 4:
			switch(num)
			{
				case 0:mideOutPut(4,1);break;
				case 1:mideOutPut(3,1);break;
				case 2:mideOutPut(2,1);break;
				case 3:mideOutPut(1,1);break;
			}
			break;
		case 5:
			switch(num)
			{
				case 0:mideOutPut(1,2);break;
				case 1:mideOutPut(7,1);break;
				case 2:mideOutPut(6,1);break;
				case 3:mideOutPut(5,1);break;
			}
			break;
		case 6:
			switch(num)
			{
				case 0:mideOutPut(5,2);break;
				case 1:mideOutPut(4,2);break;
				case 2:mideOutPut(3,2);break;
				case 3:mideOutPut(2,2);break;
			}
			break;
		case 7:
			switch(num)
			{
				case 0:mideOutPut(2,2);break;
				case 1:mideOutPut(1,2);break;
				case 2:mideOutPut(7,2);break;
				case 3:mideOutPut(6,2);break;
			}
			break;
	
	}

}
void OnDisplay()
{
	if(g_tracking.GetColorImageBuffer() == 0) return;
	// Render
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	// Draw color buffer from camera
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPixelZoom((float)g_winWidth/g_tracking.GetColorImageWidth(), -(float)g_winHeight/g_tracking.GetColorImageHeight()); glRasterPos2f(-1, 1);
	//glDrawPixels(g_tracking.GetColorImageWidth(), g_tracking.GetColorImageHeight(), GL_RGB, GL_UNSIGNED_BYTE, g_tracking.GetColorImageBuffer());
	if(g_layers[3])
	{
		auto dimx = g_tracking.GetDepthImageWidth(), dimy = g_tracking.GetDepthImageHeight();
		glMatrixMode(GL_PROJECTION); 
		glPushMatrix();
		//glOrtho(0, 800, 800, 0, -1, 1);
		//cout<<"dimx="<<dimx<<"dimy="<<dimy;
		glOrtho(dimx, 0, dimy, 0, -1, 1); 
		glBegin(GL_POINTS);
		for(int y=0; y<dimy; ++y) 
			for(int x=0; x<dimx; ++x)
			{
				int segment = g_tracking.GetSegmentationMask()[y*dimx+x];
				if(segment > 0) 
				{ 
					glColor3fv(segment == 1 ? hskl::float3(0,1,1) : hskl::float3(1,0,1)); 
					glVertex2i(x,y); 
				}
			}
		glEnd(); 
		glPopMatrix();
	}
	glPopAttrib();
	glClear(GL_DEPTH_BUFFER_BIT); // on Intel Graphics we noticed drawpixels seemed to affect the depth buffer

	// Set up OpenGL state
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_NORMALIZE);	glEnable(GL_COLOR_MATERIAL); glEnable(GL_CULL_FACE);

	// Use a left-handed perspective projection matrix whose horizontal and vertical field of view mimic the sensor device
	glMatrixMode(GL_PROJECTION); glPushMatrix(); glMultMatrixf(g_tracking.GetSensorPerspectiveMatrix(g_nearZ,g_farZ)); 
	float light_position[] = {0,0,0,1}; 
	glEnable(GL_LIGHT0); 
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glMatrixMode(GL_MODELVIEW); 
	glPushMatrix(); 
	gluLookAt(0,0,1, 0,0,0, 0,-1,0);
	
	// Draw meshes and basis vectors for each bone
	int tips[] = {0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1, 0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1};
	for(int i=0; i<g_tracking.GetBoneCount();  i++)
	{
		// Set up transformation for bone
		glPushMatrix(); glMultMatrixf(g_tracking.GetBoneModelMatrix(i));
		glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING); 
		if(g_layers[0]) // Draw bone geometry used inside tracking library (in a real application you would use a skinned mesh of some sort)
		{
			glBegin(GL_TRIANGLES);
			if(g_tracking.GetTrackingError(i) < 0.5f) glColor4f(0.3f,0.3f,1,0.75f);
			else glColor4f(0.7f,0.3f,1,0.75f);
			auto verts = g_tracking.GetVertices(i);
			auto tris = g_tracking.GetTriangles(i);
			for(int j=0; j<g_tracking.GetTriangleCount(i); ++j)
			{
				// Compute a normal from the vertices of this bone. This yields a "flat shaded" look on the mesh.
				auto v0 = verts[tris[j].x], v1 = verts[tris[j].y], v2 = verts[tris[j].z];
				glNormal3fv(cross(v1-v0,v2-v0)); glVertex3fv(v0); glVertex3fv(v1); glVertex3fv(v2);
			}
			glEnd();
		}

		glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING); 
		if(g_layers[2]) // Draw 1 cm long colored basis vectors for bone frame of reference
		{
			glBegin(GL_LINES);
			glColor3f(1,0,0); glVertex3f(0, 0, 0); glVertex3f(0.01f, 0, 0);
			glColor3f(0,1,0); glVertex3f(0, 0, 0); glVertex3f(0, 0.01f, 0);
			glColor3f(0,0.5f,1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 0.01f);
			glEnd();
		}
		glPopMatrix();
	}
	drawLine();
	if(g_layers[1]) // Draw "skeleton" from connections between bones and finger tip locations
	{
		// the position (local origin) of each bone is where it connects (joined) to its "parent" bone
		glBegin(GL_LINES); 
		glColor3f(1,1,1);
		for(int i=0; i<g_tracking.GetBoneCount();  i++)
		{
			int parent = g_tracking.GetParentBone(i); 
			if(parent == HSKL_BAD_INDEX) continue;
			glVertex3fv(g_tracking.GetPosition(parent)); 
			glVertex3fv(g_tracking.GetPosition(i)); 
		}
		glEnd();

		// there isn't a "joint" at the tips/ends of each of the last bone of each finger, so we 
		// use the GetBoneTip method to walk to the upper (local-z) extreme of the bone
		glBegin(GL_LINES); 		
		for(int finger=0; finger<5; ++finger)  // to fingertips
		{
			auto p = g_tracking.GetBoneTip(4+3*finger);
			glColor3f(1,1,1);
			glVertex3fv(g_tracking.GetPosition(4+3*finger)); 
			glVertex3fv(p);
			glColor3f(1,1,0);
			glVertex3f(p.x-0.002f,p.y,p.z); 
			glVertex3f(p.x+0.002f,p.y,p.z);
			glVertex3f(p.x,p.y-0.002f,p.z); 
			glVertex3f(p.x,p.y+0.002f,p.z);
		}
		glEnd();
	}
	
	glPopMatrix(); 
	glMatrixMode(GL_PROJECTION); glPopMatrix(); glPopAttrib();
	
	// Draw overlay text
	glPushMatrix(); 
	glOrtho(0,g_winWidth,g_winHeight,0,-1,1);
	DrawString(10,24, "Tracking %s at %d FPS with error of %0.4f (Press r/l/t to change) [%d bit]", g_profileString, g_fps, g_tracking.GetTrackingError(0), sizeof(void *)*8);
	DrawString(10,40, "Assuming hand width of %0.2f cm and hand length of %0.2f cm (Press w/a/s/d to adjust)", g_handWidth*100, g_handLength*100);
	DrawString(10,56, "Displayed layers: 1-mesh %s, 2-skeleton %s, 3-bases %s, 4-segmentation %s (Press keys 1/2/3/4 to toggle)", g_layers[0]?"on":"off", g_layers[1]?"on":"off", g_layers[2]?"on":"off", g_layers[3]?"on":"off");
	glPopMatrix();
	
	//drawLine();
	glutSwapBuffers();
}

int main(int argc, char * argv[])
{	

    int i, keyPress;
    int nPorts;
    char input;

	// Check available ports.
    nPorts = midiout->getPortCount();
    if ( nPorts == 0 ) {
        printf( "No ports available!\n" );
    }
        // List Available Ports
	printf( "\nPort Count = %d", nPorts );
	printf( "Available Output Ports\n-----------------------------\n" );
    for( i=0; i<nPorts; i++ )
    {
        try {
			    printf("Output Port Number  %d \n", i);
        }
        catch(RtError &error) {
            error.printMessage();
        }
    }
     midiout->openPort(0);
	for(i=0;i<3;i++)
	{
		for(int j=0;j<7;j++)
			musicControl[i][j]=true;
	}
	if(!g_tracking.Init()) { fprintf(stderr, "Unable to initialize tracking\n"); return -1; }

	// Initialise GLUT and start running
	g_winWidth = g_tracking.GetColorImageWidth(); g_winHeight = g_tracking.GetColorImageHeight();
	if(g_winWidth < 500) { g_winWidth *= 2; g_winHeight *= 2; }
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - g_winWidth)/2, (glutGet(GLUT_SCREEN_HEIGHT) - g_winHeight)/2);
	glutInitWindowSize(g_winWidth, g_winHeight);
	glutCreateWindow("Hand track viewer demo");
	glutKeyboardFunc(OnKeyboard);
	glutIdleFunc(UpdateTracking);
	glutDisplayFunc(OnDisplay);
	glutMainLoop();
	return 0;
}