package com.dark.devel;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;
import android.view.Gravity;
import android.graphics.Color;

public class MainActivity extends AppCompatActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        TextView tv = new TextView(this);
        tv.setText("DARK DEVEL\nWORKING!\n\nAgar ye dikh raha hai to app sahi hai");
        tv.setGravity(Gravity.CENTER);
        tv.setTextSize(22);
        tv.setTextColor(Color.WHITE);
        tv.setBackgroundColor(Color.parseColor("#121212"));
        tv.setPadding(50, 300, 50, 300);
        setContentView(tv);
    }
}
