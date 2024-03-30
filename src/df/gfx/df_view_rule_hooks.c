// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

////////////////////////////////
//~ rjf: Helpers

internal Vec4F32
df_view_rule_hooks__rgba_from_eval(DF_Eval eval, TG_Graph *graph, RDI_Parsed *raddbg, DF_Entity *process)
{
  Vec4F32 rgba = {0};
  Temp scratch = scratch_begin(0, 0);
  TG_Key type_key = eval.type_key;
  TG_Kind type_kind = tg_kind_from_key(type_key);
  switch(type_kind)
  {
    default:{}break;
    
    // rjf: extract r/g/b/a bytes from u32
    case TG_Kind_U32:
    case TG_Kind_S32:
    {
      U32 hex_val = (U32)eval.imm_u64;
      rgba = rgba_from_u32(hex_val);
    }break;
    
    // rjf: extract r/g/b/a values from array
    case TG_Kind_Array:
    if(eval.mode == EVAL_EvalMode_Addr)
    {
      U64 array_total_size = tg_byte_size_from_graph_rdi_key(graph, raddbg, type_key);
      U64 array_total_size_capped = ClampTop(array_total_size, 64);
      Rng1U64 array_memory_vaddr_rng = r1u64(eval.offset, eval.offset + array_total_size_capped);
      CTRL_ProcessMemorySlice array_slice = ctrl_query_cached_data_from_process_vaddr_range(scratch.arena, process->ctrl_machine_id, process->ctrl_handle, array_memory_vaddr_rng, 0);
      String8 array_memory = array_slice.data;
      TG_Key element_type_key = tg_unwrapped_direct_from_graph_rdi_key(graph, raddbg, type_key);
      TG_Kind element_type_kind = tg_kind_from_key(element_type_key);
      U64 element_type_size = tg_byte_size_from_graph_rdi_key(graph, raddbg, element_type_key);
      for(U64 element_idx = 0; element_idx < 4; element_idx += 1)
      {
        U64 offset = element_idx*element_type_size;
        if(offset >= array_memory.size)
        {
          break;
        }
        switch(element_type_kind)
        {
          default:{}break;
          case TG_Kind_U8:
          {
            U8 byte = array_memory.str[offset];
            rgba.v[element_idx] = byte/255.f;
          }break;
          case TG_Kind_F32:
          {
            rgba.v[element_idx] = *(F32 *)(array_memory.str+offset);
          }break;
        }
      }
    }break;
    
    // rjf: extract r/g/b/a values from struct
    case TG_Kind_Struct:
    case TG_Kind_Class:
    case TG_Kind_Union:
    if(eval.mode == EVAL_EvalMode_Addr)
    {
      U64 struct_total_size = tg_byte_size_from_graph_rdi_key(graph, raddbg, type_key);
      U64 struct_total_size_capped = ClampTop(struct_total_size, 64);
      Rng1U64 struct_memory_vaddr_rng = r1u64(eval.offset, eval.offset + struct_total_size_capped);
      CTRL_ProcessMemorySlice struct_slice = ctrl_query_cached_data_from_process_vaddr_range(scratch.arena, process->ctrl_machine_id, process->ctrl_handle, struct_memory_vaddr_rng, 0);
      String8 struct_memory = struct_slice.data;
      TG_Type *type = tg_type_from_graph_rdi_key(scratch.arena, graph, raddbg, type_key);
      for(U64 element_idx = 0, member_idx = 0;
          element_idx < 4 && member_idx < type->count;
          member_idx += 1)
      {
        TG_Member *member = &type->members[member_idx];
        TG_Key member_type_key = member->type_key;
        TG_Kind member_type_kind = tg_kind_from_key(member_type_key);
        B32 member_is_component = 1;
        switch(member_type_kind)
        {
          default:{member_is_component = 0;}break;
          case TG_Kind_U8:
          {
            rgba.v[element_idx] = struct_memory.str[member->off]/255.f;
          }break;
          case TG_Kind_F32:
          {
            rgba.v[element_idx] = *(F32 *)(struct_memory.str + member->off);
          }break;
        }
        if(member_is_component)
        {
          element_idx += 1;
        }
      }
    }break;
  }
  scratch_end(scratch);
  return rgba;
}

internal void
df_view_rule_hooks__eval_commit_rgba(DF_Eval eval, TG_Graph *graph, RDI_Parsed *raddbg, DF_CtrlCtx *ctrl_ctx, Vec4F32 rgba)
{
  TG_Key type_key = eval.type_key;
  TG_Kind type_kind = tg_kind_from_key(type_key);
  switch(type_kind)
  {
    default:{}break;
    
    // rjf: extract r/g/b/a bytes from u32
    case TG_Kind_U32:
    case TG_Kind_S32:
    {
      U32 val = u32_from_rgba(rgba);
      DF_Eval src_eval = eval;
      src_eval.mode = EVAL_EvalMode_Value;
      src_eval.imm_u64 = (U64)val;
      df_commit_eval_value(graph, raddbg, ctrl_ctx, eval, src_eval);
    }break;
    
#if 0
    // rjf: extract r/g/b/a values from array
    case TG_Kind_Array:
    if(eval.mode == EVAL_EvalMode_Addr)
    {
      U64 array_total_size = tg_byte_size_from_graph_rdi_key(graph, raddbg, type_key);
      U64 array_total_size_capped = ClampTop(array_total_size, 64);
      Rng1U64 array_memory_vaddr_rng = r1u64(eval.offset, eval.offset + array_total_size_capped);
      String8 array_memory = ctrl_query_cached_data_from_process_vaddr_range(scratch.arena, process->ctrl_machine_id, process->ctrl_handle, array_memory_vaddr_rng);
      TG_Key element_type_key = tg_unwrapped_direct_from_graph_rdi_key(graph, raddbg, type_key);
      TG_Kind element_type_kind = tg_kind_from_key(element_type_key);
      U64 element_type_size = tg_byte_size_from_graph_rdi_key(graph, raddbg, element_type_key);
      for(U64 element_idx = 0; element_idx < 4; element_idx += 1)
      {
        U64 offset = element_idx*element_type_size;
        if(offset >= array_memory.size)
        {
          break;
        }
        switch(element_type_kind)
        {
          default:{}break;
          case TG_Kind_U8:
          {
            U8 byte = array_memory.str[offset];
            rgba.v[element_idx] = byte/255.f;
          }break;
          case TG_Kind_F32:
          {
            rgba.v[element_idx] = *(F32 *)(array_memory.str+offset);
          }break;
        }
      }
    }break;
    
    // rjf: extract r/g/b/a values from struct
    case TG_Kind_Struct:
    case TG_Kind_Class:
    case TG_Kind_Union:
    if(eval.mode == EVAL_EvalMode_Addr)
    {
      U64 struct_total_size = tg_byte_size_from_graph_rdi_key(graph, raddbg, type_key);
      U64 struct_total_size_capped = ClampTop(struct_total_size, 64);
      Rng1U64 struct_memory_vaddr_rng = r1u64(eval.offset, eval.offset + struct_total_size_capped);
      String8 struct_memory = ctrl_query_cached_data_from_process_vaddr_range(scratch.arena, process->ctrl_machine_id, process->ctrl_handle, struct_memory_vaddr_rng);
      TG_Type *type = tg_type_from_graph_rdi_key(scratch.arena, graph, raddbg, type_key);
      for(U64 element_idx = 0, member_idx = 0;
          element_idx < 4 && member_idx < type->count;
          member_idx += 1)
      {
        TG_Member *member = &type->members[member_idx];
        TG_Key member_type_key = member->type_key;
        TG_Kind member_type_kind = tg_kind_from_key(member_type_key);
        B32 member_is_component = 1;
        switch(member_type_kind)
        {
          default:{member_is_component = 0;}break;
          case TG_Kind_U8:
          {
            rgba.v[element_idx] = struct_memory.str[member->off]/255.f;
          }break;
          case TG_Kind_F32:
          {
            rgba.v[element_idx] = *(F32 *)(struct_memory.str + member->off);
          }break;
        }
        if(member_is_component)
        {
          element_idx += 1;
        }
      }
    }break;
#endif
  }
}

internal DF_BitmapTopologyInfo
df_view_rule_hooks__bitmap_topology_info_from_cfg(DBGI_Scope *scope, DF_CtrlCtx *ctrl_ctx, EVAL_ParseCtx *parse_ctx, EVAL_String2ExprMap *macro_map, DF_CfgNode *cfg)
{
  Temp scratch = scratch_begin(0, 0);
  DF_BitmapTopologyInfo info = {0};
  {
    info.fmt = R_Tex2DFormat_RGBA8;
  }
  {
    DF_CfgNode *width_cfg  = df_cfg_node_child_from_string(cfg, str8_lit("w"), 0);
    DF_CfgNode *height_cfg = df_cfg_node_child_from_string(cfg, str8_lit("h"), 0);
    DF_CfgNode *fmt_cfg  = df_cfg_node_child_from_string(cfg, str8_lit("fmt"), 0);
    String8List width_expr_strs = {0};
    String8List height_expr_strs = {0};
    for(DF_CfgNode *child = width_cfg->first; child != &df_g_nil_cfg_node; child = child->next)
    {
      str8_list_push(scratch.arena, &width_expr_strs, child->string);
    }
    for(DF_CfgNode *child = height_cfg->first; child != &df_g_nil_cfg_node; child = child->next)
    {
      str8_list_push(scratch.arena, &height_expr_strs, child->string);
    }
    String8 width_expr = str8_list_join(scratch.arena, &width_expr_strs, 0);
    String8 height_expr = str8_list_join(scratch.arena, &height_expr_strs, 0);
    String8 fmt_string = fmt_cfg->first->string;
    DF_Eval width_eval = df_eval_from_string(scratch.arena, scope, ctrl_ctx, parse_ctx, macro_map, width_expr);
    DF_Eval width_eval_value = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, width_eval);
    info.width = width_eval_value.imm_u64;
    DF_Eval height_eval = df_eval_from_string(scratch.arena, scope, ctrl_ctx, parse_ctx, macro_map, height_expr);
    DF_Eval height_eval_value = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, height_eval);
    info.height = height_eval_value.imm_u64;
    if(fmt_string.size != 0)
    {
      for(R_Tex2DFormat fmt = (R_Tex2DFormat)0; fmt < R_Tex2DFormat_COUNT; fmt = (R_Tex2DFormat)(fmt+1))
      {
        if(str8_match(r_tex2d_format_display_string_table[fmt], fmt_string, StringMatchFlag_CaseInsensitive))
        {
          info.fmt = fmt;
          break;
        }
      }
    }
  }
  scratch_end(scratch);
  return info;
}

