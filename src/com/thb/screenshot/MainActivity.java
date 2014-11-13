
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
            Toast.makeText(MainActivity.this, "�����ļ�ʧ�ܣ�",
                    Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(MainActivity.this, "�����ļ��ɹ���",
                    Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * Ӧ�ó������������ȡ RootȨ�ޣ��豸�������ƽ�(���ROOTȨ��)
     * 
     * @return Ӧ�ó�����/���ȡRootȨ��
     */
    private boolean getRoot() {
        String pkgCodePath = getPackageCodePath();
        Process process = null;
        DataOutputStream os = null;
        try {
            String cmd = "chmod 777 " + pkgCodePath;
            process = Runtime.getRuntime().exec("su"); // �л���root�ʺ�
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
            process = Runtime.getRuntime().exec("su"); // �л���root�ʺ�
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
