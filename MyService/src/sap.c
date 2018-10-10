/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "myservice.h"
#include <glib.h>
#include <sap.h>
#include <sap_message_exchange.h>

#define MEX_PROFILE_ID "/sample/hellomessage"

struct priv {
	sap_agent_h agent;
	sap_peer_agent_h peer_agent;
};

void mex_message_delivery_status_cb(sap_peer_agent_h peer_agent_h, int transaction_id, sap_connectionless_transfer_status_e status, void *user_data);

static struct priv priv_data = { 0 };

void mex_message_delivery_status_cb(sap_peer_agent_h peer_agent_h, int transaction_id, sap_connectionless_transfer_status_e status, void *user_data)
{
	dlog_print(DLOG_DEBUG, TAG, "sap_pa_message_delivery_status_cb:  transaction_id:%d, status:%d", transaction_id, status);
}

void mex_send(char *message, int length, gboolean is_secured)
{
	int result;
	sap_peer_agent_h pa = priv_data.peer_agent;

    unsigned char bytes[4];
    unsigned long n = 1976539;

    bytes[0] = (n>>24) & 0xFF;
    bytes[1] = (n>>16) & 0xFF;
    bytes[2] = (n>>8) & 0xFF;
    bytes[3] = n & 0xFF;

	if (pa == NULL) {
//		update_ui("No peer, call find peer.");
		dlog_print(DLOG_INFO, TAG, "Wrong PEER AGENT ID");
		return;
	}

	dlog_print(DLOG_DEBUG, TAG, "pa:%u, length :%d, message:%s", pa, length,
		   message);

	if (sap_peer_agent_is_feature_enabled(pa, SAP_FEATURE_MESSAGE)) {
		result = sap_peer_agent_send_data(pa, bytes, 4, is_secured, mex_message_delivery_status_cb, NULL);
		if (result <= 0) {
			dlog_print(DLOG_DEBUG, TAG, "Error in sending mex data");
			free(message);
			dlog_print(DLOG_DEBUG, TAG, "try again or check error val , %d", result);
		}
	} else {
		dlog_print(DLOG_DEBUG, TAG, "MEX is not supported by the Peer framework");
//		update_ui("Message feature is not supported by the Peer");
		//Fallback to socket connection
	}

}

void mex_send_number(int n, int length, gboolean is_secured)
{
	int result;
	sap_peer_agent_h pa = priv_data.peer_agent;

    unsigned char bytes[4];

    bytes[0] = (n>>24) & 0xFF;
    bytes[1] = (n>>16) & 0xFF;
    bytes[2] = (n>>8) & 0xFF;
    bytes[3] = n & 0xFF;

	if (pa == NULL) {
//		update_ui("No peer, call find peer.");
		dlog_print(DLOG_INFO, TAG, "Wrong PEER AGENT ID");
		return;
	}

	if (sap_peer_agent_is_feature_enabled(pa, SAP_FEATURE_MESSAGE)) {
		result = sap_peer_agent_send_data(pa, bytes, 4, 0, mex_message_delivery_status_cb, NULL);
		if (result <= 0) {
			dlog_print(DLOG_DEBUG, TAG, "Error in sending mex data");
			free(bytes);
			dlog_print(DLOG_DEBUG, TAG, "try again or check error val , %d", result);
		}
	} else {
		dlog_print(DLOG_DEBUG, TAG, "MEX is not supported by the Peer framework");
//		update_ui("Message feature is not supported by the Peer");
		//Fallback to socket connection
	}
}


void mex_data_received_cb(sap_peer_agent_h peer_agent,
			  unsigned int payload_length,
			  void *buffer,
			  void *user_data)
{
	char *msg = g_strdup_printf("Received data: %s", (char *)buffer);
	dlog_print(DLOG_INFO, TAG, "message:%s, length:%d", buffer, payload_length);
//	update_ui(msg);
	g_free(msg);
}

void on_peer_agent_updated(sap_peer_agent_h peer_agent,
			   sap_peer_agent_status_e peer_status,
			   sap_peer_agent_found_result_e result,
			   void *user_data)
{
	switch (result) {
	case SAP_PEER_AGENT_FOUND_RESULT_DEVICE_NOT_CONNECTED:
		dlog_print(DLOG_DEBUG, TAG, "device is not connected");
//		update_ui("Device is not connected yet.");
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_FOUND:

		if (peer_status == SAP_PEER_AGENT_STATUS_AVAILABLE) {
			priv_data.peer_agent = peer_agent;
//			update_ui("peer found. Now u can send data.");
		} else {
			sap_peer_agent_destroy(peer_agent);
//			update_ui("peer lost.");
			priv_data.peer_agent = NULL;
		}

		break;

	case SAP_PEER_AGENT_FOUND_RESULT_SERVICE_NOT_FOUND:
		dlog_print(DLOG_DEBUG, TAG, "service not found");
//		update_ui("peer not found.");
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_TIMEDOUT:
		dlog_print(DLOG_DEBUG, TAG, "peer agent find timed out");
//		update_ui("find peer timed out");
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_INTERNAL_ERROR:
		dlog_print(DLOG_DEBUG, TAG, "peer agent find search failed");
//		update_ui("find peer sap error");
		break;
	}
}

