package com.example.kbud499.datareceiver;

import java.io.DataOutputStream;
import java.io.OutputStream;
import java.net.Socket;

public class SocketThread implements Runnable
{
    private static int data = -1;

    public static void setData(int data)
    {
        SocketThread.data = data;
    }

    @Override
    public void run()
    {
        OutputStream os = null;
        DataOutputStream dos = null;
        Socket socket = null;

        try
        {
            socket = new Socket("192.168.43.184", 6789);
            os = socket.getOutputStream();
            dos = new DataOutputStream(os);

            while(true)
            {
                if(data > -1)
                {
                    dos.writeInt(data);
//                    os.write(String.valueOf(data).getBytes());
                    data = -1;
//                    Thread.sleep(35);
                }
            }
        }
        catch (Exception e)
        {
         e.printStackTrace();
        }
        finally {
            try
            {
                dos.close();
                os.close();
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
        }
    }
}
