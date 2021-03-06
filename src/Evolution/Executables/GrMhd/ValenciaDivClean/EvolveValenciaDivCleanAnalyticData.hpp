// Distributed under the MIT License.
// See LICENSE.txt for details.

#pragma once

#include <vector>

#include "Domain/Creators/RegisterDerivedWithCharm.hpp"
#include "Domain/Tags.hpp"
#include "ErrorHandling/FloatingPointExceptions.hpp"
#include "Evolution/Actions/ComputeTimeDerivative.hpp"
#include "Evolution/Actions/ComputeVolumeFluxes.hpp"
#include "Evolution/Actions/ComputeVolumeSources.hpp"
#include "Evolution/Conservative/UpdateConservatives.hpp"
#include "Evolution/Conservative/UpdatePrimitives.hpp"
#include "Evolution/DiscontinuousGalerkin/DgElementArray.hpp"
#include "Evolution/DiscontinuousGalerkin/Limiters/LimiterActions.hpp"
#include "Evolution/DiscontinuousGalerkin/Limiters/Minmod.hpp"
#include "Evolution/DiscontinuousGalerkin/Limiters/Tags.hpp"
#include "Evolution/DiscontinuousGalerkin/ObserveFields.hpp"
#include "Evolution/EventsAndTriggers/Actions/RunEventsAndTriggers.hpp"  // IWYU pragma: keep
#include "Evolution/EventsAndTriggers/Event.hpp"
#include "Evolution/EventsAndTriggers/EventsAndTriggers.hpp"  // IWYU pragma: keep
#include "Evolution/EventsAndTriggers/Tags.hpp"
#include "Evolution/Systems/GrMhd/ValenciaDivClean/FixConservatives.hpp"
#include "Evolution/Systems/GrMhd/ValenciaDivClean/Initialize.hpp"
#include "Evolution/Systems/GrMhd/ValenciaDivClean/NewmanHamlin.hpp"
#include "Evolution/Systems/GrMhd/ValenciaDivClean/PalenzuelaEtAl.hpp"
#include "Evolution/Systems/GrMhd/ValenciaDivClean/System.hpp"
#include "Evolution/Systems/GrMhd/ValenciaDivClean/Tags.hpp"
#include "Evolution/VariableFixing/Actions.hpp"
#include "Evolution/VariableFixing/FixToAtmosphere.hpp"
#include "Evolution/VariableFixing/Tags.hpp"
#include "IO/Observer/Actions.hpp"
#include "IO/Observer/Helpers.hpp"
#include "IO/Observer/ObserverComponent.hpp"
#include "IO/Observer/RegisterObservers.hpp"
#include "NumericalAlgorithms/DiscontinuousGalerkin/Actions/ApplyBoundaryFluxesLocalTimeStepping.hpp"
#include "NumericalAlgorithms/DiscontinuousGalerkin/Actions/ApplyFluxes.hpp"
#include "NumericalAlgorithms/DiscontinuousGalerkin/Actions/FluxCommunication.hpp"
#include "NumericalAlgorithms/DiscontinuousGalerkin/NumericalFluxes/LocalLaxFriedrichs.hpp"
#include "NumericalAlgorithms/DiscontinuousGalerkin/Tags.hpp"
#include "Options/Options.hpp"
#include "Parallel/Actions/TerminatePhase.hpp"
#include "Parallel/InitializationFunctions.hpp"
#include "Parallel/PhaseDependentActionList.hpp"
#include "Parallel/RegisterDerivedClassesWithCharm.hpp"
#include "PointwiseFunctions/AnalyticData/GrMhd/CylindricalBlastWave.hpp"
#include "PointwiseFunctions/AnalyticData/Tags.hpp"
#include "PointwiseFunctions/Hydro/Tags.hpp"
#include "Time/Actions/AdvanceTime.hpp"
#include "Time/Actions/ChangeStepSize.hpp"
#include "Time/Actions/RecordTimeStepperData.hpp"
#include "Time/Actions/SelfStartActions.hpp"  // IWYU pragma: keep
#include "Time/Actions/UpdateU.hpp"
#include "Time/StepChoosers/Cfl.hpp"
#include "Time/StepChoosers/Constant.hpp"
#include "Time/StepChoosers/Increase.hpp"
#include "Time/StepChoosers/StepChooser.hpp"
#include "Time/StepControllers/StepController.hpp"
#include "Time/Tags.hpp"
#include "Time/TimeSteppers/TimeStepper.hpp"
#include "Time/Triggers/TimeTriggers.hpp"
#include "Utilities/Functional.hpp"
#include "Utilities/TMPL.hpp"

