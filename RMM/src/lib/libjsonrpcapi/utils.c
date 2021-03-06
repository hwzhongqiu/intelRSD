/**
 * Copyright (c)  2015, Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *   http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <pthread.h>

#include "libjsonrpc/jsonrpc.h"
#include "libcfg/cfg.h"

#include "libjsonrpcapi/assetd_socket.h"
#include "libutils/sock.h"
#include "libjsonrpcapi/parser.h"
#include "libassetd/assetd_type.h"
#include "libassetd/assetd_jrpc_def.h"
#include "liblog/log.h"
#include "libsecurec/safe_lib.h"


int send_msg_to_fd(jrpc_req_pkg_t *req, jrpc_rsp_pkg_t *resp, int evt_id, int fd)
{
#define MAX_RETRIES		2
#define RECV_TIMEO		5000000 /* in useconds */
	int rc, retries = MAX_RETRIES;
	fd_set fds;
	struct timeval timeo;
	static unsigned int seqnum;
	char * req_str = NULL;
	static pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;
	char rsp_string[JSONRPC_MAX_STRING_LEN] = {0};
	int64 error_code = 0;

	memset(resp, 0, sizeof(jrpc_rsp_pkg_t));
	pthread_mutex_lock(&socket_mutex);
	if (fd < 0) {
		exit(-1);
	}
	req->id = ++seqnum;

	if (NULL == (req_str = jrpc_create_req_string(req->id, am_cmd_map[evt_id].cmd_name, req->num_of_params, req->params))) {
		error_code = -1;
		goto end;
	}

	rmm_log(INFO, "evt_id: %d ,req_str: %s.\n", evt_id, req_str);
	if((rc = socket_send(fd, req_str, strnlen_s(req_str, RSIZE_MAX_STR) + 1)) < 0) {
		error_code = -1;
		goto end;
	}

	for (;;) {
		if (--retries < 0)
			break;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		timeo.tv_sec = RECV_TIMEO/1000000;//RECV_TIMEO;
		timeo.tv_usec = RECV_TIMEO%1000000;

		rc = select(fd + 1, &fds, NULL, NULL, &timeo);
		if (rc <= 0)
			continue;

		rc = socket_recv(fd, rsp_string, sizeof(rsp_string));

		if(-1 == rc || 0 != parse_rsp(rsp_string, resp)) {
			error_code = -1;
			goto end;
		}

		rmm_log(INFO, "receive: %s.\n", rsp_string);

		if (resp->id_type == JSONRPC_ID_TYPE_NORMAL &&
			resp->id == req->id) {
			if (resp->rsp_type == JSONRPC_RSP_ERROR) {
				error_code = resp->data.error.code;
			} else if (resp->rsp_type == JSONRPC_RSP_INVALID) {
				error_code = -1;
			} else {
				error_code = 0;
				goto end;
			}
		} else if (resp->id < req->id) {
			jrpc_rsp_pkg_free(resp);
			continue;
		} else {
			goto end;
		}

	}

end:
	if (req_str != NULL) {
		jrpc_free_string(req_str);
		req_str = NULL;
	}

	pthread_mutex_unlock(&socket_mutex);
	return (int)error_code;
}

int fill_param(jrpc_req_pkg_t * req, char * name, void * value, json_type type)
{
	if (req->num_of_params + 1 > sizeof(req->params)/sizeof(req->params[0]))
		return -1;
	
	req->params[req->num_of_params].name = name;
	req->params[req->num_of_params].value = value;
	req->params[req->num_of_params].value_type = type;
	req->num_of_params++;

	return 0;
}

