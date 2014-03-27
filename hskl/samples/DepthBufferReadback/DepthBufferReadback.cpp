// This sample feeds depth data from a generic 3D camera to the Hand Tracking library and evaluates the
// tracked pose by rendering it to a depth buffer and comparing that to the camera's own depth buffer.
#include "hsklu.h"
#include <gl/freeglut.h>
#include <vector>

hskl::Tracker tracker;
float handThickness=0.08f,handLength=0.19f;
int CAM_WIDTH,CAM_HEIGHT;
int t0,t1,frames,fps;
std::vector<float> vbufferA, vbufferB;
float * bufferA, * bufferB;

void OnKeyboard(unsigned char ch, int x, int y)
{
	if(ch == 27) exit(0);
	if(ch == 'w') handLength += 0.19f/32;
	if(ch == 'a') handThickness -= 0.08f/32;
	if(ch == 's') handLength -= 0.19f/32;
	if(ch == 'd') handThickness += 0.08f/32;
	tracker.SetHandMeasurements(handThickness, handLength);
}

void DrawString(int x, int y, const char * format, ...)
{
	glRasterPos2i(x,y);
	va_list args; char buffer[1024];
	va_start(args, format);
	int n = vsnprintf(buffer, sizeof(buffer), format, args);
	for(int i=0; i<n; ++i) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, buffer[i]);
	va_end(args);
}

const float NEAR_CLIP = 0.1f, FAR_CLIP = 0.6f; // Expressed in meters
void OnIdle()
{
	tracker.Update();

	// Render 
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	float light_position[] = {3,5,8,1};
	glEnable(GL_LIGHT0); glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_NORMALIZE);	glEnable(GL_COLOR_MATERIAL); glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION); glPushMatrix(); glMultMatrixf(tracker.GetSensorPerspectiveMatrix(NEAR_CLIP, FAR_CLIP));
	glMatrixMode(GL_MODELVIEW);	glPushMatrix(); gluLookAt(0,0,0, 0,0,1, 0,-1,0);
	glViewport(0,0,CAM_WIDTH,CAM_HEIGHT);

	for(int i=0, n=tracker.GetBoneCount(); i<n; i++)
	{
		// Draw bone geometry 
		glPushMatrix(); glMultMatrixf(tracker.GetBoneModelMatrix(i));
		glEnable(GL_LIGHTING);
		glBegin(GL_TRIANGLES); glColor4f(1,1,1,0.5f);
		auto tris = tracker.GetTriangles(i);
		auto verts = tracker.GetVertices(i);
		for(int j=0, m=tracker.GetTriangleCount(i); j<m; j++)
		{
			// Compute a normal from the vertices of this bone. This yields a "flat shaded" look on the mesh.
			auto v0 = verts[tris[j].x], v1 = verts[tris[j].y], v2 = verts[tris[j].z];
			glNormal3fv(cross(v1-v0,v2-v0)); glVertex3fv(v0); glVertex3fv(v1); glVertex3fv(v2);
		}
		glEnd(); glPopMatrix();
	}
	glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
	glPopAttrib();

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// First, read non-linear depth map values (bottom-up order) into buffer
	glReadPixels(0,0,CAM_WIDTH,CAM_HEIGHT,GL_DEPTH_COMPONENT,GL_FLOAT,bufferA);
		
	// Now, convert these values linearly into the range of 0 to 1
	for(float * it = bufferA, * end = bufferA + CAM_WIDTH*CAM_HEIGHT; it!=end; ++it)
	{
		*it = (NEAR_CLIP * FAR_CLIP) / (FAR_CLIP - *it * (FAR_CLIP-NEAR_CLIP)); // Converts to true depth
		*it = (*it - NEAR_CLIP) / (FAR_CLIP-NEAR_CLIP); // Convert to [0,1]
	}
	glRasterPos2f(-1,0);
	glDrawPixels(CAM_WIDTH,CAM_HEIGHT,GL_LUMINANCE,GL_FLOAT,bufferA);

	// Fill the second buffer with depth values from the kinect, converted linearly into the range of 0 to 1
	float * out = bufferB;
	for(int y=0; y<CAM_HEIGHT; ++y)
	{
		const unsigned short * in = tracker.GetDepthImageBuffer() + (CAM_HEIGHT-1-y)*CAM_WIDTH;
		for(int x=0; x<CAM_WIDTH; ++x)
		{
			unsigned short depth = *in++;
			float depthValue = depth == 0 ? 1.0f : ((depth * 0.001f) - NEAR_CLIP) / (FAR_CLIP-NEAR_CLIP);
			if(depthValue > 1) depthValue = 1;
			if(depthValue < 0) depthValue = 0;
			*out++ = depthValue;
		}
	}
	glRasterPos2f(0,0);
	glDrawPixels(CAM_WIDTH,CAM_HEIGHT,GL_LUMINANCE,GL_FLOAT,bufferB);

	// Finally draw the absolute difference between the two
	for(int i=0; i<CAM_WIDTH*CAM_HEIGHT; ++i)
	{
		float error = fabsf(bufferA[i] - bufferB[i]); 
		bufferA[i] = error * 3;
	}
	glRasterPos2f(0,-1);
	glDrawPixels(CAM_WIDTH,CAM_HEIGHT,GL_LUMINANCE,GL_FLOAT,bufferA);

	glPopAttrib();

	glPushMatrix(); glOrtho(0,CAM_WIDTH*2,CAM_HEIGHT*2,0,-1,1);
	if(++frames == 4) { t1 = glutGet(GLUT_ELAPSED_TIME); fps = 1000 * frames / (t1-t0); t0 = t1; frames = 0; }
	DrawString(10,CAM_HEIGHT+24, "Tracking right hand at %d FPS with error term %f", fps, tracker.GetTrackingError(0));
	DrawString(10,CAM_HEIGHT+36, "Scaling hand width by %0.2f and hand length by %0.2f (Press w/a/s/d to adjust)", handThickness, handLength);
	glPopMatrix(); glutSwapBuffers();
}

int main(int argc, char * argv[])
{
	// Start tracking and initialize memory for buffers
	tracker.Init();
	CAM_WIDTH = tracker.GetDepthImageWidth(); CAM_HEIGHT = tracker.GetDepthImageHeight();
	vbufferA.resize(CAM_WIDTH*CAM_HEIGHT); vbufferB.resize(CAM_WIDTH*CAM_HEIGHT);
	bufferA = &vbufferA[0]; bufferB = &vbufferB[0];

	// Init GLUT window
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - CAM_WIDTH*2)/2, (glutGet(GLUT_SCREEN_HEIGHT) - CAM_HEIGHT*2)/2);
	glutInitWindowSize(CAM_WIDTH*2,CAM_HEIGHT*2);
	glutCreateWindow("Depth buffer readback demo");
	glutKeyboardFunc(OnKeyboard);
	glutIdleFunc(OnIdle);
	glutMainLoop();
	return 0;
}