/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Matthew Allum  <mallum@openedhand.com>
 *
 * Copyright (C) 2006 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CLUTTER_STAGE_H__
#define __CLUTTER_STAGE_H__

#if !defined(__CLUTTER_H_INSIDE__) && !defined(CLUTTER_COMPILATION)
#error "Only <clutter/clutter.h> can be included directly."
#endif

#include <clutter/clutter-types.h>
#include <clutter/clutter-group.h>

G_BEGIN_DECLS

#define CLUTTER_TYPE_STAGE              (clutter_stage_get_type())

#define CLUTTER_STAGE(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), CLUTTER_TYPE_STAGE, ClutterStage))
#define CLUTTER_STAGE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), CLUTTER_TYPE_STAGE, ClutterStageClass))
#define CLUTTER_IS_STAGE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CLUTTER_TYPE_STAGE))
#define CLUTTER_IS_STAGE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CLUTTER_TYPE_STAGE))
#define CLUTTER_STAGE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), CLUTTER_TYPE_STAGE, ClutterStageClass))

typedef struct _ClutterStageClass   ClutterStageClass;
typedef struct _ClutterStagePrivate ClutterStagePrivate;

/**
 * ClutterStage:
 *
 * The #ClutterStage structure contains only private data
 * and should be accessed using the provided API
 *
 * Since: 0.2
 */
struct _ClutterStage
{
  /*< private >*/
  ClutterGroup parent_instance;

  ClutterStagePrivate *priv;
};
/**
 * ClutterStageClass:
 * @fullscreen: handler for the #ClutterStage::fullscreen signal
 * @unfullscreen: handler for the #ClutterStage::unfullscreen signal
 * @activate: handler for the #ClutterStage::activate signal
 * @deactivate: handler for the #ClutterStage::deactivate signal
 * @delete_event: handler for the #ClutterStage::delete-event signal
 *
 * The #ClutterStageClass structure contains only private data
 *
 * Since: 0.2
 */

struct _ClutterStageClass
{
  /*< private >*/
  ClutterGroupClass parent_class;

  /*< public >*/
  /* signals */
  void (* fullscreen)   (ClutterStage *stage);
  void (* unfullscreen) (ClutterStage *stage);
  void (* activate)     (ClutterStage *stage);
  void (* deactivate)   (ClutterStage *stage);

  gboolean (* delete_event) (ClutterStage *stage,
                             ClutterEvent *event);

  /*< private >*/
  /* padding for future expansion */
  gpointer _padding_dummy[31];
};

/**
 * ClutterPerspective:
 * @fovy: the field of view angle, in degrees, in the y direction
 * @aspect: the aspect ratio that determines the field of view in the x
 *   direction. The aspect ratio is the ratio of x (width) to y (height)
 * @z_near: the distance from the viewer to the near clipping
 *   plane (always positive)
 * @z_far: the distance from the viewer to the far clipping
 *   plane (always positive)
 *
 * Stage perspective definition. #ClutterPerspective is only used by
 * the fixed point version of clutter_stage_set_perspective().
 *
 * Since: 0.4
 */
struct _ClutterPerspective
{
  gfloat fovy;
  gfloat aspect;
  gfloat z_near;
  gfloat z_far;
};

/**
 * ClutterFog:
 * @z_near: starting distance from the viewer to the near clipping
 *   plane (always positive)
 * @z_far: final distance from the viewer to the far clipping
 *   plane (always positive)
 *
 * Fog settings used to create the depth cueing effect.
 *
 * Since: 0.6
 *
 * Deprecated: 1.10: The fog-related API in #ClutterStage has been
 *   deprecated as well.
 */
struct _ClutterFog
{
  gfloat z_near;
  gfloat z_far;
};

/**
 * ClutterFrameInfo: (skip)
 */
struct _ClutterFrameInfo
{
  int64_t frame_counter;
  int64_t presentation_time;
  float refresh_rate;
};

typedef struct _ClutterCapture
{
  cairo_surface_t *image;
  cairo_rectangle_int_t rect;
} ClutterCapture;

CLUTTER_EXPORT
GType clutter_perspective_get_type (void) G_GNUC_CONST;
CLUTTER_DEPRECATED
GType clutter_fog_get_type (void) G_GNUC_CONST;
CLUTTER_EXPORT
GType clutter_stage_get_type (void) G_GNUC_CONST;

CLUTTER_EXPORT
ClutterActor *  clutter_stage_new                               (void);

CLUTTER_EXPORT
void            clutter_stage_set_perspective                   (ClutterStage          *stage,
			                                         ClutterPerspective    *perspective);
CLUTTER_EXPORT
void            clutter_stage_get_perspective                   (ClutterStage          *stage,
			                                         ClutterPerspective    *perspective);
