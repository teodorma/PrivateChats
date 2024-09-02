package com.example.privatechats.ui.home;

import android.annotation.SuppressLint;
import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.example.privatechats.database.MessengerContract;
import com.example.privatechats.database.MessengerDbHelper;
import com.example.privatechats.databinding.FragmentHomeBinding;
import com.example.privatechats.ui.home.chat.ChatDetailActivity;
import com.example.privatechats.ui.home.chat.Chat;
import com.example.privatechats.ui.home.chat.ChatAdapter;

import java.util.ArrayList;
import java.util.List;

public class HomeFragment extends Fragment {

    private FragmentHomeBinding binding;
    private ChatAdapter chatAdapter;
    private final List<Chat> chatList = new ArrayList<>();

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        HomeViewModel homeViewModel =
                new ViewModelProvider(this).get(HomeViewModel.class);

        binding = FragmentHomeBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        final RecyclerView recyclerView = binding.recyclerView;
        recyclerView.setLayoutManager(new LinearLayoutManager(getContext()));
        chatAdapter = new ChatAdapter(chatList, this::openChatDetail);
        recyclerView.setAdapter(chatAdapter);

        loadChats();
        return root;
    }

    @Override
    public void onResume() {
        super.onResume();
        loadChats();
    }

    @SuppressLint("NotifyDataSetChanged")
    private void loadChats() {
        chatList.clear();

        MessengerDbHelper dbHelper = new MessengerDbHelper(getContext());
        SQLiteDatabase db = dbHelper.getReadableDatabase();

        String query = "SELECT " +
                "MAX(" + MessengerContract.MessagesEntry.COLUMN_NAME_TIMESTAMP + ") AS last_timestamp, " +
                MessengerContract.MessagesEntry.COLUMN_NAME_MESSAGE + ", " +
                MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_PHONE + ", " +
                MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_PHONE +
                " FROM " + MessengerContract.MessagesEntry.TABLE_NAME +
                " GROUP BY " +
                " CASE WHEN " + MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_PHONE + " = 'Me' THEN " +
                MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_PHONE +
                " ELSE " + MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_PHONE + " END" +
                " ORDER BY last_timestamp DESC";

        Cursor cursor = db.rawQuery(query, null);
        while (cursor.moveToNext()) {
            @SuppressLint("Range") String senderPhone = cursor.getString(cursor.getColumnIndex(MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_PHONE));
            @SuppressLint("Range") String receiverPhone = cursor.getString(cursor.getColumnIndex(MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_PHONE));
            @SuppressLint("Range") String messageText = cursor.getString(cursor.getColumnIndex(MessengerContract.MessagesEntry.COLUMN_NAME_MESSAGE));

            // Determine the contact's phone number
            String contactPhone = senderPhone.equals("Me") ? receiverPhone : senderPhone;

            // Retrieve the contact name if possible
            String contactName = getContactNameFromPhoneNumber(contactPhone);

            chatList.add(new Chat(contactName, contactPhone, messageText)); // Add contact phone number to Chat
        }
        cursor.close();
        db.close();

        chatAdapter.notifyDataSetChanged();
    }

    @SuppressLint("Range")
    private String getContactNameFromPhoneNumber(String phoneNumber) {
        String contactName = phoneNumber; // Default to phone number if no name found
        ContentResolver contentResolver = getContext().getContentResolver();
        Uri uri = Uri.withAppendedPath(ContactsContract.PhoneLookup.CONTENT_FILTER_URI, Uri.encode(phoneNumber));
        Cursor cursor = contentResolver.query(uri, new String[]{ContactsContract.PhoneLookup.DISPLAY_NAME}, null, null, null);

        if (cursor != null) {
            if (cursor.moveToFirst()) {
                contactName = cursor.getString(cursor.getColumnIndex(ContactsContract.PhoneLookup.DISPLAY_NAME));
            }
            cursor.close();
        }
        return contactName;
    }


    private void openChatDetail(Chat chat) {
        Intent intent = new Intent(getActivity(), ChatDetailActivity.class);
        intent.putExtra("contactName", chat.getContactName());
        intent.putExtra("contactPhoneNumber", chat.getContactPhone()); // Pass the phone number as well
        startActivity(intent);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }
}
