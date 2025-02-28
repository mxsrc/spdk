/*   SPDX-License-Identifier: BSD-3-Clause
 *   Copyright (C) 2018 Intel Corporation.
 *   Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 *   All rights reserved.
 */

#include "vbdev_passthru.h"
#include "spdk/rpc.h"
#include "spdk/util.h"
#include "spdk/string.h"
#include "spdk/log.h"

/* Structure to hold the parameters for this RPC method. */
struct rpc_bdev_passthru_create {
	char *base_bdev_name;
	char *name;
	struct spdk_uuid uuid;
};

/* Free the allocated memory resource after the RPC handling. */
static void
free_rpc_bdev_passthru_create(struct rpc_bdev_passthru_create *r)
{
	free(r->base_bdev_name);
	free(r->name);
}

/* Structure to decode the input parameters for this RPC method. */
static const struct spdk_json_object_decoder rpc_bdev_passthru_create_decoders[] = {
	{"base_bdev_name", offsetof(struct rpc_bdev_passthru_create, base_bdev_name), spdk_json_decode_string},
	{"name", offsetof(struct rpc_bdev_passthru_create, name), spdk_json_decode_string},
	{"uuid", offsetof(struct rpc_bdev_passthru_create, uuid), spdk_json_decode_uuid, true},
};

/* Decode the parameters for this RPC method and properly construct the passthru
 * device. Error status returned in the failed cases.
 */
static void
rpc_bdev_passthru_create(struct spdk_jsonrpc_request *request,
			 const struct spdk_json_val *params)
{
	struct rpc_bdev_passthru_create req = {NULL};
	struct spdk_json_write_ctx *w;
	int rc;

	if (spdk_json_decode_object(params, rpc_bdev_passthru_create_decoders,
				    SPDK_COUNTOF(rpc_bdev_passthru_create_decoders),
				    &req)) {
		SPDK_DEBUGLOG(vbdev_passthru, "spdk_json_decode_object failed\n");
		spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INTERNAL_ERROR,
						 "spdk_json_decode_object failed");
		goto cleanup;
	}

	rc = bdev_passthru_create_disk(req.base_bdev_name, req.name, &req.uuid);
	if (rc != 0) {
		spdk_jsonrpc_send_error_response(request, rc, spdk_strerror(-rc));
		goto cleanup;
	}

	w = spdk_jsonrpc_begin_result(request);
	spdk_json_write_string(w, req.name);
	spdk_jsonrpc_end_result(request, w);

cleanup:
	free_rpc_bdev_passthru_create(&req);
}
SPDK_RPC_REGISTER("bdev_passthru_create", rpc_bdev_passthru_create, SPDK_RPC_RUNTIME)

struct rpc_bdev_passthru_delete {
	char *name;
};

static void
free_rpc_bdev_passthru_delete(struct rpc_bdev_passthru_delete *req)
{
	free(req->name);
}

static const struct spdk_json_object_decoder rpc_bdev_passthru_delete_decoders[] = {
	{"name", offsetof(struct rpc_bdev_passthru_delete, name), spdk_json_decode_string},
};

static void
rpc_bdev_passthru_delete_cb(void *cb_arg, int bdeverrno)
{
	struct spdk_jsonrpc_request *request = cb_arg;

	if (bdeverrno == 0) {
		spdk_jsonrpc_send_bool_response(request, true);
	} else {
		spdk_jsonrpc_send_error_response(request, bdeverrno, spdk_strerror(-bdeverrno));
	}
}

static void
rpc_bdev_passthru_delete(struct spdk_jsonrpc_request *request,
			 const struct spdk_json_val *params)
{
	struct rpc_bdev_passthru_delete req = {NULL};

	if (spdk_json_decode_object(params, rpc_bdev_passthru_delete_decoders,
				    SPDK_COUNTOF(rpc_bdev_passthru_delete_decoders),
				    &req)) {
		spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INTERNAL_ERROR,
						 "spdk_json_decode_object failed");
		goto cleanup;
	}

	bdev_passthru_delete_disk(req.name, rpc_bdev_passthru_delete_cb, request);

