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
    for(int i = (center.y - r); i <= (center.y + r); ++i)//访问矩形框内的像素值
    {
        uchar* data = srcImage.ptr<uchar>(i);
        for(int j = (center.x - r); j <= (center.x + r); ++j)
        {
            double d = (i-center.y)*(i-center.y) + (j-center.x)*(j-center.x);
            if(d < r*r)
            {
                ++pixCircleNumber;
                graysum += (int)data[j];
            }
        }
    }
    return (graysum / pixCircleNumber);
}

int avgGray(int(*p), Mat img) {

    //读入原图片，并获取长宽
    int length = img.rows;
    int width = img.cols;
    //建立一个全是0的图片掩膜
    Mat imgM(length, width, CV_8UC1, Scalar(0));
    Point points[1][4];
    points[0][0] = Point(p[0], p[1]);
    points[0][1] = Point(p[2], p[3]);
    points[0][2] = Point(p[4], p[5]);
    points[0][3] = Point(p[6], p[7]);
    //设定传入点的个数
    int npt[] = { 4 };
    //将四个点围成的区域像素变为1
    const Point *pt[1] = { points[0] };
    fillPoly(imgM, pt, npt, 1, Scalar(1));
    //将图片与掩膜对应相乘,得到只有区域部分非零的图像
    Mat reginImg = img.mul(imgM);
    //求出来四边形区域内这些像素值的总和。
    Scalar theSum = sum(reginImg);
    double theSumDouble = theSum.val[0]; //因为求和后是Scalar类型的数据，是含有一个4维数组的结构体，所以需要转化这个sum为double类型。
    int theNum = countNonZero(reginImg);   //求出这个四边形区域内有多少个像素值。
    int theAvg = theSumDouble / theNum;
    return theAvg;
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
    Point center(cvRound(srcImage.cols / 2), cvRound(srcImage.rows / 2));
    int rectLength=100;
    int pointCoor[4][2]={
            {center.x-rectLength,center.y-rectLength},
            {center.x+rectLength,center.y-rectLength},
            {center.x+rectLength,center.y+rectLength},
            {center.x-rectLength,center.y+rectLength},
                        };
    int points[8] = { pointCoor[0][0], pointCoor[0][1], pointCoor[1][0], pointCoor[1][1],
                      pointCoor[2][0], pointCoor[2][1], pointCoor[3][0], pointCoor[3][1]};
    int theAvgGray = avgGray(points, srcImage);
    return theAvgGray;
//    //彩色图像转化成灰度图
//    Mat src_gray;
//    cvtColor(srcImage, src_gray, COLOR_BGR2GRAY);
//    //对灰度图像进行双边滤波
//    const int kvalue = 15;//双边滤波邻域大小
//    Mat bf;
//    bilateralFilter(src_gray, bf, kvalue, kvalue*2, kvalue/2);
//    //计算平均灰度
//    Point center(cvRound(bf.cols / 2), cvRound(bf.rows / 2));
//    int radius = cvRound(300);
//    int average = graylevel(bf, center, radius);
//    return average;
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