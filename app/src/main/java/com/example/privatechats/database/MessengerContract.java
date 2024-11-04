package com.example.privatechats.database;

import android.provider.BaseColumns;

public final class MessengerContract {
    private MessengerContract() {}

    public static class ContactsEntry implements BaseColumns {
        public static final String TABLE_NAME = "contacts";
        public static final String COLUMN_NAME_PHONE = "phone_number";
        public static final String COLUMN_NAME_NAME = "name";
    }

    public static class MessagesEntry implements BaseColumns {
        public static final String TABLE_NAME = "messages";
        public static final String COLUMN_NAME_SENDER_PHONE = "sender_phone";
        public static final String COLUMN_NAME_RECEIVER_PHONE = "receiver_phone";
        public static final String COLUMN_NAME_MESSAGE = "message";
        public static final String COLUMN_NAME_TIMESTAMP = "timestamp";
    }

    public static class KeyEntry implements BaseColumns {
        public static final String TABLE_NAME = "keys";
        public static final String COLUMN_NAME_PHONE = "phone_number"; // Primary key
        public static final String COLUMN_NAME_CONTACT_KEY = "contact_key";
    }
}
