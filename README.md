# TizenHRMDataCollector

An application written on Tizen 2.3.2 to retrieve HRM green LED raw data and transmit to a paried phone. This code was tested on Tizen wearable 3.0.0.1 OS. 

MyService and MyServiceLauncher are written with tizen which is the UI component and the service component. These were written using sample code provided by samsung 
DataReceiver is the code to receive the values from the watch. At the moment, there's a socket program written to strem the receiving values from a phone to a computer, give the phone and the computer connected to the same network 
