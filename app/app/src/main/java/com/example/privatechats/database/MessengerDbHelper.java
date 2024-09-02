package com.example.privatechats.database;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class MessengerDbHelper extends SQLiteOpenHelper {
    private static final int DATABASE_VERSION = 1;
    private static final String DATABASE_NAME = "MessengerApp.db";

    private static final String SQL_CREATE_CONTACTS_TABLE =
            "CREATE TABLE " + MessengerContract.ContactsEntry.TABLE_NAME + " (" +
                    MessengerContract.ContactsEntry.COLUMN_NAME_PHONE + " TEXT PRIMARY KEY," +
                    MessengerContract.ContactsEntry.COLUMN_NAME_NAME + " TEXT NOT NULL)";

    private static final String SQL_CREATE_MESSAGES_TABLE =
            "CREATE TABLE " + MessengerContract.MessagesEntry.TABLE_NAME + " (" +
                    MessengerContract.MessagesEntry._ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                    MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_PHONE + " TEXT NOT NULL," +
                    MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_PHONE + " TEXT NOT NULL," +
                    MessengerContract.MessagesEntry.COLUMN_NAME_MESSAGE + " TEXT NOT NULL," +
                    MessengerContract.MessagesEntry.COLUMN_NAME_TIMESTAMP + " DATETIME DEFAULT CURRENT_TIMESTAMP," +
                    "FOREIGN KEY(" + MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_PHONE + ") REFERENCES " +
                    MessengerContract.ContactsEntry.TABLE_NAME + "(" + MessengerContract.ContactsEntry.COLUMN_NAME_PHONE + ")," +
                    "FOREIGN KEY(" + MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_PHONE + ") REFERENCES " +
                    MessengerContract.ContactsEntry.TABLE_NAME + "(" + MessengerContract.ContactsEntry.COLUMN_NAME_PHONE + "))";

    private static final String SQL_CREATE_KEYS_TABLE =
            "CREATE TABLE " + MessengerContract.KeyEntry.TABLE_NAME + " (" +
                    MessengerContract.KeyEntry.COLUMN_NAME_PHONE + " TEXT PRIMARY KEY," +
                    MessengerContract.KeyEntry.COLUMN_NAME_CONTACT_KEY + " TEXT)";

    private static final String SQL_DELETE_CONTACTS_TABLE =
            "DROP TABLE IF EXISTS " + MessengerContract.ContactsEntry.TABLE_NAME;

    private static final String SQL_DELETE_MESSAGES_TABLE =
            "DROP TABLE IF EXISTS " + MessengerContract.MessagesEntry.TABLE_NAME;

    private static final String SQL_DELETE_KEYS_TABLE =
            "DROP TABLE IF EXISTS " + MessengerContract.KeyEntry.TABLE_NAME;

    public MessengerDbHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL(SQL_CREATE_CONTACTS_TABLE);
        db.execSQL(SQL_CREATE_MESSAGES_TABLE);
        db.execSQL(SQL_CREATE_KEYS_TABLE);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        db.execSQL(SQL_DELETE_CONTACTS_TABLE);
        db.execSQL(SQL_DELETE_MESSAGES_TABLE);
        db.execSQL(SQL_DELETE_KEYS_TABLE);
        onCreate(db);
    }
}
