
package com.thb.screenshot;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import com.thb.screenshot.jni.Jni;

import java.io.DataOutputStream;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void screenShot(View v) {
        if (!setReadable())
            return;
        int i = Jni.getFrameBuffer();
        if (i == -1) {
            Toast.makeText(MainActivity.this, "生成文件失败！",
                    Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(MainActivity.this, "生成文件成功！",
                    Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * 应用程序运行命令获取 Root权限，设备必须已破解(获得ROOT权限)
     * 
     * @return 应用程序是/否获取Root权限
     */
    private boolean getRoot() {
        String pkgCodePath = getPackageCodePath();
        Process process = null;
        DataOutputStream os = null;
        try {
            String cmd = "chmod 777 " + pkgCodePath;
            process = Runtime.getRuntime().exec("su"); // 切换到root帐号
            os = new DataOutputStream(process.getOutputStream());
            os.writeBytes(cmd + "\n");
            os.writeBytes("exit\n");
            os.flush();
            process.waitFor();
        } catch (Exception e) {
            return false;
        } finally {
            try {
                if (os != null) {
                    os.close();
                }
                process.destroy();
            } catch (Exception e) {
            }
        }
        return true;
    }

    public boolean setReadable() {
        Process process = null;
        DataOutputStream os = null;
        try {
            String cmd = "chmod 777 /dev/graphics/fb0";
            process = Runtime.getRuntime().exec("su"); // 切换到root帐号
            os = new DataOutputStream(process.getOutputStream());
            os.writeBytes(cmd + "\n");
            os.writeBytes("exit\n");
            os.flush();
            process.waitFor();
        } catch (Exception e) {
            return false;
        } finally {
            try {
                if (os != null) {
                    os.close();
                }
                process.destroy();
            } catch (Exception e) {
            }
        }
        return true;
    }
}
