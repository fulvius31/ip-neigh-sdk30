package it.alessangiorgi.ipneigh30;

import android.os.ParcelFileDescriptor;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;


public class ArpNDK {

    private static final String TAG = "ArpNDK";
    private static final String ARPNDK_FAILED = "Arp failed";

    static {
        System.loadLibrary("ipneigh-android");
    }

    private static native int ARPFromJNI(int fd);

    public static String getARP() {
        StringBuilder stringBuilder = new StringBuilder();
        try {
            ParcelFileDescriptor[] pipe = ParcelFileDescriptor.createPipe();
            ParcelFileDescriptor readSidePfd = pipe[0];
            ParcelFileDescriptor writeSidePfd = pipe[1];
            ParcelFileDescriptor.AutoCloseInputStream inputStream = new ParcelFileDescriptor.AutoCloseInputStream(readSidePfd);
            int fd_write = writeSidePfd.detachFd();
            int returnCode = ARPFromJNI(fd_write);

            if(returnCode != 0)
                return ARPNDK_FAILED;

            BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream, StandardCharsets.UTF_8));
            String line = "";
            while ((line = reader.readLine()) != null) {
                Log.e(TAG, "getARP: "+line );
                stringBuilder.append(line).append("\n");
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return stringBuilder.toString();
    }

}