cleanup:
	free_rpc_bdev_passthru_delete(&req);
}
SPDK_RPC_REGISTER("bdev_passthru_delete", rpc_bdev_passthru_delete, SPDK_RPC_RUNTIME)

struct rpc_bdev_passthru_set_mode {
	char *name;
	char *mode;
};

static void
free_rpc_bdev_passthru_set_mode(struct rpc_bdev_passthru_set_mode *req)
{
	free(req->name);
}

static const struct spdk_json_object_decoder rpc_bdev_passthru_set_mode_decoders[] = {
	{"name", offsetof(struct rpc_bdev_passthru_set_mode, name), spdk_json_decode_string},
	{"mode", offsetof(struct rpc_bdev_passthru_set_mode, mode), spdk_json_decode_string},
};

static void
rpc_bdev_passthru_set_mode(struct spdk_jsonrpc_request *request,
			   const struct spdk_json_val *params)
{
	struct rpc_bdev_passthru_set_mode req = {NULL};

	if (spdk_json_decode_object(params, rpc_bdev_passthru_set_mode_decoders,
				    SPDK_COUNTOF(rpc_bdev_passthru_set_mode_decoders),
				    &req)) {
		spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INTERNAL_ERROR,
						 "spdk_json_decode_object failed");
		goto cleanup;
	}

	enum spdk_vbdev_passthru_mode mode;

	if (strcmp(req.mode, "full") == 0) {
		mode = SPDK_VBDEV_PASSTHRU_MODE_FULL;
	} else if (strcmp(req.mode, "read-only") == 0) {
		mode = SPDK_VBDEV_PASSTHRU_MODE_READ_ONLY;
	} else if (strcmp(req.mode, "blocked") == 0) {
		mode = SPDK_VBDEV_PASSTHRU_MODE_BLOCKED;
	} else {
		spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INTERNAL_ERROR,
						 "invalid mode");
		goto cleanup;
	}

	int rc = bdev_passthru_set_mode(req.name, mode);
	if (rc != 0) {
		spdk_jsonrpc_send_error_response(request, rc, spdk_strerror(-rc));
	} else {
		spdk_jsonrpc_send_bool_response(request, true);
	}

cleanup:
	free_rpc_bdev_passthru_set_mode(&req);
}
SPDK_RPC_REGISTER("bdev_passthru_set_mode", rpc_bdev_passthru_set_mode, SPDK_RPC_RUNTIME)

struct rpc_bdev_passthru_get_statistics {
	char* name;
};

static void
free_rpc_bdev_passthru_get_statistics(struct rpc_bdev_passthru_get_statistics *req)
{
	free(req->name);
}

static const struct spdk_json_object_decoder rpc_bdev_passthru_get_statistics_decoders[] = {
	{"name", offsetof(struct rpc_bdev_passthru_set_mode, name), spdk_json_decode_string},
};

static void
rpc_bdev_passthru_get_statistics(struct spdk_jsonrpc_request *request,
			   const struct spdk_json_val *params)
{
	struct spdk_json_write_ctx *json_ctx;

	struct rpc_bdev_passthru_get_statistics req = {NULL};

	if (spdk_json_decode_object(params, rpc_bdev_passthru_get_statistics_decoders,
				    SPDK_COUNTOF(rpc_bdev_passthru_get_statistics_decoders),
				    &req)) {
		spdk_jsonrpc_send_error_response(request, SPDK_JSONRPC_ERROR_INTERNAL_ERROR,
						 "spdk_json_decode_object failed");
		goto cleanup;
	}

	json_ctx = spdk_jsonrpc_begin_result(request);
	bdev_passthru_get_statistics(req.name, json_ctx);
	spdk_jsonrpc_end_result(request, json_ctx);

cleanup:
	free_rpc_bdev_passthru_get_statistics(&req);
}

SPDK_RPC_REGISTER("bdev_passthru_get_statistics", rpc_bdev_passthru_get_statistics, SPDK_RPC_RUNTIME)