/// \cond
namespace Frame {
struct Inertial;
}  // namespace Frame
namespace Parallel {
template <typename Metavariables>
class CProxy_ConstGlobalCache;
}  // namespace Parallel
/// \endcond

struct EvolutionMetavars {
  using analytic_data = grmhd::AnalyticData::CylindricalBlastWave;

  using system = grmhd::ValenciaDivClean::System<
      typename analytic_data::equation_of_state_type>;
  static constexpr size_t thermodynamic_dim = system::thermodynamic_dim;
  using temporal_id = Tags::TimeId;
  static constexpr bool local_time_stepping = false;
  using analytic_data_tag = OptionTags::AnalyticData<analytic_data>;
  using analytic_variables_tags =
      typename system::primitive_variables_tag::tags_list;
  using equation_of_state_tag = hydro::Tags::EquationOfState<
      typename analytic_data_tag::type::equation_of_state_type>;
  using normal_dot_numerical_flux = OptionTags::NumericalFlux<
      dg::NumericalFluxes::LocalLaxFriedrichs<system>>;
  // GRMHD is only implemented in 3D.
  // Do not limit the divergence-cleaning field Phi
  using limiter = OptionTags::Limiter<Limiters::Minmod<
      3, tmpl::list<grmhd::ValenciaDivClean::Tags::TildeD,
                    grmhd::ValenciaDivClean::Tags::TildeTau,
                    grmhd::ValenciaDivClean::Tags::TildeS<Frame::Inertial>,
                    grmhd::ValenciaDivClean::Tags::TildeB<Frame::Inertial>>>>;

  // public for use by the Charm++ registration code
  using events = tmpl::list<
      dg::Events::Registrars::ObserveFields<
          3,
          tmpl::append<
              db::get_variables_tags_list<system::variables_tag>,
              db::get_variables_tags_list<system::primitive_variables_tag>>>>;
  using triggers = Triggers::time_triggers;

  using step_choosers =
      tmpl::list<StepChoosers::Registrars::Cfl<3, Frame::Inertial>,
                 StepChoosers::Registrars::Constant,
                 StepChoosers::Registrars::Increase>;
  using ordered_list_of_primitive_recovery_schemes = tmpl::list<
      grmhd::ValenciaDivClean::PrimitiveRecoverySchemes::NewmanHamlin,
      grmhd::ValenciaDivClean::PrimitiveRecoverySchemes::PalenzuelaEtAl>;

  struct ObservationType {};
  using element_observation_type = ObservationType;

  using observed_reduction_data_tags =
      observers::collect_reduction_data_tags<Event<events>::creatable_classes>;

  using compute_rhs = tmpl::flatten<
      tmpl::list<Actions::ComputeVolumeFluxes,
                 dg::Actions::SendDataForFluxes<EvolutionMetavars>,
                 Actions::ComputeVolumeSources, Actions::ComputeTimeDerivative,
                 dg::Actions::ReceiveDataForFluxes<EvolutionMetavars>,
                 tmpl::conditional_t<local_time_stepping, tmpl::list<>,
                                     dg::Actions::ApplyFluxes>,
                 Actions::RecordTimeStepperData>>;

  using update_variables = tmpl::flatten<tmpl::list<
      tmpl::conditional_t<local_time_stepping,
                          dg::Actions::ApplyBoundaryFluxesLocalTimeStepping,
                          tmpl::list<>>,
      Actions::UpdateU, Limiters::Actions::SendData<EvolutionMetavars>,
      Limiters::Actions::Limit<EvolutionMetavars>,
      VariableFixing::Actions::FixVariables<VariableFixing::FixConservatives>,
      Actions::UpdatePrimitives>>;

  enum class Phase {
    Initialization,
    InitializeTimeStepperHistory,
    RegisterWithObserver,
    Evolve,
    Exit
  };