internal DF_GeoTopologyInfo
df_view_rule_hooks__geo_topology_info_from_cfg(DBGI_Scope *scope, DF_CtrlCtx *ctrl_ctx, EVAL_ParseCtx *parse_ctx, EVAL_String2ExprMap *macro_map, DF_CfgNode *cfg)
{
  Temp scratch = scratch_begin(0, 0);
  DF_GeoTopologyInfo result = {0};
  {
    StringJoin join = {0};
    join.sep = str8_lit(" ");
    DF_CfgNode *count_cfg         = df_cfg_node_child_from_string(cfg, str8_lit("count"), 0);
    DF_CfgNode *vertices_base_cfg = df_cfg_node_child_from_string(cfg, str8_lit("vertices_base"), 0);
    DF_CfgNode *vertices_size_cfg = df_cfg_node_child_from_string(cfg, str8_lit("vertices_size"), 0);
    String8List count_expr_strs = {0};
    String8List vertices_base_expr_strs = {0};
    String8List vertices_size_expr_strs = {0};
    for(DF_CfgNode *child = count_cfg->first; child != &df_g_nil_cfg_node; child = child->next)
    {
      str8_list_push(scratch.arena, &count_expr_strs, child->string);
    }
    for(DF_CfgNode *child = vertices_base_cfg->first; child != &df_g_nil_cfg_node; child = child->next)
    {
      str8_list_push(scratch.arena, &vertices_base_expr_strs, child->string);
    }
    for(DF_CfgNode *child = vertices_size_cfg->first; child != &df_g_nil_cfg_node; child = child->next)
    {
      str8_list_push(scratch.arena, &vertices_size_expr_strs, child->string);
    }
    String8 count_expr = str8_list_join(scratch.arena, &count_expr_strs, &join);
    String8 vertices_base_expr = str8_list_join(scratch.arena, &vertices_base_expr_strs, &join);
    String8 vertices_size_expr = str8_list_join(scratch.arena, &vertices_size_expr_strs, &join);
    DF_Eval count_eval = df_eval_from_string(scratch.arena, scope, ctrl_ctx, parse_ctx, macro_map, count_expr);
    DF_Eval vertices_base_eval = df_eval_from_string(scratch.arena, scope, ctrl_ctx, parse_ctx, macro_map, vertices_base_expr);
    DF_Eval vertices_size_eval = df_eval_from_string(scratch.arena, scope, ctrl_ctx, parse_ctx, macro_map, vertices_size_expr);
    DF_Eval count_val_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, count_eval);
    DF_Eval vertices_base_val_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, vertices_base_eval);
    DF_Eval vertices_size_val_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, vertices_size_eval);
    U64 vertices_base_vaddr = vertices_base_val_eval.imm_u64 ? vertices_base_val_eval.imm_u64 : vertices_base_val_eval.offset;
    result.index_count = count_val_eval.imm_u64;
    result.vertices_vaddr_range = r1u64(vertices_base_vaddr, vertices_base_vaddr+vertices_size_val_eval.imm_u64);
  }
  scratch_end(scratch);
  return result;
}

internal DF_TxtTopologyInfo
df_view_rule_hooks__txt_topology_info_from_cfg(DBGI_Scope *scope, DF_CtrlCtx *ctrl_ctx, EVAL_ParseCtx *parse_ctx, EVAL_String2ExprMap *macro_map, DF_CfgNode *cfg)
{
  Temp scratch = scratch_begin(0, 0);
  DF_TxtTopologyInfo result = zero_struct;
  {
    StringJoin join = {0};
    join.sep = str8_lit(" ");
    DF_CfgNode *size_cfg = df_cfg_node_child_from_string(cfg, str8_lit("size"), 0);
    DF_CfgNode *lang_cfg = df_cfg_node_child_from_string(cfg, str8_lit("lang"), 0);
    String8List size_expr_strs = {0};
    String8 lang_string = {0};
    for(DF_CfgNode *child = size_cfg->first; child != &df_g_nil_cfg_node; child = child->next)
    {
      str8_list_push(scratch.arena, &size_expr_strs, child->string);
    }
    lang_string = lang_cfg->first->string;
    String8 size_expr = str8_list_join(scratch.arena, &size_expr_strs, &join);
    DF_Eval size_eval = df_eval_from_string(scratch.arena, scope, ctrl_ctx, parse_ctx, macro_map, size_expr);
    DF_Eval size_val_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, size_eval);
    result.lang = txt_lang_kind_from_extension(lang_string);
    result.size_cap = size_val_eval.imm_u64;
  }
  scratch_end(scratch);
  return result;
}

internal DF_DisasmTopologyInfo
df_view_rule_hooks__disasm_topology_info_from_cfg(DBGI_Scope *scope, DF_CtrlCtx *ctrl_ctx, EVAL_ParseCtx *parse_ctx, EVAL_String2ExprMap *macro_map, DF_CfgNode *cfg)
{
  Temp scratch = scratch_begin(0, 0);
  DF_DisasmTopologyInfo result = zero_struct;
  {
    StringJoin join = {0};
    join.sep = str8_lit(" ");
    DF_CfgNode *size_cfg = df_cfg_node_child_from_string(cfg, str8_lit("size"), 0);
    DF_CfgNode *arch_cfg = df_cfg_node_child_from_string(cfg, str8_lit("arch"), 0);
    String8List size_expr_strs = {0};
    String8 arch_string = {0};
    for(DF_CfgNode *child = size_cfg->first; child != &df_g_nil_cfg_node; child = child->next)
    {
      str8_list_push(scratch.arena, &size_expr_strs, child->string);
    }
    arch_string = arch_cfg->first->string;
    String8 size_expr = str8_list_join(scratch.arena, &size_expr_strs, &join);
    DF_Eval size_eval = df_eval_from_string(scratch.arena, scope, ctrl_ctx, parse_ctx, macro_map, size_expr);
    DF_Eval size_val_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, size_eval);
    if(str8_match(arch_string, str8_lit("x64"), StringMatchFlag_CaseInsensitive))
    {
      result.arch = Architecture_x64;
    }
    result.size_cap = size_val_eval.imm_u64;
  }
  scratch_end(scratch);
  return result;
}

////////////////////////////////
//~ rjf: "array"

DF_CORE_VIEW_RULE_EVAL_RESOLUTION_FUNCTION_DEF(array)
{
  Temp scratch = scratch_begin(&arena, 1);
  TG_Key type_key = eval.type_key;
  TG_Kind type_kind = tg_kind_from_key(type_key);
  if(type_kind == TG_Kind_Ptr || type_kind == TG_Kind_LRef || type_kind == TG_Kind_RRef)
  {
    DF_CfgNode *array_node = val->last;
    if(array_node != &df_g_nil_cfg_node)
    {
      // rjf: determine array size
      U64 array_size = 0;
      {
        String8List array_size_expr_strs = {0};
        for(DF_CfgNode *child = array_node->first; child != &df_g_nil_cfg_node; child = child->next)
        {
          str8_list_push(scratch.arena, &array_size_expr_strs, child->string);
        }
        String8 array_size_expr = str8_list_join(scratch.arena, &array_size_expr_strs, 0);
        DF_Eval array_size_eval = df_eval_from_string(arena, dbgi_scope, ctrl_ctx, parse_ctx, macro_map, array_size_expr);
        DF_Eval array_size_eval_value = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, array_size_eval);
        eval_error_list_concat_in_place(&eval.errors, &array_size_eval.errors);
        array_size = array_size_eval_value.imm_u64;
      }
      
      // rjf: apply array size to type
      TG_Key pointee = tg_ptee_from_graph_rdi_key(parse_ctx->type_graph, parse_ctx->rdi, type_key);
      TG_Key array_type = tg_cons_type_make(parse_ctx->type_graph, TG_Kind_Array, pointee, array_size);
      eval.type_key = tg_cons_type_make(parse_ctx->type_graph, TG_Kind_Ptr, array_type, 0);
    }
  }
  scratch_end(scratch);
  return eval;
}

////////////////////////////////
//~ bill: "slice"


internal TG_Member *tg_member_from_name(TG_MemberArray array, String8 name, StringMatchFlags flags)
{
  for (U64 i = 0; i < array.count; i++)
  {
    TG_Member *member = &array.v[i];
    if (str8_match(member->name, name, flags))
    {
      return member;
    }
  }
  return NULL;
}

