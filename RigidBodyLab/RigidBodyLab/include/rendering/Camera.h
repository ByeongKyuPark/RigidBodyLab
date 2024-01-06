#pragma once
#include <math/Math.h>
namespace Rendering 
{
    class Camera
    {
        int     width, height;      /*  Viewport width & height */
        bool    resized;            /*  to trigger projection matrix update */
        float   nearPlane, farPlane, topPlane, bottomPlane, rightPlane, leftPlane;

        int     radius;
        int     alpha;
        int     beta;
        Vec3    upVec;
        bool    moved;              /*  to trigger view matrix update */
        Vec3    pos, lookAt;

        friend class Renderer;
    public:
        /*  Default parameters construct a camera
            lying along the z-axis at CAM_MAX_RADIUS from origin.
        */

        Camera(int _width= Camera::DISPLAY_SIZE, int _height= Camera::DISPLAY_SIZE) :
            width(_width), height(_height),
            resized(true),
            nearPlane(0.1f * CAM_RADIUS_STEP), farPlane(5.0f * CAM_MAX_RADIUS),
            topPlane(nearPlane / 900.0f * height), bottomPlane(-topPlane),
            rightPlane(topPlane* width / height), leftPlane(-rightPlane),
            radius(INIT_RADIUS), alpha(INIT_ALPHA), beta(INIT_BETA),
            upVec(BASIS[Y]), lookAt(Vec3(0, 0, 0)), pos{},moved(true) { }

        /*  Functions to update camera position upon user input */
        void Reset();
        void MoveUp();
        void MoveDown();
        void MoveLeft();
        void MoveRight();
        void MoveCloser();
        void MoveFarther();
        void ComputeUpVec();
        Mat4 ViewMat();
        Mat4 ProjMat() const;
        Vec3 GetPos()const { return pos; }
        Vec3 GetUpVec()const { return upVec; }

        int GetWidth() const { return width; }
        int GetHeight() const { return height; }

        static constexpr int DISPLAY_SIZE = 1080;
        static constexpr int GUI_WIDTH = 400;
        /*  The angles and radius in this file are defined as integers to avoid accumulated imprecision */

        /*  The steps for rotation */
        static constexpr int   NUM_STEPS_PI = 36;
        static constexpr int   NUM_STEPS_TWO_PI = NUM_STEPS_PI * 2;
        static constexpr int   NUM_STEPS_HALF_PI = NUM_STEPS_PI / 2;
        static constexpr int   NUM_STEPS_QUARTER_PI = NUM_STEPS_PI / 4;
        static constexpr int   NUM_STEPS_ONEANDHALF_PI = NUM_STEPS_HALF_PI * 3;
        static constexpr float ONE_STEP = PI / NUM_STEPS_PI;

        /*  Step for camera angle change */
        static constexpr int CAM_ANGLE_STEP = 1;
        /*  Limit for camera distance from the origin */
        static constexpr int CAM_MAX_RADIUS = 20;
        static constexpr int CAM_MIN_RADIUS = 3;
        static constexpr int CAM_RADIUS_STEP = 1;
        static constexpr int INIT_ALPHA = 5;
        static constexpr int INIT_BETA = 22;     /*  90 degrees */
        static constexpr int INIT_RADIUS = 10;
    };


    /*  Pre-defined camera */
    extern Camera mainCam;
    extern Camera mirrorCam;
}