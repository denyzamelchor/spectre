// Distributed under the MIT License.
// See LICENSE.txt for details.

#pragma once

#include "AlgorithmSingleton.hpp"
#include "DataStructures/DataBox/DataBox.hpp"
#include "DataStructures/DataBox/Prefixes.hpp"
#include "DataStructures/DenseMatrix.hpp"
#include "IO/Observer/Actions.hpp"
#include "Informer/Tags.hpp"
#include "NumericalAlgorithms/LinearSolver/Observe.hpp"
#include "NumericalAlgorithms/LinearSolver/Tags.hpp"
#include "Options/Options.hpp"
#include "Parallel/AddOptionsToDataBox.hpp"
#include "Parallel/ConstGlobalCache.hpp"
#include "Parallel/Info.hpp"
#include "Parallel/Invoke.hpp"
#include "Utilities/TMPL.hpp"

/// \cond
namespace tuples {
template <typename...>
class TaggedTuple;
}  // namespace tuples
namespace LinearSolver {
namespace gmres_detail {
template <typename Metavariables>
struct InitializeResidualMonitor;
}  // namespace gmres_detail
}  // namespace LinearSolver
namespace Convergence {
struct Criteria;
}  // namespace Convergence
/// \endcond

namespace LinearSolver {
namespace gmres_detail {

template <typename Metavariables>
struct ResidualMonitor {
  using chare_type = Parallel::Algorithms::Singleton;
  using const_global_cache_tag_list =
      tmpl::list<LinearSolver::OptionTags::Verbosity,
                 LinearSolver::OptionTags::ConvergenceCriteria>;
  using options = tmpl::list<>;
  using add_options_to_databox = Parallel::AddNoOptionsToDataBox;
  using metavariables = Metavariables;
  using phase_dependent_action_list = tmpl::list<
      Parallel::PhaseActions<
          typename Metavariables::Phase, Metavariables::Phase::Initialization,
          tmpl::list<InitializeResidualMonitor<Metavariables>>>,

      Parallel::PhaseActions<
          typename Metavariables::Phase,
          Metavariables::Phase::RegisterWithObserver,
          tmpl::list<observers::Actions::RegisterSingletonWithObserverWriter<
              LinearSolver::observe_detail::Registration>>>>;

  static void initialize(Parallel::CProxy_ConstGlobalCache<
                         Metavariables>& /*global_cache*/) noexcept {}

  static void execute_next_phase(
      const typename Metavariables::Phase next_phase,
      Parallel::CProxy_ConstGlobalCache<Metavariables>& global_cache) noexcept {
    auto& local_cache = *(global_cache.ckLocalBranch());
    Parallel::get_parallel_component<ResidualMonitor>(local_cache)
        .start_phase(next_phase);
  }
};

template <typename Metavariables>
struct InitializeResidualMonitor {
 private:
  using fields_tag = typename Metavariables::system::fields_tag;
  using residual_magnitude_tag = db::add_tag_prefix<
      LinearSolver::Tags::Magnitude,
      db::add_tag_prefix<LinearSolver::Tags::Residual, fields_tag>>;
  using initial_residual_magnitude_tag =
      db::add_tag_prefix<LinearSolver::Tags::Initial, residual_magnitude_tag>;
  using orthogonalization_iteration_id_tag =
      db::add_tag_prefix<LinearSolver::Tags::Orthogonalization,
                         LinearSolver::Tags::IterationId>;
  using orthogonalization_history_tag =
      db::add_tag_prefix<LinearSolver::Tags::OrthogonalizationHistory,
                         fields_tag>;

 public:
  template <
      typename DbTagsList, typename... InboxTags, typename ArrayIndex,
      typename ActionList, typename ParallelComponent,
      Requires<not tmpl::list_contains_v<DbTagsList, residual_magnitude_tag>> =
          nullptr>
  static auto apply(db::DataBox<DbTagsList>& box,
                    const tuples::TaggedTuple<InboxTags...>& /*inboxes*/,
                    const Parallel::ConstGlobalCache<Metavariables>& /*cache*/,
                    const ArrayIndex& /*array_index*/,
                    const ActionList /*meta*/,
                    const ParallelComponent* const /*meta*/) noexcept {
    auto init_box = db::create_from<
        db::RemoveTags<>,
        db::AddSimpleTags<
            residual_magnitude_tag, initial_residual_magnitude_tag,
            LinearSolver::Tags::IterationId, orthogonalization_iteration_id_tag,
            orthogonalization_history_tag>,
        db::AddComputeTags<
            LinearSolver::Tags::HasConvergedCompute<fields_tag>>>(
        std::move(box), std::numeric_limits<double>::signaling_NaN(),
        std::numeric_limits<double>::signaling_NaN(),
        db::item_type<LinearSolver::Tags::IterationId>{0},
        db::item_type<orthogonalization_iteration_id_tag>{0},
        DenseMatrix<double>{2, 1, 0.});
    return std::make_tuple(std::move(init_box), true);
  }

  template <typename DbTagsList, typename... InboxTags, typename ArrayIndex,
            typename ActionList, typename ParallelComponent,
            Requires<tmpl::list_contains_v<DbTagsList,
                                           residual_magnitude_tag>> = nullptr>
  static std::tuple<db::DataBox<DbTagsList>&&, bool> apply(
      const db::DataBox<DbTagsList>& /*box*/,
      const tuples::TaggedTuple<InboxTags...>& /*inboxes*/,
      const Parallel::ConstGlobalCache<Metavariables>& /*cache*/,
      const ArrayIndex& /*array_index*/, const ActionList /*meta*/,
      const ParallelComponent* const /*meta*/) noexcept {
    ERROR(
        "Re-initialization not supported. Did you forget to terminate the "
        "initialization phase?");
  }
};

}  // namespace gmres_detail
}  // namespace LinearSolver