internal U64 df_evaluate_integer_from_eval(EVAL_ParseCtx *parse_ctx, DF_CtrlCtx *ctrl_ctx, DF_Eval eval, TG_Member *member)
{
  DF_Eval res = zero_struct;
  res.mode = EVAL_EvalMode_Addr;
  res.offset = eval.offset + member->off;
  res.type_key = member->type_key;
  res = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, res);
  if (res.mode == EVAL_EvalMode_Value)
  {
    return res.imm_u64;
  }
  return 0;
}

DF_CORE_VIEW_RULE_EVAL_RESOLUTION_FUNCTION_DEF(slice)
{
  TG_Key type_key = eval.type_key;
  TG_Kind type_kind = tg_kind_from_key(type_key);

  if (type_kind == TG_Kind_Struct)
  {
    DF_CfgNode *struct_node = val->last;
    if (struct_node != &df_g_nil_cfg_node)
    {
      Temp scratch = scratch_begin(&arena, 1);

      TG_MemberArray data_members = tg_data_members_from_graph_rdi_key(scratch.arena, parse_ctx->type_graph, parse_ctx->rdi, type_key);
      TG_Member *member_ptr = NULL;
      TG_Member *member_len = NULL;
      member_ptr = member_ptr ? member_ptr : tg_member_from_name(data_members, str8_lit("data"),   StringMatchFlag_CaseInsensitive);
      member_ptr = member_ptr ? member_ptr : tg_member_from_name(data_members, str8_lit("str"),    StringMatchFlag_CaseInsensitive);
      member_ptr = member_ptr ? member_ptr : tg_member_from_name(data_members, str8_lit("ptr"),    StringMatchFlag_CaseInsensitive);

      member_len = member_len ? member_len : tg_member_from_name(data_members, str8_lit("len"),    StringMatchFlag_CaseInsensitive);
      member_len = member_len ? member_len : tg_member_from_name(data_members, str8_lit("length"), StringMatchFlag_CaseInsensitive);
      member_len = member_len ? member_len : tg_member_from_name(data_members, str8_lit("count"),  StringMatchFlag_CaseInsensitive);
      member_len = member_len ? member_len : tg_member_from_name(data_members, str8_lit("size"),   StringMatchFlag_CaseInsensitive);

      if (member_ptr && member_len)
      {
        U64 slice_len = df_evaluate_integer_from_eval(parse_ctx, ctrl_ctx, eval, member_len);

        TG_Key pointee = tg_ptee_from_graph_rdi_key(parse_ctx->type_graph, parse_ctx->rdi, member_ptr->type_key);
        TG_Key array_type = tg_cons_type_make(parse_ctx->type_graph, TG_Kind_Array, pointee, slice_len);

        // TODO(bill): How do you make this render with the original name, if possible?
        DF_Eval new_eval = zero_struct;
        new_eval.mode = EVAL_EvalMode_Addr;
        new_eval.offset = eval.offset + member_ptr->off;
        new_eval.type_key = tg_cons_type_make(parse_ctx->type_graph, TG_Kind_Ptr, array_type, 0);

        eval = new_eval;
      }

      scratch_end(scratch);
    }
  }
  return eval;
}


////////////////////////////////
//~ bill: "odin_map"


typedef struct DF_OdinMapCellInfo DF_OdinMapCellInfo;
struct DF_OdinMapCellInfo {
  U64 size_of_type;
  U64 size_of_cell;
  U64 elements_per_cell;
};

internal DF_OdinMapCellInfo df_odin_map_cell_info(Arena *arena, EVAL_ParseCtx *parse_ctx, TG_Key type, TG_Key cell_type)
{
  DF_OdinMapCellInfo info = zero_struct;
  info.size_of_type = tg_byte_size_from_graph_rdi_key(parse_ctx->type_graph, parse_ctx->rdi, type);
  info.size_of_cell = tg_byte_size_from_graph_rdi_key(parse_ctx->type_graph, parse_ctx->rdi, cell_type);

  if (info.size_of_type != info.size_of_cell) {
      Temp scratch = scratch_begin(&arena, 1);

      TG_Kind kind = tg_kind_from_key(cell_type);
      Assert(kind == TG_Kind_Struct);

      TG_MemberArray data_members = tg_data_members_from_graph_rdi_key(scratch.arena, parse_ctx->type_graph, parse_ctx->rdi, cell_type);
      Assert(data_members.count >= 1);
      TG_Key array_type = data_members.v[0].type_key;
      U64 size_of_array = tg_byte_size_from_graph_rdi_key(parse_ctx->type_graph, parse_ctx->rdi, array_type);
      if (size_of_array > 0 && info.size_of_type > 0) {
        info.elements_per_cell = size_of_array / info.size_of_type;
      }

      scratch_end(scratch);

  }
  if (info.elements_per_cell == 0) {
    info.elements_per_cell = 1;
  }

  return info;
}


internal U64 df_odin_map_cell_index(U64 base, DF_OdinMapCellInfo const *info, U64 index)
{
  U64 elements_per_cell = info->elements_per_cell;
  U64 size_of_cell = info->size_of_cell;
  U64 size_of_type = info->size_of_type;
  U64 cell_index = 0;
  U64 data_index = 0;
  switch (elements_per_cell) {
  case 1:
    return base + (index * size_of_cell);
  case 2:
    cell_index = index >> 1;
    data_index = index & 1;
    break;
  case 4:
    cell_index = index >> 2;
    data_index = index & 3;
    break;
  case 8:
    cell_index = index >> 3;
    data_index = index & 7;
    break;
  case 16:
    cell_index = index >> 4;
    data_index = index & 15;
    break;
  case 32:
    cell_index = index >> 5;
    data_index = index & 31;
    break;
  default:
    cell_index = index / elements_per_cell;
    data_index = index % elements_per_cell;
    break;
  }
  return base + (cell_index * size_of_cell) + (data_index * size_of_type);
}

// internal void dbg_printf(char const *fmt, ...) {
//   va_list argp;
//   va_start(argp, fmt);
//   char buf[4096] = {};
//   vsnprintf_s(buf, 4095, fmt, argp);
//   va_end(argp);
//   OutputDebugStringA(buf);
// }


internal U64 df_evaluate_hash_from_offset(EVAL_ParseCtx *parse_ctx, DF_CtrlCtx *ctrl_ctx, U64 offset, TG_Key hash_type)
{
  DF_Eval res = zero_struct;
  res.mode = EVAL_EvalMode_Addr;
  res.offset = offset;
  res.type_key = hash_type;
  res = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, res);
  if (res.mode == EVAL_EvalMode_Value)
  {
    return res.imm_u64;
  }
  return 0;
}


DF_CORE_VIEW_RULE_EVAL_RESOLUTION_FUNCTION_DEF(odin_map)
{
  TG_Key type_key = eval.type_key;
  TG_Kind type_kind = tg_kind_from_key(type_key);

  if (type_kind == TG_Kind_Struct)
  {
    DF_CfgNode *struct_node = val->last;
    if (struct_node != &df_g_nil_cfg_node)
    {
      TG_MemberArray data_members = tg_data_members_from_graph_rdi_key(arena, parse_ctx->type_graph, parse_ctx->rdi, type_key);

      TG_Member *member_data = tg_member_from_name(data_members, str8_lit("data"), 0);
      TG_Member *member_len  = tg_member_from_name(data_members, str8_lit("len"),  0);

      if (member_data && member_len)
      {
        TG_Key metadata = member_data->type_key;
        metadata = tg_ptee_from_graph_rdi_key(parse_ctx->type_graph, parse_ctx->rdi, metadata);
        if (tg_kind_from_key(metadata) != TG_Kind_Struct)
        {
          return eval;
        }

        TG_MemberArray md_members = tg_data_members_from_graph_rdi_key(arena, parse_ctx->type_graph, parse_ctx->rdi, metadata);
        TG_Member *m_key        = tg_member_from_name(md_members, str8_lit("key"),        0);
        TG_Member *m_value      = tg_member_from_name(md_members, str8_lit("value"),      0);
        TG_Member *m_hash       = tg_member_from_name(md_members, str8_lit("hash"),       0);
        TG_Member *m_key_cell   = tg_member_from_name(md_members, str8_lit("key_cell"),   0);
        TG_Member *m_value_cell = tg_member_from_name(md_members, str8_lit("value_cell"), 0);

        if (m_key        == NULL ||
            m_value      == NULL ||
            m_hash       == NULL ||
            m_key_cell   == NULL ||
            m_value_cell == NULL)
        {
          return eval;
        }

        TG_Key key        = m_key->type_key;
        TG_Key value      = m_value->type_key;
        TG_Key hash       = m_hash->type_key;
        TG_Key key_cell   = m_key_cell->type_key;
        TG_Key value_cell = m_value_cell->type_key;

        U64 raw_data = df_evaluate_integer_from_eval(parse_ctx, ctrl_ctx, eval, member_data);
        U64 ptr = raw_data & ~(U64)63;
        U64 len  = df_evaluate_integer_from_eval(parse_ctx, ctrl_ctx, eval, member_len);
        U64 cap_log2 = raw_data & 63;
        U64 cap = cap_log2 ? ((U64)1)<<cap_log2 : 0;

        TG_Key array_type = zero_struct;

        DF_OdinMapCellInfo key_cell_info   = df_odin_map_cell_info(arena, parse_ctx, key,   key_cell);
        DF_OdinMapCellInfo value_cell_info = df_odin_map_cell_info(arena, parse_ctx, value, value_cell);
        U64 size_of_hash = tg_byte_size_from_graph_rdi_key(parse_ctx->type_graph, parse_ctx->rdi, hash);

        U64 key_ptr   = ptr;
        U64 value_ptr = df_odin_map_cell_index(key_ptr,   &key_cell_info,   cap);
        U64 hash_ptr  = df_odin_map_cell_index(value_ptr, &value_cell_info, cap);
        (void)value_ptr;
        (void)hash_ptr;

        U64 TOMBSTONE_MASK = ((U64)1u)<<(size_of_hash*8 - 1);

        for (U64 i = 0; i < cap; i += 1) {
          U64 offset_hash = hash_ptr + i*size_of_hash;

          U64 h = df_evaluate_hash_from_offset(parse_ctx, ctrl_ctx, offset_hash, hash);
          if (h != 0 && (h & TOMBSTONE_MASK) == 0) {
            U64 offset_key   = df_odin_map_cell_index(key_ptr,   &key_cell_info,   i);
            U64 offset_value = df_odin_map_cell_index(value_ptr, &value_cell_info, i);


            DF_Eval addr_key = zero_struct;
            addr_key.mode = EVAL_EvalMode_Addr;
            addr_key.offset = offset_key;
            addr_key.type_key = key;

            DF_Eval addr_value = zero_struct;
            addr_value.mode = EVAL_EvalMode_Addr;
            addr_value.offset = offset_value;
            addr_value.type_key = value;

            // render element
          }
        }

        TG_Key key_array_type = tg_cons_type_make(parse_ctx->type_graph, TG_Kind_Array, key_cell, cap/key_cell_info.elements_per_cell);
        DF_Eval new_eval = zero_struct;
        new_eval.mode = EVAL_EvalMode_Value;
        new_eval.imm_u64 = key_ptr;
        new_eval.type_key = tg_cons_type_make(parse_ctx->type_graph, TG_Kind_Ptr, key_array_type, 0);
        eval = new_eval;

      }
    }
  }

  return eval;
}


