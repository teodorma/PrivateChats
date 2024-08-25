package com.example.privatechats;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

public class TcpClientService extends Service {

    private final IBinder binder = new LocalBinder();
    private Operations tcpClient;

    public class LocalBinder extends Binder {
        public TcpClientService getService() {
            return TcpClientService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d("TcpClientService", "Service created");
    }

    public void initializeTcpClient(String ip, String phone) {
        if (tcpClient == null) {
            tcpClient = Operations.getInstance(this, ip);
            new Thread(() -> {
                try {
                    if (tcpClient.connectToServer()) {
                        tcpClient.registerClient(phone);
                        tcpClient.startListeningForMessages();
                        Log.d("TcpClientService", "TCP client initialized and listening.");
                    } else {
                        Log.e("TcpClientService", "Failed to connect to the server.");
                    }
                } catch (Exception e) {
                    Log.e("TcpClientService", "Error initializing TCP client", e);
                }
            }).start();
        }
    }

    public Operations getTcpClient() {
        return tcpClient;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (tcpClient != null) {
            tcpClient.disconnect();
        }
        Log.d("TcpClientService", "Service destroyed");
    }
}
