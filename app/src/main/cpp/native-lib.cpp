#include <jni.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <android/bitmap.h>
#include <android/log.h>

#include <string.h>
#include <iostream>
#include <vector>

char APPNAME[] = {"app"};
using namespace cv;
/*
 *  计算圆内灰度值方法
 * @param src_gray 灰度图像
 * @param cen 圆中心
 * @param r 圆半径
 * */
int graylevel(Mat srcImage, Point center, int r)//求取圆形区域内的平均灰度值
{
    int graysum = 0;
    int pixCircleNumber = 0;
//    for (int row = (center.y - r); row <= (center.y + r); row++)//遍历圆内像素
//    {
//        for (int col = (center.x - r); col <= (center.x + r); ++col) {
//            double d = (row - center.y) * (row - center.y) + (col - center.x) * (col - center.x);
//            if(d < r * r){//三通道且在圆内
//                //得到像素值
//                int b = srcImage.at<Vec3b>(row, col)[0];
//                int g = srcImage.at<Vec3b>(row, col)[1];
//                int r = srcImage.at<Vec3b>(row, col)[2];
//                //计算平均灰度值
//                int gray=(b+g+r)/3;
//                //像素数自增
//                ++pixCircleNumber;
//                graysum += (int) gray;
//            }
//        }
//    }
    for (int row = (center.y - r); row <= (center.y + r); row++)//遍历圆内像素
    {
        uchar* data  = srcImage.ptr<uchar>(row);
        for (int col = (center.x - r); col <= (center.x + r); ++col) {
            double d = (row - center.y) * (row - center.y) + (col - center.x) * (col - center.x);
            if(d < r * r){//三通道且在圆内
                //像素数自增
                ++pixCircleNumber;
                graysum += (int)data[col];
            }
        }
    }
    return (graysum / pixCircleNumber);
}
/*
 *  计算灰度值
 * @param bitmap 拍照图像
 * */
extern "C"
JNIEXPORT jdouble JNICALL
Java_terry_com_greyleveltoconcentrationcamera_MainActivity_getBitmapGray(JNIEnv *env,
                                                                             jobject instance,
                                                                             jobject bitmap) {

    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Scaning getMagicColorBitmap");
    int ret;
    AndroidBitmapInfo info;
    void* pixels = 0;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME,"AndroidBitmap_getInfo() failed ! error=%d", ret);
        return -1;
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888 )
    {       __android_log_print(ANDROID_LOG_VERBOSE, APPNAME,"Bitmap format is not RGBA_8888!");
        return -1;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        __android_log_print(ANDROID_LOG_VERBOSE, APPNAME,"AndroidBitmap_lockPixels() failed ! error=%d", ret);
    }
    //读取bitmap到Mat
    Mat srcImage(info.height, info.width, CV_8UC4, pixels);
    //彩色图像转化成灰度图
    Mat src_gray;
    cvtColor(srcImage, src_gray, COLOR_BGR2GRAY);
    //计算平均灰度
    Point center(cvRound(src_gray.cols / 2), cvRound(src_gray.rows / 2));
    int radius = cvRound(300);
    int average = graylevel(src_gray, center, radius);
    return average;
}
/*
 *  生成圆形图像
 * @param buf 拍摄图像int数组
 * @param w 宽度
 * @param h 高度
 * */
extern "C"
JNIEXPORT jintArray JNICALL
Java_terry_com_greyleveltoconcentrationcamera_MainActivity_getCirclePicture(JNIEnv *env, jobject instance,
                                                                jintArray buf, jint w, jint h) {
    //得到图像结果
    jint *cbuf = env->GetIntArrayElements(buf, JNI_FALSE );
    if (cbuf == NULL) {
        return 0;
    }
    Mat srcImage(h, w, CV_8UC4, (unsigned char *) cbuf);//源图像
    Mat dst = Mat::zeros(srcImage.size(), srcImage.type());//结果图像
    Mat mask = Mat::zeros(srcImage.size(),CV_8U);//掩码
    //得到圆形处理图像
    Point circleCenter(mask.cols / 2, mask.rows / 2);
    int radius = 400;
    circle(mask, circleCenter, radius, Scalar(255),-1);
    srcImage.copyTo(dst, mask);

    uchar *ptr = dst.data;
    //创建新的结果数组
    int size = w * h;
    jintArray result = env->NewIntArray(size);
    //将结果设置到新的数组
    env->SetIntArrayRegion(result, 0, size, (const jint *) ptr);
    //释放中间数组
    env->ReleaseIntArrayElements(buf, cbuf, 0);
    return result;
}