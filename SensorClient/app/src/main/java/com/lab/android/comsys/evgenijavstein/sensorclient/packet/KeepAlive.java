package com.lab.android.comsys.evgenijavstein.sensorclient.packet;

import com.lab.android.comsys.evgenijavstein.sensorclient.SensorClient;

/**
 * Created by evgenijavstein on 12/05/15.
 */
public class KeepAlive implements Packet {


    @Override
    public byte[] getBytes() {
        byte[] b=new byte[1];
        b[0]= SensorClient.TYPE_KEEPALIVE;
        return b;
    }
}
