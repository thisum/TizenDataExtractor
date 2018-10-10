#include <tizen.h> // Standard header from the template
#include <service_app.h> // Standard header from the template
#include "myservice.h" // A header automatically created along with the template

// Headers that will be needed for our service:
#include <sensor.h>
#include <player.h>
#include <stdlib.h>
#include <stdio.h>
#include <device/power.h>
#include <app_preference.h>

//Some constant values used in the app
#define MAX_PATH_SIZE 4096 // max file path size
#define ACCELEROMETER_SHAKE_THRESHOLD_VALUE 15 // the value read on any accelerometer axis to be considered the shake event
#define MYSERVICELAUNCHER_APP_ID "org.example.myservicelauncher" // an ID of the UI application of our package
#define STRNCMP_LIMIT 256 // the limit of characters to be compared using strncmp function
#define ALARM_SOUND_FILENAME "alarm.ogg"	// the name of the sound file that will be used as an alarm sound
#define PREF_KEY_HIDDEN_MODE "hidden_mode"	// the key name for the hidden mode setting in preferences
#define BIG_NUMBER 100000


sensor_h gSensorHRM, gSensorPPG;
sensor_listener_h gListenerHRM, gListenerPPG;
bool gSensorSupported[2];
int gCount = 0, gHRMCount = 0, gPPGCount = 0;
unsigned long long gHRMTimeArray[BIG_NUMBER], gPPGTimeArray[BIG_NUMBER];
float gHRMBeatArray[BIG_NUMBER];
int gPPGLightArray[BIG_NUMBER];
int PPGData[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
//int index = 0;
int light = 0;
int sum = 0;
float average = 0.0;
float prev_ag = {0,0};
int count = 0;
float grad = 0.0;

// Application data (context) that will be passed to functions when needed
typedef struct appdata
{
	sensor_h sensor; // Sensor handle
	sensor_listener_h listener; // Sensor listener handle
	player_h player; // Player handle
	char sound_path[MAX_PATH_SIZE]; // A path to a sound file that is used as an alarm
} appdata_s;

static appdata_s *object;

void playback_completed_cb(void *user_data)
{
	// Extracting application data
	appdata_s* ad = (appdata_s*)user_data;

	dlog_print(DLOG_INFO, LOG_TAG, "Sound playback completed.");
	if (sensor_listener_start(ad->listener) == SENSOR_ERROR_NONE)
	{
		dlog_print(DLOG_INFO, LOG_TAG, "Sensor listener started again.");
	}
	// Playback completed, we should reset the player:
	if (player_unprepare(ad->player) == PLAYER_ERROR_NONE)
	{
		dlog_print(DLOG_INFO, LOG_TAG, "Player unprepared.");
	}
	else
	{
		dlog_print(DLOG_ERROR, LOG_TAG, "Error when unpreparing!");
	}
}

// Sensor event callback implementation
void sensor_event_callback(sensor_h sensor, sensor_event_s *event, void *user_data)
{
    sensor_type_e type;
    sensor_get_type(sensor, &type);

    if (type == SENSOR_HRM_LED_GREEN)
    {
        light = event->values[0];
        	     PPGData[index] = light;
        	     index++;
        	     if(index ==10)
        	     {
        	    	 	 index = 0;
        	     }
        	     sum = 0;
        	     for (int var = 0; var < 10; ++var) {
        	    	 	 sum += PPGData[var];
        		}
        	     average = sum/10;
        	     if(count%2==0)
        	     {
        	    	 	 grad = (average - prev_ag[0])/2;
        	    	 	prev_ag[0] = average;
        	     }
        	     else
        	     {
        	    	 	 grad = (average - prev_ag[1])/2;
        	    	 	prev_ag[1] = average;
        	     }

        	     //TODO use the average


		mex_send_number(light, 9, FALSE);
    }



	// Extracting application data
//	appdata_s* ad = (appdata_s*)user_data;
//
//	sensor_type_e type = SENSOR_ALL;
//
//	if((sensor_get_type(sensor, &type) == SENSOR_ERROR_NONE) && type == SENSOR_ACCELEROMETER)
//	{
//		if (event->values[0] >= ACCELEROMETER_SHAKE_THRESHOLD_VALUE
//			|| event->values[1] >= ACCELEROMETER_SHAKE_THRESHOLD_VALUE
//			|| event->values[2] >= ACCELEROMETER_SHAKE_THRESHOLD_VALUE
//			|| event->values[0] <= -ACCELEROMETER_SHAKE_THRESHOLD_VALUE
//			|| event->values[1] <= -ACCELEROMETER_SHAKE_THRESHOLD_VALUE
//			|| event->values[2] <= -ACCELEROMETER_SHAKE_THRESHOLD_VALUE)
//		{
//			// Event criteria were met, now we must wake up the device
//			// in case the screen was off when the event occurred.
//			if (device_power_wakeup(false)==DEVICE_ERROR_NONE)
//			{
//				dlog_print(DLOG_INFO, LOG_TAG, "Waking up the device!");
//			}
//			else
//			{
//				dlog_print(DLOG_ERROR, LOG_TAG, "Couldn't wake up!");
//			}
//			dlog_print(DLOG_INFO, LOG_TAG, "Event occurred!");
//			// We stop the sensor listener, because until the sound playback
//			// is finished we don't need to monitor sensors.
//			if (sensor_listener_stop(ad->listener) == SENSOR_ERROR_NONE)
//			{
//				dlog_print(DLOG_INFO, LOG_TAG, "Sensor listener temporarily stopped!");
//			}
//			else
//			{
//				dlog_print(DLOG_INFO, LOG_TAG, "Something went wrong! Sensor listener could not be stopped!");
//			}
//
//			// Starting playback:
//			player_state_e state = PLAYER_STATE_NONE;
//			if ((player_prepare(ad->player) == PLAYER_ERROR_NONE)
//				&& (player_get_state(ad->player, &state) == PLAYER_ERROR_NONE)
//				&& (state == PLAYER_STATE_READY))
//			{
//				dlog_print(DLOG_INFO, LOG_TAG, "Player is ready.");
//				if (player_start(ad->player)== PLAYER_ERROR_NONE)
//				{
//					dlog_print(DLOG_INFO, LOG_TAG, "Player started!");
//				}
//				else
//				{
//					dlog_print(DLOG_ERROR, LOG_TAG, "Starting player failed! Restarting sensor listener...");
//					if (sensor_listener_start(ad->listener) == SENSOR_ERROR_NONE)
//					{
//						dlog_print(DLOG_INFO, LOG_TAG, "Sensor restarted.");
//					}
//					else
//					{
//						dlog_print(DLOG_ERROR, LOG_TAG, "Restarting sensor failed!");
//					}
//				}
//			}
//			else
//			{
//				dlog_print(DLOG_ERROR, LOG_TAG, "Player is not ready! Player state: %d", state);
//				dlog_print(DLOG_ERROR, LOG_TAG, "Restarting sensor listener...");
//				if (sensor_listener_start(ad->listener) == SENSOR_ERROR_NONE)
//				{
//					dlog_print(DLOG_INFO, LOG_TAG, "Sensor restarted.");
//				}
//				else
//				{
//					dlog_print(DLOG_ERROR, LOG_TAG, "Restarting sensor failed!");
//				}
//			}
//
//			// We launch the launcher UI application only when the hidden mode setting is off.
//			bool mode = false;
//			if ((preference_get_boolean(PREF_KEY_HIDDEN_MODE, &mode) == PREFERENCE_ERROR_NONE)
//				&& (mode == false))
//			{
//				//Now launching the launcher...
//				app_control_h app_control = NULL;
//				if (app_control_create(&app_control)== APP_CONTROL_ERROR_NONE)
//				{
//					//Setting an app ID.
//					if (app_control_set_app_id(app_control, MYSERVICELAUNCHER_APP_ID) == APP_CONTROL_ERROR_NONE)
//					{
//						if(app_control_send_launch_request(app_control, NULL, NULL) == APP_CONTROL_ERROR_NONE)
//						{
//							dlog_print(DLOG_INFO, LOG_TAG, "App launch request sent!");
//						}
//					}
//					if (app_control_destroy(app_control) == APP_CONTROL_ERROR_NONE)
//					{
//						dlog_print(DLOG_INFO, LOG_TAG, "App control destroyed.");
//					}
//				}
//			}
//		}
//	}
}



bool service_app_create(void *data)
{
	// Extracting application data
	appdata_s* ad = (appdata_s*)data;

	//------------------------------------------------

	initialize_sap();

	char tmp_txt[100];
	object = data;

	bool supported = false;

	sensor_is_supported(SENSOR_HRM_LED_GREEN, &supported);
	if(supported)
	{
		gSensorSupported[1] = true;
		sensor_get_default_sensor(SENSOR_HRM_LED_GREEN, &gSensorPPG);
		sensor_create_listener(gSensorPPG, &gListenerPPG);
		//100 and 0 for 10hz
		//50 for 25hz
		//10 to 100hz
		//20 to 50Hz

		sensor_listener_set_event_cb(gListenerPPG, 50, sensor_event_callback, ad);
		sensor_listener_set_option(gListenerPPG,SENSOR_OPTION_ALWAYS_ON);
		sensor_listener_start(gListenerPPG);
	}
	else
	{
		gSensorSupported[0] = false;
		dlog_print(DLOG_INFO, LOG_TAG, "PPG: not supported");
	}


	find_peers();

	//------------------------------------------------

//	dlog_print(DLOG_INFO, LOG_TAG, "Starting sensor service version 3...");
//
//	bool sensor_supported = false;
//	if (sensor_is_supported(SENSOR_ACCELEROMETER, &sensor_supported) != SENSOR_ERROR_NONE || sensor_supported == false)
//	{
//		dlog_print(DLOG_ERROR, LOG_TAG, "Accelerometer not supported! Service is useless, exiting...");
//		service_app_exit();
//		return false;
//	}
//
//	// Preparing a path to the sound file.
//	// Here we get the path to a resource folder where we will find the
//	// sound file provided in the "res" folder of the application source code.
//	char *resource_path = app_get_resource_path();
//	if (resource_path != NULL)
//	{
//		snprintf(ad->sound_path, MAX_PATH_SIZE, "%s%s",  resource_path, ALARM_SOUND_FILENAME);
//		free(resource_path);
//	}
//
//	if (player_create(&(ad->player)) == PLAYER_ERROR_NONE
//		&& player_set_uri(ad->player, ad->sound_path) == PLAYER_ERROR_NONE
//		&& player_set_completed_cb(ad->player, playback_completed_cb, ad) == PLAYER_ERROR_NONE)
//	{
//		dlog_print(DLOG_INFO, LOG_TAG, "Player created.");
//	}
//	else
//	{
//		dlog_print(DLOG_ERROR, LOG_TAG, "Error when creating player!");
//	}
//
//	// Preparing and starting the sensor listener for the accelerometer.
//	if (sensor_get_default_sensor(SENSOR_ACCELEROMETER, &(ad->sensor)) == SENSOR_ERROR_NONE)
//	{
//		if (sensor_create_listener(ad->sensor, &(ad->listener)) == SENSOR_ERROR_NONE
//			&& sensor_listener_set_event_cb(ad->listener, 200, sensor_event_callback, ad) == SENSOR_ERROR_NONE
//			&& sensor_listener_set_option(ad->listener, SENSOR_OPTION_ALWAYS_ON) == SENSOR_ERROR_NONE)
//		{
//			if (sensor_listener_start(ad->listener) == SENSOR_ERROR_NONE)
//			{
//				dlog_print(DLOG_INFO, LOG_TAG, "Sensor listener started.");
//			}
//		}
//	}
	return true;
}

void service_app_terminate(void *data)
{
	// Extracting application data
	appdata_s* ad = (appdata_s*)data;

	//Stopping & destroying sensor listener
	if ((sensor_listener_stop(ad->listener) == SENSOR_ERROR_NONE)
		&& (sensor_destroy_listener(ad->listener) == SENSOR_ERROR_NONE))
	{
		dlog_print(DLOG_INFO, LOG_TAG, "Sensor listener destroyed.");
	}
	else
	{
		dlog_print(DLOG_INFO, LOG_TAG, "Error occurred when destroying sensor listener or a sensor listener was never created!");
	}

	if(gSensorSupported[0] == true) {
		sensor_listener_stop(gListenerHRM);
		sensor_destroy_listener(gListenerHRM);
	}
	if(gSensorSupported[1] == true) {
		sensor_listener_stop(gListenerPPG);
		sensor_destroy_listener(gListenerPPG);
	}


	//Stopping & destroying player
//	if (player_destroy(ad->player) == PLAYER_ERROR_NONE)
//	{
//		dlog_print(DLOG_INFO, LOG_TAG, "Player destroyed.");
//	}
//	else
//	{
//		dlog_print(DLOG_ERROR, LOG_TAG, "Error when destroying player or a player was never created!");
//	}
}

void service_app_control(app_control_h app_control, void *data)
{
	char *caller_id = NULL, *action_value = NULL;

	// Handling "stop service" app control request
	if ((app_control_get_caller(app_control, &caller_id) == APP_CONTROL_ERROR_NONE)
		&& (app_control_get_extra_data(app_control, "service_action", &action_value) == APP_CONTROL_ERROR_NONE))
	{
		if((caller_id != NULL) && (action_value != NULL)
			&& (!strncmp(caller_id, MYSERVICELAUNCHER_APP_ID, STRNCMP_LIMIT))
			&& (!strncmp(action_value,"stop", STRNCMP_LIMIT)))
		{
			dlog_print(DLOG_INFO, LOG_TAG, "Stopping MyService!");
			free(caller_id);
			free(action_value);
			service_app_exit();
			return;
		}
		else
		{
			dlog_print(DLOG_INFO, LOG_TAG, "Unsupported action! Doing nothing...");
			free(caller_id);
			free(action_value);
			caller_id = NULL;
			action_value = NULL;
		}
	}

}

static void
service_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	return;
}

static void
service_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
service_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
service_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char* argv[])
{
	// We declare application data as a structure defined earlier
	appdata_s ad = {0,};
	service_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	service_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, service_app_low_battery, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, service_app_low_memory, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, service_app_lang_changed, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, service_app_region_changed, &ad);

	// We keep a template code above and then modify the line below
	return service_app_main(argc, argv, &event_callback, &ad);
}
