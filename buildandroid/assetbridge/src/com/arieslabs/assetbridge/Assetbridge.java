package com.arieslabs.assetbridge;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;



public class Assetbridge {
    static {
        System.loadLibrary("assetbridge");
    }


    // unpack 
    public static void unpack(Context c) {

        try {
            // first let's get the temp directory
            String tmpdir = c.getCacheDir().getPath();

            // now we need the assetmanager
            AssetManager am = c.getAssets();
            String[] assets = am.list("");

            // iterate on the files...
            for(String asset : assets) {
                copyAssetFolder(am, asset, tmpdir + "/" + asset);
            }

            // last, set the ASSETDIR environment variable for the C
            // parts of the procee
            setassetdir(c.getCacheDir().getPath());

        } catch (IOException e) {
            Log.e("Assetbridge", "Can't unpack assets from APK", e);
        }

    }


    public static void copyAssetFolder(AssetManager am, String src, String dest)
    	throws IOException{

        InputStream srcIS = null;
        File destfh;

        // this is the only way we can tell if this is a file or a
        // folder - we have to open the asset, and if the open fails,
        // it's a folder...
        boolean isDir = false;
        try {
            srcIS = am.open(src);
        } catch (FileNotFoundException e) {
            isDir = true;
        }

        // either way, we'll use the dest as a File
        destfh = new File(dest);

        // and now, depending on ..
    	if(isDir) {

            // If the directory doesn't yet exist, create it
            if( !destfh.exists() ){
                destfh.mkdir();
            }

            // list the assets in the directory...
            String assets[] = am.list(src);

            // and copy them all using same.
            for(String asset : assets) {
                copyAssetFolder(am, src + "/" + asset, dest + "/" + asset);
            }

    	} else {

            int count, buffer_len = 2048;
            byte[] data = new byte[buffer_len];

            // copy the file from the assets subsystem to the filesystem
            FileOutputStream destOS = new FileOutputStream(destfh);

    	    //copy the file content in bytes 
            while( (count = srcIS.read(data, 0, buffer_len)) != -1) {
                destOS.write(data, 0, count);
            }

            // and close the two files
            srcIS.close();
            destOS.close();

    	}
    }


    // the native method to set the environment variable
    public static native void setassetdir(String s);
}
