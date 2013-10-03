/*
 * SHL - Epoll DBus Integration
 *
 * Copyright (c) 2010-2013 David Herrmann <dh.herrmann@gmail.com>
 * Dedicated to the Public Domain
 */

/*
 * Epoll DBus Integration
 */

#include <dbus/dbus.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

struct shl_edbus;
struct shl_edbus_source;

struct shl_edbus {
	DBusConnection *dbus;
	struct shl_edbus_source *free_list;
	int efd;
	unsigned int owns_bus : 1;
};

struct shl_edbus_source {
	struct shl_edbus_source *next;
	void *object;
	int fd;
	unsigned int timeout : 1;
	unsigned int dupped : 1;
	unsigned int dead : 1;
};

static void shl_edbus_kill_source(struct shl_edbus *ctx,
				  struct shl_edbus_source *s)
{
	if (!s->dead) {
		s->dead = 1;
		s->next = ctx->free_list;
		ctx->free_list = s;
	}
}

static int shl_edbus_watch(int efd, int fd, unsigned int events,
			   struct shl_edbus_source *s)
{
	struct epoll_event ev;
	int r;

	memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = s;

	r = epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
	return (r < 0) ? -errno : 0;
}

static void shl_edbus_unwatch(int efd, int fd)
{
	/* No need to check for errors. If the fd is valid, it will get removed
	 * correctly. If it's invalid, we don't care whether this fails. */
	epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
}

static void shl_edbus_rewatch(int efd, int fd, unsigned int events,
			      struct shl_edbus_source *s)
{
	struct epoll_event ev;

	memset(&ev, 0, sizeof(ev));
	ev.events = events;
	ev.data.ptr = s;

	/* No need to check for errors. If the fd is valid, this will _always_
	 * succeed. If it's invalid, we don't care for errors. */
	epoll_ctl(efd, EPOLL_CTL_MOD, fd, &ev);
}

static void shl_edbus_store_timespec(struct timespec *ts, int64_t usec)
{
	ts->tv_sec = (time_t) (usec / 1000000ULL);
	ts->tv_nsec = (long int) ((usec % 1000000ULL) * 1000ULL);
}

static int shl_edbus_arm_timerfd(int fd, int64_t usecs)
{
	struct itimerspec its;
	int r;

	memset(&its, 0, sizeof(its));
	shl_edbus_store_timespec(&its.it_value, usecs);
	its.it_interval = its.it_value;

	r = timerfd_settime(fd, 0, &its, NULL);
	return (r < 0) ? -errno : 0;
}

static uint32_t shl_edbus_get_watch_events(struct shl_edbus_source *s)
{
	unsigned int flags;
	uint32_t events = 0;

	if (!dbus_watch_get_enabled(s->object))
		return 0;

	flags = dbus_watch_get_flags(s->object);
	if (flags & DBUS_WATCH_READABLE)
		events |= EPOLLIN;
	if (flags & DBUS_WATCH_WRITABLE)
		events |= EPOLLOUT;

	return events;
}

static unsigned int shl_edbus_events_to_flags(uint32_t events)
{
	unsigned int flags = 0;

	if (events & EPOLLIN)
		flags |= DBUS_WATCH_READABLE;
	if (events & EPOLLOUT)
		flags |= DBUS_WATCH_WRITABLE;
	if (events & EPOLLHUP)
		flags |= DBUS_WATCH_HANGUP;
	if (events & EPOLLERR)
		flags |= DBUS_WATCH_ERROR;

	return flags;
}

