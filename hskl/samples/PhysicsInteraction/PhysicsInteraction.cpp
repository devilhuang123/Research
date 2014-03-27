//
// 3D Articulated Skeletal Hand Tracking
// Interaction Sample Program
//
// This sample tracks the user's right hand and then feeds
// the tracked pose into the Bullet physics engine
// using it to drive a virtual hand model so that it can interact
// with other virtual objects in the physics scene.
// The hand model is driven dynamically with finite forces to 
// make the interaction more physical in nature.
// This is commonly known as the powered-rag-doll technique.
//
//

#include "hsklu.h"			// For skeletal tracking
#include <gl/freeglut.h>			// For rendering output (OpenGL/FreeGLUT)
#include <btBulletDynamicsCommon.h>	// For physical simulation (Bullet)
#include <vector>
using namespace hskl;

enum { LAYER_WORLD=1, LAYER_OBJECT=2, LAYER_HAND=4 };
struct HandJoint   // used to drive relative orientations between connected bones
{
	int							parent, child;
	btGeneric6DofConstraint *	constraint;
	HandJoint(std::vector<btRigidBody*> bodies, int parent, int child);
	void Update();
};
struct HandRoot  // used to drive positions of the palm bone(s)
{
	int							bone;
	btRigidBody *				root;
	HandRoot(int bone, btRigidBody * body);
	void Update();
};
struct BoneObject { int bone; btDefaultMotionState * state; };
struct BoxObject { float3 size; float3 color; btDefaultMotionState * state; };

hskl::Tracker							g_tracking;

// Physics state
btDefaultCollisionConfiguration			g_config;
btCollisionDispatcher					g_dispatcher(&g_config);
btAxisSweep3							g_broadphase(btVector3(-64,-64,-64),btVector3(64,64,64));
btSequentialImpulseConstraintSolver		g_solver;
btDiscreteDynamicsWorld					g_world(&g_dispatcher,&g_broadphase,&g_solver,&g_config);
std::vector<HandJoint>					g_joints; 
std::vector<HandRoot>					g_roots;  
std::vector<BoneObject>					g_bones;
std::vector<BoxObject>					g_boxes;

inline btTransform ToBullet(float3 pos, float4 ori) { return btTransform(btQuaternion(ori.x,ori.y,ori.z,ori.w), btVector3(pos.x,pos.y,pos.z)); }

btRigidBody * Box(float3 size, float3 color, float3 pos, float mass)
{
	auto shape = new btBoxShape(btVector3(size.x/2,size.y/2,size.z/2));
	shape->setMargin(0.0005f);
	auto state = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(pos.x,pos.y,pos.z)));
	btVector3 inertia; shape->calculateLocalInertia(mass, inertia);
	auto body = new btRigidBody(mass, state, shape, inertia);
	body->setDamping(0.1f, 0.3f);
	if(mass == 0)
	{
		body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		g_world.addRigidBody(body,LAYER_WORLD, LAYER_OBJECT);
	}
	else g_world.addRigidBody(body,LAYER_OBJECT, LAYER_WORLD|LAYER_OBJECT|LAYER_HAND);
	BoxObject obj = {size,color,state}; g_boxes.push_back(obj);
	return body;
}
void WBox(float3 color, float3 b0, float3 b1) { Box(b1-b0, color, (b0+b1)*0.5f, 0); }

HandJoint::HandJoint(std::vector<btRigidBody*> bodies, int parent, int child) : parent(parent), child(child) 
{
	auto anchorA = g_tracking.GetAnchorPoint(child)-g_tracking.GetCenterOfMass(parent), anchorB = -g_tracking.GetCenterOfMass(child);
	constraint = new btGeneric6DofConstraint(*bodies[parent], *bodies[child],
		btTransform(btQuaternion(0,0,0,1), btVector3(anchorA.x,anchorA.y,anchorA.z)),
		btTransform(btQuaternion(0,0,0,1), btVector3(anchorB.x,anchorB.y,anchorB.z)), false);
	constraint->setAngularLowerLimit(btVector3(-1e-5f,-1e-5f,-1e-5f));
	constraint->setAngularUpperLimit(btVector3( 1e-5f, 1e-5f, 1e-5f));
	g_world.addConstraint(constraint,true);
}

