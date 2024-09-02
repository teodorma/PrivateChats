package com.example.privatechats.ui.home.contacts;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.ContentValues;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.example.privatechats.R;
import com.example.privatechats.database.MessengerContract;
import com.example.privatechats.database.MessengerDbHelper;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class ContactsActivity extends AppCompatActivity {

    private static final int PERMISSIONS_REQUEST_READ_CONTACTS = 100;
    private RecyclerView recyclerView;
    private ContactAdapter contactAdapter;
    private List<Contact> contactList = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_contacts);

        recyclerView = findViewById(R.id.recyclerViewContacts);
        recyclerView.setLayoutManager(new LinearLayoutManager(this));

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_CONTACTS)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.READ_CONTACTS},
                    PERMISSIONS_REQUEST_READ_CONTACTS);
        } else {
            loadContacts();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSIONS_REQUEST_READ_CONTACTS) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                loadContacts();
            } else {
                Toast.makeText(this, "Permission denied to read contacts", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void loadContacts() {
        contactList.clear();
        Cursor cursor = getContentResolver().query(
                ContactsContract.CommonDataKinds.Phone.CONTENT_URI,
                null,
                null,
                null,
                null
        );

        Set<String> phoneNumberSet = new HashSet<>();
        MessengerDbHelper dbHelper = new MessengerDbHelper(this);
        SQLiteDatabase db = dbHelper.getWritableDatabase();

        if (cursor != null) {
            while (cursor.moveToNext()) {
                @SuppressLint("Range") String name = cursor.getString(
                        cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME)
                );
                @SuppressLint("Range") String phoneNumber = cursor.getString(
                        cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER)
                );

                String cleanedPhoneNumber = phoneNumber.replaceAll("\\s", ""); // Remove all spaces

                if (cleanedPhoneNumber.startsWith("+4")) {
                    cleanedPhoneNumber = cleanedPhoneNumber.substring(2);
                }

                if (phoneNumberSet.add(cleanedPhoneNumber)) {
                    contactList.add(new Contact(name, cleanedPhoneNumber));

                    ContentValues values = new ContentValues();
                    values.put(MessengerContract.ContactsEntry.COLUMN_NAME_NAME, name);
                    values.put(MessengerContract.ContactsEntry.COLUMN_NAME_PHONE, cleanedPhoneNumber);

                    long newRowId = db.insert(MessengerContract.ContactsEntry.TABLE_NAME, null, values);
                    if (newRowId == -1) {
                        Log.d("Database", "Error saving contact to database.");
                    } else {
                        Log.d("Database", "Contact saved to database with ID: " + newRowId);
                    }
                }
            }
            cursor.close();
        }

        db.close();

        contactList.sort(new Comparator<Contact>() {
            @Override
            public int compare(Contact c1, Contact c2) {
                return c1.getName().compareToIgnoreCase(c2.getName());
            }
        });

        contactAdapter = new ContactAdapter(this, contactList);
        recyclerView.setAdapter(contactAdapter);
    }

}
