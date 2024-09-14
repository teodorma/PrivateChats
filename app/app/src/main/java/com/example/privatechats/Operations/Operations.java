package com.example.privatechats;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

import com.example.privatechats.database.MessengerContract;
import com.example.privatechats.database.MessengerDbHelper;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.math.BigInteger;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.RSAPrivateKeySpec;
import java.security.spec.RSAPublicKeySpec;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.zip.DataFormatException;

public class Operations {
    private final String SERVER_ADDRESS;
    private static final int SERVER_PORT = 8080;
    private static Operations instance;
    private Socket socket;
    private OutputStreamWriter out;
    private BufferedReader in;

    private Context context;
    private final SharedPreferences sharedPreferences;

    private static final BlockingQueue<JSONObject> requestQueue = new LinkedBlockingQueue<>();
    private static final Map<String, BlockingQueue<JSONObject>> messageQueues = new HashMap<>();

    private BigInteger myPublicKeyModulus;
    private BigInteger myPublicKeyExponent;
    private BigInteger myPrivateKeyModulus;
    private BigInteger myPrivateKeyExponent;

    private BigInteger serverPublicKeyModulus;
    private static final BigInteger serverPublicExponent = BigInteger.valueOf(65537);
    private final Map<String, BigInteger> PublicKeys = new HashMap<>();

    public Operations(Context context, String ip) {
        this.context = context;
        this.SERVER_ADDRESS = ip;
        sharedPreferences = context.getSharedPreferences("UserDetails", Context.MODE_PRIVATE);

        String storedKey = sharedPreferences.getString("ServerKey", null);
        if (storedKey != null) {
            serverPublicKeyModulus = new BigInteger(storedKey, 16);
            Log.d("Operations", "Loaded server public key modulus from SharedPreferences: " + serverPublicKeyModulus.toString(16));
        } else {
            Log.d("Operations", "Server public key modulus not found in SharedPreferences.");
        }
    }

    public static synchronized Operations getInstance(Context context, String ip) {
        if (instance == null) {
            instance = new Operations(context, ip);
        }
        return instance;
    }

    public void registerClient(String phoneNumber) throws IOException, JSONException, GeneralSecurityException, DataFormatException, InterruptedException {
        generateRSAKeyPair();

        JSONObject registerJson = new JSONObject();
        registerJson.put("TYPE", "REGISTER");
        registerJson.put("PHONE", phoneNumber);
        registerJson.put("KEY", myPublicKeyModulus.toString(16));

        sendRequest(registerJson, false);
        JSONObject response = getResponse();

        if (response.has("KEY")) {
            serverPublicKeyModulus = new BigInteger(response.getString("KEY"), 16);

            SharedPreferences.Editor editor = sharedPreferences.edit();
            editor.putString("ServerKey", serverPublicKeyModulus.toString(16));
            editor.apply();

            Log.d("Operations", "Server public key modulus received and stored: " + serverPublicKeyModulus.toString(16));
        } else {
            Log.e("Operations", "Server public key modulus not found in response.");
            throw new RuntimeException("Server public key modulus not found in response.");
        }
    }


    public void startListeningForMessages() {
        Thread listeningThread = new Thread(this::listener);
        listeningThread.start();
    }

