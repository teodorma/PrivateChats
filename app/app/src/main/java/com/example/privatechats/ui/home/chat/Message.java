package com.example.privatechats.ui.home.chat;

public class Message {
    private final String sender;
    private final String text;
    private final long timestamp;
    private final boolean isSentByCurrentUser;

    public Message(String sender, String text, long timestamp, boolean isSentByCurrentUser) {
        this.sender = sender;
        this.text = text;
        this.timestamp = timestamp;
        this.isSentByCurrentUser = isSentByCurrentUser;
    }

    public String getSender() {
        return sender;
    }

    public String getText() {
        return text;
    }

    public long getTimestamp() {
        return timestamp;
    }

    public boolean isSentByCurrentUser() {
        return isSentByCurrentUser;
    }
}