////////////////////////////////
//~ rjf: "list"

DF_CORE_VIEW_RULE_VIZ_BLOCK_PROD_FUNCTION_DEF(list)
{
  
}

DF_GFX_VIEW_RULE_VIZ_ROW_PROD_FUNCTION_DEF(list)
{
  
}

////////////////////////////////
//~ rjf: "bswap"

DF_CORE_VIEW_RULE_EVAL_RESOLUTION_FUNCTION_DEF(bswap)
{
  Temp scratch = scratch_begin(&arena, 1);
  TG_Key type_key = eval.type_key;
  TG_Kind type_kind = tg_kind_from_key(type_key);
  U64 type_size_bytes = tg_byte_size_from_graph_rdi_key(parse_ctx->type_graph, parse_ctx->rdi, type_key);
  if(TG_Kind_Char8 <= type_kind && type_kind <= TG_Kind_S256 &&
     (type_size_bytes == 2 ||
      type_size_bytes == 4 ||
      type_size_bytes == 8))
  {
    DF_Eval value_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, eval);
    if(value_eval.mode == EVAL_EvalMode_Value)
    {
      switch(type_size_bytes)
      {
        default:{}break;
        case 2:{U16 v = (U16)value_eval.imm_u64; v = bswap_u16(v); value_eval.imm_u64 = (U64)v;}break;
        case 4:{U32 v = (U32)value_eval.imm_u64; v = bswap_u32(v); value_eval.imm_u64 = (U64)v;}break;
        case 8:{U64 v =      value_eval.imm_u64; v = bswap_u64(v); value_eval.imm_u64 =      v;}break;
      }
    }
    eval = value_eval;
  }
  scratch_end(scratch);
  return eval;
}

////////////////////////////////
//~ rjf: "dec"

DF_GFX_VIEW_RULE_LINE_STRINGIZE_FUNCTION_DEF(dec)
{
  
}

////////////////////////////////
//~ rjf: "bin"

DF_GFX_VIEW_RULE_LINE_STRINGIZE_FUNCTION_DEF(bin)
{
  
}

////////////////////////////////
//~ rjf: "oct"

DF_GFX_VIEW_RULE_LINE_STRINGIZE_FUNCTION_DEF(oct)
{
  
}

////////////////////////////////
//~ rjf: "hex"

DF_GFX_VIEW_RULE_LINE_STRINGIZE_FUNCTION_DEF(hex)
{
  
}

////////////////////////////////
//~ rjf: "only"

DF_CORE_VIEW_RULE_VIZ_BLOCK_PROD_FUNCTION_DEF(only)
{
  
}

DF_GFX_VIEW_RULE_VIZ_ROW_PROD_FUNCTION_DEF(only)
{
  
}

DF_GFX_VIEW_RULE_LINE_STRINGIZE_FUNCTION_DEF(only)
{
  
}

////////////////////////////////
//~ rjf: "omit"

DF_CORE_VIEW_RULE_VIZ_BLOCK_PROD_FUNCTION_DEF(omit)
{
  
}

DF_GFX_VIEW_RULE_VIZ_ROW_PROD_FUNCTION_DEF(omit)
{
  
}

DF_GFX_VIEW_RULE_LINE_STRINGIZE_FUNCTION_DEF(omit)
{
  
}

////////////////////////////////
//~ rjf: "no_addr"

DF_GFX_VIEW_RULE_LINE_STRINGIZE_FUNCTION_DEF(no_addr)
{
}

////////////////////////////////
//~ rjf: "rgba"

typedef struct DF_ViewRuleHooks_RGBAState DF_ViewRuleHooks_RGBAState;
struct DF_ViewRuleHooks_RGBAState
{
  Vec4F32 hsva;
  U64 memgen_idx;
};

DF_CORE_VIEW_RULE_VIZ_BLOCK_PROD_FUNCTION_DEF(rgba)
{
  DF_EvalVizBlock *vb = df_eval_viz_block_begin(arena, DF_EvalVizBlockKind_Canvas, key, df_expand_key_make(df_hash_from_expand_key(key), 1), depth);
  vb->eval = eval;
  vb->cfg_table = *cfg_table;
  vb->visual_idx_range = r1u64(0, 8);
  vb->semantic_idx_range = r1u64(0, 1);
  df_eval_viz_block_end(out, vb);
}

DF_GFX_VIEW_RULE_ROW_UI_FUNCTION_DEF(rgba)
{
  Temp scratch = scratch_begin(0, 0);
  DF_Entity *thread = df_entity_from_handle(ctrl_ctx->thread);
  DF_Entity *process = df_entity_ancestor_from_kind(thread, DF_EntityKind_Process);
  
  //- rjf: grab hsva
  DF_Eval value_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, eval);
  Vec4F32 rgba = df_view_rule_hooks__rgba_from_eval(value_eval, parse_ctx->type_graph, parse_ctx->rdi, process);
  Vec4F32 hsva = hsva_from_rgba(rgba);
  
  //- rjf: build text box
  UI_Box *text_box = &ui_g_nil_box;
  UI_WidthFill UI_Font(df_font_from_slot(DF_FontSlot_Code))
  {
    text_box = ui_build_box_from_key(UI_BoxFlag_DrawText, ui_key_zero());
    D_FancyStringList fancy_strings = {0};
    {
      D_FancyString open_paren = {ui_top_font(), str8_lit("("), ui_top_text_color(), ui_top_font_size(), 0, 0};
      D_FancyString comma = {ui_top_font(), str8_lit(", "), ui_top_text_color(), ui_top_font_size(), 0, 0};
      D_FancyString r_fstr = {ui_top_font(), push_str8f(scratch.arena, "%.2f", rgba.x), v4f32(1.f, 0.25f, 0.25f, 1.f), ui_top_font_size(), 4.f, 0};
      D_FancyString g_fstr = {ui_top_font(), push_str8f(scratch.arena, "%.2f", rgba.y), v4f32(0.25f, 1.f, 0.25f, 1.f), ui_top_font_size(), 4.f, 0};
      D_FancyString b_fstr = {ui_top_font(), push_str8f(scratch.arena, "%.2f", rgba.z), v4f32(0.25f, 0.25f, 1.f, 1.f), ui_top_font_size(), 4.f, 0};
      D_FancyString a_fstr = {ui_top_font(), push_str8f(scratch.arena, "%.2f", rgba.w), v4f32(1.f,   1.f,   1.f, 1.f), ui_top_font_size(), 4.f, 0};
      D_FancyString clse_paren = {ui_top_font(), str8_lit(")"), ui_top_text_color(), ui_top_font_size(), 0, 0};
      d_fancy_string_list_push(scratch.arena, &fancy_strings, &open_paren);
      d_fancy_string_list_push(scratch.arena, &fancy_strings, &r_fstr);
      d_fancy_string_list_push(scratch.arena, &fancy_strings, &comma);
      d_fancy_string_list_push(scratch.arena, &fancy_strings, &g_fstr);
      d_fancy_string_list_push(scratch.arena, &fancy_strings, &comma);
      d_fancy_string_list_push(scratch.arena, &fancy_strings, &b_fstr);
      d_fancy_string_list_push(scratch.arena, &fancy_strings, &comma);
      d_fancy_string_list_push(scratch.arena, &fancy_strings, &a_fstr);
      d_fancy_string_list_push(scratch.arena, &fancy_strings, &clse_paren);
    }
    ui_box_equip_display_fancy_strings(text_box, &fancy_strings);
  }
  
  //- rjf: build color box
  UI_Box *color_box = &ui_g_nil_box;
  UI_PrefWidth(ui_em(1.875f, 1.f)) UI_ChildLayoutAxis(Axis2_Y)
  {
    color_box = ui_build_box_from_stringf(UI_BoxFlag_Clickable, "color_box");
    UI_Parent(color_box) UI_PrefHeight(ui_em(1.875f, 1.f)) UI_Padding(ui_pct(1, 0))
    {
      UI_BackgroundColor(rgba) UI_CornerRadius(ui_top_font_size()*0.5f)
        ui_build_box_from_key(UI_BoxFlag_DrawBackground|UI_BoxFlag_DrawBorder, ui_key_zero());
    }
  }
  
  //- rjf: space
  ui_spacer(ui_em(0.375f, 1.f));
  
  //- rjf: hover color box -> show components
  UI_Signal sig = ui_signal_from_box(color_box);
  if(ui_hovering(sig))
  {
    ui_do_color_tooltip_hsva(hsva);
  }
  
  scratch_end(scratch);
}

