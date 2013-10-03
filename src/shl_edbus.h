/*
 * SHL - Epoll DBus Integration
 *
 * Copyright (c) 2010-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain
 */

/*
 * Epoll DBus Integration
 * This module provides a very simple API to integrate a DBusConnection into
 * any existing event-loop. Internally it is based on embedded-epoll objects.
 * The external API allows a user to get a _single_ file-descriptor for a
 * DBusConnection object which he has to dispatch on read-events.
 */

#ifndef SHL_EDBUS_H
#define SHL_EDBUS_H

#include <dbus/dbus.h>
#include <errno.h>
#include <stdlib.h>

/* opaque edbus context object */
struct shl_edbus;

/*
 * Create a new dbus-context object for the given dbus-connection @c. The
 * context is returned in @out on success. Otherwise, @out is left untouched.
 * This function returns a negative error code on failure. Otherwise, the
 * file-descriptor of the edbus context object is returned. Whenever this
 * file-descriptor is readable, you should call shl_edbus_dispatch() to
 * dispatch dbus events. Thanks to epoll you never have to watch the fd for
 * write-events but still get asynchronous writes.
 */
int shl_edbus_new(DBusConnection *c, struct shl_edbus **out);

/* Same as shl_edbus_new but creates a private dbus-connection with sane
 * default values for your. You can specify the bus to connect to via @bus. */
int shl_edbus_new_bus(DBusBusType bus, DBusConnection **cout,
		      struct shl_edbus **out);

/* Free a edbus context object. The underlying DBusConnection object is only
 * disconnected if you created it via shl_edbus_new_bus(). Otherwise, this is
 * left to the caller. */
void shl_edbus_free(struct shl_edbus *ctx);

/*
 * Dispatch events on @ctx. Call this whenever the edbus file-descriptor is
 * readable. @timeout_ms is passed untouched to epoll_wait() so you can
 * control whether this is non-blocking or blocking.
 * This dispatches all outstanding events on the dbus connection. You're free
 * to perform any dbus actions you want from within your dbus callbacks.
 */
int shl_edbus_dispatch(struct shl_edbus *ctx, int timeout_ms);

#endif /* SHL_EDBUS_H */
