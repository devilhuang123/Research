/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2013 Intel Corporation. All Rights Reserved.

*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAND_SKELETON_LIBRARY_INCLUDE_GUARD
#define HAND_SKELETON_LIBRARY_INCLUDE_GUARD

typedef struct hskl_tracker_ *  hskl_tracker;     /* The overall tracking engine */
typedef struct {int x,y,z;}     hskl_int3;        /* 3-dimensional vector (e.g. triangle indices) */
typedef struct {float x,y,z;}   hskl_float3;      /* 3-dimensional vector (e.g. position, offset) */
typedef struct {float x,y,z,w;} hskl_float4;      /* 4-dimensional vector (e.g. orientation quaternion) */
typedef enum hskl_coords_       hskl_coords;      /* Choice of coordinate systems in which to provide pose and mesh data */
typedef enum hskl_model_        hskl_model;       /* Articulated models which we currently support for tracking */
typedef enum hskl_sensor_       hskl_sensor;      /* Depth sensors which we can accept as inputs */
typedef enum hskl_api_version_  hskl_api_version; /* Version of the library we are compiling against */

hskl_tracker                    hsklCreateTracker       ( hskl_coords coords, hskl_api_version version ); /* Instantiate tracking context, always pass HSKL_API_VERSION for version */
void                            hsklSetModelType        ( hskl_tracker tracker, hskl_model model ); /* What should the tracking engine track? Default is HSKL_MODEL_RIGHT_HAND */
void                            hsklSetHandMeasurements ( hskl_tracker tracker, float width, float length ); /* Width, length in meters. Defaults to 0.08 and 0.19 */							
void                            hsklSetSensorProperties ( hskl_tracker tracker, hskl_sensor sensor, int width, int height, float fov_x, float fov_y ); /* Which sensor will be used, and in what mode? */
void                            hsklTrackOneFrame       ( hskl_tracker tracker, const unsigned short * depth, const unsigned short * ir );
void                            hsklDestroyTracker      ( hskl_tracker tracker );

hskl_float3                     hsklGetPosition         ( hskl_tracker tracker, int bone ); /* World space position of bone, as 3D vector describing translation from reference pose */
hskl_float4                     hsklGetOrientation      ( hskl_tracker tracker, int bone ); /* World space orientation of bone, as quaternion describing rotation from reference pose */
float                           hsklGetTrackingError    ( hskl_tracker tracker, int bone ); /* Nonnegative fitting error in tracking algorithm. Lower values indicate better fit. */
const unsigned char *           hsklGetSegmentationMask ( hskl_tracker tracker );           /* Mask corresponding to depth buffer indicating pixels that have been used to track a given hand. */

int                             hsklGetBoneCount        ( hskl_tracker tracker );           /* Number of bones being tracked. Use zero-based indices to access bones. */
hskl_float3                     hsklGetCenterOfMass     ( hskl_tracker tracker, int bone ); /* Center of volume occupied by bone's tracking mesh, in local space of bone */
int                             hsklGetVertexCount      ( hskl_tracker tracker, int bone ); /* Length of array of vertices from bone's tracking mesh */
const hskl_float3 *             hsklGetVertices         ( hskl_tracker tracker, int bone ); /* Pointer to array of vertices from bone's tracking mesh in reference pose */
int                             hsklGetTriangleCount    ( hskl_tracker tracker, int bone ); /* Length of array of triangles from bone's tracking mesh */
const hskl_int3 *               hsklGetTriangles        ( hskl_tracker tracker, int bone ); /* Pointer to array of triangles from bone's tracking mesh */
int                             hsklGetParentBone       ( hskl_tracker tracker, int bone ); /* Index of parent bone of bone, or HSKL_BAD_INDEX if bone is at root of hierarchy */
hskl_float3                     hsklGetAnchorPoint      ( hskl_tracker tracker, int bone ); /* Point at which bone is attached to parent bone, in local space of parent bone */

enum {                          HSKL_BAD_INDEX = -1 };             /* Index used to represent "no bone" */
enum hskl_api_version_ {        HSKL_API_VERSION = 1            }; /* Must pass this constant to hsklCreateTracker */
enum hskl_coords_ {             HSKL_COORDS_X_RIGHT_Y_DOWN_Z_FWD , /* Right handed coordinates with XY in camera plane and Z along increasing depth */
                                HSKL_COORDS_X_RIGHT_Y_UP_Z_FWD  }; /* Left handed coordinates with XY in camera plane and Z along increasing depth */
enum hskl_model_ {              HSKL_MODEL_RIGHT_HAND            , /* 0-16: forearm, palm, pinky{base/mid/tip}, ring{base/mid/tip}, middle{base/mid/tip}, index{base/mid/tip}, thumb{base/mid/tip} */
                                HSKL_MODEL_LEFT_HAND             , /* 0-16: forearm, palm, pinky{base/mid/tip}, ring{base/mid/tip}, middle{base/mid/tip}, index{base/mid/tip}, thumb{base/mid/tip} */
                                HSKL_MODEL_TWO_HAND             }; /* 0-16: right hand bones, 17-33: left hand bones */
enum hskl_sensor_ {             HSKL_SENSOR_IDEAL                , /* Ideal sensor which produces depth data with no error */
                                HSKL_SENSOR_CREATIVE            }; /* Creative Interactive Gesture Camera */

#endif

#ifdef __cplusplus
}
#endif