DF_GFX_VIEW_RULE_BLOCK_UI_FUNCTION_DEF(rgba)
{
  Temp scratch = scratch_begin(0, 0);
  DF_Entity *thread = df_entity_from_handle(ctrl_ctx->thread);
  DF_Entity *process = df_entity_ancestor_from_kind(thread, DF_EntityKind_Process);
  DF_ViewRuleHooks_RGBAState *state = df_view_rule_block_user_state(key, DF_ViewRuleHooks_RGBAState);
  
  //- rjf: grab hsva
  Vec4F32 rgba = {0};
  Vec4F32 hsva = {0};
  {
    if(state->memgen_idx >= ctrl_mem_gen())
    {
      hsva = state->hsva;
      rgba = rgba_from_hsva(hsva);
    }
    else
    {
      DF_Eval value_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, eval);
      rgba = df_view_rule_hooks__rgba_from_eval(value_eval, parse_ctx->type_graph, parse_ctx->rdi, process);
      state->hsva = hsva = hsva_from_rgba(rgba);
      state->memgen_idx = ctrl_mem_gen();
    }
  }
  Vec4F32 initial_hsva = hsva;
  
  //- rjf: build color picker
  B32 commit = 0;
  UI_Padding(ui_pct(1.f, 0.f))
  {
    UI_PrefWidth(ui_px(dim.y, 1.f))
    {
      UI_Signal sv_sig = ui_sat_val_pickerf(hsva.x, &hsva.y, &hsva.z, "sat_val_picker");
      commit = commit || ui_released(sv_sig);
    }
    UI_PrefWidth(ui_em(3.f, 1.f))
    {
      UI_Signal h_sig  = ui_hue_pickerf(&hsva.x, hsva.y, hsva.z, "hue_picker");
      commit = commit || ui_released(h_sig);
    }
    UI_PrefWidth(ui_children_sum(1)) UI_Column UI_PrefWidth(ui_text_dim(10, 1)) UI_Font(df_font_from_slot(DF_FontSlot_Code)) UI_TextColor(df_rgba_from_theme_color(DF_ThemeColor_WeakText))
    {
      ui_labelf("Hex");
      ui_labelf("R");
      ui_labelf("G");
      ui_labelf("B");
      ui_labelf("H");
      ui_labelf("S");
      ui_labelf("V");
      ui_labelf("A");
    }
    UI_PrefWidth(ui_children_sum(1)) UI_Column UI_PrefWidth(ui_text_dim(10, 1)) UI_Font(df_font_from_slot(DF_FontSlot_Code))
    {
      String8 hex_string = hex_string_from_rgba_4f32(scratch.arena, rgba);
      ui_label(hex_string);
      ui_labelf("%.2f", rgba.x);
      ui_labelf("%.2f", rgba.y);
      ui_labelf("%.2f", rgba.z);
      ui_labelf("%.2f", hsva.x);
      ui_labelf("%.2f", hsva.y);
      ui_labelf("%.2f", hsva.z);
      ui_labelf("%.2f", rgba.w);
    }
  }
  
  //- rjf: commit edited hsva back
  if(commit)
  {
    Vec4F32 rgba = rgba_from_hsva(hsva);
    df_view_rule_hooks__eval_commit_rgba(eval, parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, rgba);
    state->memgen_idx = ctrl_mem_gen();
  }
  
  //- rjf: commit possible edited value to state
  state->hsva = hsva;
  
  scratch_end(scratch);
}

////////////////////////////////
//~ rjf: "text"

typedef struct DF_ViewRuleHooks_TextState DF_ViewRuleHooks_TextState;
struct DF_ViewRuleHooks_TextState
{
  B32 initialized;
  TxtPt cursor;
  TxtPt mark;
  S64 preferred_column;
  U64 last_open_frame_idx;
  F32 loaded_t;
};

DF_CORE_VIEW_RULE_VIZ_BLOCK_PROD_FUNCTION_DEF(text)
{
  DF_EvalVizBlock *vb = df_eval_viz_block_begin(arena, DF_EvalVizBlockKind_Canvas, key, df_expand_key_make(df_hash_from_expand_key(key), 1), depth);
  vb->eval = eval;
  vb->cfg_table = *cfg_table;
  vb->visual_idx_range = r1u64(0, 8);
  vb->semantic_idx_range = r1u64(0, 1);
  df_eval_viz_block_end(out, vb);
}

DF_GFX_VIEW_RULE_BLOCK_UI_FUNCTION_DEF(text)
{
  Temp scratch = scratch_begin(0, 0);
  HS_Scope *hs_scope = hs_scope_open();
  TXT_Scope *txt_scope = txt_scope_open();
  
  //////////////////////////////
  //- rjf: get & initialize state
  //
  DF_ViewRuleHooks_TextState *state = df_view_rule_block_user_state(key, DF_ViewRuleHooks_TextState);
  if(!state->initialized)
  {
    state->initialized = 1;
    state->cursor = state->mark = txt_pt(1, 1);
  }
  
  //////////////////////////////
  //- rjf: unpack evaluation / view rule params
  //
  DF_Entity *thread = df_entity_from_handle(ctrl_ctx->thread);
  DF_Entity *process = df_entity_ancestor_from_kind(thread, DF_EntityKind_Process);
  DF_TxtTopologyInfo top = df_view_rule_hooks__txt_topology_info_from_cfg(dbgi_scope, ctrl_ctx, parse_ctx, macro_map, cfg);
  DF_Eval value_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, eval);
  U64 base_vaddr = value_eval.imm_u64 ? value_eval.imm_u64 : value_eval.offset;
  Rng1U64 vaddr_range = r1u64(base_vaddr, base_vaddr + (top.size_cap ? top.size_cap : 2048));
  
  //////////////////////////////
  //- rjf: evaluation info -> text visualization info
  //
  String8 data = {0};
  TXT_TextInfo info = {0};
  TXT_LineTokensSlice line_tokens_slice = {0};
  {
    U128 text_hash = {0};
    U128 text_key = ctrl_hash_store_key_from_process_vaddr_range(process->ctrl_machine_id, process->ctrl_handle, vaddr_range, 1);
    info = txt_text_info_from_key_lang(txt_scope, text_key, top.lang, &text_hash);
    data = hs_data_from_hash(hs_scope, text_hash);
    line_tokens_slice = txt_line_tokens_slice_from_info_data_line_range(scratch.arena, &info, data, r1s64(1, info.lines_count));
  }
  
  //////////////////////////////
  //- rjf: info -> code slice info
  //
  DF_CodeSliceParams code_slice_params = {0};
  {
    code_slice_params.flags = DF_CodeSliceFlag_LineNums;
    code_slice_params.line_num_range = r1s64(1, info.lines_count);
    code_slice_params.line_text = push_array(scratch.arena, String8, info.lines_count);
    code_slice_params.line_ranges = push_array(scratch.arena, Rng1U64, info.lines_count);
    code_slice_params.line_tokens = push_array(scratch.arena, TXT_TokenArray, info.lines_count);
    code_slice_params.line_bps = push_array(scratch.arena, DF_EntityList, info.lines_count);
    code_slice_params.line_ips = push_array(scratch.arena, DF_EntityList, info.lines_count);
    code_slice_params.line_pins = push_array(scratch.arena, DF_EntityList, info.lines_count);
    code_slice_params.line_dasm2src = push_array(scratch.arena, DF_TextLineDasm2SrcInfoList, info.lines_count);
    code_slice_params.line_src2dasm = push_array(scratch.arena, DF_TextLineSrc2DasmInfoList, info.lines_count);
    for(U64 line_idx = 0; line_idx < info.lines_count; line_idx += 1)
    {
      code_slice_params.line_text[line_idx] = str8_substr(data, info.lines_ranges[line_idx]);
      code_slice_params.line_ranges[line_idx] = info.lines_ranges[line_idx];
      code_slice_params.line_tokens[line_idx] = line_tokens_slice.line_tokens[line_idx];
    }
    code_slice_params.font = df_font_from_slot(DF_FontSlot_Code);
    code_slice_params.font_size = ui_top_font_size();
    code_slice_params.line_height_px = ui_top_font_size()*1.5f;
    code_slice_params.margin_width_px = 0;
    code_slice_params.line_num_width_px = ui_top_font_size()*5.f;
    code_slice_params.line_text_max_width_px = ui_top_font_size()*2.f*info.lines_max_size;
  }
  
  //////////////////////////////
  //- rjf: build UI
  //
  if(info.lines_count != 0)
  {
    //- rjf: build top-level container
    UI_Box *container = &ui_g_nil_box;
    UI_PrefWidth(ui_px(dim.x, 1.f)) UI_PrefHeight(ui_px(dim.y, 1.f))
    {
      container = ui_build_box_from_stringf(UI_BoxFlag_AllowOverflow|UI_BoxFlag_Clip, "###text_container");
    }
    
    //- rjf: build code slice
    UI_WidthFill UI_HeightFill UI_Parent(container)
    {
      DF_CodeSliceSignal slice_sig = df_code_slice(ws, ctrl_ctx, parse_ctx, &code_slice_params, &state->cursor, &state->mark, &state->preferred_column, str8_lit("###slice"));
    }
  }
  
  txt_scope_close(txt_scope);
  hs_scope_close(hs_scope);
  scratch_end(scratch);
}