void HandJoint::Update()
{
	auto a = g_tracking.GetOrientation(parent), b = g_tracking.GetOrientation(child);
	constraint->getFrameOffsetA().setRotation(btQuaternion(-a.x,-a.y,-a.z,a.w));
	constraint->getFrameOffsetB().setRotation(btQuaternion(-b.x,-b.y,-b.z,b.w));
}

HandRoot::HandRoot(int bone, btRigidBody * body) : bone(bone)
{
	auto pos=g_tracking.GetPosition(bone), com=g_tracking.GetCenterOfMass(bone); 
	auto ori=g_tracking.GetOrientation(bone); pos=pos+qtransform(ori,com);
	auto state = new btDefaultMotionState(ToBullet(pos,ori),ToBullet(-com,float4(0,0,0,1)));
	root = new btRigidBody(0, state, 0);
	root->setCollisionFlags(root->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	g_world.addRigidBody(root, 0, 0);
	auto constraint = new btGeneric6DofConstraint(*root, *body, btTransform(btMatrix3x3(1,0,0,0,1,0,0,0,1)), btTransform(btMatrix3x3(1,0,0,0,1,0,0,0,1)), false);
	constraint->setAngularLowerLimit(btVector3(-1e-5f,-1e-5f,-1e-5f));
	constraint->setAngularUpperLimit(btVector3( 1e-5f, 1e-5f, 1e-5f));
	g_world.addConstraint(constraint,true);
}

void HandRoot::Update()
{
	auto pos=g_tracking.GetPosition(bone), com=g_tracking.GetCenterOfMass(bone); 
	auto ori=g_tracking.GetOrientation(bone); pos=(pos-float3(0,0,0.7f))*4+qtransform(ori,com);
	root->setWorldTransform(ToBullet(pos,ori));
}

int g_winWidth = 1280, g_winHeight = 720;
void OnReshape(int width, int height) { g_winWidth = width; g_winHeight = height; }

void DrawScene(bool shadow)
{
	for(size_t i=0; i<g_bones.size(); ++i)
	{
		float transform[16]; g_bones[i].state->m_graphicsWorldTrans.getOpenGLMatrix(transform);
		glPushMatrix(); glMultMatrixf(transform); glBegin(GL_TRIANGLES);
		if(!shadow) { if(g_tracking.GetTrackingError((int)i) <= 1.0f) glColor3f(0.3f,0.3f,1); else glColor3f(0.7f,0.3f,1); }
		auto verts = g_tracking.GetVertices(g_bones[i].bone);
		auto tris = g_tracking.GetTriangles(g_bones[i].bone);
		for(int j=0; j<g_tracking.GetTriangleCount(g_bones[i].bone); ++j)
		{
			auto v0 = verts[tris[j].x], v1 = verts[tris[j].y], v2 = verts[tris[j].z];
			glNormal3fv(cross(v1-v0,v2-v0)); glVertex3fv(v0); glVertex3fv(v1); glVertex3fv(v2);
		}
		glEnd(); glPopMatrix();
	}
	for(size_t i=shadow?1:0; i<g_boxes.size(); ++i)
	{
		float transform[16]; g_boxes[i].state->m_graphicsWorldTrans.getOpenGLMatrix(transform); const auto s = g_boxes[i].size*0.5f; 
		glPushMatrix(); glMultMatrixf(transform); glBegin(GL_QUADS); if(!shadow) glColor3fv(&g_boxes[i].color.x);
		glNormal3f(+1,0,0); glVertex3f(+s.x,-s.y,-s.z);	glVertex3f(+s.x,+s.y,-s.z);	glVertex3f(+s.x,+s.y,+s.z);	glVertex3f(+s.x,-s.y,+s.z);
		glNormal3f(-1,0,0); glVertex3f(-s.x,+s.y,-s.z);	glVertex3f(-s.x,-s.y,-s.z);	glVertex3f(-s.x,-s.y,+s.z);	glVertex3f(-s.x,+s.y,+s.z);
		glNormal3f(0,+1,0);	glVertex3f(-s.x,+s.y,-s.z);	glVertex3f(-s.x,+s.y,+s.z);	glVertex3f(+s.x,+s.y,+s.z);	glVertex3f(+s.x,+s.y,-s.z);
		glNormal3f(0,-1,0);	glVertex3f(-s.x,-s.y,+s.z);	glVertex3f(-s.x,-s.y,-s.z);	glVertex3f(+s.x,-s.y,-s.z);	glVertex3f(+s.x,-s.y,+s.z);
		glNormal3f(0,0,+1);	glVertex3f(-s.x,-s.y,+s.z);	glVertex3f(+s.x,-s.y,+s.z);	glVertex3f(+s.x,+s.y,+s.z);	glVertex3f(-s.x,+s.y,+s.z);
		glNormal3f(0,0,-1);	glVertex3f(+s.x,-s.y,-s.z);	glVertex3f(-s.x,-s.y,-s.z);	glVertex3f(-s.x,+s.y,-s.z);	glVertex3f(+s.x,+s.y,-s.z);
		glEnd(); glPopMatrix();
	}
}

const float ground = 0.35f, objDepth = -1.3f, linkLength = 0.05f;
void OnIdle()
{ 
	// Simulate
	g_tracking.Update();  // have the tracking system process the next frame from the input depth camera
	for(size_t i=0; i<g_joints.size(); ++i) g_joints[i].Update();
	for(size_t i=0; i<g_roots.size(); ++i) g_roots[i].Update();
	g_world.stepSimulation(1.0f/60);  // bullet physics engine simulation step

	// Render
	glPushAttrib(GL_ALL_ATTRIB_BITS); glEnable(GL_NORMALIZE); glEnable(GL_COLOR_MATERIAL); glEnable(GL_CULL_FACE);
	glViewport(0,0,g_winWidth,g_winHeight); glMatrixMode(GL_PROJECTION); glPushMatrix(); gluPerspective(60,(double)g_winWidth/g_winHeight,1.0f/16,64);
	glMatrixMode(GL_MODELVIEW); glPushMatrix(); gluLookAt(0,ground-0.35f,-0.1f, 0,ground-0.35f,-1, 0,-1,0);
	glClearColor(0.2f,0.5f,1,1); glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST); float3 lightDir(0.8f,1,-0.5f); float lightPos[]={-lightDir.x,-lightDir.y,-lightDir.z,0};
	glEnable(GL_LIGHT0); glLightfv(GL_LIGHT0, GL_POSITION, lightPos); glEnable(GL_LIGHTING); DrawScene(false);
	auto n = float3(0,-1,0); float d = ground-0.0001f, vn = dot(lightDir,n);
	auto pv = lightDir*(-1/vn), pw = lightDir*(-d/vn);
	const float shadow[16] = {
		pv.x*n.x+1, pv.y*n.x, pv.z*n.x, 0,
		pv.x*n.y, pv.y*n.y+1, pv.z*n.y, 0,
		pv.x*n.z, pv.y*n.z, pv.z*n.z+1, 0,
		pw.x, pw.y, pw.z, 1
	};
	glPushMatrix(); glMultMatrixf(shadow); glDisable(GL_LIGHTING); glColor3f(0,0,0); DrawScene(true);
	glPopMatrix(); glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glPopAttrib();
	glutSwapBuffers(); 
}

