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
 *     * Neither the name of Samsung Electronics Co., Ltd. nor the names of its contributors may be used to endorse
 *       or promote products derived from this software without specific prior written permission.
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

import java.io.IOException;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.GregorianCalendar;

import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.widget.Toast;
import android.util.Log;

import com.samsung.android.sdk.SsdkUnsupportedException;
import com.samsung.android.sdk.accessory.*;

public class ProviderService extends SAAgent {
    private static final String TAG = "HelloMessage(P)";
    private final IBinder mBinder = new LocalBinder();
    Handler mHandler = new Handler();
    private SAMessage mMessage = null;
    private Toast mToast;
    private int receivedVal = 0;

    public ProviderService() {
        super(TAG);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        SA mAccessory = new SA();
        try {
            mAccessory.initialize(this);
        } catch (SsdkUnsupportedException e) {
            // try to handle SsdkUnsupportedException
            if (processUnsupportedException(e) == true) {
                return;
            }
        } catch (Exception e1) {
            e1.printStackTrace();
            /*
             * Your application can not use Samsung Accessory SDK. Your application should work smoothly
             * without using this SDK, or you may want to notify user and close your application gracefully
             * (release resources, stop Service threads, close UI thread, etc.)
             */
            stopSelf();
        }

        mMessage = new SAMessage(this) {

            @Override
            protected void onSent(SAPeerAgent peerAgent, int id) {
                Log.d(TAG, "onSent(), id: " + id + ", ToAgent: " + peerAgent.getPeerId());
            }

            @Override
            protected void onError(SAPeerAgent peerAgent, int id, int errorCode) {
                Log.d(TAG, "onError(), id: " + id + ", ToAgent: " + peerAgent.getPeerId() + ", errorCode: " + errorCode);
            }

            @Override
            protected void onReceive(final SAPeerAgent peerAgent, final byte[] message) {

                receivedVal = ByteBuffer.wrap(message, 0, 4).getInt();

//                Log.d(TAG, "onReceive(), FromAgent : " + receivedVal);
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        ConsumerActivity.updateTextView(receivedVal);
                    }
                });

//                displayToast("MESSAGE RECEIVED", Toast.LENGTH_SHORT);
//                Calendar calendar = new GregorianCalendar();
//                SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy.MM.dd aa hh:mm:ss.SSS");
//                String timeStr = " " + dateFormat.format(calendar.getTime());
//                String strToUpdateUI = new String(message);
//                final String str = strToUpdateUI.concat(timeStr);
//                new Thread(new Runnable() {
//                    public void run() {
//                        sendData(peerAgent, str);
//                    }
//                }).start();
            }
        };
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    protected void onFindPeerAgentsResponse(SAPeerAgent[] peerAgents, int result) {
        Log.d(TAG, "onFindPeerAgentResponse : result =" + result);
    }

    @Override
    protected void onAuthenticationResponse(SAPeerAgent peerAgent, SAAuthenticationToken authToken, int error) {
        /*
         * The authenticatePeerAgent(peerAgent) API may not be working properly depending on the firmware
         * version of accessory device. Please refer to another sample application for Security.
         */
    }

    @Override
    protected void onError(SAPeerAgent peerAgent, String errorMessage, int errorCode) {
        super.onError(peerAgent, errorMessage, errorCode);
    }

    public int sendData(SAPeerAgent peerAgent, String message) {
        int tid;
        if (mMessage != null) {
            try {
                tid = mMessage.send(peerAgent, message.getBytes());
                return tid;
            } catch (IOException e) {
                e.printStackTrace();
                Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_SHORT).show();
                return -1;
            } catch (IllegalArgumentException e) {
                e.printStackTrace();
                Toast.makeText(getApplicationContext(), e.getMessage(), Toast.LENGTH_SHORT).show();
                return -1;
            }
        }
        return -1;
    }

    private boolean processUnsupportedException(SsdkUnsupportedException e) {
        e.printStackTrace();
        int errType = e.getType();
        if (errType == SsdkUnsupportedException.VENDOR_NOT_SUPPORTED
                || errType == SsdkUnsupportedException.DEVICE_NOT_SUPPORTED) {
            /*
             * Your application can not use Samsung Accessory SDK. You application should work smoothly
             * without using this SDK, or you may want to notify user and close your app gracefully (release
             * resources, stop Service threads, close UI thread, etc.)
             */
            stopSelf();
        } else if (errType == SsdkUnsupportedException.LIBRARY_NOT_INSTALLED) {
            Log.e(TAG, "You need to install Samsung Accessory SDK to use this application.");
        } else if (errType == SsdkUnsupportedException.LIBRARY_UPDATE_IS_REQUIRED) {
            Log.e(TAG, "You need to update Samsung Accessory SDK to use this application.");
        } else if (errType == SsdkUnsupportedException.LIBRARY_UPDATE_IS_RECOMMENDED) {
            Log.e(TAG, "We recommend that you update your Samsung Accessory SDK before using this application.");
            return false;
        }
        return true;
    }

    private void displayToast(String str, int duration) {
        if(mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(getApplicationContext(), str, duration);
        mToast.show();
    }

    public class LocalBinder extends Binder {
        public ProviderService getService() {
            return ProviderService.this;
        }
    }

}
