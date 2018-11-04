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
    //复制图像
    Mat dst = mbgra.clone();
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
    int radius = min(mask.cols, mask.rows)/2;
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