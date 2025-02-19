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
 * The Original Code is Copyright (C) 2006 Blender Foundation.
 * All rights reserved.
 */

/** \file
 * \ingroup cmpnodes
 */

#include "node_composite_util.hh"

#include "BKE_global.h"
#include "BKE_image.h"

/* **************** VIEWER ******************** */

namespace blender::nodes {

static void cmp_node_viewer_declare(NodeDeclarationBuilder &b)
{
  b.add_input<decl::Color>(N_("Image")).default_value({0.0f, 0.0f, 0.0f, 1.0f});
  b.add_input<decl::Float>(N_("Alpha")).default_value(1.0f).min(0.0f).max(1.0f);
  b.add_input<decl::Float>(N_("Z")).default_value(1.0f).min(0.0f).max(1.0f);
}

}  // namespace blender::nodes

static void node_composit_init_viewer(bNodeTree *UNUSED(ntree), bNode *node)
{
  ImageUser *iuser = (ImageUser *)MEM_callocN(sizeof(ImageUser), "node image user");
  node->storage = iuser;
  iuser->sfra = 1;
  iuser->ok = 1;
  node->custom3 = 0.5f;
  node->custom4 = 0.5f;

  node->id = (ID *)BKE_image_ensure_viewer(G.main, IMA_TYPE_COMPOSITE, "Viewer Node");
}

void register_node_type_cmp_viewer(void)
{
  static bNodeType ntype;

  cmp_node_type_base(&ntype, CMP_NODE_VIEWER, "Viewer", NODE_CLASS_OUTPUT, NODE_PREVIEW);
  ntype.declare = blender::nodes::cmp_node_viewer_declare;
  node_type_init(&ntype, node_composit_init_viewer);
  node_type_storage(&ntype, "ImageUser", node_free_standard_storage, node_copy_standard_storage);

  node_type_internal_links(&ntype, nullptr);

  nodeRegisterType(&ntype);
}
