package com.example.privatechats.ui.home.chat;

import android.annotation.SuppressLint;
import android.content.ContentValues;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
import android.util.Log;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.example.privatechats.R;
import com.example.privatechats.Operations;
import com.example.privatechats.database.MessengerContract;
import com.example.privatechats.database.MessengerDbHelper;

import java.util.ArrayList;
import java.util.List;

public class ChatDetailActivity extends AppCompatActivity {

    private RecyclerView recyclerView;
    private MessageAdapter messageAdapter;
    private List<Message> messageList = new ArrayList<>();
    private EditText messageInput;
    private ImageButton sendButton;
    private String contactName;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_chat_detail);

        recyclerView = findViewById(R.id.recyclerViewMessages);
        messageInput = findViewById(R.id.messageInput);
        sendButton = findViewById(R.id.sendButton);

        Intent intent = getIntent();
        contactName = intent.getStringExtra("contactName");
        String contactPhoneNumber = intent.getStringExtra("contactPhoneNumber");

        TextView contactNameView = findViewById(R.id.contactName);
        contactNameView.setText(contactName);

        recyclerView.setLayoutManager(new LinearLayoutManager(this));
        messageAdapter = new MessageAdapter(messageList);
        recyclerView.setAdapter(messageAdapter);

        loadMessages();

        sendButton.setOnClickListener(v -> sendMessage());
    }
    @SuppressLint("NotifyDataSetChanged")
    private void loadMessages() {
        MessengerDbHelper dbHelper = new MessengerDbHelper(this);
        SQLiteDatabase db = dbHelper.getReadableDatabase();

        String[] projection = {
                MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_ID,
                MessengerContract.MessagesEntry.COLUMN_NAME_MESSAGE,
                MessengerContract.MessagesEntry.COLUMN_NAME_TIMESTAMP
        };


        String selection = MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_ID + " = ? OR " +
                MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_ID + " = ?";
        String[] selectionArgs = { contactName, contactName };

        Cursor cursor = db.query(
                MessengerContract.MessagesEntry.TABLE_NAME,
                projection,
                selection,
                selectionArgs,
                null,
                null,
                MessengerContract.MessagesEntry.COLUMN_NAME_TIMESTAMP + " ASC"
        );

        messageList.clear();

        while (cursor.moveToNext()) {
            @SuppressLint("Range") String sender = cursor.getString(cursor.getColumnIndex(MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_ID));
            @SuppressLint("Range") String messageText = cursor.getString(cursor.getColumnIndex(MessengerContract.MessagesEntry.COLUMN_NAME_MESSAGE));
            @SuppressLint("Range") long timestamp = cursor.getLong(cursor.getColumnIndex(MessengerContract.MessagesEntry.COLUMN_NAME_TIMESTAMP));
            boolean isSentByCurrentUser = sender.equals("Me");

            Message message = new Message(sender, messageText, timestamp, isSentByCurrentUser);
            messageList.add(message);
        }
        cursor.close();
        db.close();

        messageAdapter.notifyDataSetChanged();
    }


    private void sendMessage() {
        String messageText = messageInput.getText().toString().trim();
        if (!messageText.isEmpty()) {
            long timestamp = System.currentTimeMillis();
            String currentUser = "Me";

            messageList.add(new Message(currentUser, messageText, timestamp, true));
            messageAdapter.notifyItemInserted(messageList.size() - 1);
            recyclerView.scrollToPosition(messageList.size() - 1);
            messageInput.setText("");

            MessengerDbHelper dbHelper = new MessengerDbHelper(this);
            SQLiteDatabase db = dbHelper.getWritableDatabase();

            ContentValues values = new ContentValues();
            values.put(MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_ID, currentUser);
            values.put(MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_ID, contactName);
            values.put(MessengerContract.MessagesEntry.COLUMN_NAME_MESSAGE, messageText);
            values.put(MessengerContract.MessagesEntry.COLUMN_NAME_TIMESTAMP, timestamp);

            long newRowId = db.insert(MessengerContract.MessagesEntry.TABLE_NAME, null, values);
            if (newRowId == -1) {
                Log.d("Database", "Error saving message to database.");
            } else {
                Log.d("Database", "Message saved to database with ID: " + newRowId);
            }
            db.close();

            SharedPreferences sharedPreferences = getSharedPreferences("UserDetails", MODE_PRIVATE);
            String IP = sharedPreferences.getString("IP", null);
            String Phone = sharedPreferences.getString("Phone", null);
            Operations client = Operations.getInstance(this, IP);

            new Thread(() -> {
                try {
                    client.sendMessage("123456", Phone, messageText);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }).start();
        }
    }

}