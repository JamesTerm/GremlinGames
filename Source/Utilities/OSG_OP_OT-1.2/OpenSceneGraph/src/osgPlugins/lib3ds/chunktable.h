/* -*- c -*- */
#ifndef INCLUDED_LIB3DS_CHUNKTABLE_H
#define INCLUDED_LIB3DS_CHUNKTABLE_H
/*
 * The 3D Studio File Format Library
 * Copyright (C) 1996-2001 by J.E. Hoffmann <je-h@gmx.net>
 * All rights reserved.
 *
 * This program is  free  software;  you can redistribute it and/or modify it
 * under the terms of the  GNU Lesser General Public License  as published by
 * the  Free Software Foundation;  either version 2.1 of the License,  or (at
 * your option) any later version.
 *
 * This  program  is  distributed in  the  hope that it will  be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public
 * License for more details.
 *
 * You should  have received  a copy of the GNU Lesser General Public License
 * along with  this program;  if not, write to the  Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: chunktable.h,v 1.4 2003/01/24 15:12:52 robert Exp $
 */

#ifndef INCLUDED_LIB3DS_CHUNK_H
#include "chunk.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Lib3dsChunkTable {
    Lib3dsDword chunk;
    const char* name;
} Lib3dsChunkTable;

static Lib3dsChunkTable lib3ds_chunk_table[]={
  {LIB3DS_NULL_CHUNK, "LIB3DS_NULL_CHUNK"},
  {LIB3DS_M3DMAGIC, "LIB3DS_M3DMAGIC"},
  {LIB3DS_SMAGIC, "LIB3DS_SMAGIC"},
  {LIB3DS_LMAGIC, "LIB3DS_LMAGIC"},
  {LIB3DS_MLIBMAGIC, "LIB3DS_MLIBMAGIC"},
  {LIB3DS_MATMAGIC, "LIB3DS_MATMAGIC"},
  {LIB3DS_CMAGIC, "LIB3DS_CMAGIC"},
  {LIB3DS_M3D_VERSION, "LIB3DS_M3D_VERSION"},
  {LIB3DS_M3D_KFVERSION, "LIB3DS_M3D_KFVERSION"},
  {LIB3DS_COLOR_F, "LIB3DS_COLOR_F"},
  {LIB3DS_COLOR_24, "LIB3DS_COLOR_24"},
  {LIB3DS_LIN_COLOR_24, "LIB3DS_LIN_COLOR_24"},
  {LIB3DS_LIN_COLOR_F, "LIB3DS_LIN_COLOR_F"},
  {LIB3DS_INT_PERCENTAGE, "LIB3DS_INT_PERCENTAGE"},
  {LIB3DS_FLOAT_PERCENTAGE, "LIB3DS_FLOAT_PERCENTAGE"},
  {LIB3DS_MDATA, "LIB3DS_MDATA"},
  {LIB3DS_MESH_VERSION, "LIB3DS_MESH_VERSION"},
  {LIB3DS_MASTER_SCALE, "LIB3DS_MASTER_SCALE"},
  {LIB3DS_LO_SHADOW_BIAS, "LIB3DS_LO_SHADOW_BIAS"},
  {LIB3DS_HI_SHADOW_BIAS, "LIB3DS_HI_SHADOW_BIAS"},
  {LIB3DS_SHADOW_MAP_SIZE, "LIB3DS_SHADOW_MAP_SIZE"},
  {LIB3DS_SHADOW_SAMPLES, "LIB3DS_SHADOW_SAMPLES"},
  {LIB3DS_SHADOW_RANGE, "LIB3DS_SHADOW_RANGE"},
  {LIB3DS_SHADOW_FILTER, "LIB3DS_SHADOW_FILTER"},
  {LIB3DS_RAY_BIAS, "LIB3DS_RAY_BIAS"},
  {LIB3DS_O_CONSTS, "LIB3DS_O_CONSTS"},
  {LIB3DS_AMBIENT_LIGHT, "LIB3DS_AMBIENT_LIGHT"},
  {LIB3DS_BIT_MAP, "LIB3DS_BIT_MAP"},
  {LIB3DS_SOLID_BGND, "LIB3DS_SOLID_BGND"},
  {LIB3DS_V_GRADIENT, "LIB3DS_V_GRADIENT"},
  {LIB3DS_USE_BIT_MAP, "LIB3DS_USE_BIT_MAP"},
  {LIB3DS_USE_SOLID_BGND, "LIB3DS_USE_SOLID_BGND"},
  {LIB3DS_USE_V_GRADIENT, "LIB3DS_USE_V_GRADIENT"},
  {LIB3DS_FOG, "LIB3DS_FOG"},
  {LIB3DS_FOG_BGND, "LIB3DS_FOG_BGND"},
  {LIB3DS_LAYER_FOG, "LIB3DS_LAYER_FOG"},
  {LIB3DS_DISTANCE_CUE, "LIB3DS_DISTANCE_CUE"},
  {LIB3DS_DCUE_BGND, "LIB3DS_DCUE_BGND"},
  {LIB3DS_USE_FOG, "LIB3DS_USE_FOG"},
  {LIB3DS_USE_LAYER_FOG, "LIB3DS_USE_LAYER_FOG"},
  {LIB3DS_USE_DISTANCE_CUE, "LIB3DS_USE_DISTANCE_CUE"},
  {LIB3DS_MAT_ENTRY, "LIB3DS_MAT_ENTRY"},
  {LIB3DS_MAT_NAME, "LIB3DS_MAT_NAME"},
  {LIB3DS_MAT_AMBIENT, "LIB3DS_MAT_AMBIENT"},
  {LIB3DS_MAT_DIFFUSE, "LIB3DS_MAT_DIFFUSE"},
  {LIB3DS_MAT_SPECULAR, "LIB3DS_MAT_SPECULAR"},
  {LIB3DS_MAT_SHININESS, "LIB3DS_MAT_SHININESS"},
  {LIB3DS_MAT_SHIN2PCT, "LIB3DS_MAT_SHIN2PCT"},
  {LIB3DS_MAT_TRANSPARENCY, "LIB3DS_MAT_TRANSPARENCY"},
  {LIB3DS_MAT_XPFALL, "LIB3DS_MAT_XPFALL"},
  {LIB3DS_MAT_USE_XPFALL, "LIB3DS_MAT_USE_XPFALL"},
  {LIB3DS_MAT_REFBLUR, "LIB3DS_MAT_REFBLUR"},
  {LIB3DS_MAT_SHADING, "LIB3DS_MAT_SHADING"},
  {LIB3DS_MAT_USE_REFBLUR, "LIB3DS_MAT_USE_REFBLUR"},
  {LIB3DS_MAT_SELF_ILLUM, "LIB3DS_MAT_SELF_ILLUM"},
  {LIB3DS_MAT_TWO_SIDE, "LIB3DS_MAT_TWO_SIDE"},
  {LIB3DS_MAT_DECAL, "LIB3DS_MAT_DECAL"},
  {LIB3DS_MAT_ADDITIVE, "LIB3DS_MAT_ADDITIVE"},
  {LIB3DS_MAT_WIRE, "LIB3DS_MAT_WIRE"},
  {LIB3DS_MAT_FACEMAP, "LIB3DS_MAT_FACEMAP"},
  {LIB3DS_MAT_PHONGSOFT, "LIB3DS_MAT_PHONGSOFT"},
  {LIB3DS_MAT_WIREABS, "LIB3DS_MAT_WIREABS"},
  {LIB3DS_MAT_WIRE_SIZE, "LIB3DS_MAT_WIRE_SIZE"},
  {LIB3DS_MAT_TEXMAP, "LIB3DS_MAT_TEXMAP"},
  {LIB3DS_MAT_SXP_TEXT_DATA, "LIB3DS_MAT_SXP_TEXT_DATA"},
  {LIB3DS_MAT_TEXMASK, "LIB3DS_MAT_TEXMASK"},
  {LIB3DS_MAT_SXP_TEXTMASK_DATA, "LIB3DS_MAT_SXP_TEXTMASK_DATA"},
  {LIB3DS_MAT_TEX2MAP, "LIB3DS_MAT_TEX2MAP"},
  {LIB3DS_MAT_SXP_TEXT2_DATA, "LIB3DS_MAT_SXP_TEXT2_DATA"},
  {LIB3DS_MAT_TEX2MASK, "LIB3DS_MAT_TEX2MASK"},
  {LIB3DS_MAT_SXP_TEXT2MASK_DATA, "LIB3DS_MAT_SXP_TEXT2MASK_DATA"},
  {LIB3DS_MAT_OPACMAP, "LIB3DS_MAT_OPACMAP"},
  {LIB3DS_MAT_SXP_OPAC_DATA, "LIB3DS_MAT_SXP_OPAC_DATA"},
  {LIB3DS_MAT_OPACMASK, "LIB3DS_MAT_OPACMASK"},
  {LIB3DS_MAT_SXP_OPACMASK_DATA, "LIB3DS_MAT_SXP_OPACMASK_DATA"},
  {LIB3DS_MAT_BUMPMAP, "LIB3DS_MAT_BUMPMAP"},
  {LIB3DS_MAT_SXP_BUMP_DATA, "LIB3DS_MAT_SXP_BUMP_DATA"},
  {LIB3DS_MAT_BUMPMASK, "LIB3DS_MAT_BUMPMASK"},
  {LIB3DS_MAT_SXP_BUMPMASK_DATA, "LIB3DS_MAT_SXP_BUMPMASK_DATA"},
  {LIB3DS_MAT_SPECMAP, "LIB3DS_MAT_SPECMAP"},
  {LIB3DS_MAT_SXP_SPEC_DATA, "LIB3DS_MAT_SXP_SPEC_DATA"},
  {LIB3DS_MAT_SPECMASK, "LIB3DS_MAT_SPECMASK"},
  {LIB3DS_MAT_SXP_SPECMASK_DATA, "LIB3DS_MAT_SXP_SPECMASK_DATA"},
  {LIB3DS_MAT_SHINMAP, "LIB3DS_MAT_SHINMAP"},
  {LIB3DS_MAT_SXP_SHIN_DATA, "LIB3DS_MAT_SXP_SHIN_DATA"},
  {LIB3DS_MAT_SHINMASK, "LIB3DS_MAT_SHINMASK"},
  {LIB3DS_MAT_SXP_SHINMASK_DATA, "LIB3DS_MAT_SXP_SHINMASK_DATA"},
  {LIB3DS_MAT_SELFIMAP, "LIB3DS_MAT_SELFIMAP"},
  {LIB3DS_MAT_SXP_SELFI_DATA, "LIB3DS_MAT_SXP_SELFI_DATA"},
  {LIB3DS_MAT_SELFIMASK, "LIB3DS_MAT_SELFIMASK"},
  {LIB3DS_MAT_SXP_SELFIMASK_DATA, "LIB3DS_MAT_SXP_SELFIMASK_DATA"},
  {LIB3DS_MAT_REFLMAP, "LIB3DS_MAT_REFLMAP"},
  {LIB3DS_MAT_REFLMASK, "LIB3DS_MAT_REFLMASK"},
  {LIB3DS_MAT_SXP_REFLMASK_DATA, "LIB3DS_MAT_SXP_REFLMASK_DATA"},
  {LIB3DS_MAT_ACUBIC, "LIB3DS_MAT_ACUBIC"},
  {LIB3DS_MAT_MAPNAME, "LIB3DS_MAT_MAPNAME"},
  {LIB3DS_MAT_MAP_TILING, "LIB3DS_MAT_MAP_TILING"},
  {LIB3DS_MAT_MAP_TEXBLUR, "LIB3DS_MAT_MAP_TEXBLUR"},
  {LIB3DS_MAT_MAP_USCALE, "LIB3DS_MAT_MAP_USCALE"},
  {LIB3DS_MAT_MAP_VSCALE, "LIB3DS_MAT_MAP_VSCALE"},
  {LIB3DS_MAT_MAP_UOFFSET, "LIB3DS_MAT_MAP_UOFFSET"},
  {LIB3DS_MAT_MAP_VOFFSET, "LIB3DS_MAT_MAP_VOFFSET"},
  {LIB3DS_MAT_MAP_ANG, "LIB3DS_MAT_MAP_ANG"},
  {LIB3DS_MAT_MAP_COL1, "LIB3DS_MAT_MAP_COL1"},
  {LIB3DS_MAT_MAP_COL2, "LIB3DS_MAT_MAP_COL2"},
  {LIB3DS_MAT_MAP_RCOL, "LIB3DS_MAT_MAP_RCOL"},
  {LIB3DS_MAT_MAP_GCOL, "LIB3DS_MAT_MAP_GCOL"},
  {LIB3DS_MAT_MAP_BCOL, "LIB3DS_MAT_MAP_BCOL"},
  {LIB3DS_NAMED_OBJECT, "LIB3DS_NAMED_OBJECT"},
  {LIB3DS_N_DIRECT_LIGHT, "LIB3DS_N_DIRECT_LIGHT"},
  {LIB3DS_DL_OFF, "LIB3DS_DL_OFF"},
  {LIB3DS_DL_OUTER_RANGE, "LIB3DS_DL_OUTER_RANGE"},
  {LIB3DS_DL_INNER_RANGE, "LIB3DS_DL_INNER_RANGE"},
  {LIB3DS_DL_MULTIPLIER, "LIB3DS_DL_MULTIPLIER"},
  {LIB3DS_DL_EXCLUDE, "LIB3DS_DL_EXCLUDE"},
  {LIB3DS_DL_ATTENUATE, "LIB3DS_DL_ATTENUATE"},
  {LIB3DS_DL_SPOTLIGHT, "LIB3DS_DL_SPOTLIGHT"},
  {LIB3DS_DL_SPOT_ROLL, "LIB3DS_DL_SPOT_ROLL"},
  {LIB3DS_DL_SHADOWED, "LIB3DS_DL_SHADOWED"},
  {LIB3DS_DL_LOCAL_SHADOW2, "LIB3DS_DL_LOCAL_SHADOW2"},
  {LIB3DS_DL_SEE_CONE, "LIB3DS_DL_SEE_CONE"},
  {LIB3DS_DL_SPOT_RECTANGULAR, "LIB3DS_DL_SPOT_RECTANGULAR"},
  {LIB3DS_DL_SPOT_ASPECT, "LIB3DS_DL_SPOT_ASPECT"},
  {LIB3DS_DL_SPOT_PROJECTOR, "LIB3DS_DL_SPOT_PROJECTOR"},
  {LIB3DS_DL_SPOT_OVERSHOOT, "LIB3DS_DL_SPOT_OVERSHOOT"},
  {LIB3DS_DL_RAY_BIAS, "LIB3DS_DL_RAY_BIAS"},
  {LIB3DS_DL_RAYSHAD, "LIB3DS_DL_RAYSHAD"},
  {LIB3DS_N_CAMERA, "LIB3DS_N_CAMERA"},
  {LIB3DS_CAM_SEE_CONE, "LIB3DS_CAM_SEE_CONE"},
  {LIB3DS_CAM_RANGES, "LIB3DS_CAM_RANGES"},
  {LIB3DS_OBJ_HIDDEN, "LIB3DS_OBJ_HIDDEN"},
  {LIB3DS_OBJ_VIS_LOFTER, "LIB3DS_OBJ_VIS_LOFTER"},
  {LIB3DS_OBJ_DOESNT_CAST, "LIB3DS_OBJ_DOESNT_CAST"},
  {LIB3DS_OBJ_DONT_RECVSHADOW, "LIB3DS_OBJ_DONT_RECVSHADOW"},
  {LIB3DS_OBJ_MATTE, "LIB3DS_OBJ_MATTE"},
  {LIB3DS_OBJ_FAST, "LIB3DS_OBJ_FAST"},
  {LIB3DS_OBJ_PROCEDURAL, "LIB3DS_OBJ_PROCEDURAL"},
  {LIB3DS_OBJ_FROZEN, "LIB3DS_OBJ_FROZEN"},
  {LIB3DS_N_TRI_OBJECT, "LIB3DS_N_TRI_OBJECT"},
  {LIB3DS_POINT_ARRAY, "LIB3DS_POINT_ARRAY"},
  {LIB3DS_POINT_FLAG_ARRAY, "LIB3DS_POINT_FLAG_ARRAY"},
  {LIB3DS_FACE_ARRAY, "LIB3DS_FACE_ARRAY"},
  {LIB3DS_MSH_MAT_GROUP, "LIB3DS_MSH_MAT_GROUP"},
  {LIB3DS_SMOOTH_GROUP, "LIB3DS_SMOOTH_GROUP"},
  {LIB3DS_MSH_BOXMAP, "LIB3DS_MSH_BOXMAP"},
  {LIB3DS_TEX_VERTS, "LIB3DS_TEX_VERTS"},
  {LIB3DS_MESH_MATRIX, "LIB3DS_MESH_MATRIX"},
  {LIB3DS_MESH_COLOR, "LIB3DS_MESH_COLOR"},
  {LIB3DS_MESH_TEXTURE_INFO, "LIB3DS_MESH_TEXTURE_INFO"},
  {LIB3DS_KFDATA, "LIB3DS_KFDATA"},
  {LIB3DS_KFHDR, "LIB3DS_KFHDR"},
  {LIB3DS_KFSEG, "LIB3DS_KFSEG"},
  {LIB3DS_KFCURTIME, "LIB3DS_KFCURTIME"},
  {LIB3DS_AMBIENT_NODE_TAG, "LIB3DS_AMBIENT_NODE_TAG"},
  {LIB3DS_OBJECT_NODE_TAG, "LIB3DS_OBJECT_NODE_TAG"},
  {LIB3DS_CAMERA_NODE_TAG, "LIB3DS_CAMERA_NODE_TAG"},
  {LIB3DS_TARGET_NODE_TAG, "LIB3DS_TARGET_NODE_TAG"},
  {LIB3DS_LIGHT_NODE_TAG, "LIB3DS_LIGHT_NODE_TAG"},
  {LIB3DS_L_TARGET_NODE_TAG, "LIB3DS_L_TARGET_NODE_TAG"},
  {LIB3DS_SPOTLIGHT_NODE_TAG, "LIB3DS_SPOTLIGHT_NODE_TAG"},
  {LIB3DS_NODE_ID, "LIB3DS_NODE_ID"},
  {LIB3DS_NODE_HDR, "LIB3DS_NODE_HDR"},
  {LIB3DS_PIVOT, "LIB3DS_PIVOT"},
  {LIB3DS_INSTANCE_NAME, "LIB3DS_INSTANCE_NAME"},
  {LIB3DS_MORPH_SMOOTH, "LIB3DS_MORPH_SMOOTH"},
  {LIB3DS_BOUNDBOX, "LIB3DS_BOUNDBOX"},
  {LIB3DS_POS_TRACK_TAG, "LIB3DS_POS_TRACK_TAG"},
  {LIB3DS_COL_TRACK_TAG, "LIB3DS_COL_TRACK_TAG"},
  {LIB3DS_ROT_TRACK_TAG, "LIB3DS_ROT_TRACK_TAG"},
  {LIB3DS_SCL_TRACK_TAG, "LIB3DS_SCL_TRACK_TAG"},
  {LIB3DS_MORPH_TRACK_TAG, "LIB3DS_MORPH_TRACK_TAG"},
  {LIB3DS_FOV_TRACK_TAG, "LIB3DS_FOV_TRACK_TAG"},
  {LIB3DS_ROLL_TRACK_TAG, "LIB3DS_ROLL_TRACK_TAG"},
  {LIB3DS_HOT_TRACK_TAG, "LIB3DS_HOT_TRACK_TAG"},
  {LIB3DS_FALL_TRACK_TAG, "LIB3DS_FALL_TRACK_TAG"},
  {LIB3DS_HIDE_TRACK_TAG, "LIB3DS_HIDE_TRACK_TAG"},
  {LIB3DS_POLY_2D, "LIB3DS_POLY_2D"},
  {LIB3DS_SHAPE_OK, "LIB3DS_SHAPE_OK"},
  {LIB3DS_SHAPE_NOT_OK, "LIB3DS_SHAPE_NOT_OK"},
  {LIB3DS_SHAPE_HOOK, "LIB3DS_SHAPE_HOOK"},
  {LIB3DS_PATH_3D, "LIB3DS_PATH_3D"},
  {LIB3DS_PATH_MATRIX, "LIB3DS_PATH_MATRIX"},
  {LIB3DS_SHAPE_2D, "LIB3DS_SHAPE_2D"},
  {LIB3DS_M_SCALE, "LIB3DS_M_SCALE"},
  {LIB3DS_M_TWIST, "LIB3DS_M_TWIST"},
  {LIB3DS_M_TEETER, "LIB3DS_M_TEETER"},
  {LIB3DS_M_FIT, "LIB3DS_M_FIT"},
  {LIB3DS_M_BEVEL, "LIB3DS_M_BEVEL"},
  {LIB3DS_XZ_CURVE, "LIB3DS_XZ_CURVE"},
  {LIB3DS_YZ_CURVE, "LIB3DS_YZ_CURVE"},
  {LIB3DS_INTERPCT, "LIB3DS_INTERPCT"},
  {LIB3DS_DEFORM_LIMIT, "LIB3DS_DEFORM_LIMIT"},
  {LIB3DS_USE_CONTOUR, "LIB3DS_USE_CONTOUR"},
  {LIB3DS_USE_TWEEN, "LIB3DS_USE_TWEEN"},
  {LIB3DS_USE_SCALE, "LIB3DS_USE_SCALE"},
  {LIB3DS_USE_TWIST, "LIB3DS_USE_TWIST"},
  {LIB3DS_USE_TEETER, "LIB3DS_USE_TEETER"},
  {LIB3DS_USE_FIT, "LIB3DS_USE_FIT"},
  {LIB3DS_USE_BEVEL, "LIB3DS_USE_BEVEL"},
  {LIB3DS_DEFAULT_VIEW, "LIB3DS_DEFAULT_VIEW"},
  {LIB3DS_VIEW_TOP, "LIB3DS_VIEW_TOP"},
  {LIB3DS_VIEW_BOTTOM, "LIB3DS_VIEW_BOTTOM"},
  {LIB3DS_VIEW_LEFT, "LIB3DS_VIEW_LEFT"},
  {LIB3DS_VIEW_RIGHT, "LIB3DS_VIEW_RIGHT"},
  {LIB3DS_VIEW_FRONT, "LIB3DS_VIEW_FRONT"},
  {LIB3DS_VIEW_BACK, "LIB3DS_VIEW_BACK"},
  {LIB3DS_VIEW_USER, "LIB3DS_VIEW_USER"},
  {LIB3DS_VIEW_CAMERA, "LIB3DS_VIEW_CAMERA"},
  {LIB3DS_VIEW_WINDOW, "LIB3DS_VIEW_WINDOW"},
  {LIB3DS_VIEWPORT_LAYOUT_OLD, "LIB3DS_VIEWPORT_LAYOUT_OLD"},
  {LIB3DS_VIEWPORT_DATA_OLD, "LIB3DS_VIEWPORT_DATA_OLD"},
  {LIB3DS_VIEWPORT_LAYOUT, "LIB3DS_VIEWPORT_LAYOUT"},
  {LIB3DS_VIEWPORT_DATA, "LIB3DS_VIEWPORT_DATA"},
  {LIB3DS_VIEWPORT_DATA_3, "LIB3DS_VIEWPORT_DATA_3"},
  {LIB3DS_VIEWPORT_SIZE, "LIB3DS_VIEWPORT_SIZE"},
  {LIB3DS_NETWORK_VIEW, "LIB3DS_NETWORK_VIEW"},
  {0,0}
};

#ifdef __cplusplus
};
#endif
#endif

