package com.example.privatechats.ui.home.chat;

public class Chat {
    private String contactName;
    private String lastMessage;

    public Chat(String contactName, String lastMessage) {
        this.contactName = contactName;
        this.lastMessage = lastMessage;
    }

    public String getContactName() {
        return contactName;
    }
    public String getLastMessage() {
        return lastMessage;
    }

    public void setContactName(String contactName) {
        this.contactName = contactName;
    }

    public void setLastMessage(String lastMessage) {
        this.lastMessage = lastMessage;
    }
}
