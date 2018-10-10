#include "myservicelauncher.h"
#include <app_preference.h>

#define MYSERVICE_APP_ID "org.example.myservice" // an ID of the service application of our package
#define SERVICE_ACTION_KEY "service_action" // the custom key name for the extra data send by app control to stop service
#define SERVICE_ACTION_STOP_VAL	"stop" // the custom value name for the extra data send by app control to stop service
#define PREF_KEY_HIDDEN_MODE "hidden_mode"	// the key name for the hidden mode setting in preferences

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;

	// We replace a list from previous version of the application with a genlist
	Evas_Object *genlist;

	// Circle genlist handle and circle surface handle
	Evas_Object *circle_genlist;
	Eext_Circle_Surface *surface;

	// A class for genlist regular item
	Elm_Genlist_Item_Class genlist_line_class;
	// A class for genlist header
	Elm_Genlist_Item_Class genlist_title_class;
	// A class for genlist ending margin
	Elm_Genlist_Item_Class genlist_padding_class;
	// A class for our genlist item with checkbox
	Elm_Genlist_Item_Class genlist_checkbox_class;

} appdata_s;

// The text fetching function that will be used for genlist_line_class
static char *plain_label_get(void *data, Evas_Object *obj, const char *part)
{
	char* label = (char*)data;
	return strdup(label);
}

// The text fetching function that will be used for genlist_checkbox_class
static char *checkbox_label_get(void *data, Evas_Object *obj, const char *part)
{
	return strdup("Hidden mode");
}

static void set_mode_cb(void *data, Evas_Object *obj, void *event_info);

static Evas_Object *checkbox_content_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.icon"))
	{
		// In a part named "elm_icon" we will place the checkbox widget.
		Evas_Object *checkbox = elm_check_add(obj);
		elm_object_style_set(checkbox, "on&off");

		// We don't want to propagate checkbox click event to the genlist item:
		evas_object_propagate_events_set(checkbox, EINA_FALSE);

		// Displaying the current hidden mode setting
		bool mode = false;
		if ((preference_get_boolean(PREF_KEY_HIDDEN_MODE, &mode) == PREFERENCE_ERROR_NONE)
			&& (mode == true))
		{
			dlog_print(DLOG_INFO, LOG_TAG, "Hidden mode is true!");
			elm_check_state_set(checkbox, EINA_TRUE);
		}
		else
		{
			dlog_print(DLOG_INFO, LOG_TAG, "Hidden mode is false!");
			elm_check_state_set(checkbox, EINA_FALSE);
		}
		evas_object_smart_callback_add(checkbox, "changed", set_mode_cb, NULL);
		return checkbox;
	}
	// We do nothing for any other part.
	else return NULL;
}

static void launch_service()
{
	app_control_h app_control = NULL;
	if (app_control_create(&app_control)== APP_CONTROL_ERROR_NONE)
	{
		if ((app_control_set_app_id(app_control, MYSERVICE_APP_ID) == APP_CONTROL_ERROR_NONE)
			&& (app_control_send_launch_request(app_control, NULL, NULL) == APP_CONTROL_ERROR_NONE))
		{
			dlog_print(DLOG_INFO, LOG_TAG, "App launch request sent!");
		}
		else
		{
			dlog_print(DLOG_ERROR, LOG_TAG, "App launch request sending failed!");
		}
		if (app_control_destroy(app_control) == APP_CONTROL_ERROR_NONE)
		{
			dlog_print(DLOG_INFO, LOG_TAG, "App control destroyed.");
		}
		// We exit our launcher app, there is no point in keeping it open.
		ui_app_exit();
	}
	else
	{
		dlog_print(DLOG_ERROR, LOG_TAG, "App control creation failed!");
	}
}

static void stop_service()
{
	app_control_h app_control = NULL;
	if (app_control_create(&app_control)== APP_CONTROL_ERROR_NONE)
	{
		if ((app_control_set_app_id(app_control, MYSERVICE_APP_ID) == APP_CONTROL_ERROR_NONE)
			&& (app_control_add_extra_data(app_control, SERVICE_ACTION_KEY, SERVICE_ACTION_STOP_VAL) == APP_CONTROL_ERROR_NONE)
			&& (app_control_send_launch_request(app_control, NULL, NULL) == APP_CONTROL_ERROR_NONE))
		{
			dlog_print(DLOG_INFO, LOG_TAG, "App launch request sent!");
		}
		else
		{
			dlog_print(DLOG_ERROR, LOG_TAG, "App launch request sending failed!");
		}
		if (app_control_destroy(app_control) == APP_CONTROL_ERROR_NONE)
		{
			dlog_print(DLOG_INFO, LOG_TAG, "App control destroyed.");
		}
		ui_app_exit();
	}
	else
	{
		dlog_print(DLOG_ERROR, LOG_TAG, "App control creation failed!");
	}
}

