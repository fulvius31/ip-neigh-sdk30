package it.alessangiorgi.ipneigh30;

import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.Reader;
import java.io.StringReader;

public class ArpNDK {

    private static final String TAG = "ArpNDK";

    static {
        System.loadLibrary("ipneigh-android");
    }
    public static native String ARPFromJNI();

    private static String ipNeighborMac(String host) {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append(ARPFromJNI());
        try {

            Reader inputString = new StringReader(stringBuilder.toString());

            BufferedReader bufferedReader = new BufferedReader(inputString);

            String line;
            while ((line = bufferedReader.readLine()) != null) {
                Log.e(TAG, line);

                /*if (line.endsWith("FAILED") || line.endsWith("INCOMPLETE")) continue;
                if (line.contains("::")) continue;*/
                String ipLine = line.split(" ")[0];

                if (ipLine.equals(host)) {
                    String[] macLine = line.split(" ");
                    String mac = macLine[macLine.length - 2];
                    return mac;
                }
            }

        } catch (IOException ignore) {
        }
        return null;
    }

}