CLUTTER_EXPORT
void            clutter_stage_set_fullscreen                    (ClutterStage          *stage,
                                                                 gboolean               fullscreen);
CLUTTER_EXPORT
gboolean        clutter_stage_get_fullscreen                    (ClutterStage          *stage);
CLUTTER_EXPORT
void            clutter_stage_show_cursor                       (ClutterStage          *stage);
CLUTTER_EXPORT
void            clutter_stage_hide_cursor                       (ClutterStage          *stage);
CLUTTER_EXPORT
void            clutter_stage_set_title                         (ClutterStage          *stage,
                                                                 const gchar           *title);
CLUTTER_EXPORT
const gchar *   clutter_stage_get_title                         (ClutterStage          *stage);
CLUTTER_EXPORT
void            clutter_stage_set_user_resizable                (ClutterStage          *stage,
						                 gboolean               resizable);
CLUTTER_EXPORT
gboolean        clutter_stage_get_user_resizable                (ClutterStage          *stage);

CLUTTER_EXPORT
void            clutter_stage_set_minimum_size                  (ClutterStage          *stage,
                                                                 guint                  width,
                                                                 guint                  height);
CLUTTER_EXPORT
void            clutter_stage_get_minimum_size                  (ClutterStage          *stage,
                                                                 guint                 *width,
                                                                 guint                 *height);
CLUTTER_EXPORT
void            clutter_stage_set_no_clear_hint                 (ClutterStage          *stage,
                                                                 gboolean               no_clear);
CLUTTER_EXPORT
gboolean        clutter_stage_get_no_clear_hint                 (ClutterStage          *stage);
CLUTTER_EXPORT
void            clutter_stage_set_use_alpha                     (ClutterStage          *stage,
                                                                 gboolean               use_alpha);
CLUTTER_EXPORT
gboolean        clutter_stage_get_use_alpha                     (ClutterStage          *stage);

CLUTTER_EXPORT
void            clutter_stage_set_key_focus                     (ClutterStage          *stage,
                                                                 ClutterActor          *actor);
CLUTTER_EXPORT
ClutterActor *  clutter_stage_get_key_focus                     (ClutterStage          *stage);
CLUTTER_EXPORT
void            clutter_stage_set_throttle_motion_events        (ClutterStage          *stage,
                                                                 gboolean               throttle);
CLUTTER_EXPORT
gboolean        clutter_stage_get_throttle_motion_events        (ClutterStage          *stage);
CLUTTER_EXPORT
void            clutter_stage_set_motion_events_enabled         (ClutterStage          *stage,
                                                                 gboolean               enabled);
CLUTTER_EXPORT
gboolean        clutter_stage_get_motion_events_enabled         (ClutterStage          *stage);
CLUTTER_EXPORT
void            clutter_stage_set_accept_focus                  (ClutterStage          *stage,
                                                                 gboolean               accept_focus);
CLUTTER_EXPORT
gboolean        clutter_stage_get_accept_focus                  (ClutterStage          *stage);
CLUTTER_EXPORT
gboolean        clutter_stage_event                             (ClutterStage          *stage,
                                                                 ClutterEvent          *event);

CLUTTER_EXPORT
ClutterActor *  clutter_stage_get_actor_at_pos                  (ClutterStage          *stage,
                                                                 ClutterPickMode        pick_mode,
                                                                 gint                   x,
                                                                 gint                   y);
CLUTTER_EXPORT
guchar *        clutter_stage_read_pixels                       (ClutterStage          *stage,
                                                                 gint                   x,
                                                                 gint                   y,
                                                                 gint                   width,
                                                                 gint                   height);

CLUTTER_EXPORT
void            clutter_stage_get_redraw_clip_bounds            (ClutterStage          *stage,
                                                                 cairo_rectangle_int_t *clip);
CLUTTER_EXPORT
void            clutter_stage_ensure_viewport                   (ClutterStage          *stage);
CLUTTER_EXPORT
void            clutter_stage_ensure_redraw                     (ClutterStage          *stage);

CLUTTER_EXPORT
gboolean        clutter_stage_is_redraw_queued                  (ClutterStage          *stage);

#ifdef CLUTTER_ENABLE_EXPERIMENTAL_API
CLUTTER_EXPORT
void            clutter_stage_set_sync_delay                    (ClutterStage          *stage,
                                                                 gint                   sync_delay);
CLUTTER_EXPORT
void            clutter_stage_skip_sync_delay                   (ClutterStage          *stage);
#endif

CLUTTER_EXPORT
gboolean clutter_stage_capture (ClutterStage          *stage,
                                gboolean               paint,
                                cairo_rectangle_int_t *rect,
                                ClutterCapture       **captures,
                                int                   *n_captures);

G_END_DECLS

#endif /* __CLUTTER_STAGE_H__ */
