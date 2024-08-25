package com.example.privatechats;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class LoginActivity extends AppCompatActivity {

    private EditText editTextPhone;
    private EditText editTextName;
    private EditText editTextIp;
    private Button buttonSubmit;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        editTextPhone = findViewById(R.id.editTextPhone);
        editTextName = findViewById(R.id.editTextName);
        editTextIp = findViewById(R.id.editTextIp);
        buttonSubmit = findViewById(R.id.buttonSubmit);

        buttonSubmit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String phone = editTextPhone.getText().toString().trim();
                String name = editTextName.getText().toString().trim();
                String ip = editTextIp.getText().toString().trim();

                if (!phone.isEmpty() && !name.isEmpty() && !ip.isEmpty()) {
                    SharedPreferences sharedPreferences = getSharedPreferences("UserDetails", MODE_PRIVATE);
                    SharedPreferences.Editor editor = sharedPreferences.edit();
                    editor.putString("Phone", phone);
                    editor.putString("Name", name);
                    editor.putString("IP", ip);
                    editor.apply();

                    Log.d("LoginActivity", "User details saved: Phone=" + phone + ", Name=" + name + ", IP=" + ip);

                    Operations client = Operations.getInstance(LoginActivity.this, ip);

                    new Thread(() -> {
                        try {
                            boolean isConnected = client.connectToServer();
                            if (isConnected) {
                                Log.d("LoginActivity", "Successfully connected to server.");
                                client.startListeningForMessages();
                                client.registerClient(phone);
                                Log.d("LoginActivity", "Client registration successful.");

                                runOnUiThread(() -> {
                                    Intent intent = new Intent(LoginActivity.this, MainActivity.class);
                                    startActivity(intent);
                                    finish();
                                });
                            } else {
                                Log.e("LoginActivity", "Failed to connect to server.");
                                runOnUiThread(() -> {
                                    Toast.makeText(LoginActivity.this, "Cannot connect to server.", Toast.LENGTH_SHORT).show();
                                });
                            }
                        } catch (Exception e) {
                            Log.e("LoginActivity", "Error during connection/registration.", e);
                            runOnUiThread(() -> {
                                Toast.makeText(LoginActivity.this, "An error occurred. Please try again.", Toast.LENGTH_SHORT).show();
                            });
                            client.disconnect();  // Ensure the client disconnects on failure
                        }
                    }).start();
                } else {
                    Toast.makeText(LoginActivity.this, "Please fill in all fields", Toast.LENGTH_SHORT).show();
                }
            }
        });

    }
}