static void set_mode_cb(void *data, Evas_Object *obj, void *event_info)
{
	bool mode = false;
	if ((preference_get_boolean(PREF_KEY_HIDDEN_MODE, &mode) == PREFERENCE_ERROR_NONE)
		&& (mode == true))
	{
		if (preference_set_boolean(PREF_KEY_HIDDEN_MODE, false) == PREFERENCE_ERROR_NONE)
		{
			dlog_print(DLOG_INFO, LOG_TAG, "Hidden mode was true! Setting to false");
		}
		else
		{
			dlog_print(DLOG_ERROR, LOG_TAG, "Setting hidden mode failed!");
		}
	}
	else
	{
		if (preference_set_boolean(PREF_KEY_HIDDEN_MODE, true) == PREFERENCE_ERROR_NONE)
		{
			dlog_print(DLOG_INFO, LOG_TAG, "Hidden mode was false! Setting to true");
		}
		else
		{
			dlog_print(DLOG_ERROR, LOG_TAG, "Setting hidden mode failed!");
		}
	}
}

static void close_app_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void launch_service_cb(void *data, Evas_Object *obj, void *event_info)
{
	launch_service();
}


static void stop_service_cb(void *data, Evas_Object *obj, void *event_info)
{
	stop_service();
}

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Circle surface */
	ad->surface = eext_circle_surface_conformant_add (ad->conform);
	/* Genlist and genlist items classes */
	ad->genlist = elm_genlist_add(ad->conform);

	ad->genlist_line_class.item_style = "1text";
	ad->genlist_line_class.func.text_get = plain_label_get;
	ad->genlist_line_class.func.content_get = NULL;
	ad->genlist_line_class.func.state_get = NULL;
	ad->genlist_line_class.func.del = NULL;

	ad->genlist_title_class.item_style = "title";
	ad->genlist_title_class.func.text_get = plain_label_get;
	ad->genlist_title_class.func.content_get = NULL;
	ad->genlist_title_class.func.state_get = NULL;
	ad->genlist_title_class.func.del = NULL;

	ad->genlist_padding_class.item_style = "padding";

	// The genlist item class for the field with a label and a checkbox to the right.
	ad->genlist_checkbox_class.item_style = "1text.1icon.1";
	ad->genlist_checkbox_class.func.text_get = checkbox_label_get;
	ad->genlist_checkbox_class.func.content_get = checkbox_content_get;
	ad->genlist_checkbox_class.func.state_get = NULL;
	ad->genlist_checkbox_class.func.del = NULL;

	elm_genlist_item_append(ad->genlist, &(ad->genlist_title_class), (void*)"My Service Launcher", NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_append(ad->genlist, &(ad->genlist_checkbox_class),(void*)ad, NULL, ELM_GENLIST_ITEM_NONE, NULL, (void*)ad);
	elm_genlist_item_append(ad->genlist, &(ad->genlist_line_class), (void*)"Launch service", NULL, ELM_GENLIST_ITEM_NONE, launch_service_cb, (void*)ad);
	elm_genlist_item_append(ad->genlist, &(ad->genlist_line_class), (void*)"Stop service", NULL, ELM_GENLIST_ITEM_NONE, stop_service_cb, NULL);
	elm_genlist_item_append(ad->genlist, &(ad->genlist_line_class), (void*)"Close", NULL, ELM_GENLIST_ITEM_NONE, close_app_cb, NULL);
	elm_genlist_item_append(ad->genlist, &(ad->genlist_padding_class), NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_object_content_set(ad->conform, ad->genlist);
	evas_object_show(ad->genlist);

	/* Circle genlist extension */
	ad->circle_genlist = eext_circle_object_genlist_add(ad->genlist, ad->surface);
	eext_circle_object_genlist_scroller_policy_set(ad->circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(ad->circle_genlist, EINA_TRUE);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	bool exists = false;
	if (preference_is_existing(PREF_KEY_HIDDEN_MODE, &exists) == PREFERENCE_ERROR_NONE)
	{
		if (exists == false)
		{
			if (preference_set_boolean(PREF_KEY_HIDDEN_MODE, false) == PREFERENCE_ERROR_NONE)
			{
				dlog_print(DLOG_INFO, LOG_TAG, "Hidden mode setting was not present, setting to default false value...");
			}
			else
			{
				dlog_print(DLOG_ERROR, LOG_TAG, "Error setting preference for hidden mode!");
			}
		}
		// If the preference already exists, everything is all right - doing nothing...
	}
	else
	{
		dlog_print(DLOG_ERROR, LOG_TAG, "Error setting preference for hidden mode!");
	}

	create_base_gui(ad);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