    private void listener() {
        try {
            if (in == null) {
                Log.e("Operations", "BufferedReader is not initialized. Cannot listen for messages.");
                connectToServer();
                return;
            }
            String response;
            StringBuilder stringBuilder = new StringBuilder();
            int expectedChunks = -1;
            int receivedChunks = 0;

            while ((response = in.readLine()) != null) {
                if (response.trim().isEmpty()) {
                    continue;
                }
                try {
                    Log.d("Operations", "Received message encrypted chunk: " + response);
                    response = decrypt(response);
                    Log.d("Operations", "Received message decrypted chunk: " + response);

                    JSONObject responseJson = new JSONObject(response);
                    if (responseJson.has("MESSAGE")) {
                        addToMessageQueue(responseJson, (String)responseJson.get("PHONE"));
                        if(responseJson.has("END")){
                            processMessageChunks((String)responseJson.get("PHONE"), "jonny");
                        }
                    }
                    if (responseJson.has("CHUNK")) {
                        String messageChunk = responseJson.getString("CHUNK");
                        int chunkNumber = responseJson.getInt("CHUNK_NUMBER");
                        int numberOfChunks = responseJson.getInt("NUMBER_OF_CHUNKS");

                        stringBuilder.append(messageChunk);
                        receivedChunks++;
                        expectedChunks = numberOfChunks;

                        if (receivedChunks == expectedChunks) {
                            JSONObject d = new JSONObject(stringBuilder.toString());
                            Log.d("Operations", "Assembled message: " + d);

                            if (d.has("RESPONSE") || d.has("KEY")) {
                                addToResponseQueue(d);
                                stringBuilder.setLength(0);
                                receivedChunks = 0;
                            }
                        }
                    }
                } catch (JSONException e) {
                    Log.e("Operations", "Failed to parse JSON response: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            Log.e("Operations", "Error listening for messages", e);
        }
    }

    private void sendRequest(JSONObject requestJson, boolean encryptWithServerKey) throws IOException, GeneralSecurityException {
        String requestString = requestJson.toString();

        if (encryptWithServerKey) {
            if (serverPublicKeyModulus == null) {
                String storedKey = sharedPreferences.getString("ServerKey", null);
                if (storedKey != null) {
                    serverPublicKeyModulus = new BigInteger(storedKey, 10);
                    Log.d("Operations", "Retrieved server public key modulus from SharedPreferences: " + serverPublicKeyModulus.toString(10));
                } else {
                    Log.e("Operations", "Server public key modulus is not stored in SharedPreferences.");
                    throw new RuntimeException("Server public key modulus is null and not found in SharedPreferences. Cannot encrypt the request.");
                }
            }

            Log.d("Operations", "Encrypting request with server public key modulus: " + serverPublicKeyModulus.toString(10));
            requestString = encrypt(requestString, serverPublicKeyModulus);
        }

        out.write(requestString + "\n");
        out.flush();
    }


    private JSONObject getResponse() throws IOException, InterruptedException {
        return requestQueue.take();
    }

    private void addToResponseQueue(JSONObject response) {
        requestQueue.add(response);
    }

    private void addToMessageQueue(JSONObject message, String senderPhone) {
        messageQueues.computeIfAbsent(senderPhone, k -> new LinkedBlockingQueue<>()).add(message);
    }

    private void generateRSAKeyPair() throws NoSuchAlgorithmException, InvalidKeySpecException {
        KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("RSA");
        keyPairGenerator.initialize(2048);
        KeyPair keyPair = keyPairGenerator.generateKeyPair();

        RSAPublicKeySpec pubKeySpec = KeyFactory.getInstance("RSA").getKeySpec(keyPair.getPublic(), RSAPublicKeySpec.class);
        myPublicKeyModulus = pubKeySpec.getModulus();
        myPublicKeyExponent = pubKeySpec.getPublicExponent();

        RSAPrivateKeySpec privKeySpec = KeyFactory.getInstance("RSA").getKeySpec(keyPair.getPrivate(), RSAPrivateKeySpec.class);
        myPrivateKeyModulus = privKeySpec.getModulus();
        myPrivateKeyExponent = privKeySpec.getPrivateExponent();
    }

    private String encrypt(String message, BigInteger publicKeyModulus) {
        try {
            byte[] messageBytes = message.getBytes(StandardCharsets.UTF_8);
            BigInteger messageAsNumber = new BigInteger(1, messageBytes);
            BigInteger ciphertext = messageAsNumber.modPow(serverPublicExponent, publicKeyModulus);
            return ciphertext.toString();
        } catch (Exception e) {
            throw new RuntimeException("Encryption error", e);
        }
    }

    private String decrypt(String encryptedMessage) {
        try {
            BigInteger ciphertext = new BigInteger(encryptedMessage);
            BigInteger plaintext = ciphertext.modPow(myPrivateKeyExponent, myPrivateKeyModulus);
            byte[] plaintextBytes = plaintext.toByteArray();
            return new String(plaintextBytes, StandardCharsets.UTF_8).trim();
        } catch (Exception e) {
            throw new RuntimeException("Decryption error", e);
        }
    }

    public void sendMessage(String recipientPhone, String senderPhone, String message) throws IOException, GeneralSecurityException, JSONException, InterruptedException, DataFormatException {
        String recipientPublicKeyModulus = getPublicKeyByPhone(recipientPhone);

        if (recipientPublicKeyModulus == null) {
            requestPublicKey(recipientPhone, senderPhone);
        }
        recipientPublicKeyModulus = getPublicKeyByPhone(recipientPhone);
        BigInteger userKey = new BigInteger(recipientPublicKeyModulus, 16);
        Log.d("Operations", "Encrypting message with recipient's " + senderPhone + " recipient public key: " + userKey.toString(16));
        String encryptedMessage = encrypt(message, userKey);
        sendEncryptedMessageInChunks(senderPhone, recipientPhone, encryptedMessage);
    }

    private void sendEncryptedMessageInChunks(String senderPhone, String recipientPhone, String encryptedMessage) throws IOException, JSONException, GeneralSecurityException, DataFormatException, InterruptedException {
        int chunkSize = 160;
        int messageLength = encryptedMessage.length();

        for (int i = 0; i < messageLength; i += chunkSize) {
            JSONObject chunkJson = new JSONObject();
            chunkJson.put("TYPE", "MESSAGE");
            chunkJson.put("PHONE", senderPhone);
            chunkJson.put("RECIPIENT_PHONE", recipientPhone);

            int end = Math.min(i + chunkSize, messageLength);
            String messageChunk = encryptedMessage.substring(i, end);

            if (end == messageLength) {
                chunkJson.put("END","TRUE");
            }
            chunkJson.put("MESSAGE", messageChunk);

            sendRequest(chunkJson, true);
            getResponse();
        }
    }


    public String getPublicKeyByPhone(String phoneNumber) {
        MessengerDbHelper dbHelper = new MessengerDbHelper(context);
        SQLiteDatabase db = dbHelper.getReadableDatabase();

        String[] projection = {
                MessengerContract.KeyEntry.COLUMN_NAME_CONTACT_KEY
        };

        String selection = MessengerContract.KeyEntry.COLUMN_NAME_PHONE + " = ?";
        String[] selectionArgs = { phoneNumber };

        Cursor cursor = null;
        String publicKey = null;

        try {
            cursor = db.query(
                    MessengerContract.KeyEntry.TABLE_NAME,
                    projection,
                    selection,
                    selectionArgs,
                    null,
                    null,
                    null
            );

            if (cursor != null && cursor.moveToFirst()) {
                publicKey = cursor.getString(
                        cursor.getColumnIndexOrThrow(MessengerContract.KeyEntry.COLUMN_NAME_CONTACT_KEY)
                );
            }
        } catch (Exception e) {
            Log.e("getPublicKeyByPhone", "Error retrieving public key for phone number: " + phoneNumber, e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
            db.close();
        }

        return publicKey;
    }



    public void requestPublicKey(String phoneNumber, String phone) throws IOException, GeneralSecurityException, JSONException, InterruptedException {
        JSONObject requestJson = new JSONObject();
        requestJson.put("TYPE", "GET_USER_KEY");
        requestJson.put("PHONE", phone);
        requestJson.put("RECIPIENT_PHONE", phoneNumber);

        sendRequest(requestJson, true);

        JSONObject response = getResponse();
        if (response.has("KEY")) {
            BigInteger publicKeyModulus = new BigInteger(response.getString("KEY"), 16);
            savePublicKeyToDatabase(phoneNumber, publicKeyModulus.toString(16));
        }
    }


    private void savePublicKeyToDatabase(String phoneNumber, String publicKey) {
        MessengerDbHelper dbHelper = new MessengerDbHelper(context);
        SQLiteDatabase db = dbHelper.getWritableDatabase();

        ContentValues values = new ContentValues();
        values.put(MessengerContract.KeyEntry.COLUMN_NAME_PHONE, phoneNumber);
        values.put(MessengerContract.KeyEntry.COLUMN_NAME_CONTACT_KEY, publicKey);

        try {
            long newRowId = db.insertWithOnConflict(
                    MessengerContract.KeyEntry.TABLE_NAME,
                    null,
                    values,
                    SQLiteDatabase.CONFLICT_REPLACE
            );

            if (newRowId == -1) {
                Log.d("Database", "Error saving public key to database for phone: " + phoneNumber);
            } else {
                Log.d("Database", "Public key saved to database with ID: " + newRowId + " for phone: " + phoneNumber);
            }
        } catch (Exception e) {
            Log.e("savePublicKeyToDatabase", "Exception occurred while saving public key for phone: " + phoneNumber, e);
        } finally {
            db.close();
        }
    }



    public void processMessageChunks(String senderPhone, String receiverPhone) throws JSONException {
        BlockingQueue<JSONObject> queue = messageQueues.get(senderPhone);
        if (queue == null) {
            Log.e("Operations", "No message queue found for sender: " + senderPhone);
            return;
        }

        StringBuilder fullMessage = new StringBuilder();
        JSONObject messageChunk;
        while ((messageChunk = queue.poll()) != null) {
            if (messageChunk.has("MESSAGE")) {
                fullMessage.append(messageChunk.getString("MESSAGE"));
            }
        }

        String decryptedMessage = decrypt(String.valueOf(fullMessage));
        Log.d("Operations", "Processed complete message: " + decryptedMessage + " from " + senderPhone);

        long timestamp = System.currentTimeMillis();

        MessengerDbHelper dbHelper = new MessengerDbHelper(context);
        SQLiteDatabase db = dbHelper.getWritableDatabase();

        ContentValues values = new ContentValues();
        values.put(MessengerContract.MessagesEntry.COLUMN_NAME_SENDER_PHONE, senderPhone);
        values.put(MessengerContract.MessagesEntry.COLUMN_NAME_RECEIVER_PHONE, receiverPhone);
        values.put(MessengerContract.MessagesEntry.COLUMN_NAME_MESSAGE, decryptedMessage);
        values.put(MessengerContract.MessagesEntry.COLUMN_NAME_TIMESTAMP, timestamp);

        long newRowId = db.insert(MessengerContract.MessagesEntry.TABLE_NAME, null, values);
        if (newRowId == -1) {
            Log.d("Operations", "Error saving message to database.");
        } else {
            Log.d("Operations", "Message saved to database with ID: " + newRowId);

            Intent intent = new Intent("com.example.privatechats.NEW_MESSAGE");
            context.sendBroadcast(intent);
        }

        db.close();
    }



    public boolean connectToServer() {
        try {
            socket = new Socket(SERVER_ADDRESS, SERVER_PORT);
            out = new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.UTF_8);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8));
            Log.d("Operations", "Successfully connected to server.");
            return true;
        } catch (IOException e) {
            Log.e("Operations", "Failed to connect to server.", e);
            disconnect();
            return false;
        }
    }

    public void disconnect() {
        try {
            if (socket != null && !socket.isClosed()) {
                socket.close();
                Log.d("Operations", "Disconnected from server.");
            }
        } catch (IOException e) {
            Log.e("Operations", "Error while disconnecting.", e);
        }
    }
}