static gboolean _find_peer_agent()
{
	sap_result_e result = SAP_RESULT_FAILURE;

	result = sap_agent_find_peer_agent(priv_data.agent, on_peer_agent_updated, NULL);

	if (result == SAP_RESULT_SUCCESS) {
		dlog_print(DLOG_DEBUG, TAG, "find peer call succeeded");
	} else {
		dlog_print(DLOG_DEBUG, TAG, "findsap_peer_agent_s is failed (%d)", result);
	}
	dlog_print(DLOG_DEBUG, TAG, "find peer call is over");
	return FALSE;
}

gboolean find_peers()
{
	_find_peer_agent();
	return TRUE;
}

static void on_agent_initialized(sap_agent_h agent,
				 sap_agent_initialized_result_e result,
				 void *user_data)
{
	switch (result) {
	case SAP_AGENT_INITIALIZED_RESULT_SUCCESS:
		dlog_print(DLOG_INFO, TAG, "agent is initialized");

		priv_data.agent = agent;
		sap_agent_set_data_received_cb(agent, mex_data_received_cb, NULL);

		break;

	case SAP_AGENT_INITIALIZED_RESULT_DUPLICATED:
		dlog_print(DLOG_DEBUG, TAG, "duplicate registration");
		break;

	case SAP_AGENT_INITIALIZED_RESULT_INVALID_ARGUMENTS:
		dlog_print(DLOG_DEBUG, TAG, "invalid arguments");
		break;

	case SAP_AGENT_INITIALIZED_RESULT_INTERNAL_ERROR:
		dlog_print(DLOG_DEBUG, TAG, "internal sap error");
		break;

	default:
		dlog_print(DLOG_DEBUG, TAG, "unknown status (%d)", result);
		break;
	}

	dlog_print(DLOG_DEBUG, TAG, "agent initialized callback is over");

}

static void on_device_status_changed(sap_device_status_e status, sap_transport_type_e transport_type,
				     void *user_data)
{
	switch (transport_type) {
	case SAP_TRANSPORT_TYPE_BT:
		dlog_print(DLOG_INFO, TAG, "connectivity type(%d): bt", transport_type);

		switch (status) {
		case SAP_DEVICE_STATUS_DETACHED:
			dlog_print(DLOG_DEBUG, TAG, "DEVICE GOT DISCONNECTED");
			sap_peer_agent_destroy(priv_data.peer_agent);
			priv_data.peer_agent = NULL;
//			update_ui("Device Disconnected. Call find peer after reconnection.");
			break;

		case SAP_DEVICE_STATUS_ATTACHED:
			dlog_print(DLOG_DEBUG, TAG, "DEVICE IS CONNECTED NOW, PLEASE CALL FIND PEER");
//			update_ui("Device Connected. Call Find Peer.");
			break;

		default:
			dlog_print(DLOG_DEBUG, TAG, "unknown status (%d)", status);
			break;
		}

		break;

	default:
		dlog_print(DLOG_DEBUG, TAG, "unknown connectivity type (%d)", transport_type);
		break;
	}
}

gboolean agent_initialize()
{
	int result = 0;

	do {
		result = sap_agent_initialize(priv_data.agent, MEX_PROFILE_ID, SAP_AGENT_ROLE_CONSUMER,
					      on_agent_initialized, NULL);
		dlog_print(DLOG_DEBUG, TAG, "SAP >>> getRegisteredServiceAgent() >>> %d", result);
	} while (result != SAP_RESULT_SUCCESS);

	return TRUE;
}

void initialize_sap()
{
	sap_agent_h agent = NULL;

	sap_agent_create(&agent);

	if (agent == NULL)
		dlog_print(DLOG_DEBUG, TAG, "ERROR in creating agent");
	else
		dlog_print(DLOG_DEBUG, TAG, "successfully created sap agent");

	priv_data.agent = agent;

	sap_set_device_status_changed_cb(on_device_status_changed, NULL);

	agent_initialize();
}
