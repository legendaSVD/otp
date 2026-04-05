#ifndef ASMJIT_CORE_RACFGBUILDER_P_H_INCLUDED
#define ASMJIT_CORE_RACFGBUILDER_P_H_INCLUDED
#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER
#include <asmjit/core/formatter.h>
#include <asmjit/core/racfgblock_p.h>
#include <asmjit/core/rainst_p.h>
#include <asmjit/core/rapass_p.h>
ASMJIT_BEGIN_NAMESPACE
template<typename This>
class RACFGBuilderT {
public:
  static inline constexpr uint32_t kRootIndentation = 2;
  static inline constexpr uint32_t kCodeIndentation = 4;
  static inline constexpr NodePosition kNodePositionUnassigned = NodePosition(0u);
  static inline constexpr NodePosition kNodePositionDidOnBefore = NodePosition(0xFFFFFFFFu);
  BaseRAPass& _pass;
  BaseCompiler& _cc;
  RABlock* _cur_block = nullptr;
  RABlock* _ret_block = nullptr;
  FuncNode* _func_node = nullptr;
  RARegsStats _block_reg_stats {};
  uint32_t _exit_label_id = Globals::kInvalidId;
  ArenaVector<uint32_t> _shared_assignments_map {};
  bool _has_code = false;
  RABlock* _last_logged_block = nullptr;
#ifndef ASMJIT_NO_LOGGING
  Logger* _logger = nullptr;
  FormatOptions _format_options {};
  StringTmp<512> _sb;
#endif
  inline RACFGBuilderT(BaseRAPass& pass) noexcept
    : _pass(pass),
      _cc(pass.cc()) {
#ifndef ASMJIT_NO_LOGGING
    _logger = _pass.has_diagnostic_option(DiagnosticOptions::kRADebugCFG) ? _pass.logger() : nullptr;
    if (_logger) {
      _format_options = _logger->options();
    }
#endif
  }
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseCompiler& cc() const noexcept { return _cc; }
  [[nodiscard]]
  Error run() noexcept {
    log("[build_cfg_nodes]\n");
    ASMJIT_PROPAGATE(prepare());
    log_node(_func_node, kRootIndentation);
    log_block(_cur_block, kRootIndentation);
    RABlock* entry_block = _cur_block;
    BaseNode* node = _func_node->next();
    if (ASMJIT_UNLIKELY(!node)) {
      return make_error(Error::kInvalidState);
    }
    _cur_block->set_first(_func_node);
    _cur_block->set_last(_func_node);
    RAInstBuilder ib;
    ArenaVector<RABlock*> blocks_with_unknown_jumps;
    for (;;) {
      BaseNode* next = node->next();
      ASMJIT_ASSERT(node->position() == kNodePositionUnassigned || node->position() == kNodePositionDidOnBefore);
      if (node->is_inst()) {
        if (ASMJIT_UNLIKELY(!_cur_block)) {
          remove_node(node);
          node = next;
          continue;
        }
        _has_code = true;
        if (node->is_invoke() || node->is_func_ret()) {
          if (node->position() != kNodePositionDidOnBefore) {
            BaseNode* prev = node->prev();
            if (node->type() == NodeType::kInvoke) {
              ASMJIT_PROPAGATE(static_cast<This*>(this)->on_before_invoke(node->as<InvokeNode>()));
            }
            else {
              ASMJIT_PROPAGATE(static_cast<This*>(this)->on_before_ret(node->as<FuncRetNode>()));
            }
            if (prev != node->prev()) {
              if (_cur_block->first() == node) {
                _cur_block->set_first(prev->next());
              }
              node->set_position(kNodePositionDidOnBefore);
              node = prev->next();
              ASMJIT_ASSERT(node->is_inst());
            }
            next = node->next();
          }
          else {
            node->set_position(kNodePositionUnassigned);
          }
        }
        InstNode* inst = node->as<InstNode>();
        log_node(inst, kCodeIndentation);
        InstControlFlow cf = InstControlFlow::kRegular;
        ib.reset(_cur_block->block_id());
        ASMJIT_PROPAGATE(static_cast<This*>(this)->on_instruction(inst, cf, ib));
        if (node->is_invoke()) {
          ASMJIT_PROPAGATE(static_cast<This*>(this)->on_invoke(inst->as<InvokeNode>(), ib));
        }
        if (node->is_func_ret()) {
          ASMJIT_PROPAGATE(static_cast<This*>(this)->on_ret(inst->as<FuncRetNode>(), ib));
          cf = InstControlFlow::kReturn;
        }
        if (cf == InstControlFlow::kJump) {
          uint32_t fixed_reg_count = 0;
          for (RATiedReg& tied_reg : ib) {
            RAWorkReg* work_reg = tied_reg.work_reg();
            if (work_reg->group() == RegGroup::kGp) {
              uint32_t use_id = tied_reg.use_id();
              if (use_id == Reg::kIdBad) {
                use_id = _pass._scratch_reg_indexes[fixed_reg_count++];
                tied_reg.set_use_id(use_id);
              }
              _cur_block->add_exit_scratch_gp_regs(Support::bit_mask<RegMask>(use_id));
            }
          }
        }
        ASMJIT_PROPAGATE(_pass.assign_ra_inst(inst, _cur_block, ib));
        _block_reg_stats.combine_with(ib._stats);
        if (cf != InstControlFlow::kRegular) {
          if (cf == InstControlFlow::kJump || cf == InstControlFlow::kBranch) {
            _cur_block->set_last(node);
            _cur_block->add_flags(RABlockFlags::kHasTerminator);
            _cur_block->make_constructed(_block_reg_stats);
            if (!inst->has_option(InstOptions::kUnfollow)) {
              Span<const Operand> operands = inst->operands();
              if (ASMJIT_UNLIKELY(operands.is_empty())) {
                return make_error(Error::kInvalidState);
              }
              if (operands.last().is_label()) {
                LabelNode* label_node;
                ASMJIT_PROPAGATE(cc().label_node_of(Out(label_node), operands.last().as<Label>()));
                RABlock* target_block = _pass.new_block_or_existing_at(label_node);
                if (ASMJIT_UNLIKELY(!target_block)) {
                  return make_error(Error::kOutOfMemory);
                }
                target_block->make_targetable();
                ASMJIT_PROPAGATE(_cur_block->append_successor(target_block));
              }
              else {
                JumpAnnotation* jump_annotation = nullptr;
                _cur_block->add_flags(RABlockFlags::kHasJumpTable);
                if (inst->type() == NodeType::kJump) {
                  jump_annotation = inst->as<JumpNode>()->annotation();
                }
                if (jump_annotation) {
                  RABlockTimestamp timestamp = _pass.next_timestamp();
                  for (uint32_t id : jump_annotation->label_ids()) {
                    LabelNode* label_node;
                    ASMJIT_PROPAGATE(cc().label_node_of(Out(label_node), id));
                    RABlock* target_block = _pass.new_block_or_existing_at(label_node);
                    if (ASMJIT_UNLIKELY(!target_block)) {
                      return make_error(Error::kOutOfMemory);
                    }
                    if (!target_block->has_timestamp(timestamp)) {
                      target_block->set_timestamp(timestamp);
                      target_block->make_targetable();
                      ASMJIT_PROPAGATE(_cur_block->append_successor(target_block));
                    }
                  }
                  ASMJIT_PROPAGATE(share_assignment_across_successors(_cur_block));
                }
                else {
                  ASMJIT_PROPAGATE(blocks_with_unknown_jumps.append(_pass.arena(), _cur_block));
                }
              }
            }
            if (cf == InstControlFlow::kJump) {
              _cur_block = nullptr;
            }
            else {
              node = next;
              if (ASMJIT_UNLIKELY(!node)) {
                return make_error(Error::kInvalidState);
              }
              RABlock* consecutive_block;
              if (node->type() == NodeType::kLabel) {
                if (node->has_pass_data()) {
                  consecutive_block = node->pass_data<RABlock>();
                }
                else {
                  consecutive_block = _pass.new_block(node);
                  if (ASMJIT_UNLIKELY(!consecutive_block)) {
                    return make_error(Error::kOutOfMemory);
                  }
                  node->set_pass_data<RABlock>(consecutive_block);
                }
              }
              else {
                consecutive_block = _pass.new_block(node);
                if (ASMJIT_UNLIKELY(!consecutive_block)) {
                  return make_error(Error::kOutOfMemory);
                }
              }
              _cur_block->add_flags(RABlockFlags::kHasConsecutive);
              ASMJIT_PROPAGATE(_cur_block->prepend_successor(consecutive_block));
              _cur_block = consecutive_block;
              _has_code = false;
              _block_reg_stats.reset();
              if (_cur_block->is_constructed()) {
                break;
              }
              ASMJIT_PROPAGATE(_pass.add_block(consecutive_block));
              log_block(_cur_block, kRootIndentation);
              continue;
            }
          }
          if (cf == InstControlFlow::kReturn) {
            _cur_block->set_last(node);
            _cur_block->make_constructed(_block_reg_stats);
            ASMJIT_PROPAGATE(_cur_block->append_successor(_ret_block));
            _cur_block = nullptr;
          }
        }
      }
      else if (node->type() == NodeType::kLabel) {
        if (!_cur_block) {
          _cur_block = node->pass_data<RABlock>();
          if (_cur_block) {
            if (_cur_block->is_constructed()) {
              break;
            }
          }
          else {
            _cur_block = _pass.new_block(node);
            if (ASMJIT_UNLIKELY(!_cur_block)) {
              return make_error(Error::kOutOfMemory);
            }
            node->set_pass_data<RABlock>(_cur_block);
          }
          _cur_block->make_targetable();
          _has_code = false;
          _block_reg_stats.reset();
          ASMJIT_PROPAGATE(_pass.add_block(_cur_block));
        }
        else {
          if (node->has_pass_data()) {
            RABlock* consecutive = node->pass_data<RABlock>();
            consecutive->make_targetable();
            if (_cur_block == consecutive) {
              if (ASMJIT_UNLIKELY(_has_code)) {
                return make_error(Error::kInvalidState);
              }
            }
            else {
              ASMJIT_ASSERT(_cur_block->last() != node);
              _cur_block->set_last(node->prev());
              _cur_block->add_flags(RABlockFlags::kHasConsecutive);
              _cur_block->make_constructed(_block_reg_stats);
              ASMJIT_PROPAGATE(_cur_block->append_successor(consecutive));
              ASMJIT_PROPAGATE(_pass.add_block(consecutive));
              _cur_block = consecutive;
              _has_code = false;
              _block_reg_stats.reset();
            }
          }
          else {
            if (_has_code || _cur_block == entry_block) {
              ASMJIT_ASSERT(_cur_block->last() != node);
              _cur_block->set_last(node->prev());
              _cur_block->add_flags(RABlockFlags::kHasConsecutive);
              _cur_block->make_constructed(_block_reg_stats);
              RABlock* consecutive = _pass.new_block(node);
              if (ASMJIT_UNLIKELY(!consecutive)) {
                return make_error(Error::kOutOfMemory);
              }
              consecutive->make_targetable();
              ASMJIT_PROPAGATE(_cur_block->append_successor(consecutive));
              ASMJIT_PROPAGATE(_pass.add_block(consecutive));
              _cur_block = consecutive;
              _has_code = false;
              _block_reg_stats.reset();
            }
            node->set_pass_data<RABlock>(_cur_block);
          }
        }
        if (_cur_block && _cur_block != _last_logged_block) {
          log_block(_cur_block, kRootIndentation);
        }
        log_node(node, kRootIndentation);
        if (ASMJIT_UNLIKELY(node->as<LabelNode>()->label_id() == _exit_label_id)) {
          _cur_block->set_last(node);
          _cur_block->make_constructed(_block_reg_stats);
          ASMJIT_PROPAGATE(_pass.add_exit_block(_cur_block));
          _cur_block = nullptr;
        }
      }
      else {
        log_node(node, kCodeIndentation);
        if (node->type() == NodeType::kSentinel) {
          if (node == _func_node->end_node()) {
            if (ASMJIT_UNLIKELY(_cur_block && _has_code)) {
              return make_error(Error::kInvalidState);
            }
            break;
          }
        }
        else if (node->type() == NodeType::kFunc) {
          if (ASMJIT_UNLIKELY(node != _func_node)) {
            return make_error(Error::kInvalidState);
          }
        }
        else {
        }
      }
      node = next;
      if (ASMJIT_UNLIKELY(!node)) {
        return make_error(Error::kInvalidState);
      }
    }
    if (_pass.has_dangling_blocks()) {
      return make_error(Error::kInvalidState);
    }
    for (RABlock* block : blocks_with_unknown_jumps) {
      ASMJIT_PROPAGATE(handle_block_with_unknown_jump(block));
    }
    return _pass.init_shared_assignments(_shared_assignments_map);
  }
  [[nodiscard]]
  Error prepare() noexcept {
    FuncNode* func = _pass.func();
    BaseNode* node = nullptr;
    _func_node = func;
    _ret_block = _pass.new_block_or_existing_at(func->exit_node(), &node);
    if (ASMJIT_UNLIKELY(!_ret_block))
      return make_error(Error::kOutOfMemory);
    _ret_block->make_targetable();
    ASMJIT_PROPAGATE(_pass.add_exit_block(_ret_block));
    if (node != func) {
      _cur_block = _pass.new_block();
      if (ASMJIT_UNLIKELY(!_cur_block))
        return make_error(Error::kOutOfMemory);
    }
    else {
      _cur_block = _ret_block;
    }
    _block_reg_stats.reset();
    _exit_label_id = func->exit_node()->label_id();
    _has_code = false;
    return _pass.add_block(_cur_block);
  }
  void remove_node(BaseNode* node) noexcept {
    log_node(node, kRootIndentation, "<Removed>");
    cc().remove_node(node);
  }
  [[nodiscard]]
  Error handle_block_with_unknown_jump(RABlock* block) noexcept {
    Span blocks = _pass.blocks();
    RABlock* consecutive = block->consecutive();
    for (size_t i = 1; i < blocks.size(); i++) {
      RABlock* candidate = blocks[i];
      if (candidate == consecutive || !candidate->is_targetable()) {
        continue;
      }
      ASMJIT_PROPAGATE(block->append_successor(candidate));
    }
    return share_assignment_across_successors(block);
  }
  [[nodiscard]]
  Error share_assignment_across_successors(RABlock* block) noexcept {
    if (block->successors().size() <= 1u) {
      return Error::kOk;
    }
    RABlock* consecutive = block->consecutive();
    uint32_t shared_assignment_id = Globals::kInvalidId;
    for (RABlock* successor : block->successors()) {
      if (successor == consecutive) {
        continue;
      }
      if (successor->has_shared_assignment_id()) {
        if (shared_assignment_id == Globals::kInvalidId) {
          shared_assignment_id = successor->shared_assignment_id();
        }
        else {
          _shared_assignments_map[successor->shared_assignment_id()] = shared_assignment_id;
        }
      }
      else {
        if (shared_assignment_id == Globals::kInvalidId) {
          ASMJIT_PROPAGATE(new_shared_assignment_id(&shared_assignment_id));
        }
        successor->set_shared_assignment_id(shared_assignment_id);
      }
    }
    return Error::kOk;
  }
  [[nodiscard]]
  Error new_shared_assignment_id(uint32_t* out) noexcept {
    uint32_t id = uint32_t(_shared_assignments_map.size());
    ASMJIT_PROPAGATE(_shared_assignments_map.append(_pass.arena(), id));
    *out = id;
    return Error::kOk;
  }
#ifndef ASMJIT_NO_LOGGING
  template<typename... Args>
  inline void log(const char* fmt, Args&&... args) noexcept {
    if (_logger) {
      _logger->logf(fmt, std::forward<Args>(args)...);
    }
  }
  inline void log_block(RABlock* block, uint32_t indentation = 0) noexcept {
    if (_logger) {
      _log_block(block, indentation);
    }
  }
  inline void log_node(BaseNode* node, uint32_t indentation = 0, const char* action = nullptr) noexcept {
    if (_logger) {
      _log_node(node, indentation, action);
    }
  }
  void _log_block(RABlock* block, uint32_t indentation) noexcept {
    _sb.clear();
    _sb.append_chars(' ', indentation);
    _sb.append_format("{#%u}\n", block->block_id());
    _logger->log(_sb);
    _last_logged_block = block;
  }
  void _log_node(BaseNode* node, uint32_t indentation, const char* action) noexcept {
    _sb.clear();
    _sb.append_chars(' ', indentation);
    if (action) {
      _sb.append(action);
      _sb.append(' ');
    }
    Formatter::format_node(_sb, _format_options, &_cc, node);
    _sb.append('\n');
    _logger->log(_sb);
  }
#else
  template<typename... Args>
  inline void log(const char* fmt, Args&&... args) noexcept {
    Support::maybe_unused(fmt);
    Support::maybe_unused(std::forward<Args>(args)...);
  }
  inline void log_block(RABlock* block, uint32_t indentation = 0) noexcept {
    Support::maybe_unused(block, indentation);
  }
  inline void log_node(BaseNode* node, uint32_t indentation = 0, const char* action = nullptr) noexcept {
    Support::maybe_unused(node, indentation, action);
  }
#endif
};
ASMJIT_END_NAMESPACE
#endif
#endif