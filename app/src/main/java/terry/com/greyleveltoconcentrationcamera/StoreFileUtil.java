package terry.com.greyleveltoconcentrationcamera;

import android.os.Environment;

import java.io.File;

public class StoreFileUtil
{
    //当前存储照片对应的文件
    static File currFile;

    //获取下一个可以存储的文件
    public static File nextFile()
    {
        //获取闪存卡路径
        String path= Environment.getExternalStorageDirectory().getAbsolutePath();
        //寻找可以使用的文件名
        int c=0;
        File fTest=new File(path+"/mc"+c+".jpg");
        while(fTest.exists())
        {
            c++;
            fTest=new File(path+"/mc"+c+".jpg");
        }
        currFile=fTest;
        return fTest;
    }

    //临时文件
    public static File tempFile()
    {
        //获取闪存卡路径
        String path= Environment.getExternalStorageDirectory().getAbsolutePath();
        File fTest=new File(path+"/BNtemp.jpg");
        return fTest;
    }
}
