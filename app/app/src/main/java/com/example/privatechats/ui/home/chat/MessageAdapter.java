package com.example.privatechats.ui.home.chat;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.example.privatechats.R;

import java.util.List;

public class MessageAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {

    private final List<Message> messageList;

    private static final int VIEW_TYPE_LEFT = 0;
    private static final int VIEW_TYPE_RIGHT = 1;

    public MessageAdapter(List<Message> messageList) {
        this.messageList = messageList;
    }

    @Override
    public int getItemViewType(int position) {
        Message message = messageList.get(position);
        return message.isSentByCurrentUser() ? VIEW_TYPE_RIGHT : VIEW_TYPE_LEFT;
    }

    @NonNull
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        if (viewType == VIEW_TYPE_LEFT) {
            View view = LayoutInflater.from(parent.getContext())
                    .inflate(R.layout.item_message_left, parent, false);
            return new LeftMessageViewHolder(view);
        } else {
            View view = LayoutInflater.from(parent.getContext())
                    .inflate(R.layout.item_message_right, parent, false);
            return new RightMessageViewHolder(view);
        }
    }

    @Override
    public void onBindViewHolder(@NonNull RecyclerView.ViewHolder holder, int position) {
        Message message = messageList.get(position);

        if (holder.getItemViewType() == VIEW_TYPE_LEFT) {
            LeftMessageViewHolder leftHolder = (LeftMessageViewHolder) holder;
            leftHolder.messageTextLeft.setText(message.getText());
        } else {
            RightMessageViewHolder rightHolder = (RightMessageViewHolder) holder;
            rightHolder.messageTextRight.setText(message.getText());
        }
    }

    @Override
    public int getItemCount() {
        return messageList.size();
    }

    static class LeftMessageViewHolder extends RecyclerView.ViewHolder {
        TextView messageTextLeft;

        LeftMessageViewHolder(View itemView) {
            super(itemView);
            messageTextLeft = itemView.findViewById(R.id.messageTextLeft);
        }
    }

    static class RightMessageViewHolder extends RecyclerView.ViewHolder {
        TextView messageTextRight;

        RightMessageViewHolder(View itemView) {
            super(itemView);
            messageTextRight = itemView.findViewById(R.id.messageTextRight);
        }
    }
}
