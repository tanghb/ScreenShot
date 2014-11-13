
package com.thb.screenshot.jni;

public class Jni {
    static {
        System.loadLibrary("save");
    }

    public static native int getFrameBuffer();
}
