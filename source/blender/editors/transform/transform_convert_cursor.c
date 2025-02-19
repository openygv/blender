/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 */

/** \file
 * \ingroup edtransform
 *
 * Instead of transforming the selection, move the 2D/3D cursor.
 */

#include "DNA_space_types.h"

#include "MEM_guardedalloc.h"

#include "BLI_math.h"

#include "BKE_context.h"
#include "BKE_report.h"
#include "BKE_scene.h"

#include "transform.h"
#include "transform_convert.h"

/* -------------------------------------------------------------------- */
/** \name Shared 2D Cursor Utilities
 * \{ */

static void createTransCursor_2D_impl(TransInfo *t, float cursor_location[2])
{
  TransData *td;
  {
    BLI_assert(t->data_container_len == 1);
    TransDataContainer *tc = t->data_container;
    tc->data_len = 1;
    td = tc->data = MEM_callocN(sizeof(TransData), "TransTexspace");
    td->ext = tc->data_ext = MEM_callocN(sizeof(TransDataExtension), "TransTexspace");
  }

  td->flag = TD_SELECTED;

  /* UV coords are scaled by aspects (see #UVsToTransData). This also applies for the Cursor in the
   * UV Editor which also means that for display and when the cursor coords are flushed
   * (recalcData_cursor_image), these are converted each time. */
  cursor_location[0] = cursor_location[0] * t->aspect[0];
  cursor_location[1] = cursor_location[1] * t->aspect[1];

  copy_v3_v3(td->center, cursor_location);
  td->ob = NULL;

  unit_m3(td->mtx);
  unit_m3(td->axismtx);
  pseudoinverse_m3_m3(td->smtx, td->mtx, PSEUDOINVERSE_EPSILON);

  td->loc = cursor_location;
  copy_v3_v3(td->iloc, cursor_location);
}

static void recalcData_cursor_2D_impl(TransInfo *t)
{
  TransDataContainer *tc = t->data_container;
  TransData *td = tc->data;
  float aspect_inv[2];

  aspect_inv[0] = 1.0f / t->aspect[0];
  aspect_inv[1] = 1.0f / t->aspect[1];

  td->loc[0] = td->loc[0] * aspect_inv[0];
  td->loc[1] = td->loc[1] * aspect_inv[1];

  DEG_id_tag_update(&t->scene->id, ID_RECALC_COPY_ON_WRITE);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Image Cursor
 * \{ */

void createTransCursor_image(TransInfo *t)
{
  SpaceImage *sima = t->area->spacedata.first;
  createTransCursor_2D_impl(t, sima->cursor);
}

void recalcData_cursor_image(TransInfo *t)
{
  recalcData_cursor_2D_impl(t);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Sequencer Cursor
 * \{ */

void createTransCursor_sequencer(TransInfo *t)
{
  SpaceSeq *sseq = t->area->spacedata.first;
  if (sseq->mainb != SEQ_DRAW_IMG_IMBUF) {
    return;
  }
  createTransCursor_2D_impl(t, sseq->cursor);
}

void recalcData_cursor_sequencer(TransInfo *t)
{
  recalcData_cursor_2D_impl(t);
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name View 3D Cursor
 * \{ */

void createTransCursor_view3d(TransInfo *t)
{
  TransData *td;

  Scene *scene = t->scene;
  if (ID_IS_LINKED(scene)) {
    BKE_report(t->reports, RPT_ERROR, "Linked data can't text-space transform");
    return;
  }

  View3DCursor *cursor = &scene->cursor;
  {
    BLI_assert(t->data_container_len == 1);
    TransDataContainer *tc = t->data_container;
    tc->data_len = 1;
    td = tc->data = MEM_callocN(sizeof(TransData), "TransTexspace");
    td->ext = tc->data_ext = MEM_callocN(sizeof(TransDataExtension), "TransTexspace");
  }

  td->flag = TD_SELECTED;
  copy_v3_v3(td->center, cursor->location);
  td->ob = NULL;

  unit_m3(td->mtx);
  BKE_scene_cursor_rot_to_mat3(cursor, td->axismtx);
  normalize_m3(td->axismtx);
  pseudoinverse_m3_m3(td->smtx, td->mtx, PSEUDOINVERSE_EPSILON);

  td->loc = cursor->location;
  copy_v3_v3(td->iloc, cursor->location);

  if (cursor->rotation_mode > 0) {
    td->ext->rot = cursor->rotation_euler;
    td->ext->rotAxis = NULL;
    td->ext->rotAngle = NULL;
    td->ext->quat = NULL;

    copy_v3_v3(td->ext->irot, cursor->rotation_euler);
  }
  else if (cursor->rotation_mode == ROT_MODE_AXISANGLE) {
    td->ext->rot = NULL;
    td->ext->rotAxis = cursor->rotation_axis;
    td->ext->rotAngle = &cursor->rotation_angle;
    td->ext->quat = NULL;

    td->ext->irotAngle = cursor->rotation_angle;
    copy_v3_v3(td->ext->irotAxis, cursor->rotation_axis);
  }
  else {
    td->ext->rot = NULL;
    td->ext->rotAxis = NULL;
    td->ext->rotAngle = NULL;
    td->ext->quat = cursor->rotation_quaternion;

    copy_qt_qt(td->ext->iquat, cursor->rotation_quaternion);
  }
  td->ext->rotOrder = cursor->rotation_mode;
}

void recalcData_cursor_view3d(TransInfo *t)
{
  DEG_id_tag_update(&t->scene->id, ID_RECALC_COPY_ON_WRITE);
}

/** \} */