  using component_list = tmpl::list<
      observers::Observer<EvolutionMetavars>,
      observers::ObserverWriter<EvolutionMetavars>,
      DgElementArray<
          EvolutionMetavars,
          tmpl::list<
              Parallel::PhaseActions<
                  Phase, Phase::Initialization,
                  tmpl::list<
                      grmhd::ValenciaDivClean::Actions::Initialize<3>,
                      VariableFixing::Actions::FixVariables<
                          VariableFixing::FixToAtmosphere<thermodynamic_dim>>,
                      Actions::UpdateConservatives>>,

              Parallel::PhaseActions<
                  Phase, Phase::InitializeTimeStepperHistory,
                  tmpl::flatten<tmpl::list<SelfStart::self_start_procedure<
                      compute_rhs, update_variables>>>>,

              Parallel::PhaseActions<
                  Phase, Phase::RegisterWithObserver,
                  tmpl::list<Actions::AdvanceTime,
                             observers::Actions::RegisterWithObservers<
                                 observers::RegisterObservers<
                                     element_observation_type>>,
                             Parallel::Actions::TerminatePhase>>,

              Parallel::PhaseActions<
                  Phase, Phase::Evolve,
                  tmpl::flatten<tmpl::list<
                      VariableFixing::Actions::FixVariables<
                          VariableFixing::FixToAtmosphere<thermodynamic_dim>>,
                      Actions::UpdateConservatives,
                      Actions::RunEventsAndTriggers,
                      tmpl::conditional_t<
                          local_time_stepping,
                          Actions::ChangeStepSize<step_choosers>, tmpl::list<>>,
                      compute_rhs, update_variables, Actions::AdvanceTime>>>>,
          grmhd::ValenciaDivClean::Actions::Initialize<
              3>::AddOptionsToDataBox>>;

  using const_global_cache_tag_list =
      tmpl::list<analytic_data_tag,
                 OptionTags::TypedTimeStepper<tmpl::conditional_t<
                     local_time_stepping, LtsTimeStepper, TimeStepper>>,
                 OptionTags::DampingParameter,
                 OptionTags::EventsAndTriggers<events, triggers>>;

  using domain_creator_tag = OptionTags::DomainCreator<3, Frame::Inertial>;

  static constexpr OptionString help{
      "Evolve analytic data using the Valencia formulation of the GRMHD system "
      "with divergence cleaning.\n\n"
      "Analytic data: cylindrical blast wave\n"
      "Slope limiter: Minmod family\n"
      "Numerical flux: Local Lax-Friedrichs\n"
      "Variable fixing: FixConservatives and FixToAtmosphere\n"
      "Local time-stepping: none\n"
      "Boundary conditions: only periodic are currently supported\n"};

  static Phase determine_next_phase(
      const Phase& current_phase,
      const Parallel::CProxy_ConstGlobalCache<
          EvolutionMetavars>& /*cache_proxy*/) noexcept {
    switch (current_phase) {
      case Phase::Initialization:
        return Phase::InitializeTimeStepperHistory;
      case Phase::InitializeTimeStepperHistory:
        return Phase::RegisterWithObserver;
      case Phase::RegisterWithObserver:
        return Phase::Evolve;
      case Phase::Evolve:
        return Phase::Exit;
      case Phase::Exit:
        ERROR(
            "Should never call determine_next_phase with the current phase "
            "being 'Exit'");
      default:
        ERROR(
            "Unknown type of phase. Did you static_cast<Phase> an integral "
            "value?");
    }
  }
};

static const std::vector<void (*)()> charm_init_node_funcs{
    &setup_error_handling,
    &domain::creators::register_derived_with_charm,
    &Parallel::register_derived_classes_with_charm<
        Event<metavariables::events>>,
    &Parallel::register_derived_classes_with_charm<
        StepChooser<EvolutionMetavars::step_choosers>>,
    &Parallel::register_derived_classes_with_charm<StepController>,
    &Parallel::register_derived_classes_with_charm<TimeStepper>,
    &Parallel::register_derived_classes_with_charm<
        Trigger<metavariables::triggers>>};

static const std::vector<void (*)()> charm_init_proc_funcs{
    &enable_floating_point_exceptions};
