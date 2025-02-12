/*   SPDX-License-Identifier: BSD-3-Clause
 *   Copyright (C) 2018 Intel Corporation.
 *   All rights reserved.
 */

#ifndef SPDK_VBDEV_PASSTHRU_H
#define SPDK_VBDEV_PASSTHRU_H

#include "spdk/stdinc.h"

#include "spdk/bdev.h"
#include "spdk/bdev_module.h"

/**
 * Create new pass through bdev.
 *
 * \param bdev_name Bdev on which pass through vbdev will be created.
 * \param vbdev_name Name of the pass through bdev.
 * \param uuid Optional UUID to assign to the pass through bdev.
 * \return 0 on success, other on failure.
 */
int bdev_passthru_create_disk(const char *bdev_name, const char *vbdev_name,
			      const struct spdk_uuid *uuid);

/**
 * Delete passthru bdev.
 *
 * \param bdev_name Name of the pass through bdev.
 * \param cb_fn Function to call after deletion.
 * \param cb_arg Argument to pass to cb_fn.
 */
void bdev_passthru_delete_disk(const char *bdev_name, spdk_bdev_unregister_cb cb_fn,
			       void *cb_arg);

enum spdk_vbdev_passthru_mode {
	SPDK_VBDEV_PASSTHRU_MODE_FULL,
	SPDK_VBDEV_PASSTHRU_MODE_READ_ONLY,
	SPDK_VBDEV_PASSTHRU_MODE_BLOCKED,

};

/**
 * Set access mode of passthru bdev.
 *
 * \param vbdev_name Name of the pass through bdev.
 * \param mode Access mode to set.
 */
int bdev_passthru_set_mode(const char *vbdev_name, enum spdk_vbdev_passthru_mode mode);

#endif /* SPDK_VBDEV_PASSTHRU_H */
