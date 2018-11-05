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

int graylevel(Mat image, Mat dst, Point cen, int r)//求取圆形区域内的平均灰度值
{
    int graysum = 0, n = 0;

    for (int i = (cen.y - r); i <= (cen.y + r); ++i)
    {
        uchar *data = image.ptr<uchar>(i);
        for (int j = (cen.x - r); j <= (cen.x + r); ++j) {
            double d = (i - cen.y) * (i - cen.y) + (j - cen.x) * (j - cen.x);
            if (d < r * r) {
                ++n;
                graysum += (int) data[j];
            }
        }
    }

    for (int i = (cen.y - r); i <= (cen.y + r); ++i)
    {
        uchar *temp = dst.ptr<uchar>(i);
        for (int j = (cen.x - r); j <= (cen.x + r); ++j) {
            double d = (i - cen.y) * (i - cen.y) + (j - cen.x) * (j - cen.x);
            if (d < r * r) {
                temp[j] = (int) (graysum / n);
            }
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
    //对灰度图像进行双边滤波
    Mat bf;
    bilateralFilter(src_gray, bf, 15, 15*2, 7.5f);
    //结果图像
    Mat dst(src_gray.size(), src_gray.type());
    dst = Scalar::all(0);
    //计算平均灰度
    Point center(cvRound(mbgra.cols / 2), cvRound(mbgra.rows / 2));
    int radius = cvRound(200);
    int average = graylevel(bf, dst, center, radius);
    return average;
}
