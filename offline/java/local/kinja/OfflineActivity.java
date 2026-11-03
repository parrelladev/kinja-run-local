package local.kinja;

import android.content.res.AssetManager;
import android.os.Bundle;
import com.unity3d.player.UnityPlayerActivity;

/** Launcher subclass retaining all original Unity lifecycle behavior. */
public final class OfflineActivity extends UnityPlayerActivity {
    private static AssetManager assetManager;

    static {
        System.loadLibrary("kinjaoffline");
    }

    private static native void configure(String filesDirectory, AssetManager manager);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        assetManager = getAssets();
        configure(getFilesDir().getAbsolutePath(), assetManager);
        super.onCreate(savedInstanceState);
    }
}
