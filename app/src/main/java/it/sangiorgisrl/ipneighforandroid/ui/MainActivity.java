package it.sangiorgisrl.ipneighforandroid.ui;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import it.alessangiorgi.ipneigh30.ArpNDK;
import it.sangiorgisrl.ipneighforandroid.R;

public class MainActivity extends AppCompatActivity {

    TextView arptable_res;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        arptable_res = findViewById(R.id.arptable);

        arptable_res.setText(ArpNDK.getARP());
    }
}