static dbus_bool_t shl_edbus_add_watch(DBusWatch *watch, void *data)
{
	struct shl_edbus *ctx = data;
	struct shl_edbus_source *s;
	unsigned int events;
	int r;

	s = calloc(1, sizeof(*s));
	if (!s)
		return FALSE;

	s->object = watch;
	s->fd = dbus_watch_get_unix_fd(watch);

	events = shl_edbus_get_watch_events(s) | EPOLLHUP | EPOLLERR;

	r = shl_edbus_watch(ctx->efd, s->fd, events, s);
	if (r == -EEXIST) {
		s->dupped = 1;
		s->fd = dup(s->fd);
		if (s->fd < 0) {
			free(s);
			return FALSE;
		}

		r = shl_edbus_watch(ctx->efd, s->fd, events, s);
	}

	if (r < 0) {
		if (s->dupped)
			close(s->fd);
		free(s);
		return FALSE;
	}

	dbus_watch_set_data(watch, s, NULL);
	return TRUE;
}

static void shl_edbus_remove_watch(DBusWatch *watch, void *data)
{
	struct shl_edbus *ctx = data;
	struct shl_edbus_source *s;

	s = dbus_watch_get_data(watch);
	if (!s)
		return;

	shl_edbus_unwatch(ctx->efd, s->fd);
	if (s->dupped)
		close(s->fd);
	shl_edbus_kill_source(ctx, s);
}

static void shl_edbus_toggle_watch(DBusWatch *watch, void *data)
{
	struct shl_edbus *ctx = data;
	struct shl_edbus_source *s;
	unsigned int events;

	s = dbus_watch_get_data(watch);
	if (!s)
		return;

	events = shl_edbus_get_watch_events(s) | EPOLLHUP | EPOLLERR;
	shl_edbus_rewatch(ctx->efd, s->fd, events, s);
}

static int shl_edbus_adjust_timeout(struct shl_edbus_source *s)
{
	int64_t t = 0;

	if (dbus_timeout_get_enabled(s->object)) {
		t = dbus_timeout_get_interval(s->object);
		t *= 1000ULL; /* msec to usec */
	}

	return shl_edbus_arm_timerfd(s->fd, t);
}

static dbus_bool_t shl_edbus_add_timeout(DBusTimeout *timeout, void *data)
{
	struct shl_edbus *ctx = data;
	struct shl_edbus_source *s;
	int r;

	s = calloc(1, sizeof(*s));
	if (!s)
		return FALSE;

	s->object = timeout;
	s->timeout = 1;
	s->dupped = 1;
	s->fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (s->fd < 0)
		goto err_free;

	r = shl_edbus_adjust_timeout(s);
	if (r < 0)
		goto err_close;

	r = shl_edbus_watch(ctx->efd, s->fd, EPOLLIN, s);
	if (r < 0)
		goto err_close;

	dbus_timeout_set_data(timeout, s, NULL);
	return TRUE;

err_close:
	close(s->fd);
err_free:
	free(s);
	return FALSE;
}

static void shl_edbus_remove_timeout(DBusTimeout *timeout, void *data)
{
	struct shl_edbus *ctx = data;
	struct shl_edbus_source *s;

	s = dbus_timeout_get_data(timeout);
	if (!s)
		return;

	shl_edbus_unwatch(ctx->efd, s->fd);
	close(s->fd);
	shl_edbus_kill_source(ctx, s);
}

static void shl_edbus_toggle_timeout(DBusTimeout *timeout, void *data)
{
	struct shl_edbus_source *s;

	s = dbus_timeout_get_data(timeout);
	if (!s)
		return;

	shl_edbus_adjust_timeout(s);
}

int shl_edbus_new(DBusConnection *c, struct shl_edbus **out)
{
	struct shl_edbus *ctx;
	bool b;
	int r;

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return -ENOMEM;
	ctx->dbus = c;

	ctx->efd = epoll_create1(EPOLL_CLOEXEC);
	if (ctx->efd < 0) {
		r = -errno;
		goto error;
	}

	b = dbus_connection_set_watch_functions(ctx->dbus,
						shl_edbus_add_watch,
						shl_edbus_remove_watch,
						shl_edbus_toggle_watch,
						ctx,
						NULL);
	if (!b) {
		r = -ENOMEM;
		goto error;
	}

	b = dbus_connection_set_timeout_functions(ctx->dbus,
						  shl_edbus_add_timeout,
						  shl_edbus_remove_timeout,
						  shl_edbus_toggle_timeout,
						  ctx,
						  NULL);
	if (!b) {
		r = -ENOMEM;
		goto error;
	}

	dbus_connection_ref(ctx->dbus);
	*out = ctx;
	return ctx->efd;

error:
	dbus_connection_set_timeout_functions(ctx->dbus, NULL, NULL, NULL,
					      NULL, NULL);
	dbus_connection_set_watch_functions(ctx->dbus, NULL, NULL, NULL,
					    NULL, NULL);
	if (ctx->efd >= 0)
		close(ctx->efd);
	free(ctx);
	return r;
}

