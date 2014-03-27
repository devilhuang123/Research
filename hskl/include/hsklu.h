/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2013 Intel Corporation. All Rights Reserved.

*******************************************************************************/

#ifndef HAND_SKELETON_UTILITIES_INCLUDE_GUARD
#define HAND_SKELETON_UTILITIES_INCLUDE_GUARD

// This header provides an easier to use C++ interface that 
// simplifies the task of connecting a sensor to and accessing the skeletal hand tracking library.
// In other words, the initialization of the PerC SDK and depth camera and passing that information
// to the lower level C tracking library are taken care of for you.
//
// To use the hand tracking system, create and Init() a hskl::Tracker object
// and make repeated calls to Tracker::Update() followed by accessor methods
// such as Tracker::GetBonePosition() to get the current pose of the user's hand.

#include "hskl.h"
#include <pxcsmartptr.h>
#include <cmath>

#ifdef NDEBUG
#pragma comment(lib, "libpxc.lib")
#else
#pragma comment(lib, "libpxc_d.lib")
#endif

namespace hskl
{
	// Basic linear algebra types and operations
	struct float3 { float x,y,z;   float3() : x(),y(),z()     {} float3(float x, float y, float z         ) : x(x),y(y),z(z)      {} operator const float * () const { return &x; } };
	struct float4 { float x,y,z,w; float4() : x(),y(),z(),w() {} float4(float x, float y, float z, float w) : x(x),y(y),z(z),w(w) {} operator const float * () const { return &x; } };
	struct float4x4 { float4 x,y,z,w; operator const float * () const { return x; } };
	inline float3 operator - (const float3 & v)                   { return float3(-v.x,-v.y,-v.z); }
	inline float3 operator + (const float3 & a, const float3 & b) { return float3(a.x+b.x,a.y+b.y,a.z+b.z); }
	inline float3 operator - (const float3 & a, const float3 & b) { return float3(a.x-b.x,a.y-b.y,a.z-b.z); }
	inline float3 operator * (const float3 & a, float b)          { return float3(a.x*b  ,a.y*b  ,a.z*b  ); }
	inline float3 cross      (const float3 & a, const float3 & b) { return float3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x); }
	inline float  dot        (const float3 & a, const float3 & b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
	inline float4 qinv       (const float4 & q)                   { return float4(-q.x,-q.y,-q.z,q.w); }
	inline float4 qmul       (const float4 & a, const float4 & b) { return float4(a.w*b.x+a.x*b.w+a.y*b.z-a.z*b.y, 
																				  a.w*b.y-a.x*b.z+a.y*b.w+a.z*b.x,
																				  a.w*b.z+a.x*b.y-a.y*b.x+a.z*b.w,
																				  a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z); }
	inline float3 qxdir      (const float4 & q)                   { return float3(1-2*(q.y*q.y+q.z*q.z), 2*(q.x*q.y+q.w*q.z), 2*(q.x*q.z-q.w*q.y)); }
	inline float3 qydir      (const float4 & q)                   { return float3(2*(q.x*q.y-q.w*q.z), 1-2*(q.x*q.x+q.z*q.z), 2*(q.y*q.z+q.w*q.x)); }
	inline float3 qzdir      (const float4 & q)                   { return float3(2*(q.x*q.z+q.w*q.y), 2*(q.y*q.z-q.w*q.x), 1-2*(q.x*q.x+q.y*q.y)); }
	inline float3 qtransform (const float4 & q, const float3 & p) { return qxdir(q)*p.x + qydir(q)*p.y + qzdir(q)*p.z; }

	// Tracking system that encapsulates a handle to tracking engine and a depth sensor, and wires them together
	class Tracker
	{
		hskl_tracker							tracker;	// Tracking library
		PXCSmartPtr<PXCSession>					session;	// PerC SDK session
		PXCSmartPtr<PXCCapture>					capture;	// ???
		PXCSmartPtr<PXCCapture::Device>			device;		// Hardware device capable of generating depth
		PXCSmartPtr<PXCCapture::VideoStream>	stream;		// Video stream containing depth images
		PXCSmartPtr<PXCImage>					image;		// An individual depth image
		PXCSmartPtr<PXCScheduler::SyncPoint>	sp;			// Used to asynchronously request the next frame
		unsigned char *							color;
		unsigned short *						depth;
		int										dimx,dimy;
		float									fovx,fovy;

												Tracker(const Tracker &);							// Disallow copy-construction
		Tracker &								operator = (const Tracker &);						// Disallow assignment
	public:
												Tracker()											: tracker(), dimx(), dimy(), fovx(), fovy(), color(), depth() {}
												~Tracker()											{ if(tracker) hsklDestroyTracker(tracker); if(sp) sp->Synchronize(); delete[] color; delete[] depth; }

		bool									Init();												// Initialize tracking, and return true if successful, false if an error occurred
		void									Update();											// Retrieve one frame from the sensor, and run tracking algorithms on it

		void									SetModelType(hskl_model model)						{ hsklSetModelType(tracker,model); }
		void									SetHandMeasurements(float width, float height)		{ hsklSetHandMeasurements(tracker,width,height); }
			
		float3									GetPosition(int bone) const							{ return (const float3 &)hsklGetPosition(tracker,bone); }
		float4									GetOrientation(int bone) const						{ return (const float4 &)hsklGetOrientation(tracker,bone); }
		float									GetTrackingError(int bone) const					{ return hsklGetTrackingError(tracker,bone); }
		float4x4								GetBoneModelMatrix(int bone) const;					// Obtain a 4x4 transformation matrix representing the pose of the bone
		float3									GetBoneTip(int bone) const;							// Obtain the position of the tip of a bone, such as a fingertip

		int										GetBoneCount() const								{ return hsklGetBoneCount(tracker); }     
		int										GetVertexCount(int bone) const						{ return hsklGetVertexCount(tracker,bone); }
		const float3 *							GetVertices(int bone) const							{ return (const float3 *)hsklGetVertices(tracker,bone); }
		int										GetTriangleCount(int bone) const					{ return hsklGetTriangleCount(tracker,bone); }
		const hskl_int3 *						GetTriangles(int bone) const						{ return hsklGetTriangles(tracker,bone); }
		float3									GetCenterOfMass(int bone) const						{ return (const float3 &)hsklGetCenterOfMass(tracker,bone); }
		int										GetParentBone(int bone) const						{ return hsklGetParentBone(tracker,bone); }
		float3									GetAnchorPoint(int bone) const						{ return (const float3 &)hsklGetAnchorPoint(tracker,bone); }

		int										GetColorImageWidth() const							{ return dimx;  }
		int										GetColorImageHeight() const							{ return dimy; }
		const unsigned char *					GetColorImageBuffer() const							{ return color; }
		int										GetDepthImageWidth() const							{ return dimx; }
		int										GetDepthImageHeight() const							{ return dimy; }
		const unsigned short *					GetDepthImageBuffer() const							{ return depth; }
		const unsigned char *					GetSegmentationMask() const							{ return hsklGetSegmentationMask(tracker); }
		float4x4								GetSensorPerspectiveMatrix(float nearZ, float farZ) const; // Obtain a view matrix from the perspective of the sensor
	};

	///////////////////////////
	// Math/geometry helpers //
	///////////////////////////

	inline float4x4 Tracker::GetBoneModelMatrix(int bone) const
	{
		const auto q = GetOrientation(bone);
		float4x4 m;
		(float3&)m.x = qxdir(q);
		(float3&)m.y = qydir(q);
		(float3&)m.z = qzdir(q);
		(float3&)m.w = GetPosition(bone); 
		m.w.w = 1;
		return m;
	}

	inline float3 Tracker::GetBoneTip(int bone) const 
	{
		float maxZ = 0; auto vertices = GetVertices(bone);
		for(int i=0; i<GetVertexCount(bone); ++i) if(vertices[i].z > maxZ) maxZ = vertices[i].z;
		return GetPosition(bone) + qzdir(GetOrientation(bone)) * maxZ;
	}

	inline float4x4	Tracker::GetSensorPerspectiveMatrix(float nearZ, float farZ) const
	{
		float4x4 p;
		p.x.x = 1/tan(fovx/2);
		p.y.y = 1/tan(fovy/2);
		p.z.z = (farZ+nearZ)/(nearZ-farZ);
		p.z.w = -1;
		p.w.z = (2*farZ*nearZ)/(nearZ-farZ);
		return p;
	}

	//////////////////////
	// PerC SDK support //
	//////////////////////

	inline void Tracker::Update() 
	{ 
		if(sp && sp->Synchronize() >= PXC_STATUS_NO_ERROR)
		{
			PXCImage::ImageData depthData; image->AcquireAccess(PXCImage::ACCESS_READ, &depthData);
			memcpy_s(depth, sizeof(unsigned short)*dimx*dimy, depthData.planes[0], sizeof(unsigned short)*dimx*dimy);
			const unsigned short * conf = reinterpret_cast<const unsigned short *>(depthData.planes[1]);
			for(int i=0; i<dimx*dimy; ++i) color[3*i+2] = color[3*i+1] = color[3*i] = conf[i]>>2; // Can we just use IR here?
			hsklTrackOneFrame(tracker, depth, conf); // Pass data to tracking library
			image->ReleaseAccess(&depthData);	
		}

		stream->ReadStreamAsync(image.ReleaseRef(), sp.ReleaseRef());
	}

	inline bool Tracker::Init()
	{
		if(PXCSession_Create(session.ReleaseRef()) < PXC_STATUS_NO_ERROR || !session.IsValid()) return false;

		for(int i=0; ; ++i) // For valid capture contexts
		{ 
			PXCSession::ImplDesc desc, filter = { PXCSession::IMPL_GROUP_SENSOR, PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE };
			if(session->QueryImpl(&filter, i, &desc) < PXC_STATUS_NO_ERROR) break;
			if(session->CreateImpl(&desc, PXCCapture::CUID, (void**)capture.ReleaseRef()) < PXC_STATUS_NO_ERROR || !capture.IsValid()) continue;

			for(int j=0; ; ++j) // For valid devices
			{ 
				PXCCapture::DeviceInfo dinfo;
				if(capture->QueryDevice(j, &dinfo) < PXC_STATUS_NO_ERROR) break;
				if(capture->CreateDevice(j, device.ReleaseRef()) < PXC_STATUS_NO_ERROR || !device.IsValid()) continue;

				for(int k=0; ; ++k) // For valid video streams
				{ 
					PXCCapture::Device::StreamInfo sinfo; 
					if(device->QueryStream(k, &sinfo) < PXC_STATUS_NO_ERROR) break;
					if(sinfo.cuid != PXCCapture::VideoStream::CUID || device->CreateStream(k, PXCCapture::VideoStream::CUID, (void**)stream.ReleaseRef()) < PXC_STATUS_NO_ERROR || !device.IsValid()) continue;

					for (int m=0; ; ++m) // For depth buffer profiles of at least 60 FPS
					{ 
						PXCCapture::VideoStream::ProfileInfo pinfo;
						if(stream->QueryProfile(m, &pinfo) < PXC_STATUS_NO_ERROR) break;
						if(pinfo.imageInfo.format != PXCImage::IMAGE_TYPE_DEPTH || pinfo.frameRateMin.numerator / pinfo.frameRateMin.denominator < 60 || stream->SetProfile(&pinfo) < PXC_STATUS_NO_ERROR) continue;
						
						// If we can read at least one frame
						stream->ReadStreamAsync(image.ReleaseRef(), sp.ReleaseRef());
						if(sp && sp->Synchronize() >= PXC_STATUS_NO_ERROR)
						{
							// Obtain useful properties and reserve room for local copies of depth and color images
							dimx = pinfo.imageInfo.width; dimy = pinfo.imageInfo.height;
							PXCPointF32 flen; device->QueryPropertyAsPoint(PXCCapture::Device::PROPERTY_DEPTH_FOCAL_LENGTH, &flen);
							fovx = atan(dimx / (flen.x*2))*2; fovy = atan(dimy / (flen.y*2))*2;
							color = new unsigned char[dimx*dimy*3];
							depth = new unsigned short[dimx*dimy];

							// Initialize tracking library
							tracker = hsklCreateTracker(HSKL_COORDS_X_RIGHT_Y_DOWN_Z_FWD, HSKL_API_VERSION);
							hsklSetSensorProperties(tracker, HSKL_SENSOR_CREATIVE, dimx, dimy, fovx, fovy);
							return true;
						}
					}
					stream.ReleaseRef();
				}
				device.ReleaseRef();
			}
			capture.ReleaseRef();
		}
		return false;
	}
}

#endif