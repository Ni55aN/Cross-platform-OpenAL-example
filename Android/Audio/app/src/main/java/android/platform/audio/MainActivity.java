package android.platform.audio;

import android.content.res.AssetManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {


    private AssetManager mgr;

    static {
        System.loadLibrary("openal_example");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


    }


    public native int play(AssetManager mgr);

    public void oPlayClick(View view) {


        mgr = getResources().getAssets();
        play(mgr);
    }
}