int shl_edbus_new_bus(DBusBusType bus, DBusConnection **cout,
		      struct shl_edbus **out)
{
	DBusConnection *c;
	struct shl_edbus *ctx;
	int r;

	/* Ihhh, global state.. stupid dbus. */
	dbus_connection_set_change_sigpipe(FALSE);

	/* This is actually synchronous. It blocks for some authentication and
	 * setup. We just trust the dbus-server here and accept this blocking
	 * call. There is no real reason to complicate things further and make
	 * this asynchronous/non-blocking. A context should be created during
	 * thead/process/app setup, so blocking calls should be fine. */
	c = dbus_bus_get_private(bus, NULL);
	if (!c)
		return -EIO;
	dbus_connection_set_exit_on_disconnect(c, FALSE);

	r = shl_edbus_new(c, &ctx);
	if (r < 0)
		goto error;

	/* the context owns the connection */
	dbus_connection_unref(c);
	ctx->owns_bus = 1;

	*cout = c;
	*out = ctx;
	return r;

error:
	dbus_connection_close(c);
	dbus_connection_unref(c);
	return r;
}

void shl_edbus_free(struct shl_edbus *ctx)
{
	struct shl_edbus_source *s;

	dbus_connection_set_timeout_functions(ctx->dbus, NULL, NULL, NULL,
					      NULL, NULL);
	dbus_connection_set_watch_functions(ctx->dbus, NULL, NULL, NULL,
					    NULL, NULL);

	if (ctx->owns_bus)
		dbus_connection_close(ctx->dbus);
	dbus_connection_unref(ctx->dbus);

	while (ctx->free_list) {
		s = ctx->free_list;
		ctx->free_list = s->next;
		free(s);
	}

	close(ctx->efd);
	free(ctx);
}

int shl_edbus_dispatch(struct shl_edbus *ctx, int timeout_ms)
{
	struct epoll_event events[64], *e;
	struct shl_edbus_source *s;
	unsigned int flags, max;
	int r, n, i;

	max = sizeof(events) / sizeof(*events);
	r = epoll_wait(ctx->efd, events, max, timeout_ms);
	if (r < 0) {
		r = -errno;
		if (r == -EAGAIN || r == -EINTR)
			return 0;
		return r;
	}

	n = r;
	for (i = 0; i < n; ++i) {
		e = &events[i];
		s = e->data.ptr;
		if (s->dead)
			continue;

		if (s->timeout) {
			if (dbus_timeout_get_enabled(s->object))
				dbus_timeout_handle(s->object);
		} else {
			if (dbus_watch_get_enabled(s->object)) {
				flags = shl_edbus_events_to_flags(e->events);
				dbus_watch_handle(s->object, flags);
			}
		}
	}

	while (ctx->free_list) {
		s = ctx->free_list;
		ctx->free_list = s->next;
		free(s);
	}

	for (;;) {
		r = dbus_connection_dispatch(ctx->dbus);
		if (r == DBUS_DISPATCH_COMPLETE)
			break;
		else if (r == DBUS_DISPATCH_DATA_REMAINS)
			continue;
		else if (r == DBUS_DISPATCH_NEED_MEMORY)
			return -ENOMEM;
		else
			return -EIO;
	}

	return 0;
}
