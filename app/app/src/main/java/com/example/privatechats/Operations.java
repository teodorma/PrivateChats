package com.example.privatechats;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

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
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.zip.DataFormatException;

public class Operations {
    private String SERVER_ADDRESS;
    private static final int SERVER_PORT = 8080;
    private static Operations instance;
    private Socket socket;
    private OutputStreamWriter out;
    private BufferedReader in;

    private Context context;
    private SharedPreferences sharedPreferences;

    private static final BlockingQueue<JSONObject> requestQueue = new LinkedBlockingQueue<>();

    private BigInteger myPublicKeyModulus;
    private BigInteger myPublicKeyExponent;
    private BigInteger myPrivateKeyModulus;
    private BigInteger myPrivateKeyExponent;

    private Thread listeningThread;

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
        listeningThread = new Thread(this::listenForMessages);
        listeningThread.start();
    }

    private void listenForMessages() {
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

                            if (d.has("MESSAGE")) {
                                // Handle the assembled message
                            } else if (d.has("RESPONSE") || d.has("KEY")) {
                                addToQueue(d);
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

    private void addToQueue(JSONObject response) {
        requestQueue.add(response);
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
        BigInteger recipientPublicKeyModulus = PublicKeys.get(recipientPhone);

        if (recipientPublicKeyModulus == null) {
            requestPublicKey(recipientPhone, senderPhone);

            recipientPublicKeyModulus = PublicKeys.get(recipientPhone);
            if (recipientPublicKeyModulus == null) {
                throw new RuntimeException("Failed to retrieve public key for recipient: " + recipientPhone);
            }
        }

        Log.d("Operations", "Encrypting message with recipient's " + senderPhone + " recipient public key: " + recipientPublicKeyModulus.toString(16));
        String encryptedMessage = encrypt(message, recipientPublicKeyModulus);
        sendEncryptedMessageInChunks(senderPhone, recipientPhone, encryptedMessage);
    }

    private void sendEncryptedMessageInChunks(String senderPhone, String recipientPhone, String encryptedMessage) throws IOException, JSONException, GeneralSecurityException, DataFormatException, InterruptedException {
        int chunkSize = 180;
        int messageLength = encryptedMessage.length();

        for (int i = 0; i < messageLength; i += chunkSize) {
            JSONObject chunkJson = new JSONObject();
            chunkJson.put("TYPE", "MESSAGE");
            chunkJson.put("PHONE", senderPhone);
            chunkJson.put("RECIPIENT_PHONE", recipientPhone);

            int end = Math.min(i + chunkSize, messageLength);
            String messageChunk = encryptedMessage.substring(i, end);

            if (end == messageLength) {
                messageChunk += "<END>";
            }
            chunkJson.put("MESSAGE", messageChunk);

            sendRequest(chunkJson, true);
            getResponse();
        }
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
            PublicKeys.put(phoneNumber, publicKeyModulus);
        }
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
