package com.example.privatechats.ui.home.chat;

public class Chat {
    private String contactName;
    private String contactPhone;  // Add a field for the phone number
    private String lastMessage;

    public Chat(String contactName, String contactPhone, String lastMessage) {
        this.contactName = contactName;
        this.contactPhone = contactPhone;
        this.lastMessage = lastMessage;
    }

    public String getContactName() {
        return contactName;
    }

    public String getContactPhone() {
        return contactPhone; // Add getter for contact phone
    }

    public String getLastMessage() {
        return lastMessage;
    }

    public void setContactName(String contactName) {
        this.contactName = contactName;
    }

    public void setContactPhone(String contactPhone) {
        this.contactPhone = contactPhone;
    }

    public void setLastMessage(String lastMessage) {
        this.lastMessage = lastMessage;
    }
}
