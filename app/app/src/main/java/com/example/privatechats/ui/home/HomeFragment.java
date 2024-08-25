package com.example.privatechats.ui.home;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
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
import com.example.privatechats.ui.home.chat.ChatDetailActivity; // Import your chat detail activity or fragment
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
        chatList.clear(); // Clear the list to avoid duplication

        MessengerDbHelper dbHelper = new MessengerDbHelper(getContext());
        SQLiteDatabase db = dbHelper.getReadableDatabase();


        String query = "SELECT " +
                "MAX(" + MessengerContract.MessagesEntry.COLUMN_NAME_TIMESTAMP + ") AS last_timestamp, " +
                MessengerContract.MessagesEntry.COLUMN_NAME_MESSAGE + ", " +
                MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_ID + ", " +
                MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_ID +
                " FROM " + MessengerContract.MessagesEntry.TABLE_NAME +
                " GROUP BY " +
                " CASE WHEN " + MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_ID + " = 'Me' THEN " +
                MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_ID +
                " ELSE " + MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_ID + " END" +
                " ORDER BY last_timestamp DESC";

        Cursor cursor = db.rawQuery(query, null);
        while (cursor.moveToNext()) {
            @SuppressLint("Range") String sender = cursor.getString(cursor.getColumnIndex(MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_ID));
            @SuppressLint("Range") String receiver = cursor.getString(cursor.getColumnIndex(MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_ID));
            @SuppressLint("Range") String messageText = cursor.getString(cursor.getColumnIndex(MessengerContract.MessagesEntry.COLUMN_NAME_MESSAGE));

            // Determine who the other contact is
            String contactName = sender.equals("Me") ? receiver : sender;

            chatList.add(new Chat(contactName, messageText));
        }
        cursor.close();
        db.close();

        chatAdapter.notifyDataSetChanged();
    }


    private void openChatDetail(Chat chat) {
        Intent intent = new Intent(getActivity(), ChatDetailActivity.class);
        intent.putExtra("contactName", chat.getContactName());
        startActivity(intent);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }
}