////////////////////////////////
//~ rjf: "disasm"

typedef struct DF_ViewRuleHooks_DisasmState DF_ViewRuleHooks_DisasmState;
struct DF_ViewRuleHooks_DisasmState
{
  B32 initialized;
  TxtPt cursor;
  TxtPt mark;
  S64 preferred_column;
  U64 last_open_frame_idx;
  F32 loaded_t;
};

DF_CORE_VIEW_RULE_VIZ_BLOCK_PROD_FUNCTION_DEF(disasm)
{
  DF_EvalVizBlock *vb = df_eval_viz_block_begin(arena, DF_EvalVizBlockKind_Canvas, key, df_expand_key_make(df_hash_from_expand_key(key), 1), depth);
  vb->eval = eval;
  vb->cfg_table = *cfg_table;
  vb->visual_idx_range = r1u64(0, 8);
  vb->semantic_idx_range = r1u64(0, 1);
  df_eval_viz_block_end(out, vb);
}

DF_GFX_VIEW_RULE_BLOCK_UI_FUNCTION_DEF(disasm)
{
  Temp scratch = scratch_begin(0, 0);
  HS_Scope *hs_scope = hs_scope_open();
  TXT_Scope *txt_scope = txt_scope_open();
  DASM_Scope *dasm_scope = dasm_scope_open();
  DF_ViewRuleHooks_DisasmState *state = df_view_rule_block_user_state(key, DF_ViewRuleHooks_DisasmState);
  if(!state->initialized)
  {
    state->initialized = 1;
    state->cursor = state->mark = txt_pt(1, 1);
  }
  if(state->last_open_frame_idx+1 < df_frame_index())
  {
    state->loaded_t = 0;
  }
  state->last_open_frame_idx = df_frame_index();
  {
    //- rjf: unpack params
    DF_DisasmTopologyInfo top = df_view_rule_hooks__disasm_topology_info_from_cfg(dbgi_scope, ctrl_ctx, parse_ctx, macro_map, cfg);
    
    //- rjf: resolve to address value & range
    DF_Eval value_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, eval);
    U64 base_vaddr = value_eval.imm_u64 ? value_eval.imm_u64 : value_eval.offset;
    Rng1U64 vaddr_range = r1u64(base_vaddr, base_vaddr + (top.size_cap ? top.size_cap : 2048));
    
    //- rjf: unpack thread/process of eval
    DF_Entity *thread = df_entity_from_handle(ctrl_ctx->thread);
    DF_Entity *process = df_entity_ancestor_from_kind(thread, DF_EntityKind_Process);
    
    //- rjf: unpack key for this region in memory
    U128 dasm_key = ctrl_hash_store_key_from_process_vaddr_range(process->ctrl_machine_id, process->ctrl_handle, vaddr_range, 0);
    
    //- rjf: key -> parsed text info
    U128 data_hash = {0};
    DASM_Info dasm_info = dasm_info_from_key_addr_arch_style(dasm_scope, dasm_key, vaddr_range.min, top.arch, 0, DASM_Syntax_Intel, &data_hash);
    String8 dasm_text_data = {0};
    TXT_TextInfo dasm_text_info = {0};
    for(U64 rewind_idx = 0; rewind_idx < 2; rewind_idx += 1)
    {
      U128 dasm_text_hash = hs_hash_from_key(dasm_info.text_key, rewind_idx);
      dasm_text_data = hs_data_from_hash(hs_scope, dasm_text_hash);
      dasm_text_info = txt_text_info_from_hash_lang(txt_scope, dasm_text_hash, TXT_LangKind_DisasmX64Intel);
      if(dasm_text_info.lines_count != 0)
      {
        break;
      }
    }
    TXT_LineTokensSlice line_tokens_slice = txt_line_tokens_slice_from_info_data_line_range(scratch.arena, &dasm_text_info, dasm_text_data, r1s64(1, dasm_info.insts.count));
    
    //- rjf: info -> code slice info
    DF_CodeSliceParams code_slice_params = {0};
    {
      code_slice_params.flags = DF_CodeSliceFlag_LineNums;
      code_slice_params.line_num_range = r1s64(1, dasm_text_info.lines_count);
      code_slice_params.line_text = push_array(scratch.arena, String8, dasm_text_info.lines_count);
      code_slice_params.line_ranges = push_array(scratch.arena, Rng1U64, dasm_text_info.lines_count);
      code_slice_params.line_tokens = push_array(scratch.arena, TXT_TokenArray, dasm_text_info.lines_count);
      code_slice_params.line_bps = push_array(scratch.arena, DF_EntityList, dasm_text_info.lines_count);
      code_slice_params.line_ips = push_array(scratch.arena, DF_EntityList, dasm_text_info.lines_count);
      code_slice_params.line_pins = push_array(scratch.arena, DF_EntityList, dasm_text_info.lines_count);
      code_slice_params.line_dasm2src = push_array(scratch.arena, DF_TextLineDasm2SrcInfoList, dasm_text_info.lines_count);
      code_slice_params.line_src2dasm = push_array(scratch.arena, DF_TextLineSrc2DasmInfoList, dasm_text_info.lines_count);
      for(U64 line_idx = 0; line_idx < dasm_text_info.lines_count; line_idx += 1)
      {
        code_slice_params.line_text[line_idx] = str8_substr(dasm_text_data, dasm_info.insts.v[line_idx].text_range);
        code_slice_params.line_ranges[line_idx] = dasm_info.insts.v[line_idx].text_range;
        code_slice_params.line_tokens[line_idx] = line_tokens_slice.line_tokens[line_idx];
      }
      code_slice_params.font = df_font_from_slot(DF_FontSlot_Code);
      code_slice_params.font_size = ui_top_font_size();
      code_slice_params.line_height_px = ui_top_font_size()*1.5f;
      code_slice_params.margin_width_px = 0;
      code_slice_params.line_num_width_px = ui_top_font_size()*5.f;
      code_slice_params.line_text_max_width_px = ui_top_font_size()*2.f*dasm_text_info.lines_max_size;
    }
    
    //- rjf: build code slice
    if(dasm_info.insts.count != 0 && dasm_text_info.lines_count != 0)
      UI_Padding(ui_pct(1, 0)) UI_PrefWidth(ui_px(dasm_text_info.lines_max_size*ui_top_font_size()*1.2f, 1.f)) UI_Column UI_Padding(ui_pct(1, 0))
    {
      DF_CodeSliceSignal sig = df_code_slice(ws, ctrl_ctx, parse_ctx, &code_slice_params, &state->cursor, &state->mark, &state->preferred_column, str8_lit("###code_slice"));
    }
  }
  dasm_scope_close(dasm_scope);
  txt_scope_close(txt_scope);
  hs_scope_close(hs_scope);
  scratch_end(scratch);
}

////////////////////////////////
//~ rjf: "bitmap"

typedef struct DF_ViewRuleHooks_BitmapState DF_ViewRuleHooks_BitmapState;
struct DF_ViewRuleHooks_BitmapState
{
  U64 last_open_frame_idx;
  F32 loaded_t;
};

typedef struct DF_ViewRuleHooks_BitmapBoxDrawData DF_ViewRuleHooks_BitmapBoxDrawData;
struct DF_ViewRuleHooks_BitmapBoxDrawData
{
  Rng2F32 src;
  R_Handle texture;
  F32 loaded_t;
  B32 hovered;
  Vec2S32 mouse_px;
  F32 ui_per_bmp_px;
};

typedef struct DF_ViewRuleHooks_BitmapZoomDrawData DF_ViewRuleHooks_BitmapZoomDrawData;
struct DF_ViewRuleHooks_BitmapZoomDrawData
{
  Rng2F32 src;
  R_Handle texture;
};

internal UI_BOX_CUSTOM_DRAW(df_view_rule_hooks__bitmap_box_draw)
{
  DF_ViewRuleHooks_BitmapBoxDrawData *draw_data = (DF_ViewRuleHooks_BitmapBoxDrawData *)user_data;
  Vec4F32 bg_color = box->background_color;
  d_img(box->rect, draw_data->src, draw_data->texture, v4f32(1, 1, 1, 1), 0, 0, 0);
  if(draw_data->loaded_t < 0.98f)
  {
    Rng2F32 clip = box->rect;
    for(UI_Box *b = box->parent; !ui_box_is_nil(b); b = b->parent)
    {
      if(b->flags & UI_BoxFlag_Clip)
      {
        clip = intersect_2f32(b->rect, clip);
      }
    }
    d_blur(intersect_2f32(clip, box->rect), 10.f-9.f*draw_data->loaded_t, 0);
  }
  if(r_handle_match(draw_data->texture, r_handle_zero()))
  {
    d_rect(box->rect, v4f32(0, 0, 0, 1), 0, 0, 0);
  }
  d_rect(box->rect, v4f32(bg_color.x*bg_color.w, bg_color.y*bg_color.w, bg_color.z*bg_color.w, 1.f-draw_data->loaded_t), 0, 0, 0);
  if(draw_data->hovered)
  {
    Vec4F32 indicator_color = df_rgba_from_theme_color(DF_ThemeColor_PlainBorder);
    indicator_color.w = 1.f;
    d_rect(pad_2f32(r2f32p(box->rect.x0 + draw_data->mouse_px.x*draw_data->ui_per_bmp_px,
                           box->rect.y0 + draw_data->mouse_px.y*draw_data->ui_per_bmp_px,
                           box->rect.x0 + draw_data->mouse_px.x*draw_data->ui_per_bmp_px + draw_data->ui_per_bmp_px,
                           box->rect.y0 + draw_data->mouse_px.y*draw_data->ui_per_bmp_px + draw_data->ui_per_bmp_px),
                    3.f),
           indicator_color, 3.f, 4.f, 1.f);
  }
}

