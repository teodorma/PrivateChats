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
    private String contactPhoneNumber;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_chat_detail);

        recyclerView = findViewById(R.id.recyclerViewMessages);
        messageInput = findViewById(R.id.messageInput);
        sendButton = findViewById(R.id.sendButton);

        Intent intent = getIntent();
        contactName = intent.getStringExtra("contactName");
        contactPhoneNumber = intent.getStringExtra("contactPhoneNumber");

        TextView contactNameView = findViewById(R.id.contactName);
        contactNameView.setText(contactName);

        recyclerView.setLayoutManager(new LinearLayoutManager(this));
        messageAdapter = new MessageAdapter(messageList);
        recyclerView.setAdapter(messageAdapter);

        loadMessages();

        sendButton.setOnClickListener(v -> sendMessage());
    }

    private void loadMessages() {
        MessengerDbHelper dbHelper = new MessengerDbHelper(this);
        SQLiteDatabase db = dbHelper.getReadableDatabase();

        String[] projection = {
                MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_PHONE,
                MessengerContract.MessagesEntry.COLUMN_NAME_MESSAGE,
                MessengerContract.MessagesEntry.COLUMN_NAME_TIMESTAMP
        };

        // Log to debug values
        Log.d("loadMessages", "contactPhoneNumber: " + contactPhoneNumber);

        if (contactPhoneNumber == null || contactPhoneNumber.isEmpty()) {
            Log.e("loadMessages", "Contact phone number is null or empty, cannot load messages.");
            return;
        }

        String selection = MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_PHONE + " = ? OR " +
                MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_PHONE + " = ?";
        String[] selectionArgs = {contactPhoneNumber, contactPhoneNumber};

        Cursor cursor = null;
        try {
            cursor = db.query(
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
                @SuppressLint("Range") String sender = cursor.getString(cursor.getColumnIndex(MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_PHONE));
                @SuppressLint("Range") String messageText = cursor.getString(cursor.getColumnIndex(MessengerContract.MessagesEntry.COLUMN_NAME_MESSAGE));
                @SuppressLint("Range") long timestamp = cursor.getLong(cursor.getColumnIndex(MessengerContract.MessagesEntry.COLUMN_NAME_TIMESTAMP));

                Log.d("loadMessages", "Loaded message: " + messageText + " from " + sender);

                boolean isSentByCurrentUser = sender.equals("Me");

                Message message = new Message(sender, messageText, timestamp, isSentByCurrentUser);
                messageList.add(message);
            }
        } catch (Exception e) {
            Log.e("loadMessages", "Error loading messages", e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
            db.close();
        }

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
            values.put(MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_PHONE, currentUser);
            values.put(MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_PHONE, contactPhoneNumber);
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
                    client.sendMessage(contactPhoneNumber, Phone, messageText);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }).start();
        }
    }
}
