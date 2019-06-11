#ifndef REALSENSEAPI_H
#define REALSENSEAPI_H

#include <librealsense2/rs.hpp>
#include "Image.h"

enum Status {
    SUCCESS,
    CAMERA_NOT_CONNECTED,
    ONE_OF_THE_SENSORS_FAIL,
    RESS_NOT_FITTING_WITH_FPS
};


class RealSense{

public:
        enum class sensor_name //TODO update names
        {
            SRERIO_MODULE,
            RGB_CAMERA,
            MOTION_MODULE
        };

        //TODO hidden rs2_stream_profile & rs2_frame_format ?
        //TODO check if data type must go with specific ress&fps
//        enum rs2_stream_profile
//        {
//            STREAM_ANY,
//            STREAM_DEPTH                            , /**< Native stream of depth data produced by RealSense device */
//            STREAM_COLOR                            , /**< Native stream of color data captured by RealSense device */
//            STREAM_INFRARED                         , /**< Native stream of infrared data captured by RealSense device */
//            STREAM_FISHEYE                          , /**< Native stream of fish-eye (wide) data captured from the dedicate motion camera */
//            STREAM_GYRO                             , /**< Native stream of gyroscope motion data produced by RealSense device */
//            STREAM_ACCEL                            , /**< Native stream of accelerometer motion data produced by RealSense device */
//            STREAM_GPIO                             , /**< Signals from external device connected through GPIO */
//            STREAM_POSE                             , /**< 6 Degrees of Freedom pose data, calculated by RealSense device */
//            STREAM_CONFIDENCE                       , /**< 4 bit per-pixel depth confidence level */
//            STREAM_COUNT
//        };

        enum rs2FrameFormat
        {
            ORMAT_ANY             , /**< When passed to enable stream, librealsense will try to provide best suited format */
            FORMAT_Z16             , /**< 16-bit linear depth values. The depth is meters is equal to depth scale * pixel value. */
            FORMAT_DISPARITY16     , /**< 16-bit float-point disparity values. Depth->Disparity conversion : Disparity = Baseline*FocalLength/Depth. */
            FORMAT_XYZ32F          , /**< 32-bit floating point 3D coordinates. */
            FORMAT_YUYV            , /**< 32-bit y0, u, y1, v data for every two pixels. Similar to YUV422 but packed in a different order - https://en.wikipedia.org/wiki/YUV */
            FORMAT_RGB8            , /**< 8-bit red, green and blue channels */
            FORMAT_BGR8            , /**< 8-bit blue, green, and red channels -- suitable for OpenCV */
            FORMAT_RGBA8           , /**< 8-bit red, green and blue channels + constant alpha channel equal to FF */
            FORMAT_BGRA8           , /**< 8-bit blue, green, and red channels + constant alpha channel equal to FF */
            FORMAT_Y8              , /**< 8-bit per-pixel grayscars2_stream_profilele image */
            FORMAT_Y16             , /**< 16-bit per-pixel grayscale image */
            FORMAT_RAW10           , /**< Four 10 bits per pixel luminance values packed into a 5-byte macropixel */
            FORMAT_RAW16           , /**< 16-bit raw image */
            FORMAT_RAW8            , /**< 8-bit raw image */
            FORMAT_UYVY            , /**< Similar to the standard YUYV pixel format, but packed in a different order */
            FORMAT_MOTION_RAW      , /**< Raw data from the motion sensor */
            FORMAT_MOTION_XYZ32F   , /**< Motion data packed as 3 32-bit float values, for X, Y, and Z axis */
            FORMAT_GPIO_RAW        , /**< Raw data from the external sensors hooked to one of the GPIO's */
            FORMAT_6DOF            , /**< Pose data packed as floats array, containing translation vector, rotation quaternion and prediction velocities and accelerations vectors */
            FORMAT_DISPARITY32     , /**< 32-bit float-point disparity values. Depth->Disparity conversion : Disparity = Baseline*FocalLength/Depth */
            FORMAT_Y10BPACK        , /**< 16-bit per-pixel grayscale image unpacked from 10 bits per pixel packed ([8:8:8:8:2222]) grey-scale image. The data is unpacked to LSB and padded with 6 zero bits */
            FORMAT_DISTANCE        , /**< 32-bit float-point depth distance value.  */
            FORMAT_COUNT             /**< Number of enumeration values. Not a valid input: intended to be used in for-loops. */
        };


        enum class rs2ColorRessolution
        {
            R_1920x1080,
            R_1280x720,
            R_960_540,
            R_848_480,
            R_640x480,
            R_640x360,
            R_424x240,
            R_320x240,
            R_320x180
        };

        enum class rs2fps
        {
            F_60hz,
            F_30hz,
            F_15hz,
            F_6hz
        };



       RealSense() = default;



       /**
        * @brief connectCamera - assumes that only 1 cammera connected to the divice
        * the function detect the cammera initilize the sensors of the camera
        * @return
        */
       Status connectCamera();

       /**TODO functions to:
        * get_depth_units :             //A Depth stream contains an image that is composed of pixels with depth information.
        //The value of each pixel is the distance from the camera, in some distance units.
        //To get the distance in units of meters, each pixel's value should be multiplied by the sensor's depth scale
        //Here is the way to grab this scale value for a "depth" sensor:
        *
        * get_field_of_view:
        *         // A sensor's stream (rs2::stream_profile) is in general a stream of data with no specific type.
        // For video streams (streams of images), the sensor that produces the data has a lens and thus has properties such
        //  as a focal point, distortion, and principal point.
        // To get these intrinsics parameters, we need to take a stream and first check if it is a video stream

        get_extrinsics:
        // If the device/sensor that you are using contains more than a single stream, and it was calibrated
        // then the SDK provides a way of getting the transformation between any two streams (if such exists)
       */


       //TODO right resulotions per fps
       //TODO which formats support
//        Status setupInfraredImage(rs2FrameFormat format,rs2_ressolution_and_fps ress_fps_set);

        //TODO which formats support
//        Status setupDepthImage(rs2FrameFormat format,rs2_ressolution_and_fps ress_fps_set);


       /**
         * @brief setupColorImage - setup the camera to steam wanted frames.
         * notice that color camera cant stream at ress: 1920x1080 or 1280x720 at 60 fps
         * and cant stream at ress: 320x240 or 320x180 at 15 fps
         * @param format: supported formats:
         * RS2_FORMAT_RGB8
         * RS2_FORMAT_BGR8
         * RS2_FORMAT_RGBA8
         * RS2_FORMAT_BGRA8
         * RS2_FORMAT_YUYV
         * RS2_FORMAT_Y16
         * @param ressolution
         * @param fps
         * @return
         */
        Status setupColorImage(/*rs2_format format,*/rs2ColorRessolution ressolution,rs2fps fps);


        /**
         * @brief startCamera - after setups all wanted frames and start
         * streaming according to the configuraion
         */
        void startCamera();


        /**
         * @brief captureFrame - save frame for future use
         * before getting data of specific frame need call this function.
         * this function will wait setuped frames from the camera.
         */
        void captureFrame();


        /**
         * @brief getColorImage - get Color image (accordint the color setups) according
         * the last capture.
         * @return
         */
        Image getColorImage();






private:

      rs2::device _camera;
      //for config settings
      rs2::sensor _stereo_module;
      rs2::sensor _rgb_camera;
      rs2::sensor _motion_module;

      rs2::pipeline _pipe;
      rs2::config _config;

      //TODO add capture
      rs2::frameset _frames;

};

#endif // REALSENSEAPI_H
