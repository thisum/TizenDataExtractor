/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved. 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that 
 * the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, 
 *       this list of conditions and the following disclaimer. 
 *     * Redistributions in binary form must reproduce the above copyright notice, 
 *       this list of conditions and the following disclaimer in the documentation and/or 
 *       other materials provided with the distribution. 
 *     * Neither the name of Samsung Electronics Co., Ltd. nor the names of its contributors may be used to endorse or 
 *       promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

package com.example.kbud499.datareceiver;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

import android.Manifest;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.samsung.android.sdk.accessory.example.hellomessage.provider.R;

public class ConsumerActivity extends Activity {
    private static TextView mTextView;
    private static TextView responseTextView;
    private static MessageAdapter mMessageAdapter;
    private boolean mIsBound = false;
    private ListView mMessageListView;
    private static boolean sendButtonClicked;
//    private ConsumerService mConsumerService = null;
    private ProviderService mProviderService = null;
    private static boolean startRecord;
    static StringBuffer stringBuffer = new StringBuffer();
    int count = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mTextView = (TextView) findViewById(R.id.tvStatus);
        responseTextView = (TextView) findViewById(R.id.response);
//        mMessageListView = (ListView) findViewById(R.id.lvMessage);
//        mMessageAdapter = new MessageAdapter();
//        mMessageListView.setAdapter(mMessageAdapter);
        sendButtonClicked = false;
        // Bind service
//        mIsBound = bindService(new Intent(ConsumerActivity.this, ConsumerService.class), mConnection, Context.BIND_AUTO_CREATE);
        mIsBound = bindService(new Intent(ConsumerActivity.this, ProviderService.class), mConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (ContextCompat.checkSelfPermission(this,
                Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {

            // Permission is not granted
            // Should we show an explanation?
            if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                // Show an explanation to the user *asynchronously* -- don't block
                // this thread waiting for the user's response! After the user
                // sees the explanation, try again to request the permission.
            } else {
                // No explanation needed; request the permission
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},  10000);

                // MY_PERMISSIONS_REQUEST_READ_CONTACTS is an
                // app-defined int constant. The callback method gets the
                // result of the request.
            }
        } else {
            // Permission has already been granted
        }

        SocketThread socketThread = new SocketThread();
        new Thread(socketThread).start();
    }

    @Override
    protected void onDestroy() {
        // Clean up connections
//        if (mIsBound == true && mConsumerService != null) {
//            updateTextView("Disconnected");
//            mMessageAdapter.clear();
//            mConsumerService.clearToast();
//        }
        // Un-bind service
        if (mIsBound) {
            unbindService(mConnection);
            mIsBound = false;
        }
        sendButtonClicked = false;
        super.onDestroy();
    }

    public void mOnClick(View v) {
        switch (v.getId()) {
            case R.id.buttonStartRecord: {
                stringBuffer.setLength(0);
                startRecord = true;
                break;
            }
            case R.id.buttonStopRecord: {
                startRecord = false;
//
//
//                new Thread(new Runnable() {
//
//                    int k = 400;
//
//                    @Override
//                    public void run() {
//                        try
//                        {
//                            while(k > 0)
//                            {
//                                k = (int)(Math.random() * 100000);
//                                SocketThread.setData(k);
//                                Log.i("APP", k+"");
//                                Thread.sleep(40);
//                                k--;
//                            }
//                        }
//                        catch (Exception e)
//                        {
//                            e.printStackTrace();
//                        }
//                    }
//                }).start();

                break;
            }
            case R.id.buttonWriteFile: {
//                writeToFile(stringBuffer.toString(), this);
                writeToFile(stringBuffer.toString());
            }
            case R.id.buttonClear: {
                startRecord = false;
                stringBuffer.setLength(0);
            }
            default:
        }
    }

    private final ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
//            mConsumerService = ((ConsumerService.LocalBinder) service).getService();
            mProviderService = ((ProviderService.LocalBinder) service).getService();
//            updateTextView("onServiceConnected");
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
//            mConsumerService = null;
            mProviderService = null;
            mIsBound = false;
//            updateTextView("onServiceDisconnected");
        }
    };

    public static void addMessage(String data) {
        responseTextView.setText(data);
//        mMessageAdapter.addMessage(new Message(data));
    }

    public static void updateTextView(final int record) {

        if(startRecord)
        {
            SocketThread.setData(record);
            mTextView.setText(record+"");
        }

    }

    public static void updateTextView(final String record)
    {
        if(startRecord)
        {
            stringBuffer.append(record + "\n");
            mTextView.setText(record);
        }

    }

    public static void updateButtonState(boolean enable) {
        sendButtonClicked = enable;
    }

    private class MessageAdapter extends BaseAdapter {
        private static final int MAX_MESSAGES_TO_DISPLAY = 20;
        private List<Message> mMessages;

        public MessageAdapter() {
            mMessages = Collections.synchronizedList(new ArrayList<Message>());
        }

        void addMessage(final Message msg) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if (mMessages.size() == MAX_MESSAGES_TO_DISPLAY) {
                        mMessages.remove(0);
                        mMessages.add(msg);
                    } else {
                        mMessages.add(msg);
                    }
                    notifyDataSetChanged();
                    mMessageListView.setSelection(getCount() - 1);
                }
            });
        }

        void clear() {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    mMessages.clear();
                    notifyDataSetChanged();
                }
            });
        }

        @Override
        public int getCount() {
            return mMessages.size();
        }

        @Override
        public Object getItem(int position) {
            return mMessages.get(position);
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            LayoutInflater inflator = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View messageRecordView = null;
            if (inflator != null) {
                messageRecordView = inflator.inflate(R.layout.message, null);
                TextView tvData = (TextView) messageRecordView.findViewById(R.id.tvData);
                Message message = (Message) getItem(position);
                tvData.setText(message.data);
            }
            return messageRecordView;
        }
    }

    private static final class Message {
        String data;

        public Message(String data) {
            super();
            this.data = data;
        }
    }

    private void writeToFile(String data,Context context) {
        File path = context.getFilesDir();
        File file = new File(path, "data_file.txt");
        OutputStreamWriter outputStreamWriter = null;
        FileOutputStream fo = null;
        try {
            fo = new FileOutputStream(file);
            outputStreamWriter = new OutputStreamWriter(fo);
            outputStreamWriter.write(data);
            fo.close();
            outputStreamWriter.close();
        }
        catch (IOException e) {
            Log.e("Exception", "File write failed: " + e.toString());
        }
    }

    public void writeToFile(String data)
    {
        // Get the directory for the user's public pictures directory.
        final File path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);

        // Make sure the path directory exists.
        if(!path.exists())
        {
            // Make it, if it doesn't exit
            path.mkdirs();
        }

        final File file = new File(path, count + "_config.txt");

        // Save your stream, don't forget to flush() it before closing it.

        try
        {
            file.createNewFile();
            FileOutputStream fOut = new FileOutputStream(file);
            OutputStreamWriter myOutWriter = new OutputStreamWriter(fOut);
            myOutWriter.append(data);

            myOutWriter.close();

            fOut.flush();
            fOut.close();
            count++;
        }
        catch (IOException e)
        {
            Log.e("Exception", "File write failed: " + e.toString());
        }
    }
}
