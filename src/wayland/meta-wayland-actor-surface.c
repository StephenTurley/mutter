/*
 * Copyright (C) 2012,2013 Intel Corporation
 * Copyright (C) 2013-2017 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */

#include "config.h"

#include "wayland/meta-wayland-actor-surface.h"

#include "backends/meta-backend-private.h"
#include "backends/meta-logical-monitor.h"
#include "compositor/meta-surface-actor-wayland.h"
#include "compositor/region-utils.h"
#include "wayland/meta-wayland-surface.h"
#include "wayland/meta-window-wayland.h"

typedef struct _MetaWaylandActorSurfacePrivate MetaWaylandActorSurfacePrivate;

struct _MetaWaylandActorSurfacePrivate
{
  MetaSurfaceActor *actor;
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (MetaWaylandActorSurface,
                                     meta_wayland_actor_surface,
                                     META_TYPE_WAYLAND_SURFACE_ROLE)

static void
meta_wayland_actor_surface_constructed (GObject *object)
{
  G_OBJECT_CLASS (meta_wayland_actor_surface_parent_class)->constructed (object);

  meta_wayland_actor_surface_reset_actor (META_WAYLAND_ACTOR_SURFACE (object));
}

static void
meta_wayland_actor_surface_dispose (GObject *object)
{
  MetaWaylandActorSurfacePrivate *priv =
    meta_wayland_actor_surface_get_instance_private (META_WAYLAND_ACTOR_SURFACE (object));
  MetaWaylandSurface *surface =
    meta_wayland_surface_role_get_surface (META_WAYLAND_SURFACE_ROLE (object));

  if (priv->actor)
    {
      g_signal_handlers_disconnect_by_func (priv->actor,
                                            meta_wayland_surface_notify_geometry_changed,
                                            surface);
      clutter_actor_set_reactive (CLUTTER_ACTOR (priv->actor), FALSE);
      g_clear_object (&priv->actor);
    }

  G_OBJECT_CLASS (meta_wayland_actor_surface_parent_class)->dispose (object);
}

static void
meta_wayland_actor_surface_assigned (MetaWaylandSurfaceRole *surface_role)
{
  MetaWaylandActorSurfacePrivate *priv =
    meta_wayland_actor_surface_get_instance_private (META_WAYLAND_ACTOR_SURFACE (surface_role));
  MetaWaylandSurface *surface =
    meta_wayland_surface_role_get_surface (surface_role);
  GList *l;

  meta_surface_actor_wayland_add_frame_callbacks (META_SURFACE_ACTOR_WAYLAND (priv->actor),
                                                  &surface->pending_frame_callback_list);
  wl_list_init (&surface->pending_frame_callback_list);

  for (l = surface->subsurfaces; l; l = l->next)
    {
      ClutterActor *subsurface_actor =
        CLUTTER_ACTOR (meta_wayland_surface_get_actor (l->data));
      clutter_actor_add_child (CLUTTER_ACTOR (priv->actor), subsurface_actor);
    }
}

void
meta_wayland_actor_surface_queue_frame_callbacks (MetaWaylandActorSurface *actor_surface,
                                                  MetaWaylandPendingState *pending)
{
  MetaWaylandActorSurfacePrivate *priv =
    meta_wayland_actor_surface_get_instance_private (actor_surface);
  MetaSurfaceActorWayland *surface_actor_wayland =
    META_SURFACE_ACTOR_WAYLAND (priv->actor);

  meta_surface_actor_wayland_add_frame_callbacks (surface_actor_wayland,
                                                  &pending->frame_callback_list);
  wl_list_init (&pending->frame_callback_list);
}

static double
meta_wayland_actor_surface_get_geometry_scale (MetaWaylandActorSurface *actor_surface)
{
  MetaWaylandSurfaceRole *surface_role =
    META_WAYLAND_SURFACE_ROLE (actor_surface);
  MetaWaylandSurface *surface =
    meta_wayland_surface_role_get_surface (surface_role);
  MetaWindow *toplevel_window;

  toplevel_window = meta_wayland_surface_get_toplevel_window (surface);
  if (meta_is_stage_views_scaled ())
    {
      return 1;
    }
  else
    {
      if (!toplevel_window ||
          toplevel_window->client_type == META_WINDOW_CLIENT_TYPE_X11)
        return 1;
      else
        return meta_window_wayland_get_geometry_scale (toplevel_window);
    }
}

double
meta_wayland_actor_surface_calculate_scale (MetaWaylandActorSurface *actor_surface)
{
  MetaWaylandSurfaceRole *surface_role =
    META_WAYLAND_SURFACE_ROLE (actor_surface);
  MetaWaylandSurface *surface =
    meta_wayland_surface_role_get_surface (surface_role);
  double geometry_scale;

  geometry_scale =
    meta_wayland_actor_surface_get_geometry_scale (actor_surface);

  return geometry_scale / (double) surface->scale;
}

static void
meta_wayland_actor_surface_real_sync_actor_state (MetaWaylandActorSurface *actor_surface)
{
  MetaWaylandActorSurfacePrivate *priv =
    meta_wayland_actor_surface_get_instance_private (actor_surface);
  MetaWaylandSurfaceRole *surface_role =
    META_WAYLAND_SURFACE_ROLE (actor_surface);
  MetaWaylandSurface *surface =
    meta_wayland_surface_role_get_surface (surface_role);
  MetaSurfaceActor *surface_actor;
  MetaShapedTexture *stex;
  double actor_scale;
  GList *l;
  cairo_rectangle_int_t surface_rect;
  int geometry_scale;

  surface_actor = priv->actor;
  stex = meta_surface_actor_get_texture (surface_actor);

  actor_scale = meta_wayland_actor_surface_calculate_scale (actor_surface);
  clutter_actor_set_scale (CLUTTER_ACTOR (stex), actor_scale, actor_scale);

  /* Wayland surface coordinate space -> stage coordinate space */
  geometry_scale = meta_wayland_actor_surface_get_geometry_scale (actor_surface);

  surface_rect = (cairo_rectangle_int_t) {
    .width = meta_wayland_surface_get_width (surface) * geometry_scale,
    .height = meta_wayland_surface_get_height (surface) * geometry_scale,
  };

  if (surface->input_region)
    {
      cairo_region_t *scaled_input_region;

      scaled_input_region = meta_region_scale (surface->input_region,
                                               geometry_scale);
      cairo_region_intersect_rectangle (scaled_input_region, &surface_rect);
      meta_surface_actor_set_input_region (surface_actor, scaled_input_region);
      cairo_region_destroy (scaled_input_region);
    }
  else
    {
      meta_surface_actor_set_input_region (surface_actor, NULL);
    }

  if (surface->opaque_region)
    {
      cairo_region_t *scaled_opaque_region;

      /* Wayland surface coordinate space -> stage coordinate space */
      scaled_opaque_region = meta_region_scale (surface->opaque_region,
                                                geometry_scale);
      cairo_region_intersect_rectangle (scaled_opaque_region, &surface_rect);
      meta_surface_actor_set_opaque_region (surface_actor,
                                            scaled_opaque_region);
      cairo_region_destroy (scaled_opaque_region);
    }
  else
    {
      meta_surface_actor_set_opaque_region (surface_actor, NULL);
    }

  meta_surface_actor_set_transform (surface_actor, surface->buffer_transform);

  if (surface->viewport.has_src_rect)
    {
      meta_surface_actor_set_viewport_src_rect (surface_actor,
                                                &surface->viewport.src_rect);
    }
  else
    {
      meta_surface_actor_reset_viewport_src_rect (surface_actor);
    }

  if (surface->viewport.has_dst_size)
    {
      meta_surface_actor_set_viewport_dst_size (surface_actor,
                                                surface->viewport.dst_width,
                                                surface->viewport.dst_height);
    }
  else
    {
      meta_surface_actor_reset_viewport_dst_size (surface_actor);
    }

  for (l = surface->subsurfaces; l; l = l->next)
    {
      MetaWaylandSurface *subsurface_surface = l->data;
      MetaWaylandActorSurface *subsurface_actor_surface =
        META_WAYLAND_ACTOR_SURFACE (subsurface_surface->role);

      meta_wayland_actor_surface_sync_actor_state (subsurface_actor_surface);
    }
}

void
meta_wayland_actor_surface_sync_actor_state (MetaWaylandActorSurface *actor_surface)
{
  MetaWaylandActorSurfaceClass *actor_surface_class =
    META_WAYLAND_ACTOR_SURFACE_GET_CLASS (actor_surface);

  actor_surface_class->sync_actor_state (actor_surface);
}

static void
meta_wayland_actor_surface_commit (MetaWaylandSurfaceRole  *surface_role,
                                   MetaWaylandPendingState *pending)
{
  MetaWaylandActorSurface *actor_surface =
    META_WAYLAND_ACTOR_SURFACE (surface_role);
  MetaWaylandSurface *surface =
    meta_wayland_surface_role_get_surface (surface_role);
  MetaWaylandSurface *toplevel_surface;

  meta_wayland_actor_surface_queue_frame_callbacks (actor_surface, pending);

  toplevel_surface = meta_wayland_surface_get_toplevel (surface);
  if (!toplevel_surface || !toplevel_surface->window)
    return;

  meta_wayland_actor_surface_sync_actor_state (actor_surface);
}

static gboolean
meta_wayland_actor_surface_is_on_logical_monitor (MetaWaylandSurfaceRole *surface_role,
                                                  MetaLogicalMonitor     *logical_monitor)
{
  MetaWaylandActorSurfacePrivate *priv =
    meta_wayland_actor_surface_get_instance_private (META_WAYLAND_ACTOR_SURFACE (surface_role));
  ClutterActor *actor = CLUTTER_ACTOR (priv->actor);
  float x, y, width, height;
  cairo_rectangle_int_t actor_rect;
  cairo_region_t *region;
  MetaRectangle logical_monitor_layout;
  gboolean is_on_monitor;

  clutter_actor_get_transformed_position (actor, &x, &y);
  clutter_actor_get_transformed_size (actor, &width, &height);

  actor_rect.x = (int) roundf (x);
  actor_rect.y = (int) roundf (y);
  actor_rect.width = (int) roundf (x + width) - actor_rect.x;
  actor_rect.height = (int) roundf (y + height) - actor_rect.y;

  /* Calculate the scaled surface actor region. */
  region = cairo_region_create_rectangle (&actor_rect);

  logical_monitor_layout = meta_logical_monitor_get_layout (logical_monitor);

  cairo_region_intersect_rectangle (region,
				    &((cairo_rectangle_int_t) {
				      .x = logical_monitor_layout.x,
				      .y = logical_monitor_layout.y,
				      .width = logical_monitor_layout.width,
				      .height = logical_monitor_layout.height,
				    }));

  is_on_monitor = !cairo_region_is_empty (region);
  cairo_region_destroy (region);

  return is_on_monitor;
}

static void
meta_wayland_actor_surface_init (MetaWaylandActorSurface *role)
{
}

static void
meta_wayland_actor_surface_class_init (MetaWaylandActorSurfaceClass *klass)
{
  MetaWaylandSurfaceRoleClass *surface_role_class =
    META_WAYLAND_SURFACE_ROLE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = meta_wayland_actor_surface_constructed;
  object_class->dispose = meta_wayland_actor_surface_dispose;

  surface_role_class->assigned = meta_wayland_actor_surface_assigned;
  surface_role_class->commit = meta_wayland_actor_surface_commit;
  surface_role_class->is_on_logical_monitor =
    meta_wayland_actor_surface_is_on_logical_monitor;

  klass->sync_actor_state = meta_wayland_actor_surface_real_sync_actor_state;
}

MetaSurfaceActor *
meta_wayland_actor_surface_get_actor (MetaWaylandActorSurface *actor_surface)
{
  MetaWaylandActorSurfacePrivate *priv =
    meta_wayland_actor_surface_get_instance_private (actor_surface);

  return priv->actor;
}

void
meta_wayland_actor_surface_reset_actor (MetaWaylandActorSurface *actor_surface)
{
  MetaWaylandActorSurfacePrivate *priv =
    meta_wayland_actor_surface_get_instance_private (actor_surface);
  MetaWaylandSurface *surface =
    meta_wayland_surface_role_get_surface (META_WAYLAND_SURFACE_ROLE (actor_surface));

  if (priv->actor)
    {
      g_signal_handlers_disconnect_by_func (priv->actor,
                                            meta_wayland_surface_notify_geometry_changed,
                                            surface);
      g_object_unref (priv->actor);
    }

  priv->actor = g_object_ref_sink (meta_surface_actor_wayland_new (surface));

  g_signal_connect_swapped (priv->actor, "notify::allocation",
                            G_CALLBACK (meta_wayland_surface_notify_geometry_changed),
                            surface);
  g_signal_connect_swapped (priv->actor, "notify::position",
                            G_CALLBACK (meta_wayland_surface_notify_geometry_changed),
                            surface);
  g_signal_connect_swapped (priv->actor, "notify::mapped",
                            G_CALLBACK (meta_wayland_surface_notify_geometry_changed),
                            surface);
}
