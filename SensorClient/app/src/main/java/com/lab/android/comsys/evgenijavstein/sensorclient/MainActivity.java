package com.lab.android.comsys.evgenijavstein.sensorclient;

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.lab.android.comsys.evgenijavstein.sensorclient.packet.Shake;

import java.util.ArrayList;
import java.util.List;


public class MainActivity extends ActionBarActivity {
    Button sendButton = null;
    Button registerButton = null;
    Button unregisterButtton = null;
    private SensorClient sensorClient = null;
    private ListView shakeEventListView = null;
    private EditText ipAdressField = null;
    private EditText portEditText = null;
    private EditText nameEditText = null;
    private List<Shake> shakeEventList = new ArrayList<Shake>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ipAdressField = (EditText) findViewById(R.id.ipAdressField);
        portEditText = (EditText) findViewById(R.id.portField);
        nameEditText = (EditText) findViewById(R.id.nameField);
        registerButton = (Button) findViewById(R.id.registerButton);
        unregisterButtton = (Button) findViewById(R.id.unregisterButton);
        ipAdressField.setText("192.168.2.104");
        portEditText.setText("3000");


        sendButton = (Button) findViewById(R.id.sendEventButton);
        shakeEventListView = (ListView) findViewById(R.id.shakeEventList);
        final BaseAdapter adapter = new BaseAdapter() {
            @Override
            public int getCount() {
                return shakeEventList.size();
            }

            @Override
            public Object getItem(int position) {
                return shakeEventList.get(position);
            }

            @Override
            public long getItemId(int position) {
                return position;
            }

            @Override
            public View getView(int position, View convertView, ViewGroup parent) {
                TextView textView = new TextView(MainActivity.this);
                textView.setText(shakeEventList.get(position).getName() + " : " +
                        shakeEventList.get(position).getHumanTimeShaken());
                return textView;

            }
        };
        shakeEventListView.setAdapter(adapter);
        sensorClient = SensorClient.newInstance();
        sensorClient.setSensorServerIp(ipAdressField.getText().toString());
        sensorClient.setSensorServerPort(Integer.parseInt(portEditText.getText().toString()));

        sensorClient.setOnShakeListener(new SensorClient.OnShakeListener() {
            @Override
            public void onShakeReceived(Shake shake) {
                shakeEventList.add(shake);
                adapter.notifyDataSetChanged();
            }

            @Override
            public void onError(String message) {
                Toast.makeText(MainActivity.this, "An error occured:" + message, Toast.LENGTH_SHORT).show();
            }
        });
        registerButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sensorClient.register(nameEditText.getText().toString());
                registerButton.setEnabled(false);
            }
        });

        sendButton.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View view) {
                sensorClient.sendEvent();
            }
        });
        unregisterButtton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                unregisterButtton.setEnabled(false);
                sensorClient.unregister();
                registerButton.setEnabled(true);
                unregisterButtton.setEnabled(true);
            }
        });
    }


}