internal UI_BOX_CUSTOM_DRAW(df_view_rule_hooks__bitmap_zoom_draw)
{
  DF_ViewRuleHooks_BitmapZoomDrawData *draw_data = (DF_ViewRuleHooks_BitmapZoomDrawData *)user_data;
  d_img(box->rect, draw_data->src, draw_data->texture, v4f32(1, 1, 1, 1), 0, 0, 0);
}

DF_CORE_VIEW_RULE_VIZ_BLOCK_PROD_FUNCTION_DEF(bitmap)
{
  DF_EvalVizBlock *vb = df_eval_viz_block_begin(arena, DF_EvalVizBlockKind_Canvas, key, df_expand_key_make(df_hash_from_expand_key(key), 1), depth);
  vb->eval = eval;
  vb->cfg_table = *cfg_table;
  vb->visual_idx_range = r1u64(0, 8);
  vb->semantic_idx_range = r1u64(0, 1);
  df_eval_viz_block_end(out, vb);
}

DF_GFX_VIEW_RULE_ROW_UI_FUNCTION_DEF(bitmap)
{
  DF_Eval value_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, eval);
  U64 base_vaddr = value_eval.imm_u64 ? value_eval.imm_u64 : value_eval.offset;
  DF_BitmapTopologyInfo topology = df_view_rule_hooks__bitmap_topology_info_from_cfg(scope, ctrl_ctx, parse_ctx, macro_map, cfg);
  U64 expected_size = topology.width*topology.height*r_tex2d_format_bytes_per_pixel_table[topology.fmt];
  UI_Font(df_font_from_slot(DF_FontSlot_Code)) UI_TextColor(df_rgba_from_theme_color(DF_ThemeColor_WeakText))
    ui_labelf("0x%I64x -> Bitmap (%I64u x %I64u)", base_vaddr, topology.width, topology.height);
}

DF_GFX_VIEW_RULE_BLOCK_UI_FUNCTION_DEF(bitmap)
{
  Temp scratch = scratch_begin(0, 0);
  HS_Scope *hs_scope = hs_scope_open();
  TEX_Scope *tex_scope = tex_scope_open();
  DF_ViewRuleHooks_BitmapState *state = df_view_rule_block_user_state(key, DF_ViewRuleHooks_BitmapState);
  if(state->last_open_frame_idx+1 < df_frame_index())
  {
    state->loaded_t = 0;
  }
  state->last_open_frame_idx = df_frame_index();
  
  //- rjf: resolve to address value
  DF_Eval value_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, eval);
  U64 base_vaddr = value_eval.imm_u64 ? value_eval.imm_u64 : value_eval.offset;
  
  //- rjf: unpack thread/process of eval
  DF_Entity *thread = df_entity_from_handle(ctrl_ctx->thread);
  DF_Entity *process = df_entity_ancestor_from_kind(thread, DF_EntityKind_Process);
  
  //- rjf: unpack image dimensions & form vaddr range
  DF_BitmapTopologyInfo topology_info = df_view_rule_hooks__bitmap_topology_info_from_cfg(dbgi_scope, ctrl_ctx, parse_ctx, macro_map, cfg);
  U64 expected_size = topology_info.width*topology_info.height*r_tex2d_format_bytes_per_pixel_table[topology_info.fmt];
  Rng1U64 vaddr_range = r1u64(base_vaddr, base_vaddr+expected_size);
  
  //- rjf: obtain key for this data range
  U128 texture_key = ctrl_hash_store_key_from_process_vaddr_range(process->ctrl_machine_id, process->ctrl_handle, vaddr_range, 0);
  
  //- rjf: hash & topology -> texture
  TEX_Topology topology = tex_topology_make(v2s32((S32)topology_info.width, (S32)topology_info.height), topology_info.fmt);
  R_Handle texture = tex_texture_from_key_topology(tex_scope, texture_key, topology);
  
  //- rjf: build preview
  F32 rate = 1 - pow_f32(2, (-15.f * df_dt()));
  if(expected_size != 0)
  {
    F32 img_dim = dim.y - ui_top_font_size()*2.f;
    UI_Padding(ui_pct(1.f, 0.f))
      UI_PrefWidth(ui_px(img_dim*((F32)topology_info.width/(F32)topology_info.height), 1.f))
      UI_Column UI_Padding(ui_pct(1.f, 0.f))
      UI_PrefHeight(ui_px(img_dim, 1.f))
    {
      ui_set_next_hover_cursor(OS_Cursor_HandPoint);
      UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_Clickable|UI_BoxFlag_DrawHotEffects, "image_box");
      UI_Signal sig = ui_signal_from_box(box);
      F32 ui_per_bmp_px = dim.y / (F32)topology_info.height;
      Vec2F32 mouse_ui_px_off     = sub_2f32(ui_mouse(), box->rect.p0);
      Vec2S32 mouse_bitmap_px_off = v2s32(floor_f32(mouse_ui_px_off.x/ui_per_bmp_px), floor_f32(mouse_ui_px_off.y/ui_per_bmp_px));
      DF_ViewRuleHooks_BitmapBoxDrawData *draw_data = push_array(ui_build_arena(), DF_ViewRuleHooks_BitmapBoxDrawData, 1);
      draw_data->texture = texture;
      draw_data->src = r2f32(v2f32(0, 0), v2f32((F32)topology_info.width, (F32)topology_info.height));
      draw_data->loaded_t = state->loaded_t;
      draw_data->hovered = ui_hovering(sig);
      draw_data->mouse_px = mouse_bitmap_px_off;
      draw_data->ui_per_bmp_px = ui_per_bmp_px;
      ui_box_equip_custom_draw(box, df_view_rule_hooks__bitmap_box_draw, draw_data);
      if(r_handle_match(r_handle_zero(), texture))
      {
        df_gfx_request_frame();
        state->loaded_t = 0;
      }
      else
      {
        state->loaded_t += (1.f - state->loaded_t) * rate;
        if(state->loaded_t < 0.99f)
        {
          df_gfx_request_frame();
        }
      }
      if(ui_hovering(sig) && r_handle_match(texture, r_handle_zero())) UI_Tooltip
      {
        ui_labelf("Texture not loaded.");
      }
      if(ui_hovering(sig) && !r_handle_match(texture, r_handle_zero()))
      {
        if(dim.y > (F32)topology_info.height)
        {
          U128 hash = hs_hash_from_key(texture_key, 0);
          String8 data = hs_data_from_hash(hs_scope, hash);
          U64 bytes_per_pixel = r_tex2d_format_bytes_per_pixel_table[topology.fmt];
          U64 mouse_pixel_off = mouse_bitmap_px_off.y*topology_info.width + mouse_bitmap_px_off.x;
          U64 mouse_byte_off = mouse_pixel_off * bytes_per_pixel;
          B32 got_color = 0;
          Vec4F32 hsva = {0};
          if(mouse_byte_off + bytes_per_pixel <= data.size)
          {
            got_color = 1;
            switch(topology.fmt)
            {
              default:{got_color = 0;}break;
              case R_Tex2DFormat_RGBA8:
              {
                U8 r = data.str[mouse_byte_off+0];
                U8 g = data.str[mouse_byte_off+1];
                U8 b = data.str[mouse_byte_off+2];
                U8 a = data.str[mouse_byte_off+3];
                Vec4F32 rgba = v4f32(r/255.f, g/255.f, b/255.f, a/255.f);
                hsva = hsva_from_rgba(rgba);
              }break;
              case R_Tex2DFormat_BGRA8:
              {
                U8 r = data.str[mouse_byte_off+2];
                U8 g = data.str[mouse_byte_off+1];
                U8 b = data.str[mouse_byte_off+0];
                U8 a = data.str[mouse_byte_off+3];
                Vec4F32 rgba = v4f32(r/255.f, g/255.f, b/255.f, a/255.f);
                hsva = hsva_from_rgba(rgba);
              }break;
              case R_Tex2DFormat_R8:
              {
                U8 r = data.str[mouse_byte_off+0];
                Vec4F32 rgba = v4f32(r/255.f, 0, 0, 1.f);
                hsva = hsva_from_rgba(rgba);
              }break;
            }
          }
          if(got_color)
          {
            ui_do_color_tooltip_hsva(hsva);
          }
        }
        else UI_Tooltip UI_Font(df_font_from_slot(DF_FontSlot_Code))
        {
          ui_label(r_tex2d_format_display_string_table[topology.fmt]);
          ui_labelf("%I64u x %I64u", topology_info.width, topology_info.height);
          UI_Padding(ui_em(2.f, 1.f))
            UI_PrefWidth(ui_children_sum(1.f))
            UI_PrefHeight(ui_children_sum(1.f))
            UI_Row
            UI_PrefWidth(ui_em(15.f, 1.f))
            UI_PrefHeight(ui_em(15.f, 1.f))
            UI_Padding(ui_em(2.f, 1.f))
          {
            UI_Box *zoom_box = ui_build_box_from_key(UI_BoxFlag_DrawBorder, ui_key_zero());
            DF_ViewRuleHooks_BitmapZoomDrawData *draw_data = push_array(ui_build_arena(), DF_ViewRuleHooks_BitmapZoomDrawData, 1);
            draw_data->src = r2f32p(mouse_bitmap_px_off.x - 16, mouse_bitmap_px_off.y - 16, mouse_bitmap_px_off.x + 16, mouse_bitmap_px_off.y + 16);
            draw_data->texture = texture;
            ui_box_equip_custom_draw(zoom_box, df_view_rule_hooks__bitmap_zoom_draw, draw_data);
          }
        }
      }
    }
  }
  
  tex_scope_close(tex_scope);
  hs_scope_close(hs_scope);
  scratch_end(scratch);
}

