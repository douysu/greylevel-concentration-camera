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
//dst为传入的灰色图像
int graylevel(Mat dst, Point p1,int rectSize)//求取圆形区域内的平均灰度值
{
    int graysum = 0, n = 0;
    for (int i = p1.y; i <= (p1.y+2*rectSize); ++i)
    {
        uchar *data = dst.ptr<uchar>(i);
        for (int j =  p1.y; j <= ( p1.y+2*rectSize); ++j) {
                ++n;
                graysum += (int) data[j];
        }
    }
    return (graysum / n);
}
extern "C"
JNIEXPORT jdouble JNICALL
Java_terry_com_greyleveltoconcentrationcamera_MainActivity_getBitmapMeanGray(JNIEnv *env,
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
    Mat mbgra(info.height, info.width, CV_8UC4, pixels);
    //彩色图像转化成灰度图
    Mat src_gray;
    cvtColor(mbgra, src_gray, COLOR_BGR2GRAY);
    //计算平均灰度
    int rectSize=50;
    /*
     * p1 --------
     *            |
     *    ------- |
     * */
    //左上角坐标
    Point center(cvRound(mbgra.cols / 2), cvRound(mbgra.rows / 2));
    Point p1(cvRound(center.x-rectSize),cvRound(center.y-rectSize));
    int average = graylevel( src_gray, p1,rectSize);
    return average;
}
