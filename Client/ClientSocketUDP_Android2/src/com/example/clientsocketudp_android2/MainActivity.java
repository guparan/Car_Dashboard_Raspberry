package com.example.clientsocketudp_android2;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.Socket;
import java.net.UnknownHostException;

import android.os.AsyncTask;
import android.os.Bundle;
import android.app.Activity;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends Activity {

    TextView textResponse, textPerformance;
    EditText editTextAddress, editTextPort;
    Button buttonConnect, buttonClear, buttonOK;
    Socket socket;
    DataInputStream input;
    BufferedReader in;
    //String message_distant;
    char[] message_distant = new char[10];
    

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        editTextAddress = (EditText)findViewById(R.id.address);
        editTextPort = (EditText)findViewById(R.id.port);
        buttonConnect = (Button)findViewById(R.id.connect);
        buttonClear = (Button)findViewById(R.id.clear);
        buttonOK = (Button)findViewById(R.id.OK);
        textResponse = (TextView)findViewById(R.id.response);
        textPerformance = (TextView)findViewById(R.id.Performance); 
        //textPerformance.setText("Hello");

        buttonConnect.setOnClickListener(buttonConnectOnClickListener);
        buttonClear.setOnClickListener(new OnClickListener()
        {
            @Override
            public void onClick(View v) {
                textResponse.setText("");
            }});
        
        buttonOK.setOnClickListener( 
        	new OnClickListener() 
	        {
	            @Override
	            public void onClick(View v) 
	            {	                
	                //textPerformance.setText( String.valueOf(message_distant) );
	            }
	        }
        );
     }

    OnClickListener buttonConnectOnClickListener = new OnClickListener()
    {
        @Override
        public void onClick(View arg0) 
        {
        	MyClientTask myClientTask = new MyClientTask();
            myClientTask.execute();
        }
	};

    public class MyClientTask extends AsyncTask<Void, Void, Void> {

        String dstAddress;
        int dstPort;
        String response;

        @Override
        protected Void doInBackground(Void... arg0) 
        {
        	try 
        	{
        		socket = new Socket(editTextAddress.getText().toString(), Integer.parseInt(editTextPort.getText().toString()) );
        		in = new BufferedReader (new InputStreamReader (socket.getInputStream()));
            } catch (UnknownHostException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
    		
    		while(true)
    		{
		        try {
					int nblus = in.read(message_distant, 0, 10);
	                textPerformance.setText( String.valueOf(message_distant) );
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}		            
		        
               // int bytesRead;
               // while ((bytesRead = inputStream.read(buffer)) != -1){
                 //   byteArrayOutputStream.write(buffer, 0, bytesRead); 
                //}   
                
                // TODO gérer le close
                //socket.close();                
		        //in.close();

                ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream(1024);               
                
                try {
					response = byteArrayOutputStream.toString("UTF-8");
				} catch (UnsupportedEncodingException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
    		}
        }
        

        @Override
        protected void onPostExecute(Void result) {
            textResponse.setText(response);
            super.onPostExecute(result);
        }

    }

}