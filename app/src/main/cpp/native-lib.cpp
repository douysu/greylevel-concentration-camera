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

    for (int i = (cen.y - r); i <= (cen.y + r); ++i)//访问矩形框内的像素值
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

    for (int i = (cen.y - r); i <= (cen.y + r); ++i)//画出圆，圆内像素值为平均灰度值
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
    //对图像进行圆处理
    Mat circleImage = Mat::zeros(mbgra.size(), mbgra.type());
    Mat mask = Mat::zeros(mbgra.size(),CV_8U);

    Point circleCenter(mask.cols / 2, mask.rows / 2);
    int radius = 600;

    circle(mask, circleCenter, radius, Scalar(255),-1);
    mbgra.copyTo(circleImage, mask);
    //复制图像
    Mat dst = circleImage.clone();
    Mat gray, mat_mean, mat_stddev;
    cvtColor(dst, gray, CV_RGB2GRAY); // 转换为灰度图，gray
    meanStdDev(gray, mat_mean, mat_stddev);//计算均值和标准偏差
    double m;
    m = mat_mean.at<double>(0, 0);
    return m;
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_terry_com_greyleveltoconcentrationcamera_MainActivity_gray(JNIEnv *env, jobject instance,
                                                                jintArray buf, jint w, jint h) {
    //得到图像结果
    jint *cbuf = env->GetIntArrayElements(buf, JNI_FALSE );
    if (cbuf == NULL) {
        return 0;
    }
    Mat srcImage(h, w, CV_8UC4, (unsigned char *) cbuf);

    Mat dst = Mat::zeros(srcImage.size(), srcImage.type());
    Mat mask = Mat::zeros(srcImage.size(),CV_8U);
    Point circleCenter(mask.cols / 2, mask.rows / 2);
    //int radius = min(mask.cols, mask.rows)/2;
    int radius = 600;
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