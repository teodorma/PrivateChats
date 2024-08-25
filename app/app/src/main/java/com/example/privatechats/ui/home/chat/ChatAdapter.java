package com.example.privatechats.ui.home.chat;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.example.privatechats.R;
import com.example.privatechats.ui.home.chat.Chat;

import java.util.List;

public class ChatAdapter extends RecyclerView.Adapter<ChatAdapter.ChatViewHolder> {

    private final List<Chat> chatList;
    private final OnChatClickListener onChatClickListener;

    public ChatAdapter(List<Chat> chatList, OnChatClickListener onChatClickListener) {
        this.chatList = chatList;
        this.onChatClickListener = onChatClickListener;
    }

    @NonNull
    @Override
    public ChatViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.chat_item, parent, false);
        return new ChatViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ChatViewHolder holder, int position) {
        Chat chat = chatList.get(position);
        holder.contactName.setText(chat.getContactName());
        holder.lastMessage.setText(chat.getLastMessage());

        holder.itemView.setOnClickListener(v -> onChatClickListener.onChatClick(chat));
    }

    @Override
    public int getItemCount() {
        return chatList.size();
    }

    public interface OnChatClickListener {
        void onChatClick(Chat chat);
    }

    static class ChatViewHolder extends RecyclerView.ViewHolder {
        TextView contactName;
        TextView lastMessage;

        ChatViewHolder(View itemView) {
            super(itemView);
            contactName = itemView.findViewById(R.id.contactName);
            lastMessage = itemView.findViewById(R.id.lastMessage);
        }
    }
}