DF_GFX_VIEW_RULE_WHOLE_UI_FUNCTION_DEF(bitmap)
{
  
}

////////////////////////////////
//~ rjf: "geo"

typedef struct DF_ViewRuleHooks_GeoState DF_ViewRuleHooks_GeoState;
struct DF_ViewRuleHooks_GeoState
{
  B32 initialized;
  U64 last_open_frame_idx;
  F32 loaded_t;
  F32 pitch;
  F32 pitch_target;
  F32 yaw;
  F32 yaw_target;
  F32 zoom;
  F32 zoom_target;
};

typedef struct DF_ViewRuleHooks_GeoBoxDrawData DF_ViewRuleHooks_GeoBoxDrawData;
struct DF_ViewRuleHooks_GeoBoxDrawData
{
  DF_ExpandKey key;
  R_Handle vertex_buffer;
  R_Handle index_buffer;
  F32 loaded_t;
};

internal UI_BOX_CUSTOM_DRAW(df_view_rule_hooks__geo_box_draw)
{
  DF_ViewRuleHooks_GeoBoxDrawData *draw_data = (DF_ViewRuleHooks_GeoBoxDrawData *)user_data;
  DF_ViewRuleHooks_GeoState *state = df_view_rule_block_user_state(draw_data->key, DF_ViewRuleHooks_GeoState);
  Vec4F32 bg_color = box->background_color;
  
  // rjf: get clip
  Rng2F32 clip = box->rect;
  for(UI_Box *b = box->parent; !ui_box_is_nil(b); b = b->parent)
  {
    if(b->flags & UI_BoxFlag_Clip)
    {
      clip = intersect_2f32(b->rect, clip);
    }
  }
  
  // rjf: calculate eye/target
  Vec3F32 target = {0};
  Vec3F32 eye = v3f32(state->zoom*cos_f32(state->yaw)*sin_f32(state->pitch),
                      state->zoom*sin_f32(state->yaw)*sin_f32(state->pitch),
                      state->zoom*cos_f32(state->pitch));
  
  // rjf: mesh
  Vec2F32 box_dim = dim_2f32(box->rect);
  R_PassParams_Geo3D *pass = d_geo3d_begin(box->rect,
                                           make_look_at_4x4f32(eye, target, v3f32(0, 0, 1)),
                                           make_perspective_4x4f32(0.25f, box_dim.x/box_dim.y, 0.1f, 500.f));
  pass->clip = clip;
  d_mesh(draw_data->vertex_buffer, draw_data->index_buffer, R_GeoTopologyKind_Triangles, R_GeoVertexFlag_TexCoord|R_GeoVertexFlag_Normals|R_GeoVertexFlag_RGB, r_handle_zero(), mat_4x4f32(1.f));
  
  // rjf: blur
  if(draw_data->loaded_t < 0.98f)
  {
    d_blur(intersect_2f32(clip, box->rect), 10.f-9.f*draw_data->loaded_t, 0);
  }
}

DF_CORE_VIEW_RULE_VIZ_BLOCK_PROD_FUNCTION_DEF(geo)
{
  DF_EvalVizBlock *vb = df_eval_viz_block_begin(arena, DF_EvalVizBlockKind_Canvas, key, df_expand_key_make(df_hash_from_expand_key(key), 1), depth);
  vb->eval = eval;
  vb->cfg_table = *cfg_table;
  vb->visual_idx_range = r1u64(0, 16);
  vb->semantic_idx_range = r1u64(0, 1);
  df_eval_viz_block_end(out, vb);
}

DF_GFX_VIEW_RULE_ROW_UI_FUNCTION_DEF(geo)
{
  DF_Eval value_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, eval);
  U64 base_vaddr = value_eval.imm_u64 ? value_eval.imm_u64 : value_eval.offset;
  UI_Font(df_font_from_slot(DF_FontSlot_Code)) UI_TextColor(df_rgba_from_theme_color(DF_ThemeColor_WeakText))
    ui_labelf("0x%I64x -> Geometry", base_vaddr);
}

DF_GFX_VIEW_RULE_BLOCK_UI_FUNCTION_DEF(geo)
{
  Temp scratch = scratch_begin(0, 0);
  GEO_Scope *geo_scope = geo_scope_open();
  DF_ViewRuleHooks_GeoState *state = df_view_rule_block_user_state(key, DF_ViewRuleHooks_GeoState);
  if(!state->initialized)
  {
    state->initialized = 1;
    state->zoom_target = 3.5f;
    state->yaw = state->yaw_target = -0.125f;
    state->pitch = state->pitch_target = -0.125f;
  }
  if(state->last_open_frame_idx+1 < df_frame_index())
  {
    state->loaded_t = 0;
  }
  state->last_open_frame_idx = df_frame_index();
  
  //- rjf: resolve to address value
  DF_Eval value_eval = df_value_mode_eval_from_eval(parse_ctx->type_graph, parse_ctx->rdi, ctrl_ctx, eval);
  U64 base_vaddr = value_eval.imm_u64 ? value_eval.imm_u64 : value_eval.offset;
  
  //- rjf: extract extra geo topology info from view rule
  DF_GeoTopologyInfo top = df_view_rule_hooks__geo_topology_info_from_cfg(dbgi_scope, ctrl_ctx, parse_ctx, macro_map, cfg);
  Rng1U64 index_buffer_vaddr_range = r1u64(base_vaddr, base_vaddr+top.index_count*sizeof(U32));
  Rng1U64 vertex_buffer_vaddr_range = top.vertices_vaddr_range;
  
  //- rjf: unpack thread/process of eval
  DF_Entity *thread = df_entity_from_handle(ctrl_ctx->thread);
  DF_Entity *process = df_entity_ancestor_from_kind(thread, DF_EntityKind_Process);
  
  //- rjf: obtain keys for index buffer & vertex buffer memory
  U128 index_buffer_key = ctrl_hash_store_key_from_process_vaddr_range(process->ctrl_machine_id, process->ctrl_handle, index_buffer_vaddr_range, 0);
  U128 vertex_buffer_key = ctrl_hash_store_key_from_process_vaddr_range(process->ctrl_machine_id, process->ctrl_handle, vertex_buffer_vaddr_range, 0);
  
  //- rjf: get gpu buffers
  R_Handle index_buffer = geo_buffer_from_key(geo_scope, index_buffer_key);
  R_Handle vertex_buffer = geo_buffer_from_key(geo_scope, vertex_buffer_key);
  
  //- rjf: build preview
  F32 rate = 1 - pow_f32(2, (-15.f * df_dt()));
  if(top.index_count != 0)
  {
    UI_Padding(ui_pct(1.f, 0.f))
      UI_PrefWidth(ui_px(dim.y, 1.f))
      UI_Column UI_Padding(ui_pct(1.f, 0.f))
      UI_PrefHeight(ui_px(dim.y, 1.f))
    {
      UI_Box *box = ui_build_box_from_stringf(UI_BoxFlag_DrawBorder|UI_BoxFlag_DrawBackground|UI_BoxFlag_Clickable, "geo_box");
      UI_Signal sig = ui_signal_from_box(box);
      if(ui_dragging(sig))
      {
        if(ui_pressed(sig))
        {
          Vec2F32 data = v2f32(state->yaw_target, state->pitch_target);
          ui_store_drag_struct(&data);
        }
        Vec2F32 drag_delta = ui_drag_delta();
        Vec2F32 drag_start_data = *ui_get_drag_struct(Vec2F32);
        state->yaw_target = drag_start_data.x + drag_delta.x/dim_2f32(box->rect).x;
        state->pitch_target = drag_start_data.y + drag_delta.y/dim_2f32(box->rect).y;
      }
      state->zoom += (state->zoom_target - state->zoom) * rate;
      state->yaw += (state->yaw_target - state->yaw) * rate;
      state->pitch += (state->pitch_target - state->pitch) * rate;
      if(abs_f32(state->zoom-state->zoom_target) > 0.001f ||
         abs_f32(state->yaw-state->yaw_target) > 0.001f ||
         abs_f32(state->pitch-state->pitch_target) > 0.001f)
      {
        df_gfx_request_frame();
      }
      DF_ViewRuleHooks_GeoBoxDrawData *draw_data = push_array(ui_build_arena(), DF_ViewRuleHooks_GeoBoxDrawData, 1);
      draw_data->key = key;
      draw_data->vertex_buffer = vertex_buffer;
      draw_data->index_buffer = index_buffer;
      draw_data->loaded_t = state->loaded_t;
      ui_box_equip_custom_draw(box, df_view_rule_hooks__geo_box_draw, draw_data);
      if(r_handle_match(r_handle_zero(), vertex_buffer))
      {
        df_gfx_request_frame();
        state->loaded_t = 0;
      }
      else
      {
        state->loaded_t += (1.f - state->loaded_t) * rate;
        if(state->loaded_t < 0.99f)
        {
          df_gfx_request_frame();
        }
      }
    }
  }
  
  geo_scope_close(geo_scope);
  scratch_end(scratch);
}