int main(int argc, char * argv[])
{
	g_tracking.Init();
	g_world.setGravity(btVector3(0,1,0));
	Box(float3(3,2,3), float3(0.9f,0.9f,0.6f), float3(0,ground+1,-1), 0);

	// Build "pyramid"
	for(int i=0; i<5; ++i)
	{
		for(int j=0; j<=i; ++j)
		{
			Box(float3(0.1f,0.1f,0.1f), float3((float)rand()/RAND_MAX,(float)rand()/RAND_MAX,(float)rand()/RAND_MAX), float3(-0.3f+j*0.11f-i*0.055f, ground-0.44f+i*0.11f,objDepth), 1);
		}
	}

	// Build "tetherball"
	auto last = Box(float3(linkLength,linkLength,linkLength), float3(0,0.8f,0.4f), float3(0.3f,ground-0.375f+0.1f,objDepth), 1);
	for(int i=0; i<6; ++i)
	{
		auto body = Box(float3(0.01f,linkLength,0.01f), float3(0.7f,0.7f,0.7f), float3(0.3f,ground-0.375f+i*linkLength*-0.9f,objDepth), 1);
		if(last)
		{
			auto constraint = new btPoint2PointConstraint(*body, *last, btVector3(0,0.0225f,0), btVector3(0,-0.0225f,0));
			constraint->setOverrideNumSolverIterations(20);
			g_world.addConstraint(constraint, true);
		}
		last = body;
	}
	auto anchor = Box(float3(linkLength,linkLength,linkLength), float3(0,0.8f,0.4f), float3(0.3f,ground-0.375f+6*linkLength*-0.9f,objDepth), 0);
	auto constraint = new btPoint2PointConstraint(*anchor, *last, btVector3(0,0.0225f,0), btVector3(0,-0.0225f,0));
	g_world.addConstraint(constraint, true);
	WBox(float3(0,0.6f,0.3f), float3(0.49f,ground-0.685f,objDepth-0.01f), float3(0.51f,ground,objDepth+0.01f));
	WBox(float3(0,0.6f,0.3f), float3(0.3f,ground-0.685f,objDepth-0.01f), float3(0.49f,ground-0.665f,objDepth+0.01f));

	// Generate bodies and constraints for hand(s)
	std::vector<btRigidBody*> bodies; 
	for(int i=0; i<g_tracking.GetBoneCount(); ++i)
	{
		// Skip forearm bones on both hands
		int parentBone = g_tracking.GetParentBone(i);
		if(parentBone == HSKL_BAD_INDEX) { bodies.push_back(0); continue; } 

		// Generate a shape centered on the center of mass
		auto pos=g_tracking.GetPosition(i), com=g_tracking.GetCenterOfMass(i); 
		auto ori=g_tracking.GetOrientation(i); pos=pos+qtransform(ori,com);
		auto shape = new btConvexHullShape();
		for(auto it=g_tracking.GetVertices(i), end=it+g_tracking.GetVertexCount(i); it!=end; ++it)
		{
			auto p = *it - com;	shape->addPoint(btVector3(p.x,p.y,p.z));
		}
		shape->setMargin(0.0005f);

		// Generate a rigid body and position it at the current pose location
		const float mass = 1; btVector3 inertia;
		auto state = new btDefaultMotionState(ToBullet(pos,ori), btTransform(btQuaternion(0,0,0,1), btVector3(-com.x,-com.y,-com.z)));
		shape->calculateLocalInertia(mass, inertia);
		auto body = new btRigidBody(mass, state, shape, inertia);
		body->setDamping(0.85f, 0.85f);
		body->setActivationState(DISABLE_DEACTIVATION);
		g_world.addRigidBody(body,LAYER_HAND,LAYER_OBJECT);
		BoneObject obj = {i,state}; g_bones.push_back(obj);
		bodies.push_back(body);

		// Create a joint if this bone has a parent, otherwise create a root constraint
		if(bodies[parentBone]) g_joints.push_back(HandJoint(bodies,parentBone,i));
		else g_roots.push_back(HandRoot(i,body));
	}

	// Initialise GLUT and start running
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - g_winWidth)/2, (glutGet(GLUT_SCREEN_HEIGHT) - g_winHeight)/2);
	glutInitWindowSize(g_winWidth, g_winHeight);
	glutCreateWindow("Physical interaction demo");
	glutReshapeFunc(OnReshape);
	glutIdleFunc(OnIdle);
	glutMainLoop();
	return 0;
}
